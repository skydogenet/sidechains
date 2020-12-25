// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <policy/corepolicy.h>

// TODO the following includes could be replaced with the core versions.
#include <amount.h>
#include <policy/fees.h>
#include <primitives/transaction.h>
#include <script/script.h>
#include <serialize.h>
#include <consensus/validation.h>

CAmount CoreGetDustThreshold(const CTxOut& txout, const CFeeRate& dustRelayFeeIn)
{
    if (txout.scriptPubKey.IsUnspendable())
        return 0;

    size_t nSize = GetSerializeSize(txout, SER_DISK, 0);
    int witnessversion = 0;
    std::vector<unsigned char> witnessprogram;

    if (txout.scriptPubKey.IsWitnessProgram(witnessversion, witnessprogram)) {
        // sum the sizes of the parts of a transaction input
        // with 75% segwit discount applied to the script size.
        nSize += (32 + 4 + 1 + (107 / CORE_WITNESS_SCALE_FACTOR) + 4);
    } else {
        nSize += (32 + 4 + 1 + 107 + 4); // the 148 mentioned above
    }

    return dustRelayFeeIn.GetFee(nSize);
}

bool CoreIsDust(const CTxOut& txout, const CFeeRate& dustRelayFeeIn)
{
    return (txout.nValue < CoreGetDustThreshold(txout, dustRelayFeeIn));
}

bool CoreIsStandard(const CScript& scriptPubKey, txnouttype& whichType, std::string& reason)
{
    // TODO should use the core version of solver, and core txnouttype(s)
    std::vector<std::vector<unsigned char> > vSolutions;
    if (!Solver(scriptPubKey, whichType, vSolutions)) {
        reason = "failed-to-solve";
        return false;
    }

    if (whichType == TX_NONSTANDARD) {
        reason = "tx-nonstandard";
        return false;
    } else if (whichType == TX_MULTISIG) {
        unsigned char m = vSolutions.front()[0];
        unsigned char n = vSolutions.back()[0];
        // Support up to x-of-3 multisig txns as standard
        if (n < 1 || n > 3) {
            reason = "multisig-nonstandard-n";
            return false;
        }
        if (m < 1 || m > n) {
            reason = "multisig-nonstandard-m";
            return false;
        }
    } else if (whichType == TX_NULL_DATA &&
               (!fAcceptDatacarrier || scriptPubKey.size() > nMaxDatacarrierBytes)) {
        reason = "null-data-nonstandard";
        return false;
    }

    return true;
}

bool CoreIsStandardTx(const CTransaction& tx, bool permit_bare_multisig, const CFeeRate& dust_relay_fee, std::string& reason)
{
    if (tx.nVersion > CORE_MAX_STANDARD_TX_VERSION || tx.nVersion < 1) {
        reason = "version";
        return false;
    }

    // Extremely large transactions with lots of inputs can cost the network
    // almost as much to process as they cost the sender in fees, because
    // computing signature hashes is O(ninputs*txsize). Limiting transactions
    // to MAX_STANDARD_TX_WEIGHT mitigates CPU exhaustion attacks.
    unsigned int sz = GetTransactionWeight(tx); // TODO use the core version
    if (sz > CORE_MAX_STANDARD_TX_WEIGHT) {
        reason = "tx-size";
        return false;
    }

    for (const CTxIn& txin : tx.vin)
    {
        // Biggest 'standard' txin is a 15-of-15 P2SH multisig with compressed
        // keys (remember the 520 byte limit on redeemScript size). That works
        // out to a (15*(33+1))+3=513 byte redeemScript, 513+1+15*(73+1)+3=1627
        // bytes of scriptSig, which we round off to 1650 bytes for some minor
        // future-proofing. That's also enough to spend a 20-of-20
        // CHECKMULTISIG scriptPubKey, though such a scriptPubKey is not
        // considered standard.
        if (txin.scriptSig.size() > 1650) {
            reason = "scriptsig-size";
            return false;
        }
        if (!txin.scriptSig.IsPushOnly()) {
            reason = "scriptsig-not-pushonly";
            return false;
        }
    }

    unsigned int nDataOut = 0;
    txnouttype whichType;
    for (const CTxOut& txout : tx.vout) {
        std::string strScriptPubKeyReason = "";
        if (!CoreIsStandard(txout.scriptPubKey, whichType, strScriptPubKeyReason)) {
            reason = "scriptpubkey - ";
            reason += strScriptPubKeyReason;
            return false;
        }

        if (whichType == TX_NULL_DATA)
            nDataOut++;
        else if ((whichType == TX_MULTISIG) && (!permit_bare_multisig)) {
            reason = "bare-multisig";
            return false;
        } else if (CoreIsDust(txout, dust_relay_fee)) {
            reason = "dust";
            return false;
        }
    }

    return true;
}
