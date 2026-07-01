//
//  GuiInit.cpp
//  SensorClock
//
//  created by yu2924 on 2025-12-06
//

#include "GuiInit.h"

#include <lvgl.h>
#include <time.h>
#include <array>
#include <cmath>
#include <functional>

#include "utilities/cjsonwrap.h"
#include "utilities/heapbuffer.h"
#include "utilities/platform.h"
#include "utilities/strutil.h"

#include "modules/AlarmSounder.h"
#include "modules/AppController.h"
#include "modules/BoardFs.h"
#include "modules/Dispatcher.h"
#include "modules/NetIf.h"
#include "modules/PropertiesStore.h"
#include "modules/SensorTask.h"
#include "modules/WeatherTask.h"

#include "lvclasses/LvArea.h"
#include "lvclasses/LvBar.h"
#include "lvclasses/LvButton.h"
#include "lvclasses/LvDropdown.h"
#include "lvclasses/LvEvent.h"
#include "lvclasses/LvImage.h"
#include "lvclasses/LvKeyboard.h"
#include "lvclasses/LvLabel.h"
#include "lvclasses/LvLine.h"
#include "lvclasses/LvScale.h"
#include "lvclasses/LvScreen.h"
#include "lvclasses/LvSlider.h"
#include "lvclasses/LvSwitch.h"
#include "lvclasses/LvTextArea.h"

#if __has_include("gui/GuiConfig.h")
#include "gui/GuiConfig.h"
#endif 

#include "gui/Common.h"
#include "gui/Dashboard.h"
#include "gui/Settings.h"
#include "gui/MainScreen.h"

namespace gui
{

	int getScreenWidth() { return GuiScreenWidth; }
	int getScreenHeight() { return GuiScreenHeight; }

	class GuiInitImpl : public GuiInit
	{
	private:
		SharedRes sharedRes;
		MainScreen mainScreen;
	public:
		GuiInitImpl(AppController* ac) : mainScreen(ac, &sharedRes)
		{
		}
		~GuiInitImpl() override
		{
			mainScreen.deleteObject();
		}
		void create() override
		{
			mainScreen.create();
			mainScreen.load();
		}
	};

	xheap::uniquePtr<GuiInit> GuiInit::createInstance(AppController* ac)
	{
		return xheap::makeUnique<GuiInit, GuiInitImpl>(ac);
	}

} // namespace gui
