#pragma once
#include <cstdint>

// cme page: http://www.cmegroup.com/confluence/display/EPICSANDBOX/iLink+Reject+Codes

namespace miye::trading::ftx
{

/*
 *   Sent in response to Order Cancel Request (tag 35-MsgType=F) message in tag 102-CxlRejReason of the Order Cancel
 * Reject (tag 35-MsgType=9)
 */
inline const char* cancelRejectCode(uint32_t code) noexcept
{
    switch (code)
    {
    case 1003:
        return "Orders may not be canceled while the market is closed";
    case 2045:
        return "This order is not on the book";
    case 2048:
        return "The order was submitted with a different SenderCompID than the requesting cancel";
    case 7024:
        return "Order cannot be modified or cancelled while the market is in No Cancel";
    default:
        return "unknown";
    }
}

inline const char* newOrderRejectCode(uint32_t code) noexcept
{
    switch (code)
    {
    case 1003:
        return "Orders may not be entered while the market is closed or Orders may not be entered while the market is "
               "paused";
    case 1007:
        return "FIX field missing or incorrect";
    case 1010:
        return "Required field missing";
    case 1011:
        return "FIX field incorrect";
    case 1012:
        return "Price must be greater than zero";
    case 1013:
        return "Invalid order qualifier";
    case 1014:
        return "The user is not authorized to trade";
    case 2013:
        return "Market price orders not supported by opposite limit";
    case 2019:
        return "Order's GTD Expire Date is before the current (or next, if not currently in a session) trading session "
               "end date";
    case 7000:
        return "Order rejected";
    case 2045:
        return "This order is not in the book";
    case 2046:
        return "Disclosed Quantity cannot be greater than total or remaining qty";
    case 2047:
        return "Order contract is unknown";
    case 2051:
        return "The Order was submitted with a different side than the requesting Cancel";
    case 2052:
        return "The Order was submitted with a different group (tag 55) than the requesting cancel";
    case 2053:
        return "The Order was submitted with a different security type than the requesting cancel";
    case 2054:
        return "The Order was submitted with a different account than the requesting cancel";
    case 2055:
        return "The Order was submitted with a different quantity than the requesting Cancel";
    case 2056:
        return "The Order was submitted with a different TraderID than the requesting Cancel";
    case 2058:
        return "Stop price maxi-mini must be greater than or equal to trigger price";
    case 2059:
        return "Stop price maxi-mini must be smaller than or equal to trigger price";
    case 2060:
        return "Sell order stop price must be below last trade price";
    case 2061:
        return "Buy order stop price must be above last trade price";
    case 2100:
        return "The modify was submitted on a different product than the original order";
    case 2101:
        return "Attempt to modify an order with a different in-flight-fill mitigation status than first modification";
    case 2102:
        return "Attempt to modify an order with a different SenderCompID than the original order";
    case 2103:
        return "Attempt to modify an order with a different TraderID than the original order";
    case 2115:
        return "Order quantity is outside of the allowable range";
    case 2130:
        return "Order type not permitted while the market is in Post Close/Pre-Open (PCP)";
    case 2137:
        return "Order price is outside the limits";
    case 2179:
        return "Order price is outside bands";
    case 2311:
        return "Order type not permitted for group";
    case 2500:
        return "Instrument has a request for cross in progress";
    case 2501:
        return "Order Quantity too low";
    case 7024:
        return "Order cannot be modified or cancelled while the market is in no cancel";
    case 7027:
        return "Order type not permitted while the market is reserved";
    case 7028:
        return "Order session date is in the past";
    case 7029:
        return "Orders may not be entered while the market is forbidden";
    case 7613:
        return "Disclosed quantity cannot be smaller than the minimum quantity ";
    default:
        return "unknown";
    }
}

} // namespace miye::trading::ftx
