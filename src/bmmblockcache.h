#ifndef BITCOIN_BMMBLOCKCACHE_H
#define BITCOIN_BMMBLOCKCACHE_H

#include "uint256.h"

#include <map>

class CBlock;

class BMMBlockCache
{
public:
    BMMBlockCache();

    bool StoreBMMBlock(const CBlock& block);

    bool GetBMMBlock(const uint256& hashBlock, CBlock& block);

    std::vector<CBlock> GetBMMBlockCache();

    void Clear();

private:
    std::map<uint256, CBlock> mapBMMBlocks;
};

#endif // BITCOIN_BMMBLOCKCACHE_H
