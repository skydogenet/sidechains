// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_POLICY_WITHDRAWAL_BUNDLE_H
#define BITCOIN_POLICY_WITHDRAWAL_BUNDLE_H

#include <policy/corepolicy.h>

static const unsigned int MAX_WITHDRAWAL_BUNDLE_WEIGHT = (CORE_MAX_STANDARD_TX_WEIGHT / CORE_WITNESS_SCALE_FACTOR) / 2;

#endif // BITCOIN_POLICY_WITHDRAWAL_BUNDLE_H
