/*
 * exchangesim_reader.hpp
 *
 * Purpose: Simulate an exchange
 */
#include "libcore/essential/assert.hpp"
#include "libcore/qstream/exchangesim_ordermanager.hpp"
#include "libcore/qstream/exchangesim_writer.hpp"
#include "libcore/qstream/qstream_reader_interface.hpp"
#include "libcore/time/timeutils.hpp"
#include "message/english.hpp"

#pragma once

namespace miye
{
namespace qstream
{
template <typename Clock>
class exchangesim_reader
    : public qstream_reader_interface<exchangesim_reader<Clock>>
{
  public:
    exchangesim_reader(Clock& clk, const std::string& description) = delete;
    exchangesim_reader() = delete;

    // can only construct one of these once you've already got a writer
    explicit exchangesim_reader(exchangesim_writer<Clock>& rhs)
        : clk(rhs.clk), description(rhs.description),
          order_responses_ref(rhs.order_responses)
    {
    }

    explicit exchangesim_reader(const exchangesim_reader<Clock>& rhs)
        : clk(rhs.clk), description(rhs.description),
          order_responses_ref(rhs.order_responses_ref)
    {
    }

    const place read()
    {
        auto sz = order_responses_ref.size();
        INVARIANT_MSG(sz > 0, "Can't read when there are no responses.");
        outgoing = order_responses_ref.front();
        order_responses_ref.pop_front();
        INVARIANT(order_responses_ref.size() == (sz - 1));
        return place(&outgoing, types::message::order_msg::msg_size);
    }
    void attest(uint64_t* next_timestamp)
    {
        auto sz = order_responses_ref.size();
        if (sz > 0)
        {
            *next_timestamp = this->clk.now();
        }
        else
        {
            // std::cerr << __PRETTY_FUNCTION__ << " ...nothing\n";

            *next_timestamp = time::max;
        }
    }

    Clock& clk;
    const std::string description;
    responses_t& order_responses_ref;
    types::message::any_order outgoing;
};

} // namespace qstream
} // namespace miye
