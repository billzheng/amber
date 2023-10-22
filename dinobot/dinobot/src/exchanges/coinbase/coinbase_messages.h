#ifndef _COINBASE_EXCHANGE_MESSAGES_H
#define _COINBASE_EXCHANGE_MESSAGES_H

#include <vector>
#include <chrono>
#include <ostream>
#include <iostream>

#include "../exchanges.h"

namespace exchange { namespace coinbase {

    enum class side_type : char
    {
        buy = 'b',
        sell = 's',
    };

	enum class order_type_t : char
	{
		limit = 'l',
		market = 'm', 
		stop  = 's',
	};

	enum class order_flight_type : char
	{
		GTC = 'g',	// good til cancel
	   	GTT = 't',	// good til time
		IOC = 'i',	// immediate or cancel
		FOK = 'f',	// fill or kill
	};

	enum class status_type : char
	{
		open = 'o',
		pending= 'p',
		active= 'a',
		done = 'd',
		settled = 's',
		outstanding = 'g',
		rejected = 'r',
	};

	enum class stp_type : char
	{
		dc = 'd',	// decrease and cancel 
		co = 'o',	// cancel oldest
		cn = 'n',	// cancel newest
		cb = 'b',	// cancel both
	};

	enum class currency_type : char
	{
		BTC = 'b',	// BITCOIN
		EUR = 'r',	// Euro
		LTC = 'l',	// Litecoin
		GBP = 'g',	// British pound
		USD = 'u',	// US Dollar
		ETH = 'e',	// Etherium
		BCH = 'c',	// BITCOIN CASH
	};

	enum class product_id_type : char
	{
		LTCEUR = 'a',
		LTCUSD = 'b',
		LTCBTC = 'c',
		ETHEUR = 'd',
		ETHUSD = 'e',
		ETHBTC = 'f',
		BTCGBP = 'g',
		BTCEUR = 'h',
		BTCUSD = 'i',
	};
	inline std::ostream& operator << (std::ostream& os, const product_id_type& obj)
	{
	    os << static_cast<std::underlying_type<product_id_type>::type>(obj);
	    return os;
	}

	enum class account_entry_type : char
	{
		transfer = 't',
		match = 'm',
		fee = 'f',
		rebate = 'r',
	};

	enum class holds_type : char
	{
		order = 'o',
		transfer = 't',
	};

	enum class done_reason_type : char 
	{
		filled = 'f',
	};

	enum class liquidity_type : char 
	{
		maker = 'm',
		taker = 't',
	};

	enum class margin_transaction_type : char
	{
		deposit = 'd',
		withdrawal = 'w',
	};

	enum class reason_type : char
	{
		filled = 'f',
		cancelled = 'c',
	};

	enum class ws_type : char
	{
		undefined = 'u',
		received = 'r',
		open = 'o',
		done = 'd',
		match ='m',
		change = 'c',
		margin_profile_update = 'p',
		activate = 'a',

	};
	inline std::ostream& operator << (std::ostream& os, const ws_type& obj)
	{
	    os << static_cast<std::underlying_type<ws_type>::type>(obj);
	    return os;
	}

	enum class position_type : char
	{
		margin_long = 'l',
		margin_short = 's',
	};

	enum class stop_type : char
	{
		entry = 'e',
	};

	struct account_details_type
	{
		char order_id[36];
		char trade_id[36];	// not sure of the correct type here ... FIXME TODO here
		product_id_type product_id;
	};

	struct new_order_response
	{
		std::chrono::system_clock::time_point receive_time;
		char id[36];
		double price;
		double size;
		product_id_type product_id;
		side_type side;
		stp_type stp;     // self trade prevention can probably be  enum ...
		order_type_t type;
		order_flight_type time_in_force;
		bool post_only;
		std::chrono::system_clock::time_point created_at; // "created_at": "2016-12-08T20:02:28.53864Z", https://stackoverflow.com/questions/44398598/c-parse-date-time-with-microseconds
		double fill_fees;
		double filled_size;
		double executed_volume;
		status_type status;
		bool settled;
	};


	struct list_accounts_response
	{
		std::chrono::system_clock::time_point receive_time;
		char id[36];
		currency_type currency;
		double balance;
		double available;
		double holds;
		char profile_id[36];
		bool margin_enabled;	// only available on margin accounts
		double funded_amount;	// only available on margin accounts
		double default_amount;	// only available on margin accounts
	};

	struct list_account_response
	{
		std::chrono::system_clock::time_point receive_time;
		char id[36];
		double balance;
		double holds;
		double available;
		char profile_id[36];
		bool margin_enabled;	// only available on margin accounts
		double funded_amount;	// only available on margin accounts
		double default_amount;	// only available on margin accounts
	};

	struct account_history 
	{
		std::chrono::system_clock::time_point receive_time;
		char id[36];
		std::chrono::system_clock::time_point created_at;
		double amount;
		double balance;
		account_entry_type type;
		account_details_type details;
	};

	struct holds_response
	{
		std::chrono::system_clock::time_point receive_time;
		char id[36];
		char account_id[36];
		std::chrono::system_clock::time_point created_at;
		double amount;
		holds_type type;
		char ref[36];
	};

	struct open_orders_response
	{
		std::chrono::system_clock::time_point receive_time;
		char id[36];
		double price;
		double size;
		product_id_type product_id;
		side_type side;
		stp_type stp;
		order_type_t type;
		order_flight_type time_in_force;
		bool post_only;
		std::chrono::system_clock::time_point created_at;
		double fill_fees;
		double filled_size;
		double executed_value;
		status_type status;
		bool settled;
	};


	struct open_order_response
	{
		std::chrono::system_clock::time_point receive_time;
		char id[36];
		double size;
		product_id_type product_id;
		side_type side;
		stp_type stp;
		double funds;
		double specified_funds;
		order_type_t type;
		bool post_only;
		std::chrono::system_clock::time_point created_at;
		std::chrono::system_clock::time_point done_at;
		done_reason_type done_reason;
		double fill_fees;
		double filled_size;
		double executed_value;
		status_type status;
		bool settled;
	};

	struct fills_response 
	{
		std::chrono::system_clock::time_point receive_time;
		uint64_t trade_id;
		product_id_type product_id;
		double price;
		double size;
		char order_id[36];
		std::chrono::system_clock::time_point created_at;
		liquidity_type liquidity;
		double fee;
		bool settled;
		side_type side;
	};

	struct funding_response
	{
		std::chrono::system_clock::time_point receive_time;
		char id[36];
		char order_id[36];
		char profile_id[36];
		double amount;
		status_type status;
		std::chrono::system_clock::time_point created_at;
		currency_type currency;
		double repaid_amount;
		double default_amount;
		bool repaid_default;
	};

	struct margin_transfer_response
	{
		std::chrono::system_clock::time_point receive_time;
		std::chrono::system_clock::time_point created_at;
		char id[36];
		char user_id[36];
		char profile_id[36];
		char margin_profile_id[36];
		margin_transaction_type	type;
		double amount;
		currency_type currency;
		char account_id[36];
		char margin_account_id[36];
		product_id_type margin_product_id;
		status_type status;
		uint32_t nonce;
	};

	struct products_response
	{
		std::chrono::system_clock::time_point receive_time;
		product_id_type id;
		currency_type base_currency;
		currency_type quote_currency;
		double base_min_size;
		double base_max_size;
		double quote_increment;
	};

	struct product_order_depth
	{
		std::chrono::system_clock::time_point receive_time;
		double price;
		double size;
		char oder_id[36];	
	};

	struct product_order_book
	{
		std::chrono::system_clock::time_point receive_time;
		uint64_t sequence;
		product_id_type id;
		std::vector<product_order_depth> bids;
		std::vector<product_order_depth> asks;
	};

	struct product_24h_Stats
	{
		std::chrono::system_clock::time_point receive_time;
		product_id_type id;
		double open;
		double high;
		double low;
		double volume;
	};

	struct currencies 
	{
		std::chrono::system_clock::time_point receive_time;
		currency_type id;
		std::string name;
		double min_size;
	};

	struct server_time
	{
		std::chrono::system_clock::time_point receive_time;
		std::chrono::system_clock::time_point iso;
		//epoch;
	};

	struct ws_heartbeat
	{
		std::chrono::system_clock::time_point receive_time;
		int64_t sequence;
		int64_t last_trade_id;
		product_id_type product_id;
		std::chrono::system_clock::time_point time;
	};

    struct ws_ticker
    {
        std::chrono::system_clock::time_point receive_time;
        int64_t trade_id;
        int64_t sequence;
        std::chrono::system_clock::time_point time;
        product_id_type product_id;
        double price;
        side_type side;
        double last_size;
        double best_bid;
        double best_ask;
    };

    // ["6500.11", "0.45054140"]
    struct ws_l2_snapshot_book
    {
        double price;
        double size;
    };

    // ["buy", "6500.09", "0.84702376"]
    struct ws_l2_update_book
    {
        side_type side;
        double price;
        double size;
    };

    // {"type": "snapshot","product_id": "BTC-EUR","bids": [["6500.11", "0.45054140"]],"asks": [["6500.15", "0.57753524"]]}
    struct ws_l2_snapshot
    {
        std::chrono::system_clock::time_point receive_time;
        product_id_type product_id;
        std::vector<ws_l2_snapshot_book> bids;
        std::vector<ws_l2_snapshot_book> asks;
    };

    //{"type": "l2update","product_id": "BTC-EUR","changes": [["buy", "6500.09", "0.84702376"],["sell", "6507.00", "1.88933140"],["sell", "6505.54", "1.12386524"],["sell", "6504.38", "0"]]}
    struct ws_l2_update
    {
        std::chrono::system_clock::time_point receive_time;
        product_id_type product_id;
        std::vector<ws_l2_update_book> changes;
    };
    
	struct ws_full_channel_received_limit
	{
		ws_type type;
		std::chrono::system_clock::time_point receive_time;
		std::chrono::system_clock::time_point time;
		product_id_type product_id;
		order_type_t order_type;
		int64_t sequence;
		char order_id[36];
		char client_oid[36];
		double size;
		double price;
		side_type side;
	};

	struct ws_full_channel_received_market
	{
		ws_type type;
		std::chrono::system_clock::time_point receive_time;
		std::chrono::system_clock::time_point time;
		product_id_type product_id;
		order_type_t order_type;
		int64_t sequence;
		char order_id[36];
		char client_oid[36];
		double funds;
		side_type side;
	};

	struct ws_full_channel_open
	{
		ws_type type;
		std::chrono::system_clock::time_point receive_time;
		std::chrono::system_clock::time_point time;
		product_id_type product_id;
		int64_t sequence;
		char order_id[36];
		double price;
		double remaining_size;
		side_type side;
	};

	struct ws_full_channel_done
	{
		ws_type type;
		std::chrono::system_clock::time_point receive_time;
		std::chrono::system_clock::time_point time;
		product_id_type product_id;
		int64_t sequence;
		double price;		// not here for market orders
		char order_id[36];
		reason_type reason;
		side_type side;
		double remaining_size;	// not here for market orders
	};

	struct ws_full_channel_match
	{
		ws_type type;
		std::chrono::system_clock::time_point receive_time;
		int64_t trade_id;
		int64_t sequence;
		char maker_order_id[36];
		char taker_order_id[36];
		std::chrono::system_clock::time_point time;
		product_id_type product_id;
		double size;
		double price;
		side_type side;
	};

	struct ws_full_channel_change
	{
		ws_type type;
		std::chrono::system_clock::time_point receive_time;
		std::chrono::system_clock::time_point time;
		int64_t sequence;
		char order_id[36];
		product_id_type product_id;
		double price;
		side_type side;
		double new_size;
		double old_size;
		double new_funds;
		double old_funds;
	};

	/*
	struct ws_full_channel_change_funds
	{
		std::chrono::system_clock::time_point receive_time;
		ws_type type;
		std::chrono::system_clock::time_point time;
		uint64_t sequence;
		char order_id[36];
		product_id_type product_id;
		double new_funds;
		double old_funds;
		double price;
		side_type side;
	};
	*/

	struct ws_full_channel_margin_profile_update
	{
		ws_type type;
		std::chrono::system_clock::time_point receive_time;
		std::chrono::system_clock::time_point timestamp;
		product_id_type product_id;
		char user_id[36];
		char profile_id[36];
		uint64_t nonce;
		position_type position; // what this
		double position_size;
		double position_compliment;
		double position_max_size;
		side_type call_side;
		double call_price;
		double call_size;
		double call_funds;
		bool covered;
		std::chrono::system_clock::time_point next_expire_time;
		double base_balance;
		double base_funding;
		double quote_balance;
		double quote_funding;
		bool margin_private;
	};

	struct ws_full_channel_margin_activate
	{
		ws_type type;
		std::chrono::system_clock::time_point receive_time;
		product_id_type product_id;
		double timestamp;
		char user_id[36];
		char profile_id[36];
		char order_id[36];
		stop_type stop_of_type;
		side_type side;
		double stop_price;
		double size;
		double funds;
		double taker_fee_rate;
		bool margin_private;
	};

	inline std::string convert_to_string_product_id(product_id_type id)
	{
		if (product_id_type::LTCEUR == id)
			return "LTC-EUR";
		if (product_id_type::LTCUSD == id)
			return "LTC-USD";
		if (product_id_type::LTCBTC == id)
			return "LTC-BTC";
		if (product_id_type::ETHEUR == id)
			return "ETH-EUR";
		if (product_id_type::ETHUSD == id)
			return "ETH-USD";
		if (product_id_type::ETHBTC == id)
			return "ETH-BTC";
		if (product_id_type::BTCGBP == id)
			return "BTC-GBP";
		if (product_id_type::BTCEUR == id)
			return "BTC-EUR";
		else //(product_id_type::BTCUSD)
			return "BTC-USD";
	};
	 
	inline product_id_type convert_product_id(std::string id)
	{
		if (!id.compare("LTC-EUR"))
			return product_id_type::LTCEUR;
		if (!id.compare("LTC-USD"))
			return product_id_type::LTCUSD;
		if (!id.compare("LTC-BTC"))
			return product_id_type::LTCBTC;
		if (!id.compare("ETH-EUR"))
			return product_id_type::ETHEUR;
		if (!id.compare("ETH-USD"))
			return product_id_type::ETHUSD;
		if (!id.compare("ETH-BTC"))
			return product_id_type::ETHBTC;
		if (!id.compare("BTC-GBP"))
			return product_id_type::BTCGBP;
		if (!id.compare("BTC-EUR"))
			return product_id_type::BTCEUR;
		if (!id.compare("BTC-USD"))
			return product_id_type::BTCUSD;
		if (!id.compare("LTCEUR"))
			return product_id_type::LTCEUR;
		if (!id.compare("LTCUSD"))
			return product_id_type::LTCUSD;
		if (!id.compare("LTCBTC"))
			return product_id_type::LTCBTC;
		if (!id.compare("ETHEUR"))
			return product_id_type::ETHEUR;
		if (!id.compare("ETHUSD"))
			return product_id_type::ETHUSD;
		if (!id.compare("ETHBTC"))
			return product_id_type::ETHBTC;
		if (!id.compare("BTCGBP"))
			return product_id_type::BTCGBP;
		if (!id.compare("BTCEUR"))
			return product_id_type::BTCEUR;
		if (!id.compare("BTCUSD"))
			return product_id_type::BTCUSD;

		// if get here error ... 
		// TODO FIX LATER
		return product_id_type::BTCUSD;
	};

	inline side_type convert_side_type(std::string side)
	{
		if (!side.compare("buy"))
			return side_type::buy;
		else // sell
			return side_type::sell;
	};

	inline std::string convert_to_string_side_type(side_type side)
	{
		if (side_type::buy == side)
			return "buy";
		else 
			return "sell";
	};

	inline reason_type convert_reason_type(std::string reason)
	{
		if(!reason.compare("filled"))
			return reason_type::filled;
		else // cancelled
			return reason_type::cancelled;
	};

	// some preliminary order book messages 
	struct l3_ob_msg
	{
		double price;
		double size;
		char order_id[36]; 
	};

	struct ob_rest_message
	{	
		uint64_t sequence;
		product_id_type product_id;
		side_type side;
		l3_ob_msg payload;
	};
	// end of order book mesage 


}} // exchange::coinbase
 
#endif 

