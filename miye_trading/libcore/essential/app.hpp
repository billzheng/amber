#pragma once

#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "git_version.h"

#include "libcore/essential/backtrace.hpp"
#include "libcore/essential/exception.hpp"
#include "libcore/utils/syscalls_misc.hpp"

namespace miye
{
namespace essential
{

static const char git_version[] = GIT_VERSION;
static sig_atomic_t terminate;

template <typename Derived>
class app
{
  public:
    app()
    {
        struct sigaction act
        {
        };
        struct sigaction oact
        {
        };

        ::memset(&act, 0, sizeof(act));
        act.sa_handler = sighandler;

        // syscalls::sigaction<false>(SIGINT, &act, &oact);
        syscalls::sigaction<false>(SIGTERM, &act, &oact);
        syscalls::sigaction<false>(SIGSEGV, &act, &oact);
        syscalls::sigaction<false>(SIGBUS, &act, &oact);

        act.sa_handler = SIG_IGN;
        syscalls::sigaction<false>(SIGPIPE, &act, &oact);
    }
    ~app() {}

    int main(int argc, char** argv)
    {
        int exit_status = 1;

        exit_status = static_cast<Derived*>(this)->run(argc, argv);

        return exit_status;
    }

    static int end() { return terminate; }

    static void stop() { terminate++; }

  protected:
    static void sighandler(int sig)
    {
        bool got_bus = false;
        switch (sig)
        {
        case SIGBUS: {
            std::cerr << "SIGBUS caught" << std::endl;
            got_bus = true;
            /* fallthrough */
        }
        case SIGSEGV: {
            if (!got_bus)
            {
                std::cerr << "SIGSEGV caught" << std::endl;
            }
            auto bt = backtrace::backtrace();
            for (auto& e : bt)
                std::cerr << e << std::endl;
            ::abort();
        }
        default: {
            terminate++;
            break;
        }
        }
    }
};

} // namespace essential
} // namespace miye
