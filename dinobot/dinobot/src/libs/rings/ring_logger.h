#ifndef _DINOBOT_RING_LOGGER_H 
#define _DINOBOT_RING_LOGGER_H 
#include <thread>
#include <fstream>
#include "ring_reader.h"
#include "ring_reader.h"

namespace dinobot { namespace lib { namespace shm {

class ring_logger 
{
public:
    ring_logger(std::string, std::string, uint32_t);
    ~ring_logger();

    // 
    void start();
    void stop();

    // starting a thread proces 
    std::thread start_thread() {
        return std::thread([=] { start(); });
    }
    
private:
    bool running_;
    std::string fn_;
    std::string log_fn_;
    uint32_t reader_id_;
    std::ofstream log_file_;

    dinobot::lib::shm::ring_reader *log_;
};


} } } /// dinobot

#endif
