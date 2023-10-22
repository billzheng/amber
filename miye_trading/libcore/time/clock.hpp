/*
 * clock.hpp
 * Purpose: clock interface
 * Author: Originally , re-written with static polymorphism 
 */

#pragma once

#include <time.h>
#include <memory>
#include <limits>
#include "libcore/essential/assert.hpp"
#include "libcore/utils/syscalls_misc.hpp"
#include "libcore/time/rdtsc.hpp"


namespace miye { namespace time {


enum class clock_type_t : uint8_t
{
    undefined = 0,
    real_clock = 1,
    event_clock = 2,
    replay_clock = 3,
    variant_clock = 8
};

inline const char* c_str(clock_type_t type)
{
    switch(type)
    {
        case clock_type_t::undefined: return "undefined";
        case clock_type_t::real_clock: return "real_clock";
        case clock_type_t::event_clock: return "event_clock";
        case clock_type_t::replay_clock: return "replay_clock";
        case clock_type_t::variant_clock: return "variant_clock";
        default:
            INVARIANT_FAIL("No such clock type, got: " << (static_cast<int>(type)));
        return "unreachable";
    }
}

template<typename OS>
OS& operator<<(OS& os, clock_type_t t)
{
    os << c_str(t);
    return os;
}

static inline clock_type_t from_str(const char* clock_desc)
{
    if (!strncmp("event_clock", clock_desc, strlen("event_clock"))) {
        return clock_type_t::event_clock;
    } else if (!strncmp("real_clock", clock_desc, strlen("real_clock"))) {
        return clock_type_t::real_clock;
    } else if (!strncmp("replay_clock", clock_desc, strlen("replay_clock"))) {
        return clock_type_t::replay_clock;
    } else if (!strncmp("variant_clock", clock_desc, strlen("variant_clock"))) {
        return clock_type_t::variant_clock;
    }

    return clock_type_t::undefined;
}

template<typename Derived>
struct clock_interface
{
    clock_interface()
    : last_time(1)
    {}
    clock_type_t clock_type() {
        return implementation()->clock_type();
    }
    void set(uint64_t time) {
        implementation()->set(time);
    }
    uint64_t now() {
        return implementation()->now();
    }
    bool can_set() {
        return implementation()->can_set();
    }

    bool be_discrete() {
        return implementation()->be_discrete();
    }

    void be_yourself() {
        implementation()->be_yourself();
    }

    Derived* implementation() { return static_cast<Derived*>(this); }
    mutable uint64_t last_time;
};


struct real_clock: public clock_interface<real_clock>
{
    real_clock(std::string description = std::string(""))
    {
        UNUSED(description);
    }
    clock_type_t clock_type() { return clock_type_t::real_clock; }
    void set(uint64_t t) { INVARIANT_FAIL("can't set a real clock to " << t); }
    bool can_set() { return false; }
    bool be_discrete() { return false; }
    void be_yourself() { }
    uint64_t now();
};

struct event_clock: public clock_interface<event_clock>
{
    event_clock(std::string description = std::string(""))
    {
        UNUSED(description);
    }
    clock_type_t clock_type() { return clock_type_t::event_clock; }
    void set(uint64_t t)
    {
        if (t > this->last_time) {
            this->last_time = t;
        }
    }
    uint64_t now() { return this->last_time; }
    bool can_set() { return true; }
    bool be_discrete() { return true; }
    void be_yourself() { }
};


struct replay_clock: public clock_interface<replay_clock>
{
    replay_clock(std::string clock_desc = std::string(""))
    : as_discrete(true)
    , ticks2ns(calibrate_ticks())
    , speedup_factor(1.0)
    {
        std::string key("speedup_factor=");
        auto idx = clock_desc.find(key);
        if(idx != std::string::npos) {
            std::string factor_string = clock_desc.substr(idx + key.size(),std::string::npos);
            speedup_factor = atof(factor_string.c_str());
            INVARIANT_MSG(speedup_factor > 0.0, DUMP(speedup_factor) << DUMP(clock_desc));
            //std::cerr << "using " << DUMP(speedup_factor) << std::endl;
        }
    }

    clock_type_t clock_type() { return clock_type_t::replay_clock; }
    void set(uint64_t t)
    {
        if (t > this->last_time) {
            if(as_discrete)
            {
                this->last_time = t;
            }
            else
            {
                auto next = t - this->last_time;
                auto start = rdtscp();
                while(true)
                {
                    if(((rdtscp() - start) * ticks2ns * speedup_factor) > next)
                    {
                        break;
                    }
                }
                this->last_time = t;
            }
        }
    }
    uint64_t now() {
        return this->last_time;
    }
    bool can_set() { return true; }
    bool be_discrete() { as_discrete = true; return true; }
    void be_yourself() { as_discrete = false;}

private:
    bool as_discrete;
    double ticks2ns;
    double speedup_factor;
};


class variant_clock
{
public:
    explicit variant_clock(const std::string& clock_desc);


    clock_type_t clock_type() {
        return type;
    }

    void set(uint64_t time) {
        switch(type)
        {
            case clock_type_t::real_clock:
                static_cast<real_clock*>(implementation())->set(time);
                break;
            case clock_type_t::event_clock:
                static_cast<event_clock*>(implementation())->set(time);
                break;
            case clock_type_t::replay_clock:
                static_cast<replay_clock*>(implementation())->set(time);
                break;
            default:
                INVARIANT_FAIL("unknown clock_type_t " << static_cast<int>(type));
        }
    }
    uint64_t now() {
        switch(type)
        {
            case clock_type_t::real_clock:
                return static_cast<real_clock*>(implementation())->now();
                break;
            case clock_type_t::event_clock:
                return static_cast<event_clock*>(implementation())->now();
                break;
            case clock_type_t::replay_clock:
                return static_cast<replay_clock*>(implementation())->now();
                break;
            default:
                INVARIANT_FAIL("unknown clock_type_t " << static_cast<int>(type));
        }
        // silence compiler
        return 0;
    }
    bool can_set() {
        switch(type)
        {
            case clock_type_t::real_clock:
                return static_cast<real_clock*>(implementation())->can_set();
                break;
            case clock_type_t::event_clock:
                return static_cast<event_clock*>(implementation())->can_set();
                break;
            case clock_type_t::replay_clock:
                return static_cast<replay_clock*>(implementation())->can_set();
                break;
            default:
                INVARIANT_FAIL("unknown clock_type_t " << static_cast<int>(type));
        }
        // silence compiler
        return false;
    }
    bool be_discrete() {
        switch(type)
        {
            case clock_type_t::real_clock:
                return static_cast<real_clock*>(implementation())->be_discrete();
                break;
            case clock_type_t::event_clock:
                return static_cast<event_clock*>(implementation())->be_discrete();
                break;
            case clock_type_t::replay_clock:
                return static_cast<replay_clock*>(implementation())->be_discrete();
                break;
            default:
                INVARIANT_FAIL("unknown clock_type_t " << static_cast<int>(type));
        }
        // silence compiler
        return false;
    }
    void be_yourself() {
        switch(type)
        {
            case clock_type_t::real_clock:
                static_cast<real_clock*>(implementation())->be_yourself();
                break;
            case clock_type_t::event_clock:
                static_cast<event_clock*>(implementation())->be_yourself();
                break;
            case clock_type_t::replay_clock:
                static_cast<replay_clock*>(implementation())->be_yourself();
                break;
            default:
                INVARIANT_FAIL("unknown clock_type_t " << static_cast<int>(type));
        }
    }

private:
    void* implementation() { return impl.get(); }
    std::shared_ptr<void> impl;
    clock_type_t type;
};


}}
