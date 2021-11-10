// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_CORE_POLICY_H
#define BITCOIN_CORE_POLICY_H

#include <script/standard.h>
#include <string>
#include <stdlib.h>
#include <stdint.h>

class CFeeRate;
class CScript;
class CTransaction;

/**
 * This class exists so that WithdrawalBundle(s) can be checked for most mainchain policies
 * and standardness requirements without contacting the mainchain.
 *
 * Bitcoin core policy settings, taken from github.com/bitcoin/bitcoin
 * policy/policy.h, primitives/transaction.h, consensus/consensus.h
 */

static const int CORE_WITNESS_SCALE_FACTOR = 4;

// TODO check when creating WithdrawalBundle
//static const size_t CORE_MIN_TRANSACTION_WEIGHT = CORE_WITNESS_SCALE_FACTOR * 60; // 60 is the lower bound for the size of a valid serialized CTransaction
//static const size_t CORE_MIN_SERIALIZABLE_TRANSACTION_WEIGHT = CORE_WITNESS_SCALE_FACTOR * 10; // 10 is the lower bound for the size of a serialized CTransaction

/** The maximum weight for transactions we're willing to relay/mine */
static const unsigned int CORE_MAX_STANDARD_TX_WEIGHT = 400000;

// TODO check when creating WithdrawalBundle
/** The minimum non-witness size for transactions we're willing to relay/mine (1 segwit input + 1 P2WPKH output = 82 bytes) */
//static const unsigned int CORE_MIN_STANDARD_TX_NONWITNESS_SIZE = 82;
/** The maximum number of sigops we're willing to relay/mine in a single tx */
//static const unsigned int CORE_MAX_STANDARD_TX_SIGOPS_COST = CORE_MAX_BLOCK_SIGOPS_COST/5;
/** Min feerate for defining dust. */
//static const unsigned int CORE_DUST_RELAY_TX_FEE = 3000;

static const int32_t CORE_CURRENT_TX_VERSION=2;
static const int32_t CORE_MAX_STANDARD_TX_VERSION=2;

CAmount CoreGetDustThreshold(const CTxOut& txout, const CFeeRate& dustRelayFee);
bool CoreIsDust(const CTxOut& txout, const CFeeRate& dustRelayFee);
bool CoreIsStandard(const CScript& scriptPubKey, txnouttype& whichType, std::string& reason);
bool CoreIsStandardTx(const CTransaction& tx, bool permit_bare_multisig, const CFeeRate& dust_relay_fee, std::string& reason);

#endif /* BITCOIN_CORE_POLICY */
