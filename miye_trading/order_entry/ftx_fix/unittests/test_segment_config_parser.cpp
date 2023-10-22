#include <boost/test/unit_test.hpp>
#include <iostream>

#include "../segment_config_loader.hpp"


BOOST_AUTO_TEST_CASE(TestSegmentConfigLoader)
{
	auto segments = qx::cme::segment_config_loader_t::load("MSGW_Config.xml");

	std::cout << "total segments: " << segments.size() << std::endl;

	std::for_each(std::begin(segments), std::end(segments),
			[](const qx::cme::segment_t& seg)
			{
				std::cout << seg << std::endl;
			});

	BOOST_CHECK_EQUAL(segments.size() == 21, true);

}

