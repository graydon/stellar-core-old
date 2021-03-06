// Copyright 2014 Stellar Development Foundation and contributors. Licensed
// under the ISC License. See the COPYING file at the top-level directory of
// this distribution or at http://opensource.org/licenses/ISC

#include "transactions/PaymentOpFrame.h"
#include "util/Logging.h"
#include "ledger/LedgerDelta.h"
#include "ledger/TrustFrame.h"
#include "ledger/OfferFrame.h"
#include "database/Database.h"
#include "OfferExchange.h"

namespace stellar
{

using namespace std;

PaymentOpFrame::PaymentOpFrame(Operation const& op, OperationResult& res,
                               TransactionFrame& parentTx)
    : OperationFrame(op, res, parentTx), mPayment(mOperation.body.paymentOp())
{
}

bool
PaymentOpFrame::doApply(LedgerDelta& delta, LedgerManager& ledgerManager)
{
    AccountFrame destAccount;

    // if sending to self directly, just mark as success
    if (mPayment.destination == getSourceID() && mPayment.path.empty())
    {
        innerResult().code(PAYMENT_SUCCESS);
        return true;
    }

    Database& db = ledgerManager.getDatabase();

    if (!AccountFrame::loadAccount(mPayment.destination, destAccount, db))
    { // this tx is creating an account
        if (mPayment.currency.type() == NATIVE)
        {
            if (mPayment.amount < ledgerManager.getMinBalance(0))
            { // not over the minBalance to make an account
                innerResult().code(PAYMENT_LOW_RESERVE);
                return false;
            }
            else
            {
                destAccount.getAccount().accountID = mPayment.destination;
                destAccount.getAccount().seqNum =
                    delta.getHeaderFrame().getStartingSequenceNumber();
                destAccount.getAccount().balance = 0;

                destAccount.storeAdd(delta, db);
            }
        }
        else
        { // trying to send credit to an unmade account
            innerResult().code(PAYMENT_NO_DESTINATION);
            return false;
        }
    }

    return sendNoCreate(destAccount, delta, ledgerManager);
}

// work backward to determine how much they need to send to get the
// specified amount of currency to the recipient
bool
PaymentOpFrame::sendNoCreate(AccountFrame& destination, LedgerDelta& delta,
                             LedgerManager& ledgerManager)
{
    Database& db = ledgerManager.getDatabase();

    bool multi_mode = mPayment.path.size() != 0;
    if (multi_mode)
    {
        innerResult().code(PAYMENT_SUCCESS_MULTI);
    }
    else
    {
        innerResult().code(PAYMENT_SUCCESS);
    }

    // tracks the last amount that was traded
    int64_t curBReceived = mPayment.amount;
    Currency curB = mPayment.currency;

    // update balances, walks backwards

    // update last balance in the chain
    {

        if (curB.type() == NATIVE)
        {
            destination.getAccount().balance += curBReceived;
            destination.storeChange(delta, db);
        }
        else if (destination.getID() != curB.isoCI().issuer)
        {
            TrustFrame destLine;

            if (!TrustFrame::loadTrustLine(destination.getID(), curB, destLine,
                                           db))
            {
                innerResult().code(PAYMENT_NO_TRUST);
                return false;
            }

            if (!destLine.getTrustLine().authorized)
            {
                innerResult().code(PAYMENT_NOT_AUTHORIZED);
                return false;
            }

            if (!destLine.addBalance(curBReceived))
            {
                innerResult().code(PAYMENT_LINE_FULL);
                return false;
            }

            destLine.storeChange(delta, db);
        }

        if (multi_mode)
        {
            innerResult().multi().last =
                SimplePaymentResult(destination.getID(), curB, curBReceived);
        }
    }

    if (multi_mode)
    {
        // now, walk the path backwards
        for (int i = (int)mPayment.path.size() - 1; i >= 0; i--)
        {
            int64_t curASent, actualCurBReceived;
            Currency const& curA = mPayment.path[i];

            OfferExchange oe(delta, ledgerManager);

            // curA -> curB
            OfferExchange::ConvertResult r =
                oe.convertWithOffers(curA, INT64_MAX, curASent, curB,
                                     curBReceived, actualCurBReceived, nullptr);
            switch (r)
            {
            case OfferExchange::eFilterStop:
                assert(false); // no filter -> should not happen
                break;
            case OfferExchange::eOK:
                if (curBReceived == actualCurBReceived)
                {
                    break;
                }
            // fall through
            case OfferExchange::ePartial:
                innerResult().code(PAYMENT_OVERSENDMAX);
                return false;
            }
            assert(curBReceived == actualCurBReceived);
            curBReceived = curASent; // next round, we need to send enough
            curB = curA;
        }
    }

    // last step: we've reached the first account in the chain, update its
    // balance

    int64_t curBSent;

    curBSent = curBReceived;

    if (curBSent > mPayment.sendMax)
    { // make sure not over the max
        innerResult().code(PAYMENT_OVERSENDMAX);
        return false;
    }

    if (curB.type() == NATIVE)
    {
        int64_t minBalance = mSourceAccount->getMinimumBalance(ledgerManager);

        if (mSourceAccount->getAccount().balance < (minBalance + curBSent))
        { // they don't have enough to send
            innerResult().code(PAYMENT_UNDERFUNDED);
            return false;
        }

        mSourceAccount->getAccount().balance -= curBSent;
        mSourceAccount->storeChange(delta, db);
    }
    else
    {
        // issuer can always send its own credit
        if (getSourceID() != curB.isoCI().issuer)
        {
            AccountFrame issuer;
            if (!AccountFrame::loadAccount(curB.isoCI().issuer, issuer, db))
            {
                throw std::runtime_error("sendCredit Issuer not found");
            }

            TrustFrame sourceLineFrame;
            if (!TrustFrame::loadTrustLine(getSourceID(), curB, sourceLineFrame,
                                           db))
            {
                innerResult().code(PAYMENT_UNDERFUNDED);
                return false;
            }

            if (!sourceLineFrame.addBalance(-curBSent))
            {
                innerResult().code(PAYMENT_UNDERFUNDED);
                return false;
            }

            sourceLineFrame.storeChange(delta, db);
        }
    }

    return true;
}

bool
PaymentOpFrame::doCheckValid(Application& app)
{
    return true;
}
}
