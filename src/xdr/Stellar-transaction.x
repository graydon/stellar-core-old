// Copyright 2015 Stellar Development Foundation and contributors. Licensed
// under the ISC License. See the COPYING file at the top-level directory of
// this distribution or at http://opensource.org/licenses/ISC

%#include "generated/Stellar-ledger-entries.h"

namespace stellar
{

struct DecoratedSignature
{
    opaque hint[4];    // first 4 bytes of the public key, used as a hint
    uint512 signature; // actual signature
};

enum OperationType
{
    PAYMENT = 0,
    CREATE_OFFER = 1,
    SET_OPTIONS = 2,
    CHANGE_TRUST = 3,
    ALLOW_TRUST = 4,
    ACCOUNT_MERGE = 5,
    INFLATION = 6
};

/* Payment

    send an amount to a destination account, optionally through a path.
    XLM payments create the destination account if it does not exist

    Threshold: med

    Result: PaymentResult
*/
struct PaymentOp
{
    AccountID destination; // recipient of the payment
    Currency currency;     // what they end up with
    int64 amount;          // amount they end up with

    opaque memo<32>;
    opaque sourceMemo<32>; // used to return a payment

    // payment over path
    Currency path<5>; // what hops it must go through to get there
    int64 sendMax; // the maximum amount of the source currency (==path[0]) to
                   // send (excluding fees).
                   // The operation will fail if can't be met
};

/* Creates, updates or deletes an offer

Threshold: med

Result: CreateOfferResult

*/
struct CreateOfferOp
{
    Currency takerGets;
    Currency takerPays;
    int64 amount; // amount taker gets. if set to 0, delete the offer
    Price price;  // =takerPaysAmount/takerGetsAmount

    // 0=create a new offer, otherwise edit an existing offer
    uint64 offerID;
};

/* Set Account Options

    updates "AccountEntry" fields.
    note: updating thresholds or signers requires high threshold

    Threshold: med or high

    Result: SetOptionsResult
*/

struct SetOptionsOp
{
    AccountID* inflationDest; // sets the inflation destination

    uint32* clearFlags; // which flags to clear
    uint32* setFlags;   // which flags to set

    Thresholds* thresholds; // update the thresholds for the account

    // Add, update or remove a signer for the account
    // signer is deleted if the weight is 0
    Signer* signer;
};

/* Creates, updates or deletes a trust line

    Threshold: med

    Result: ChangeTrustResult

*/
struct ChangeTrustOp
{
    Currency line;

    // if limit is set to 0, deletes the trust line
    int64 limit;
};

/* Updates the "authorized" flag of an existing trust line
   this is called by the issuer of the related currency.

   Threshold: low

   Result: AllowTrustResult
*/
struct AllowTrustOp
{
    AccountID trustor;
    union switch (CurrencyType type)
    {
    // NATIVE is not allowed
    case ISO4217:
        opaque currencyCode[4];

        // add other currency types here in the future
    }
    currency;

    bool authorize;
};

/* Inflation
    Runs inflation

Threshold: low

Result: InflationResult

*/

/* AccountMerge
    Transfers native balance to destination account.

    Threshold: high

    Result : AccountMergeResult
*/

/* An operation is the lowest unit of work that a transaction does */
struct Operation
{
    // sourceAccount is the account used to run the operation
    // if not set, the runtime defaults to "account" specified at
    // the transaction level
    AccountID* sourceAccount;

    union switch (OperationType type)
    {
    case PAYMENT:
        PaymentOp paymentOp;
    case CREATE_OFFER:
        CreateOfferOp createOfferOp;
    case SET_OPTIONS:
        SetOptionsOp setOptionsOp;
    case CHANGE_TRUST:
        ChangeTrustOp changeTrustOp;
    case ALLOW_TRUST:
        AllowTrustOp allowTrustOp;
    case ACCOUNT_MERGE:
        uint256 destination;
    case INFLATION:
        uint32 inflationSeq;
    }
    body;
};

/* a transaction is a container for a set of operations
    - is executed by an account
    - fees are collected from the account
    - operations are executed in order as one ACID transaction
          either all operations are applied or none are
          if any returns a failing code
*/

struct Transaction
{
    // account used to run the transaction
    AccountID account;

    // maximum fee this transaction can collect
    int32 maxFee;

    // sequence number to consume in the account
    SequenceNumber seqNum;

    // validity range for the ledger sequence number
    uint32 minLedger;
    uint32 maxLedger;

    Operation operations<100>;
};

/* A TransactionEnvelope wraps a transaction with signatures. */
struct TransactionEnvelope
{
    Transaction tx;
    DecoratedSignature signatures<20>;
};

/* Operation Results section */

/* This result is used when offers are taken during an operation */
struct ClaimOfferAtom
{
    // emited to identify the offer
    AccountID offerOwner; // Account that owns the offer
    uint64 offerID;

    // amount and currency taken from the owner
    Currency currencyClaimed;
    int64 amountClaimed;

    // should we also include the amount that the owner gets in return?
};

/******* Payment Result ********/

enum PaymentResultCode
{
    // codes considered as "success" for the operation
    PAYMENT_SUCCESS = 0,       // simple payment success
    PAYMENT_SUCCESS_MULTI = 1, // multi-path payment success

    // codes considered as "failure" for the operation
    PAYMENT_UNDERFUNDED = 2,    // not enough funds in source account
    PAYMENT_NO_DESTINATION = 3, // destination account does not exist
    PAYMENT_NO_TRUST = 4,       // destination missing a trust line for currency
    PAYMENT_NOT_AUTHORIZED = 5, // destination not authorized to hold currency
    PAYMENT_LINE_FULL = 6,      // destination would go above their limit
    PAYMENT_OVERSENDMAX = 7,    // multi-path payment could not satisfy sendmax
    PAYMENT_LOW_RESERVE = 8,    // would create an account below the min reserve
    PAYMENT_NOT_ENOUGH_OFFERS = 9 // not enough offers to fund payment
};

struct SimplePaymentResult
{
    AccountID destination;
    Currency currency;
    int64 amount;
};

struct PaymentSuccessMultiResult
{
    ClaimOfferAtom offers<>;
    SimplePaymentResult last;
};

union PaymentResult switch (PaymentResultCode code)
{
case PAYMENT_SUCCESS:
    void;
case PAYMENT_SUCCESS_MULTI:
    PaymentSuccessMultiResult multi;
default:
    void;
};

/******* CreateOffer Result ********/

enum CreateOfferResultCode
{
    // codes considered as "success" for the operation
    CREATE_OFFER_SUCCESS = 0,

    // codes considered as "failure" for the operation
    CREATE_OFFER_NO_TRUST = 1,       // can't hold what it's buying
    CREATE_OFFER_NOT_AUTHORIZED = 2, // not authorized to hold what it's buying
    CREATE_OFFER_MALFORMED = 3,      // generated offer would be invalid
    CREATE_OFFER_UNDERFUNDED = 4,    // doesn't hold what it's trying to sell
    CREATE_OFFER_CROSS_SELF = 5,     // would cross an offer from the same user

    // update errors
    CREATE_OFFER_NOT_FOUND = 6, // offerID does not match an existing offer
    CREATE_OFFER_MISMATCH = 7,  // currencies don't match offer

    CREATE_OFFER_LOW_RESERVE = 8 // not enough funds to create a new Offer

};

enum CreateOfferEffect
{
    CREATE_OFFER_CREATED = 0,
    CREATE_OFFER_UPDATED = 1,
    CREATE_OFFER_DELETED = 2
};

struct CreateOfferSuccessResult
{
    // offers that got claimed while creating this offer
    ClaimOfferAtom offersClaimed<>;

    union switch (CreateOfferEffect effect)
    {
    case CREATE_OFFER_CREATED:
    case CREATE_OFFER_UPDATED:
        OfferEntry offer;
    default:
        void;
    }
    offer;
};

union CreateOfferResult switch (CreateOfferResultCode code)
{
case CREATE_OFFER_SUCCESS:
    CreateOfferSuccessResult success;
default:
    void;
};

/******* SetOptions Result ********/

enum SetOptionsResultCode
{
    // codes considered as "success" for the operation
    SET_OPTIONS_SUCCESS = 0,
    // codes considered as "failure" for the operation
    SET_OPTIONS_RATE_FIXED = 1,
    SET_OPTIONS_RATE_TOO_HIGH = 2,
    SET_OPTIONS_LOW_RESERVE = 3,      // not enough funds to add a signer
    SET_OPTIONS_TOO_MANY_SIGNERS = 4, // max number of signers already reached
    SET_OPTIONS_BAD_FLAGS = 5         // invalid combination of clear/set flags
};

union SetOptionsResult switch (SetOptionsResultCode code)
{
case SET_OPTIONS_SUCCESS:
    void;
default:
    void;
};

/******* ChangeTrust Result ********/

enum ChangeTrustResultCode
{
    // codes considered as "success" for the operation
    CHANGE_TRUST_SUCCESS = 0,
    // codes considered as "failure" for the operation
    CHANGE_TRUST_NO_ISSUER = 1,     // could not find issuer
    CHANGE_TRUST_INVALID_LIMIT = 2, // cannot drop limit below balance
    CHANGE_TRUST_LOW_RESERVE = 3 // not enough funds to create a new trust line
};

union ChangeTrustResult switch (ChangeTrustResultCode code)
{
case CHANGE_TRUST_SUCCESS:
    void;
default:
    void;
};

/******* AllowTrust Result ********/

enum AllowTrustResultCode
{
    // codes considered as "success" for the operation
    ALLOW_TRUST_SUCCESS = 0,
    // codes considered as "failure" for the operation
    ALLOW_TRUST_MALFORMED = 1,         // currency is not ISO4217
    ALLOW_TRUST_NO_TRUST_LINE = 2,     // trustor does not have a trustline
    ALLOW_TRUST_TRUST_NOT_REQUIRED = 3 // source account does not require trust
};

union AllowTrustResult switch (AllowTrustResultCode code)
{
case ALLOW_TRUST_SUCCESS:
    void;
default:
    void;
};

/******* AccountMerge Result ********/

enum AccountMergeResultCode
{
    // codes considered as "success" for the operation
    ACCOUNT_MERGE_SUCCESS = 0,
    // codes considered as "failure" for the operation
    ACCOUNT_MERGE_MALFORMED = 1,  // can't merge onto itself
    ACCOUNT_MERGE_NO_ACCOUNT = 2, // destination does not exist
    ACCOUNT_MERGE_HAS_CREDIT = 3, // account has active trust lines
    ACCOUNT_MERGE_CREDIT_HELD = 4 // an issuer cannot be merged if used
};

union AccountMergeResult switch (AccountMergeResultCode code)
{
case ACCOUNT_MERGE_SUCCESS:
    void;
default:
    void;
};

/******* Inflation Result ********/

enum InflationResultCode
{
    // codes considered as "success" for the operation
    INFLATION_SUCCESS = 0,
    // codes considered as "failure" for the operation
    INFLATION_NOT_TIME = 1
};

struct inflationPayout // or use PaymentResultAtom to limit types?
{
    AccountID destination;
    int64 amount;
};

union InflationResult switch (InflationResultCode code)
{
case INFLATION_SUCCESS:
    inflationPayout payouts<>;
default:
    void;
};

/* High level Operation Result */

enum OperationResultCode
{
    opINNER = 0, // inner object result is valid

    opBAD_AUTH = 1,  // not enough signatures to perform operation
    opNO_ACCOUNT = 2 // account was not found
};

union OperationResult switch (OperationResultCode code)
{
case opINNER:
    union switch (OperationType type)
    {
    case PAYMENT:
        PaymentResult paymentResult;
    case CREATE_OFFER:
        CreateOfferResult createOfferResult;
    case SET_OPTIONS:
        SetOptionsResult setOptionsResult;
    case CHANGE_TRUST:
        ChangeTrustResult changeTrustResult;
    case ALLOW_TRUST:
        AllowTrustResult allowTrustResult;
    case ACCOUNT_MERGE:
        AccountMergeResult accountMergeResult;
    case INFLATION:
        InflationResult inflationResult;
    }
    tr;
default:
    void;
};

enum TransactionResultCode
{
    txSUCCESS = 0,

    txFAILED = 1,
    txBAD_LEDGER = 2,
    txDUPLICATE = 3,
    txMALFORMED = 4,
    txBAD_SEQ = 5,

    txBAD_AUTH = 6, // not enough signatures to perform transaction
    txINSUFFICIENT_BALANCE = 7,
    txNO_ACCOUNT = 8,
    txINSUFFICIENT_FEE = 9, // max fee is too small
    txINTERNAL_ERROR = 10 // an unknown error occured
};

struct TransactionResult
{
    int64 feeCharged;

    union switch (TransactionResultCode code)
    {
    case txSUCCESS:
    case txFAILED:
        OperationResult results<>;
    default:
        void;
    }
    result;
};
}
