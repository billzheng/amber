#pragma once

#include "libcore/utils/logging.hpp"
#include <netdb.h>

namespace miye
{
namespace utils
{

bool set_cpu_affinity(int cpu)
{
    if (cpu > 0)
    {
        long cpus_installed = sysconf(_SC_NPROCESSORS_ONLN);

        if (cpus_installed < 0)
        {
            LOG_WARNING("unable to determine cpus on machine - not setting cpu "
                        "affinity");
        }
        else
        {
            if (cpu > cpus_installed)
            {
                LOG_ERROR("cpu specified ["
                          << cpu << "] greater than cpus on machine ["
                          << cpus_installed << "]");
            }

            cpu_set_t my_set;
            CPU_ZERO(&my_set);
            CPU_SET(cpu, &my_set);
            if (sched_setaffinity(0, sizeof(cpu_set_t), &my_set) < 0)
            {
                LOG_ERROR("failed to set cpu afinity");
            }
            else
            {
                LOG_INFO("cpu affinity: " << cpu);
                return true;
            }
        }

        // check that the affinity is not already taken ?
        // the script should manage this on its own ?
    }
    else
    {
        LOG_WARNING("cpu affinity undefined");
    }
    return false;
}

bool get_ip_from_hostname(const std::string& host, uint32_t* ip)
{

    struct hostent* he;
    struct sockaddr_in server;

    he = gethostbyname(host.c_str());

    if (!he)
    {
        return false;
    }

    memcpy(&server.sin_addr, he->h_addr_list[0], he->h_length);

    *ip = server.sin_addr.s_addr;

    return true;
}

} // namespace utils
} // namespace miye
