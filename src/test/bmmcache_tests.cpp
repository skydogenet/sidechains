// Copyright (c) 2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <bmmcache.h>
#include <deque>
#include <random.h>
#include <uint256.h>
#include <validation.h>

#include <test/test_bitcoin.h>

#include <boost/test/unit_test.hpp>

std::deque<uint256> GenerateRandomHashChain(int nCount)
{
    std::deque<uint256> dHash;
    for (int i = 0; i < nCount; i++) {
        dHash.push_back(GetRandHash());
    }
    return dHash;
}

BOOST_FIXTURE_TEST_SUITE(bmmcache_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(bmmcache_1_block)
{
    // Instance of BMMCache for test
    BMMCache cache;

    // Generate a 1 block chain, cache it with BMMCache and verify it.
    std::deque<uint256> dHashNew = GenerateRandomHashChain(1);
    // Create a copy because UpdateMainBlockCache can modify contents
    std::deque<uint256> dHashNewCopy = dHashNew;

    bool fReorg = false;
    std::vector<uint256> vOrphan;
    BOOST_CHECK(cache.UpdateMainBlockCache(dHashNewCopy, fReorg, vOrphan));
    BOOST_CHECK((unsigned int)cache.GetCachedBlockCount() == dHashNew.size());
    BOOST_CHECK(!fReorg);
    BOOST_CHECK(vOrphan.empty());

    std::vector<uint256> vHashCached = cache.GetMainBlockHashCache();
    BOOST_CHECK(vHashCached.size() == dHashNew.size());

    if (vHashCached.size() != dHashNew.size())
        return;

    bool fMatch = true;
    for (size_t i = 0; i < vHashCached.size(); i++) {
        if (vHashCached[i] != dHashNew[i]) {
            fMatch = false;
            break;
        }
    }
    BOOST_CHECK(fMatch);
}

BOOST_AUTO_TEST_CASE(bmmcache_2_block)
{
    // Instance of BMMCache for test
    BMMCache cache;

    // Generate a 2 block chain, cache it with BMMCache and verify it.
    std::deque<uint256> dHashNew = GenerateRandomHashChain(2);
    // Create a copy because UpdateMainBlockCache can modify contents
    std::deque<uint256> dHashNewCopy = dHashNew;

    bool fReorg = false;
    std::vector<uint256> vOrphan;
    BOOST_CHECK(cache.UpdateMainBlockCache(dHashNewCopy, fReorg, vOrphan));
    BOOST_CHECK((unsigned int)cache.GetCachedBlockCount() == dHashNew.size());
    BOOST_CHECK(!fReorg);
    BOOST_CHECK(vOrphan.empty());

    std::vector<uint256> vHashCached = cache.GetMainBlockHashCache();
    BOOST_CHECK(vHashCached.size() == dHashNew.size());

    if (vHashCached.size() != dHashNew.size())
        return;

    bool fMatch = true;
    for (size_t i = 0; i < vHashCached.size(); i++) {
        if (vHashCached[i] != dHashNew[i]) {
            fMatch = false;
            break;
        }
    }
    BOOST_CHECK(fMatch);
}

BOOST_AUTO_TEST_CASE(bmmcache_3_block)
{
    // Instance of BMMCache for test
    BMMCache cache;

    // Generate a 3 block chain, cache it with BMMCache and verify it.
    std::deque<uint256> dHashNew = GenerateRandomHashChain(3);
    // Create a copy because UpdateMainBlockCache can modify contents
    std::deque<uint256> dHashNewCopy = dHashNew;

    bool fReorg = false;
    std::vector<uint256> vOrphan;
    BOOST_CHECK(cache.UpdateMainBlockCache(dHashNewCopy, fReorg, vOrphan));
    BOOST_CHECK((unsigned int)cache.GetCachedBlockCount() == dHashNew.size());
    BOOST_CHECK(!fReorg);
    BOOST_CHECK(vOrphan.empty());

    std::vector<uint256> vHashCached = cache.GetMainBlockHashCache();
    BOOST_CHECK(vHashCached.size() == dHashNew.size());

    if (vHashCached.size() != dHashNew.size())
        return;

    bool fMatch = true;
    for (size_t i = 0; i < vHashCached.size(); i++) {
        if (vHashCached[i] != dHashNew[i]) {
            fMatch = false;
            break;
        }
    }
    BOOST_CHECK(fMatch);
}

BOOST_AUTO_TEST_CASE(bmmcache_100_block)
{
    // Instance of BMMCache for test
    BMMCache cache;

    // Generate a 100 block chain, cache it with BMMCache and verify it.
    std::deque<uint256> dHashNew = GenerateRandomHashChain(100);
    // Create a copy because UpdateMainBlockCache can modify contents
    std::deque<uint256> dHashNewCopy = dHashNew;

    bool fReorg = false;
    std::vector<uint256> vOrphan;
    BOOST_CHECK(cache.UpdateMainBlockCache(dHashNewCopy, fReorg, vOrphan));
    BOOST_CHECK((unsigned int)cache.GetCachedBlockCount() == dHashNew.size());
    BOOST_CHECK(!fReorg);
    BOOST_CHECK(vOrphan.empty());

    std::vector<uint256> vHashCached = cache.GetMainBlockHashCache();
    BOOST_CHECK(vHashCached.size() == dHashNew.size());

    if (vHashCached.size() != dHashNew.size())
        return;

    bool fMatch = true;
    for (size_t i = 0; i < vHashCached.size(); i++) {
        if (vHashCached[i] != dHashNew[i]) {
            fMatch = false;
            break;
        }
    }
    BOOST_CHECK(fMatch);
}

BOOST_AUTO_TEST_CASE(bmmcache_1000_block)
{
    // Instance of BMMCache for test
    BMMCache cache;

    // Generate a 1000 block chain, cache it with BMMCache and verify it.
    std::deque<uint256> dHashNew = GenerateRandomHashChain(1000);
    // Create a copy because UpdateMainBlockCache can modify contents
    std::deque<uint256> dHashNewCopy = dHashNew;

    bool fReorg = false;
    std::vector<uint256> vOrphan;
    BOOST_CHECK(cache.UpdateMainBlockCache(dHashNewCopy, fReorg, vOrphan));
    BOOST_CHECK((unsigned int)cache.GetCachedBlockCount() == dHashNew.size());
    BOOST_CHECK(!fReorg);
    BOOST_CHECK(vOrphan.empty());

    std::vector<uint256> vHashCached = cache.GetMainBlockHashCache();
    BOOST_CHECK(vHashCached.size() == dHashNew.size());

    if (vHashCached.size() != dHashNew.size())
        return;

    bool fMatch = true;
    for (size_t i = 0; i < vHashCached.size(); i++) {
        if (vHashCached[i] != dHashNew[i]) {
            fMatch = false;
            break;
        }
    }
    BOOST_CHECK(fMatch);
}

BOOST_AUTO_TEST_CASE(bmmcache_25000_block)
{
    // Instance of BMMCache for test
    BMMCache cache;

    // Generate a 25000 block chain, cache it with BMMCache and verify it.
    std::deque<uint256> dHashNew = GenerateRandomHashChain(25000);
    // Create a copy because UpdateMainBlockCache can modify contents
    std::deque<uint256> dHashNewCopy = dHashNew;

    bool fReorg = false;
    std::vector<uint256> vOrphan;
    BOOST_CHECK(cache.UpdateMainBlockCache(dHashNewCopy, fReorg, vOrphan));
    BOOST_CHECK((unsigned int)cache.GetCachedBlockCount() == dHashNew.size());
    BOOST_CHECK(!fReorg);
    BOOST_CHECK(vOrphan.empty());

    std::vector<uint256> vHashCached = cache.GetMainBlockHashCache();
    BOOST_CHECK(vHashCached.size() == dHashNew.size());

    if (vHashCached.size() != dHashNew.size())
        return;

    bool fMatch = true;
    for (size_t i = 0; i < vHashCached.size(); i++) {
        if (vHashCached[i] != dHashNew[i]) {
            fMatch = false;
            break;
        }
    }
    BOOST_CHECK(fMatch);
}

BOOST_AUTO_TEST_CASE(bmmcache_reorg_1)
{
    // Test a 1 block deep reorg

    // Instance of BMMCache for test
    BMMCache cache;

    // Generate a 1000 block chain, cache it with BMMCache and verify it.
    std::deque<uint256> dHashNew = GenerateRandomHashChain(1000);
    // Create a copy because UpdateMainBlockCache can modify contents
    std::deque<uint256> dHashNewCopy = dHashNew;

    bool fReorg = false;
    std::vector<uint256> vOrphan;
    BOOST_CHECK(cache.UpdateMainBlockCache(dHashNewCopy, fReorg, vOrphan));
    BOOST_CHECK((unsigned int)cache.GetCachedBlockCount() == dHashNew.size());
    BOOST_CHECK(!fReorg);
    BOOST_CHECK(vOrphan.empty());

    std::vector<uint256> vHashCached = cache.GetMainBlockHashCache();
    BOOST_CHECK(vHashCached.size() == dHashNew.size());

    if (vHashCached.size() != dHashNew.size())
        return;

    bool fMatch = true;
    for (size_t i = 0; i < vHashCached.size(); i++) {
        if (vHashCached[i] != dHashNew[i]) {
            fMatch = false;
            break;
        }
    }
    BOOST_CHECK(fMatch);

    // Now create a 1 block deep reorg and check handling
    std::deque<uint256> dHashReorg = dHashNew;

    // Remove blocks to reorg depth and keep track of them
    std::vector<uint256> vOrphanCheck;
    size_t nReorgDepth = 1;
    for (size_t i = 0; i <= nReorgDepth; i++) {
        vOrphanCheck.push_back(dHashReorg.back());
        dHashReorg.pop_back();
    }
    // Replace blocks
    for (size_t i = 0; i <= nReorgDepth; i++) {
        dHashReorg.push_back(GetRandHash());
    }

    // Test reorg handling

    // Copy what validation does to find the newest block in the update that
    // is already in the cache
    std::deque<uint256> dHashReorgNewBlocks;
    for (auto rit = dHashReorg.rbegin(); rit != dHashReorg.rend(); rit++) {
        if (cache.HaveMainBlock(*rit)) {
            dHashReorgNewBlocks.push_front(*rit);
            break;
        }
        dHashReorgNewBlocks.push_front(*rit);
    }

    fReorg = false;
    vOrphan.clear();
    BOOST_CHECK(cache.UpdateMainBlockCache(dHashReorgNewBlocks, fReorg, vOrphan));
    BOOST_CHECK((unsigned int)cache.GetCachedBlockCount() == dHashReorg.size());
    BOOST_CHECK(fReorg);
    BOOST_CHECK(!vOrphan.empty());

    vHashCached.clear();
    vHashCached = cache.GetMainBlockHashCache();
    BOOST_CHECK(vHashCached.size() == dHashReorg.size());

    if (vHashCached.size() != dHashReorg.size())
        return;

    fMatch = true;
    for (size_t i = 0; i < vHashCached.size(); i++) {
        if (vHashCached[i] != dHashReorg[i]) {
            fMatch = false;
            break;
        }
    }
    BOOST_CHECK(fMatch);

    // Verify that the correct blocks were orphaned from the cache
    BOOST_CHECK(vOrphan == vOrphanCheck);
}


BOOST_AUTO_TEST_CASE(bmmcache_reorg_2)
{
    // Test a 2 block deep reorg

    // Instance of BMMCache for test
    BMMCache cache;

    // Generate a 1000 block chain, cache it with BMMCache and verify it.
    std::deque<uint256> dHashNew = GenerateRandomHashChain(1000);
    // Create a copy because UpdateMainBlockCache can modify contents
    std::deque<uint256> dHashNewCopy = dHashNew;

    bool fReorg = false;
    std::vector<uint256> vOrphan;
    BOOST_CHECK(cache.UpdateMainBlockCache(dHashNewCopy, fReorg, vOrphan));
    BOOST_CHECK((unsigned int)cache.GetCachedBlockCount() == dHashNew.size());
    BOOST_CHECK(!fReorg);
    BOOST_CHECK(vOrphan.empty());

    std::vector<uint256> vHashCached = cache.GetMainBlockHashCache();
    BOOST_CHECK(vHashCached.size() == dHashNew.size());

    if (vHashCached.size() != dHashNew.size())
        return;

    bool fMatch = true;
    for (size_t i = 0; i < vHashCached.size(); i++) {
        if (vHashCached[i] != dHashNew[i]) {
            fMatch = false;
            break;
        }
    }
    BOOST_CHECK(fMatch);

    // Now create a 2 block deep reorg and check handling
    std::deque<uint256> dHashReorg = dHashNew;

    // Remove blocks to reorg depth and keep track of them
    std::vector<uint256> vOrphanCheck;
    size_t nReorgDepth = 2;
    for (size_t i = 0; i <= nReorgDepth; i++) {
        vOrphanCheck.push_back(dHashReorg.back());
        dHashReorg.pop_back();
    }
    // Replace blocks
    for (size_t i = 0; i <= nReorgDepth; i++) {
        dHashReorg.push_back(GetRandHash());
    }

    // Test reorg handling

    // Copy what validation does to find the newest block in the update that
    // is already in the cache
    std::deque<uint256> dHashReorgNewBlocks;
    for (auto rit = dHashReorg.rbegin(); rit != dHashReorg.rend(); rit++) {
        if (cache.HaveMainBlock(*rit)) {
            dHashReorgNewBlocks.push_front(*rit);
            break;
        }
        dHashReorgNewBlocks.push_front(*rit);
    }

    fReorg = false;
    vOrphan.clear();
    BOOST_CHECK(cache.UpdateMainBlockCache(dHashReorgNewBlocks, fReorg, vOrphan));
    BOOST_CHECK((unsigned int)cache.GetCachedBlockCount() == dHashReorg.size());
    BOOST_CHECK(fReorg);
    BOOST_CHECK(!vOrphan.empty());

    vHashCached.clear();
    vHashCached = cache.GetMainBlockHashCache();
    BOOST_CHECK(vHashCached.size() == dHashReorg.size());

    if (vHashCached.size() != dHashReorg.size())
        return;

    fMatch = true;
    for (size_t i = 0; i < vHashCached.size(); i++) {
        if (vHashCached[i] != dHashReorg[i]) {
            fMatch = false;
            break;
        }
    }
    BOOST_CHECK(fMatch);

    // Verify that the correct blocks were orphaned from the cache
    BOOST_CHECK(vOrphan == vOrphanCheck);
}

BOOST_AUTO_TEST_CASE(bmmcache_reorg_3)
{
    // Test a 3 block deep reorg

    // Instance of BMMCache for test
    BMMCache cache;

    // Generate a 1000 block chain, cache it with BMMCache and verify it.
    std::deque<uint256> dHashNew = GenerateRandomHashChain(1000);
    // Create a copy because UpdateMainBlockCache can modify contents
    std::deque<uint256> dHashNewCopy = dHashNew;

    bool fReorg = false;
    std::vector<uint256> vOrphan;
    BOOST_CHECK(cache.UpdateMainBlockCache(dHashNewCopy, fReorg, vOrphan));
    BOOST_CHECK((unsigned int)cache.GetCachedBlockCount() == dHashNew.size());
    BOOST_CHECK(!fReorg);
    BOOST_CHECK(vOrphan.empty());

    std::vector<uint256> vHashCached = cache.GetMainBlockHashCache();
    BOOST_CHECK(vHashCached.size() == dHashNew.size());

    if (vHashCached.size() != dHashNew.size())
        return;

    bool fMatch = true;
    for (size_t i = 0; i < vHashCached.size(); i++) {
        if (vHashCached[i] != dHashNew[i]) {
            fMatch = false;
            break;
        }
    }
    BOOST_CHECK(fMatch);

    // Now create a 3 block deep reorg and check handling
    std::deque<uint256> dHashReorg = dHashNew;

    // Remove blocks to reorg depth and keep track of them
    std::vector<uint256> vOrphanCheck;
    size_t nReorgDepth = 3;
    for (size_t i = 0; i <= nReorgDepth; i++) {
        vOrphanCheck.push_back(dHashReorg.back());
        dHashReorg.pop_back();
    }
    // Replace blocks
    for (size_t i = 0; i <= nReorgDepth; i++) {
        dHashReorg.push_back(GetRandHash());
    }

    // Test reorg handling

    // Copy what validation does to find the newest block in the update that
    // is already in the cache
    std::deque<uint256> dHashReorgNewBlocks;
    for (auto rit = dHashReorg.rbegin(); rit != dHashReorg.rend(); rit++) {
        if (cache.HaveMainBlock(*rit)) {
            dHashReorgNewBlocks.push_front(*rit);
            break;
        }
        dHashReorgNewBlocks.push_front(*rit);
    }

    fReorg = false;
    vOrphan.clear();
    BOOST_CHECK(cache.UpdateMainBlockCache(dHashReorgNewBlocks, fReorg, vOrphan));
    BOOST_CHECK((unsigned int)cache.GetCachedBlockCount() == dHashReorg.size());
    BOOST_CHECK(fReorg);
    BOOST_CHECK(!vOrphan.empty());

    vHashCached.clear();
    vHashCached = cache.GetMainBlockHashCache();
    BOOST_CHECK(vHashCached.size() == dHashReorg.size());

    if (vHashCached.size() != dHashReorg.size())
        return;

    fMatch = true;
    for (size_t i = 0; i < vHashCached.size(); i++) {
        if (vHashCached[i] != dHashReorg[i]) {
            fMatch = false;
            break;
        }
    }
    BOOST_CHECK(fMatch);

    // Verify that the correct blocks were orphaned from the cache
    BOOST_CHECK(vOrphan == vOrphanCheck);
}

BOOST_AUTO_TEST_CASE(bmmcache_reorg_100)
{
    // Test a 100 block deep reorg

    // Instance of BMMCache for test
    BMMCache cache;

    // Generate a 1000 block chain, cache it with BMMCache and verify it.
    std::deque<uint256> dHashNew = GenerateRandomHashChain(1000);
    // Create a copy because UpdateMainBlockCache can modify contents
    std::deque<uint256> dHashNewCopy = dHashNew;

    bool fReorg = false;
    std::vector<uint256> vOrphan;
    BOOST_CHECK(cache.UpdateMainBlockCache(dHashNewCopy, fReorg, vOrphan));
    BOOST_CHECK((unsigned int)cache.GetCachedBlockCount() == dHashNew.size());
    BOOST_CHECK(!fReorg);
    BOOST_CHECK(vOrphan.empty());

    std::vector<uint256> vHashCached = cache.GetMainBlockHashCache();
    BOOST_CHECK(vHashCached.size() == dHashNew.size());

    if (vHashCached.size() != dHashNew.size())
        return;

    bool fMatch = true;
    for (size_t i = 0; i < vHashCached.size(); i++) {
        if (vHashCached[i] != dHashNew[i]) {
            fMatch = false;
            break;
        }
    }
    BOOST_CHECK(fMatch);

    // Now create a 100 block deep reorg and check handling
    std::deque<uint256> dHashReorg = dHashNew;

    // Remove blocks to reorg depth and keep track of them
    std::vector<uint256> vOrphanCheck;
    size_t nReorgDepth = 100;
    for (size_t i = 0; i <= nReorgDepth; i++) {
        vOrphanCheck.push_back(dHashReorg.back());
        dHashReorg.pop_back();
    }
    // Replace blocks
    for (size_t i = 0; i <= nReorgDepth; i++) {
        dHashReorg.push_back(GetRandHash());
    }

    // Test reorg handling

    // Copy what validation does to find the newest block in the update that
    // is already in the cache
    std::deque<uint256> dHashReorgNewBlocks;
    for (auto rit = dHashReorg.rbegin(); rit != dHashReorg.rend(); rit++) {
        if (cache.HaveMainBlock(*rit)) {
            dHashReorgNewBlocks.push_front(*rit);
            break;
        }
        dHashReorgNewBlocks.push_front(*rit);
    }

    fReorg = false;
    vOrphan.clear();
    BOOST_CHECK(cache.UpdateMainBlockCache(dHashReorgNewBlocks, fReorg, vOrphan));
    BOOST_CHECK((unsigned int)cache.GetCachedBlockCount() == dHashReorg.size());
    BOOST_CHECK(fReorg);
    BOOST_CHECK(!vOrphan.empty());

    vHashCached.clear();
    vHashCached = cache.GetMainBlockHashCache();
    BOOST_CHECK(vHashCached.size() == dHashReorg.size());

    if (vHashCached.size() != dHashReorg.size())
        return;

    fMatch = true;
    for (size_t i = 0; i < vHashCached.size(); i++) {
        if (vHashCached[i] != dHashReorg[i]) {
            fMatch = false;
            break;
        }
    }
    BOOST_CHECK(fMatch);

    // Verify that the correct blocks were orphaned from the cache
    BOOST_CHECK(vOrphan == vOrphanCheck);
}

BOOST_AUTO_TEST_CASE(bmmcache_reorg_1000)
{
    // Test a 1000 block deep reorg

    // Instance of BMMCache for test
    BMMCache cache;

    // Generate a 1001 block chain, cache it with BMMCache and verify it.
    std::deque<uint256> dHashNew = GenerateRandomHashChain(1010);
    // Create a copy because UpdateMainBlockCache can modify contents
    std::deque<uint256> dHashNewCopy = dHashNew;

    bool fReorg = false;
    std::vector<uint256> vOrphan;
    BOOST_CHECK(cache.UpdateMainBlockCache(dHashNewCopy, fReorg, vOrphan));
    BOOST_CHECK((unsigned int)cache.GetCachedBlockCount() == dHashNew.size());
    BOOST_CHECK(!fReorg);
    BOOST_CHECK(vOrphan.empty());

    std::vector<uint256> vHashCached = cache.GetMainBlockHashCache();
    BOOST_CHECK(vHashCached.size() == dHashNew.size());

    if (vHashCached.size() != dHashNew.size())
        return;

    bool fMatch = true;
    for (size_t i = 0; i < vHashCached.size(); i++) {
        if (vHashCached[i] != dHashNew[i]) {
            fMatch = false;
            break;
        }
    }
    BOOST_CHECK(fMatch);

    // Now create a 1000 block deep reorg and check handling
    std::deque<uint256> dHashReorg = dHashNew;

    // Remove blocks to reorg depth and keep track of them
    std::vector<uint256> vOrphanCheck;
    size_t nReorgDepth = 1000;
    for (size_t i = 0; i <= nReorgDepth; i++) {
        vOrphanCheck.push_back(dHashReorg.back());
        dHashReorg.pop_back();
    }
    // Replace blocks
    for (size_t i = 0; i <= nReorgDepth; i++) {
        dHashReorg.push_back(GetRandHash());
    }

    // Test reorg handling

    // Copy what validation does to find the newest block in the update that
    // is already in the cache
    std::deque<uint256> dHashReorgNewBlocks;
    for (auto rit = dHashReorg.rbegin(); rit != dHashReorg.rend(); rit++) {
        if (cache.HaveMainBlock(*rit)) {
            dHashReorgNewBlocks.push_front(*rit);
            break;
        }
        dHashReorgNewBlocks.push_front(*rit);
    }

    fReorg = false;
    vOrphan.clear();
    BOOST_CHECK(cache.UpdateMainBlockCache(dHashReorgNewBlocks, fReorg, vOrphan));
    BOOST_CHECK((unsigned int)cache.GetCachedBlockCount() == dHashReorg.size());
    BOOST_CHECK(fReorg);
    BOOST_CHECK(!vOrphan.empty());

    vHashCached.clear();
    vHashCached = cache.GetMainBlockHashCache();
    BOOST_CHECK(vHashCached.size() == dHashReorg.size());

    if (vHashCached.size() != dHashReorg.size())
        return;

    fMatch = true;
    for (size_t i = 0; i < vHashCached.size(); i++) {
        if (vHashCached[i] != dHashReorg[i]) {
            fMatch = false;
            break;
        }
    }
    BOOST_CHECK(fMatch);

    // Verify that the correct blocks were orphaned from the cache
    BOOST_CHECK(vOrphan == vOrphanCheck);
}

BOOST_AUTO_TEST_CASE(bmmcache_repeat_genesis)
{
    // Test sending the genesis block multiple times without any other blocks

    // Instance of BMMCache for test
    BMMCache cache;

    // Generate a 1 block chain, cache it with BMMCache and verify it.
    std::deque<uint256> dHashNew = GenerateRandomHashChain(1);
    // Create a copy because UpdateMainBlockCache can modify contents
    std::deque<uint256> dHashNewCopy = dHashNew;

    bool fReorg = false;
    std::vector<uint256> vOrphan;
    BOOST_CHECK(cache.UpdateMainBlockCache(dHashNewCopy, fReorg, vOrphan));
    BOOST_CHECK((unsigned int)cache.GetCachedBlockCount() == dHashNew.size());
    BOOST_CHECK(!fReorg);
    BOOST_CHECK(vOrphan.empty());

    std::vector<uint256> vHashCached = cache.GetMainBlockHashCache();
    BOOST_CHECK(vHashCached.size() == dHashNew.size());

    if (vHashCached.size() != dHashNew.size())
        return;

    bool fMatch = true;
    for (size_t i = 0; i < vHashCached.size(); i++) {
        if (vHashCached[i] != dHashNew[i]) {
            fMatch = false;
            break;
        }
    }
    BOOST_CHECK(fMatch);

    // Create a another copy with only the genesis block that was already added
    std::deque<uint256> dHashRepeat = dHashNew;

    fReorg = false;
    vOrphan.clear();
    BOOST_CHECK(cache.UpdateMainBlockCache(dHashRepeat, fReorg, vOrphan));
    BOOST_CHECK((unsigned int)cache.GetCachedBlockCount() == dHashNew.size());
    BOOST_CHECK(!fReorg);
    BOOST_CHECK(vOrphan.empty());

    vHashCached.clear();
    vHashCached = cache.GetMainBlockHashCache();
    BOOST_CHECK(vHashCached.size() == dHashNew.size());

    if (vHashCached.size() != dHashNew.size())
        return;

    fMatch = true;
    for (size_t i = 0; i < vHashCached.size(); i++) {
        if (vHashCached[i] != dHashNew[i]) {
            fMatch = false;
            break;
        }
    }
    BOOST_CHECK(fMatch);
}

BOOST_AUTO_TEST_CASE(bmmcache_repeat_tip)
{
    // Test sending the tip multiple times without any other blocks

    // Instance of BMMCache for test
    BMMCache cache;

    // Generate a 10 block chain, cache it with BMMCache and verify it.
    std::deque<uint256> dHashNew = GenerateRandomHashChain(10);
    // Create a copy because UpdateMainBlockCache can modify contents
    std::deque<uint256> dHashNewCopy = dHashNew;

    bool fReorg = false;
    std::vector<uint256> vOrphan;
    BOOST_CHECK(cache.UpdateMainBlockCache(dHashNewCopy, fReorg, vOrphan));
    BOOST_CHECK((unsigned int)cache.GetCachedBlockCount() == dHashNew.size());
    BOOST_CHECK(!fReorg);
    BOOST_CHECK(vOrphan.empty());

    std::vector<uint256> vHashCached = cache.GetMainBlockHashCache();
    BOOST_CHECK(vHashCached.size() == dHashNew.size());

    if (vHashCached.size() != dHashNew.size())
        return;

    bool fMatch = true;
    for (size_t i = 0; i < vHashCached.size(); i++) {
        if (vHashCached[i] != dHashNew[i]) {
            fMatch = false;
            break;
        }
    }
    BOOST_CHECK(fMatch);

    // Create a another copy with only the tip block that was already added
    std::deque<uint256> dHashRepeat;
    dHashRepeat.push_back(dHashNew.back());

    fReorg = false;
    vOrphan.clear();
    BOOST_CHECK(cache.UpdateMainBlockCache(dHashRepeat, fReorg, vOrphan));
    BOOST_CHECK((unsigned int)cache.GetCachedBlockCount() == dHashNew.size());
    BOOST_CHECK(!fReorg);
    BOOST_CHECK(vOrphan.empty());

    vHashCached.clear();
    vHashCached = cache.GetMainBlockHashCache();
    BOOST_CHECK(vHashCached.size() == dHashNew.size());

    if (vHashCached.size() != dHashNew.size())
        return;

    fMatch = true;
    for (size_t i = 0; i < vHashCached.size(); i++) {
        if (vHashCached[i] != dHashNew[i]) {
            fMatch = false;
            break;
        }
    }
    BOOST_CHECK(fMatch);
}

BOOST_AUTO_TEST_CASE(bmmcache_replace_tip)
{
    // Test a reorg that only replaces the tip

    // Instance of BMMCache for test
    BMMCache cache;

    // Generate a 1000 block chain, cache it with BMMCache and verify it.
    std::deque<uint256> dHashNew = GenerateRandomHashChain(1000);
    // Create a copy because UpdateMainBlockCache can modify contents
    std::deque<uint256> dHashNewCopy = dHashNew;

    bool fReorg = false;
    std::vector<uint256> vOrphan;
    BOOST_CHECK(cache.UpdateMainBlockCache(dHashNewCopy, fReorg, vOrphan));
    BOOST_CHECK((unsigned int)cache.GetCachedBlockCount() == dHashNew.size());
    BOOST_CHECK(!fReorg);
    BOOST_CHECK(vOrphan.empty());

    std::vector<uint256> vHashCached = cache.GetMainBlockHashCache();
    BOOST_CHECK(vHashCached.size() == dHashNew.size());

    if (vHashCached.size() != dHashNew.size())
        return;

    bool fMatch = true;
    for (size_t i = 0; i < vHashCached.size(); i++) {
        if (vHashCached[i] != dHashNew[i]) {
            fMatch = false;
            break;
        }
    }
    BOOST_CHECK(fMatch);

    std::deque<uint256> dHashReorg = dHashNew;

    // Replace the tip
    std::vector<uint256> vOrphanCheck;
    vOrphanCheck.push_back(dHashReorg.back());
    dHashReorg.pop_back();
    dHashReorg.push_back(GetRandHash());

    // Test reorg handling

    // Copy what validation does to find the newest block in the update that
    // is already in the cache
    std::deque<uint256> dHashReorgNewBlocks;
    for (auto rit = dHashReorg.rbegin(); rit != dHashReorg.rend(); rit++) {
        if (cache.HaveMainBlock(*rit)) {
            dHashReorgNewBlocks.push_front(*rit);
            break;
        }
        dHashReorgNewBlocks.push_front(*rit);
    }

    fReorg = false;
    vOrphan.clear();
    BOOST_CHECK(cache.UpdateMainBlockCache(dHashReorgNewBlocks, fReorg, vOrphan));
    BOOST_CHECK((unsigned int)cache.GetCachedBlockCount() == dHashReorg.size());
    BOOST_CHECK(fReorg);
    BOOST_CHECK(!vOrphan.empty());

    vHashCached.clear();
    vHashCached = cache.GetMainBlockHashCache();
    BOOST_CHECK(vHashCached.size() == dHashReorg.size());

    if (vHashCached.size() != dHashReorg.size())
        return;

    fMatch = true;
    for (size_t i = 0; i < vHashCached.size(); i++) {
        if (vHashCached[i] != dHashReorg[i]) {
            fMatch = false;
            break;
        }
    }
    BOOST_CHECK(fMatch);

    // Verify that the correct blocks were orphaned from the cache
    BOOST_CHECK(vOrphan == vOrphanCheck);
}

BOOST_AUTO_TEST_SUITE_END()
