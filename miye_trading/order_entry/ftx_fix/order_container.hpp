#pragma once

#include <unordered_map>
#include <cassert>
#include <iostream>
#include "../ftx_ftx/order.hpp"

namespace qx
{
namespace cme
{

class order_container
{
public:
	using order_map = std::unordered_map<order_id_t, order_t>;

public:
	order_container() = default;

public:
	order_t* get(order_id_t clord_id)
	{
		auto it = orders.find(clord_id);
		return it == orders.end() ? nullptr : &it->second;
	}

	const order_t* get(order_id_t clord_id) const
	{
		auto it = orders.find(clord_id);
		return it == orders.end() ? nullptr : &it->second;
	}

    const order_t* getBySequence(uint32_t sequence) const
    {
        for (auto it = orders.begin(); it != orders.end(); ++it)
        {
            if (it->second.fix_sequence == sequence)
            {
                return &it->second;
            }
        }
        return nullptr;
    }

	void add(order_id_t clord_id, order_t&& order)
	{
		auto it = orders.insert(std::make_pair(clord_id, std::forward<order_t>(order)));
		//assert(it.second == true);
		if (it.second)
		{
			//std::cout << __PRETTY_FUNCTION__ << ", add order to container: " << order << std::endl;
		}
		else
		{
			//std::cout << __PRETTY_FUNCTION__ << ", add order failed. existing order: " << it.first->second << std::endl;
		}
	}

    void update(order_id_t clord_id, order_id_t order_id, price_t price, quantity_t qty)
    {
        auto it = orders.find(clord_id);
        if (it != orders.end())
        {
            auto& order    = it->second;
            order.price    = price;
            order.qty      = qty;
            order.exchange_key.key  = order_id;

            //std::cout << __PRETTY_FUNCTION__ << ", clord_id: " << clord_id << ", order_id: " << order_id
            //        << ", price: " << price << ", qty: " << qty << std::endl;
        }
        else
        {
            //std::cout << __PRETTY_FUNCTION__ << ", order not found from container. clord_id: " << clord_id
            //         << ", order_id: " << order_id << ", price: " << price << ", qty: " << qty << std::endl;
        }
    }

    size_t remove(order_id_t clord_id)
	{
		auto const cnt = orders.erase(clord_id);
		//std::cout << __PRETTY_FUNCTION__ << " remove order " << cnt << " from container: " << ", order_id: " << clord_id << std::endl;
		return cnt;
	}

private:
	order_map orders;

};

} // namespace cme
} // namespace qx
