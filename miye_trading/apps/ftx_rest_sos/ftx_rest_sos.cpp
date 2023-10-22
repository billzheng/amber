#include "libcore/essential/app.hpp"
#include "libs/cxxopts/include/cxxopts.hpp"

#include "libs/ftx/rest/client.h"
#include <iomanip>
#include <iostream>
#include <string>

namespace miye::trading
{

class FtxRestSosApp : public essential::app<FtxRestSosApp>
{
    ftx::RESTClient client;

  public:
    int run(int argc, char** argv)
    {
        cxxopts::Options options("ftx rest api", "ftx rest api simple order sender");

        options.add_options()("d,debug", "Enable debugging")                                     // a bool parameter
            ("a,action", "place_order, cancel_order, list_order", cxxopts::value<std::string>()) //
            ("s,symbol", "contract name", cxxopts::value<std::string>())                         //
            ("S,side", "buy or sell", cxxopts::value<std::string>())                             //
            ("p,price", "order price", cxxopts::value<double>())                                 //
            ("q,quantity", "order quantity", cxxopts::value<double>())                           //
            ("o,order_id", "order id", cxxopts::value<std::string>())                            //
            ("t,order_type", "order type:limit, market", cxxopts::value<std::string>())          //
            ("i,ioc", "is ioc order", cxxopts::value<bool>()->default_value("false"))            //
            ("P,post", "is postOnly ", cxxopts::value<bool>()->default_value("false"))           //
            ("r,reduce", "is reduceOnly", cxxopts::value<bool>()->default_value("false"))        //
            ("v,verbose", "Verbose output", cxxopts::value<bool>()->default_value("false"))      //
            ("h,help", "Print usage")                                                            //
            ;

        auto option = options.parse(argc, argv);

        if (option.count("help"))
        {
            std::cout << options.help() << std::endl;
            exit(0);
        }

        if (!option.count("action"))
        {
            std::cout << "please specify action type, options are: place_order, cancel_order, list_order" << std::endl;
            std::cout << options.help() << std::endl;
            exit(0);
        }

        auto const action = option["action"].as<std::string>();

        if (action == "list_order")
        {
            auto const info = client.get_open_orders();

            std::cout << "open orders:" << info << std::endl;
        }
        else if (action == "cancel_order")
        {
            if (!option.count("order_id"))
            {
                std::cout << "order_id is required for canceling order" << std::endl;
                exit(0);
            }
            auto const order_id = option["order_id"].as<std::string>();
            std::cout << "cancel order:" << order_id << std::endl;

            auto const output = client.cancel_order(order_id);
            std::cout << "cancel order output:" << output << std::endl;
        }
        else if (action == "place_order")
        {
            placeOrder(option, action);
        }
        else if (action == "list_futures")
        {
            auto const info     = client.list_futures();
            auto const& results = info["result"];
            for (auto const& item : results)
            {
                // std::cout << "market item :" << item << std::endl;
                auto const name            = item["name"];
                auto const openInterest    = item["openInterest"];
                auto const openInterestUsd = item["openInterestUsd"];
                auto const volume          = item["volume"];

                std::cout << "symbol:" << name << " openInterest:" << openInterest
                          << " openInterestUsd:" << openInterestUsd << " volume:" << volume << std::endl;
            }
            // std::cout << "list futures:" << info << std::endl;
        }
        else if (action == "list_markets")
        {
            auto const info = client.list_markets();

            // std::cout << "list markets:" << info << std::endl;
        }
        else if (action == "list_future_stats")
        {
            auto const symbol = option["symbol"].as<std::string>();
            auto const info   = client.get_fut_stats(symbol);

            auto const& item = info["result"];

            auto const openInterest             = item["openInterest"];
            auto const volume                   = item["volume"];
            auto const nextFundingRate          = item["nextFundingRate"];
            auto const nextFundingTime          = item["nextFundingTime"];
            auto const expirationPrice          = item["expirationPrice"];
            auto const predictedExpirationPrice = item["predictedExpirationPrice"];
            std::cout << "symbol:" << symbol << " openInterest:" << openInterest << " volume:" << volume
                      << " nextFundingRate:" << nextFundingRate << " nextFundingTime:" << nextFundingTime << std::endl;

            std::cout << "list future stats:" << info << std::endl;
        }
        else
        {
            std::cout << "invalid action:" << action << std::endl;
        }
        return 0;
    }

    void placeOrder(const cxxopts::ParseResult& option, const std::string& action)
    {
        if (!option.count("symbol"))
        {
            std::cout << "symbol is required for placing order" << std::endl;
            exit(0);
        }
        auto const symbol = option["symbol"].as<std::string>();

        if (!option.count("side"))
        {
            std::cout << "side is required for placing order" << std::endl;
            exit(0);
        }
        auto const side = option["side"].as<std::string>();

        if (!option.count("price"))
        {
            std::cout << "price is required for placing order" << std::endl;
            exit(0);
        }

        auto const price = option["price"].as<double>();
        if (!option.count("quantity"))
        {
            std::cout << "quantity is required for placing order" << std::endl;
            exit(0);
        }

        bool isIoc = false;
        if (option.count("ioc"))
        {
            isIoc = option["ioc"].as<bool>();
            std::cout << "ioc flag is set for order" << std::endl;
        }

        bool isPostOnly = false;
        if (option.count("post"))
        {
            isPostOnly = option["post"].as<bool>();
            std::cout << "postOnly flag is set for order" << std::endl;
        }

        bool isReduceOnly = false;
        if (option.count("reduce"))
        {
            isReduceOnly = option["reduce"].as<bool>();
            std::cout << "reduceOnly flag is set for order" << std::endl;
        }

        auto const qty = option["quantity"].as<double>();
        std::cout << "action:" << action << " symbol:" << symbol << " side:" << side << " price:" << price
                  << " quantity:" << qty << " isIoc:" << isIoc << " isPostOnly:" << isPostOnly
                  << " isReduceOnly:" << isReduceOnly << std::endl;

        auto result = client.place_order(symbol, side, price, qty, isIoc, isPostOnly, isReduceOnly);
        std::cout << "place_order result:" << std::setw(4) << result << std::endl;
    }
};
} // namespace miye::trading

int main(int argc, char** argv)
{
    miye::trading::FtxRestSosApp app;
    return app.main(argc, argv);
}
