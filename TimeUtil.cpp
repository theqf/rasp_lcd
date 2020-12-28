#include <thread>
#include "TimeUtil.h"


int64_t TimeUtil::currentTimeMillis()
{
	using namespace std::chrono;
	return time_point_cast<milliseconds>(steady_clock::now()).time_since_epoch().count();
}

int32_t TimeUtil::currentTimeInDayMillis()
{
	return (int32_t) (currentTimeMillis() % (24 * 3600 * 1000));
}
