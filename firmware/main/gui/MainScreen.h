//
//  MainScreen.h
//  SensorClock
//
//  created by yu2924 on 2025-11-21
//

#pragma once

// 
// the LV object layers contained in this implementation (Except for intermediate layers for layout purposes)
// 
// - MainScreen
//     +- HeaderPane
//     +- dashboard::ContainerPane
//     +- settings::ContainerPane
// 

namespace gui
{

	struct HeaderPane : public LvObj, private WifiConnection::Listener
	{
		static constexpr int32_t ItemSize = Metrics::HeaderHeight;
		AppController* appController;
		SharedRes* sharedRes;
		LvButton prefButton;
		LvButton backButton;
		LvLabel titleLabel;
		LvLabel indicatorWifi;
		int activePage = 0;
		std::function<void(uint32_t)> onRequestChangeActivePage;
		HeaderPane() = delete;
		HeaderPane(AppController* ac, SharedRes* sr) : appController(ac), sharedRes(sr)
		{
			appController->getWifiConnection()->addListener(this);
		}
		~HeaderPane() override
		{
			appController->getWifiConnection()->removeListener(this);
		}
		void create(lv_obj_t* parent)
		{
			LV_ASSERT(!ptr);
			ptr = LvObj::create(parent);
			setStyleTextFont(sharedRes->fontLatin1, LV_PART_MAIN);
			setStyleRadius(0, LV_PART_MAIN);
			setStyleBorderWidth(0, LV_PART_MAIN);
			setStylePadHor(8, LV_PART_MAIN);
			setStylePadVer(0, LV_PART_MAIN);
			setStylePadColRow(0, 0, LV_PART_MAIN);
			setStyleBgColor(sharedRes->colorScreenBackground, LV_PART_MAIN);
			setStyleTextColor(sharedRes->colorHeaderRegularText, LV_PART_MAIN);
			setFlexFlow(LV_FLEX_FLOW_ROW);
			setHeight(Metrics::HeaderHeight);
			prefButton = LvButton::createStaticTextButton(ptr, LV_SYMBOL_SETTINGS);
			prefButton.setStyleShadowWidth(0, LV_PART_MAIN);
			prefButton.setStyleBgOpa(LV_OPA_0, LV_PART_MAIN);
			prefButton.setSize(ItemSize, ItemSize);
			prefButton.align(LV_ALIGN_TOP_LEFT, 0, 0);
			prefButton.addTypedEventCb(this, LV_EVENT_CLICKED);
			backButton = LvButton::createStaticTextButton(ptr, LV_SYMBOL_LEFT);
			backButton.setStyleShadowWidth(0, LV_PART_MAIN);
			backButton.setStyleBgOpa(LV_OPA_0, LV_PART_MAIN);
			backButton.setSize(ItemSize, ItemSize);
			backButton.align(LV_ALIGN_TOP_LEFT, 0, 0);
			backButton.addTypedEventCb(this, LV_EVENT_CLICKED);
			titleLabel = LvLabel::create(ptr);
			int32_t cy = Metrics::HeaderHeight, cyt = titleLabel.getStyleTextFont(LV_PART_MAIN)->line_height;
			titleLabel.setHeight(cy);
			titleLabel.setStylePadTop((cy - cyt) / 2, LV_PART_MAIN);
			titleLabel.setStyleTextAlign(LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
			titleLabel.setFlexGrow(1);
			indicatorWifi = LvLabel::create(ptr);
			indicatorWifi.setSize(ItemSize, ItemSize);
			indicatorWifi.setStylePadTop((cy - cyt) / 2, LV_PART_MAIN);
			indicatorWifi.setTextStatic(LV_SYMBOL_WIFI);
			updateIndicator();
		}
		void updateIndicator()
		{
			indicatorWifi.setStyleTextColor(appController->getWifiConnection()->isConnected() ? sharedRes->colorHeaderLightText : sharedRes->colorHeaderRegularText, LV_PART_MAIN);
		}
		void setActivePage(int v, const char* static_title)
		{
			activePage = std::max(0, std::min(1, v));
			prefButton.setHidden(activePage != 0);
			backButton.setHidden(activePage != 1);
			titleLabel.setTextStatic(static_title);
		}
		// LvObj typed event callback
		void onLvEventCallback(lv_event_t* e)
		{
			LvEvent ev(e);
			if     (ev.testTargetAndCode(prefButton, LV_EVENT_CLICKED)) { if(onRequestChangeActivePage) { onRequestChangeActivePage(1); } }
			else if(ev.testTargetAndCode(backButton, LV_EVENT_CLICKED)) { if(onRequestChangeActivePage) { onRequestChangeActivePage(0); } }
		}
		// WifiConnection::Listener
		void wifiConnectionStatusDidUpdate(WifiConnection*, bool) override
		{
			ScopedLvLock sl;
			updateIndicator();
		}
	};

	// --------------------------------------------------------------------------------

	struct MainScreen : public LvScreen
	{
		AppController* appController = nullptr;
		SharedRes* sharedRes = nullptr;
		HeaderPane headerPane;
		dashboard::ContainerPane dashboardPane;
		settings::ContainerPane settingsPane;
		uint32_t activePage = 0;
		MainScreen() = delete;
		MainScreen(AppController* ac, SharedRes* sr) : appController(ac), sharedRes(sr), headerPane(ac, sr), dashboardPane(ac, sr), settingsPane(ac, sr)
		{
		}
		void create()
		{
			LV_ASSERT(!ptr);
			ptr = LvScreen::create();
			setStylePadAll(0, LV_PART_MAIN);
			setStylePadColRow(0, 0, LV_PART_MAIN);
			removeFlag(LV_OBJ_FLAG_SCROLLABLE);
			headerPane.create(ptr);
			dashboardPane.create(ptr);
			settingsPane.create(ptr);
			headerPane.onRequestChangeActivePage = [this](uint32_t ipage) { setActivePage(ipage, true); };
			updateLayout();
			setActivePage(0, false);
			addTypedEventCb(this, LV_EVENT_SIZE_CHANGED);
		}
		void updateLayout()
		{
			LvArea rc = getBounds();
			headerPane.setBounds(rc.removeFromTop(headerPane.getHeight()));
			dashboardPane.setBounds(rc);
			settingsPane.setBounds(rc);
		}
		void setActivePage(uint32_t ipage, bool anim)
		{
			static const char* Titles[] = { "Dashboard", "Settings" };
			activePage = std::min((uint32_t)1, ipage);
			headerPane.setActivePage(activePage, ((size_t)ipage < std::size(Titles)) ? Titles[activePage] : "");
			lvObjSetVisibility(dashboardPane, activePage == 0, anim);
			lvObjSetVisibility(settingsPane, activePage == 1, anim);
			sendEvent(sharedRes->EventActivePageDidChange, &activePage);
		}
		// LvObj typed event callback
		void onLvEventCallback(lv_event_t* e)
		{
			LvEvent ev(e);
			if(ev.testTargetAndCode(ptr, LV_EVENT_SIZE_CHANGED)) { updateLayout(); }
		}
	};

} // namespace gui
