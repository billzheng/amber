#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>

#include "libcore/essential/assert.hpp"
#include "libcore/essential/platform_defs.hpp"
#include "libcore/math/fast-math.hpp"
#include "libcore/parsing/visit.hpp"
#include "libcore/utils/nano_time.h"

#define ENUM_UNKNOWN "unknown"

namespace miye
{
template <typename E>
constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}

using price_t    = double;
using quantity_t = double;
using symbol_t   = std::string;
using account_t  = std::string;

enum TradeDir
{
    None = 0,
    BUY  = 1,
    SELL = -1
};

enum class Side : int8_t
{
    UNKNOWN = -1,
    BUY,
    SELL,
    SHORT_SELL
};

inline const char* toString(Side side)
{
    switch (side)
    {
    case Side::BUY:
        return "BUY";
    case Side::SELL:
        return "SELL";
    case Side::SHORT_SELL:
        return "SHORT_SELL";
    default:
        return "UNKNOWN";
    }
    return "UNKNOWN";
}

inline Side fromSideStr(const std::string& v)
{
    if (v == "buy")
    {
        return Side::BUY;
    }
    if (v == "sell")
    {
        return Side::SELL;
    }
    if (v == "short_sell")
    {
        return Side::SHORT_SELL;
    }
    return Side::UNKNOWN;
}

struct trade_t
{
    price_t price{};
    quantity_t qty{};
    Side side{};
};

enum OrderStatus
{
    NEW,       // pending confirmation
    CLOSED,    // assume canceled, waiting for the trade server confirmation
    CONFIRMED, // confirmed by the market
    CANCELED,  // pending cancel
    ALTERED,   // pending cancel/replace
    NUM_STATUS
};

enum OrderType
{
    UNKNOWN,
    LIMIT,
    MARKET,
    MARKET_ON_OPEN,
    MARKET_ON_CLOSE,
    LIMIT_ON_OPEN,
    LIMIT_ON_CLOSE,
    POST_ONLY,
    HIDDEN,
    HIDDEN_POSTONLY,
    LIMIT_POSTONLY,
    OPEN_IMBALANCE_ONLY,
    CLOSE_IMBALANCE_ONLY,
    INTERNAL,
    PEGGED,
    TOP_OF_BOOK,
    LIMIT_AT_BSET,
    TOP_PLUS,
    LIMIT_TO_MOC,
    STOP,
    STOP_POSTONLY,
    TRANSFER
};

enum TIF
{
    DAY,
    GTC,
    FOK,
    IOC,
    EXTENDED,
    GFA,
    NUM_TIF
};

struct order_t
{
    symbol_t symbol{};
    uint32_t seqnum{};
    Side side{Side::UNKNOWN};
    price_t price{NAN};
    quantity_t qty{NAN};

    int32_t clientOrderId{};
    uint8_t status{};
    int32_t cid{0};
    quantity_t entrySize{0.0};
    quantity_t size{0.0};
    quantity_t canceledSize{0.0};

    price_t entryPrice{0.0};
    OrderType orderType{};
    time::NanoTime entryTime{};
    time::NanoTime updateTime{};
    quantity_t filled{0.0};
    uint64_t mktId{0}; // is exchangeId always int?

    quantity_t getCanceledSize() const { return canceledSize; }
    void setCanceledSize(quantity_t canceledSize) { this->canceledSize = canceledSize; }

    int32_t getCid() const { return cid; }
    void setCid(int32_t cid) { this->cid = cid; }

    int32_t getClientOrderId() const { return clientOrderId; }
    void setClientOrderId(int32_t clientOrderId) { this->clientOrderId = clientOrderId; }

    price_t getPrice() const { return price; }
    void setPrice(price_t price) { this->price = price; }

    price_t getEntryPrice() const { return entryPrice; }
    void setEntryPrice(price_t entryPrice) { this->entryPrice = entryPrice; }

    price_t getSize() const { return size; }
    void setSize(price_t size) { this->size = size; }

    quantity_t getEntrySize() const { return entrySize; }
    void setEntrySize(quantity_t entrySize) { this->entrySize = entrySize; }

    const time::NanoTime& getEntryTime() const { return entryTime; }
    void setEntryTime(const time::NanoTime& entryTime) { this->entryTime = entryTime; }

    quantity_t getFilled() const { return filled; }
    void setFilled(quantity_t filled) { this->filled = filled; }

    uint64_t getMarketId() const { return mktId; }
    void setMarketId(uint64_t marketId) { this->mktId = marketId; }

    quantity_t getQty() const { return qty; }
    void setQty(quantity_t qty) { this->qty = qty; }

    uint32_t getSeqnum() const { return seqnum; }
    void setSeqnum(uint32_t seqnum) { this->seqnum = seqnum; }

    Side getSide() const { return side; }
    void setSide(Side side) { this->side = side; }

    uint8_t getStatus() const { return status; }
    void setStatus(uint8_t status) { this->status = status; }

    OrderType getOrderType() const { return this->orderType; }
    void setOrderType(OrderType v) { this->orderType = v; }
    const time::NanoTime& getUpdateTime() const { return updateTime; }
    void setUpdateTime(const time::NanoTime& updateTime) { this->updateTime = updateTime; }
};

template <typename OS>
OS& operator<<(OS& os, const order_t& o)
{
    os << "cid:" << o.cid << " symbol:" << o.symbol << " seqnum:" << o.seqnum << " side:" << toString(o.side)
       << " price:" << o.price << " qty:" << o.qty << " clientOrderId:" << o.clientOrderId << " status:" << +o.status
       << " entryPrice:" << o.entryPrice << " entrySize:" << o.entrySize << " canceledSize:" << o.canceledSize
       << " filled:" << o.filled << " mktId:" << o.mktId; /* << " entryTime:" << o.entryTime
       << " updateTie:" << o.updateTime;*/

    return os;
}

} // namespace miye

namespace miye
{
namespace types
{

template <size_t LENGTH>
class fixed_string;

typedef uint8_t version_t;
typedef uint16_t session_t;
typedef uint32_t order_ref_t;
typedef uint32_t instrument_id_t;
typedef float price_t;
typedef float fp_t;
typedef uint32_t quantity_t;
typedef uint16_t strategy_id_t;
typedef uint16_t order_count_t;
typedef uint32_t sequence_t;
typedef uint64_t match_id_t;
typedef uint64_t exchange_key_t;
typedef fixed_string<13> reject_text_t;
typedef fixed_string<16> strategy_name_t;
typedef fixed_string<12> exchange_symbol_t;
typedef fixed_string<3> currency_symbol_t;
typedef fixed_string<6> currency_pair_symbol_t;
typedef types::fixed_string<8> user_name_t;
typedef types::fixed_string<16> password_t;

/*
 * session
 * 0-1023 servers
 * 0-63 processes per server
 *
 * 0/0 is reserved
 */

static const int SERVER_MAX(std::numeric_limits<uint16_t>::max() >> 6);
static const int PROCESS_MAX(static_cast<uint8_t>((std::numeric_limits<uint16_t>::max() << 10) >> 10));

inline uint16_t session_server(types::session_t session) { return session >> 6; }

inline uint16_t session_process(types::session_t session) { return ((uint16_t(session << 10)) >> 10); }

inline types::session_t make_session(uint16_t server, uint8_t process)
{
    ASSERT(server <= SERVER_MAX);
    ASSERT(process <= PROCESS_MAX);

    return types::session_t((server << 6) + process);
}

struct order_key_t
{
    using key_t = uint64_t;

    explicit order_key_t(const order_key_t::key_t key) : key(key) {}

    explicit order_key_t(const session_t session, const strategy_id_t strategy, const order_ref_t ref)
        : ref(ref), session(session), strategy(strategy)
    {
    }

    union {
        order_key_t::key_t key;
        struct
        {
            order_ref_t ref;
            session_t session;
            strategy_id_t strategy;
        };
    };

    inline bool is_initialised() const { return static_cast<bool>(ref); }

    inline order_key_t& operator++()
    {
        ++ref;
        return *this;
    }

    VISITOR
    {
        VISIT(session);
        VISIT(strategy);
        VISIT(ref);
    }

    bool operator==(const order_key_t& rhs) const { return (key == rhs.key); }
};

template <typename OStream>
OStream& operator<<(OStream& os, order_key_t k)
{
    os << "[" << session_server(k.session) << ":" << session_process(k.session) << ":" << k.strategy << ":" << k.ref
       << "]";
    return os;
}

struct global_strategy_id
{
    using key_t = uint32_t;

    //    explicit global_strategy_id()
    //    : id(0)
    //    {}

    explicit global_strategy_id(const uint32_t key) : id(key) {}

    explicit global_strategy_id(const order_key_t order_key) : session(order_key.session), strategy(order_key.strategy)
    {
    }

    explicit global_strategy_id(const session_t session, const strategy_id_t strategy_id)
        : session(session), strategy(strategy_id)
    {
    }

    union {
        global_strategy_id::key_t id;
        struct
        {
            session_t session;
            strategy_id_t strategy;
        };
    };

    uint32_t asInt() const { return id; }

    bool operator==(const global_strategy_id& rhs) const { return (id == rhs.id); }
    bool operator!=(const global_strategy_id& rhs) const { return (id != rhs.id); }
    bool operator<(const global_strategy_id& rhs) const { return (id < rhs.id); }

    VISITOR
    {
        VISIT(session);
        VISIT(strategy);
    }
};

template <typename OStream>
OStream& operator<<(OStream& os, global_strategy_id k)
{
    os << k.id << " (" << k.session << " : " << k.strategy << ")";
    return os;
}

struct order_md_key
{

    order_md_key() : key(0) {}

    order_md_key(uint64_t key) : key(key) {}

    order_md_key(uint32_t ip, uint32_t packet_id) : ip(ip), packet_id(packet_id) {}

    union {
        uint64_t key;
        struct
        {
            uint32_t ip;
            uint32_t packet_id;
        };
    };

    bool operator==(const order_md_key& rhs) const { return (key == rhs.key); }
    bool operator>(const order_md_key& rhs) const { return key > rhs.key; }
    bool operator!=(const order_md_key& rhs) const { return key != rhs.key; }

    VISITOR
    {
        VISIT(ip);
        VISIT(packet_id);
    }
};

struct signal_t
{

    struct signal_flags
    {
        static const uint32_t sell_only = 0x1; // mandatory flag set and alpha wants to sell
        static const uint32_t buy_only  = 0x2; // mandatory flag set and alpha wants to buy
    };

    fp_t value;
    uint32_t flags;

    explicit signal_t(fp_t value, uint32_t flags) : value(value), flags(flags) {}

    signal_t() : value(0.0), flags(0) {}

    void set(fp_t v, uint32_t f)
    {
        value = v;
        flags = f;
    }

    fp_t getValue() { return value; }

    void setValue(fp_t v) { value = v; }

    inline bool MandatoryOk() const
    {
        if (value > 0.0)
        {
            return allowedToBuy();
        }
        else if (value < 0.0)
        {
            return allowedToSell();
        }
        return true;
    }

    inline void SetBuyOnly() { flags |= signal_flags::buy_only; }

    inline void SetSellOnly() { flags |= signal_flags::sell_only; }

    inline bool allowedToBuy() const { return !(flags & signal_flags::sell_only); }

    inline bool allowedToSell() const { return !(flags & signal_flags::buy_only); }

    inline signal_t& operator+=(const signal_t& rhs)
    {
        this->value = this->value + rhs.value;
        this->flags |= rhs.flags;
        return *this;
    }

    inline signal_t& operator+(const signal_t& rhs)
    {
        this->value = this->value + rhs.value;
        this->flags |= rhs.flags;
        return *this;
    }

    inline signal_t& operator=(signal_t s)
    {
        value = s.value;
        flags = s.flags;
        return *this;
    }

    inline signal_t& operator*(const double rhs)
    {
        this->value = this->value * rhs;
        return *this;
    }

    inline signal_t& operator*(const float rhs)
    {
        this->value = this->value * rhs;
        return *this;
    }

    inline bool operator>(const double v) { return value > v; }

    inline bool operator<(const double v) { return value < v; }

    inline bool operator>(const float v) { return value > v; }

    inline bool operator<(const float v) { return value < v; }

    inline bool operator==(signal_t s) const
    {
        return (math::fast_fabs(value - s.value) < 0.0001) && (flags == s.flags);
    }
    inline bool operator!=(signal_t s) const { return !operator==(s); }

    VISITOR
    {
        VISIT(value);
        VISIT(flags);
    }
};

template <typename OStream>
OStream& operator<<(OStream& os, signal_t k)
{
    os << k.value << ",(" << k.flags << ")";
    return os;
}

static const session_t BROADCAST_COMMAND = 0;

#pragma pack(push, 1)

template <size_t LENGTH>
class fixed_string
{
  public:
    enum
    {
        fixed_size = LENGTH
    };

    fixed_string() { clear(); }

    fixed_string(const char* chars)
    {
        clear();
        assign(chars, std::min(strlen(chars), LENGTH));
    }

    fixed_string(std::string const& rhs)
    {
        clear();
        assign(rhs.c_str(), rhs.size());
    }

    fixed_string& operator=(fixed_string const& rhs)
    {
        clear();
        return assign(rhs.c_str(), rhs.capacity());
    }

    fixed_string& operator=(const char* rhs)
    {
        clear();
        return assign(rhs, strlen(rhs));
    }

    fixed_string& operator=(const std::string& rhs)
    {
        clear();
        return assign(rhs.c_str(), rhs.size());
    }

    inline void clear() { ::memset(data, char(0), LENGTH); }

    inline fixed_string& assign(const char* chars, size_t n)
    {
        memcpy(data, chars, std::min(n, LENGTH));
        return *this;
    }

    size_t capacity() const { return fixed_size; }

    bool empty() const { return data[0] == 0; }

    size_t size() const
    {
        if (data[fixed_size - 1] != 0)
        {
            return fixed_size;
        }
        size_t size = 0;
        while (size < fixed_size && data[size] != 0)
        {
            size++;
        }
        return size;
    }

    std::string str() const { return std::string(data, size()); }

    std::string rtrimstr() const
    {
        size_t i = 0;
        while (!std::isspace(data[i]))
        {
            i++;
            if (i > size())
            {
                break;
            }
        }
        return std::string(data, i);
    }
    const char* c_str() const { return data; }

    char operator[](size_t pos) const
    {
        ASSERT(pos < LENGTH);
        return data[pos];
    }

    char& operator[](size_t pos)
    {
        ASSERT(pos < LENGTH);
        return data[pos];
    }
    char data[LENGTH];
};
#pragma pack(pop)

template <size_t LENGTH>
inline bool operator==(char const* lhs, const fixed_string<LENGTH>& rhs)
{
    return !strncmp(rhs.data, lhs, rhs.size());
}

template <size_t LENGTH>
inline bool operator==(const fixed_string<LENGTH>& lhs, std::string& rhs)
{
    return !strncmp(lhs.data, rhs.c_str(), lhs.size());
}

template <size_t LENGTH>
inline bool operator==(const std::string& lhs, const fixed_string<LENGTH>& rhs)
{
    return !strncmp(rhs.data, lhs.c_str(), rhs.size());
}

template <size_t LENGTH>
inline bool operator==(const fixed_string<LENGTH>& lhs, char const* rhs)
{
    return !strncmp(lhs.data, rhs, lhs.size());
}

template <size_t LENGTH>
inline bool operator==(const fixed_string<LENGTH>& lhs, const fixed_string<LENGTH>& rhs)
{
    return !memcmp(lhs.c_str(), rhs.c_str(), LENGTH);
}

template <typename OStream, size_t LENGTH>
OStream& operator<<(OStream& os, fixed_string<LENGTH> const& s)
{
    os << s.str();
    return os;
}

template <size_t Size>
class fixedstring_ostreambuf : public std::basic_streambuf<char>
{
  public:
    fixedstring_ostreambuf(fixed_string<Size>& p_string) : p_string(p_string), bytes_written(0) {}
    virtual ~fixedstring_ostreambuf() {}

    std::streamsize xsputn(char_type const* s, std::streamsize n)
    {
        if (bytes_written + n <= Size)
        {
            for (std::streamsize i = 0; i < n; ++i, ++bytes_written)
                p_string[bytes_written] = s[i];
            return n;
        }
        return 0;
    }

    int_type overflow(int_type c)
    {
        if (bytes_written + 1 < Size && c != EOF)
        {
            p_string[bytes_written++] = c;
            return c;
        }
        ASSERT(false);
        return EOF;
    }

  private:
    fixed_string<Size>& p_string;
    size_t bytes_written;
};

template <size_t Size>
struct fixedstring_ostream : public std::ostream
{
  public:
    fixedstring_ostream(fixed_string<Size>& p_str) : std::ostream(&buffer), buffer(p_str) {}

  private:
    fixedstring_ostreambuf<Size> buffer;
};

} // namespace types
} // namespace miye

namespace std
{

// note full length is used as this can be used for binary data - do not stop
// when '0' is hit
template <size_t N>
struct hash<miye::types::fixed_string<N>>
{
  public:
    size_t operator()(const miye::types::fixed_string<N>& rhs) const
    {
        size_t result = 0;
        const char* p = rhs.c_str();
        for (const char* e = p + N; p != e; ++p)
            result = (result * 131) + *p;
        return result;
    }
};
} // namespace std

namespace std
{

// note full length is used as this can be used for binary data - do not stop
// when '0' is hit
template <>
struct hash<miye::types::global_strategy_id>
{
  public:
    size_t operator()(const miye::types::global_strategy_id& rhs) const { return rhs.id; }
};
} // namespace std
