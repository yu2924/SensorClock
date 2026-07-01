//
//  cjsonwrap.h
//  SensorClock
//
//  created by yu2924 on 2026-06-28
//

#pragma once

#include <cJSON.h>
#include "utilities/xheap.h"

namespace cjsonwrap
{
	struct Deleter
	{
		void operator()(void* p) const noexcept
		{
			cJSON_Delete((cJSON*)p);
		}
	};
	using cJSONUniquePtr = std::unique_ptr<cJSON, Deleter>;
	static void initWithHeapAllocators()
	{
		cJSON_Hooks hooks{};
		hooks.malloc_fn = xheap::memAlloc;
		hooks.free_fn = xheap::memFree;
		cJSON_InitHooks(&hooks);
	}
}
