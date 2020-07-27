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

/* Sidechain Identifiers */

//! Sidechain address bytes
static const std::string SIDECHAIN_ADDRESS_BYTES = "0186ff51f527ffdcf2413d50bdf8fab1feb20e5f82815dad48c73cf462b8b313";

//! Sidechain build commit hash
static const std::string SIDECHAIN_BUILD_COMMIT_HASH = "efe0934d9dd57d149205ed96c3ba0f5d9a798baf";

//! Sidechain build tar hash
static const std::string SIDECHAIN_BUILD_TAR_HASH = "f20e1f628c7b6702184b3dce1157ce30cfb47d5988ab13d6d3a6199e8ee31032";

static const int MAINCHAIN_WTPRIME_MIN_WORKSCORE = 40; //140;

static const unsigned int DEFAULT_MIN_WT_CREATE_WTPRIME = 10;

// Temporary testnet value
static const int WTPRIME_FAIL_WAIT_PERIOD = 20;
// Real value final release: static const int WTPRIME_FAIL_WAIT_PERIOD = 144;

struct Sidechain {
    uint8_t nSidechain;
    CScript depositScript;

    std::string ToString() const;
    std::string GetSidechainName() const;
};

enum Sidechains {
    // This sidechain
    SIDECHAIN_TEST = 0,
};

//! WT status / zone (unspent, included in a WT^, paid out)
static const char WT_UNSPENT = 'u';
static const char WT_IN_WTPRIME = 'p';
static const char WT_SPENT = 's';

//! WT^ status / zone
static const char WTPRIME_CREATED = 'c';
static const char WTPRIME_FAILED = 'f';
static const char WTPRIME_SPENT = 'o';

//! KeyID for testing
static const std::string testkey = "b5437dc6a4e5da5597548cf87db009237d286636";
//mx3PT9t2kzCFgAURR9HeK6B5wN8egReUxY
//cN5CqwXiaNWhNhx3oBQtA8iLjThSKxyZjfmieTsyMpG6NnHBzR7J

//! Key ID for fee script
static const std::string feeKey = "5f8f196a4f0c212fee1b4eda31e3ef383c52d9fc";
// 19iGcwHuZA1edpd6veLfbkHtbDPS9hAXbh
// ed7565854e9b7a334e39e33614abce078a6c06603b048a9536a7e41abf3da504

//! The default payment amount to mainchain miner for critical data commitment
static const CAmount DEFAULT_CRITICAL_DATA_AMOUNT = 0.0001 * COIN;

//! The fee for sidechain deposits on this sidechain
static const CAmount SIDECHAIN_DEPOSIT_FEE = 0.00001 * COIN;

static const char DB_SIDECHAIN_DEPOSIT_OP = 'D';
static const char DB_SIDECHAIN_WT_OP = 'W';
static const char DB_SIDECHAIN_WTPRIME_OP = 'P';

/**
 * Base object for sidechain related database entries
 */
struct SidechainObj {
    char sidechainop;

    SidechainObj(void) { }
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
    std::string strRefundDestination;
    CAmount amount;
    CAmount mainchainFee;
    char status;
    uint256 hashBlindWTX; // The hash of the WT transaction minus the WT script

    SidechainWT(void) : SidechainObj() { sidechainop = DB_SIDECHAIN_WT_OP; status = WT_UNSPENT; }
    virtual ~SidechainWT(void) { }

    ADD_SERIALIZE_METHODS

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(sidechainop);
        READWRITE(nSidechain);
        READWRITE(strDestination);
        READWRITE(strRefundDestination);
        READWRITE(amount);
        READWRITE(mainchainFee);
        READWRITE(status);
        READWRITE(hashBlindWTX);
    }

    std::string ToString(void) const;
    std::string GetStatusStr(void) const;

    uint256 GetID() const {
        SidechainWT wt(*this);
        wt.status = WT_UNSPENT;
        return wt.GetHash();
    }
};

/**
 * Sidechain joined withdraw proposal (WT^) database object
 */
struct SidechainWTPrime: public SidechainObj {
    uint8_t nSidechain;
    CMutableTransaction wtPrime;
    std::vector<uint256> vWT; // The id in ldb of WT(s) that this WT^ is using
    int nHeight;
    // If the WT^ fails we keep track of the sidechain height that it was marked
    // failed at so that we can wait WTPRIME_FAIL_WAIT_PERIOD before trying the
    // next WT^.
    int nFailHeight;
    char status;

    SidechainWTPrime(void) : SidechainObj() { sidechainop = DB_SIDECHAIN_WTPRIME_OP; status = WTPRIME_CREATED; nHeight = 0;}
    virtual ~SidechainWTPrime(void) { }

    ADD_SERIALIZE_METHODS

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(sidechainop);
        READWRITE(nSidechain);
        READWRITE(wtPrime);
        READWRITE(vWT);
        READWRITE(status);
        READWRITE(nHeight);
        READWRITE(nFailHeight);
    }

    uint256 GetID() const {
        SidechainWTPrime wt(*this);
        wt.status = WTPRIME_CREATED;
        wt.nHeight = 0;
        wt.nFailHeight = 0;
        return wt.GetHash();
    }

    std::string ToString(void) const;

    std::string GetStatusStr(void) const;
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
    uint32_t n;

    SidechainDeposit(void) : SidechainObj() { sidechainop = DB_SIDECHAIN_DEPOSIT_OP; }
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
        READWRITE(n);
    }

    std::string ToString(void) const;

    bool operator==(const SidechainDeposit& d) const
    {
        if (sidechainop == d.sidechainop &&
                nSidechain == d.nSidechain &&
                keyID == d.keyID &&
                amtUserPayout == d.amtUserPayout &&
                dtx == d.dtx &&
                proof == d.proof &&
                n == d.n) {
            return true;
        }
        return false;
    }

    uint256 GetID() const
    {
        SidechainDeposit deposit(*this);
        deposit.amtUserPayout = CAmount(0);
        return deposit.GetHash();
    }
};

struct SidechainBMMProof
{
    uint256 hashBMMBlock;
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

// Functions for both WT^ creation and the GUI to use in order to make sure that
// what the GUI displays (on the pending WT table) is the same as what the WT^
// creation code will actually select.

// Sort a vector of SidechainWT by mainchain fee in descending order
void SortWTByFee(std::vector<SidechainWT>& vWT);

// Sort a vector of SidechainWTPrime by height in descending order
void SortWTPrimeByHeight(std::vector<SidechainWTPrime>& vWTPrime);

// Erase all SidechainWT from a vector which do not have WT_UNSPENT status
void SelectUnspentWT(std::vector<SidechainWT>& vWT);

#endif // BITCOIN_PRIMITIVES_SIDECHAIN_H
