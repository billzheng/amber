#pragma once

#include "spdlog/async.h"
#include "spdlog/cfg/env.h" // support for loading levels from the environment variable
#include "spdlog/fmt/ostr.h" // support for user defined types
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"

namespace miye
{
namespace logger
{

using Logger = spdlog::logger;

inline std::shared_ptr<spdlog::logger> createLogger(
    const std::string& log_filename)
{
    const bool truncate = false;
    std::shared_ptr<spdlog::logger> spdlog =
        spdlog::basic_logger_mt<spdlog::async_factory>(
            "text_logger", log_filename, truncate);

    spdlog->set_level(spdlog::level::info); // Set specific logger's log level

    // https://spdlog.docsforge.com/v1.x/3.custom-formatting/#customizing-format-using-set_pattern
    spdlog::set_pattern("%Y%m%d %H:%M:%S.%f %l %v");
    spdlog::flush_every(std::chrono::seconds(1));
    return spdlog;
}

} // namespace logger
} // namespace miye
