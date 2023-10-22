#pragma once
#include <array>
#include <cstdint>

#include "../ftx_fix/field_ptr.hpp"
#include "../ftx_fix/fix_enum.hpp"
#include "../ftx_fix/fix_istream.hpp"

namespace miye::trading::fix
{

struct ExecutionReportReader
{
    using ClOrdID_t            = FieldPtr<11>;
    using OrderID_t            = FieldPtr<37>;
    using ExecID_t             = FieldPtr<17>;
    using Symbol_t             = FieldPtr<55>;
    using Side_t               = FieldPtr<54>;
    using OrderQty_t           = FieldPtr<38>;
    using Price_t              = FieldPtr<44>;
    using ExecType_t           = FieldPtr<150>;
    using OrdStatus_t          = FieldPtr<39>;
    using CumQty_t             = FieldPtr<14>;
    using LeavesQty_t          = FieldPtr<151>;
    using CxlQty_t             = FieldPtr<84>;
    using TransactTime_t       = FieldPtr<60>;
    using LastPx_t             = FieldPtr<31>;
    using LastQty_t            = FieldPtr<32>;
    using AggressorIndicator_t = FieldPtr<1057>;
    using FillTradeID_t        = FieldPtr<1366>;
    using AvgPx_t              = FieldPtr<6>;
    using Commission_t         = FieldPtr<12>;
    using CommType_t           = FieldPtr<13>;
    using OrdRejReason_t       = FieldPtr<103>;
    using Text_t               = FieldPtr<58>;
    using Liquidation_t        = FieldPtr<5000>;

    ExecutionReportReader() = default;

    FixIstream& read(FixIstream& fs)
    {
        while (fs.hasNext())
        {
            auto const field = fs.getCurrent();
            switch (std::get<0>(field))
            {
            case AvgPx_t::Id():
                avgPx.setPointers(field);
                break;
            case ClOrdID_t::Id():
                clOrdId.setPointers(field);
                break;
            case CumQty_t::Id():
                cumQty.setPointers(field);
                break;
            case ExecID_t::Id():
                execId.setPointers(field);
                break;
            case LastPx_t::Id():
                lastPx.setPointers(field);
                break;
            case LastQty_t::Id():
                lastQty.setPointers(field);
                break;
            case OrderID_t::Id():
                orderId.setPointers(field);
                break;
            case OrderQty_t::Id():
                orderQty.setPointers(field);
                break;
            case OrdStatus_t::Id():
                ordStatus.setPointers(field);
                break;
            case Price_t::Id():
                price.setPointers(field);
                break;
            case Side_t::Id():
                side.setPointers(field);
                break;
            case Symbol_t::Id():
                symbol.setPointers(field);
                break;
            case Text_t::Id():
                text.setPointers(field);
                break;
            case TransactTime_t::Id():
                transactTime.setPointers(field);
                break;
            case OrdRejReason_t::Id():
                ordRejReason.setPointers(field);
                break;
            case ExecType_t::Id():
                execType.setPointers(field);
                break;
            case LeavesQty_t::Id():
                leavesQty.setPointers(field);
                break;
            case CxlQty_t::Id():
                cxlQty.setPointers(field);
                break;
            case AggressorIndicator_t::Id():
                aggressorIndicator.setPointers(field);
                break;
            case Liquidation_t::Id():
                liquidation.setPointers(field);
                break;
            case Commission_t::Id():
                commission.setPointers(field);
                break;
            case CommType_t::Id():
                commType.setPointers(field);
                break;
            case FillTradeID_t::Id():
                fillTradeId.setPointers(field);
                break;
            default:
                return fs;
            }
        }

        return fs;
    }

    void reset()
    {
        avgPx.reset();
        commission.reset();
        commType.reset();
        clOrdId.reset();
        cumQty.reset();
        execId.reset();
        lastPx.reset();
        lastQty.reset();
        orderId.reset();
        orderQty.reset();
        ordStatus.reset();
        price.reset();
        side.reset();
        symbol.reset();
        text.reset();
        transactTime.reset();
        ordRejReason.reset();
        execType.reset();
        leavesQty.reset();
        cxlQty.reset();
        aggressorIndicator.reset();
        fillTradeId.reset();
        liquidation.reset();
    }

    AvgPx_t avgPx;
    Commission_t commission;
    CommType_t commType;
    ClOrdID_t clOrdId;
    CumQty_t cumQty;
    ExecID_t execId;
    LastPx_t lastPx;
    LastQty_t lastQty;
    OrderID_t orderId;
    OrderQty_t orderQty;
    OrdStatus_t ordStatus;
    Price_t price;
    Side_t side;
    Symbol_t symbol;
    Text_t text;
    TransactTime_t transactTime;
    OrdRejReason_t ordRejReason;
    ExecType_t execType;
    LeavesQty_t leavesQty;
    CxlQty_t cxlQty;
    AggressorIndicator_t aggressorIndicator;
    FillTradeID_t fillTradeId;
    Liquidation_t liquidation;
};

struct HeartbeatReader
{
    using TestReqID_t = FieldPtr<112>;

    HeartbeatReader() = default;

    FixIstream& read(FixIstream& fs)
    {
        while (fs.hasNext())
        {
            auto const field = fs.getCurrent();
            switch (std::get<0>(field))
            {
            case TestReqID_t::Id():
                testReqId.setPointers(field);
                break;
            default:
                return fs;
            }
        }

        return fs;
    }

    void reset() { testReqId.reset(); }

    TestReqID_t testReqId;
};

struct LogonReader
{
    using EncryptMethod_t = FieldPtr<98>;
    using HeartBtlnt_t    = FieldPtr<108>;

    LogonReader() = default;

    FixIstream& read(FixIstream& fs)
    {
        while (fs.hasNext())
        {
            auto const field = fs.getCurrent();
            switch (std::get<0>(field))
            {
            case EncryptMethod_t::Id():
                encryptMethod.setPointers(field);
                break;
            case HeartBtlnt_t::Id():
                heartBtlnt.setPointers(field);
                break;
            default:
                return fs;
            }
        }

        return fs;
    }

    void reset()
    {
        encryptMethod.reset();
        heartBtlnt.reset();
    }

    EncryptMethod_t encryptMethod;
    HeartBtlnt_t heartBtlnt;
};

struct LogoutReader
{
    using Text_t = FieldPtr<58>;

    LogoutReader() = default;

    FixIstream& read(FixIstream& fs)
    {
        while (fs.hasNext())
        {
        }

        return fs;
    }

    void reset() {}
};

struct NewOrderCrossReader
{
    using HandInst_t             = FieldPtr<21>;
    using SecurityIDsource_t     = FieldPtr<22>;
    using OrdType_t              = FieldPtr<40>;
    using Price_t                = FieldPtr<44>;
    using SecurityID_t           = FieldPtr<48>;
    using Symbol_t               = FieldPtr<55>;
    using TransactTime_t         = FieldPtr<60>;
    using SecurityDesc_t         = FieldPtr<107>;
    using SecurityType_t         = FieldPtr<167>;
    using TransBkdTime_t         = FieldPtr<483>;
    using CrossID_t              = FieldPtr<548>;
    using CrossType_t            = FieldPtr<549>;
    using CrossPrioritization_t  = FieldPtr<550>;
    using NoSides_t              = FieldPtr<552>;
    using ManualOrderIndicator_t = FieldPtr<1028>;

    NewOrderCrossReader() = default;

    FixIstream& read(FixIstream& fs)
    {
        while (fs.hasNext())
        {
            auto const field = fs.getCurrent();
            switch (std::get<0>(field))
            {
            case HandInst_t::Id():
                handInst.setPointers(field);
                break;
            case SecurityIDsource_t::Id():
                securityIDsource.setPointers(field);
                break;
            case OrdType_t::Id():
                ordType.setPointers(field);
                break;
            case Price_t::Id():
                price.setPointers(field);
                break;
            case SecurityID_t::Id():
                securityId.setPointers(field);
                break;
            case Symbol_t::Id():
                symbol.setPointers(field);
                break;
            case TransactTime_t::Id():
                transactTime.setPointers(field);
                break;
            case SecurityDesc_t::Id():
                securityDesc.setPointers(field);
                break;
            case SecurityType_t::Id():
                securityType.setPointers(field);
                break;
            case TransBkdTime_t::Id():
                transBkdTime.setPointers(field);
                break;
            case CrossID_t::Id():
                crossId.setPointers(field);
                break;
            case CrossType_t::Id():
                crossType.setPointers(field);
                break;
            case CrossPrioritization_t::Id():
                crossPrioritization.setPointers(field);
                break;
            case NoSides_t::Id():
                noSides.setPointers(field);
                break;
            case ManualOrderIndicator_t::Id():
                manualOrderIndicator.setPointers(field);
                break;
            default:
                return fs;
            }
        }

        return fs;
    }

    void reset()
    {
        handInst.reset();
        securityIDsource.reset();
        ordType.reset();
        price.reset();
        securityId.reset();
        symbol.reset();
        transactTime.reset();
        securityDesc.reset();
        securityType.reset();
        transBkdTime.reset();
        crossId.reset();
        crossType.reset();
        crossPrioritization.reset();
        noSides.reset();
        manualOrderIndicator.reset();
    }

    HandInst_t handInst;
    SecurityIDsource_t securityIDsource;
    OrdType_t ordType;
    Price_t price;
    SecurityID_t securityId;
    Symbol_t symbol;
    TransactTime_t transactTime;
    SecurityDesc_t securityDesc;
    SecurityType_t securityType;
    TransBkdTime_t transBkdTime;
    CrossID_t crossId;
    CrossType_t crossType;
    CrossPrioritization_t crossPrioritization;
    NoSides_t noSides;
    ManualOrderIndicator_t manualOrderIndicator;
};

struct SessionLevelRejectReader
{
    using RefSeqNum_t             = FieldPtr<45>;
    using Text_t                  = FieldPtr<58>;
    using RefTagID_t              = FieldPtr<371>;
    using SessionRejectReason_t   = FieldPtr<373>;
    using ManualOrderIndicator_t  = FieldPtr<1028>;
    using CustOrderHandlingInst_t = FieldPtr<1031>;

    SessionLevelRejectReader() = default;

    FixIstream& read(FixIstream& fs)
    {
        while (fs.hasNext())
        {
            auto const field = fs.getCurrent();
            switch (std::get<0>(field))
            {
            case RefSeqNum_t::Id():
                refSeqNum.setPointers(field);
                break;
            case Text_t::Id():
                text.setPointers(field);
                break;
            case RefTagID_t::Id():
                refTagId.setPointers(field);
                break;
            case SessionRejectReason_t::Id():
                sessionRejectReason.setPointers(field);
                break;
            case ManualOrderIndicator_t::Id():
                manualOrderIndicator.setPointers(field);
                break;
            case CustOrderHandlingInst_t::Id():
                custOrderHandlingInst.setPointers(field);
                break;
            default:
                return fs;
            }
        }

        return fs;
    }

    void reset()
    {
        refSeqNum.reset();
        text.reset();
        refTagId.reset();
        sessionRejectReason.reset();
        manualOrderIndicator.reset();
        custOrderHandlingInst.reset();
    }

    RefSeqNum_t refSeqNum;
    Text_t text;
    RefTagID_t refTagId;
    SessionRejectReason_t sessionRejectReason;
    ManualOrderIndicator_t manualOrderIndicator;
    CustOrderHandlingInst_t custOrderHandlingInst;
};

struct OrderCancelRejectReader
{
    using ClOrdID_t          = FieldPtr<11>;
    using OrderID_t          = FieldPtr<37>;
    using OrgiClOrdID_t      = FieldPtr<41>;
    using OrdStatus_t        = FieldPtr<39>;
    using CXlRejReason_t     = FieldPtr<102>;
    using CxlRejResponseTo_t = FieldPtr<434>;

    OrderCancelRejectReader() = default;

    FixIstream& read(FixIstream& fs)
    {
        while (fs.hasNext())
        {
            auto const field = fs.getCurrent();
            switch (std::get<0>(field))
            {
            case ClOrdID_t::Id():
                clOrdId.setPointers(field);
                break;
                break;
            case OrderID_t::Id():
                orderId.setPointers(field);
                break;
            case OrdStatus_t::Id():
                ordStatus.setPointers(field);
                break;
            case OrgiClOrdID_t::Id():
                orgiClOrdId.setPointers(field);
                break;
            case CxlRejResponseTo_t::Id():
                cxlRejResponseTo.setPointers(field);
                break;
            case CXlRejReason_t::Id():
                cXlRejReason.setPointers(field);
                break;

            default:
                return fs;
            }
        }

        return fs;
    }

    void reset()
    {
        clOrdId.reset();
        orderId.reset();
        ordStatus.reset();
        orgiClOrdId.reset();
        cXlRejReason.reset();
        cxlRejResponseTo.reset();
    }

    ClOrdID_t clOrdId;
    OrderID_t orderId;
    OrdStatus_t ordStatus;
    OrgiClOrdID_t orgiClOrdId;
    CXlRejReason_t cXlRejReason;
    CxlRejResponseTo_t cxlRejResponseTo;
};

struct OrderStatusRequestReader
{
    using ClOrdID_t              = FieldPtr<11>;
    using OrderID_t              = FieldPtr<37>;
    using Side_t                 = FieldPtr<54>;
    using Symbol_t               = FieldPtr<55>;
    using TransactTime_t         = FieldPtr<60>;
    using SecurityDesc_t         = FieldPtr<107>;
    using SecurityType_t         = FieldPtr<167>;
    using ManualOrderIndicator_t = FieldPtr<1028>;
    using CorrelationClOrdID_t   = FieldPtr<9717>;

    OrderStatusRequestReader() = default;

    FixIstream& read(FixIstream& fs)
    {
        while (fs.hasNext())
        {
            auto const field = fs.getCurrent();
            switch (std::get<0>(field))
            {
            case ClOrdID_t::Id():
                clOrdId.setPointers(field);
                break;
            case OrderID_t::Id():
                orderId.setPointers(field);
                break;
            case Side_t::Id():
                side.setPointers(field);
                break;
            case Symbol_t::Id():
                symbol.setPointers(field);
                break;
            case TransactTime_t::Id():
                transactTime.setPointers(field);
                break;
            case SecurityDesc_t::Id():
                securityDesc.setPointers(field);
                break;
            case SecurityType_t::Id():
                securityType.setPointers(field);
                break;
            case ManualOrderIndicator_t::Id():
                manualOrderIndicator.setPointers(field);
                break;
            case CorrelationClOrdID_t::Id():
                correlationClOrdId.setPointers(field);
                break;
            default:
                return fs;
            }
        }

        return fs;
    }

    void reset()
    {
        clOrdId.reset();
        orderId.reset();
        side.reset();
        symbol.reset();
        transactTime.reset();
        securityDesc.reset();
        securityType.reset();
        manualOrderIndicator.reset();
        correlationClOrdId.reset();
    }

    ClOrdID_t clOrdId;
    OrderID_t orderId;
    Side_t side;
    Symbol_t symbol;
    TransactTime_t transactTime;
    SecurityDesc_t securityDesc;
    SecurityType_t securityType;
    ManualOrderIndicator_t manualOrderIndicator;
    CorrelationClOrdID_t correlationClOrdId;
};

struct OrderStatusRequestAcknowledgementReader
{
    using Account_t              = FieldPtr<1>;
    using AvgPrice_t             = FieldPtr<6>;
    using ClOrdID_t              = FieldPtr<11>;
    using CumQty_t               = FieldPtr<14>;
    using ExecID_t               = FieldPtr<17>;
    using ExecTransType_t        = FieldPtr<20>;
    using OrderID_t              = FieldPtr<37>;
    using OrderQty_t             = FieldPtr<38>;
    using OrdStatus_t            = FieldPtr<39>;
    using OrdType_t              = FieldPtr<40>;
    using OrigClOrdID_t          = FieldPtr<41>;
    using Price_t                = FieldPtr<44>;
    using SecurityID_t           = FieldPtr<48>;
    using Side_t                 = FieldPtr<54>;
    using Symbol_t               = FieldPtr<55>;
    using Text_t                 = FieldPtr<58>;
    using TimeInForce_t          = FieldPtr<59>;
    using TransactTime_t         = FieldPtr<60>;
    using TradeDate_t            = FieldPtr<75>;
    using StopPx_t               = FieldPtr<99>;
    using SecurityDesc_t         = FieldPtr<107>;
    using MinQty_t               = FieldPtr<110>;
    using ExecType_t             = FieldPtr<150>;
    using LeavesQty_t            = FieldPtr<151>;
    using SecurityType_t         = FieldPtr<167>;
    using MaxShow_t              = FieldPtr<210>;
    using RefMsgType_t           = FieldPtr<372>;
    using BusinessRejectReason_t = FieldPtr<380>;
    using ExpireDate_t           = FieldPtr<432>;
    using CrossID_t              = FieldPtr<548>;
    using CrossType_t            = FieldPtr<549>;
    using MassStatusReqID_t      = FieldPtr<584>;
    using LastRptRequested_t     = FieldPtr<912>;
    using HostCrossID_t          = FieldPtr<961>;
    using ManualOrderIndicator_t = FieldPtr<1028>;
    using RequestTime_t          = FieldPtr<5979>;
    using CorrelationClOrdID_t   = FieldPtr<9717>;

    OrderStatusRequestAcknowledgementReader() = default;

    FixIstream& read(FixIstream& fs)
    {
        while (fs.hasNext())
        {
            auto const field = fs.getCurrent();
            switch (std::get<0>(field))
            {
            case Account_t::Id():
                account.setPointers(field);
                break;
            case AvgPrice_t::Id():
                avgPrice.setPointers(field);
                break;
            case ClOrdID_t::Id():
                clOrdId.setPointers(field);
                break;
            case CumQty_t::Id():
                cumQty.setPointers(field);
                break;
            case ExecID_t::Id():
                execId.setPointers(field);
                break;
            case ExecTransType_t::Id():
                execTransType.setPointers(field);
                break;
            case OrderID_t::Id():
                orderId.setPointers(field);
                break;
            case OrderQty_t::Id():
                orderQty.setPointers(field);
                break;
            case OrdStatus_t::Id():
                ordStatus.setPointers(field);
                break;
            case OrdType_t::Id():
                ordType.setPointers(field);
                break;
            case OrigClOrdID_t::Id():
                origClOrdId.setPointers(field);
                break;
            case Price_t::Id():
                price.setPointers(field);
                break;
            case SecurityID_t::Id():
                securityId.setPointers(field);
                break;
            case Side_t::Id():
                side.setPointers(field);
                break;
            case Symbol_t::Id():
                symbol.setPointers(field);
                break;
            case Text_t::Id():
                text.setPointers(field);
                break;
            case TimeInForce_t::Id():
                timeInForce.setPointers(field);
                break;
            case TransactTime_t::Id():
                transactTime.setPointers(field);
                break;
            case TradeDate_t::Id():
                tradeDate.setPointers(field);
                break;
            case StopPx_t::Id():
                stopPx.setPointers(field);
                break;
            case SecurityDesc_t::Id():
                securityDesc.setPointers(field);
                break;
            case MinQty_t::Id():
                minQty.setPointers(field);
                break;
            case ExecType_t::Id():
                execType.setPointers(field);
                break;
            case LeavesQty_t::Id():
                leavesQty.setPointers(field);
                break;
            case SecurityType_t::Id():
                securityType.setPointers(field);
                break;
            case MaxShow_t::Id():
                maxShow.setPointers(field);
                break;
            case RefMsgType_t::Id():
                refMsgType.setPointers(field);
                break;
            case BusinessRejectReason_t::Id():
                businessRejectReason.setPointers(field);
                break;
            case ExpireDate_t::Id():
                expireDate.setPointers(field);
                break;
            case CrossID_t::Id():
                crossId.setPointers(field);
                break;
            case CrossType_t::Id():
                crossType.setPointers(field);
                break;
            case MassStatusReqID_t::Id():
                massStatusReqId.setPointers(field);
                break;
            case LastRptRequested_t::Id():
                lastRptRequested.setPointers(field);
                break;
            case HostCrossID_t::Id():
                hostCrossId.setPointers(field);
                break;
            case ManualOrderIndicator_t::Id():
                manualOrderIndicator.setPointers(field);
                break;
            case RequestTime_t::Id():
                requestTime.setPointers(field);
                break;
            case CorrelationClOrdID_t::Id():
                correlationClOrdId.setPointers(field);
                break;
            default:
                return fs;
            }
        }

        return fs;
    }

    void reset()
    {
        account.reset();
        avgPrice.reset();
        clOrdId.reset();
        cumQty.reset();
        execId.reset();
        execTransType.reset();
        orderId.reset();
        orderQty.reset();
        ordStatus.reset();
        ordType.reset();
        origClOrdId.reset();
        price.reset();
        securityId.reset();
        side.reset();
        symbol.reset();
        text.reset();
        timeInForce.reset();
        transactTime.reset();
        tradeDate.reset();
        stopPx.reset();
        securityDesc.reset();
        minQty.reset();
        execType.reset();
        leavesQty.reset();
        securityType.reset();
        maxShow.reset();
        refMsgType.reset();
        businessRejectReason.reset();
        expireDate.reset();
        crossId.reset();
        crossType.reset();
        massStatusReqId.reset();
        lastRptRequested.reset();
        hostCrossId.reset();
        manualOrderIndicator.reset();
        requestTime.reset();
        correlationClOrdId.reset();
    }

    Account_t account;
    AvgPrice_t avgPrice;
    ClOrdID_t clOrdId;
    CumQty_t cumQty;
    ExecID_t execId;
    ExecTransType_t execTransType;
    OrderID_t orderId;
    OrderQty_t orderQty;
    OrdStatus_t ordStatus;
    OrdType_t ordType;
    OrigClOrdID_t origClOrdId;
    Price_t price;
    SecurityID_t securityId;
    Side_t side;
    Symbol_t symbol;
    Text_t text;
    TimeInForce_t timeInForce;
    TransactTime_t transactTime;
    TradeDate_t tradeDate;
    StopPx_t stopPx;
    SecurityDesc_t securityDesc;
    MinQty_t minQty;
    ExecType_t execType;
    LeavesQty_t leavesQty;
    SecurityType_t securityType;
    MaxShow_t maxShow;
    RefMsgType_t refMsgType;
    BusinessRejectReason_t businessRejectReason;
    ExpireDate_t expireDate;
    CrossID_t crossId;
    CrossType_t crossType;
    MassStatusReqID_t massStatusReqId;
    LastRptRequested_t lastRptRequested;
    HostCrossID_t hostCrossId;
    ManualOrderIndicator_t manualOrderIndicator;
    RequestTime_t requestTime;
    CorrelationClOrdID_t correlationClOrdId;
};

struct ResendRequestReader
{
    using BeginSeqNo_t = FieldPtr<7>;
    using EndSeqNo_t   = FieldPtr<16>;

    ResendRequestReader() = default;

    FixIstream& read(FixIstream& fs)
    {
        while (fs.hasNext())
        {
            auto const field = fs.getCurrent();
            switch (std::get<0>(field))
            {
            case BeginSeqNo_t::Id():
                beginSeqNo.setPointers(field);
                break;
            case EndSeqNo_t::Id():
                endSeqNo.setPointers(field);
                break;
            default:
                return fs;
            }
        }

        return fs;
    }

    void reset()
    {
        beginSeqNo.reset();
        endSeqNo.reset();
    }

    BeginSeqNo_t beginSeqNo;
    EndSeqNo_t endSeqNo;
};

struct StandardHeaderReader
{
    using BeginString_t  = FieldPtr<8>;
    using BodyLength_t   = FieldPtr<9>;
    using MsgType_t      = FieldPtr<35>;
    using SenderCompID_t = FieldPtr<49>;
    using TargetCompID_t = FieldPtr<56>;
    using MsgSeqNum_t    = FieldPtr<34>;

    StandardHeaderReader() = default;

    FixIstream& read(FixIstream& fs)
    {
        while (fs.hasNext())
        {
            auto const field = fs.getCurrent();
            switch (std::get<0>(field))
            {
            case BeginString_t::Id():
                beginString.setPointers(field);
                break;
            case BodyLength_t::Id():
                bodyLength.setPointers(field);
                break;
                break;
            case MsgType_t::Id():
                msgType.setPointers(field);
                break;
            case SenderCompID_t::Id():
                senderCompId.setPointers(field);
                break;
            case TargetCompID_t::Id():
                targetCompId.setPointers(field);
                break;
            default:
                return fs;
            }
        }

        return fs;
    }

    void reset()
    {
        beginString.reset();
        bodyLength.reset();
        msgType.reset();
        senderCompId.reset();
        targetCompId.reset();
    }

    BeginString_t beginString;
    BodyLength_t bodyLength;
    MsgType_t msgType;
    MsgSeqNum_t msgSeqNum;
    SenderCompID_t senderCompId;
    TargetCompID_t targetCompId;
};

struct TestRequestReader
{
    using TestReqID_t = FieldPtr<112>;

    TestRequestReader() = default;

    FixIstream& read(FixIstream& fs)
    {
        while (fs.hasNext())
        {
            auto const field = fs.getCurrent();
            switch (std::get<0>(field))
            {
            case TestReqID_t::Id():
                testReqId.setPointers(field);
                break;
            default:
                return fs;
            }
        }

        return fs;
    }

    void reset() { testReqId.reset(); }

    TestReqID_t testReqId;
};

} // namespace miye::trading::fix
