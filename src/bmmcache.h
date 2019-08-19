#ifndef BITCOIN_BMMCACHE_H
#define BITCOIN_BMMCACHE_H

#include "uint256.h"

#include <map>
#include <set>

class CBlock;

class BMMCache
{
public:
    BMMCache();

    bool StoreBMMBlock(const CBlock& block);

    bool GetBMMBlock(const uint256& hashBlock, CBlock& block);

    std::vector<CBlock> GetBMMBlockCache() const;

    std::vector<uint256> GetBMMWTPrimeCache() const;

    void ClearBMMBlocks();

    void StoreBroadcastedWTPrime(const uint256& hashWTPrime);

    bool HaveBroadcastedWTPrime(const uint256& hashWTPrime) const;

    bool HaveBMMProof(const uint256& hashBlock, const uint256& hashCritical) const;

    void CacheBMMProof(const uint256& hashBlock, const uint256& hashCritical);

    void UpdateMainBlocks(const std::vector<uint256>& vMainBlockHashIn);

    uint256 GetLastMainBlockHash() const;

private:
    // BMM blocks (without proof) that we have created with the intention of
    // adding to the side blockchain once proof is aquired from the main chain.
    std::map<uint256, CBlock> mapBMMBlocks;

    // Cache of BMM proof(s) which we have already verified with the main chain.
    std::map<uint256 /* hashBlock */, uint256 /* hashCriticalTx */> mapBMMCache;

    // WT^(s) that we have already broadcasted to the mainchain.
    std::set<uint256> setWTPrimeBroadcasted;

    // Cache of mainchain block hashes that we've scanned for BMM commitments
    std::vector<uint256> vMainBlockHash;

    // The last mainchain block that we've scanned
    uint256 hashMainBlockLastSeen;
};

#endif // BITCOIN_BMMCACHE_H
