//
//  platform.cpp
//  SensorClock
//
//  created by yu2924 on 2025-12-21
//

#include "platform.h"

#if defined(_WINDOWS)

// ================================================================================
// Prototype code for Windows

#define STRICT
#define WIN32_LEAN_AND_MEAN
#if !defined NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <cstdio>
#include <cstring>
#include <iterator>

namespace platform
{
	// --------------------------------------------------------------------------------
	// debug
	void debugPrintV(char level, const char* fmt, va_list va) noexcept
	{
		struct TimeStamp
		{
			static uint32_t getMilliseconds()
			{
				return (uint32_t)((int64_t)clock() * 1000 / CLOCKS_PER_SEC);
			}
		};
		char buf[1024];
		int lpfx = snprintf(buf, std::size(buf), "%c (%u) ", level, TimeStamp::getMilliseconds());
		vsnprintf(buf + lpfx, std::size(buf) - lpfx, fmt, va);
		strcat_s(buf, std::size(buf), "\n");
		OutputDebugStringA(buf);
	}
	void debugPrint(char level, const char* fmt, ...) noexcept
	{
		va_list va;
		va_start(va, fmt);
		debugPrintV(level, fmt, va);
		va_end(va);
	}
	// --------------------------------------------------------------------------------
	// app properties
	const char* getAppName() noexcept
	{
		return "SensorClock";
	}
	const char* getAppVersion() noexcept
	{
		return "0.1.1";
	}
	// --------------------------------------------------------------------------------
	// system time
	bool setSystemTimeZone(const char*) noexcept
	{
		return true;
	}
	bool setSystemTime(time_t) noexcept
	{
		return true;
	}
	const tm* localtime(const time_t* t, tm* ptm) noexcept
	{
		return (localtime_s(ptm, t) == 0) ? ptm : nullptr;
	}
	// --------------------------------------------------------------------------------
	// thread
	uint32_t getCurrentThreadId() noexcept
	{
		return GetCurrentThreadId();
	}
	void sleepMilliseconds(uint32_t t) noexcept
	{
		Sleep(t);
	}
} // namespace platform

#elif defined(ESP_PLATFORM)

// ================================================================================
// Production code for ESP32

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_app_desc.h>
#include <esp_log.h>
#include <sys/time.h>

namespace platform
{
	// --------------------------------------------------------------------------------
	// debug
	void debugPrintV(char level, const char* fmt, va_list va) noexcept
	{
		static const char* Tag = "App";
		esp_log(ESP_LOG_CONFIG_INIT(ESP_LOG_INFO), Tag, "%c (%ld) %s: ", level, esp_log_timestamp(), Tag);
		vprintf(fmt, va);
		putchar('\n');
	}
	void debugPrint(char level, const char* fmt, ...) noexcept
	{
		va_list va;
		va_start(va, fmt);
		debugPrintV(level, fmt, va);
		va_end(va);
	}
	// --------------------------------------------------------------------------------
	// app properties
	const char* getAppName() noexcept
	{
		const esp_app_desc_t* desc = esp_app_get_description();
		return desc->project_name;
	}
	const char* getAppVersion() noexcept
	{
		const esp_app_desc_t* desc = esp_app_get_description();
		return desc->version;
	}
	// --------------------------------------------------------------------------------
	// system time
	bool setSystemTimeZone(const char* tzposix) noexcept
	{
		if(setenv("TZ", tzposix, 1) != 0) { return false; }
		tzset();
		return true;
	}
	bool setSystemTime(time_t t) noexcept
	{
		if(t < 0) return false;
		struct timeval tv {};
		tv.tv_sec = t;
		tv.tv_usec = 0;
		return settimeofday(&tv, NULL) == 0;
	}
	const tm* localtime(const time_t* t, tm* ptm) noexcept
	{
		return localtime_r(t, ptm);
	}
	// --------------------------------------------------------------------------------
	// thread
	uint32_t getCurrentThreadId() noexcept
	{
		return (uint32_t)xTaskGetCurrentTaskHandle();
	}
	void sleepMilliseconds(uint32_t t) noexcept
	{
		vTaskDelay(pdMS_TO_TICKS(t));
	}
} // namespace platform

#else
#error unknown platform
#endif // defined(_WINDOWS) / defined(ESP_PLATFORM)
