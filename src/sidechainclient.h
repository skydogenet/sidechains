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

// TODO refactor: Move BMM validation cache code here, or remove class status.
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
    std::vector<SidechainDeposit> UpdateDeposits(const std::string& strAddressBytes, const uint256& hashLastDeposit, const uint32_t n);

    /*
     * Note: true return value indicates the request to verify the
     * txout proof went through. However, txid (returned by reference)
     * must also be verified to match the txCritical hash.
     */
    bool VerifyCriticalHashProof(const std::string& criticalProof, uint256& txid);

    /*
     * Search for BMM in a mainchain block
     */
    bool GetBMM(const uint256& hashMainBlock, const uint256& hashBMM, uint256& txid, uint32_t& nTime);

    /*
     * Send BMM critical data request
     */
    uint256 SendBMMRequest(const uint256& hashBMM, const uint256& hashBlockMain, int nHeight = 0, CAmount amount = CAmount(0));

    /*
     * Request the CTIP - Critical Transaction Index Pair for this sidechain
     */
    bool GetCTIP(std::pair<uint256, uint32_t>& ctip);

    /*
     * Automatically check our BMM requests on the mainchain and create new BMM
     * requests if needed.
     */
    bool RefreshBMM(const CAmount& amount, std::string& strError, uint256& hashCreatedMerkleRoot, uint256& hashConnected, uint256& hashConnectedMerkleRoot, uint256& txid, int& nTxn, CAmount& nFees, bool fCreateNew = true, const uint256& hashPrevBlock = uint256());

    bool CreateBMMBlock(CBlock& block, std::string& strError, CAmount& nFees, const uint256& hashPrevBlock = uint256());

    bool SubmitBMMBlock(const CBlock& block);

    bool GetAverageFees(int nBlocks, int nStartHeight, CAmount& nAverageFees);

    bool GetBlockCount(int& nBlocks);

    bool GetWorkScore(const uint256& hashWTPrime, int& nWorkScore);

    bool ListWTPrimeStatus(std::vector<uint256>& vHashWTPrime);

    bool GetBlockHash(int nHeight, uint256& hashBlock);

    bool HaveSpentWTPrime(const uint256& hashWTPrime);

    bool HaveFailedWTPrime(const uint256& hashWTPrime);

private:
    /*
     * Send json request to local node
     */
    bool SendRequestToMainchain(const std::string& json, boost::property_tree::ptree &ptree);
};

#endif // SIDECHAINCLIENT_H
