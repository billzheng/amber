/*
 * logging.cpp
 *
 * Purpose: provide runtime logging
 *
 * Author: 
 *
 */
#include "libcore/utils/logging.hpp"
#include <algorithm>
#include <linux/limits.h>

namespace miye
{
namespace utils
{

any_logger logger_instance;
std::vector<std::string> log_opts;
uint64_t last_primary_write = 0;
char log_buffer[LOGBUFSIZE];

void add_logging_sink(std::string const& sink)
{
    log_opts.push_back(sink);
}

void log_process_details()
{

    LOG_INFO("PID: " << ::getpid());
    char buffer[PATH_MAX];
    ::gethostname(buffer, PATH_MAX);
    std::string h(buffer);
    LOG_INFO("HOST: " << h);

    int fd = open("/proc/self/cmdline", O_RDONLY);
    size_t l = read(fd, buffer, PATH_MAX);
    if (l > 0)
    {
        std::string s(buffer, l);
        std::replace(s.begin(), s.end(), '\0', ' ');
        size_t pos = 0;
        while (pos < l)
        {
            size_t length = std::min((size_t)LOGTEXTSIZE, (l - pos));
            LOG_INFO("COMMAND: " << s.substr(pos, length));
            pos += length;
        }
    }
    else
    {
        LOG_ERROR("Unable to get process info");
    }
    LOG_INFO("VER:" << GIT_VERSION);
}

} // namespace utils
} // namespace miye
