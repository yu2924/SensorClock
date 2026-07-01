//
//  Settings.h
//  SensorClock
//
//  created by yu2924 on 2025-11-21
//

#pragma once

// 
// the LV object layers contained in this implementation (Except for intermediate layers for layout purposes)
// 
// - ContainerPane
//     +- WifiSectionPane
//     |    +- WifiApRowPane
//     |    +- WifiApScanModalRowPane
//     +- SystemTimeSectionPane
//     |    +- TimeZoneRowPane
//     |    +- CurrentTimeRowPane
//     |    +- SntpRowPane
//     +- WeatherServiceSectionPane
//     |    +- WeatherApiKeyRowPane
//     |    +- WeatherLocationRowPane
//     |    +- WeatherLocSearchModalRowPane
//     +- HistoryServerSectionPane
//     |    +- HistoryServerHostRowPane
//     |    +- HistoryServerUserRowPane
//     +- SensorAlertSectionPane
//     +- AboutSectionPane
//          +- LicenseNoticeGridPane
// - LicenseNoticePopup
// 

namespace gui::settings
{

	struct FieldRowPaneImplBase : public LvObj
	{
		static void adjustLabelVerticalAlignCenter(LvLabel& label)
		{
			// note: to force vertical center alignment, add padding to the ceiling
			int32_t hinner = Metrics::FieldRowHeight;
			const lv_font_t* font = label.getStyleTextFont(LV_PART_MAIN);
			int32_t hline = font ? (font->line_height - font->base_line) : 0;
			int32_t vpad = (hline < hinner) ? ((hinner - hline) / 2) : 0;
			label.setStylePadTop(vpad, LV_PART_MAIN);
		}
		static constexpr int32_t MAXFIELDVALUES = 4;
		AppController* appController;
		SharedRes* sharedRes;
		LvKeyboard keyboard;
		LvLabel nameLabel;
		std::array<LvObj, MAXFIELDVALUES> fieldValueList;
		int32_t numFieldValues = 0;
		LvButton applyButton;
		LvButton cancelButton;
		bool editing = false;
		enum { BTNF_APPLY = 0x01, BTNF_CANCEL = 0x02 };
		FieldRowPaneImplBase() = delete;
		FieldRowPaneImplBase(AppController* ac, SharedRes* sr) : appController(ac), sharedRes(sr)
		{
		}
		void create(LvKeyboard kb, LvObj parent, const char* static_nametext, int btnf)
		{
			keyboard = kb;
			LV_ASSERT(!ptr);
			ptr = LvObj::create(parent);
			setStyleTextFont(sharedRes->fontLatin1, LV_PART_MAIN);
			setFlexFlow(LV_FLEX_FLOW_ROW);
			setStyleBorderWidth(0, LV_PART_MAIN);
			setStyleOutlineWidth(0, LV_PART_MAIN);
			setStyleBgOpa(LV_OPA_0, LV_PART_MAIN);
			setStylePadAll(0, LV_PART_MAIN);
			setStylePadColRow(Metrics::Padding, 0, LV_PART_MAIN);
			setStyleTextColor(sharedRes->colorSettingsRegularText, LV_PART_MAIN);
			setSize(LV_PCT(100), LV_SIZE_CONTENT);
			// name label
			nameLabel = LvLabel::create(ptr);
			nameLabel.setSize(Metrics::FieldLabelWidth, Metrics::FieldRowHeight);
			nameLabel.setStylePadLeft(Metrics::Padding, LV_PART_MAIN);
			nameLabel.setStylePadRight(Metrics::Padding, LV_PART_MAIN);
			if(static_nametext) { nameLabel.setTextStatic(static_nametext); }
			adjustLabelVerticalAlignCenter(nameLabel);
			nameLabel.setStyleTextAlign(LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
			// buttons
			if(btnf & BTNF_APPLY) { applyButton = LvButton::createStaticTextButton(ptr, "Apply"); }
			if(btnf & BTNF_CANCEL) { cancelButton = LvButton::createStaticTextButton(ptr, "Cancel"); }
			getScreen().addTypedEventCb(this, sharedRes->EventActivePageDidChange);
			setEditing(false);
		}
		LvTextArea addTextArea()
		{
			LV_ASSERT(numFieldValues < MAXFIELDVALUES);
			if(MAXFIELDVALUES <= numFieldValues) return {};
			LvTextArea fv = LvTextArea::create(ptr);
			fv.setWidth(Metrics::FieldValueWidth);
			fv.moveToIndex(1 + numFieldValues);
			fv.setOneLine(true);
			fv.addTypedEventCb(this, LV_EVENT_FOCUSED);
			fv.addTypedEventCb(this, LV_EVENT_DEFOCUSED);
			fieldValueList[numFieldValues] = fv;
			++numFieldValues;
			return fv;
		}
		LvDropdown addDropdown()
		{
			LV_ASSERT(numFieldValues < MAXFIELDVALUES);
			if(MAXFIELDVALUES <= numFieldValues) return {};
			LvDropdown fv = LvDropdown::create(ptr);
			fv.getList().setStyleTextFont(sharedRes->fontLatin1, LV_PART_MAIN);
			fv.clearOptions();
			fv.setWidth(Metrics::FieldValueWidth);
			fv.moveToIndex(1 + numFieldValues);
			fieldValueList[numFieldValues] = fv;
			++numFieldValues;
			return fv;
		}
		void setEditing(bool edit)
		{
			editing = edit;
			lv_color_t clr = editing ? sharedRes->colorSettingsTextAreaBgEdit : sharedRes->colorSettingsTextAreaBgNormal;
			for(int i = 0; i < numFieldValues; ++i) { fieldValueList[i].setStyleBgColor(clr, LV_PART_MAIN); }
			if(applyButton) { applyButton.setHidden(!editing); }
			if(cancelButton) { cancelButton.setHidden(!editing); }
		}
		// LvObj typed event callback
		void onLvEventCallback(lv_event_t* e)
		{
			LvEvent ev(e);
			if(ev.getCode() == LV_EVENT_FOCUSED)
			{
				LvObj focus = ev.getTargetObj();
				if(!focus || focus.isDisabled()) return;
				keyboard.setTextarea(focus);
				keyboard.setHidden(false);
				focus.scrollToViewRecursive(LV_ANIM_OFF);
			}
			else if(ev.getCode() == LV_EVENT_DEFOCUSED)
			{
				keyboard.setTextarea(nullptr);
				keyboard.setHidden(true);
			}
			else if(ev.getCode() == sharedRes->EventActivePageDidChange)
			{
				if(cancelButton && !cancelButton.isHidden())
				{
					cancelButton.sendEvent(LV_EVENT_CLICKED, nullptr);
				}
			}
		}
	};

	struct SectionPaneImplBase : public LvObj
	{
		AppController* appController;
		SharedRes* sharedRes;
		LvKeyboard keyboard;
		LvLabel titleLabel;
		SectionPaneImplBase() = delete;
		SectionPaneImplBase(AppController* ac, SharedRes* sr) : appController(ac), sharedRes(sr)
		{
		}
		void create(LvKeyboard kb, LvObj parent, const char* statictitle)
		{
			keyboard = kb;
			LV_ASSERT(!ptr);
			ptr = LvObj::create(parent);
			setStyleTextFont(sharedRes->fontLatin1, LV_PART_MAIN);
			setFlexFlow(LV_FLEX_FLOW_COLUMN);
			setStyleRadius(Metrics::CornerRadius, LV_PART_MAIN);
			setStyleBgColor(sharedRes->colorSettingsSectionBackground, LV_PART_MAIN);
			setStyleTextColor(sharedRes->colorSettingsRegularText, LV_PART_MAIN);
			setStyleBorderWidth(0, LV_PART_MAIN);
			setStylePadAll(Metrics::Padding, LV_PART_MAIN);
			setStylePadBottom(Metrics::Padding * 2, LV_PART_MAIN);
			setStylePadColRow(Metrics::Padding, Metrics::Padding, LV_PART_MAIN);
			titleLabel = LvLabel::create(ptr);
			titleLabel.setStyleTextFont(sharedRes->fontLatin1Bold, LV_PART_MAIN);
			titleLabel.setStyleTextAlign(LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
			if(statictitle) titleLabel.setTextStatic(statictitle);
			else titleLabel.setFlag(LV_OBJ_FLAG_HIDDEN, true);
		}
	};

	// ================================================================================
	// wifi panes

	struct WifiApScanModalRowPane : public FieldRowPaneImplBase
	{
		LvDropdown dropdown;
		LvButton searchButton;
		LvButton closeButton;
		xheap::uniquePtr<WifiApScanner> wifiApScanner = WifiApScanner::createInstance();
		WifiApScanner::Result wifiApList{};
		std::function<void(const WifiAp&)> onItemSelectedCallback;
		std::function<void()> onRequestCloseCallback;
		WifiApScanModalRowPane() = delete;
		WifiApScanModalRowPane(AppController* ac, SharedRes* sr) : FieldRowPaneImplBase(ac, sr)
		{
		}
		void create(LvKeyboard kb, LvObj parent)
		{
			FieldRowPaneImplBase::create(kb, parent, nullptr, 0);
			dropdown = addDropdown();
			dropdown.addTypedEventCb(this, LV_EVENT_VALUE_CHANGED);
			searchButton = LvButton::createStaticTextButton(ptr, LV_SYMBOL_REFRESH);
			searchButton.addTypedEventCb(this, LV_EVENT_CLICKED);
			closeButton = LvButton::createStaticTextButton(ptr, LV_SYMBOL_CLOSE);
			closeButton.addTypedEventCb(this, LV_EVENT_CLICKED);
		}
		void onCmdSearch()
		{
			DEBUGPRINTI("WifiApScan: start");
			WifiConnection* wificon = appController->getWifiConnection();
			wifiApScanner->start(wificon, [this](const WifiApScanner::Result& res)
			{
				DEBUGPRINTI("WifiApScan: finish items=%d", res.numWifiAp);
				wifiApList = res;
				ScopedLvLock sl;
				dropdown.clearOptions();
				dropdown.setSelected(-1);
				for(size_t c = wifiApList.numWifiAp, i = 0; i < c; ++i) { dropdown.addOption(wifiApList.arrWifiAp[i].ssid, (uint32_t)i); }
				dropdown.open();
			});
		}
		void onCmdClose()
		{
			dropdown.close();
			if(onRequestCloseCallback) { onRequestCloseCallback(); }
		}
		void onItemSelected()
		{
			uint32_t i = dropdown.getSelected();
			if(i < wifiApList.numWifiAp)
			{
				if(onItemSelectedCallback) { onItemSelectedCallback(wifiApList.arrWifiAp[i]); }
			}
		}
		// LvObj typed event callback
		void onLvEventCallback(lv_event_t* e)
		{
			LvEvent ev(e);
			if     (ev.testTargetAndCode(searchButton	, LV_EVENT_CLICKED		)) { callOnMain([this]() { ScopedLvLock sl; onCmdSearch(); }); }
			else if(ev.testTargetAndCode(closeButton	, LV_EVENT_CLICKED		)) { callOnMain([this]() { ScopedLvLock sl; onCmdClose(); }); }
			else if(ev.testTargetAndCode(dropdown		, LV_EVENT_VALUE_CHANGED)) { callOnMain([this]() { ScopedLvLock sl; onItemSelected(); }); }
		}
	};

	struct WifiApRowPane : public FieldRowPaneImplBase, private WifiConnection::Listener
	{
		LvTextArea ssidTextArea;
		LvTextArea passTextArea;
		LvButton scanButton;
		WifiApScanModalRowPane scanModalRowPane;
		WifiApRowPane() = delete;
		WifiApRowPane(AppController* ac, SharedRes* sr) : FieldRowPaneImplBase(ac, sr), scanModalRowPane(ac, sr)
		{
			appController->getWifiConnection()->addListener(this);
		}
		~WifiApRowPane() override
		{
			appController->getWifiConnection()->removeListener(this);
		}
		void create(LvKeyboard kb, LvObj parent)
		{
			FieldRowPaneImplBase::create(kb, parent, "Access Point", BTNF_APPLY | BTNF_CANCEL);
			ssidTextArea = addTextArea();
			ssidTextArea.setPlaceholderText("SSID");
			ssidTextArea.setMaxLength(MaxSSID);
			ssidTextArea.addTypedEventCb(this, LV_EVENT_VALUE_CHANGED);
			passTextArea = addTextArea();
			passTextArea.setPlaceholderText("Password");
			passTextArea.setMaxLength(63);
			passTextArea.setPasswordMode(true);
			passTextArea.addTypedEventCb(this, LV_EVENT_VALUE_CHANGED);
			scanButton = LvButton::createStaticTextButton(ptr, LV_SYMBOL_LIST);
			scanButton.addTypedEventCb(this, LV_EVENT_CLICKED);
			scanModalRowPane.create(kb, parent);
			scanModalRowPane.onItemSelectedCallback = [this](const WifiAp& ap) { onItemSelected(ap); };
			scanModalRowPane.onRequestCloseCallback = [this]() { showModalRow(false); };
			applyButton.addTypedEventCb(this, LV_EVENT_CLICKED);
			cancelButton.addTypedEventCb(this, LV_EVENT_CLICKED);
			showModalRow(false);
			restore();
		}
		void showModalRow(bool show)
		{
			scanButton.setDisabled(show);
			scanModalRowPane.setHidden(!show);
		}
		void onItemSelected(const WifiAp& ap)
		{
			ssidTextArea.setText(ap.ssid);
			if(!editing) setEditing(true);
		}
		void restore()
		{
			const WifiConfig& cfg = appController->getWifiConnection()->getConfig();
			ssidTextArea.setText(cfg.ssid);
			passTextArea.setText(cfg.password);
			callOnMain([this]() { ScopedLvLock sl; setEditing(false); });
		}
		void apply()
		{
			WifiConfig cfg;
			cfg.ssid = ssidTextArea.getText();
			cfg.password = passTextArea.getText();
			appController->getWifiConnection()->setConfig(cfg);
			showModalRow(false);
		}
		void revert()
		{
			showModalRow(false);
			restore();
		}
		// LvObj typed event handler
		void onLvEventCallback(lv_event_t* e)
		{
			LvEvent ev(e);
			if     (ev.testTargetAndCode(ssidTextArea	, LV_EVENT_VALUE_CHANGED)) { callOnMain([this]() { ScopedLvLock sl; if(!editing) { setEditing(true); } }); }
			else if(ev.testTargetAndCode(passTextArea	, LV_EVENT_VALUE_CHANGED)) { callOnMain([this]() { ScopedLvLock sl; if(!editing) { setEditing(true); } }); }
			else if(ev.testTargetAndCode(scanButton		, LV_EVENT_CLICKED		)) { callOnMain([this]() { ScopedLvLock sl; showModalRow(true); }); }
			else if(ev.testTargetAndCode(applyButton	, LV_EVENT_CLICKED		)) { callOnMain([this]() { ScopedLvLock sl; apply(); }); }
			else if(ev.testTargetAndCode(cancelButton	, LV_EVENT_CLICKED		)) { callOnMain([this]() { ScopedLvLock sl; revert(); }); }
		}
		// WifiConnection::Listener
		void wifiConnectionConfigDidChange(WifiConnection*, const WifiConfig&) override
		{
			ScopedLvLock sl;
			restore();
		}
	};

	struct WifiSectionPane : public SectionPaneImplBase
	{
		WifiApRowPane wifiApRowPane;
		WifiSectionPane() = delete;
		WifiSectionPane(AppController* ac, SharedRes* sr) : SectionPaneImplBase(ac, sr), wifiApRowPane(ac, sr)
		{
		}
		void create(LvKeyboard kb, LvObj parent)
		{
			SectionPaneImplBase::create(kb, parent, "Wi-Fi");
			wifiApRowPane.create(kb, ptr);
		}
	};

	// ================================================================================
	// system time panes

	struct TimeZoneRowPane : public FieldRowPaneImplBase, private AppController::Listener
	{
		LvDropdown dropdown;
		TimeZoneRowPane() = delete;
		TimeZoneRowPane(AppController* ac, SharedRes* sr) : FieldRowPaneImplBase(ac, sr)
		{
			appController->addListener(this);
		}
		~TimeZoneRowPane() override
		{
			appController->removeListener(this);
		}
		void create(LvKeyboard kb, LvObj parent)
		{
			FieldRowPaneImplBase::create(kb, parent, "Time Zone", BTNF_APPLY | BTNF_CANCEL);
			dropdown = addDropdown();
			AppController::TzArray tzlist = appController->getTimeZoneList();
			strutil::Str<128> sbuf;
			for(size_t c = tzlist.size, i = 0; i < c; ++i)
			{
				sbuf.format("%s (%s)", tzlist.ptr[i].cityName, tzlist.ptr[i].utcoffset);
				dropdown.addOption(sbuf, (uint32_t)i);
			}
			dropdown.addTypedEventCb(this, LV_EVENT_VALUE_CHANGED);
			applyButton.addTypedEventCb(this, LV_EVENT_CLICKED);
			cancelButton.addTypedEventCb(this, LV_EVENT_CLICKED);
			restore();
		}
		void restore()
		{
			dropdown.setSelected((uint32_t)appController->getTimeZoneIndex());
			callOnMain([this]() { ScopedLvLock sl; setEditing(false); });
		}
		void apply()
		{
			uint32_t i = dropdown.getSelected();
			if(i < appController->getTimeZoneList().size) appController->setTimeZoneIndex(i);
		}
		void revert()
		{
			restore();
		}
		// LvObj typed event callback
		void onLvEventCallback(lv_event_t* e)
		{
			LvEvent ev(e);
			if     (ev.testTargetAndCode(dropdown		, LV_EVENT_VALUE_CHANGED)) { callOnMain([this]() { ScopedLvLock sl; if(!editing) { setEditing(true); } }); }
			else if(ev.testTargetAndCode(applyButton	, LV_EVENT_CLICKED		)) { callOnMain([this]() { ScopedLvLock sl; apply(); }); }
			else if(ev.testTargetAndCode(cancelButton	, LV_EVENT_CLICKED		)) { callOnMain([this]() { ScopedLvLock sl; revert(); }); }
		}
		// AppController::Listener
		void appControllerTimeZoneIndexDidChange(AppController*, size_t) override
		{
			ScopedLvLock sl;
			restore();
		}
	};

	struct CurrentTimeRowPane : public FieldRowPaneImplBase
	{
		LvTextArea dateTextArea;
		LvTextArea timeTextArea;
		LvButton refreshButton;
		CurrentTimeRowPane() = delete;
		CurrentTimeRowPane(AppController* ac, SharedRes* sr) : FieldRowPaneImplBase(ac, sr)
		{
		}
		void create(LvKeyboard kb, LvObj parent)
		{
			FieldRowPaneImplBase::create(kb, parent, "Local Time", BTNF_APPLY | BTNF_CANCEL);
			dateTextArea = addTextArea();
			dateTextArea.setPlaceholderText("YYYY-mm-dd");
			dateTextArea.addTypedEventCb(this, LV_EVENT_VALUE_CHANGED);
			timeTextArea = addTextArea();
			timeTextArea.setPlaceholderText("HH:MM:SS");
			timeTextArea.addTypedEventCb(this, LV_EVENT_VALUE_CHANGED);
			applyButton.addTypedEventCb(this, LV_EVENT_CLICKED);
			cancelButton.addTypedEventCb(this, LV_EVENT_CLICKED);
			refreshButton = LvButton::createStaticTextButton(ptr, LV_SYMBOL_REFRESH);
			refreshButton.moveToIndex(applyButton.getIndex());
			refreshButton.addTypedEventCb(this, LV_EVENT_CLICKED);
			restore();
		}
		void restore()
		{
			time_t t = time(nullptr);
			tm stm{};
			const tm* ptm = platform::localtime(&t, &stm);
			strutil::Str<16> dbuf; strftime(dbuf.getBuffer(), dbuf.capacity(), "%Y-%m-%d", ptm);
			strutil::Str<16> tbuf; strftime(tbuf.getBuffer(), tbuf.capacity(), "%H:%M:%S", ptm);
			dateTextArea.setText(dbuf);
			timeTextArea.setText(tbuf);
			callOnMain([this]() { ScopedLvLock sl; setEditing(false); });
		}
		void apply()
		{
			tm stm{};
			char a = 0, b = 0;
			int dcs = sscanf(dateTextArea.getText(), "%d%c%d%c%d", &stm.tm_year, &a, &stm.tm_mon, &b, &stm.tm_mday); stm.tm_year -= 1900; stm.tm_mon -= 1;
			int tcs = sscanf(timeTextArea.getText(), "%d%c%d%c%d", &stm.tm_hour, &a, &stm.tm_min, &b, &stm.tm_sec);
			if((dcs == 3) && (tcs == 3))
			{
				time_t t = mktime(&stm);
				if(0 <= t) { platform::setSystemTime(t); }
			}
			restore();
		}
		void revert()
		{
			restore();
		}
		// LvObj typed event callback
		void onLvEventCallback(lv_event_t* e)
		{
			LvEvent ev(e);
			if     (ev.testTargetAndCode(dateTextArea	, LV_EVENT_VALUE_CHANGED)) { callOnMain([this]() { ScopedLvLock sl; if(!editing) { setEditing(true); } }); }
			else if(ev.testTargetAndCode(timeTextArea	, LV_EVENT_VALUE_CHANGED)) { callOnMain([this]() { ScopedLvLock sl; if(!editing) { setEditing(true); } }); }
			else if(ev.testTargetAndCode(applyButton	, LV_EVENT_CLICKED		)) { callOnMain([this]() { ScopedLvLock sl; apply(); }); }
			else if(ev.testTargetAndCode(cancelButton	, LV_EVENT_CLICKED		)) { callOnMain([this]() { ScopedLvLock sl; revert(); }); }
			else if(ev.testTargetAndCode(refreshButton	, LV_EVENT_CLICKED		)) { callOnMain([this]() { ScopedLvLock sl; bool editingsave = editing; restore(); if(editingsave) setEditing(true); }); }
		}
	};

	struct SntpRowPane : public FieldRowPaneImplBase, private AppController::Listener
	{
		LvTextArea sntpTextArea;
		SntpRowPane() = delete;
		SntpRowPane(AppController* ac, SharedRes* sr) : FieldRowPaneImplBase(ac, sr)
		{
			appController->addListener(this);
		}
		~SntpRowPane() override
		{
			appController->removeListener(this);
		}
		void create(LvKeyboard kb, LvObj parent)
		{
			FieldRowPaneImplBase::create(kb, parent, "SNTP Server", BTNF_APPLY | BTNF_CANCEL);
			sntpTextArea = addTextArea();
			sntpTextArea.setPlaceholderText("server url");
			sntpTextArea.addTypedEventCb(this, LV_EVENT_VALUE_CHANGED);
			applyButton.addTypedEventCb(this, LV_EVENT_CLICKED);
			cancelButton.addTypedEventCb(this, LV_EVENT_CLICKED);
			restore();
		}
		void restore()
		{
			sntpTextArea.setText(appController->getSntpServer());
			callOnMain([this]() { ScopedLvLock sl; setEditing(false); });
		}
		void apply()
		{
			appController->setSntpServer(sntpTextArea.getText());
		}
		void revert()
		{
			restore();
		}
		// LvObj typed event callback
		void onLvEventCallback(lv_event_t* e)
		{
			LvEvent ev(e);
			if     (ev.testTargetAndCode(sntpTextArea	, LV_EVENT_VALUE_CHANGED)) { callOnMain([this]() { ScopedLvLock sl; if(!editing) { setEditing(true); } }); }
			else if(ev.testTargetAndCode(applyButton	, LV_EVENT_CLICKED		)) { callOnMain([this]() { ScopedLvLock sl; apply(); }); }
			else if(ev.testTargetAndCode(cancelButton	, LV_EVENT_CLICKED		)) { callOnMain([this]() { ScopedLvLock sl; revert(); }); }
		}
		// AppController::Listener
		void appControllerSntpServerDidChange(AppController*, const char*) override
		{
			ScopedLvLock sl;
			restore();
		}
	};

	struct SystemTimeSectionPane : public SectionPaneImplBase
	{
		TimeZoneRowPane timeZoneRowPane;
		CurrentTimeRowPane currentTimeRowPane;
		SntpRowPane sntpRowPane;
		SystemTimeSectionPane() = delete;
		SystemTimeSectionPane(AppController* ac, SharedRes* sr) : SectionPaneImplBase(ac, sr), timeZoneRowPane(ac, sr), currentTimeRowPane(ac, sr), sntpRowPane(ac, sr)
		{
		}
		void create(LvKeyboard kb, LvObj parent)
		{
			SectionPaneImplBase::create(kb, parent, "System Time");
			timeZoneRowPane.create(kb, ptr);
			currentTimeRowPane.create(kb, ptr);
			sntpRowPane.create(kb, ptr);
		}
	};

	// ================================================================================
	// weather service panes

	struct WeatherApiKeyRowPane : public FieldRowPaneImplBase, private WeatherTask::Listener
	{
		LvTextArea apikeyTextArea;
		WeatherApiKeyRowPane() = delete;
		WeatherApiKeyRowPane(AppController* ac, SharedRes* sr)  : FieldRowPaneImplBase(ac, sr)
		{
			appController->getWeatherTask()->addListener(this);
		}
		~WeatherApiKeyRowPane() override
		{
			appController->getWeatherTask()->removeListener(this);
		}
		void create(LvKeyboard kb, LvObj parent)
		{
			FieldRowPaneImplBase::create(kb, parent, "API Key", BTNF_APPLY | BTNF_CANCEL);
			apikeyTextArea = addTextArea();
			apikeyTextArea.setPlaceholderText("api key");
			apikeyTextArea.addTypedEventCb(this, LV_EVENT_VALUE_CHANGED);
			applyButton.addTypedEventCb(this, LV_EVENT_CLICKED);
			cancelButton.addTypedEventCb(this, LV_EVENT_CLICKED);
			restore();
		}
		void restore()
		{
			apikeyTextArea.setText(appController->getWeatherTask()->getApiKey());
			callOnMain([this]() { ScopedLvLock sl; setEditing(false); });
		}
		void apply()
		{
			appController->getWeatherTask()->setApiKey(apikeyTextArea.getText());
		}
		void revert()
		{
			restore();
		}
		// LvObj typed event callback
		void onLvEventCallback(lv_event_t* e)
		{
			LvEvent ev(e);
			if     (ev.testTargetAndCode(apikeyTextArea	, LV_EVENT_VALUE_CHANGED)) { callOnMain([this]() { ScopedLvLock sl; if(!editing) { setEditing(true); } }); }
			else if(ev.testTargetAndCode(applyButton	, LV_EVENT_CLICKED		)) { callOnMain([this]() { ScopedLvLock sl; apply(); }); }
			else if(ev.testTargetAndCode(cancelButton	, LV_EVENT_CLICKED		)) { callOnMain([this]() { ScopedLvLock sl; revert(); }); }
		}
		// WeatherTask::Listener
		void weatherTaskConfigDidChange(WeatherTask*, const char*, const WeatherLocation&) override
		{
			ScopedLvLock sl;
			restore();
		}
	};

	struct WeatherLocSearchModalRowPane : public FieldRowPaneImplBase
	{
		LvTextArea queryTextArea;
		LvDropdown dropdown;
		LvButton searchButton;
		LvButton closeButton;
		xheap::uniquePtr<WeatherLocationQuery> locationQuery = WeatherLocationQuery::createInstance();
		std::array<WeatherLocation, WeatherLocationQuery::MaxLocationList> arrLocations;
		size_t numLocations = 0;
		std::function<void(const WeatherLocation&)> onItemSelectedCallback;
		std::function<void()> onRequestCloseCallback;
		WeatherLocSearchModalRowPane() = delete;
		WeatherLocSearchModalRowPane(AppController* ac, SharedRes* sr) : FieldRowPaneImplBase(ac, sr)
		{
		}
		void create(LvKeyboard kb, LvObj parent)
		{
			FieldRowPaneImplBase::create(kb, parent, nullptr, 0);
			queryTextArea = addTextArea();
			queryTextArea.setPlaceholderText("city to search");
			dropdown = addDropdown();
			dropdown.addTypedEventCb(this, LV_EVENT_VALUE_CHANGED);
			searchButton = LvButton::createStaticTextButton(ptr, LV_SYMBOL_REFRESH);
			searchButton.addTypedEventCb(this, LV_EVENT_CLICKED);
			closeButton = LvButton::createStaticTextButton(ptr, LV_SYMBOL_CLOSE);
			closeButton.addTypedEventCb(this, LV_EVENT_CLICKED);
		}
		void onCmdSearch()
		{
			const char* apikey = appController->getWeatherTask()->getApiKey();
			const char* query = queryTextArea.getText();
			if(strutil::isEmpty(apikey) || strutil::isEmpty(query)) return;
			DEBUGPRINTI("WeatherLocationQuery: start, query=%s", query);
			locationQuery->query(apikey, query, [this](const WeatherLocationQuery::Result& r)
			{
				DEBUGPRINTI("WeatherLocationQuery: finished, status=%d size=%d", r.apiStatus, (int)r.numLocations);
				if(r.apiStatus != 200) return;
				arrLocations = r.arrLocations;
				numLocations = r.numLocations;
				ScopedLvLock sl;
				dropdown.clearOptions();
				for(size_t c = numLocations, i = 0; i < c; ++i) { dropdown.addOption(arrLocations[i].cityName, (uint32_t)i); }
				dropdown.open();
			});
		}
		void onCmdClose()
		{
			dropdown.close();
			if(onRequestCloseCallback) { onRequestCloseCallback(); }
		}
		void onItemSelected()
		{
			uint32_t i = dropdown.getSelected();
			if(i < numLocations)
			{
				if(onItemSelectedCallback) { onItemSelectedCallback(arrLocations[i]); }
			}
		}
		// LvObj typed event callback
		void onLvEventCallback(lv_event_t* e)
		{
			LvEvent ev(e);
			if     (ev.testTargetAndCode(searchButton	, LV_EVENT_CLICKED		)) { callOnMain([this]() { ScopedLvLock sl; onCmdSearch(); }); }
			else if(ev.testTargetAndCode(closeButton	, LV_EVENT_CLICKED		)) { callOnMain([this]() { ScopedLvLock sl; onCmdClose(); }); }
			else if(ev.testTargetAndCode(dropdown		, LV_EVENT_VALUE_CHANGED)) { callOnMain([this]() { ScopedLvLock sl; onItemSelected(); }); }
		}
	};

	struct WeatherLocationRowPane : public FieldRowPaneImplBase, private WeatherTask::Listener
	{
		LvTextArea locTextArea;
		LvButton searchButton;
		WeatherLocSearchModalRowPane searchModalRowPane;
		WeatherLocation curLocation{};
		WeatherLocationRowPane() = delete;
		WeatherLocationRowPane(AppController* ac, SharedRes* sr) : FieldRowPaneImplBase(ac, sr), searchModalRowPane(ac, sr)
		{
			appController->getWeatherTask()->addListener(this);
		}
		~WeatherLocationRowPane() override
		{
			appController->getWeatherTask()->removeListener(this);
		}
		void create(LvKeyboard kb, LvObj parent)
		{
			FieldRowPaneImplBase::create(kb, parent, "Location", BTNF_APPLY | BTNF_CANCEL);
			locTextArea = addTextArea();
			locTextArea.setDisabled(true);
			searchButton = LvButton::createStaticTextButton(ptr, LV_SYMBOL_LIST);
			searchButton.addTypedEventCb(this, LV_EVENT_CLICKED);
			searchModalRowPane.create(kb, parent);
			searchModalRowPane.onItemSelectedCallback = [this](const WeatherLocation& loc) { onItemSelected(loc); };
			searchModalRowPane.onRequestCloseCallback = [this]() { showModalRow(false); };
			applyButton.addTypedEventCb(this, LV_EVENT_CLICKED);
			cancelButton.addTypedEventCb(this, LV_EVENT_CLICKED);
			showModalRow(false);
			restore();
		}
		void showModalRow(bool show)
		{
			searchButton.setDisabled(show);
			searchModalRowPane.setHidden(!show);
		}
		void onItemSelected(const WeatherLocation& loc)
		{
			curLocation = loc;
			locTextArea.setText(curLocation.cityName);
			if(!editing) setEditing(true);
		}
		void restore()
		{
			curLocation = appController->getWeatherTask()->getLocation();
			locTextArea.setText(curLocation.cityName);
			callOnMain([this]() { ScopedLvLock sl; setEditing(false); });
		}
		void apply()
		{
			appController->getWeatherTask()->setLocation(curLocation);
			showModalRow(false);
		}
		void revert()
		{
			showModalRow(false);
			restore();
		}
		// LvObj typed event callback
		void onLvEventCallback(lv_event_t* e)
		{
			LvEvent ev(e);
			if     (ev.testTargetAndCode(searchButton	, LV_EVENT_CLICKED		)) { callOnMain([this]() { ScopedLvLock sl; showModalRow(true); }); }
			else if(ev.testTargetAndCode(applyButton	, LV_EVENT_CLICKED		)) { callOnMain([this]() { ScopedLvLock sl; apply(); }); }
			else if(ev.testTargetAndCode(cancelButton	, LV_EVENT_CLICKED		)) { callOnMain([this]() { ScopedLvLock sl; revert(); }); }
		}
		// WeatherTask::Listener
		void weatherTaskConfigDidChange(WeatherTask*, const char*, const WeatherLocation&) override
		{
			ScopedLvLock sl;
			restore();
		}
	};

	struct WeatherServiceSectionPane : public SectionPaneImplBase
	{
		WeatherApiKeyRowPane weatherApiKeyRowPane;
		WeatherLocationRowPane weatherLocationRowPane;
		WeatherServiceSectionPane() = delete;
		WeatherServiceSectionPane(AppController* ac, SharedRes* sr) : SectionPaneImplBase(ac, sr), weatherApiKeyRowPane(ac, sr), weatherLocationRowPane(ac, sr)
		{
		}
		void create(LvKeyboard kb, LvObj parent)
		{
			SectionPaneImplBase::create(kb, parent, "Weather Service powered by OpenWeather \xc2\xae");
			weatherApiKeyRowPane.create(kb, ptr);
			weatherLocationRowPane.create(kb, ptr);
		}
	};

	// ================================================================================
	// history server panes

	struct HistoryServerHostRowPane : public FieldRowPaneImplBase, private HistoryServer::Listener
	{
		LvTextArea hostTextArea;
		LvTextArea portTextArea;
		HistoryServerHostRowPane() = delete;
		HistoryServerHostRowPane(AppController* ac, SharedRes* sr) : FieldRowPaneImplBase(ac, sr)
		{
			appController->getHistoryServer()->addListener(this);
		}
		~HistoryServerHostRowPane() override
		{
			appController->getHistoryServer()->removeListener(this);
		}
		void create(LvKeyboard kb, LvObj parent)
		{
			FieldRowPaneImplBase::create(kb, parent, "Web Server", BTNF_APPLY | BTNF_CANCEL);
			hostTextArea = addTextArea();
			hostTextArea.setPlaceholderText("Hostname");
			hostTextArea.setMaxLength(15);
			hostTextArea.addTypedEventCb(this, LV_EVENT_VALUE_CHANGED);
			portTextArea = addTextArea();
			portTextArea.setPlaceholderText("Port");
			portTextArea.setMaxLength(8);
			portTextArea.setAcceptedChars("0123456789");
			portTextArea.setStyleTextAlign(LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
			portTextArea.addTypedEventCb(this, LV_EVENT_VALUE_CHANGED);
			applyButton.addTypedEventCb(this, LV_EVENT_CLICKED);
			cancelButton.addTypedEventCb(this, LV_EVENT_CLICKED);
			restore();
		}
		void restore()
		{
			const HistoryServer::Config& cfg = appController->getHistoryServer()->getConfig();
			hostTextArea.setText(cfg.hostname);
			portTextArea.setText(strutil::Str<16>::formatted("%u", cfg.port));
			callOnMain([this]() { ScopedLvLock sl; setEditing(false); });
		}
		void apply()
		{
			HistoryServer::Config cfg = appController->getHistoryServer()->getConfig();
			cfg.hostname = hostTextArea.getText();
			cfg.port = std::max(0, std::min(65535, (int)std::strtol(portTextArea.getText(), nullptr, 0)));
			appController->getHistoryServer()->setConfig(cfg);
		}
		void revert()
		{
			restore();
		}
		// LvObj typed event handler
		void onLvEventCallback(lv_event_t* e)
		{
			LvEvent ev(e);
			if     (ev.testTargetAndCode(hostTextArea	, LV_EVENT_VALUE_CHANGED)) { callOnMain([this]() { ScopedLvLock sl; if(!editing) { setEditing(true); } }); }
			else if(ev.testTargetAndCode(portTextArea	, LV_EVENT_VALUE_CHANGED)) { callOnMain([this]() { ScopedLvLock sl; if(!editing) { setEditing(true); } }); }
			else if(ev.testTargetAndCode(applyButton	, LV_EVENT_CLICKED		)) { callOnMain([this]() { ScopedLvLock sl; apply(); }); }
			else if(ev.testTargetAndCode(cancelButton	, LV_EVENT_CLICKED		)) { callOnMain([this]() { ScopedLvLock sl; revert(); }); }
		}
		// WifiConnection::Listener
		void historyServerConfigDidChange(HistoryServer*, const HistoryServer::Config&) override
		{
			ScopedLvLock sl;
			restore();
		}
	};

	struct HistoryServerUserRowPane : public FieldRowPaneImplBase, private HistoryServer::Listener
	{
		LvTextArea userTextArea;
		LvTextArea passTextArea;
		HistoryServerUserRowPane() = delete;
		HistoryServerUserRowPane(AppController* ac, SharedRes* sr) : FieldRowPaneImplBase(ac, sr)
		{
			appController->getHistoryServer()->addListener(this);
		}
		~HistoryServerUserRowPane() override
		{
			appController->getHistoryServer()->removeListener(this);
		}
		void create(LvKeyboard kb, LvObj parent)
		{
			FieldRowPaneImplBase::create(kb, parent, "Authentication", BTNF_APPLY | BTNF_CANCEL);
			userTextArea = addTextArea();
			userTextArea.setPlaceholderText("Username");
			userTextArea.setMaxLength(15);
			userTextArea.addTypedEventCb(this, LV_EVENT_VALUE_CHANGED);
			passTextArea = addTextArea();
			passTextArea.setPlaceholderText("Password");
			passTextArea.setMaxLength(15);
			passTextArea.setPasswordMode(true);
			passTextArea.addTypedEventCb(this, LV_EVENT_VALUE_CHANGED);
			applyButton.addTypedEventCb(this, LV_EVENT_CLICKED);
			cancelButton.addTypedEventCb(this, LV_EVENT_CLICKED);
			restore();
		}
		void restore()
		{
			const HistoryServer::Config& cfg = appController->getHistoryServer()->getConfig();
			userTextArea.setText(cfg.username);
			passTextArea.setText(cfg.password);
			callOnMain([this]() { ScopedLvLock sl; setEditing(false); });
		}
		void apply()
		{
			HistoryServer::Config cfg = appController->getHistoryServer()->getConfig();
			cfg.username = userTextArea.getText();
			cfg.password = passTextArea.getText();
			appController->getHistoryServer()->setConfig(cfg);
		}
		void revert()
		{
			restore();
		}
		// LvObj typed event handler
		void onLvEventCallback(lv_event_t* e)
		{
			LvEvent ev(e);
			if     (ev.testTargetAndCode(userTextArea	, LV_EVENT_VALUE_CHANGED)) { callOnMain([this]() { ScopedLvLock sl; if(!editing) { setEditing(true); } }); }
			else if(ev.testTargetAndCode(passTextArea	, LV_EVENT_VALUE_CHANGED)) { callOnMain([this]() { ScopedLvLock sl; if(!editing) { setEditing(true); } }); }
			else if(ev.testTargetAndCode(applyButton	, LV_EVENT_CLICKED		)) { callOnMain([this]() { ScopedLvLock sl; apply(); }); }
			else if(ev.testTargetAndCode(cancelButton	, LV_EVENT_CLICKED		)) { callOnMain([this]() { ScopedLvLock sl; revert(); }); }
		}
		// WifiConnection::Listener
		void historyServerConfigDidChange(HistoryServer*, const HistoryServer::Config&) override
		{
			ScopedLvLock sl;
			restore();
		}
	};

	struct HistoryServerSectionPane : public SectionPaneImplBase
	{
		HistoryServerHostRowPane historyServerHostRowPane;
		HistoryServerUserRowPane historyServerUserRowPane;
		HistoryServerSectionPane() = delete;
		HistoryServerSectionPane(AppController* ac, SharedRes* sr) : SectionPaneImplBase(ac, sr), historyServerHostRowPane(ac, sr), historyServerUserRowPane(ac, sr)
		{
		}
		void create(LvKeyboard kb, LvObj parent)
		{
			SectionPaneImplBase::create(kb, parent, "Sensor History Server");
			historyServerHostRowPane.create(kb, ptr);
			historyServerUserRowPane.create(kb, ptr);
		}
	};

	// ================================================================================
	// sensor alert panes

	static const std::array<int, 5> SensorIndices =
	{
		SensorTask::Ibme680Temp,
		SensorTask::Ibme680Hum,
		SensorTask::Ibme680Pres,
		SensorTask::Iscd4xCo2,
		SensorTask::Ibme680Iaq
	};

	struct SensorAlertSectionPane : public SectionPaneImplBase, private SensorTask::Listener, private AlarmSounder::Listener
	{
		struct RowEnt
		{
			LvLabel nameLabel;
			LvSlider valSlider;
			LvLabel valLabel;
			LvSwitch enswitch;
			strutil::Str<20> valBuf;
		};
		std::array<RowEnt, 5> rowEntList;
		RowEnt sndRowEnt;
		LvButton audButton;
		SensorAlertSectionPane() = delete;
		SensorAlertSectionPane(AppController* ac, SharedRes* sr) : SectionPaneImplBase(ac, sr)
		{
			appController->getSensorTask()->addListener(this);
			appController->getAlarmSounder()->addListener(this);
		}
		~SensorAlertSectionPane() override
		{
			appController->getAlarmSounder()->removeListener(this);
			appController->getSensorTask()->removeListener(this);
		}
		void create(LvKeyboard kb, LvObj parent)
		{
			SectionPaneImplBase::create(kb, parent, "Sensor Alert");
			LvObj grid = LvObj::create(ptr);
			grid.setStyleBorderWidth(0, LV_PART_MAIN);
			grid.setStyleOutlineWidth(0, LV_PART_MAIN);
			grid.setStyleBgOpa(LV_OPA_0, LV_PART_MAIN);
			grid.setStylePadAll(0, LV_PART_MAIN);
			grid.setStylePadBottom(Metrics::Padding, LV_PART_MAIN);
			grid.setStylePadColRow(Metrics::Padding, Metrics::Padding, LV_PART_MAIN);
			grid.setSize(LV_PCT(100), LV_SIZE_CONTENT);
			enum { ColName, ColValSlider, ColValLabel, ColEnSwitch };
			enum { RowThrHdr, RowTemp, RowHum, RowPres, RowCo2, RowIaq, RowSndHdr, RowSnd };
			constexpr int32_t ValSliderWidth = Metrics::FieldValueWidth * 6 / 4;
			constexpr int32_t ValLabelWidth = Metrics::FieldLabelWidth * 4 / 4;
			constexpr int32_t EnSwitchWidth = Metrics::FieldLabelWidth * 3 / 8;
			constexpr int32_t HdrRowHeight = Metrics::Font5Height * 6 / 4;
			constexpr int32_t RowHeight = Metrics::FieldRowHeight * 5 / 4;
			constexpr int32_t AudBtnWidth = RowHeight;
			static const int32_t GridColDsc[] = { Metrics::FieldLabelWidth, ValSliderWidth, ValLabelWidth, EnSwitchWidth, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
			static const int32_t GridRowDsc[] = { HdrRowHeight, RowHeight, RowHeight, RowHeight, RowHeight, RowHeight, HdrRowHeight, RowHeight, LV_GRID_TEMPLATE_LAST };
			grid.setGridDscArray(GridColDsc, GridRowDsc);
			// threthold header row
			LvLabel hdrrng = LvLabel::create(grid);
			hdrrng.setTextStatic("Comfortable Range");
			hdrrng.setGridCell(LV_GRID_ALIGN_CENTER, ColValSlider, 1, LV_GRID_ALIGN_END, RowThrHdr, 1);
			LvLabel hdrwrn = LvLabel::create(grid);
			hdrwrn.setTextStatic("Alert");
			hdrwrn.setGridCell(LV_GRID_ALIGN_CENTER, ColEnSwitch, 1, LV_GRID_ALIGN_END, RowThrHdr, 1);
			// threthold parameter rows
			for(int irow = 0; irow < 5; ++irow)
			{
				int isensor = SensorIndices[irow];
				RowEnt& rent = rowEntList[irow];
				const auto& rng = SensorTask::Thresholds::Ranges.array[isensor];
				rent.nameLabel = LvLabel::create(grid);
				rent.nameLabel.setGridCell(LV_GRID_ALIGN_END, ColName, 1, LV_GRID_ALIGN_CENTER, irow + 1, 1);
				rent.nameLabel.setTextStatic(SensorTask::MetadataList[isensor].title);
				rent.valSlider = LvSlider::create(grid);
				rent.valSlider.setSize(ValSliderWidth, Metrics::FieldRowHeight * 3 / 4);
				rent.valSlider.setGridCell(LV_GRID_ALIGN_CENTER, ColValSlider, 1, LV_GRID_ALIGN_CENTER, irow + 1, 1);
				rent.valSlider.setStylePadHor(Metrics::FieldRowHeight / 2, LV_PART_MAIN); // reduce the swipe range to avoid clipping
				rent.valSlider.setStylePadVer(Metrics::FieldRowHeight / 8, LV_PART_MAIN); // rail thickness
				rent.valSlider.setMode(LV_SLIDER_MODE_RANGE);
				rent.valSlider.setRange((int32_t)rng.under, (int32_t)rng.over);
				rent.valSlider.addTypedEventCb(this, LV_EVENT_VALUE_CHANGED);
				rent.valLabel = LvLabel::create(grid);
				rent.valLabel.setGridCell(LV_GRID_ALIGN_CENTER, ColValLabel, 1, LV_GRID_ALIGN_CENTER, irow + 1, 1);
				rent.enswitch = LvSwitch::create(grid);
				rent.enswitch.setSize(EnSwitchWidth, Metrics::FieldRowHeight * 6 / 8);
				rent.enswitch.setGridCell(LV_GRID_ALIGN_CENTER, ColEnSwitch, 1, LV_GRID_ALIGN_CENTER, irow + 1, 1);
				rent.enswitch.addTypedEventCb(this, LV_EVENT_VALUE_CHANGED);
			}
			// sound header row
			LvLabel hdrvol = LvLabel::create(grid);
			hdrvol.setTextStatic("Volume");
			hdrvol.setGridCell(LV_GRID_ALIGN_CENTER, ColValSlider, 1, LV_GRID_ALIGN_END, RowSndHdr, 1);
			LvLabel hdrsen = LvLabel::create(grid);
			hdrsen.setTextStatic("Enable");
			hdrsen.setGridCell(LV_GRID_ALIGN_CENTER, ColEnSwitch, 1, LV_GRID_ALIGN_END, RowSndHdr, 1);
			// sound volume row
			LvLabel sname = LvLabel::create(grid);
			sname.setTextStatic("Sound");
			sname.setGridCell(LV_GRID_ALIGN_END, ColName, 1, LV_GRID_ALIGN_CENTER, RowSnd, 1);
			audButton = LvButton::createStaticTextButton(grid, LV_SYMBOL_PLAY);
			audButton.setSize(AudBtnWidth, Metrics::FieldRowHeight);
			audButton.setGridCell(LV_GRID_ALIGN_START, ColValSlider, 1, LV_GRID_ALIGN_CENTER, RowSnd, 1);
			audButton.addTypedEventCb(this, LV_EVENT_CLICKED);
			sndRowEnt.valSlider = LvSlider::create(grid);
			sndRowEnt.valSlider.setSize(ValSliderWidth - AudBtnWidth - Metrics::Padding, Metrics::FieldRowHeight * 3 / 4);
			sndRowEnt.valSlider.setGridCell(LV_GRID_ALIGN_END, ColValSlider, 1, LV_GRID_ALIGN_CENTER, RowSnd, 1);
			sndRowEnt.valSlider.setStylePadHor(Metrics::FieldRowHeight / 2, LV_PART_MAIN); // reduce the swipe range to avoid clipping the knob
			sndRowEnt.valSlider.setStylePadVer(Metrics::FieldRowHeight / 8, LV_PART_MAIN); // rail thickness
			sndRowEnt.valSlider.setRange(0, 100);
			sndRowEnt.valSlider.addTypedEventCb(this, LV_EVENT_VALUE_CHANGED);
			sndRowEnt.valLabel = LvLabel::create(grid);
			sndRowEnt.valLabel.setGridCell(LV_GRID_ALIGN_CENTER, ColValLabel, 1, LV_GRID_ALIGN_CENTER, RowSnd, 1);
			sndRowEnt.enswitch = LvSwitch::create(grid);
			sndRowEnt.enswitch.setSize(EnSwitchWidth, Metrics::FieldRowHeight * 6 / 8);
			sndRowEnt.enswitch.setGridCell(LV_GRID_ALIGN_CENTER, ColEnSwitch, 1, LV_GRID_ALIGN_CENTER, RowSnd, 1);
			sndRowEnt.enswitch.addTypedEventCb(this, LV_EVENT_VALUE_CHANGED);
			sndRowEnt.enswitch.addTypedEventCb(this, LV_EVENT_VALUE_CHANGED);
			getScreen().addTypedEventCb(this, sharedRes->EventActivePageDidChange);
			restoreThr();
			restoreSnd();
			restoreaudition();
		}
		void restoreThr()
		{
			const SensorTask::Thresholds& th = appController->getSensorTask()->getThresholds();
			for(int irow = 0; irow < 5; ++irow)
			{
				int isensor = SensorIndices[irow];
				RowEnt& rent = rowEntList[irow];
				const auto& rng = th.array[isensor];
				rent.valSlider.setValue((int32_t)rng.over, LV_ANIM_OFF); // set before the left value
				rent.valSlider.setStartValue((int32_t)rng.under, LV_ANIM_OFF);
				rent.valBuf.format("%g - %g %s", rng.under, rng.over, SensorTask::MetadataList[isensor].unit);
				rent.valLabel.setTextStatic(rent.valBuf);
				const SensorTask::Falerts ma = 1 << isensor;
				rent.enswitch.setCheck(th.alerts & ma);
			}
		}
		void applyThr(int irow)
		{
			SensorTask::Thresholds th = appController->getSensorTask()->getThresholds();
			int isensor = SensorIndices[irow];
			RowEnt& rent = rowEntList[irow];
			auto rng = th.array[isensor];
			rng.over = (float)rent.valSlider.getValue();
			rng.under = (float)rent.valSlider.getLeftValue();
			rent.valBuf.format("%g - %g %s", rng.under, rng.over, SensorTask::MetadataList[isensor].unit);
			rent.valLabel.setTextStatic(rent.valBuf);
			th.array[isensor] = rng;
			const SensorTask::Falerts ma = 1 << isensor;
			th.alerts = (th.alerts & ~ma) | (rent.enswitch.isChecked() ? ma : 0);
			appController->getSensorTask()->setThresholds(th);
		}
		void restoreSnd()
		{
			AlarmSounder* as = appController->getAlarmSounder();
			int vol = as->getVolume();
			bool en = as->isEnabled();
			sndRowEnt.valSlider.setValue(vol, LV_ANIM_OFF);
			sndRowEnt.valBuf.format("%d", vol);
			sndRowEnt.valLabel.setTextStatic(sndRowEnt.valBuf);
			sndRowEnt.enswitch.setCheck(en);
			sndRowEnt.valSlider.setDisabled(!en);
			sndRowEnt.valLabel.setDisabled(!en);
			audButton.setDisabled(!en);
		}
		void applySnd()
		{
			AlarmSounder* as = appController->getAlarmSounder();
			int vol = sndRowEnt.valSlider.getValue();
			bool en = sndRowEnt.enswitch.isChecked();
			sndRowEnt.valBuf.format("%d", vol);
			sndRowEnt.valLabel.setTextStatic(sndRowEnt.valBuf);
			sndRowEnt.valSlider.setDisabled(!en);
			sndRowEnt.valLabel.setDisabled(!en);
			audButton.setDisabled(!en);
			as->setVolume(vol);
			as->setEnabled(en);
			if(!en) { audButton.setCheck(false); applyAudition(); }
		}
		void restoreaudition()
		{
			audButton.setCheck(appController->getAlarmSounder()->isAuditionOn());
		}
		void applyAudition()
		{
			appController->getAlarmSounder()->setAuditionOn(audButton.isChecked());
		}
		// LvObj typed event callback
		void onLvEventCallback(lv_event_t* e)
		{
			LvEvent ev(e);
			for(int irow = 0; irow < 5; ++irow)
			{
				RowEnt& rent = rowEntList[irow];
				if(ev.testTargetAndCode(rent.valSlider	, LV_EVENT_VALUE_CHANGED)) { callOnMain([this, irow]() { ScopedLvLock sl; applyThr(irow); }); return; }
				if(ev.testTargetAndCode(rent.enswitch	, LV_EVENT_VALUE_CHANGED)) { callOnMain([this, irow]() { ScopedLvLock sl; applyThr(irow); }); return; }
			}
			if(ev.testTargetAndCode(sndRowEnt.valSlider	, LV_EVENT_VALUE_CHANGED)) { callOnMain([this]() { ScopedLvLock sl; applySnd(); }); return; }
			if(ev.testTargetAndCode(sndRowEnt.enswitch	, LV_EVENT_VALUE_CHANGED)) { callOnMain([this]() { ScopedLvLock sl; applySnd(); }); return; }
			if(ev.testTargetAndCode(audButton			, LV_EVENT_CLICKED		)) { callOnMain([this]() { ScopedLvLock sl; audButton.setCheck(!audButton.isChecked()); applyAudition(); }); return; }
			if(ev.getCode() == sharedRes->EventActivePageDidChange)				   { callOnMain([this]() { ScopedLvLock sl; audButton.setCheck(false); applyAudition(); }); return; }
		}
		// SensorTask::Listener
		void sensorTaskThresholdsDidChange(SensorTask*, const SensorTask::Thresholds&) override
		{
			ScopedLvLock sl;
			restoreThr();
		}
		// AlarmSounder::Listener
		void alertSounderSettingsDidChange(AlarmSounder*, int, bool, bool) override
		{
			ScopedLvLock sl;
			restoreSnd();
		}
	};

	// ================================================================================
	// aboutbox panes

	struct LicenseNoticePopup : public LvObj
	{
		SharedRes* sharedRes;
		LvDropdown licDropdown;
		LvButton closeButton;
		LvLabel authorLabel;
		LvLabel typeLabel;
		LvLabel urlLabel;
		LvObj scrollPane;
		LvLabel textLabel;
		HeapBufferT<char> textBuffer;
		struct LicenseFileRef
		{
			const char* title;
			const char* author;
			const char* type;
			const char* url;
			const char* file;
		};
		struct LicenseTable
		{
			cjsonwrap::cJSONUniquePtr jsonRoot;
			HeapBufferT<LicenseFileRef> fileRefList;
			void load()
			{
				strutil::Str<128> path;
				BoardFs::resolveAssetFilePath("table_of_licenses.json", path.getBuffer(), path.capacity());
				HeapBufferT<char> json;
				BoardFs::loadStringFromFile(path, &json);
				jsonRoot.reset(cJSON_Parse(json.data()));
				const int32_t numlic = cJSON_GetArraySize(jsonRoot.get());
				fileRefList.resize(numlic);
				for(int32_t i = 0; i < numlic; ++i)
				{
					cJSON* dic = cJSON_GetArrayItem(jsonRoot.get(), (int)i);
					LicenseFileRef& ref = fileRefList[i];
					ref.title = cJSON_GetStringValue(cJSON_GetObjectItem(dic, "title"));
					ref.author = cJSON_GetStringValue(cJSON_GetObjectItem(dic, "author"));
					ref.type = cJSON_GetStringValue(cJSON_GetObjectItem(dic, "type"));
					ref.url = cJSON_GetStringValue(cJSON_GetObjectItem(dic, "url"));
					ref.file = cJSON_GetStringValue(cJSON_GetObjectItem(dic, "file"));
				}
			}
		} licTable;
		LicenseNoticePopup(SharedRes* sr) : sharedRes(sr)
		{
		}
		void create(LvObj parent)
		{
			LV_ASSERT(!ptr);
			textBuffer.reserve(16384);
			licTable.load();
			ptr = LvObj::create(parent);
			setStyleTextFont(sharedRes->fontLatin1, LV_PART_MAIN);
			setFlag(LV_OBJ_FLAG_FLOATING, true);
			setStyleRadius(Metrics::CornerRadius, LV_PART_MAIN);
			setStyleBorderWidth(0, LV_PART_MAIN);
			setStyleOutlineWidth(1, LV_PART_MAIN);
			setStyleOutlineColor(lv_color_make(0, 0, 0), LV_PART_MAIN);
			setStylePadAll(Metrics::Padding, LV_PART_MAIN);
			setStylePadColRow(0, Metrics::Margin, LV_PART_MAIN);
			setFlexFlow(LV_FLEX_FLOW_COLUMN);
			LvObj hdr = LvObj::create(ptr);
			hdr.setStyleBorderWidth(0, LV_PART_MAIN);
			hdr.setStylePadHor(0, LV_PART_MAIN);
			hdr.setStylePadVer(0, LV_PART_MAIN);
			hdr.setStylePadColRow(0, 0, LV_PART_MAIN);
			static const int32_t GridColDsc[] = { LV_GRID_FR(1), 40, LV_GRID_TEMPLATE_LAST };
			static const int32_t GridRowDsc[] = { LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
			hdr.setGridDscArray(GridColDsc, GridRowDsc);
			hdr.setSize(LV_PCT(100), LV_SIZE_CONTENT);
			licDropdown = LvDropdown::create(hdr);
			licDropdown.setGridCell(LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);
			licDropdown.setWidth(LV_PCT(50));
			licDropdown.setStyleTextFont(sharedRes->fontLatin1Bold, LV_PART_MAIN);
			licDropdown.setStyleTextAlign(LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
			licDropdown.clearOptions();
			for(const auto& ref : licTable.fileRefList)
			{
				licDropdown.addOption(ref.title, LV_DROPDOWN_POS_LAST);
			}
			licDropdown.setSelected(0);
			licDropdown.addTypedEventCb(this, LV_EVENT_VALUE_CHANGED);
			closeButton = LvButton::createStaticTextButton(hdr, LV_SYMBOL_CLOSE);
			closeButton.setGridCell(LV_GRID_ALIGN_END, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);
			closeButton.addTypedEventCb(this, LV_EVENT_CLICKED);
			authorLabel = LvLabel::create(ptr);
			authorLabel.setSize(LV_PCT(100), LV_SIZE_CONTENT);
			typeLabel = LvLabel::create(ptr);
			typeLabel.setSize(LV_PCT(100), LV_SIZE_CONTENT);
			urlLabel = LvLabel::create(ptr);
			urlLabel.setSize(LV_PCT(100), LV_SIZE_CONTENT);
			scrollPane = LvObj::create(ptr);
			scrollPane.setWidth(LV_PCT(100));
			scrollPane.setFlexGrow(1);
			textLabel = LvLabel::create(scrollPane);
			textLabel.setStyleBorderColor(LV_COLOR_MAKE(128, 128, 128), LV_PART_MAIN);
			textLabel.setStyleTextColor(LV_COLOR_MAKE(0, 0, 0), LV_PART_MAIN);
			textLabel.setSize(LV_PCT(100), LV_SIZE_CONTENT);
			textLabel.setLongMode(LV_LABEL_LONG_MODE_WRAP);
			getScreen().addTypedEventCb(this, sharedRes->EventActivePageDidChange);
			setHidden(true);
			setStyleOpa(LV_OPA_0, LV_PART_MAIN);
			showLicenseEntry(0);
		}
		void present()
		{
			LvObj parent = getParent();
			LvArea rc = parent.getBounds();
			setBounds(rc.increased(-16, -16));
			moveToIndex(-1);
			lvObjPresent(*this, true);
		}
		void dismiss()
		{
			lvObjDismiss(*this, true);
		}
		void showLicenseEntry(uint32_t ii)
		{
			if(ii < licTable.fileRefList.size())
			{
				const LicenseFileRef& ref = licTable.fileRefList[ii];
				authorLabel.setTextStatic(ref.author);
				typeLabel.setTextStatic(ref.type);
				urlLabel.setTextStatic(ref.url);
				{
					strutil::Str<128> path;
					BoardFs::resolveAssetFilePath(ref.file, path.getBuffer(), path.capacity());
					BoardFs::loadStringFromFile(path, &textBuffer);
					textLabel.setTextStatic(textBuffer.data());
				}
			}
			else
			{
				authorLabel.setTextStatic("");
				typeLabel.setTextStatic("");
				urlLabel.setTextStatic("");
				textLabel.setTextStatic("");
			}
			scrollPane.scrollToY(0, LV_ANIM_OFF);
		}
		// LvObj typed event callback
		void onLvEventCallback(lv_event_t* e)
		{
			LvEvent ev(e);
			if(ev.testTargetAndCode(closeButton, LV_EVENT_CLICKED)) { dismiss(); return; }
			if(ev.testTargetAndCode(licDropdown, LV_EVENT_VALUE_CHANGED)) { showLicenseEntry(licDropdown.getSelected()); }
			if(ev.getCode() == sharedRes->EventActivePageDidChange) { dismiss(); return; }
		}
	};

	struct AboutSectionPane : public SectionPaneImplBase
	{
		static void strreplace(const char* str, const char* key, const char* val, HeapBufferT<char>* buf)
		{
			if(!str) { return; }
			const char* pk = strstr(str, key); if(!pk) { return; }
			size_t ls = strlen(str), lk = strlen(key), lv = strlen(val);
			buf->reserve(ls + lv - lk + 1);
			buf->clear();
			buf->append(str, pk - str);
			buf->append(val, lv);
			buf->append(pk + lk, ls - (pk - str + lk));
			buf->append("", 1);
		}
		HeapBufferT<char> versionBuffer;
		LvButton licPopupButton;
		LicenseNoticePopup licenseNoticePopup;
		AboutSectionPane() = delete;
		AboutSectionPane(AppController* ac, SharedRes* sr) : SectionPaneImplBase(ac, sr), licenseNoticePopup(sr)
		{
		}
		void create(LvKeyboard kb, LvObj parent)
		{
			SectionPaneImplBase::create(kb, parent, "Technical Conformity of this hardware");
			// conformity certification marks
			LvLabel conflabel = LvLabel::create(ptr);
			conflabel.setTextStatic("This board contains the ESP32-C6-MINI-1 wireless module.");
			LvImage confimage = LvImage::create(ptr);
			{
				strutil::Str<128> path;
				BoardFs::resolveLvglAssetFilePath("telec_techconf.png", path.getBuffer(), path.capacity());
				confimage.setSrc(path);
			}
			// app version
			LvLabel vertitlelabel = LvLabel::create(ptr);
			vertitlelabel.setStyleTextFont(sharedRes->fontLatin1Bold, LV_PART_MAIN);
			vertitlelabel.setTextStatic("About this software");
			LvLabel verlabel = LvLabel::create(ptr);
			verlabel.setStyleRadius(Metrics::CornerRadius, LV_PART_MAIN);
			verlabel.setStyleBorderWidth(1, LV_PART_MAIN);
			verlabel.setStylePadAll(Metrics::Padding, LV_PART_MAIN);
			verlabel.setLongMode(LV_LABEL_LONG_MODE_WRAP);
			verlabel.setWidth(LV_PCT(100));
			verlabel.setStyleBorderColor(sharedRes->colorSettingsThinText, LV_PART_MAIN);
			{
				strutil::Str<128> path;
				BoardFs::resolveAssetFilePath("app_version.txt", path.getBuffer(), path.capacity());
				BoardFs::loadStringFromFile(path, &versionBuffer);
				HeapBufferT<char> buf;
				strreplace(versionBuffer.data(), "{PROJECT_NAME}", platform::getAppName(), &buf);
				strreplace(buf.data(), "{PROJECT_VER}", platform::getAppVersion(), &versionBuffer);
			}
			verlabel.setTextStatic(versionBuffer.data());
			// license notice
			LvLabel lictitlelabel = LvLabel::create(ptr);
			lictitlelabel.setStyleTextFont(sharedRes->fontLatin1Bold, LV_PART_MAIN);
			lictitlelabel.setTextStatic("Thirdrd Party Software License Notice");
			LvLabel licdesclabel = LvLabel::create(ptr);
			licdesclabel.setTextStatic("This software includes the following licensed components:");
			licPopupButton = LvButton::createStaticTextButton(ptr, "Show License Notice");
			licPopupButton.setSize(Metrics::FieldValueWidth, Metrics::FieldRowHeight);
			licPopupButton.addTypedEventCb(this, LV_EVENT_CLICKED);
			licenseNoticePopup.create(getScreen());
		}
		// LvObj typed event callback
		void onLvEventCallback(lv_event_t* e)
		{
			LvEvent ev(e);
			if(ev.testTargetAndCode(licPopupButton, LV_EVENT_CLICKED)) { licenseNoticePopup.present(); }
		}
	};

	// ================================================================================
	// container pane

	struct ContainerPane : public LvObj
	{
		AppController* appController;
		SharedRes* sharedRes;
		LvObj scrollContainer;
		LvKeyboard keyboard;
		WifiSectionPane wifiSectionPane;
		SystemTimeSectionPane systemTimeSectionPane;
		WeatherServiceSectionPane weatherServiceSectionPane;
		HistoryServerSectionPane historyServerSectionPane;
		SensorAlertSectionPane sensorAlertSectionPane;
		AboutSectionPane aboutSectionPane;
		ContainerPane(AppController* ac, SharedRes* sr)
			: appController(ac)
			, sharedRes(sr)
			, wifiSectionPane(ac, sr)
			, systemTimeSectionPane(ac, sr)
			, weatherServiceSectionPane(ac, sr)
			, historyServerSectionPane(ac, sr)
			, sensorAlertSectionPane(ac, sr)
			, aboutSectionPane(ac, sr)
		{
		}
		void create(lv_obj_t* parent)
		{
			LV_ASSERT(!ptr);
			ptr = LvObj::create(parent);
			setStyleRadius(0, LV_PART_MAIN);
			setStyleBorderWidth(0, LV_PART_MAIN);
			setStylePadAll(Metrics::Margin, LV_PART_MAIN);
			setStylePadTop(0, LV_PART_MAIN);
			setStylePadColRow(Metrics::Margin, Metrics::Margin, LV_PART_MAIN);
			setStyleBgColor(sharedRes->colorScreenBackground, LV_PART_MAIN);
			setFlexFlow(LV_FLEX_FLOW_COLUMN);
			// container
			scrollContainer = LvObj::create(ptr);
			scrollContainer.setFlexFlow(LV_FLEX_FLOW_COLUMN);
			scrollContainer.setStyleBgColor(sharedRes->colorScreenBackground, LV_PART_MAIN);
			scrollContainer.setStyleBorderWidth(0, LV_PART_MAIN);
			scrollContainer.setStylePadAll(0, LV_PART_MAIN);
			scrollContainer.setStylePadColRow(0, Metrics::Padding, LV_PART_MAIN);
			scrollContainer.setSize(LV_PCT(100), LV_SIZE_CONTENT);
			scrollContainer.setFlexGrow(1);
			// keyboard
			keyboard = LvKeyboard::create(ptr);
			keyboard.setHidden(true);
			keyboard.addTypedEventCb(this, LV_EVENT_CANCEL);
			// contents
			wifiSectionPane.create(keyboard, scrollContainer);
			wifiSectionPane.setSize(LV_PCT(100), LV_SIZE_CONTENT);
			systemTimeSectionPane.create(keyboard, scrollContainer);
			systemTimeSectionPane.setSize(LV_PCT(100), LV_SIZE_CONTENT);
			weatherServiceSectionPane.create(keyboard, scrollContainer);
			weatherServiceSectionPane.setSize(LV_PCT(100), LV_SIZE_CONTENT);
			historyServerSectionPane.create(keyboard, scrollContainer);
			historyServerSectionPane.setSize(LV_PCT(100), LV_SIZE_CONTENT);
			sensorAlertSectionPane.create(keyboard, scrollContainer);
			sensorAlertSectionPane.setSize(LV_PCT(100), LV_SIZE_CONTENT);
			aboutSectionPane.create(keyboard, scrollContainer);
			aboutSectionPane.setSize(LV_PCT(100), LV_SIZE_CONTENT);
		}
		// LvObj typed event callback
		void onLvEventCallback(lv_event_t* e)
		{
			LvEvent ev(e);
			if(ev.testTargetAndCode(keyboard, LV_EVENT_CANCEL))
			{
				if(LvObj focus = keyboard.getTextarea()) { focus.setState(LV_STATE_FOCUSED, false); }
				keyboard.setTextarea(nullptr);
				keyboard.setHidden(true);
				int32_t hkb = keyboard.getHeight();
				scrollContainer.scrollByBounded(0, hkb, LV_ANIM_OFF);
			}
		}
	};

} // namespace gui::settings
