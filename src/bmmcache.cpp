#include "bmmcache.h"

#include "primitives/block.h"

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

std::vector<uint256> BMMCache::GetBMMWTPrimeCache() const
{
    std::vector<uint256> vHash;
    for (const auto& u : setWTPrimeBroadcasted) {
        vHash.push_back(u);
    }
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

void BMMCache::UpdateMainBlocks(const std::vector<uint256>& vMainBlockHashIn)
{
    vMainBlockHash = vMainBlockHashIn;
    hashMainBlockLastSeen = vMainBlockHash.back();
}

uint256 BMMCache::GetLastMainBlockHash() const
{
    return hashMainBlockLastSeen;
}
