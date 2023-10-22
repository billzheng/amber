#include "../exchanges/binance/binance_exchange.h"
#include "../exchanges/coinbase/coinbase_exchange.h"
#include "../exchanges/bitmex/bitmex_exchange.h"
#include "../exchanges/bitfinex/bitfinex_exchange.h"
#include "../exchanges/bitstamp/bitstamp_exchange.h"
#include "../exchanges/deribit/deribit_exchange.h"
#include "../exchanges/kraken/kraken_exchange.h"

#include "../libs/configs/configs.h" 
#include "../utils/string_time.h"


// test app to see ho we can start to record market data 
// need to replace al lthis with proper app class odwn the track 
int usage()
{
    std::cout << "usage: prog_name -c configfile" << std::endl;
    return 1;
}

int main(int argc, char *argv[])
{
    if ((argc != 3) || (std::strcmp(argv[1], "-c"))) 
        usage();

    dinobot::lib::configs config(argv[2]);

    std::string exch_name = config.get_config<std::string>("exchange", "name");

    if (!exch_name.compare("binance"))
    {
        dinobot::exchanges::binance_exchange exchange(config);
        exchange.set_finish_time( find_midnight_time() );
        exchange.init();
        exchange.start();
    } 
    else if (!exch_name.compare("coinbase"))
    {
        dinobot::exchanges::coinbase_exchange exchange(config);
        exchange.set_finish_time( find_midnight_time() );
        exchange.init();
        exchange.start();
    }
    else if (!exch_name.compare("bitmex"))
    {
        dinobot::exchanges::bitmex_exchange exchange(config);
        exchange.set_finish_time( find_midnight_time() );
        exchange.init();
        exchange.start();
    }
    else if (!exch_name.compare("bitfinex"))
    {
        dinobot::exchanges::bitfinex_exchange exchange(config);
        exchange.set_finish_time( find_midnight_time() );
        exchange.init();
        exchange.start();
    }
    else if (!exch_name.compare("bitstamp"))
    {
        dinobot::exchanges::bitstamp_exchange exchange(config);
        exchange.set_finish_time( find_midnight_time() );
        exchange.init();
        exchange.start();
    }
    else if (!exch_name.compare("deribit"))
    {
        dinobot::exchanges::deribit_exchange exchange(config);
        exchange.set_finish_time( find_midnight_time() );
        exchange.init();
        exchange.start();
    }
    else if (!exch_name.compare("kraken"))
    {
        dinobot::exchanges::kraken_exchange exchange(config);
        exchange.set_finish_time( find_midnight_time() );
        exchange.init();
        exchange.start();
    }
    else
    {
        std::cout << "exchange name does not exist, exiting" << std::endl;
        return 1;
    }
    
    return 0;
}
