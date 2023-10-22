#include "../../utils/date.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "../../libs/json/fast/sajson.h"
#include "../../libs/json/fast/json_helper.h"
#pragma GCC diagnostic pop

namespace dinobot { namespace exchange { namespace bitmex {

using price_t = double;
using sz_t = long;
using id_t = uint64_t;

/*
 * These are all the defined keys strings we will use in these messages,
 * defined here to speed up the parsing of the messages
 */
class cache
{
public:
    static inline const sajson::string id = sajson::string("id", 2);
    static inline const sajson::string data = sajson::string("data", 4);
    static inline const sajson::string side = sajson::string("side", 4);
    static inline const sajson::string size = sajson::string("size", 4);
    static inline const sajson::string table = sajson::string("table", 5);
    static inline const sajson::string price = sajson::string("price", 5);
    static inline const sajson::string action = sajson::string("action", 6);
    static inline const sajson::string symbol = sajson::string("symbol", 6);
    static inline const sajson::string bidsz = sajson::string("bidSize", 7);
    static inline const sajson::string asksz = sajson::string("askSize", 7);
    static inline const sajson::string bidpx = sajson::string("bidPrice", 8);
    static inline const sajson::string askpx = sajson::string("askPrice", 8);
    static inline const sajson::string timestamp = sajson::string("timestamp", 9);
    static inline const sajson::string trdMatchID = sajson::string("trdMatchID", 10);
    static inline const sajson::string grossValue = sajson::string("grossValue", 10);
    static inline const sajson::string homeNotional = sajson::string("homeNotional", 12);
    static inline const sajson::string tickDirection = sajson::string("tickDirection", 13);
    static inline const sajson::string foreignNotional = sajson::string("foreignNotional", 15);
    //maybe don't need below ??
    static inline const sajson::string partial = sajson::string("partial", 7);
    static inline const sajson::string insert = sajson::string("insert", 6);
    static inline const sajson::string update = sajson::string("update", 6);
    static inline const sajson::string delete_s = sajson::string("delete", 6);
};


// treat the uuid as 2 uints 
struct orderid_t 
{
    uint64_t upper;
    uint64_t lower;
};

enum class tick_dir_t : uint8_t
{
    PlusTick        = 0,
    MinusTick       = 1,
    ZeroPlusTick    = 2,
    ZeroMinusTick   = 3,
};

enum class side_t : char
{
    bid = 'b',
    ask = 'a',
};
side_t build_side_t(const sajson::value &node, const sajson::string &str)
{
    auto  s = node.get_value_of_key(str).as_string();
    if (s == "Sell")
	return side_t::ask;
    return side_t::bid;
}
char read_side_t(side_t s)
{
    if (s == side_t::bid)
        return 'b';
    return 'a';
}

enum class action_t : char 
{
    del = 'd', // delete action (but i can't use delete as a key workd
    insert = 'i',
    update = 'u',
};

enum class symbol_t : uint16_t
{
    XBTUSD = 0,
    ETHUSD = 1,
};



std::chrono::system_clock::time_point parse_date(std::string ss)
{
    using namespace std::chrono;
    using namespace date;

    std::stringstream datestr(ss);

#ifdef __APPLE__
    date::sys_time<std::chrono::microseconds> tp;
#else
    date::sys_time<std::chrono::nanoseconds> tp; // mac doesn't seems to have nanos currently
#endif

    datestr >> date::parse("%FT%T", tp);
    if (datestr.fail())
        std::cout << "Could not parse date time string, exiting " << std::endl;
    return tp;
}

// {"table":"quote","action":"insert","data":[{"timestamp":"2018-10-06T06:27:22.745Z","symbol":"ETHUSD","bidSize":130388,"bidPrice":225.8,"askPrice":225.85,"askSize":338939}]}
struct quote_insert
{
    uint64_t receive_time;
    std::chrono::system_clock::time_point timestamp;
    symbol_t symbol;
    sz_t bidSize;
    sz_t askSize;
    price_t bidPrice;
    price_t askPrice;
};
void parse_quote_insert(const sajson::value &node, uint64_t ts)
{
    using namespace dinobot::libs::json;
    bool partial = false;

    if (node.get_value_of_key(cache::action).as_string() ==  "partial")
        partial = true;
    
    // data is an array we need to loop through ... 
    auto datas = node.get_value_of_key(cache::data);
    auto length = datas.get_length();
    for (auto i = 0u ; i < length ; ++i)
    {
        quote_insert msg; // TODO this is only to test parsing ... 
        auto quote = datas.get_array_element(i);
        msg.receive_time = ts;
        msg.symbol = symbol_t::XBTUSD; // XXX
        msg.timestamp = parse_date(quote.get_value_of_key(cache::timestamp).as_string());
        msg.bidSize = quote.get_value_of_key(cache::bidsz).get_integer_value();
        msg.bidPrice = get_double_value(quote.get_value_of_key(cache::bidpx));
        msg.askSize = quote.get_value_of_key(cache::asksz).get_integer_value();
        msg.askPrice = get_double_value(quote.get_value_of_key(cache::askpx));

        if (partial) // don't like this here in this loop but for now OK
        {
            ;// TODO 
        }
    }
}

// {"table":"trade","action":"insert","data":[{"timestamp":"2018-10-04T11:56:11.873Z","symbol":"XBTUSD","side":"Sell","size":100,"price":6535.5,"tickDirection":"ZeroMinusTick","trdMatchID":"ace25670-cbe6-5cda-b1df-1a8fb38cd7cc","grossValue":1530100,"homeNotional":0.015301,"foreignNotional":100}]}
struct  trade_insert
{
    uint64_t receive_time;
    std::chrono::system_clock::time_point timestamp;
    symbol_t symbol;
    side_t side;
    sz_t size;
    price_t price;
    tick_dir_t tickDirection; 
    orderid_t trdMatchID;
    double grossValue; 
    double homeNotional;
    double foreignNotional;
};
void parse_trade_insert(const sajson::value &node, uint64_t ts)
{
    using namespace dinobot::libs::json;
    bool partial = false;

    if (node.get_value_of_key(cache::action).as_string() ==  "partial")
        partial = true;
    
    // data is an array we need to loop through ... 
    auto datas = node.get_value_of_key(cache::data);
    auto length = datas.get_length();
    for (auto i = 0u ; i < length ; ++i)
    {
	    trade_insert msg; // can probablly use a n already fdefined class struct to save an allocation here
    	auto trade = datas.get_array_element(i);
        msg.receive_time = ts;
        msg.symbol = symbol_t::XBTUSD; // XXX
        msg.timestamp = parse_date(trade.get_value_of_key(cache::timestamp).as_string());
	    msg.side = build_side_t(trade, cache::side);
    	msg.size = trade.get_value_of_key(cache::size).get_integer_value();
    	msg.price = get_double_value(trade.get_value_of_key(cache::price));
    	msg.tickDirection = tick_dir_t::ZeroMinusTick; // XXX
    	// XXX msg.trdMatchID = get some code to play with uuids 
        msg.grossValue = get_64bit_value(trade.get_value_of_key(cache::grossValue));
    	msg.homeNotional = get_double_value(trade.get_value_of_key(cache::homeNotional));
    	msg.foreignNotional = get_double_value(trade.get_value_of_key(cache::foreignNotional));

        if (partial) // don't liek this, but will do for now 
        {
            ;// we have parsed 
        }
    } 
}

//{"table":"orderBookL2","action":"insert","data":[{"symbol":"XBTUSD","id":8799997500,"side":"Buy","size":31,"price":25}]}
struct obl2_insert
{
    uint64_t receive_time;
    symbol_t symbol; 
    id_t id; 
    side_t side;
    sz_t size; 
    price_t price;
};
void parse_orderbook_insert(const sajson::value &node, uint64_t ts)
{
    using namespace dinobot::libs::json;

    // data is an array we need to loop through ... 
    auto datas = node.get_value_of_key(cache::data);
    auto length = datas.get_length();
    for (auto i = 0u ; i < length ; ++i)
    {
	    obl2_insert msg; // can probablly use an already defined class struct to save an allocation here
    	auto insert = datas.get_array_element(i);
        msg.receive_time = ts;
        msg.symbol = symbol_t::XBTUSD; // XXX
        msg.id = get_64bit_value(insert.get_value_of_key(cache::id));
        msg.side = build_side_t(insert, cache::side);
        msg.size = get_64bit_value(insert.get_value_of_key(cache::size));
        msg.price = get_double_value(insert.get_value_of_key(cache::price));
    }
}


//{"table":"orderBookL2","action":"update","data":[{"symbol":"XBTUSD","id":8799345950,"side":"Sell","size":142598}]}
struct obl2_update
{
    uint64_t receive_time;
    symbol_t symbol;
    id_t id;
    side_t side;
    sz_t size; 
};
void parse_orderbook_update(const sajson::value &node, uint64_t ts)
{
    using namespace dinobot::libs::json;

    // data is an array we need to loop through ... 
    auto datas = node.get_value_of_key(cache::data);
    auto length = datas.get_length();
    for (auto i = 0u ; i < length ; ++i)
    {
	    obl2_update msg; // can probablly use an already defined class struct to save an allocation here
    	auto update = datas.get_array_element(i);
        msg.receive_time = ts;
        msg.symbol = symbol_t::XBTUSD; // XXX
        msg.id = get_64bit_value(update.get_value_of_key(cache::id));
        msg.side = build_side_t(update, cache::side);
        msg.size = get_64bit_value(update.get_value_of_key(cache::size));
    }
}


//{"table":"orderBookL2","action":"delete","data":[{"symbol":"XBTUSD","id":8799997600,"side":"Buy"}]}
struct obl2_delete
{
    uint64_t receive_time;
    symbol_t symbol;
    id_t id;
    side_t side;
};
void parse_orderbook_delete(const sajson::value &node, uint64_t ts)
{
    using namespace dinobot::libs::json;

    // data is an array we need to loop through ... 
    auto datas = node.get_value_of_key(cache::data);
    auto length = datas.get_length();
    for (auto i = 0u ; i < length ; ++i)
    {
	    obl2_delete msg; // can probablly use an already defined class struct to save an allocation here
    	auto del = datas.get_array_element(i);
        msg.receive_time = ts;
        msg.symbol = symbol_t::XBTUSD; // XXX
        msg.id = get_64bit_value(del.get_value_of_key(cache::id));
        msg.side = build_side_t(del, cache::side);
    }
}

// {"symbol":"XBTZ18","id":29099900002,"side":"Sell","size":499999,"price":49999},
struct obl2_partial 
{
    symbol_t symbol;
    id_t id;
    side_t side;
    sz_t size;
    price_t price;
};
void parse_orderbook_partial(const sajson::value &node)
{
    using namespace dinobot::libs::json;
    std::vector<obl2_partial> bids;
    std::vector<obl2_partial> asks;

    // data is an array we need to loop through ... 
    auto datas = node.get_value_of_key(cache::data);
    auto length = datas.get_length();
    for (auto i = 0u ; i < length ; ++i)
    {
	    obl2_partial msg; // can probablly use an already defined class struct to save an allocation here
    	auto partial = datas.get_array_element(i);
        msg.symbol = symbol_t::XBTUSD; // XXX
        msg.id = get_64bit_value(partial.get_value_of_key(cache::id));
        msg.side = build_side_t(partial, cache::side);
        msg.size = get_64bit_value(partial.get_value_of_key(cache::size));
        msg.price = get_double_value(partial.get_value_of_key(cache::price));

        if (msg.side == side_t::bid)
            bids.push_back(msg);
        else
            asks.push_back(msg);
    }
    // let's create a list of order book ids 
    // so we can build the book 
    for (auto &a:bids)
        std::cout << "id: " << a.id<< " side: "<<read_side_t(a.side) << " price: "<<a.price << std::endl;
    for (auto &a:asks)
        std::cout << "id: " << a.id<< " side: "<<read_side_t(a.side) << " price: "<<a.price << std::endl;
}


}}} // dinobot::exhcange::bitmex 
