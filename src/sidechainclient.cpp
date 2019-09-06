// Copyright (c) 2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <sidechainclient.h>

#include <bmmcache.h>
#include <chainparams.h>
#include <core_io.h>
#include <miner.h>
#include <sidechain.h>
#include <streams.h>
#include <uint256.h>
#include <univalue.h>
#include <utilmoneystr.h>
#include <utilstrencodings.h>
#include <util.h>

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/foreach.hpp>

using boost::asio::ip::tcp;

SidechainClient::SidechainClient()
{

}

bool SidechainClient::BroadcastWTPrime(const std::string& hex)
{
    // JSON for sending the WT^ to mainchain via HTTP-RPC
    std::string json;
    json.append("{\"jsonrpc\": \"1.0\", \"id\":\"SidechainClient\", ");
    json.append("\"method\": \"receivewtprime\", \"params\": ");
    json.append("[");
    json.append(UniValue(SIDECHAIN_TEST).write());
    json.append(",\"");
    json.append(hex);
    json.append("\"] }");

    // TODO Read result
    // the mainchain will return the txid if WT^ has been received
    boost::property_tree::ptree ptree;
    return SendRequestToMainchain(json, ptree);
}

// TODO return bool & state / fail string
std::vector<SidechainDeposit> SidechainClient::UpdateDeposits(const std::string& strBuildHash)
{
    // List of deposits in sidechain format for DB
    std::vector<SidechainDeposit> incoming;

    // JSON for requesting sidechain deposits via mainchain HTTP-RPC
    std::string json;
    json.append("{\"jsonrpc\": \"1.0\", \"id\":\"SidechainClient\", ");
    json.append("\"method\": \"listsidechaindeposits\", \"params\": ");
    json.append("[\"");
    json.append(strBuildHash);
    json.append("\"");
    json.append("] }");

    // Try to request deposits from mainchain
    boost::property_tree::ptree ptree;
    if (!SendRequestToMainchain(json, ptree)) {
        // TODO LogPrintf("ERROR Sidechain client failed to request new deposits\n");
        return incoming;  // TODO return false
    }

    // Process deposits
    BOOST_FOREACH(boost::property_tree::ptree::value_type &value, ptree.get_child("result")) {
        // Looping through list of deposits
        SidechainDeposit deposit;
        uint32_t n = 0;
        BOOST_FOREACH(boost::property_tree::ptree::value_type &v, value.second.get_child("")) {
            // Looping through this deposit's members
            if (v.first == "nsidechain") {
                // Read sidechain number
                std::string data = v.second.data();
                if (!data.length())
                    continue;
                uint8_t nSidechain = std::stoi(data);
                if (nSidechain != SIDECHAIN_TEST)
                    continue;

                deposit.nSidechain = nSidechain;
            }
            else
            if (v.first == "keyid") {
                // Read keyID
                std::string data = v.second.data();
                if (!data.length())
                    continue;

                deposit.keyID.SetHex(data);
            }
            else
            if (v.first == "txhex") {
                // Read deposit transaction hex
                std::string data = v.second.data();
                if (!data.length())
                    continue;
                if (!IsHex(data))
                    continue;
                if (!DecodeHexTx(deposit.dtx, data))
                    continue;
            }
            else
            if (v.first == "n") {
                // Read deposit output index
                std::string data = v.second.data();
                if (!data.length())
                    continue;

                n = (unsigned int)std::stoi(data);
            }
            else
            if (v.first == "proofhex") {
                // Read serialized merkleblock txout proof
                std::string data = v.second.data();
                if (!data.length())
                    continue;
                if (!IsHex(data))
                    continue;

                CDataStream ssMB(ParseHex(data), SER_NETWORK, PROTOCOL_VERSION | SERIALIZE_TRANSACTION_NO_WITNESS);
                ssMB >> deposit.proof;
            }
        }

        if (n >= deposit.dtx.vout.size())
            continue;

        // Get the user payout amount from the deposit output
        deposit.amtUserPayout = deposit.dtx.vout[n].nValue;

        // TODO check the deposit output 'N' scriptPubKey (compare to THIS_SIDECHAIN)

        // Add this deposit to the list
        incoming.push_back(deposit);
    }
    // TODO LogPrintf("Sidechain client received %d deposits\n", incoming.size());

    // return valid (in terms of format) deposits in sidechain format
    return incoming;
}

bool SidechainClient::VerifyCriticalHashProof(const std::string& criticalProof, uint256 &txid)
{
    // JSON for verifying critical hash
    std::string json;
    json.append("{\"jsonrpc\": \"1.0\", \"id\":\"SidechainClient\", ");
    json.append("\"method\": \"verifytxoutproof\", \"params\": ");
    json.append("[\"");
    json.append(criticalProof);
    json.append("\"] }");

    boost::property_tree::ptree ptree;
    if (!SendRequestToMainchain(json, ptree))
        return false;

    BOOST_FOREACH(boost::property_tree::ptree::value_type &value, ptree.get_child("result")) {
        std::string data = value.second.data();
        if (data.size() != 64)
            continue;
        txid = uint256S(data);
    }
    return true;
}

bool SidechainClient::RequestBMMProof(const uint256& hashMainBlock, const uint256& hashBMMBlock, SidechainBMMProof& proof)
{
    // JSON for requesting BMM proof via mainchain HTTP-RPC
    std::string json;
    json.append("{\"jsonrpc\": \"1.0\", \"id\":\"SidechainClient\", ");
    json.append("\"method\": \"getbmmproof\", \"params\": ");
    json.append("[\"");
    json.append(hashMainBlock.ToString());
    json.append("\",\"");
    json.append(hashBMMBlock.ToString());
    json.append("\"");
    json.append("] }");

    // Try to request BMM proof from mainchain
    boost::property_tree::ptree ptree;
    if (!SendRequestToMainchain(json, ptree)) {
        // Can be enabled for debug -- too noisy
        // LogPrintf("ERROR Sidechain client failed to request BMM proof\n");
        return false;
    }

    // Process result
    BOOST_FOREACH(boost::property_tree::ptree::value_type &value, ptree.get_child("result")) {
        BOOST_FOREACH(boost::property_tree::ptree::value_type &v, value.second.get_child("")) {
            // Looping through members
            if (v.first == "coinbasehex") {
                // Read coinbase hex
                std::string data = v.second.data();
                if (!data.length())
                    continue;

                proof.coinbaseHex = data;
            }
            else
            if (v.first == "proof") {
                // Read TxOut proof
                std::string data = v.second.data();
                if (!data.length())
                    continue;

                proof.txOutProof = data;
            }
        }
    }

    if (proof.HasProof()) {
        LogPrintf("Sidechain client received BMM proof for: %s\n", hashBMMBlock.ToString());
        return true;
    } else {
        LogPrintf("Sidechain client received no BMM proof.\n");
        return false;
    }
}

// TODO rename
uint256 SidechainClient::SendBMMCriticalDataRequest(const uint256& hashCritical, const uint256& hashBlockMain, int nHeight, const CAmount& amount)
{
    uint256 txid = uint256();
    LOCK(cs_main);
    std::string strPrevHash = hashBlockMain.ToString();

    // JSON for sending critical data request to mainchain via mainchain HTTP-RPC
    std::string json;
    json.append("{\"jsonrpc\": \"1.0\", \"id\":\"SidechainClient\", ");
    json.append("\"method\": \"createbmmcriticaldatatx\", \"params\": ");
    json.append("[\"");
    // TODO use amount
    json.append(ValueFromAmount(DEFAULT_CRITICAL_DATA_AMOUNT).write());
    json.append("\",");
    json.append(UniValue(nHeight).write());
    json.append(",\"");
    json.append(hashCritical.ToString());
    json.append("\",");
    json.append(UniValue(SIDECHAIN_TEST).write());
    json.append(",");
    json.append(UniValue(0).write());
    json.append(",\"");
    json.append(strPrevHash.substr(strPrevHash.size() - 4, strPrevHash.size() - 1));
    json.append("\"");
    json.append("] }");

    // Try to send critical data request to mainchain
    boost::property_tree::ptree ptree;
    if (!SendRequestToMainchain(json, ptree)) {
        LogPrintf("ERROR Sidechain client failed to send critical data request!\n");
        return txid; // TODO
    }

    // Process result
    BOOST_FOREACH(boost::property_tree::ptree::value_type &value, ptree.get_child("result")) {
        BOOST_FOREACH(boost::property_tree::ptree::value_type &v, value.second.get_child("")) {
            // Looping through members
            if (v.first == "txid") {
                // Read txid
                std::string data = v.second.data();
                if (!data.length())
                    continue;

                txid = uint256S(data);
            }
        }
    }
    if (!txid.IsNull())
        LogPrintf("Sidechain client created critical data request. TXID: %s\n", txid.ToString());

    return txid;
}

std::vector<uint256> SidechainClient::RequestMainBlockHashes()
{
    std::vector<uint256> vHash;

    // JSON for sending critical data request to mainchain via mainchain HTTP-RPC
    std::string json;
    json.append("{\"jsonrpc\": \"1.0\", \"id\":\"SidechainClient\", ");
    json.append("\"method\": \"listpreviousblockhashes\", \"params\": ");
    json.append("[]");
    json.append("}");

    // Try to request main:block hashes from mainchain
    boost::property_tree::ptree ptree;
    if (!SendRequestToMainchain(json, ptree)) {
        LogPrintf("ERROR Sidechain client failed to request main:block hashes\n");
        return vHash; // TODO boolean
    }

    // Process result
    BOOST_FOREACH(boost::property_tree::ptree::value_type &value, ptree.get_child("result")) {
        BOOST_FOREACH(boost::property_tree::ptree::value_type &v, value.second.get_child("")) {
            // Looping through members
            if (v.first == "hash") {
                // Read txid
                std::string data = v.second.data();
                if (!data.length())
                    continue;

                uint256 hashBlock = uint256S(data);
                if (!hashBlock.IsNull())
                    vHash.push_back(hashBlock);
            }
        }
    }
    // Can be enabled for debug -- too noisy
    //LogPrintf("Sidechain client received %d main:block hashes.\n", vHash.size());

    return vHash;
}

bool SidechainClient::GetCTIP(std::pair<uint256, uint32_t>& ctip)
{
    // JSON for requesting sidechain CTIP via mainchain HTTP-RPC
    std::string json;
    json.append("{\"jsonrpc\": \"1.0\", \"id\":\"SidechainClient\", ");
    json.append("\"method\": \"listsidechainctip\", \"params\": ");
    json.append("[");
    json.append(std::to_string(SIDECHAIN_TEST));
    json.append("] }");

    // Try to request CTIP from mainchain
    boost::property_tree::ptree ptree;
    if (!SendRequestToMainchain(json, ptree)) {
        // TODO LogPrintf("ERROR Sidechain client failed to request CTIP\n");
        return false;
    }

    // Process CTIP
    uint256 txid;
    uint32_t n;
    BOOST_FOREACH(boost::property_tree::ptree::value_type &value, ptree.get_child("result")) {
        if (value.first == "n") {
            // Read n
            std::string data = value.second.data();
            if (!data.length())
                continue;
            n = std::stoi(data);
        }
        else
        if (value.first == "txid") {
            // Read TXID
            std::string data = value.second.data();
            if (!data.length())
                continue;

            txid = uint256S(data);
        }
    }
    // TODO LogPrintf("Sidechain client received CTIP\n");

    ctip = std::make_pair(txid, n);

    return true;
}

bool SidechainClient::RefreshBMM(std::string& strError, uint256& hashCreated, uint256& hashConnected)
{
    // Get updated list of recent main:blocks
    std::vector<uint256> vHashMainBlockNew = RequestMainBlockHashes();

    if (vHashMainBlockNew.empty()) {
        strError = "Failed to request new mainchain block hashes!";
        return false;
    }

    // Update hashBlockLastSeen and keep a backup for later
    uint256 hashMainBlockLastSeenOld = bmmCache.GetLastMainBlockHash();
    bmmCache.UpdateMainBlocks(vHashMainBlockNew);

    // Get our cached BMM blocks
    std::vector<CBlock> vBMMCache = bmmCache.GetBMMBlockCache();
    if (vBMMCache.empty()) {
        // If we don't have any existing BMM requests cached, create our first
        CBlock block;
        if (CreateBMMBlock(block, strError)) {
            // TODO refactor so that we don't create a BMM request here - it
            // happens later in the function as well.
            // TODO check return value
            hashCreated = block.GetBlindHash();
            SendBMMCriticalDataRequest(block.GetBlindHash(), vHashMainBlockNew.back(), 0, 0);
            return true;
        } else {
            return false;
        }
    }

    // TODO this could be more efficient
    // Check new main:blocks for our bmm requests
    for (const uint256& u : vHashMainBlockNew) {
        // Check main:block for any of our current BMM requests
        for (const CBlock& b : vBMMCache) {
            const uint256& hashBMMBlock = b.GetBlindHash();
            // Send 'getbmmproof' rpc request to mainchain
            SidechainBMMProof proof;
            proof.hashBMMBlock = hashBMMBlock;
            if (RequestBMMProof(u, hashBMMBlock, proof)) {
                CBlock block = b;

                block.criticalProof = proof.txOutProof;

                if (!DecodeHexTx(block.criticalTx, proof.coinbaseHex))
                    continue;

                // Submit BMM block
                if (!SubmitBMMBlock(block))
                    continue;
                else
                    hashConnected = block.GetHash();
            }
        }
    }

    // Was there a new mainchain block?
    if (hashMainBlockLastSeenOld != vHashMainBlockNew.back()) {
        // Clear out the bmm cache, the old requests are invalid now
        bmmCache.ClearBMMBlocks();

        // Create a new BMM request (old ones have expired)
        CBlock block;
        if (CreateBMMBlock(block, strError)) {
            // Create BMM critical data request
            hashCreated = block.GetBlindHash();
            SendBMMCriticalDataRequest(block.GetBlindHash(), vHashMainBlockNew.back());
        } else {
            return false;
        }
    }
    return true;
}

bool SidechainClient::CreateBMMBlock(CBlock& block, std::string& strError)
{
    CScript scriptPubKey;
    if (!BlockAssembler(Params()).GenerateBMMBlock(scriptPubKey, block, strError)) {
        return false;
    }

    if (!bmmCache.StoreBMMBlock(block)) {
        // Failed to store BMM candidate block
        strError = "Failed to store BMM block!\n";
        return false;
    }

    return true;
}

bool SidechainClient::SubmitBMMBlock(const CBlock& block)
{
    std::shared_ptr<const CBlock> shared_pblock = std::make_shared<const CBlock>(block);
    return ProcessNewBlock(Params(), shared_pblock, true, NULL);
}

bool SidechainClient::GetAverageFees(int nBlocks, int nStartHeight, CAmount& nAverageFee)
{
    // JSON for 'getaveragefees' mainchain HTTP-RPC
    std::string json;
    json.append("{\"jsonrpc\": \"1.0\", \"id\":\"SidechainClient\", ");
    json.append("\"method\": \"getaveragefee\", \"params\": ");
    json.append("[");
    json.append(std::to_string(nBlocks));
    json.append(",");
    json.append(std::to_string(nStartHeight));
    json.append("]");
    json.append("}");

    // Try to request average fees from mainchain
    boost::property_tree::ptree ptree;
    if (!SendRequestToMainchain(json, ptree)) {
        LogPrintf("ERROR Sidechain client failed to request average fees\n");
        return false;
    }

    // Process result
    BOOST_FOREACH(boost::property_tree::ptree::value_type &value, ptree.get_child("result")) {
        // Looping through members
        if (value.first == "feeaverage") {
            // Read
            std::string data = value.second.data();
            if (!data.length()) {
                LogPrintf("ERROR Sidechain client received invalid data\n");
                return false;
            }

            if (ParseMoney(data, nAverageFee)) {
                LogPrintf("Sidechain client received average mainchain fee: %d.\n", nAverageFee);
                return true;
            }
        }
    }
    return false;
}

bool SidechainClient::GetBlockCount(int& nBlocks)
{
    // JSON for 'getblockcount' mainchain HTTP-RPC
    std::string json;
    json.append("{\"jsonrpc\": \"1.0\", \"id\":\"SidechainClient\", ");
    json.append("\"method\": \"getblockcount\", \"params\": ");
    json.append("[] }");

    // Try to request mainchain block count
    boost::property_tree::ptree ptree;
    if (!SendRequestToMainchain(json, ptree)) {
        LogPrintf("ERROR Sidechain client failed to request block count\n");
        return false;
    }

    // Process result
    nBlocks = ptree.get("result", 0);

    return nBlocks > 0;
}

bool SidechainClient::GetWorkScore(const uint256& hashWTPrime, int& nWorkScore)
{
    // JSON for 'getworkscore' mainchain HTTP-RPC
    std::string json;
    json.append("{\"jsonrpc\": \"1.0\", \"id\":\"SidechainClient\", ");
    json.append("\"method\": \"getworkscore\", \"params\": ");
    json.append("[");
    json.append(std::to_string(SIDECHAIN_TEST));
    json.append("\"");
    json.append(hashWTPrime.ToString());
    json.append("\"");
    json.append("] }");

    // Try to request mainchain block count
    boost::property_tree::ptree ptree;
    if (!SendRequestToMainchain(json, ptree)) {
        LogPrintf("ERROR Sidechain client failed to request workscore\n");
        return false;
    }

    // Process result, note that starting workscore on mainchain is 1
    nWorkScore = ptree.get("workscore", 0);

    return nWorkScore > 0;
}

bool SidechainClient::ListWTPrimes(std::vector<uint256>& vHashWTPrime)
{
    // JSON for 'listwtprimes' mainchain HTTP-RPC
    std::string json;
    json.append("{\"jsonrpc\": \"1.0\", \"id\":\"SidechainClient\", ");
    json.append("\"method\": \"listwtprimes\", \"params\": ");
    json.append("[");
    json.append(std::to_string(SIDECHAIN_TEST));
    json.append("] }");

    // Try to request mainchain block count
    boost::property_tree::ptree ptree;
    if (!SendRequestToMainchain(json, ptree)) {
        LogPrintf("ERROR Sidechain client failed to request WT^(s)\n");
        return false;
    }

    // Process result
    BOOST_FOREACH(boost::property_tree::ptree::value_type &value, ptree.get_child("result")) {
        BOOST_FOREACH(boost::property_tree::ptree::value_type &v, value.second.get_child("")) {
            // Looping through members
            if (v.first == "hashwtprime") {
                // Read txid
                std::string data = v.second.data();
                if (!data.length())
                    continue;

                uint256 hash = uint256S(data);
                if (!hash.IsNull())
                    vHashWTPrime.push_back(hash);
            }
        }
    }

    return vHashWTPrime.size() > 0;
}

bool SidechainClient::SendRequestToMainchain(const std::string& json, boost::property_tree::ptree &ptree)
{
    // Format user:pass for authentication
    std::string auth = gArgs.GetArg("-rpcuser", "") + ":" + gArgs.GetArg("-rpcpassword", "");
    if (auth == ":")
        return false;

    // Mainnet RPC = 8332
    // Testnet RPC = 18332
    // Regtest RPC = 18443

    int port = gArgs.GetArg("-mainchainrpcport", 8332);

    try {
        // Setup BOOST ASIO for a synchronus call to the mainchain
        boost::asio::io_service io_service;
        tcp::resolver resolver(io_service);
        tcp::resolver::query query("127.0.0.1", std::to_string(port));
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        tcp::resolver::iterator end;

        tcp::socket socket(io_service);
        boost::system::error_code error = boost::asio::error::host_not_found;

        // Try to connect
        while (error && endpoint_iterator != end)
        {
          socket.close();
          socket.connect(*endpoint_iterator++, error);
        }

        if (error) throw boost::system::system_error(error);

        // HTTP request (package the json for sending)
        boost::asio::streambuf output;
        std::ostream os(&output);
        os << "POST / HTTP/1.1\n";
        os << "Host: 127.0.0.1\n";
        os << "Content-Type: application/json\n";
        os << "Authorization: Basic " << EncodeBase64(auth) << std::endl;
        os << "Connection: close\n";
        os << "Content-Length: " << json.size() << "\n\n";
        os << json;

        // Send the request
        boost::asio::write(socket, output);

        // Read the reponse
        std::string data;
        for (;;)
        {
            boost::array<char, 4096> buf;

            // Read until end of file (socket closed)
            boost::system::error_code e;
            size_t sz = socket.read_some(boost::asio::buffer(buf), e);

            data.insert(data.size(), buf.data(), sz);

            if (e == boost::asio::error::eof)
                break; // socket closed
            else if (e)
                throw boost::system::system_error(e);
        }

        std::stringstream ss;
        ss << data;

        // Get response code
        ss.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
        int code;
        ss >> code;

        // Check response code
        if (code != 200)
            return false;

        // Skip the rest of the header
        for (size_t i = 0; i < 5; i++)
            ss.ignore(std::numeric_limits<std::streamsize>::max(), '\r');

        // Parse json response;
        std::string JSON;
        ss >> JSON;
        std::stringstream jss;
        jss << JSON;
        boost::property_tree::json_parser::read_json(jss, ptree);
    } catch (std::exception &exception) {
        LogPrintf("ERROR Sidechain client (sendRequestToMainchain): %s\n", exception.what());
        return false;
    }
    return true;
}
