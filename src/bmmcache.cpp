#include <bmmcache.h>

#include <primitives/block.h>
#include <util.h>

BMMCache::BMMCache()
{

}

bool BMMCache::StoreBMMBlock(const CBlock& block)
{
    if (!block.vtx.size())
        return false;

    uint256 hashBlock = block.GetBlindHash();

    // Already have block stored
    if (mapBMMBlocks.find(hashBlock) != mapBMMBlocks.end())
        return false;

    mapBMMBlocks[hashBlock] = block;

    return true;
}

bool BMMCache::GetBMMBlock(const uint256& hashBlock, CBlock& block)
{
    if (mapBMMBlocks.find(hashBlock) == mapBMMBlocks.end())
        return false;

    block = mapBMMBlocks[hashBlock];

    return true;
}

std::vector<CBlock> BMMCache::GetBMMBlockCache() const
{
    std::vector<CBlock> vBlock;
    for (const auto& b : mapBMMBlocks) {
        vBlock.push_back(b.second);
    }
    return vBlock;
}

std::vector<uint256> BMMCache::GetBroadcastedWTPrimeCache() const
{
    std::vector<uint256> vHash;
    for (const auto& u : setWTPrimeBroadcasted) {
        vHash.push_back(u);
    }
    return vHash;
}

std::vector<uint256> BMMCache::GetMainBlockHashCache() const
{
    return vMainBlockHash;
}

std::vector<uint256> BMMCache::GetRecentMainBlockHashes() const
{
    // Return up to three of the most recent mainchain block hashes
    std::vector<uint256> vHash;
    std::vector<uint256>::const_reverse_iterator rit = vMainBlockHash.rbegin();
    for (; rit != vMainBlockHash.rend(); rit++) {
        vHash.push_back(*rit);
        if (vHash.size() == 3)
            break;
    }
    std::reverse(vHash.begin(), vHash.end());
    return vHash;
}

void BMMCache::ClearBMMBlocks()
{
    mapBMMBlocks.clear();
}

void BMMCache::StoreBroadcastedWTPrime(const uint256& hashWTPrime)
{
    setWTPrimeBroadcasted.insert(hashWTPrime);
}

void BMMCache::StorePrevBlockBMMCreated(const uint256& hashPrevBlock)
{
    setPrevBlockBMMCreated.insert(hashPrevBlock);
}

bool BMMCache::HaveBroadcastedWTPrime(const uint256& hashWTPrime) const
{
    if (hashWTPrime.IsNull())
        return false;

    if (setWTPrimeBroadcasted.count(hashWTPrime))
        return true;

    return false;
}

bool BMMCache::HaveBMMProof(const uint256& hashBlock, const uint256& hashCritical) const
{
    if (hashBlock.IsNull() || hashCritical.IsNull())
        return false;

    const auto it = mapBMMCache.find(hashBlock);
    if (it == mapBMMCache.end())
        return false;

    if (hashCritical == it->second)
        return true;

    return false;
}

void BMMCache::CacheBMMProof(const uint256& hashBlock, const uint256& hashCritical)
{
    if (hashBlock.IsNull() || hashCritical.IsNull())
        return;

    mapBMMCache.insert(std::make_pair(hashBlock, hashCritical));
}

void BMMCache::CacheMainBlockHash(const uint256& hash)
{
    // Don't re-cache the genesis block
    if (vMainBlockHash.size() == 1 && hash == vMainBlockHash.front())
        return;

    // Add to ordered vector of main block hashes
    vMainBlockHash.push_back(hash);

    // Add to index of hashes
    MainBlockIndex index;
    index.hash = hash;
    index.index = vMainBlockHash.size() - 1;

    mapMainBlock[hash] = index;
}

bool BMMCache::UpdateMainBlockCache(std::deque<uint256>& deqHashNew, bool& fReorg, std::vector<uint256>& vOrphan)
{
    if (deqHashNew.empty()) {
        LogPrintf("%s: Error - called with empty list of new block hashes!\n", __func__);
        return false;
    }

    // If the main block cache doesn't have the genesis block yet, add it first
    if (vMainBlockHash.empty())
        CacheMainBlockHash(deqHashNew.front());

    // Figure out the block in our cache that we will append the new blocks to
    MainBlockIndex index;
    if (mapMainBlock.count(deqHashNew.front())) {
        index = mapMainBlock[deqHashNew.front()];
    } else {
        LogPrintf("%s: Error - New blocks do not connect to cached chain!\n", __func__);
        return false;
    }

    // If there were any blocks in our cache after the block we will be building
    // on, remove them, add them to vOrphan as they were disconnected, set
    // fReorg true.
    if (index.index != vMainBlockHash.size() - 1) {
        LogPrintf("%s: Mainchain reorg detected!\n", __func__);
        fReorg = true;
    }

    for (size_t i = vMainBlockHash.size() - 1; i > index.index; i--) {
        vOrphan.push_back(vMainBlockHash[i]);
        vMainBlockHash.pop_back();
    }

    // Remove disconnected blocks from index map
    for (const uint256& u : vOrphan)
        mapMainBlock.erase(u);

    // It's possible that the first block in the list of new blocks (which
    // connects to our cached chain by a prevblock) was already cached.
    // The first block that connected by prevblock to one of our cached blocks
    // could either be a new different block or a block we already know
    // depending on how the fork / reorg happened.
    //
    // Check if we already know the first block in the deque and remove it if
    // we do.
    if (HaveMainBlock(deqHashNew.front()))
        deqHashNew.pop_front();

    // Append new blocks
    for (const uint256& u : deqHashNew)
        CacheMainBlockHash(u);

    LogPrintf("%s: Updated cached mainchain tip to: %s.\n", __func__, deqHashNew.back().ToString());

    return true;
}

uint256 BMMCache::GetLastMainBlockHash() const
{
    if (vMainBlockHash.empty())
        return uint256();

    return vMainBlockHash.back();
}

int BMMCache::GetCachedBlockCount() const
{
    return vMainBlockHash.size();
}

bool BMMCache::HaveMainBlock(const uint256& hash) const
{
    return mapMainBlock.count(hash);
}

bool BMMCache::HaveBMMRequestForPrevBlock(const uint256& hashPrevBlock) const
{
    return setPrevBlockBMMCreated.count(hashPrevBlock);
}

void BMMCache::AddCheckedMainBlock(const uint256& hashBlock)
{
    setMainBlockChecked.insert(hashBlock);
}

bool BMMCache::MainBlockChecked(const uint256& hashBlock) const
{
    return setMainBlockChecked.count(hashBlock);
}

void BMMCache::ResetMainBlockCache()
{
    vMainBlockHash.clear();
    mapMainBlock.clear();
}

void BMMCache::CacheWTID(const uint256& wtid)
{
    vWTIDCache.push_back(wtid);
}

std::vector<uint256> BMMCache::GetCachedWTID()
{
    return vWTIDCache;
}
