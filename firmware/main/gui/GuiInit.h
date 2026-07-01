//
//  GuiInit.h
//  SensorClock
//
//  created by yu2924 on 2025-12-06
//

#pragma once

#include "utilities/xheap.h"
#include "modules/AppController.h"

namespace gui
{

	int getScreenWidth();
	int getScreenHeight();

	class GuiInit
	{
	public:
		virtual ~GuiInit() {}
		virtual void create() = 0;
		// factory
		static xheap::uniquePtr<GuiInit> createInstance(AppController* ac);
	};

} // namespace gui
