
#include "clock.hpp"
//#include "libcore/utils/logging.hpp"
#include "timeutils.hpp"
#include <limits>

namespace miye
{
namespace time
{

uint64_t real_clock::now()
{
    struct timespec ts;
    uint64_t tmp;
    syscalls::clock_gettime(CLOCK_REALTIME, &ts);
    tmp = (seconds(ts.tv_sec) + ts.tv_nsec);
    if (tmp < this->last_time)
    {
        // LOG_ERROR("time went backwards!");
    }
    else
    {
        this->last_time = tmp;
    }
    return tmp;
}

variant_clock::variant_clock(const std::string& clock_desc)
{
    type = from_str(clock_desc.c_str());
    switch (type)
    {
    case clock_type_t::event_clock:
        impl = std::make_shared<event_clock>(clock_desc);
        break;
    case clock_type_t::real_clock:
        impl = std::make_shared<real_clock>(clock_desc);
        break;
    case clock_type_t::replay_clock:
        impl = std::make_shared<replay_clock>(clock_desc);
        break;
    default:
        INVARIANT_FAIL(clock_desc << "  gives an unhandled clock type");
    }
}

} // namespace time
} // namespace miye
