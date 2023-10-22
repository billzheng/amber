#pragma once
#include "channel_base.h"
#include <stdint.h>

namespace miye
{
namespace tech
{

struct ReadCB
{
};

struct WriteCB
{
};

struct ErrorCB
{
};

struct DispatcherBase
{
  protected:
    struct FdChannel
    {
        ReadCB* readCh{};
        WriteCB* writeCh{};
        ErrorCB* errorCh{};
    };

    struct ChannelFd
    {
        int readFd{};
        int writeFd{};
        int errorFd{};
        int timeoutIdle{};
    };

  public:
    enum class Mask
    {
        ON_READ = 1,
        ON_WRITE = 2,
        ON_ERROR = 4,
        ON_TIMEOUT = 8,
        ON_PRIORITY = 16,
        ON_IDLE = 32,
        ON_ALL = ON_READ | ON_WRITE | ON_ERROR | ON_TIMEOUT | ON_IDLE
    };

    DispatcherBase() = default;
    virtual ~DispatcherBase() {}

    virtual int addChannel(ChannelBase* channel, int mask) = 0;
    virtual int removeChannel(ChannelBase* channel, int mask = Mask::ON_ALL) = 0;

    virtual int run() = 0;
    virtual int stop() = 0;
    virtual int mode() const = 0;
    virtual int32_t numCallBacks() const = 0;
};

/*
 * temp dispatch class
 */

struct Dispatch : public DispatcherBase
{
    int addChannel(ChannelBase* channel, int mask) override { return 0; }
};

} // namespace tech
} // namespace miye
