#ifndef BITCOIN_BMMBLOCKCACHE_H
#define BITCOIN_BMMBLOCKCACHE_H

#include "uint256.h"

#include <map>
#include <set>

class CBlock;

class BMMBlockCache
{
public:
    BMMBlockCache();

    bool StoreBMMBlock(const CBlock& block);

    bool GetBMMBlock(const uint256& hashBlock, CBlock& block);

    std::vector<CBlock> GetBMMBlockCache();

    void ClearBMMBlocks();

    void StoreBroadcastedWTPrime(const uint256& hashWTPrime); // TODO move

    bool HaveBroadcastedWTPrime(const uint256& hashWTPrime); // TODO move

private:
    std::map<uint256, CBlock> mapBMMBlocks;
    std::set<uint256> setWTPrimeBroadcasted; // TODO move
};

#endif // BITCOIN_BMMBLOCKCACHE_H
