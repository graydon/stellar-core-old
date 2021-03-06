#pragma once

// Copyright 2015 Stellar Development Foundation and contributors. Licensed
// under the ISC License. See the COPYING file at the top-level directory of
// this distribution or at http://opensource.org/licenses/ISC

#include "transactions/OperationFrame.h"

namespace stellar
{

class PaymentOpFrame : public OperationFrame
{
    // destination must exist
    bool sendNoCreate(AccountFrame& destination, LedgerDelta& delta,
                      LedgerManager& ledgerManager);

    PaymentResult&
    innerResult()
    {
        return mResult.tr().paymentResult();
    }
    PaymentOp const& mPayment;

  public:
    PaymentOpFrame(Operation const& op, OperationResult& res,
                   TransactionFrame& parentTx);

    bool doApply(LedgerDelta& delta, LedgerManager& ledgerManager);
    bool doCheckValid(Application& app);

    static PaymentResultCode
    getInnerCode(OperationResult const& res)
    {
        return res.tr().paymentResult().code();
    }
};
}
