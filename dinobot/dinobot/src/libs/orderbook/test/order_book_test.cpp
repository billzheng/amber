#include <iostream>

#include "../order_book.h"

using namespace dinobot::orderbook;

void print_orders_number_info(orderbook &ob)
{
    std::cout << "number_of_orders info in aggregate" << std::endl;
    std::cout << "num_ask_orders " << ob.num_ask_orders() << std::endl;
    std::cout << "num_bid_orders " << ob.num_bid_orders() << std::endl;
    std::cout << "num_orders " << ob.num_orders() << std::endl;
    std::cout << std::endl;
}

void print_num_price_levels(orderbook &ob)
{
    std::cout << "number of price levels" << std::endl;
    std::cout << "num_ask_price_levels " << ob.num_ask_price_levels() << std::endl;
    std::cout << "num_bid_price_levels " << ob.num_bid_price_levels() << std::endl;
    std::cout << "num_price_levels " << ob.num_price_levels() << std::endl;
    std::cout << std::endl;
}

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    orderbook OB;

    std::cout << "generated empty order book" << std::endl;
    //print_orders_number_info(OB);
    //print_num_price_levels(OB);

    std::cout << "lets start adding some orders" << std::endl;
    OB.add(1001, 100, 10, order_t::side_t::buy);
    OB.add(1002, 100, 5, order_t::side_t::buy);
    OB.add(1003, 100, 20, order_t::side_t::buy);
    OB.add(1004, 101, 20, order_t::side_t::buy);
    OB.add(1005, 110, 10, order_t::side_t::sell);
    OB.add(1006, 111, 5, order_t::side_t::sell);
    OB.add(1007, 112, 20, order_t::side_t::sell);
    OB.add(1008, 112, 20, order_t::side_t::sell);
    //print_orders_number_info(OB);
    //print_num_price_levels(OB);

    OB.print_book();
    std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
    OB.execute(1005, 2);

    //std::cout << "after execute" << std::endl;
    std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;

    OB.print_book();

    return 0;
}   
