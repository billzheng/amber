#include "libcore/essential/app.hpp"
#include "libcore/qstream/arbiter.hpp"
#include "libcore/qstream/kernel_arbiter.hpp"
#include "libcore/qstream/tcp_reader.hpp"
#include "libcore/qstream/tcp_writer.hpp"
#include "libcore/time/clock.hpp"

namespace miye::trading
{
class TestApp : public essential::app<TestApp>
{
  public:
    using Clock_t        = time::variant_clock;
    using tcp_reader_t   = qstream::tcp_reader<Clock_t>;
    using tcp_writer_t   = qstream::tcp_writer<Clock_t>;
    using high_arbiter_t = qstream::arbiter<Clock_t, tcp_reader_t, 20>;
    using low_arbiter_t  = qstream::kernel_arbiter<Clock_t>;

  public:
    int run(int argc, char** argv)
    {
        Clock_t clk("real_clock");
        high_arbiter_t hi_arb(clk);
        low_arbiter_t lo_arb(clk);

        init_arb(hi_arb, lo_arb);
        /*
         * skip existing messages in order request files
         */
        loop(clk, hi_arb, lo_arb);
        return 0;
    }
    void loop(Clock_t& clk, high_arbiter_t& hi_arb, low_arbiter_t& lo_arb)
    {
        uint64_t repeats           = 1;
        qstream::stream_id_t hi_id = qstream::arbiter_error::end;
        qstream::stream_id_t lo_id = qstream::arbiter_error::end;

        while (!end() && hi_id != qstream::arbiter_error::empty)
        {
            hi_id = hi_arb.ruling();
            switch (hi_id)
            {
            case qstream::arbiter_error::timeout:
            case qstream::arbiter_error::interrupt:
            case qstream::arbiter_error::empty:
                break;
            case qstream::arbiter_error::end:
                ++repeats;
                break;
            default: {

                break;
            }
            }

            lo_id = lo_arb.ruling();
            switch (lo_id)
            {
            case qstream::arbiter_error::timeout:
            case qstream::arbiter_error::interrupt:
            case qstream::arbiter_error::empty:
                break;
            case qstream::arbiter_error::end:
                break;
            default: {

                break;
            }
            }
        }
    }
    void init_arb(high_arbiter_t& hi_arb, low_arbiter_t& lo_arb)
    {
        //        for (size_t i = 0; i < mmr_order_requests.size(); ++i)
        //        {
        //            auto const request_idx = hi_arb.submit(&mmr_order_requests[i]);
        //            INVARIANT(request_idx == i + 1);
        //            mmr_order_reader_ids[request_idx] = static_cast<qstream::stream_id_t>(i);
        //        }
        hi_arb.submission_complete();

        //        for (size_t i = 0; i != tcp_fix_readers.size(); ++i)
        //        {
        //            qstream::stream_id_t desired_id = static_cast<qstream::stream_id_t>(i);
        //            auto id                         = lo_arb.submit(tcp_fix_readers[i].get_fd(), desired_id);
        //            CARP(id == desired_id, DUMP(+desired_id) << DUMP(+id));
        //        }
        lo_arb.submission_complete();
    }
};
} // namespace miye::trading

int main(int argc, char** argv)
{
    miye::trading::TestApp app;
    return app.main(argc, argv);
}
