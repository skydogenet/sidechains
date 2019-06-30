// Copyright (c) 2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "consensus/validation.h"
#include "core_io.h"
#include "miner.h"
#include "random.h"
#include "script/sigcache.h"
#include "sidechain.h"
#include "uint256.h"
#include "utilstrencodings.h"
#include "validation.h"

#include "test/test_bitcoin.h"

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(sidechain_tests, TestChain100Setup)

BOOST_AUTO_TEST_CASE(sidechain_deposit)
{
    BOOST_CHECK(!psidechaintree->HaveDeposits());
}

// TODO :
// sidechain_wt
// sidechain_wt^
// sidechain block bmm proof
// sidechain deposit proof
// sidechain wt reuse in WT^
// sidechain deposit reuse
// sidechain deposit maturity

BOOST_AUTO_TEST_SUITE_END()
