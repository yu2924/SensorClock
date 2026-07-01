//
//  xheap.cpp
//  SensorClock
//
//  created by yu2924 on 2025-05-14
//

#include "xheap.h"

#if defined(_WINDOWS)

namespace xheap
{
	void* memAlloc(size_t size)
	{
		return std::malloc(size);
	}
	void* memRealloc(void* p, size_t new_size)
	{
		return std::realloc(p, new_size);
	}
	void memFree(void* p)
	{
		std::free(p);
	}
}

#elif defined(ESP_PLATFORM)

#include "esp_heap_caps.h"

namespace xheap
{
	void* memAlloc(size_t size)
	{
		return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
	}
	void* memRealloc(void* p, size_t new_size)
	{
		return heap_caps_realloc(p, new_size, MALLOC_CAP_SPIRAM);
	}
	void memFree(void* p)
	{
		heap_caps_free(p);
	}
}

#else
#error unknown platform
#endif // defined(_WINDOWS) / defined(ESP_PLATFORM)
