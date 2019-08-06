// Copyright (c) 2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SIDECHAINCLIENT_H
#define SIDECHAINCLIENT_H

#include <amount.h>
#include <uint256.h>
#include <validation.h>

#include <string>
#include <vector>

#include <boost/property_tree/json_parser.hpp>

class SidechainDeposit;
class SidechainBMMProof;

class SidechainClient
{
public:
    SidechainClient();

    /*
     * Send B-WT^ hash to local node
     */
    bool BroadcastWTPrime(const std::string& hex);

    /*
     * Ask for an updated list of recent deposits
     */
    std::vector<SidechainDeposit> UpdateDeposits(const std::string& strBuildHash);

    /*
     * Note: true return value indicates the request to verify the
     * txout proof went through. However, txid (returned by reference)
     * must also be verified to match the txCritical hash.
     */
    bool VerifyCriticalHashProof(const std::string& criticalProof, uint256& txid);

    /*
     * Request BMM proof for a block
     */
    bool RequestBMMProof(const uint256& hashMainBlock, const uint256& hashBMMBlock, SidechainBMMProof& proof);

    /*
     * Send BMM critical data request
     */
    uint256 SendBMMCriticalDataRequest(const uint256& hashCritical, const uint256& hashBlockMain, int nHeight = 0, const CAmount& amount = CAmount(0));

    /*
     * Request main:block hashes
     */
    std::vector<uint256> RequestMainBlockHashes();

    /*
     * Request the CTIP - Critical Transaction Index Pair for this sidechain
     */
    bool GetCTIP(std::pair<uint256, uint32_t>& ctip);

    /*
     * Automatically check our BMM requests on the mainchain and create new BMM
     * requests if needed.
     */
    void RefreshBMM();

    bool CreateBMMBlock(CBlock& block, std::string& strError);

    bool SubmitBMMBlock(const CBlock& block);

private:
    /*
     * Send json request to local node
     */
    bool SendRequestToMainchain(const std::string& json, boost::property_tree::ptree &ptree);

    std::vector<uint256> vMainBlockHash;

    uint256 hashMainBlockLastSeen;
};

#endif // SIDECHAINCLIENT_H
