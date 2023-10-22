#include "../ftx_fix/fix_session.hpp"

#include "../ftx_fix/time_utils.hpp"
#include "../ftx_ftx/fix_number_utils.hpp"
#include "../ftx_ftx/ftx_fix_reader.hpp"

namespace miye::trading::fix::ftx
{

FixSession::~FixSession() { saveSeqNum(); }

void FixSession::saveSeqNum() { fix::utils::save_seq_num_id(sequence_file, seqNum_.getSeqNum(), exchangeSeqNum_); }

void FixSession::init(uint32_t seqNum, uint32_t exchSeqNum)
{
    static const std::string FUT("FUT");

    seqNum_.setSeqNum(seqNum);
    exchangeSeqNum_       = exchSeqNum;
    expectNextExchSeqNum_ = exchangeSeqNum_ + 1;

    // std::cout << "[om] set exchange seq num to: " << exchangeSeqNum_ << ", next expected seq num to: " <<
    // expectNextExchSeqNum_ << std::endl;

    /*
     * StandardHeader to CME common static fields
     */
    outHeader_.setBeginString("FIX.4.2");
    outHeader_.setSenderCompID(config_.sender_comp_id);
    outHeader_.setTargetCompID("FTX");

    /*
     * OrderSingleNew common static fields
     */
    newOrder_.setHandInst(HandInstEnum::AutomatedExecution);
    newOrder_.setCustomerOrFirm(CustomerOrFirmEnum::Customer);
    newOrder_.setManualOrderIndicator(ManualOrderIndicatorEnum::Automated);
    newOrder_.setCtiCode(CtiCodeEnum::Cti4);
    newOrder_.setAccount(config_.account);
    newOrder_.setSecurityType(FUT);

    /*
     * OrderCancelRequest static fields
     */
    orderCancelRequest_.setAccount(config_.account);
    orderCancelRequest_.setManualOrderIndicator(ManualOrderIndicatorEnum::Automated);

    /*
     * OrderCancelReplaceRequest
     */
    orderCancelReplaceRequest_.setManualOrderIndicator(ManualOrderIndicatorEnum::Automated);
    orderCancelReplaceRequest_.setCustomerOrFirm(CustomerOrFirmEnum::Firm);
    orderCancelReplaceRequest_.setAccount(config_.account);
    orderCancelReplaceRequest_.setSecurityType(FUT);
    orderCancelReplaceRequest_.setCtiCode(CtiCodeEnum::Cti4);
}

size_t FixSession::logon()
{
    outHeader_.setMsgType(MsgTypeEnum::Logon);
    outHeader_.setMsgSeqNum(seqNum_.getNextSeqNum());
    outHeader_.setSendingTime(time_utils::getTime());

    Logon msg;
    //    if (inSessionResetSeq_)
    //    {
    //        msg.setResetSeqNumFlag("Y");
    //    }

    msg.setRawData(config_.password);
    msg.setRawDataLength(config_.password.length());
    msg.setEncryptMethod(EncryptMethodEnum::None);

    // FTX heartbeat interval must set to 30
    constexpr static const std::chrono::seconds HeartBeatInterval = std::chrono::seconds(30);
    msg.setHeartbtlnt(HeartBeatInterval.count());
    msg.setApplicationSystemName(config_.app_name);
    msg.setApplicationSystemVendor(config_.system_vendor);
    msg.setTradingSystemVersion(config_.app_version);

    outHeader_.setBodyLength(outHeader_.calcLen() + msg.calcLen());
    auto const len = render(outHeader_, msg);

    return len;
}

size_t FixSession::logout()
{
    outHeader_.setMsgSeqNum(seqNum_.getNextSeqNum());
    outHeader_.setMsgType(MsgTypeEnum::Logout);
    outHeader_.setSendingTime(time_utils::getTime());

    Logout msg;
    msg.setText("bye");
    outHeader_.setBodyLength(outHeader_.calcLen() + msg.calcLen());
    return render(outHeader_, msg);
}

size_t FixSession::sendHeartBeat()
{
    outHeader_.setMsgSeqNum(seqNum_.getNextSeqNum());
    outHeader_.setMsgType(MsgTypeEnum::HeartBeat);
    outHeader_.setSendingTime(time_utils::getTime());

    Heartbeat msg;
    outHeader_.setBodyLength(outHeader_.calcLen() + msg.calcLen());
    return render(outHeader_, msg); // msg is empty if testReqId is not set
}

size_t FixSession::resendRequest()
{
    if (has_sent_replay_) // have sent one resend request already, don't send next one
    {
        return 0;
    }
    const std::string timestamp = time_utils::getTime();
    outHeader_.setMsgSeqNum(seqNum_.getNextSeqNum());
    outHeader_.setMsgType(MsgTypeEnum::ResendRequest);
    outHeader_.setSendingTime(timestamp);

    ResendRequest msg;
    msg.setBeginSeqNo(exchangeSeqNum_);

    if (expectNextExchSeqNum_ - exchangeSeqNum_ < 2500)
    {
        msg.setBeginSeqNo(exchangeSeqNum_ + 1);
        msg.setEndSeqNo(0);
    }
    else
    {
        msg.setBeginSeqNo(exchangeSeqNum_ + 1);
        msg.setEndSeqNo(exchangeSeqNum_ + 2499);
    }
    has_sent_replay_ = true;
    outHeader_.setBodyLength(outHeader_.calcLen() + msg.calcLen());
    return render(outHeader_, msg);
}

size_t FixSession::testRequest()
{
    const std::string timestamp = time_utils::getTime();
    outHeader_.setMsgSeqNum(seqNum_.getNextSeqNum());
    outHeader_.setMsgType(MsgTypeEnum::TestRequest);
    outHeader_.setSendingTime(timestamp);

    // response from exchange will be HeartBeat
    TestRequest msg;
    msg.setTestReqID("test_req_id");
    outHeader_.setBodyLength(outHeader_.calcLen() + msg.calcLen());
    return render(outHeader_, msg);
}

size_t FixSession::onTestRequest()
{
    TestRequestReader r;
    r.read(fixIstream_);

    outHeader_.setMsgSeqNum(seqNum_.getNextSeqNum());
    outHeader_.setMsgType(MsgTypeEnum::HeartBeat);
    outHeader_.setSendingTime(time_utils::getTime());

    Heartbeat msg;
    msg.setTestReqID(r.testReqId.toString());
    outHeader_.setBodyLength(outHeader_.calcLen() + msg.calcLen());
    return render(outHeader_, msg);
}

size_t FixSession::onSequenceReset(uint32_t exchExpectSeqFromUs)
{
    const std::string timestamp = time_utils::getTime();

    outHeader_.setMsgType(MsgTypeEnum::SequenceReset);
    outHeader_.setSendingTime(timestamp);
    outHeader_.setPossDupFlag("Y");

    outHeader_.setOrigSendingTime(inFixHeader_.sendingTime.toString());
    outHeader_.setLastMsgSeqNumProcessed(inFixHeader_.msgSeqNum.toInteger<uint32_t>());

    SequenceReset msg;
    msg.setGapFillFlag("Y");

    outHeader_.setMsgSeqNum(exchExpectSeqFromUs);

    msg.setNewSeqNo(seqNum_.getSeqNum() + 1);
    outHeader_.setBodyLength(outHeader_.calcLen() + msg.calcLen());
    auto const len = render(outHeader_, msg);

    outHeader_.reset();
    init(seqNum_.getSeqNum(), exchangeSeqNum_);
    return len;
}

void FixSession::onHeartBeat()
{
    HeartbeatReader r;
    r.read(fixIstream_);
    has_sent_replay_ = false; // clear flat
}

uint32_t FixSession::getSeq(const std::string& text)
{
    uint32_t seq{};

    auto begin = text.find('[');

    if (begin != std::string::npos && begin < text.size())
    {
        begin++;
        auto end = text.find(']', begin);

        if (end != std::string::npos && end > begin)
        {
            std::string s = text.substr(begin, end - begin);
            return fix::toUInteger<uint32_t>(s);
        }
    }

    return seq;
}

/*
void FixSession::onLogon()
{
    canSendOrder_ = true;
    LogonReader r;
    r.read(fixIstream_);
    exchangeSeqNum_  = inFixHeader_.msgSeqNum.toInteger<uint32_t>();
    expectNextExchSeqNum_ = exchangeSeqNum_ + 2;		// because we do not wait for gateway logout message, so we
assume we receive it.
    //std::cout << "[om] [onLogon] set exchange seq num to: " << exchangeSeqNum_ << ", next expected seq num to: " <<
expectNextExchSeqNum_ << std::endl; inSessionResetSeq_ = true;
}

void FixSession::onLogout()
{
    canSendOrder_ = false;
    saveSeqNum();
}
*/
const char* toCxlReasonStr(uint32_t reason)
{
    switch (reason)
    {
    case 0:
        return "Too late to cancel";
    case 1:
        return "Unknown order";
    case 2:
        return "Broker/Exchange option";
    case 4:
        return "Unable to process Order Mass Cancel request";
    case 5:
        return "OrigOrdModTime (586) did not match last TransactTime (60) of order";
    case 6:
        return "Duplicate ClOrdID (tag 11 value) received";
    case 7:
        return "Price exceeds current price";
    case 8:
        return "Price exceeds current price band";
    case 18:
        return "Invalid price increment";
    case 99:
        return "Other";
    case 1003:
        return "Orders may not be cancelled while the market is closed";
    case 2045:
        return "This order is not in the book";
    case 2048:
        return "The order was submitted with a different SenderCompID than the requesting cancel";
    case 2058:
        return "Stop price maxi-mini must be greater than or equal to trigger price";
    case 2060:
        return "Sell order stop price must be below last trade price";
    case 2061:
        return "Buy order stop price must be above last trade price";
    case 2137:
        return "Order price is outside the limits";
    case 2179:
        return "Order price is outside bands";
    case 7009:
        return "The contract for this order is past expiration/for future activation date and may no longer/not "
               "yet be traded";
    case 7024:
        return "Order cannot be modified or cancelled while the market is in No Cancel";
    default:
        assert(0);
    }
    return "Unknown";
}

} // namespace miye::trading::fix::ftx
