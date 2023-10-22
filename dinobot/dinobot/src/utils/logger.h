#ifndef _DINOBOT_UTILS_LOGGER_H
#define _DINOBOT_UTILS_LOGGER_H

#include <string>
#include <mutex>
#include <fstream>

namespace dinobot { namespace utils {

class logger
{
public:
    logger(std::string);
    ~logger();

    bool write_log(std::string);
    bool write_log(unsigned char *, uint32_t);
    bool write_log(const char *, uint32_t);

private:
    std::string filename_;
    std::ofstream log_;
    std::mutex mtx_;
};

}} // dinobot::utils

#endif
