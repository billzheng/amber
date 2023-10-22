#pragma once

#include <cstdint>

#include "../ftx_fix/float_utils.hpp"

namespace qx
{
namespace price_utils
{

inline double round_by_tick_and_precision(float value, float tick_size, uint32_t precision)
{
    double v = value;
    if (tick_size > 0)
    {
        v = qx::math::utils::to_nearest_tick(value, tick_size);
    }
    if (precision > 0)
    {
        double m = math::apply_exponent_scaler(1.0, precision);
        v        = round(v * m) / m;
    }
    return v;
}

inline int32_t get_precision_digit(float display_factor)
{
	constexpr static const float epsilon = 0.00000001;

	if (display_factor >= 1.0 || qx::float_utils::approximatelyEqual(display_factor, 1.0, epsilon))
	{
		return 0;
	}

	if (qx::float_utils::approximatelyEqual(display_factor, 0.1, epsilon))
	{
		return 1;
	}
	if (qx::float_utils::approximatelyEqual(display_factor, 0.01, epsilon))
	{
		return 2;
	}
	if (qx::float_utils::approximatelyEqual(display_factor, 0.001, epsilon))
	{
		return 3;
	}
	if (qx::float_utils::approximatelyEqual(display_factor, 0.0001, epsilon))
	{
		return 4;
	}
	if (qx::float_utils::approximatelyEqual(display_factor, 0.00001, epsilon))
	{
		return 5;
	}
	if (qx::float_utils::approximatelyEqual(display_factor, 0.000001, epsilon))
	{
		return 6;
	}
	if (qx::float_utils::approximatelyEqual(display_factor, 0.0000001, epsilon))
	{
		return 7;
	}
	// by default, max 7 digits
	return 7;
}

inline int32_t get_precision_delta(int32_t precision, double display_factor)
{
	int32_t display_factor_precision = get_precision_digit(display_factor);

	int32_t precision_delta = 0;
	if (display_factor_precision >= precision)
	{
		//std::cout << "precision: " << precision << ", display factor precision: " << display_factor_precision << ", precision delta: " << precision_delta << std::endl;
		return 0;
	}
	else
	{
		precision_delta = precision - display_factor_precision;
	}

	//std::cout << "precision: " << precision << ", display factor precision: " << display_factor_precision << ", precision delta: " << precision_delta << std::endl;

	return precision_delta;
}

inline double apply_price_factor_to_out_price(double price, double display_factor)
{
	return price / display_factor;
}

inline double apply_price_factor_to_in_price(double price, double display_factor)
{
	return price * display_factor;
}

} // namespace price_utils
} // namespace qx
