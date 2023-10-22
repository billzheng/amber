//#include <vector>
#include <string>

/**
 * Create a string of the format 20180403
 * We pass in the time when to generate the date string
 *
 * Note: we don't use timezones here. This code uses the systems timezone generally UTC
 */
std::string generate_date_string(int future_seconds /*=0*/)
{
    time_t nowiw = time(0);                                            

    time_t futuretime = nowiw + future_seconds;

    tm *ltm = localtime(&futuretime);                                  
	std::string zero_month;
	std::string zero_day;

	if ((1 + ltm->tm_mon) < 10)
		zero_month = std::string("0").append( std::to_string(1 + ltm->tm_mon));
	else
		zero_month = std::to_string(1 + ltm->tm_mon);

	if ((ltm->tm_mday) < 10)
		zero_day = std::string("0").append( std::to_string(ltm->tm_mday));
	else
		zero_day = std::to_string(ltm->tm_mday);

    std::string logg = std::to_string(1900 + ltm->tm_year) + zero_month + zero_day ;

    return logg;
}

/**
 * Gets current time (plus 2 minutes) then finds 00:00 
 * then adds 24 hours to that for a stop time just after midnight
 * this is here to auto stop the thing at UTC midnight
 *
 * This is just a convenience function, and the reason the 2minutes addition
 * is that i will generally start this via cron at 23:59:59 so it is ready 
 * for recording and trading at 00:00. 
 */
time_t find_midnight_time()
{
    time_t nowiw = time(0);                                            

    time_t futuretime = nowiw + 120; /* 2 minutes */

    time_t midnight = (futuretime / 86400 * 86400) + 86400; 
                                                                       
    return midnight;
}
