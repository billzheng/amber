#pragma once
#include <stdint.h>

namespace miye
{
namespace tech
{

enum class ChannalAction : int8_t
{
    REMOVE_CHANNEL,
    DELETE_CHANNEL
};

struct ChannelBase
{
    enum Mode
    {
        SPIN_MODE = 1,
        SELECT_MODE = 2,
        PRIORITY_MODE = 4,
        PREFER_SPIN_MODE = 8,
        SELECT_PREPROCESS_MODE = 16,
        SELECT_PREPROCESS_REACB_MODE = 32
    };
    ChannelBase() = default;
    virtual ~ChannelBase() {}
    /*
     * callback function when dispatcher is closing down
     *
     */
    virtual int onClose() = 0;
    virtual int mode() const = 0;
};

} // namespace tech
} // namespace miye
