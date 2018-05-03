// Copyright (c) 2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_PRIMITIVES_SIDECHAIN_H
#define BITCOIN_PRIMITIVES_SIDECHAIN_H

#include "amount.h"
#include "merkleblock.h"
#include "primitives/transaction.h"
#include "pubkey.h"
#include "script/script.h"
#include "serialize.h"
#include "uint256.h"

#include <limits.h>
#include <string>
#include <vector>

struct Sidechain {
    uint8_t nSidechain;
    uint16_t nWTPrimeBroadcastInterval;
    CScript depositScript;

    std::string ToString() const;
    std::string GetSidechainName() const;
};

enum Sidechains {
    // This sidechain
    SIDECHAIN_TEST = 0,
};

//! KeyID for testing
static const std::string testkey = "b5437dc6a4e5da5597548cf87db009237d286636";
//mx3PT9t2kzCFgAURR9HeK6B5wN8egReUxY
//cN5CqwXiaNWhNhx3oBQtA8iLjThSKxyZjfmieTsyMpG6NnHBzR7J

//! This sidechain as it has been described to the mainchain
static const Sidechain THIS_SIDECHAIN = {
    SIDECHAIN_TEST, 10, CScript() << THIS_SIDECHAIN.nSidechain << ToByteVector(testkey) << OP_NOP4
};

//! This sidechain's fee script
static const CScript SIDECHAIN_FEESCRIPT = CScript() << OP_DUP << OP_HASH160 << ToByteVector(testkey) << OP_EQUALVERIFY << OP_CHECKSIG;
//! Max number of WT^(s) per sidechain per period
static const int SIDECHAIN_MAX_WT = 3;
//! State script version number
static const int SIDECHAIN_STATE_VERSION = 0;

//! The default payment amount to mainchain miner for critical data commitment
static const CAmount DEFAULT_CRITICAL_DATA_AMOUNT = 1 * CENT;

/**
 * Base object for sidechain related database entries
 */
struct SidechainObj {
    char sidechainop;
    uint32_t nHeight;
    uint256 txid;

    SidechainObj(void): nHeight(INT_MAX) { }
    virtual ~SidechainObj(void) { }

    uint256 GetHash(void) const;
    CScript GetScript(void) const;
    virtual std::string ToString(void) const;
};

/**
 * Sidechain individual withdrawal (WT) database object
 */
struct SidechainWT: public SidechainObj {
    uint8_t nSidechain;
    std::string strDestination;
    CMutableTransaction wt;

    SidechainWT(void) : SidechainObj() { sidechainop = 'W'; }
    virtual ~SidechainWT(void) { }

    ADD_SERIALIZE_METHODS

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(sidechainop);
        READWRITE(nSidechain);
        READWRITE(strDestination);
        READWRITE(wt);
    }

    std::string ToString(void) const;
};

/**
 * Sidechain joined withdraw proposal (WT^) database object
 */
struct SidechainWTJoin: public SidechainObj {
    uint8_t nSidechain;
    CMutableTransaction wtJoin;

    SidechainWTJoin(void) : SidechainObj() { sidechainop = 'J'; }
    virtual ~SidechainWTJoin(void) { }

    ADD_SERIALIZE_METHODS

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(sidechainop);
        READWRITE(nSidechain);
        READWRITE(wtJoin);
    }

    std::string ToString(void) const;
};

/**
 * Sidechain deposit database object
 */
struct SidechainDeposit : public SidechainObj {
    uint8_t nSidechain;
    CKeyID keyID;
    CAmount amtUserPayout;
    CMutableTransaction dtx;
    CMainchainMerkleBlock proof;

    SidechainDeposit(void) : SidechainObj() { sidechainop = 'D'; }
    virtual ~SidechainDeposit(void) { }

    ADD_SERIALIZE_METHODS

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(sidechainop);
        READWRITE(nSidechain);
        READWRITE(keyID);
        READWRITE(amtUserPayout);
        READWRITE(dtx);
        READWRITE(proof);
    }

    std::string ToString(void) const;
};

struct SidechainBMMProof
{
    uint256 hashBMMBlock; // TODO remove
    std::string txOutProof;
    std::string coinbaseHex;

    bool HasProof()
    {
        return (txOutProof.size() && coinbaseHex.size());
    }
};

/**
 * Create sidechain object
 */
SidechainObj *SidechainObjCtr(const CScript &);

#endif // BITCOIN_PRIMITIVES_SIDECHAIN_H
