#include "bmmblockcache.h"

#include "primitives/block.h"

BMMBlockCache::BMMBlockCache()
{

}

bool BMMBlockCache::StoreBMMBlock(const CBlock& block)
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

bool BMMBlockCache::GetBMMBlock(const uint256& hashBlock, CBlock& block)
{
    if (mapBMMBlocks.find(hashBlock) == mapBMMBlocks.end())
        return false;

    block = mapBMMBlocks[hashBlock];

    return true;
}

std::vector<CBlock> BMMBlockCache::GetBMMBlockCache()
{
    std::vector<CBlock> vBlock;
    for (auto& b : mapBMMBlocks) {
        vBlock.push_back(b.second);
    }
    return vBlock;
}

void BMMBlockCache::ClearBMMBlocks()
{
    mapBMMBlocks.clear();
}

void BMMBlockCache::StoreBroadcastedWTPrime(const uint256& hashWTPrime)
{
    setWTPrimeBroadcasted.insert(hashWTPrime);
}

bool BMMBlockCache::HaveBroadcastedWTPrime(const uint256& hashWTPrime)
{
    if (hashWTPrime.IsNull())
        return false;

    if (setWTPrimeBroadcasted.count(hashWTPrime))
        return true;

    return false;
}
