//
//  platform.h
//  SensorClock
//
//  created by yu2924 on 2025-12-21
//

#pragma once

#include <cstdarg>
#include <cstdint>
#include <ctime>

namespace platform
{
	// debug
	// level: 'V', 'I', 'W', 'E', 'D'
	void debugPrintV(char level, const char* fmt, va_list va) noexcept;
	void debugPrint(char level, const char* fmt, ...) noexcept;
	// app properties
	const char* getAppName() noexcept;
	const char* getAppVersion() noexcept;
	// system time
	bool setSystemTimeZone(const char* tzposix) noexcept;
	bool setSystemTime(time_t t) noexcept;
	const tm* localtime(const time_t* t, tm* ptm) noexcept;
	// thread
	uint32_t getCurrentThreadId() noexcept;
	void sleepMilliseconds(uint32_t t) noexcept;
} // namespace platform

#if NDEBUG
#define DEBUGPRINTI(...)
#else
#define DEBUGPRINTI(...) platform::debugPrint('I', __VA_ARGS__)
#endif
#define DEBUGPRINTW(...) platform::debugPrint('W', __VA_ARGS__)
#define DEBUGPRINTE(...) platform::debugPrint('E', __VA_ARGS__)
