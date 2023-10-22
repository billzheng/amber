#pragma once

#include <cassert>
#include <chrono>
#include <stdint.h>
#include <string.h>
#include <string>
#include <sys/time.h>

#include "../ftx_fix/fix_config.hpp"
#include "../ftx_fix/fix_enum.hpp"
#include "../ftx_fix/fix_sequence_id_file_parser.hpp"
#include "../ftx_fix/fix_utils.hpp"
#include "../ftx_fix/ftx_fix_reader.hpp"
#include "../ftx_fix/ftx_fix_writer.hpp"
#include "../ftx_fix/seq_num_generator.hpp"
#include "../ftx_ftx/type_utils.hpp"

namespace miye::trading::fix::ftx
{

using order_id_t   = uint64_t;
using clorder_id_t = uint64_t;
using price_t      = double;
using quantity_t   = uint32_t;

struct VanilaRiskCheck
{
    VanilaRiskCheck() = default;

    double maxPrice{};
    double minPrice{};
    double maxQty{};
    std::chrono::microseconds expireTime{};
};

class FixSession
{
  public:
    constexpr static const uint32_t InBufferSize  = SUPERPAGE_SIZE;
    constexpr static const uint32_t OutBufferSize = 1024;

    using CheckSum_t = Field<10, 3>;

  public:
    explicit FixSession(const fix_config_t& config, const std::string& segment, const std::string& sequence_file,
                        char* out_buffer)
        : outBuffer_(out_buffer), config_(config), segment_(segment), sequence_file(sequence_file)
    {
        assert(!sequence_file.empty());
        // auto const seq_pair = fix::utils::get_seq_num_id(sequence_file);

        // fix_in_buf = std::make_shared<fundamentals::reassembly_buffer<InBufferSize>>();

        //        // set expected_exchange_seq to 1 so we accept what ever exchange sequence at logon
        //        LOG_INFO("[fix session] segment: " << segment << ", loading sequence file: " << sequence_file << ",
        //        local seq: "
        //                                           << seq_pair.first << ", exchange seq: " << seq_pair.second);
        // init(seq_pair.first, 0);
    }

    ~FixSession();

  public:
    /*---------------------------------------------------------------------------------
    * messages to FIX exchange
    ---------------------------------------------------------------------------------*/

    size_t logon();
    size_t logout();
    size_t sendHeartBeat();
    size_t testRequest();
    size_t resendRequest();

    size_t placeOrder(OrderTypeEnum orderType, TimeInforceEnum tif, uint64_t clOrdId, SideEnum side, price_t price,
                      uint32_t precision, quantity_t qty, const std::string& symbol, const std::string& securityDesc,
                      const std::string& timestamp)
    {
        outHeader_.setMsgSeqNum(seqNum_.getNextSeqNum());
        outHeader_.setMsgType(MsgTypeEnum::OrderSingle);

        newOrder_.setTimeInForce(tif);
        newOrder_.setOrdType(orderType);

        // newOrder_.setTransactTime(timestamp);
        assert(!timestamp.empty());
        // newOrder_.setSymbol(symbol);
        newOrder_.setSide(side);
        if (orderType != OrderTypeEnum::MarketLimitOrder)
        {
            newOrder_.setPrice(price, precision);
        }
        newOrder_.setOrderQty(qty);
        newOrder_.setClOrdID(clOrdId);

        outHeader_.setBodyLength(outHeader_.calcLen() + newOrder_.calcLen());
        return render(outHeader_, newOrder_);
    }

    size_t placeOrder(OrderTypeEnum orderType, TimeInforceEnum tif, uint64_t clOrdId, SideEnum side, uint64_t price,
                      quantity_t qty, const std::string& symbol, const std::string& securityDesc,
                      const std::string& timestamp)
    {
        outHeader_.setMsgSeqNum(seqNum_.getNextSeqNum());
        outHeader_.setMsgType(MsgTypeEnum::OrderSingle);
        // outHeader_.setSendingTime(timestamp);

        newOrder_.setTimeInForce(tif);
        newOrder_.setOrdType(orderType);

        assert(!timestamp.empty());
        newOrder_.setSide(side);
        if (orderType != OrderTypeEnum::MarketLimitOrder)
        {
            newOrder_.setPrice(price);
        }
        newOrder_.setOrderQty(qty);
        newOrder_.setClOrdID(clOrdId);

        outHeader_.setBodyLength(outHeader_.calcLen() + newOrder_.calcLen());
        return render(outHeader_, newOrder_);
    }

    size_t cancelOrder(SideEnum side, order_id_t orderId, clorder_id_t clOrdId, const std::string& symbol,
                       const std::string& securityDesc, const std::string& timestamp)
    {
        outHeader_.setMsgSeqNum(seqNum_.getNextSeqNum());
        outHeader_.setMsgType(MsgTypeEnum::OrderCancelRequest);
        // TODO
        // outHeader_.setSendingTime(timestamp);

        assert(!timestamp.empty());
        orderCancelRequest_.setOrderID(orderId);
        orderCancelRequest_.setOrigClOrdID(clOrdId);

        outHeader_.setBodyLength(outHeader_.calcLen() + orderCancelRequest_.calcLen());
        return render(outHeader_, orderCancelRequest_);
    }

    template <typename Msg>
    size_t render(ToFtxStandardHeader& header, Msg& msg)
    {
        char* p = header.render(outBuffer_);
        p       = msg.render(p);
        // TODO:FIX LEN
        //        const uint16_t cksum = (header.getCheckSum() + msg.getCheckSum()) % 256;
        //        tail_.set(cksum);
        //        auto const tail_len = tail_.render(p);
        //        auto len            = p - outBuffer_ + tail_len;
        // ASSERT_MSG(len <= OutBufferSize, DUMP(len) << DUMP(OutBufferSize));
        tail_.reset();
        return 0;
    }

    /*---------------------------------------------------------------------------------
    *  Fix Reader callbacks
    ---------------------------------------------------------------------------------*/
    template <typename Handler>
    void onLogon(Handler& handler)
    {
        canSendOrder_ = true;
        LogonReader r;
        r.read(fixIstream_);
        exchangeSeqNum_ = inFixHeader_.msgSeqNum.toInteger<uint32_t>();
        expectNextExchSeqNum_ =
            exchangeSeqNum_ + 2; // because we do not wait for gateway logout message, so we assume we receive it.

        inSessionResetSeq_ = true;
        handler.on_logon();
    }

    template <typename Handler>
    void onLogout(Handler& handler)
    {
        canSendOrder_ = false;
        saveSeqNum();
        LogoutReader r;
        r.read(fixIstream_);

        //        uint32_t new_seq = getSeq(r.text.toString());
        //        if (new_seq > 0)
        //        {
        //            seqNum_.setSeqNum(new_seq - 1);
        //        }

        handler.on_logout();
    }

    template <typename Handler>
    void onExecutionReport(Handler& handler)
    {
        auto& reader = executionReportReader_;

        reader.read(fixIstream_);
        auto const orderStatus = reader.ordStatus.toEnum<OrdStatusEnum>();

        switch (orderStatus)
        {
        case OrdStatusEnum::NewOrderAck: {
            handler.onOrderAck(reader.orderId.toInteger<order_id_t>(),
                               reader.clOrdId.toInteger<clorder_id_t>(),
                               reader.orderQty.toInteger<quantity_t>(),
                               reader.price.toFloatingPoint<double>(),
                               reader.side.toEnum<SideEnum>());
            break;
        }
        case OrdStatusEnum::PartialFill:
        case OrdStatusEnum::OrderFullyFilled: {
            handler.onFill(reader.orderId.toInteger<order_id_t>(),
                           reader.clOrdId.toInteger<clorder_id_t>(),
                           reader.lastPx.toFloatingPoint<double>(),
                           reader.lastQty.toInteger<quantity_t>(),
                           reader.leavesQty.toInteger<quantity_t>(),
                           reader.side.toEnum<SideEnum>(),
                           orderStatus == OrdStatusEnum::OrderFullyFilled);

            break;
        }
        case OrdStatusEnum::CancelAck: {
            handler.onCancelAck(reader.orderId.toInteger<order_id_t>(),
                                reader.clOrdId.toInteger<clorder_id_t>(),
                                reader.orderQty.toInteger<quantity_t>(),
                                reader.price.toFloatingPoint<double>(),
                                reader.side.toEnum<SideEnum>());
            break;
        }
        case OrdStatusEnum::ModifyAck:
            assert(0); // no cancel_replace support
            break;
            //        case OrdStatusEnum::Rejected: // order level, Nack
            //        {
            //            handler.onOrderReject(reader.orderId.toInteger<order_id_t>(),
            //                                  reader.clOrdId.toInteger<clorder_id_t>(),
            //                                  reader.orderQty.toInteger<quantity_t>(),
            //                                  reader.ordRejReason.toInteger<uint32_t>(),
            //                                  reader.text.toString());
            //            break;
            //        }
            //        case OrdStatusEnum::Expired: // Order Elimination
            //        {
            //            // 39=C
            //            // 150=C
            //            // unsolicited cancel from exchange
            //            handler.onCancelAck(reader.orderId.toInteger<order_id_t>(),
            //                                reader.clOrdId.toInteger<clorder_id_t>(),
            //                                reader.orderQty.toInteger<quantity_t>(),
            //                                reader.price.toFloatingPoint<double>(),
            //                                reader.side.toEnum<SideEnum>());
            //            break;
            //        }
            //        case OrdStatusEnum::TradeCanceled: {
            //            // 39=H
            //            handler.onTradeCancelled(reader.orderId.toInteger<order_id_t>(),
            //                                     reader.clOrdId.toInteger<clorder_id_t>(),
            //                                     reader.lastPx.toFloatingPoint<double>(),
            //                                     reader.lastQty.toInteger<quantity_t>(),
            //                                     reader.side.toEnum<SideEnum>(),
            //                                     reader.execId.toString());
            //
            //            break;
            //        }
        default:
            // assert(0);
            // ignore all other order status
            break;
        }
    }

    template <typename Handler>
    void onSessionLevelReject(Handler& handler, uint32_t seq, const std::string& text)
    {
        handler.onSessionLevelReject(seq, text);
    }

    //    template <typename Handler>
    //    void onOrderCancelReject(Handler& handler)
    //    {
    //        auto& reader = orderCancelRejectReader_;
    //        reader.read(fixIstream_);
    //        handler.onOrderCancelReject(
    //            reader.orderId.toInteger<order_id_t>(), reader.clOrdId.toInteger<clorder_id_t>(),
    //            reader.text.toString());
    //    }

    void onHeartBeat();
    size_t onTestRequest();
    size_t onSequenceReset(uint32_t exchExpectSeqFromUs);
    void onResendRequest();

    template <typename Handler>
    uint32_t processIncomingMsg(Handler& handler, const char* begin, size_t bufLen)
    {
        handler.log(begin, bufLen);

        const char* p = begin;

        fixIstream_.setBuffer(begin, bufLen);
        inFixHeader_.read(fixIstream_);

        auto const& bodyLengthField = inFixHeader_.bodyLength;
        uint32_t fixMsgLen          = 10 + 3 + bodyLengthField.getLen() + bodyLengthField.toInteger<uint32_t>() +
                             7; // FIXME magic numbers 10, 3 & 7
        p += fixMsgLen;

        auto arrivedSeqNum = inFixHeader_.msgSeqNum.toInteger<uint32_t>();

        // check sequence gap
        if (expectNextExchSeqNum_ == 1) // first msg
        {
            expectNextExchSeqNum_ = arrivedSeqNum + 1;
            exchangeSeqNum_       = arrivedSeqNum;
        }
        else if (arrivedSeqNum == exchangeSeqNum_ + 1) // normal msg update
        {
            arrivedSeqNum += 1;
            exchangeSeqNum_ = arrivedSeqNum;
        }
        else if (arrivedSeqNum > exchangeSeqNum_ + 1) // gap detected, request resend
        {
            expectNextExchSeqNum_ = arrivedSeqNum + 1;

            if (handler.session_enabled())
            {
                return resendRequest();
            }
        }
        else
        {
            exchangeSeqNum_ = arrivedSeqNum;
        }

        switch (inFixHeader_.msgType.toEnum<MsgTypeEnum>())
        {
        case MsgTypeEnum::Reject: // session level reject
        {
            //            SessionLevelRejectReader r;
            //            r.read(fixIstream_);
            //            LOG_ERROR("MSGW session level order placement reject, RefSeqNum: " <<
            //            r.refSeqNum.toInteger<uint32_t>()
            //                                                                               << ", reason: " <<
            //                                                                               r.text.toString());
            //            onSessionLevelReject(handler, r.refSeqNum.toInteger<uint32_t>(), r.text.toString());
            break;
        }
        case MsgTypeEnum::ExecutionReport: {
            onExecutionReport(handler);
            executionReportReader_.reset();
            break;
        }
        case MsgTypeEnum::OrderCancelReject: {
            onOrderCancelReject(handler);
            break;
        }
        case MsgTypeEnum::Logon: {
            // LOG_INFO("MSGW session level Logon received, segment " << segment_);
            onLogon(handler);
            break;
        }
        case MsgTypeEnum::Logout: {
            // LOG_INFO("MSGW session level Logout received, segment " << segment_);
            onLogout(handler);
            break;
        }
        case MsgTypeEnum::SequenceReset: {
            //            SequenceResetReader r;
            //            r.read(fixIstream_);
            //            LOG_INFO("MSGW session level Sequence Reset received, we had << " << DUMP(exchangeSeqNum_) <<
            //            "setting to "
            //                                                                              <<
            //                                                                              r.newSeqNo.toInteger<uint32_t>()
            //                                                                              << " "
            //                                                                              << DUMP(segment_));
            //            exchangeSeqNum_ = r.newSeqNo.toInteger<uint32_t>();
            // no fixIstream.reset() ?
            return 0;
        }
        case MsgTypeEnum::TestRequest: {
            // LOG_INFO("Test Request received, sgement= " << segment_);
            // no fixIstream.reset() ?
            return onTestRequest();
        }
        case MsgTypeEnum::HeartBeat: {
            // LOG_INFO("heartbeat, segment= " << segment_);
            onHeartBeat();
            break;
        }
        case MsgTypeEnum::ResendRequest: {
            //            ResendRequestReader r;
            //            r.read(fixIstream_);
            //
            //            auto const exchExpectSeqFromUs = r.beginSeqNo.toInteger<uint32_t>();
            //            // no fixIstream.reset() ?
            //            return onSequenceReset(exchExpectSeqFromUs);
            break;
        }
        default:
            // LOG_WARNING("unsupported message type from CME " <<
            // (uint32_t)inFixHeader_.msgType.toEnum<MsgTypeEnum>());
            break;
        }

        fixIstream_.reset();
        return 0;
    }

    //    template <typename Handler>
    //    void consume_tcp_buffer(Handler& handler, const ::qx::qstream::place& plc, bool logged_in)
    //    {
    //        static const size_t read_prefix_len = 16;
    //#if 0
    //        // if there was leftover junk in the packet after the previous fix message
    //        // it will remain in the buffer and we have to dump it.
    //        // is this possible? Testing will tell.
    //        if(!::strncmp((char*)plc.start, "8=FIX.4.2", 9)) {
    //            fix_in_buf->clear();
    //        }
    //#endif
    //        fix_in_buf->copy_to_back((char*)plc.start, plc.size);
    //        if (!logged_in && *fix_in_buf->data() != '8')
    //        {
    //            // error messages when not yet logged in are not in fix protocol
    //            LOG_ERROR("Error msg from exchange on " << DUMP(segment_) << "while not logged in "
    //                                                    << std::string(fix_in_buf->data(),
    //                                                    fix_in_buf->buffer_used()));
    //
    //            // dump the input buffer in its entirety and get out
    //            fix_in_buf->clear();
    //            return;
    //        }
    //        // should now be sure to have a fix message at the start of the buffer
    //        INVARIANT_MSG(*fix_in_buf->data() == '8', DUMP(std::string(fix_in_buf->data(),
    //        fix_in_buf->buffer_used())));
    //
    //        // if we have enough of the message to know its size
    //        while (fix_in_buf->buffer_used() > read_prefix_len)
    //        {
    //            auto const fix_body_len = fix::get_fix_body_len(fix_in_buf->data());
    //            auto const fix_msg_len  = fix::get_single_fix_msg_len(fix_body_len);
    //            if (fix_in_buf->buffer_used() < fix_msg_len)
    //            {
    //                // still haven't received all fix message yet, append and return to loop
    //                return;
    //            }
    //            else
    //            {
    //                INVARIANT_MSG(*fix_in_buf->data() == '8', DUMP(fix_in_buf->data()));
    //                consume_single_fix(handler, fix_in_buf->data(), fix_msg_len);
    //                fix_in_buf->remove_from_front(fix_msg_len);
    //            }
    //        }
    //    }
    //
    //    template <typename Handler>
    //    void consume_single_fix(Handler& handler, const char* p, size_t total_len)
    //    {
    //        auto const len = processIncomingMsg(handler, p, total_len);
    //        if (len > 0)
    //        {
    //            handler.send_and_log(outBuffer_, len);
    //        }
    //    }

    void saveSeqNum();
    uint32_t getSeq(const std::string& text);
    const std::string& getSegment() const { return segment_; }

    uint32_t getLastSequenceNum() const { return seqNum_.getSeqNum(); }

  private:
    /*
     * Set *static* header fields
     * They will remain same during the application runtime
     */
    void init(uint32_t seqNum, uint32_t exchSeqNum);

  private:
    char* outBuffer_{};

    SequenceNumGenerator seqNum_{};
    ToFtxStandardHeader outHeader_;
    NewOrderSingle newOrder_;
    OrderCancelRequest orderCancelRequest_;
    CheckSumField tail_;

    // std::shared_ptr<fundamentals::reassembly_buffer<InBufferSize>> fix_in_buf;

    FixIstream fixIstream_;
    StandardHeaderReader inFixHeader_{};
    ExecutionReportReader executionReportReader_{};
    OrderCancelRejectReader orderCancelRejectReader_{};

    bool canSendOrder_{};
    uint32_t exchangeSeqNum_{0};
    uint32_t expectNextExchSeqNum_{1};
    fix_config_t config_{};
    std::string segment_;

    bool inSessionResetSeq_{};
    bool has_sent_replay_{};
    std::string sequence_file;
};

} // namespace miye::trading::fix::ftx
