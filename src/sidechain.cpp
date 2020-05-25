// Copyright (c) 2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "sidechain.h"

#include "clientversion.h"
#include "core_io.h"
#include "hash.h"
#include "streams.h"
#include "utilmoneystr.h"

#include <sstream>

const uint32_t nType = 1;
const uint32_t nVersion = 1;

uint256 SidechainObj::GetHash(void) const
{
    uint256 ret;
    if (sidechainop == DB_SIDECHAIN_WT_OP)
        ret = SerializeHash(*(SidechainWT *) this);
    else
    if (sidechainop == DB_SIDECHAIN_WTPRIME_OP)
        ret = SerializeHash(*(SidechainWTPrime *) this);
    else
    if (sidechainop == DB_SIDECHAIN_DEPOSIT_OP)
        ret = SerializeHash(*(SidechainDeposit *) this);

    return ret;
}

CScript SidechainObj::GetScript(void) const
{
    CDataStream ds (SER_DISK, CLIENT_VERSION);
    if (sidechainop == DB_SIDECHAIN_WT_OP)
        ((SidechainWT *) this)->Serialize(ds);
    else
    if (sidechainop == DB_SIDECHAIN_WTPRIME_OP)
        ((SidechainWTPrime *) this)->Serialize(ds);
    else
    if (sidechainop == DB_SIDECHAIN_DEPOSIT_OP)
        ((SidechainDeposit *) this)->Serialize(ds);

    CScript script;
    script << std::vector<unsigned char>(ds.begin(), ds.end()) << OP_SIDECHAIN;
    return script;
}

SidechainObj *SidechainObjCtr(const CScript &script)
{
    CScript::const_iterator pc = script.begin();
    std::vector<unsigned char> vch;
    opcodetype opcode;

    if (!script.GetOp(pc, opcode, vch))
        return NULL;
    if (vch.size() == 0)
        return NULL;
    const char *vch0 = (const char *) &vch.begin()[0];
    CDataStream ds(vch0, vch0+vch.size(), SER_DISK, CLIENT_VERSION);

    if (*vch0 == DB_SIDECHAIN_WT_OP) {
        SidechainWT *obj = new SidechainWT;
        obj->Unserialize(ds);
        return obj;
    }
    else
    if (*vch0 == DB_SIDECHAIN_WTPRIME_OP) {
        SidechainWTPrime *obj = new SidechainWTPrime;
        obj->Unserialize(ds);
        return obj;
    }
    else
    if (*vch0 == DB_SIDECHAIN_DEPOSIT_OP) {
        SidechainDeposit *obj = new SidechainDeposit;
        obj->Unserialize(ds);
        return obj;
    }
    return NULL;
}

std::string SidechainObj::ToString(void) const
{
    std::stringstream str;
    str << "sidechainop=" << sidechainop << std::endl;
    return str.str();
}

std::string SidechainWT::ToString() const
{
    std::stringstream str;
    str << "sidechainop=" << sidechainop << std::endl;
    str << "nSidechain=" << std::to_string(nSidechain) << std::endl;
    str << "destination=" << strDestination << std::endl;
    str << "amount=" << FormatMoney(amount) << std::endl;

    std::string strStatus;
    if (status == WT_UNSPENT) {
        strStatus = "WT_UNSPENT";
    }
    else
    if (status == WT_IN_WTPRIME) {
        strStatus = "WT_IN_WTPRIME";
    }
    else
    {
        strStatus = "WT_SPENT";
    }

    str << "status=" << strStatus << std::endl;
    str << "hashBlindWTX=" << hashBlindWTX.ToString() << std::endl;
    return str.str();
}

std::string SidechainWTPrime::ToString() const
{
    std::stringstream str;
    str << "sidechainop=" << sidechainop << std::endl;
    str << "nSidechain=" << std::to_string(nSidechain) << std::endl;
    str << "wtprime=" << CTransaction(wtPrime).ToString() << std::endl;
    std::string strStatus;
    if (status == WTPRIME_CREATED) {
        strStatus = "WTPRIME_CREATED";
    }
    else
    if (status == WTPRIME_FAILED) {
        strStatus = "WTPRIME_FAILED";
    }
    else
    if (status == WTPRIME_SPENT) {
        strStatus = "WTPRIME_SPENT";
    }
    str << "status=" << strStatus << std::endl;
    return str.str();
}

std::string SidechainDeposit::ToString() const
{
    std::stringstream str;
    str << "sidechainop=" << sidechainop << std::endl;
    str << "nSidechain=" << (unsigned int)nSidechain << std::endl;
    str << "nSidechain=" << std::to_string(nSidechain) << std::endl;
    str << "keyID=" << keyID.ToString() << std::endl;
    str << "payout=" << FormatMoney(amtUserPayout) << std::endl;
    str << "mainchaintxid=" << dtx.GetHash().ToString() << std::endl;
    str << "n=" << std::to_string(n) << std::endl;
    str << "inputs:\n";
    for (const CTxIn& in : dtx.vin) {
        str << in.prevout.ToString() << std::endl;
    }
    return str.str();
}
