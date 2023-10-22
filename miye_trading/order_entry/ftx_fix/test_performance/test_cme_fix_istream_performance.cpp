#include "benchmark/benchmark.h"
#include <iostream>
#include <string.h>

#include "../../ftx_fix/fix_number_utils.hpp"
#include "../../ftx_fix/ftx_fix_reader.hpp"
#include "../../ftx_fix/ftx_fix_writer.hpp"
#include "../../ftx_fix/string_const.hpp"

static void fix_reader(benchmark::State& state)
{
    const char* fix="8=FIX.4.2\0019=314\00135=8\00134=9589\001369=9904\00152=20170406-12:31:53.061\00149=CME\00150=84\00156=LJT0QLN\00157=RVINE\001143=HK\0011=88234\0016=0\00111=1019100002\00114=0\00117=84285:27897646\00120=0\00137=844213050107\00138=7\00139=0\00140=2\00141=0\00144=151.46875\00148=39039\00154=1\00155=ZB\00159=0\00160=20170406-12:31:53.060\001107=ZBM7\001150=0\001151=7\001167=FUT\001432=20170406\0011028=N\0015979=1491481913060258537\00110=184\001";
    qx::cme::StandardHeaderReader headerReader;
    qx::cme::ExecutionReportReader msgReader;

    while(state.KeepRunning())
    {
        qx::cme::FixIstream fixIstream;
        fixIstream.setBuffer(fix, strlen(fix));

        benchmark::DoNotOptimize(headerReader.read(fixIstream));
        benchmark::DoNotOptimize(msgReader.read(fixIstream));

        headerReader.reset();
        msgReader.reset();
    }
}

BENCHMARK(fix_reader);

static void OrderNewWriter(benchmark::State& state)
{
    qx::cme::ToCmeStandardHeader header;

    header.setBeginString("FIX.4.2");
    header.setMsgSeqNum(123456789);
    header.setBodyLength(122);
    header.setMsgType("D");
    header.setSenderCompID("LJT0QLN");
    header.setSenderSubID("RVINE");
    header.setSendingTime("20170406-12:29:10.711");
    header.setTargetCompID("CME");
    header.setTargetSubID("84");
    header.setSenderLocationID("HK");

    qx::cme::NewOrder o;
    o.setOrdType("2");
    o.setTransactTime("20170406-12:29:10.711");
    o.setSecurityDesc("ESH7");
    o.setSymbol("ESH7");
    o.setSecurityType("FUT");
    o.setSide("1");
    o.setPrice(std::to_string(47.55));
    o.setOrderQty(42);
    o.setCustOrderHandlingInst("1");
    o.setCustomerOrFirm(0);
    o.setManualOrderIndicator("N");
    o.setCtiCode("4");

    std::array<char, 512> buffer{};
    std::fill(std::begin(buffer), std::end(buffer), 0x01);

    qx::cme::Field<10, 3> tail;
    int i = 1;

    constexpr acutus::string_const Limit("1");
    constexpr acutus::string_const bid("1");
    constexpr acutus::string_const ask("2");
    constexpr acutus::string_const symbol("ESH7");   
    constexpr acutus::string_const securityType("FUT");
    
    std::string timestamp("20170406-12:29:10.711");

    while(state.KeepRunning())
    {
        header.setMsgSeqNum(++i);
        header.setBodyLength(header.calcLen() + o.calcLen());
        
        o.setOrdType(Limit);
        o.setTransactTime(timestamp.data(), timestamp.size());
        o.setSecurityDesc(symbol);
        o.setSymbol(symbol);
        o.setSecurityType(securityType);
        o.setSide(ask);
        o.setPrice(47.55);
        o.setOrderQty(42);
        o.setClOrdID(100000001);
        auto p = header.render(buffer.data());
        p = o.render(p);

        auto const cksum = (header.getCheckSum() + o.getCheckSum()) % 256;

        tail.set(cksum);
        std::memcpy(p, tail.begin(), tail.len());
    }
}

BENCHMARK(OrderNewWriter);

static void OrderCancelWriter(benchmark::State& state)
{
    qx::cme::ToCmeStandardHeader header;

    header.setBeginString("FIX.4.2");
    header.setMsgSeqNum(123456789);
    header.setBodyLength(122);
    header.setMsgType("D");
    header.setSenderCompID("LJT0QLN");
    header.setSenderSubID("RVINE");
    header.setSendingTime("20170406-12:29:10.711");
    header.setTargetCompID("CME");
    header.setTargetSubID("84");
    header.setSenderLocationID("HK");

    qx::cme::OrderCancelRequest o;
    o.setTransactTime("20170406-12:29:10.711");
    o.setSecurityDesc("ESH7");
    o.setSymbol("ESH7");
    o.setSecurityType("FUT");
    o.setManualOrderIndicator("N");

    std::array<char, 512> buffer{};
    std::fill(std::begin(buffer), std::end(buffer), 0x01);

    qx::cme::Field<10, 3> tail;
    int i = 1;

    while(state.KeepRunning())
    {
        header.setMsgSeqNum(++i);
        header.setBodyLength(header.calcLen() + o.calcLen());
        
        o.setOrderID(++i);
        o.setClOrdID(100000001);
        o.setClOrdID(i);
        o.setTransactTime(acutus::string_const("20170406-12:29:10.711"));
        o.setSecurityDesc(acutus::string_const("ESH7"));
        o.setSymbol(acutus::string_const("ESH7"));
        o.setSecurityType(acutus::string_const("FUT"));
        o.setSide(acutus::string_const("1"));
        
        auto p = header.render(buffer.begin());
        p = o.render(p);
        auto const cksum = (header.getCheckSum() + o.getCheckSum()) % 256;

        tail.set(cksum);
        std::memcpy(p, tail.begin(), tail.len());
    }
}

BENCHMARK(OrderCancelWriter);

BENCHMARK_MAIN();
