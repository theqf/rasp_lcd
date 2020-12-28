#ifndef TIMEUTIL_H
#define TIMEUTIL_H

#include <cstdint>
#include <chrono>


class TimeUtil {
public:
	static int64_t currentTimeMillis();
	static int32_t currentTimeInDayMillis();
};
#endif

