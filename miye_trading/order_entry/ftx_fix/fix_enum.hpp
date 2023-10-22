#pragma once
#include <cstdint>

namespace miye::trading::fix
{

/*
 * Below enum values are taken from FTX and standard FIX 4.2 protocol
 */

enum class SessionRejectReasonEnum : char
{
    RequiredTagMissing          = 1,
    ValueIncorrectForThisTag    = 5,
    IncorrectDataFormatForValue = 6,
    InvalidMessageType          = 11
};

enum class OrdRejReasonEnum : char
{
    OtherErrors       = 0,
    RiskLimitExceeded = 1,
    TooManyRequests   = 2
};

enum class ExecTypeEnum : char
{
    NewOrder         = '0',
    NewFillForOrder  = '1',
    OrderDone        = '3',
    OrderCancelled   = '4',
    OrderResized     = '5',
    OrderPlaced      = 'A',
    OrderRejected    = '8',
    OrderCanceled    = '6',
    OrderStatusReply = 'I'
};

enum class MsgTypeEnum : char
{
    HeartBeat                     = '0',
    TestRequest                   = '1',
    ResendRequest                 = '2',
    Reject                        = '3',
    SequenceReset                 = '4',
    Logout                        = '5',
    IndicationOfInterest          = '6',
    Advertisement                 = '7',
    ExecutionReport               = '8',
    OrderCancelReject             = '9',
    QuoteStatusRequest            = 'a',
    Logon                         = 'A',
    News                          = 'B',
    QuoteAcknowledgement          = 'b',
    Email                         = 'C',
    SecurityDefinitionRequest     = 'c',
    OrderSingle                   = 'D',
    SecurityDefinition            = 'd',
    OrderList                     = 'E',
    SecurityStatusRequest         = 'e',
    SecurityStatus                = 'f',
    OrderCancelRequest            = 'F',
    OrderCancelReplaceRequest     = 'G',
    TradingSessionStatusRequeset  = 'g',
    OrderStatusRequest            = 'H',
    TradingSessionStatus          = 'h',
    MassQuote                     = 'i',
    BusinessMessageReject         = 'j',
    Allocation                    = 'J',
    ListCancelRequest             = 'K',
    BidRequest                    = 'k',
    BidResponse                   = 'l',
    ListExecute                   = 'L',
    ListStrikePrice               = 'm',
    ListStatusRequest             = 'M',
    ListStatus                    = 'N',
    AllocationAck                 = 'P',
    DontKnowTrade                 = 'Q',
    QuoteRequest                  = 'R',
    Quote                         = 'S',
    SettlementInstructions        = 'T',
    MarketDataRequest             = 'V',
    MarketDataSnapshotFullRefresh = 'W',
    MarketDataIncrementalRefresh  = 'X',
    MarketDataRequestReject       = 'Y',
    QuoteCancel                   = 'Z'
};

enum class HandInstEnum : uint8_t
{
    Unknown            = 0,
    AutomatedExecution = 1
};

enum class CustOrderHandlingInst : char
{
    Unknown                                = '0',
    PhoneSimple                            = 'A',
    PhoneComplex                           = 'B',
    FcmProvidedScreen                      = 'C',
    OtherProvidedScreen                    = 'D',
    ClientProvidedPlatformControlledByFcm  = 'E',
    ClientProvidedPlatformDirectToExchange = 'F',
    FcmApiOrFix                            = 'G',
    AlgoEngine                             = 'H',
    PriceAtExecution                       = 'J',
    DeskElectronic                         = 'W',
    DeskPit                                = 'X',
    ClientElectronic                       = 'Y',
    ClientPit                              = 'Z'
};

enum class OrderTypeEnum : char
{
    Unknown          = '0',
    MarketOrder      = '1',
    Limitorder       = '2',
    StopOrder        = '3',
    StopLimitOrder   = '4',
    MarketLimitOrder = 'K'
};

enum class SideEnum : char
{
    Unknown = '0',
    Buy     = '1',
    Sell    = '2'
};

enum class OrdStatusEnum : char
{
    NewOrderAck      = '0',
    PartialFill      = '1',
    OrderFullyFilled = '3',
    CancelAck        = '4',
    ModifyAck        = '5',
    PendingCancel    = '6',
    //    Stopped            = '7',
    //    Rejected           = '8',
    //    Suspended          = '9',
    PendingNew = 'A',
    //    Calculated         = 'B',
    //    Expired            = 'C',
    //    AcceptedForBidding = 'D',
    //    PendingReplace     = 'E',
    //    TradeCanceled      = 'H',
};

enum class CustomerOrFirmEnum : uint8_t
{
    Customer = 0,
    Firm     = 1
};

enum class TimeInforceEnum : char
{
    Day            = '0',
    GoodTillCancel = '1',
    FillAndKill    = '3',
    GoodTillDate   = '6'
};

enum class ManualOrderIndicatorEnum : char
{
    Manual    = 'Y',
    Automated = 'N'
};

enum class SelfMatchPreventionInstructionEnum : char
{
    CancelResting    = 'O',
    CancelAggressing = 'N'
};

enum class EncryptMethodEnum : uint8_t
{
    None = 0
};

enum class CancelOrdersOnDisconnectEnum : char
{
    AllAccountOrders        = 'Y',
    AllCurrentSessionOrders = 'S'
};

enum class MassCancelRequestTypeEnum : char
{
    CancelAllOrdersOnAllMarkets  = '7',
    CancelAllOrdersOnThisSession = '1'
};

} // namespace miye::trading::fix
