#pragma once
#include <cassert>
#include <stdint.h>

#include "../ftx_fix/field.hpp"
#include "../ftx_fix/type_utils.hpp"
#include "../ftx_ftx/string_const.hpp"

namespace miye::trading::fix
{

class Heartbeat
{
  public:
    using TestReqID_t = Field<112, 20>;

  public:
    Heartbeat() = default;

    template <typename... T>
    void setTestReqID(T&&... t) noexcept
    {
        testReqId_.set(std::forward<T>(t)...);
    }
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    void setTestReqID(T v) noexcept
    {
        testReqId_.set(toUnderlyingType(v));
    }

    void reset() { testReqId_.reset(); }

    uint32_t calcLen() noexcept
    {
        len_ = 0;

        if (testReqId_.isSet())
        {
            len_ += testReqId_.len();
        }

        return len_;
    }

    char* render(char* p)
    {
        uint32_t checkSum{};
        char* ptr(p);

        if (testReqId_.isSet())
        {

            std::memcpy(ptr, testReqId_.begin(), testReqId_.len());
            ptr += testReqId_.len();
            checkSum += testReqId_.getCheckSum();
        }

        checkSum_ = checkSum;
        return ptr;
    }

    uint32_t getCheckSum() const noexcept { return checkSum_; }
    uint32_t getLen() const noexcept { return len_; }

  private:
    TestReqID_t testReqId_;

  private:
    uint32_t checkSum_{};
    uint32_t len_{};
};

class TestRequest
{
  public:
    using TestReqID_t = Field<112, 20>;

  public:
    TestRequest() = default;

    template <typename... T>
    void setTestReqID(T&&... t) noexcept
    {
        testReqId_.set(std::forward<T>(t)...);
    }
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    void setTestReqID(T v) noexcept
    {
        testReqId_.set(toUnderlyingType(v));
    }

    void reset() { testReqId_.reset(); }

    uint32_t calcLen() noexcept
    {
        len_ = 0;
        len_ += testReqId_.len();

        return len_;
    }

    char* render(char* p)
    {
        uint32_t checkSum{};
        char* ptr(p);

        std::memcpy(ptr, testReqId_.begin(), testReqId_.len());
        ptr += testReqId_.len();
        checkSum += testReqId_.getCheckSum();

        checkSum_ = checkSum;
        return ptr;
    }

    uint32_t getCheckSum() const noexcept { return checkSum_; }
    uint32_t getLen() const noexcept { return len_; }

  private:
    TestReqID_t testReqId_;

  private:
    uint32_t checkSum_{};
    uint32_t len_{};
};

class Logon
{
  public:
    using Account_t                    = Field<1, 12>;
    using RawData_t                    = Field<96, 20>;
    using EncryptMethod_t              = Field<98, 1>;
    using Heartbtlnt_t                 = Field<108, 3>;
    using CancelOrderOnDisconnection_t = Field<8013, 1>;

  public:
    Logon() = default;

    template <typename... T>
    void setAccount(T&&... t) noexcept
    {
        account_.set(std::forward<T>(t)...);
    }
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    void setAccount(T v) noexcept
    {
        account_.set(toUnderlyingType(v));
    }

    template <typename... T>
    void setRawData(T&&... t) noexcept
    {
        rawData_.set(std::forward<T>(t)...);
    }
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    void setRawData(T v) noexcept
    {
        rawData_.set(toUnderlyingType(v));
    }

    template <typename... T>
    void setEncryptMethod(T&&... t) noexcept
    {
        encryptMethod_.set(std::forward<T>(t)...);
    }
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    void setEncryptMethod(T v) noexcept
    {
        encryptMethod_.set(toUnderlyingType(v));
    }

    template <typename... T>
    void setHeartbtlnt(T&&... t) noexcept
    {
        heartbtlnt_.set(std::forward<T>(t)...);
    }
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    void setHeartbtlnt(T v) noexcept
    {
        heartbtlnt_.set(toUnderlyingType(v));
    }

    void reset()
    {
        account_.reset();
        rawData_.reset();
        cod_.reset();
        encryptMethod_.reset();
        heartbtlnt_.reset();
    }

    uint32_t calcLen() noexcept
    {
        len_ = 0;
        if (account_.isSet())
        {
            len_ += account_.len();
        }
        if (rawData_.isSet())
        {
            len_ += rawData_.len();
        }
        if (cod_.isSet())
        {
            len_ += cod_.len();
        }
        len_ += encryptMethod_.len();
        len_ += heartbtlnt_.len();

        return len_;
    }

    char* render(char* p)
    {
        uint32_t checkSum{};
        char* ptr(p);

        if (rawData_.isSet())
        {
            std::memcpy(ptr, rawData_.begin(), rawData_.len());
            ptr += rawData_.len();
            checkSum += rawData_.getCheckSum();
        }
        if (cod_.isSet())
        {
            std::memcpy(ptr, cod_.begin(), cod_.len());
            ptr += cod_.len();
            checkSum += cod_.getCheckSum();
        }

        if (account_.isSet())
        {
            std::memcpy(ptr, account_.begin(), account_.len());
            ptr += account_.len();
        }

        std::memcpy(ptr, encryptMethod_.begin(), encryptMethod_.len());
        ptr += encryptMethod_.len();

        std::memcpy(ptr, heartbtlnt_.begin(), heartbtlnt_.len());
        ptr += heartbtlnt_.len();

        return ptr;
    }

    uint32_t getLen() const noexcept { return len_; }

  private:
    EncryptMethod_t encryptMethod_;
    Heartbtlnt_t heartbtlnt_;
    RawData_t rawData_;
    CancelOrderOnDisconnection_t cod_;
    Account_t account_;

  private:
    uint32_t len_{};
};

class Logout
{
  public:
    Logout() = default;

    //    template <typename... T>
    //    void setText(T&&... t) noexcept
    //    {
    //        text_.set(std::forward<T>(t)...);
    //    }
    //    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    //    void setText(T v) noexcept
    //    {
    //        text_.set(toUnderlyingType(v));
    //    }
    //
    //    template <typename... T>
    //    void setNextExpectedMsgSeqNum(T&&... t) noexcept
    //    {
    //        nextExpectedMsgSeqNum_.set(std::forward<T>(t)...);
    //    }
    //    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    //    void setNextExpectedMsgSeqNum(T v) noexcept
    //    {
    //        nextExpectedMsgSeqNum_.set(toUnderlyingType(v));
    //    }

    void reset() {}

    uint32_t calcLen() noexcept
    {
        len_ = 0;

        //        if (text_.isSet())
        //        {
        //            len_ += text_.len();
        //        }
        //
        //        if (nextExpectedMsgSeqNum_.isSet())
        //        {
        //            len_ += nextExpectedMsgSeqNum_.len();
        //        }

        return len_;
    }

    char* render(char* p) { return p; }

    uint32_t getLen() const noexcept { return len_; }

  private:
    uint32_t len_{};
};

class NewOrderSingle
{
  public:
    using ClOrdID_t           = Field<11, 20>;
    using HandInst_t          = Field<21, 1>;
    using ExecInst_t          = Field<18, 1>;
    using RejectOnPriceBand_t = Field<1368, 1>;
    using RejectAfterTs_t     = Field<1369, 10>;

    using OrderQty_t    = Field<38, 9>;
    using OrdType_t     = Field<40, 1>;
    using Price_t       = Field<44, 20>;
    using Side_t        = Field<54, 1>;
    using Symbol_t      = Field<55, 6>;
    using TimeInForce_t = Field<59, 1>;

  public:
    NewOrderSingle() = default;

    template <typename... T>
    void setClOrdID(T&&... t) noexcept
    {
        clOrdId_.set(std::forward<T>(t)...);
    }
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    void setClOrdID(T v) noexcept
    {
        clOrdId_.set(toUnderlyingType(v));
    }

    template <typename... T>
    void setHandInst(T&&... t) noexcept
    {
        handInst_.set(std::forward<T>(t)...);
    }
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    void setHandInst(T v) noexcept
    {
        handInst_.set(toUnderlyingType(v));
    }

    template <typename... T>
    void setOrderQty(T&&... t) noexcept
    {
        orderQty_.set(std::forward<T>(t)...);
    }
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    void setOrderQty(T v) noexcept
    {
        orderQty_.set(toUnderlyingType(v));
    }

    template <typename... T>
    void setOrdType(T&&... t) noexcept
    {
        ordType_.set(std::forward<T>(t)...);
    }
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    void setOrdType(T v) noexcept
    {
        ordType_.set(toUnderlyingType(v));
    }

    template <typename... T>
    void setPrice(T&&... t) noexcept
    {
        price_.set(std::forward<T>(t)...);
    }
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    void setPrice(T v) noexcept
    {
        price_.set(toUnderlyingType(v));
    }

    template <typename... T>
    void setSide(T&&... t) noexcept
    {
        side_.set(std::forward<T>(t)...);
    }
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    void setSide(T v) noexcept
    {
        side_.set(toUnderlyingType(v));
    }

    template <typename... T>
    void setSymbol(T&&... t) noexcept
    {
        symbol_.set(std::forward<T>(t)...);
    }
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    void setSymbol(T v) noexcept
    {
        symbol_.set(toUnderlyingType(v));
    }

    template <typename... T>
    void setTimeInForce(T&&... t) noexcept
    {
        timeInForce_.set(std::forward<T>(t)...);
    }
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    void setTimeInForce(T v) noexcept
    {
        timeInForce_.set(toUnderlyingType(v));
    }

    void reset()
    {
        handInst_.reset();
        clOrdId_.reset();
        symbol_.reset();
        ordType_.reset();
        orderQty_.reset();
        price_.reset();
        side_.reset();
        timeInForce_.reset();
        execInst_.reset();
        // transactTime_.reset();
        rejOnPriceBand_.reset();
        rejAfterTs_.reset();
    }

    uint32_t calcLen() noexcept
    {
        len_ = 0;
        len_ += clOrdId_.len();

        if (handInst_.isSet())
        {
            len_ += handInst_.len();
        }

        if (clOrdId_.isSet())
        {
            len_ += clOrdId_.len();
        }

        if (symbol_.isSet())
        {
            len_ += symbol_.len();
        }
        len_ += ordType_.len();

        if (orderQty_.isSet())
        {
            len_ += orderQty_.len();
        }
        if (price_.isSet())
        {
            len_ += price_.len();
        }
        if (side_.isSet())
        {
            len_ += side_.len();
        }

        if (price_.isSet())
        {
            len_ += price_.len();
        }
        len_ += timeInForce_.len();
        len_ += execInst_.len();

        if (rejOnPriceBand_.isSet())
        {
            len_ += rejOnPriceBand_.len();
        }
        if (rejAfterTs_.isSet())
        {
            len_ += rejAfterTs_.len();
        }

        return len_;
    }

    char* render(char* p)
    {
        uint32_t checkSum{};
        char* ptr(p);

        if (handInst_.isSet())
        {
            std::memcpy(ptr, handInst_.begin(), handInst_.len());
            ptr += handInst_.len();
        }

        std::memcpy(ptr, clOrdId_.begin(), clOrdId_.len());
        ptr += clOrdId_.len();
        std::memcpy(ptr, symbol_.begin(), symbol_.len());
        ptr += symbol_.len();

        std::memcpy(ptr, ordType_.begin(), ordType_.len());
        ptr += ordType_.len();

        if (orderQty_.isSet())
        {
            std::memcpy(ptr, orderQty_.begin(), orderQty_.len());
            ptr += orderQty_.len();
        }

        if (price_.isSet())
        {
            std::memcpy(ptr, price_.begin(), price_.len());
            ptr += price_.len();
        }

        std::memcpy(ptr, side_.begin(), side_.len());
        ptr += side_.len();

        if (timeInForce_.isSet())
        {
            std::memcpy(ptr, timeInForce_.begin(), timeInForce_.len());
            ptr += timeInForce_.len();
        }

        if (execInst_.isSet())
        {
            std::memcpy(ptr, execInst_.begin(), execInst_.len());
            ptr += execInst_.len();
        }

        if (rejOnPriceBand_.isSet())
        {
            std::memcpy(ptr, rejOnPriceBand_.begin(), rejOnPriceBand_.len());
            ptr += rejOnPriceBand_.len();
        }

        if (rejAfterTs_.isSet())
        {
            std::memcpy(ptr, rejAfterTs_.begin(), rejAfterTs_.len());
            ptr += rejAfterTs_.len();
        }

        return ptr;
    }

    uint32_t getLen() const noexcept { return len_; }

  private:
    HandInst_t handInst_;
    ClOrdID_t clOrdId_;
    Symbol_t symbol_;
    OrdType_t ordType_;
    OrderQty_t orderQty_;
    Price_t price_;
    Side_t side_;
    TimeInForce_t timeInForce_;
    ExecInst_t execInst_;
    RejectOnPriceBand_t rejOnPriceBand_;
    RejectAfterTs_t rejAfterTs_{};

  private:
    uint32_t len_{};
};

class OrderCancelRequest
{
  public:
    using OrderID_t     = Field<37, 17>;
    using OrigClOrdID_t = Field<41, 20>;

  public:
    OrderCancelRequest() = default;

    template <typename... T>
    void setOrderID(T&&... t) noexcept
    {
        orderId_.set(std::forward<T>(t)...);
    }
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    void setOrderID(T v) noexcept
    {
        orderId_.set(toUnderlyingType(v));
    }

    template <typename... T>
    void setOrigClOrdID(T&&... t) noexcept
    {
        origClOrdId_.set(std::forward<T>(t)...);
    }
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    void setOrigClOrdID(T v) noexcept
    {
        origClOrdId_.set(toUnderlyingType(v));
    }

    void reset()
    {
        orderId_.reset();
        origClOrdId_.reset();
    }

    uint32_t calcLen() noexcept
    {
        len_ = 0;

        if (orderId_.isSet())
        {
            len_ += orderId_.len();
        }
        if (origClOrdId_.isSet())
        {
            len_ += origClOrdId_.len();
        }

        return len_;
    }

    char* render(char* p)
    {
        uint32_t checkSum{};
        char* ptr(p);

        assert(!(orderId_.isSet() & origClOrdId_.isSet));
        /*
         * TODO: Only one of OrderID (37) and OrigClOrdID (41) should be provided.
         */
        if (orderId_.isSet())
        {

            std::memcpy(ptr, orderId_.begin(), orderId_.len());
            ptr += orderId_.len();
            checkSum += orderId_.getCheckSum();
        }

        if (origClOrdId_.isSet())
        {
            std::memcpy(ptr, origClOrdId_.begin(), origClOrdId_.len());
            ptr += origClOrdId_.len();
            checkSum += origClOrdId_.getCheckSum();
        }
        checkSum_ = checkSum;
        return ptr;
    }

    uint32_t getCheckSum() const noexcept { return checkSum_; }
    uint32_t getLen() const noexcept { return len_; }

  private:
    OrderID_t orderId_;
    OrigClOrdID_t origClOrdId_;

  private:
    uint32_t checkSum_{};
    uint32_t len_{};
};

class MassOrderCancelRequest
{
  public:
    using MassCancelRequestType_t = Field<530, 1>;
    using ClOrdID_t               = Field<11, 10>;
    using Symbol_t                = Field<55, 6>;

  public:
    MassOrderCancelRequest() = default;

    template <typename... T>
    void setMassCancelReqType(T&&... t) noexcept
    {
        massCancelReqType_.set(std::forward<T>(t)...);
    }
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    void setMassCancelReqType(T v) noexcept
    {
        massCancelReqType_.set(toUnderlyingType(v));
    }

    void reset()
    {
        massCancelReqType_.reset();
        clOrdId_.reset();
        symbol_.reset();
    }

    uint32_t calcLen() noexcept
    {
        len_ = 0;

        if (massCancelReqType_.isSet())
        {
            len_ += massCancelReqType_.len();
        }
        if (clOrdId_.isSet())
        {
            len_ += clOrdId_.len();
        }
        if (symbol_.isSet())
        {
            len_ += symbol_.len();
        }
        return len_;
    }

    char* render(char* p)
    {
        uint32_t checkSum{};
        char* ptr(p);

        if (massCancelReqType_.isSet())
        {
            std::memcpy(ptr, massCancelReqType_.begin(), massCancelReqType_.len());
            ptr += massCancelReqType_.len();
            checkSum += massCancelReqType_.getCheckSum();
        }

        if (clOrdId_.isSet())
        {
            std::memcpy(ptr, clOrdId_.begin(), clOrdId_.len());
            ptr += clOrdId_.len();
            checkSum += clOrdId_.getCheckSum();
        }
        if (symbol_.isSet())
        {
            std::memcpy(ptr, symbol_.begin(), symbol_.len());
            ptr += symbol_.len();
            checkSum += symbol_.getCheckSum();
        }
        checkSum_ = checkSum;
        return ptr;
    }

    uint32_t getCheckSum() const noexcept { return checkSum_; }
    uint32_t getLen() const noexcept { return len_; }

  private:
    MassCancelRequestType_t massCancelReqType_{};
    ClOrdID_t clOrdId_{}; // optional; client-assigned ID for mass order cancellation request
    Symbol_t symbol_{};   // optional; symbol name. This field is is required if. sample value:BTC-PERP

  private:
    uint32_t checkSum_{};
    uint32_t len_{};
};

class OrderStatusRequest
{
  public:
    using OrderID_t         = Field<37, 17>;
    using OrigClOrdID_t     = Field<41, 20>;
    using Symbol_t          = Field<55, 6>;
    using IncludeFillInfo_t = Field<20000, 1>;

  public:
    void reset()
    {
        orderId_.reset();
        origOrdId_.reset();
        symbol_.reset();
        includeFillInfo_.reset();
    }

    uint32_t calcLen() noexcept
    {
        len_ = 0;

        if (orderId_.isSet())
        {
            len_ += orderId_.len();
        }
        if (origOrdId_.isSet())
        {
            len_ += origOrdId_.len();
        }
        if (symbol_.isSet())
        {
            len_ += symbol_.len();
        }
        if (includeFillInfo_.isSet())
        {
            len_ += includeFillInfo_.len();
        }
        return len_;
    }

    char* render(char* p)
    {
        uint32_t checkSum{};
        char* ptr(p);

        if (orderId_.isSet())
        {
            std::memcpy(ptr, orderId_.begin(), orderId_.len());
            ptr += orderId_.len();
            checkSum += orderId_.getCheckSum();
        }

        if (origOrdId_.isSet())
        {
            std::memcpy(ptr, origOrdId_.begin(), origOrdId_.len());
            ptr += origOrdId_.len();
            checkSum += origOrdId_.getCheckSum();
        }
        if (symbol_.isSet())
        {
            std::memcpy(ptr, symbol_.begin(), symbol_.len());
            ptr += symbol_.len();
            checkSum += symbol_.getCheckSum();
        }
        if (includeFillInfo_.isSet())
        {
            std::memcpy(ptr, includeFillInfo_.begin(), includeFillInfo_.len());
            ptr += includeFillInfo_.len();
            checkSum += includeFillInfo_.getCheckSum();
        }
        checkSum_ = checkSum;
        return ptr;
    }

    uint32_t getCheckSum() const noexcept { return checkSum_; }
    uint32_t getLen() const noexcept { return len_; }

  private:
    OrderID_t orderId_{};       // optional; client-assigned ID for mass order cancellation request
    OrigClOrdID_t origOrdId_{}; // optional; client-assigned ID for mass order cancellation request
    Symbol_t symbol_{};         // optional; symbol name. This field is is required if. sample value:BTC-PERP
    IncludeFillInfo_t includeFillInfo_{};

  private:
    uint32_t checkSum_{};
    uint32_t len_{};
};

class ToFtxStandardHeader
{
  public:
    using BeginString_t            = Field<8, 7>;
    using BodyLength_t             = Field<9, 6>;
    using MsgType_t                = Field<35, 2>;
    using MsgSeqNum_t              = Field<34, 9>;
    using SenderCompID_t           = Field<49, 7>;
    using SendingTime_t            = Field<52, 21>;
    using LastMsgSeqNumProcessed_t = Field<369, 9>;
    using TargetCompID_t           = Field<56, 7>;
    using OrigSendingTime_t        = Field<122, 21>;

  public:
    ToFtxStandardHeader() = default;

    template <typename... T>
    void setBeginString(T&&... t) noexcept
    {
        beginString_.set(std::forward<T>(t)...);
    }
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    void setBeginString(T v) noexcept
    {
        beginString_.set(toUnderlyingType(v));
    }

    template <typename... T>
    void setBodyLength(T&&... t) noexcept
    {
        bodyLength_.set(std::forward<T>(t)...);
    }
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    void setBodyLength(T v) noexcept
    {
        bodyLength_.set(toUnderlyingType(v));
    }

    template <typename... T>
    void setMsgType(T&&... t) noexcept
    {
        msgType_.set(std::forward<T>(t)...);
    }
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    void setMsgType(T v) noexcept
    {
        msgType_.set(toUnderlyingType(v));
    }

    template <typename... T>
    void setMsgSeqNum(T&&... t) noexcept
    {
        msgSeqNum_.set(std::forward<T>(t)...);
    }
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    void setMsgSeqNum(T v) noexcept
    {
        msgSeqNum_.set(toUnderlyingType(v));
    }

    template <typename... T>
    void setSenderCompID(T&&... t) noexcept
    {
        senderCompId_.set(std::forward<T>(t)...);
    }
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    void setSenderCompID(T v) noexcept
    {
        senderCompId_.set(toUnderlyingType(v));
    }

    template <typename... T>
    void setSendingTime(T&&... t) noexcept
    {
        sendingTime_.set(std::forward<T>(t)...);
    }
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    void setSendingTime(T v) noexcept
    {
        sendingTime_.set(toUnderlyingType(v));
    }

    template <typename... T>
    void setLastMsgSeqNumProcessed(T&&... t) noexcept
    {
        lastMsgSeqNumProcessed_.set(std::forward<T>(t)...);
    }
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    void setLastMsgSeqNumProcessed(T v) noexcept
    {
        lastMsgSeqNumProcessed_.set(toUnderlyingType(v));
    }

    template <typename... T>
    void setTargetCompID(T&&... t) noexcept
    {
        targetCompId_.set(std::forward<T>(t)...);
    }
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    void setTargetCompID(T v) noexcept
    {
        targetCompId_.set(toUnderlyingType(v));
    }

    template <typename... T>
    void setOrigSendingTime(T&&... t) noexcept
    {
        origSendingTime_.set(std::forward<T>(t)...);
    }
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    void setOrigSendingTime(T v) noexcept
    {
        origSendingTime_.set(toUnderlyingType(v));
    }

    void reset()
    {
        beginString_.reset();
        bodyLength_.reset();
        msgType_.reset();
        msgSeqNum_.reset();
        senderCompId_.reset();
        sendingTime_.reset();
        lastMsgSeqNumProcessed_.reset();
        targetCompId_.reset();
        origSendingTime_.reset();
    }

    uint32_t calcLen() noexcept
    {
        len_ = 0;
        len_ += msgType_.len();
        len_ += msgSeqNum_.len();

        len_ += senderCompId_.len();

        len_ += sendingTime_.len();

        if (lastMsgSeqNumProcessed_.isSet())
        {
            len_ += lastMsgSeqNumProcessed_.len();
        }
        len_ += targetCompId_.len();

        if (origSendingTime_.isSet())
        {
            len_ += origSendingTime_.len();
        }
        return len_;
    }

    char* render(char* p)
    {
        uint32_t checkSum{};
        char* ptr(p);

        std::memcpy(ptr, beginString_.begin(), beginString_.len());
        ptr += beginString_.len();
        checkSum += beginString_.getCheckSum();

        std::memcpy(ptr, bodyLength_.begin(), bodyLength_.len());
        ptr += bodyLength_.len();
        checkSum += bodyLength_.getCheckSum();

        std::memcpy(ptr, msgType_.begin(), msgType_.len());
        ptr += msgType_.len();
        checkSum += msgType_.getCheckSum();

        std::memcpy(ptr, msgSeqNum_.begin(), msgSeqNum_.len());
        ptr += msgSeqNum_.len();
        checkSum += msgSeqNum_.getCheckSum();

        std::memcpy(ptr, senderCompId_.begin(), senderCompId_.len());
        ptr += senderCompId_.len();
        checkSum += senderCompId_.getCheckSum();

        std::memcpy(ptr, sendingTime_.begin(), sendingTime_.len());
        ptr += sendingTime_.len();
        checkSum += sendingTime_.getCheckSum();

        if (lastMsgSeqNumProcessed_.isSet())
        {

            std::memcpy(ptr, lastMsgSeqNumProcessed_.begin(), lastMsgSeqNumProcessed_.len());
            ptr += lastMsgSeqNumProcessed_.len();
            checkSum += lastMsgSeqNumProcessed_.getCheckSum();
        }

        std::memcpy(ptr, targetCompId_.begin(), targetCompId_.len());
        ptr += targetCompId_.len();
        checkSum += targetCompId_.getCheckSum();

        if (origSendingTime_.isSet())
        {

            std::memcpy(ptr, origSendingTime_.begin(), origSendingTime_.len());
            ptr += origSendingTime_.len();
            checkSum += origSendingTime_.getCheckSum();
        }

        checkSum_ = checkSum;
        return ptr;
    }

    uint32_t getCheckSum() const noexcept { return checkSum_; }
    uint32_t getLen() const noexcept { return len_; }

  private:
    BeginString_t beginString_;
    BodyLength_t bodyLength_;
    MsgType_t msgType_;
    MsgSeqNum_t msgSeqNum_;
    SenderCompID_t senderCompId_;
    SendingTime_t sendingTime_;
    LastMsgSeqNumProcessed_t lastMsgSeqNumProcessed_;
    TargetCompID_t targetCompId_;
    OrigSendingTime_t origSendingTime_;

  private:
    uint32_t checkSum_{};
    uint32_t len_{};
};
} // namespace miye::trading::fix
