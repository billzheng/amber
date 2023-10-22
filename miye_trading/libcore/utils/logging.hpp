/*
 * logging.hpp
 *
 * Purpose: provide runtime logging
 *
 * Author:
 *
 */

#pragma once

#include <limits>
#include <unordered_set>
#include <vector>

#include "libcore/essential/platform_defs.hpp"
#include "libcore/essential/utils.hpp"
#include "libcore/qstream/variantqstream_writer.hpp"
#include "libcore/time/clock.hpp"
#include "libcore/time/time.hpp"
#include "libcore/utils/logging_def.hpp"
//#include "message/message.hpp"
//#include "trading/engine/app_globals.hpp"

namespace miye
{
namespace utils
{

template <bool ts_size>
class fixed_size_ostreambuf : public std::basic_streambuf<char>
{
  public:
    explicit fixed_size_ostreambuf(char* buffer, size_t size, bool has_internal_size, size_t internal_size_offset)
        : buffer_size(size - ((!!ts_size) * 16)), bytes_written(0), buffer_(buffer + (!!ts_size) * 16),
          has_internal_size(has_internal_size), internal_size_offset(internal_size_offset)
    {
    }
    size_t capacity() const { return buffer_size; }
    void reset() { bytes_written = 0; }
    char const* buffer() const { return buffer_; }
    size_t size() const { return bytes_written; }
    void add_new_line()
    {
        sputc('\n');
        bytes_written++;
    }
    void add_null()
    {
        sputc('0');
        bytes_written++;
    }
    void set_payload_size()
    {
        if (has_internal_size)
        {
            ASSERT(bytes_written > sizeof(size_t));
            size_t internal_size = size_t(bytes_written - internal_size_offset - sizeof(size_t));
            memcpy(buffer_ + internal_size_offset, &internal_size, sizeof(size_t));
        }
    }
    std::streamsize xsputn(char_type const* s, std::streamsize n)
    {
#define FMT(z) #z " is: " << z << " "
        if (bytes_written + n > buffer_size)
        {
            std::cerr << time::as_utc(time::system_nanos()) << " Warning: log write truncated\n";

            n = buffer_size - bytes_written;
        }
        if (bytes_written + n <= buffer_size)
        {
            memcpy(buffer_ + bytes_written, s, n);
            bytes_written += n;
            return n;
        }
        ASSERT(false);
        return 0;
    }
    int_type overflow(int_type c)
    {
        ASSERT_MSG(bytes_written + 1 < buffer_size && c != EOF, FMT(bytes_written) << FMT(buffer_size) << FMT(c));
#undef FMT
        if (bytes_written + 1 < buffer_size && c != EOF)
        {
            *(buffer_ + bytes_written) = c;
            return c;
        }
        return EOF;
    }

  private:
    size_t buffer_size;
    size_t bytes_written;
    char* buffer_;
    bool has_internal_size;
    size_t internal_size_offset;
};

#ifdef USE_STREAMING_WRITES
typedef utils::fixed_size_ostreambuf<true> logging_buffer;
#else
typedef utils::fixed_size_ostreambuf<false> logging_buffer;
#endif

extern uint64_t last_primary_write;

template <typename Clock_Type>
class logger
{
  public:
    explicit logger(Clock_Type& clk) : clk(clk), primary_(nullptr) {}

    void add_log_sink(log_level_t level, qstream::variantqstream_writer<Clock_Type>& sink, bool primary = false)
    {
        log_sinks_.push_back(std::make_tuple(level, sink, primary));
    }

    void write_to_primary(const void* p, size_t n)
    {
        for (auto l = log_sinks_.begin(); l != log_sinks_.end(); ++l)
        {
            if (std::get<2>(*l))
            {
                std::get<1>(*l).write(p, n);
                last_primary_write = clk.now();
            }
        }
    }

    NEVER_INLINE void write_to_primary(types::session_t target, types::message::command_t type,
                                       types::message::command_direction_t direction,
                                       types::message::data_type_t data_type, types::message::command_creator_t creator,
                                       types::user_name_t user, const void* data, size_t size)
    {
        // xxx check alloca
        types::message::command* cmd =
            new (alloca(types::message::command::msg_header_size + size)) types::message::command(
                trading::session, target, type, direction, creator, data_type, user, trading::command_id++, data, size);

        for (auto l = log_sinks_.begin(); l != log_sinks_.end(); ++l)
        {
            if (std::get<2>(*l))
            {
                std::get<1>(*l).write(cmd, types::message::command::msg_header_size + size);
                last_primary_write = clk.now();
            }
        }
    }

    NEVER_INLINE void write_to_primary(types::message::meta_type_t type, types::message::data_type_t data_type,
                                       const void* data, size_t size)
    {
        // xxx check alloca
        types::message::meta* cmd = new (alloca(types::message::meta::msg_header_size + size))
            types::message::meta(trading::session, type, data_type, data, size);

        for (auto l = log_sinks_.begin(); l != log_sinks_.end(); ++l)
        {
            if (std::get<2>(*l))
            {
                std::get<1>(*l).write(cmd, types::message::meta::msg_header_size + size);
                last_primary_write = clk.now();
            }
        }
    }

    void log_binary(log_level_t level, logging_buffer& buffer)
    {
        buffer.set_payload_size();
        for (auto l = log_sinks_.begin(); l != log_sinks_.end(); ++l)
        {
            if (level <= std::get<0>(*l))
                std::get<1>(*l).write(buffer.buffer(),
                                      std::min(buffer.size(),
                                               (size_t)qstream::mmap_max_recordsize - 32)); // TODO fix constant
        }
    }

    void list()
    {
        for (auto l = log_sinks_.begin(); l != log_sinks_.end(); ++l)
        {
            std::cerr << std::get<0>(*l) << " " << std::get<1>(*l).describe() << " primary=" << std::get<2>(*l) << "\n";
        }
    }

  private:
    Clock_Type& clk;
    std::vector<std::tuple<log_level_t, qstream::variantqstream_writer<Clock_Type>, bool>> log_sinks_;
    qstream::variantqstream_writer<Clock_Type>* primary_;
};

struct any_logger
{
  public:
    any_logger() : clock_type(time::clock_type_t::undefined), default_clock(time::variant_clock("event_clock"))
    {
        ::memset(store, 0, sizeof(store));
        init(default_clock);
    }

    ~any_logger() { clear(); }

    void init(time::variant_clock& clk)
    {
        clear();
        clock_type = time::clock_type_t::variant_clock;
        any        = new (store) any_logger_t(clk);
    }

    void init(time::event_clock& clk)
    {
        clear();
        clock_type = time::clock_type_t::event_clock;
        event      = new (store) event_logger_t(clk);
    }

    void init(time::real_clock& clk)
    {
        clear();
        clock_type = time::clock_type_t::real_clock;
        real       = new (store) real_logger_t(clk);
    }

    void init(time::replay_clock& clk)
    {
        clear();
        clock_type = time::clock_type_t::replay_clock;
        replay     = new (store) replay_logger_t(clk);
    }

    void write_to_primary(const void* p, size_t n)
    {
        switch (clock_type)
        {
        case time::clock_type_t::variant_clock:
            any->write_to_primary(p, n);
            break;
        case time::clock_type_t::event_clock:
            event->write_to_primary(p, n);
            break;
        case time::clock_type_t::real_clock:
            real->write_to_primary(p, n);
            break;
        case time::clock_type_t::replay_clock:
            replay->write_to_primary(p, n);
            break;

        default:
            ASSERT(false);
            break;
        }
    }

    void write_to_primary(types::session_t target, types::message::command_t type,
                          types::message::command_direction_t direction, types::message::data_type_t data_type,
                          types::message::command_creator_t creator, types::user_name_t user, const void* data,
                          size_t size)

    {
        switch (clock_type)
        {
        case time::clock_type_t::variant_clock:
            any->write_to_primary(target, type, direction, data_type, creator, user, data, size);
            break;
        case time::clock_type_t::event_clock:
            event->write_to_primary(target, type, direction, data_type, creator, user, data, size);
            break;
        case time::clock_type_t::real_clock:
            real->write_to_primary(target, type, direction, data_type, creator, user, data, size);
            break;
        case time::clock_type_t::replay_clock:
            replay->write_to_primary(target, type, direction, data_type, creator, user, data, size);
            break;
        default:
            ASSERT(false);
            break;
        }
    }

    void write_to_primary(types::message::meta_type_t type, types::message::data_type_t data_type, const void* data,
                          size_t size)
    {
        switch (clock_type)
        {
        case time::clock_type_t::variant_clock:
            any->write_to_primary(type, data_type, data, size);
            break;
        case time::clock_type_t::event_clock:
            event->write_to_primary(type, data_type, data, size);
            break;
        case time::clock_type_t::real_clock:
            real->write_to_primary(type, data_type, data, size);
            break;
        case time::clock_type_t::replay_clock:
            replay->write_to_primary(type, data_type, data, size);
            break;
        default:
            ASSERT(false);
            break;
        }
    }

    void log_binary(log_level_t level, logging_buffer& buffer)
    {
        switch (clock_type)
        {
        case time::clock_type_t::variant_clock:
            any->log_binary(level, buffer);
            break;
        case time::clock_type_t::event_clock:
            event->log_binary(level, buffer);
            break;
        case time::clock_type_t::real_clock:
            real->log_binary(level, buffer);
            break;
        case time::clock_type_t::replay_clock:
            replay->log_binary(level, buffer);
            break;
        default:
            ASSERT(false);
            break;
        }
    }

    void add_log_sink(log_level_t level, qstream::variantqstream_writer<time::variant_clock>& sink,
                      bool primary = false)
    {
        ASSERT(clock_type == time::clock_type_t::variant_clock);
        any->add_log_sink(level, sink, primary);
    }

    void add_log_sink(log_level_t level, qstream::variantqstream_writer<time::event_clock>& sink, bool primary = false)
    {
        ASSERT(clock_type == time::clock_type_t::event_clock);
        event->add_log_sink(level, sink, primary);
    }

    void add_log_sink(log_level_t level, qstream::variantqstream_writer<time::real_clock>& sink, bool primary = false)
    {
        ASSERT(clock_type == time::clock_type_t::real_clock);
        real->add_log_sink(level, sink, primary);
    }

    void list()
    {
        switch (clock_type)
        {
        case time::clock_type_t::variant_clock:
            any->list();
            break;
        case time::clock_type_t::event_clock:
            any->list();
            break;
        case time::clock_type_t::real_clock:
            any->list();
            break;
        case time::clock_type_t::replay_clock:
            any->list();
            break;
        default:
            ASSERT(false);
            break;
        }
    }

  private:
    time::clock_type_t clock_type;
    time::variant_clock default_clock;

    typedef logger<time::event_clock> event_logger_t;
    typedef logger<time::real_clock> real_logger_t;
    typedef logger<time::replay_clock> replay_logger_t;
    typedef logger<time::variant_clock> any_logger_t;

    union {
        event_logger_t* event;
        real_logger_t* real;
        replay_logger_t* replay;
        any_logger_t* any;
    };

    char store[essential::max_size<event_logger_t, real_logger_t, any_logger_t>::size()];

    void clear() { clock_type = time::clock_type_t::undefined; }
};

void add_logging_sink(std::string const&);
// void add_logging_options(boost::program_options::options_description& args,
// bool use_default);
void log_process_details();

extern any_logger logger_instance;
extern std::vector<std::string> log_opts;
extern char log_buffer[];

template <typename clock_type>
void process_logging_options(clock_type& clk)
{
    logger_instance.init(clk);

    struct log_item
    {
        log_item(std::string url, log_level_t level, bool primary) : url(url), level(level), primary(primary) {}

        std::string url;
        log_level_t level;
        bool primary;
    };

    bool primary_defined = false;

    std::vector<log_item> urls;

    for (auto o = log_opts.begin(); o != log_opts.end(); ++o)
    {
        bool primary = false;

        // a valid logging option with look like: '(primary) url @ log-level'
        std::istringstream iss(*o);
        std::string url;
        std::string sep;
        std::string level;
        iss >> url;
        if (!iss)
        {
            INVARIANT_FAIL("Invalid logging option: " << *o << " must be of the form '(primary) url @ log-level'");
        }
        if (url == "primary")
        {
            if (primary_defined)
            {
                INVARIANT_FAIL("Primary log already defined");
            }
            primary         = true;
            primary_defined = true;
            iss >> url;
        }
        iss >> sep;
        if (!iss || sep != "@")
        {
            INVARIANT_FAIL("Invalid logging option: " << *o << " must be of the form '(primary) url @ log-level'");
        }
        iss >> level;
        if (!iss)
        {
            INVARIANT_FAIL("Invalid logging option: " << *o << " must be of the form '(primary) url @ log-level'");
        }
        log_level_t l = log_level_t::info;
        if (level == "error")
        {
            l = log_level_t::error;
        }
        else if (level == "warning")
        {
            l = log_level_t::warning;
        }
        else if (level == "ui-info")
        {
            l = log_level_t::ui_info;
        }
        else if (level == "info")
        {
            l = log_level_t::info;
        }
        else if (level == "debug")
        {
            l = log_level_t::debug;
        }
        else if (level == "trace")
        {
            l = log_level_t::trace;
        }
        std::cerr << "Defining " << url << " " << l << " as primary\n";
        urls.emplace_back(url, l, primary);
    }
    if (!primary_defined && urls.size() > 0)
    {
        std::cout << "Making " << urls[0].url << " the primary logfile\n";
        urls[0].primary = true;
        primary_defined = true;
    }

    for (auto& u : urls)
    {
        qstream::variantqstream_writer<clock_type> log_stream(clk, u.url);
        logger_instance.add_log_sink(u.level, log_stream, u.primary);
    }
    if (!primary_defined)
    {
        INVARIANT_FAIL("Invalid logging set up. A log must be defined as "
                       "primary : i.e. of the form 'primary url @ log-level'");
    }
}

} // namespace utils
} // namespace miye

#define CONCAT(a, b) a##b
#define CONCAT2(a, b) CONCAT(a, b)

#define LOG(level, ...)                                                                                                \
    do                                                                                                                 \
    {                                                                                                                  \
        utils::logging_buffer __buffer(utils::log_buffer, LOGBUFSIZE, true, types::message::log::size_offset);         \
        types::message::log __m(trading::session, level);                                                              \
        __buffer.xsputn(reinterpret_cast<char*>(&__m), types::message::log::msg_header_size);                          \
        std::ostream stream(&__buffer);                                                                                \
        stream << __VA_ARGS__ << std::flush;                                                                           \
        utils::logger_instance.log_binary(level, __buffer);                                                            \
    } while (false)

#define LOGEX(level, source, value, ...)                                                                               \
    do                                                                                                                 \
    {                                                                                                                  \
        utils::logging_buffer __buffer(utils::log_buffer, LOGBUFSIZE, true, types::message::log_ex::size_offset);      \
        types::message::log_ex __m(trading::session, level, source, value);                                            \
        __buffer.xsputn(reinterpret_cast<char*>(&__m), types::message::log_ex::msg_header_size);                       \
        std::ostream stream(&__buffer);                                                                                \
        stream << __VA_ARGS__ << std::flush;                                                                           \
        utils::logger_instance.log_binary(level, __buffer);                                                            \
    } while (false)

#define LOG_WRITE(p, n) utils::logger_instance.write_to_primary(p, n)
#define LOG_WRITE_COMMAND(target, type, direction, data_type, creator, user, data, size)                               \
    utils::logger_instance.write_to_primary(target, type, direction, data_type, creator, user, data, size)
#define LOG_WRITE_META(type, data_type, data, size) utils::logger_instance.write_to_primary(type, data_type, data, size)
#define LOG_WRITE_PROCESS_META(l)                                                                                      \
    do                                                                                                                 \
    {                                                                                                                  \
        {                                                                                                              \
            types::message::process_meta _pm;                                                                          \
            _pm.set_process_data(l);                                                                                   \
            std::ostringstream _oss;                                                                                   \
            _oss << parsing::json(_pm, true, 10);                                                                      \
            utils::logger_instance.write_to_primary(types::message::meta_type_t::process,                              \
                                                    types::message::data_type_t::json,                                 \
                                                    _oss.str().c_str(),                                                \
                                                    _oss.str().size());                                                \
        }                                                                                                              \
    } while (false)

#define LOG_WRITE_PROCESS_CONFIG_META(tsc)                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        {                                                                                                              \
            std::ostringstream _oss;                                                                                   \
            _oss << parsing::json(tsc, true);                                                                          \
            utils::logger_instance.write_to_primary(types::message::meta_type_t::process_config,                       \
                                                    types::message::data_type_t::json,                                 \
                                                    _oss.str().c_str(),                                                \
                                                    _oss.str().size());                                                \
        }                                                                                                              \
    } while (false)

#define LOG_WRITE_MANIFEST()                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        {                                                                                                              \
            {                                                                                                          \
                std::ostringstream oss;                                                                                \
                message::manifest_meta manifest;                                                                       \
                manifest.load();                                                                                       \
                oss.str(std::string());                                                                                \
                oss << parsing::json(manifest, true);                                                                  \
                std::string str = oss.str();                                                                           \
                LOG_WRITE_META(message::meta_type_t::manifest, message::data_type_t::json, str.c_str(), str.size());   \
            }                                                                                                          \
        }                                                                                                              \
    } while (false)

#define LOG_TRACE(...)                                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
    } while (0)
#define LOG_SCOPE_TRACE(...)                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
    } while (0)
#define LOG_DEBUG(...) LOG(utils::log_level_t::debug, __VA_ARGS__)
#define LOG_SCOPE_DEBUG(...) LOG_SCOPE(utils::log_level_t::debug, __VA_ARGS__)
#define LOG_INFO(...) LOG(utils::log_level_t::info, __VA_ARGS__)
#define LOG_SCOPE_INFO(...) LOG_SCOPE(utils::log_level_t::info, __VA_ARGS__)
#define LOG_UI_INFO(...) LOG(utils::log_level_t::ui_info, __VA_ARGS__)
#define LOG_SCOPE_UI_INFO(...) LOG_SCOPE(utils::log_level_t::ui_info, __VA_ARGS__)
#define LOG_WARNING(...) LOG(utils::log_level_t::warning, __VA_ARGS__)
#define LOG_SCOPE_WARNING(...) LOG_SCOPE(utils::log_level_t::warning, __VA_ARGS__)
#define LOG_ERROR(...) LOG(utils::log_level_t::error, __VA_ARGS__)
#define LOG_SCOPE_ERROR(...) LOG_SCOPE(utils::log_level_t::error, __VA_ARGS__)

#define LOGEX_INFO(source, value, ...) LOGEX(utils::log_level_t::info, source, value, __VA_ARGS__)
#define LOGEX_UI_INFO(source, value, ...) LOGEX(utils::log_level_t::ui_info, source, value, __VA_ARGS__)
#define LOGEX_WARNING(source, value, ...) LOGEX(utils::log_level_t::warning, source, value, __VA_ARGS__)
#define LOGEX_ERROR(source, value, ...) LOGEX(utils::log_level_t::error, source, value, __VA_ARGS__)
