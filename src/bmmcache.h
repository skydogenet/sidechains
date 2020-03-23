#ifndef BITCOIN_BMMCACHE_H
#define BITCOIN_BMMCACHE_H

#include "uint256.h"

#include <deque>
#include <map>
#include <set>
#include <vector>

class CBlock;

struct MainBlockIndex
{
    size_t index;
    uint256 hash;
};

class BMMCache
{
public:
    BMMCache();

    bool StoreBMMBlock(const CBlock& block);

    bool GetBMMBlock(const uint256& hashBlock, CBlock& block);

    std::vector<CBlock> GetBMMBlockCache() const;

    std::vector<uint256> GetBMMWTPrimeCache() const;

    std::vector<uint256> GetMainBlockHashCache() const;

    std::vector<uint256> GetRecentMainBlockHashes() const;

    void ClearBMMBlocks();

    void StoreBroadcastedWTPrime(const uint256& hashWTPrime);

    void StorePrevBlockBMMCreated(const uint256& hashPrevBlock);

    bool HaveBroadcastedWTPrime(const uint256& hashWTPrime) const;

    bool HaveBMMProof(const uint256& hashBlock, const uint256& hashCritical) const;

    void CacheBMMProof(const uint256& hashBlock, const uint256& hashCritical);

    void CacheMainBlockHash(const uint256& hash);

    void CacheMainBlockHash(const std::vector<uint256>& vHash);

    bool UpdateMainBlockCache(std::deque<uint256>& deqHashNew, bool& fReorg, std::vector<uint256>& vOrphan);

    uint256 GetLastMainBlockHash() const;

    void SetLatestWTPrime(const uint256& hashWTPrime);

    uint256 GetLatestWTPrime() const;

    int GetCachedBlockCount() const;

    bool HaveMainBlock(const uint256& hash) const;

    bool HaveBMMRequestForPrevBlock(const uint256& hashPrevBlock) const;

private:
    // BMM blocks (without proof) that we have created with the intention of
    // adding to the side blockchain once proof is aquired from the main chain.
    std::map<uint256, CBlock> mapBMMBlocks;

    // Cache of BMM proof(s) which we have already verified with the main chain.
    std::map<uint256 /* hashBlock */, uint256 /* hashCriticalTx */> mapBMMCache;

    // WT^(s) that we have already broadcasted to the mainchain.
    std::set<uint256> setWTPrimeBroadcasted;

    // Index of mainchain block hash in vMainBlockHash
    std::map<uint256 /* hashMainchainBlock */, MainBlockIndex> mapMainBlock;

    // List of all known mainchain block hashes in order
    std::vector<uint256> vMainBlockHash;

    // The last WT^ added to the sidechain tree
    uint256 hashLatestWTPrime;

    // TODO we could also cache a map of mainchain block hashes that we created
    // BMM requests for. That way, to check for BMM commitments we can just
    // check recent blocks that we created a commitment for instead of scanning
    // all of them.
    //
    // Set of hashes for which we've created a BMM request with this mainchain
    // prevblock. (Meaning the BMM request was created when the hash was the
    // mainchain tip)
    std::set<uint256> setPrevBlockBMMCreated;
};

#endif // BITCOIN_BMMCACHE_H
