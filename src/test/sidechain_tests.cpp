// Copyright (c) 2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "bmmcache.h"
#include "chainparams.h"
#include "consensus/validation.h"
#include "core_io.h"
#include "miner.h"
#include "policy/policy.h"
#include "random.h"
#include "script/sigcache.h"
#include "sidechain.h"
#include "uint256.h"
#include "util.h"
#include "utilstrencodings.h"
#include "validation.h"

#include "test/test_bitcoin.h"

#include <boost/test/unit_test.hpp>

static CFeeRate blockMinFeeRate = CFeeRate(DEFAULT_BLOCK_MIN_TX_FEE);

static BlockAssembler AssemblerForTest(const CChainParams& params) {
    BlockAssembler::Options options;

    options.nBlockMaxWeight = MAX_BLOCK_WEIGHT;
    options.blockMinFeeRate = blockMinFeeRate;
    return BlockAssembler(params, options);
}

BOOST_FIXTURE_TEST_SUITE(sidechain_tests, TestChain100Setup)


BOOST_AUTO_TEST_CASE(sidechain_bmm_valid_not_verified)
{
    // Test CheckBlock a block with valid BMM when -verifybmmcheckblock = false
    const CChainParams chainparams = Params();

    BOOST_CHECK(chainActive.Height() == 100);

    // Set -verifybmmcheckblock arg to false
    gArgs.ForceSetArg("-verifybmmcheckblock", "0");

    // Generate BMM block
    CScript scriptPubKey;
    CBlock block;
    std::string strError = "";
    BOOST_CHECK(AssemblerForTest(Params()).GenerateBMMBlock(scriptPubKey, block, strError));

    std::shared_ptr<const CBlock> shared_pblock = std::make_shared<const CBlock>(block);
    BOOST_CHECK(ProcessNewBlock(chainparams, shared_pblock, true, nullptr));

    BOOST_CHECK(chainActive.Height() == 101);
}

BOOST_AUTO_TEST_CASE(sidechain_bmm_valid)
{
    // A real example of a BMM (blinded) block before BMM proof has been added
    std::string blockHex = "00000020b1e9cedbf0f6ee194d88a795fe7d0a53b9c0355921b2155ef16757de27388130814b76a3d8af13bbb73118eadb735b09f6f8190683f4f221ebdb75edb6bdfd3df0a12b5dffff7f200100000000030000003f000000000000000000000000000000000000000000000000000000000000000000000000000001030000003f0001010000000000000000000000000000000000000000000000000000000000000000ffffffff03520101ffffffff020000000000000000000000000000000000266a24aa21a9ede2f61c3f71d1defd3fa999dfa36953755c690689799962b48bebd836974e8cf90120000000000000000000000000000000000000000000000000000000000000000000000000";

    // A real example of a mainchain coinbase BMM h* commitment
    std::string coinbaseHex = "020000000001010000000000000000000000000000000000000000000000000000000000000000ffffffff0502f3000101ffffffff03cc02062a01000000232102cda85899a9c51865e2482db94287f783ded2c77bb70ba327f3d51db67b318f59ac0000000000000000346a24d16173687f5fbe3f1cd68618c06e3236e3c73ee9081c56d0e185a8ea411343529f8ae55700bf0000000833323334363536330000000000000000266a24aa21a9ed607fb8779d041f21a8bb6292f63d82c6b2b4f7c470cc5481f3b371779a6384f50120000000000000000000000000000000000000000000000000000000000000000000000000";

    // A real example of the partial merkle tree proof used for BMM proof
    std::string proofHex = "00000020ec24cb98378b55fd9a570beb8a8d1b347388d29a29aba43b594a4c3cb1d5c18fd9030dbeee976380722d48f2c62b950580a0612060cf21800a84a5b2e37b63a925a22b5dffff5f1dab2c700002000000027beb77071246b2c3dd64ac5e755a35711700c0340faa1cd08a0b08824ae08285329fb8a709c787029e92d0cd12391380f3ad2935818123151c4607fd29ea195b0103";

    // Deserialize the example block
    CDataStream ss(ParseHex(blockHex), SER_DISK, PROTOCOL_VERSION);
    CBlock block;
    block.Unserialize(ss);

    // Should fail here, no BMM proof in block
    BOOST_CHECK(!VerifyCriticalHashProof(block));

    // Add BMM proof to the blck
    BOOST_CHECK(DecodeHexTx(block.criticalTx, coinbaseHex));
    block.criticalProof = proofHex;

    // Should fail here again, as we have no connection to the mainchain and
    // haven't cached the validation of this BMM proof
    BOOST_CHECK(!VerifyCriticalHashProof(block));

    // Cache the validation of this BMM proof
    bmmCache.CacheBMMProof(block.GetHash(), block.criticalTx.GetHash());
    BOOST_CHECK(bmmCache.HaveBMMProof(block.GetHash(), block.criticalTx.GetHash()));

    // It should pass now that we have added the BMM proof to the block and
    // cached the validation of that BMM proof.
    BOOST_CHECK(VerifyCriticalHashProof(block));
}

BOOST_AUTO_TEST_CASE(sidechain_bmm_invalid_verified)
{
    // Test CheckBlock without valid BMM when -verifybmmcheckblock = true
    const CChainParams chainparams = Params();

    BOOST_CHECK(chainActive.Height() == 100);

    // Set -verifybmmcheckblock arg to true
    gArgs.ForceSetArg("-verifybmmcheckblock", "1");

    // Generate BMM block
    CScript scriptPubKey;
    CBlock block;
    std::string strError = "";
    BOOST_CHECK(AssemblerForTest(Params()).GenerateBMMBlock(scriptPubKey, block, strError));

    std::shared_ptr<const CBlock> shared_pblock = std::make_shared<const CBlock>(block);
    BOOST_CHECK(!ProcessNewBlock(chainparams, shared_pblock, true, nullptr));

    BOOST_CHECK(chainActive.Height() == 100);

    // Set -verifybmmcheckblock arg back to default (gArgs effect all tests)
    gArgs.ForceSetArg("-verifybmmcheckblock", "0");
}

// TODO although more complicated than it may seem at first, it would be nice
// if we could also have a valid test that uses the TestChain100 and extends
// that chain. For now we can call VerifyCriticalHashProof on the block.

BOOST_AUTO_TEST_CASE(sidechain_bmm_cache)
{
    uint256 hashRand = GetRandHash();

    // Reject null block hash
    bmmCache.CacheBMMProof(uint256(), hashRand);
    BOOST_CHECK(!bmmCache.HaveBMMProof(uint256(), hashRand));

    // Reject null critical tx hash
    bmmCache.CacheBMMProof(hashRand, uint256());
    BOOST_CHECK(!bmmCache.HaveBMMProof(hashRand, uint256()));

    // Accept valid
    uint256 hash1 = GetRandHash();
    uint256 hash2 = GetRandHash();
    bmmCache.CacheBMMProof(hash1, hash2);
    BOOST_CHECK(bmmCache.HaveBMMProof(hash1, hash2));

    // Reject another
    bmmCache.CacheBMMProof(uint256(), hash1);
    BOOST_CHECK(!bmmCache.HaveBMMProof(uint256(), hashRand));

    // Accept many & check that they have been cached
    for (int i = 0; i <= 1776; i++) {
        uint256 hashRand1 = GetRandHash();
        uint256 hashRand2 = GetRandHash();
        bmmCache.CacheBMMProof(hashRand1, hashRand2);
        BOOST_CHECK(bmmCache.HaveBMMProof(hashRand1, hashRand2));
    }

    // Check that the first cached item still can be looked up
    BOOST_CHECK(bmmCache.HaveBMMProof(hash1, hash2));
}


// TODO :
// sidechain_wt
// sidechain_wt^
// sidechain deposit proof
// sidechain wt reuse in WT^
// sidechain deposit reuse
// sidechain deposit maturity

// TODO finish
BOOST_AUTO_TEST_CASE(sidechain_deposit)
{
    BOOST_CHECK(!psidechaintree->HaveDeposits());

    const CChainParams chainparams = Params();

    BOOST_CHECK(chainActive.Height() == 100);
/*
    // Generate BMM block
    CScript scriptPubKey;
    CBlock block;
    std::string strError = "";
    BOOST_CHECK(AssemblerForTest(Params()).GenerateBMMBlock(scriptPubKey, block, strError));


    std::shared_ptr<const CBlock> shared_pblock = std::make_shared<const CBlock>(block);
    BOOST_CHECK(ProcessNewBlock(chainparams, shared_pblock, true, nullptr));

    BOOST_CHECK(chainActive.Height() == 101);
*/
}

BOOST_AUTO_TEST_SUITE_END()
