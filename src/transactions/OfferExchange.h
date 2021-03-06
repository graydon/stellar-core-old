#pragma once

// Copyright 2015 Stellar Development Foundation and contributors. Licensed
// under the ISC License. See the COPYING file at the top-level directory of
// this distribution or at http://opensource.org/licenses/ISC

#include "transactions/OperationFrame.h"
#include "ledger/OfferFrame.h"
#include <vector>
#include <functional>

namespace stellar
{

class OfferExchange
{

    LedgerDelta& mDelta;
    LedgerManager& mLedgerManager;

    std::vector<ClaimOfferAtom> mOfferTrail;

  public:
    OfferExchange(LedgerDelta& delta, LedgerManager& ledgerManager);

    // buys wheat with sheep from a single offer
    enum CrossOfferResult
    {
        eOfferPartial,
        eOfferTaken,
        eOfferCantConvert
    };
    CrossOfferResult crossOffer(OfferFrame& sellingWheatOffer,
                                int64_t maxWheatReceived,
                                int64_t& numWheatReceived, int64_t maxSheepSend,
                                int64_t& numSheepSent);

    enum OfferFilterResult
    {
        eKeep,
        eStop
    };

    enum ConvertResult
    {
        eOK,
        ePartial,
        eFilterStop
    };
    // buys wheat with sheep, crossing as many offers as necessary
    ConvertResult convertWithOffers(
        Currency const& sheep, int64_t maxSheepSent, int64_t& sheepSend,
        Currency const& wheat, int64_t maxWheatReceive, int64_t& weatReceived,
        std::function<OfferFilterResult(OfferFrame const&)> filter);

    std::vector<ClaimOfferAtom> const&
    getOfferTrail() const
    {
        return mOfferTrail;
    }
};
}
