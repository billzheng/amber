#pragma once

#include <stdint.h>
#include <time.h>

#include "libcore/qstream/variantqstream_writer.hpp"
#include "libcore/time/profiler_types.hpp"
#include "libcore/time/rdtsc.hpp"
#include "libcore/time/timeutils.hpp"
#include "message/message.hpp"

namespace miye { namespace time {

template<typename ClkType, typename StreamType>
class profiler
{
public:
    profiler(bool enabled, ClkType& clk, std::string file_name = "profiling-")
    : mask_(0xffffffffffffffff)
    , performance_log_(nullptr)
    , clk_(clk)
    , profiling_enabled_(enabled)
    , file_name_(file_name)
    {
        if(profiling_enabled_) {
            open_log();
        }
        ticks2ns = time::calibrate_ticks();
    }


    inline void open_log()
    {
        syscalls::posix_memalign((void**)&performance_log_, CACHE_LINE_SIZE, sizeof(StreamType));
        new (performance_log_) StreamType(clk_, file_name_);
    }

    inline void enable_profiling()
    {
        if(profiling_enabled_) {
            return;
        }
        open_log();
        profiling_enabled_ = true;
    }

    inline void disable_profiling()
    {
        if(profiling_enabled_ && performance_log_)
        {

            performance_log_->~StreamType();
            free(performance_log_);
            performance_log_ = nullptr;
        }
        profiling_enabled_ = false;
    }

    inline bool is_profiling_enabled()
    {
        return profiling_enabled_;
    }


    inline void write_timing_record(
            profiler_trigger const& trig,
            uint64_t const& cycles,
            uint64_t const& uint64_data,
            uint32_t const& uint32_data,
            uint16_t const& uint16_data,
            uint8_t const& uint8_data,
            uint8_t const& uint8_data2)
    {
        if(is_profiling_enabled() && ((uint64_t)trig & mask_))
        {
            auto plc = performance_log_->pledge(types::message::profiler::msg_size);
            new (plc.start) types::message::profiler (trig, cycles, cycles * ticks2ns, uint64_data, uint32_data, uint16_data, uint8_data, uint8_data2);
            performance_log_->announce(plc);
        }
    }

    ~profiler()
    {
        if(performance_log_) {
            performance_log_->~StreamType();
            free(performance_log_);
            performance_log_ = nullptr;
        }
    }
private:
    uint64_t mask_;
    StreamType *performance_log_;
    ClkType& clk_;
    bool profiling_enabled_;
    std::string file_name_;
    double ticks2ns;
};

#ifdef PROFILING

#define TIME(varname) uint64_t varname = essential::rdtscp()

#define WRITE_TIME(cur_profiler__, trig, cycles, uint64_data, uint32_data, uint16_data, uint8_data, uint8_data2) \
    (cur_profiler__).write_timing_record(trig, cycles, uint64_data, uint32_data, uint16_data, uint8_data, uint8_data2)

#else

#define TIME(varname)
#define WRITE_TIME(cur_profiler__, trig, cycles, uint64_data, uint32_data, uint16_data, uint8_data, uint8_data2)

#endif



}}

