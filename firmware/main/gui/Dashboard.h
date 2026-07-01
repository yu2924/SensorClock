//
//  Dashboard.h
//  SensorClock
//
//  created by yu2924 on 2025-11-21
//

#pragma once

// 
// the LV object layers contained in this implementation (Except for intermediate layers for layout purposes)
// 
// - ContainerPane
//     +- ClockSectionPane
//     |    +- AnalogClockPane
//     |    +- DigitalClockPane
//     +- WeatherSectionPane
//     |    +- WeatherCurrentPane
//     |    +- WeatherForecastRowPane[]
//     +- TemperatureSectionPane
//     +- HumiditySectionPane
//     +- AirPressureSectionPane
//     +- Co2LevelSectionPane
//     +- IaqSectionPane
// 

namespace gui::dashboard
{

	struct SectionPaneImplBase : public LvObj
	{
		static constexpr int32_t SecionMargin = 8;
		AppController* appController;
		SharedRes* sharedRes;
		LvLabel titleLabel;
		SectionPaneImplBase() = delete;
		SectionPaneImplBase(AppController* ac, SharedRes* sr) : appController(ac), sharedRes(sr)
		{
		}
		void create(LvObj parent, const char* static_title)
		{
			LV_ASSERT(!ptr);
			ptr = LvObj::create(parent);
			setStyleTextFont(sharedRes->fontLatin1, LV_PART_MAIN);
			setStyleRadius(Metrics::CornerRadius, LV_PART_MAIN);
			setStyleBgColor(sharedRes->colorDashboardSectionBackground, LV_PART_MAIN);
			setStyleBorderWidth(0, LV_PART_MAIN);
			setStylePadAll(Metrics::Padding, LV_PART_MAIN);
			setStylePadColRow(Metrics::Padding, Metrics::Padding, LV_PART_MAIN);
			setStyleTextColor(sharedRes->colorDashboardRegularText, LV_PART_MAIN);
			if(static_title)
			{
				titleLabel = LvLabel::create(ptr);
				titleLabel.setStyleTextColor(sharedRes->colorDashboardThinText, LV_PART_MAIN);
				titleLabel.setStyleBgColor(sharedRes->colorDashboardSectionBackground, LV_PART_MAIN);
				titleLabel.setStyleBgOpa(LV_OPA_COVER, LV_PART_MAIN);
				titleLabel.setStyleRadius(Metrics::CornerRadius, LV_PART_MAIN);
				int32_t mx = titleLabel.getStyleTextFont(LV_PART_MAIN)->line_height / 3;
				titleLabel.setStylePadHor(mx, LV_PART_MAIN);
				titleLabel.setStylePadVer(0, LV_PART_MAIN);
				titleLabel.setStyleTextFont(sharedRes->fontLatin1Bold, LV_PART_MAIN);
				titleLabel.setAlign(LV_ALIGN_TOP_LEFT);
				titleLabel.setTextStatic(static_title);
			}
		}
	};

	// ================================================================================
	// clock panes

	struct AnalogClockPane : public LvObj
	{
		SharedRes* sharedRes;
		LvScale scale;
		LvLine hourHand;
		LvLine minuteHand;
		LvLine secondHand;
		std::array<lv_point_precise_t, 2> minuteHandPoints{}, hourHandPoints{}, secondHandPoints{};
		LvLabel dateLabel;
		strutil::Str<32> dateBuf;
		int32_t clockRadius = 100;
		AnalogClockPane() = delete;
		AnalogClockPane(SharedRes* sr) : sharedRes(sr)
		{
		}
		void create(LvObj parent)
		{
			LV_ASSERT(!ptr);
			ptr = LvObj::create(parent);
			setFlag(LV_OBJ_FLAG_CLICKABLE, false);
			setStyleBgOpa(LV_OPA_0, LV_PART_MAIN);
			setStyleBorderWidth(0, LV_PART_MAIN);
			setStyleOutlineWidth(0, LV_PART_MAIN);
			setStylePadAll(0, LV_PART_MAIN);
			// scale
			scale = LvScale::create(ptr);
			scale.setFlag(LV_OBJ_FLAG_CLICKABLE, false);
			scale.setMode(LV_SCALE_MODE_ROUND_INNER);
			scale.setStyleBgOpa(LV_OPA_0, LV_PART_MAIN);
			scale.setStyleRadius(LV_RADIUS_CIRCLE, LV_PART_MAIN);
			scale.setStyleClipCorner(true, LV_PART_MAIN);
			scale.setLabelShow(true);
			scale.setTotalTickCount(61);
			scale.setMajorTickEvery(5);
			scale.setRange(0, 60);
			scale.setAngleRange(360);
			scale.setRotation(270);
			// major ticks
			static const char* HourTicks[] = { "12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", nullptr };
			scale.setTextSrc(HourTicks);
			scale.setStyleTextFont(sharedRes->font3, LV_PART_INDICATOR);
			scale.setStyleTextColor(sharedRes->colorDashboardRegularText, LV_PART_INDICATOR);
			scale.setStyleLineColor(sharedRes->colorDashboardHourHand, LV_PART_INDICATOR);
			scale.setStyleLength(6, LV_PART_INDICATOR);
			scale.setStyleLineWidth(2, LV_PART_INDICATOR);
			scale.setStylePadRadial(10, LV_PART_INDICATOR);
			scale.center();
			// minor ticks
			scale.setStyleLineColor(sharedRes->colorDashboardMinuteHand, LV_PART_ITEMS);
			scale.setStyleLength(6, LV_PART_ITEMS);;
			scale.setStyleLineWidth(2, LV_PART_ITEMS);
			// outline
			scale.setStyleArcColor(LV_COLOR_MAKE(0, 0, 0), LV_PART_MAIN);
			scale.setStyleArcWidth(1, LV_PART_MAIN);
			// hour hand
			hourHand = LvLine::create(scale);
			hourHand.setPointsMutable(hourHandPoints.data(), (uint32_t)hourHandPoints.size());
			hourHand.setStyleLineWidth(6, LV_PART_MAIN);
			hourHand.setStyleLineRounded(true, LV_PART_MAIN);
			hourHand.setStyleLineColor(sharedRes->colorDashboardHourHand, LV_PART_MAIN);
			// minute hand
			minuteHand = LvLine::create(scale);
			minuteHand.setPointsMutable(minuteHandPoints.data(), (uint32_t)minuteHandPoints.size());
			minuteHand.setStyleLineWidth(4, LV_PART_MAIN);
			minuteHand.setStyleLineRounded(true, LV_PART_MAIN);
			minuteHand.setStyleLineColor(sharedRes->colorDashboardMinuteHand, LV_PART_MAIN);
			// second hand
			secondHand = LvLine::create(scale);
			secondHand.setPointsMutable(secondHandPoints.data(), (uint32_t)secondHandPoints.size());
			secondHand.setStyleLineWidth(2, LV_PART_MAIN);
			secondHand.setStyleLineRounded(true, LV_PART_MAIN);
			secondHand.setStyleLineColor(sharedRes->colorDashboardSecondHand, LV_PART_MAIN);
			// date
			dateLabel = LvLabel::create(ptr);
			dateLabel.setFlag(LV_OBJ_FLAG_CLICKABLE, false);
			dateLabel.setStyleTextColor(sharedRes->colorDashboardRegularText, LV_PART_MAIN);
			dateLabel.setStyleTextFont(sharedRes->font3, LV_PART_MAIN);
			dateLabel.setStyleTextAlign(LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
			dateLabel.setLongMode(LV_LABEL_LONG_MODE_WRAP);
			dateLabel.setAlign(LV_ALIGN_BOTTOM_LEFT);
			// events
			addTypedEventCb(this, LV_EVENT_SIZE_CHANGED);
		}
		void setTmTime(const tm* ptm)
		{
			int hh = 0, mm = 0, ss = 0;
			if(ptm) { hh = ptm->tm_hour; mm = ptm->tm_min; ss = ptm->tm_sec; }
			scale.setLineNeedleValue(hourHand  , clockRadius * 6 / 10, (hh % 12) * 5 + (mm / 12));
			scale.setLineNeedleValue(minuteHand, clockRadius * 8 / 10, mm);
			scale.setLineNeedleValue(secondHand, clockRadius * 8 / 10, ss);
			if(ptm) { strftime(dateBuf.getBuffer(), dateBuf.capacity(), "%I:%M %p\n%Y-%m-%d\n%A", ptm); }
			else { dateBuf = ""; }
			dateLabel.setTextStatic(dateBuf);
		}
		// LvObj typed event callback
		void onLvEventCallback(lv_event_t* e)
		{
			LvEvent ev(e);
			if(ev.testTargetAndCode(ptr, LV_EVENT_SIZE_CHANGED))
			{
				LvArea rc = LvArea(getBounds()).increased(-1, -1);
				clockRadius = std::min(rc.getWidth(), rc.getHeight()) / 2;
				scale.setSize(clockRadius * 2, clockRadius * 2);
				// repaint
				time_t t = time(nullptr);
				tm stm{};
				const tm* ptm = platform::localtime(&t, &stm);
				setTmTime(ptm);
			}
		}
	};

	struct DigitalClockPane : public LvObj
	{
		struct MonoSpacedTimeLabel : public LvObj
		{
			struct Char
			{
				LvLabel label;
				char buf[2]{};
			};
			std::array<Char, 8> charList;
			void create(LvObj parent)
			{
				LV_ASSERT(!ptr);
				ptr = LvObj::create(parent);
				setFlexFlow(LV_FLEX_FLOW_ROW);
				setStyleBgOpa(LV_OPA_0, LV_PART_MAIN);
				setStyleBorderWidth(0, LV_PART_MAIN);
				setStylePadAll(0, LV_PART_MAIN);
				setStylePadColRow(0, 0, LV_PART_MAIN);
				for(size_t c = charList.size(), i = 0; i < c; ++i)
				{
					Char& ch = charList[i];
					ch.label = LvLabel::create(ptr);
					ch.label.setStyleTextAlign(LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
					if((i == 2) || (i == 5))	ch.label.setSize(LV_PCT(8), LV_SIZE_CONTENT);
					else						ch.label.setSize(LV_PCT(14), LV_SIZE_CONTENT);
				}
				setSize(LV_PCT(90), LV_SIZE_CONTENT);
			}
			void setTmTime(const tm* ptm)
			{
				strutil::Str<16> str;
				if(ptm) { str.format("%02d:%02d:%02d", ptm->tm_hour, ptm->tm_min, ptm->tm_sec); }
				else { str = "__:__:__"; }
				for(size_t c = std::min(charList.size(), str.length()), i = 0; i < c; ++i)
				{
					Char& ch = charList[i];
					ch.buf[0] = str[i];
					ch.label.setTextStatic(ch.buf);
				}
			}
		};
		static constexpr int32_t LineWidth = 2;
		SharedRes* sharedRes;
		MonoSpacedTimeLabel timeLabel;
		LvLabel dateLabel;
		strutil::Str<32> dateBuf;
		DigitalClockPane() = delete;
		DigitalClockPane(SharedRes* sr) : sharedRes(sr)
		{
		}
		void create(LvObj parent)
		{
			LV_ASSERT(!ptr);
			ptr = LvObj::create(parent);
			setFlag(LV_OBJ_FLAG_CLICKABLE, false);
			setStyleBgOpa(LV_OPA_0, LV_PART_MAIN);
			setStyleBorderWidth(0, LV_PART_MAIN);
			setStyleOutlineWidth(0, LV_PART_MAIN);
			setStylePadAll(0, LV_PART_MAIN);
			static const int32_t GridColDsc[] = { LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
			static const int32_t GridRowDsc[] = { LV_GRID_FR(1), Metrics::Font1Height, LineWidth, LV_GRID_FR(1), Metrics::Font2Height, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
			setGridDscArray(GridColDsc, GridRowDsc);
			// time
			timeLabel.create(ptr);
			timeLabel.setFlag(LV_OBJ_FLAG_CLICKABLE, false);
			timeLabel.setGridCell(LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);
			timeLabel.setStyleTextColor(sharedRes->colorDashboardRegularText, LV_PART_MAIN);
			timeLabel.setStyleTextFont(sharedRes->font1, LV_PART_MAIN);
			// underline
			LvLine line = LvLine::create(ptr);
			line.setGridCell(LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);
			line.setStyleLineWidth(LineWidth, LV_PART_MAIN);
			line.setStyleLineColor(sharedRes->colorDashboardThinText, LV_PART_MAIN);
			static const lv_point_precise_t ptlist[] = { { 0, 0 }, { LV_PCT(100), 0} };
			line.setPoints(ptlist, std::size(ptlist));
			line.setSize(LV_PCT(90), 1);
			// date
			dateLabel = LvLabel::create(ptr);
			dateLabel.setFlag(LV_OBJ_FLAG_CLICKABLE, false);
			dateLabel.setGridCell(LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 4, 1);
			dateLabel.setStyleTextColor(sharedRes->colorDashboardRegularText, LV_PART_MAIN);
			dateLabel.setStyleTextFont(sharedRes->font2, LV_PART_MAIN);
		}
		void setTmTime(const tm* ptm)
		{
			timeLabel.setTmTime(ptm);
			if(ptm) { strftime(dateBuf.getBuffer(), dateBuf.capacity(), "%Y-%m-%d %A", ptm); }
			else { dateBuf = ""; }
			dateLabel.setTextStatic(dateBuf);
		}
	};

	struct ClockSectionPane : public SectionPaneImplBase
	{
		DigitalClockPane digitalClockPane;
		AnalogClockPane analogClockPane;
		lv_timer_t* timer = nullptr;
		time_t lastTime = 0;
		bool analogStyle = false;
		ClockSectionPane() = delete;
		ClockSectionPane(AppController* ac, SharedRes* sr)
			: SectionPaneImplBase(ac, sr)
			, digitalClockPane(sr)
			, analogClockPane(sr)
		{
		}
		void create(LvObj parent)
		{
			SectionPaneImplBase::create(parent, nullptr);
			digitalClockPane.create(ptr);
			digitalClockPane.setSize(LV_PCT(100), LV_PCT(100));
			analogClockPane.create(ptr);
			analogClockPane.setSize(LV_PCT(100), LV_PCT(100));
			setAnalogStyle(appController->getPropertiesStore()->getBoolValue("analogclk", true), false);
			// event
			addTypedEventCb(this, LV_EVENT_CLICKED);
			addTypedEventCb(this, LV_EVENT_DELETE);
			timer = lv_timer_create(timerHandler, 250, this);
			setTime(time(nullptr));
		}
		void setAnalogStyle(bool ana, bool save)
		{
			analogStyle = ana;
			digitalClockPane.setHidden(analogStyle);
			analogClockPane.setHidden(!analogStyle);
			if(save) { appController->getPropertiesStore()->setBoolValue("analogclk", analogStyle); }
		}
		void setTime(time_t t)
		{
			lastTime = t;
			tm stm{};
			const tm* ptm = platform::localtime(&t, &stm);
			digitalClockPane.setTmTime(ptm);
			analogClockPane.setTmTime(ptm);
		}
		// LvObj typed event callback
		void onLvEventCallback(lv_event_t* e)
		{
			LvEvent ev(e);
			if     (ev.testTargetAndCode(ptr, LV_EVENT_CLICKED	)) { setAnalogStyle(!analogStyle, true); }
			else if(ev.testTargetAndCode(ptr, LV_EVENT_DELETE	)) { lv_timer_delete(timer); timer = nullptr; }
		}
		// lv_timer callback
		static void timerHandler(lv_timer_t* pt)
		{
			if(ClockSectionPane* p = static_cast<ClockSectionPane*>(lv_timer_get_user_data(pt))) { p->onTimeDidElapsed(pt); }
		}
		void onTimeDidElapsed(lv_timer_t*)
		{
			time_t tcur = time(nullptr);
			if(lastTime != tcur) { setTime(tcur); }
		}
	};

	// ================================================================================
	// weather service panes

	struct WeatherCurrentPane : public LvObj
	{
		SharedRes* sharedRes;
		LvImage iconImage;
		LvLabel tempValueLabel, tempUnitLabel;
		strutil::Str<16> tempBuf;
		struct Fields { LvLabel name, value, unit; strutil::Str<16> buf; } fieldList[3];
		WeatherCurrentPane() = delete;
		WeatherCurrentPane(SharedRes* sr) : sharedRes(sr)
		{
		}
		void create(LvObj parent)
		{
			LV_ASSERT(!ptr);
			ptr = LvObj::create(parent);
			setStyleBorderWidth(0, LV_PART_MAIN);
			setStyleBgOpa(LV_OPA_0, LV_PART_MAIN);
			setStylePadAll(0, LV_PART_MAIN);
			setStylePadColRow(0, 0, LV_PART_MAIN);
			setSize(LV_PCT(100), LV_SIZE_CONTENT);
			setFlexFlow(LV_FLEX_FLOW_COLUMN);
			// main grid
			LvObj maingrid = LvObj::create(ptr);
			maingrid.setStyleBorderWidth(0, LV_PART_MAIN);
			maingrid.setStyleBgOpa(LV_OPA_0, LV_PART_MAIN);
			maingrid.setStylePadAll(0, LV_PART_MAIN);
			maingrid.setStylePadColRow(0, 0, LV_PART_MAIN);
			maingrid.setSize(LV_PCT(100), LV_SIZE_CONTENT);
			static constexpr int32_t IconSize = (WeatherIconResourceSize * 6) / 8;
			static const int32_t HeadGridColDesc[] = { LV_GRID_FR(2), LV_GRID_FR(2), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
			static const int32_t HeadGridRowDesc[] = { IconSize, LV_GRID_TEMPLATE_LAST };
			maingrid.setGridDscArray(HeadGridColDesc, HeadGridRowDesc);
			const lv_font_t* fontv = sharedRes->font3;
			const lv_font_t* fontu = sharedRes->font4;
			int32_t hv = fontv ? (fontv->line_height - fontv->base_line) : 0;
			int32_t hu = fontu ? (fontu->line_height - fontu->base_line) : 0;
			iconImage = LvImage::create(maingrid);
			iconImage.setGridCell(LV_GRID_ALIGN_END, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);
			iconImage.setSize(IconSize, IconSize);
			iconImage.setScale((LV_SCALE_NONE * 6 * Metrics::WeatherIconScaleNum) / (8 * Metrics::WeatherIconScaleDen));
			tempValueLabel = LvLabel::create(maingrid);
			tempValueLabel.setGridCell(LV_GRID_ALIGN_END, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);
			tempValueLabel.setStyleTextColor(sharedRes->colorDashboardRegularText, LV_PART_MAIN);
			tempValueLabel.setStyleTextFont(fontv, LV_PART_MAIN);
			tempUnitLabel = LvLabel::create(maingrid);
			tempUnitLabel.setGridCell(LV_GRID_ALIGN_START, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);
			tempUnitLabel.setStyleTextColor(sharedRes->colorDashboardRegularText, LV_PART_MAIN);
			tempUnitLabel.setStyleTextFont(fontu, LV_PART_MAIN);
			tempUnitLabel.setStylePadTop(hv - hu, LV_PART_MAIN); // align with the baseline
			tempUnitLabel.setTextStatic("\xc2\xb0""C");
			// detail grid
			LvObj detailgrid = LvObj::create(ptr);
			detailgrid.setStyleBorderWidth(0, LV_PART_MAIN);
			detailgrid.setStyleBgOpa(LV_OPA_0, LV_PART_MAIN);
			detailgrid.setStylePadAll(0, LV_PART_MAIN);
			detailgrid.setStylePadColRow(4, 0, LV_PART_MAIN);
			detailgrid.setSize(LV_PCT(100), LV_SIZE_CONTENT);
			static constexpr int32_t RowHeight = (Metrics::Font5Height * 96) / 72;
			static const int32_t detailGridColDesc[] = { LV_GRID_FR(3), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
			static const int32_t detailGridRowDesc[] = { RowHeight, RowHeight, RowHeight, LV_GRID_TEMPLATE_LAST };
			detailgrid.setGridDscArray(detailGridColDesc, detailGridRowDesc);
			static const struct { const char* name; const char* unit; } TextLabels[3] = { { "Air Pressure", "hPa" }, { "Humidity", "%" }, { "Clouds","%" } };
			for(int c = (int)std::size(fieldList), i = 0; i < c; ++i)
			{
				fieldList[i].name = LvLabel::create(detailgrid);
				fieldList[i].name.setGridCell(LV_GRID_ALIGN_END, 0, 1, LV_GRID_ALIGN_CENTER, i, 1);
				fieldList[i].name.setStyleTextColor(sharedRes->colorDashboardRegularText, LV_PART_MAIN);
				fieldList[i].name.setTextStatic(TextLabels[i].name);
				fieldList[i].value = LvLabel::create(detailgrid);
				fieldList[i].value.setGridCell(LV_GRID_ALIGN_END, 1, 1, LV_GRID_ALIGN_CENTER, i, 1);
				fieldList[i].value.setStyleTextColor(sharedRes->colorDashboardRegularText, LV_PART_MAIN);
				fieldList[i].unit = LvLabel::create(detailgrid);
				fieldList[i].unit.setGridCell(LV_GRID_ALIGN_START, 2, 1, LV_GRID_ALIGN_CENTER, i, 1);
				fieldList[i].unit.setStyleTextColor(sharedRes->colorDashboardRegularText, LV_PART_MAIN);
				fieldList[i].unit.setTextStatic(TextLabels[i].unit);
			}
			setWeatherCurrentStatus(nullptr);
		}
		void setWeatherCurrentStatus(const WeatherCurrentStatus* stat)
		{
			if(stat && !stat->weatherIcon.isEmpty())
			{
				strutil::Str<16> fn; fn << stat->weatherIcon << "@2x.png";
				strutil::Str<128> path;
				BoardFs::resolveLvglAssetFilePath(fn, path.getBuffer(), path.capacity());
				iconImage.setSrc(path);
			}
			else
			{
				iconImage.setSrc(nullptr);
			}
			if(stat)
			{
				tempBuf.format("%.1f", stat->tempC);
				tempValueLabel.setTextStatic(tempBuf);
				fieldList[0].buf.format("%d", stat->pressureHpa);
				fieldList[1].buf.format("%d", stat->humidityPct);
				fieldList[2].buf.format("%d", stat->cloudsPct);
				for(auto&& f : fieldList) { f.value.setTextStatic(f.buf); }
			}
			else
			{
				tempValueLabel.setTextStatic("--");
				for(auto&& f : fieldList) { f.value.setTextStatic("--"); }
			}
		}
	};

	struct WeatherForecastRowPane : public LvObj
	{
		SharedRes* sharedRes = nullptr;
		LvImage iconImage;
		LvLabel dateLabel, wdayLabel, tempLabel;
		strutil::Str<16> dateBuf, wdayBuf, tempBuf;
		WeatherForecastRowPane() = delete;
		WeatherForecastRowPane(SharedRes* sr) : sharedRes(sr)
		{
		}
		void create(LvObj parent)
		{
			LV_ASSERT(!ptr);
			ptr = LvObj::create(parent);
			setFlexFlow(LV_FLEX_FLOW_ROW);
			setStyleBorderWidth(0, LV_PART_MAIN);
			setStyleBgOpa(LV_OPA_0, LV_PART_MAIN);
			setStylePadHor(Metrics::Padding / 2, LV_PART_MAIN);
			setStylePadVer(0, LV_PART_MAIN);
			setStylePadColRow(0, 0, LV_PART_MAIN);
			setSize(LV_PCT(100), LV_SIZE_CONTENT);
			int32_t rowheight = getStyleTextFont(LV_PART_MAIN)->line_height;
			iconImage = LvImage::create(ptr);
			iconImage.setSize(rowheight, rowheight);
			iconImage.setScale((LV_SCALE_NONE * rowheight * Metrics::WeatherIconScaleNum) / (WeatherIconResourceSize * Metrics::WeatherIconScaleDen));
			dateLabel = LvLabel::create(ptr);
			wdayLabel = LvLabel::create(ptr);
			tempLabel = LvLabel::create(ptr);
			dateLabel.setStyleFlexGrow(1, LV_PART_MAIN);
			wdayLabel.setStyleFlexGrow(1, LV_PART_MAIN);
			tempLabel.setStyleFlexGrow(1, LV_PART_MAIN);
			dateLabel.setStyleTextAlign(LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
			wdayLabel.setStyleTextAlign(LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
			tempLabel.setStyleTextAlign(LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
			dateLabel.setStyleTextColor(sharedRes->colorDashboardRegularText, LV_PART_MAIN);
			wdayLabel.setStyleTextColor(sharedRes->colorDashboardRegularText, LV_PART_MAIN);
			tempLabel.setStyleTextColor(sharedRes->colorDashboardRegularText, LV_PART_MAIN);
			setWeatherDailyStatus(nullptr);
		}
		void setWeatherDailyStatus(const WeatherDailyStatus* stat)
		{
			if(stat && !stat->weatherIcon.isEmpty())
			{
				strutil::Str<16> fn; fn << stat->weatherIcon << "@2x.png";
				strutil::Str<128> path;
				BoardFs::resolveLvglAssetFilePath(fn, path.getBuffer(), path.capacity());
				iconImage.setSrc(path);
			}
			else
			{
				iconImage.setSrc(nullptr);
			}
			tm stm{};
			const tm* ptm = stat ? platform::localtime(&stat->dt, &stm) : nullptr;
			if(ptm)
			{
				strftime(dateBuf.getBuffer(), dateBuf.capacity(), "%m-%d", ptm); dateLabel.setTextStatic(dateBuf);
				strftime(wdayBuf.getBuffer(), wdayBuf.capacity(), "%a", ptm); wdayLabel.setTextStatic(wdayBuf);
				lv_color_t clr = sharedRes->colorDashboardRegularText;
				if     (ptm->tm_wday == 0) clr = sharedRes->colorDashboardSundayText;
				else if(ptm->tm_wday == 6) clr = sharedRes->colorDashboardSaturdayText;
				dateLabel.setStyleTextColor(clr, LV_PART_MAIN);
				wdayLabel.setStyleTextColor(clr, LV_PART_MAIN);
				tempBuf.format("%.1f""\xc2\xb0""C", stat->tempC); tempLabel.setTextStatic(tempBuf);
			}
			else
			{
				dateLabel.setTextStatic("--");
				wdayLabel.setTextStatic("");
				tempLabel.setTextStatic("--");
			}
		}
	};

	struct WeatherSectionPane : public SectionPaneImplBase, private WeatherTask::Listener
	{
		WeatherCurrentPane currentPane;
		std::array<WeatherForecastRowPane, WeatherQuery::MaxForecastList> rowPaneList;
		lv_grad_dsc_t gradBgNight{}, gradBgCloudy{}, gradBgFine{};
		WeatherSectionPane() = delete;
		WeatherSectionPane(AppController* ac, SharedRes* sr)
			: SectionPaneImplBase(ac, sr)
			, currentPane(sr)
			, rowPaneList
			{
				WeatherForecastRowPane(sr),
				WeatherForecastRowPane(sr),
				WeatherForecastRowPane(sr),
				WeatherForecastRowPane(sr),
				WeatherForecastRowPane(sr),
				WeatherForecastRowPane(sr),
				WeatherForecastRowPane(sr),
				WeatherForecastRowPane(sr)
			}
		{
			appController->getWeatherTask()->addListener(this);
		}
		~WeatherSectionPane() override
		{
			appController->getWeatherTask()->removeListener(this);
		}
		void create(LvObj parent)
		{
			SectionPaneImplBase::create(parent, "Weather");
			setFlexFlow(LV_FLEX_FLOW_COLUMN);
			setStylePadAll(Metrics::Padding, LV_PART_MAIN);
			LvObj contentpane = LvObj::create(ptr);
			contentpane.setStyleBgOpa(LV_OPA_0, LV_PART_MAIN);
			contentpane.setStyleBorderWidth(0, LV_PART_MAIN);
			contentpane.setStyleOutlineWidth(0, LV_PART_MAIN);
			contentpane.setStylePadAll(0, LV_PART_MAIN);
			contentpane.setFlexGrow(1);
			contentpane.setFlexFlow(LV_FLEX_FLOW_COLUMN);
			contentpane.setWidth(LV_PCT(100));
			contentpane.setStyleFlexMainPlace(LV_FLEX_ALIGN_SPACE_EVENLY, LV_PART_MAIN);
			// current weather
			currentPane.create(contentpane);
			// weather forecast
			LvObj arraypane = LvObj::create(contentpane);
			arraypane.setStyleBgOpa(LV_OPA_0, LV_PART_MAIN);
			arraypane.setStyleBorderWidth(0, LV_PART_MAIN);
			arraypane.setStyleOutlineWidth(0, LV_PART_MAIN);
			arraypane.setStylePadAll(0, LV_PART_MAIN);
			arraypane.setStylePadColRow(0, 4, LV_PART_MAIN);
			arraypane.setFlexFlow(LV_FLEX_FLOW_COLUMN);
			arraypane.setSize(LV_PCT(100), LV_SIZE_CONTENT);
			createSeparator(arraypane);
			for(size_t c = rowPaneList.size(), i = 0; i < c; ++i)
			{
				rowPaneList[i].create(arraypane);
				createSeparator(arraypane);
			}
			// background default night
			lv_color_t clrbgn[] = { LV_COLOR_MAKE(0, 0, 0), sharedRes->colorDashboardSectionBackground };
			uint8_t stpbgn[] = { 96, 192 };
			lv_grad_init_stops(&gradBgNight, clrbgn, nullptr, stpbgn, 2);
			lv_grad_vertical_init(&gradBgNight);
			// background daytime cloudy
			lv_color_t clrbgc[] = { LV_COLOR_MAKE(182, 195, 207), sharedRes->colorDashboardSectionBackground };
			uint8_t stpbgc[] = { 96, 192 };
			lv_grad_init_stops(&gradBgCloudy, clrbgc, nullptr, stpbgc, 2);
			lv_grad_vertical_init(&gradBgCloudy);
			// background daytime fine
			lv_color_t clrbgf[] = { LV_COLOR_MAKE(99, 152, 197), sharedRes->colorDashboardSectionBackground };
			uint8_t stpngf[] = { 96, 192 };
			lv_grad_init_stops(&gradBgFine, clrbgf, nullptr, stpngf, 2);
			lv_grad_vertical_init(&gradBgFine);
		}
		LvLine createSeparator(LvObj parent)
		{
			LvLine line = LvLine::create(parent);
			line.setStyleLineWidth(1, LV_PART_MAIN);
			line.setStyleLineColor(sharedRes->colorDashboardThinText, LV_PART_MAIN);
			static const lv_point_precise_t ptlist[] = { { 0, 0 }, { LV_PCT(100), 0 } };
			line.setPoints(ptlist, std::size(ptlist));
			line.setSize(LV_PCT(100), 1);
			return line;
		}
		void setWeatherQueryResult(const WeatherQuery::Result& res)
		{
			if(res.apiStatus != 200) { return; }
			const lv_grad_dsc_t* grad = nullptr;
			if((res.current.dt <= res.current.sunrise) || (res.current.sunset <= res.current.dt)) { grad = &gradBgNight; }
			else if(90 <= res.current.cloudsPct) { grad = &gradBgCloudy; }
			else { grad = &gradBgFine; }
			setStyleBgGrad(grad, LV_PART_MAIN);
			currentPane.setWeatherCurrentStatus(&res.current);
			for(size_t c = rowPaneList.size(), i = 0; i < c; ++i)
			{
				rowPaneList[i].setWeatherDailyStatus((i < res.numforecasts) ? &res.arrForecasts[i] : nullptr);
			}
		}
		// WeatherTask::Listener
		void weatherTaskResultDidChange(WeatherTask*, const WeatherQuery::Result& v) override
		{
			ScopedLvLock sl;
			setWeatherQueryResult(v);
		}
	};

	// ================================================================================
	// sensor panes

	struct SensorSectionPaneImplBase : public SectionPaneImplBase, protected SensorTask::Listener
	{
		const int sensorIndex;
		const lv_color_t colorUnder{};
		const lv_color_t colorGood{};
		const lv_color_t colorOver{};
		LvLabel valueLabel, unitLabel;
		strutil::Str<8> valueBuf;
		LvBar valueBar;
		lv_grad_dsc_t barGrad{};
		SensorSectionPaneImplBase() = delete;
		SensorSectionPaneImplBase(AppController* ac, SharedRes* sr, int isensor, lv_color_t clrunder, lv_color_t clrgood, lv_color_t clrover)
			: SectionPaneImplBase(ac, sr)
			, sensorIndex(isensor)
			, colorUnder(clrunder)
			, colorGood(clrgood)
			, colorOver(clrover)
		{
			appController->getSensorTask()->addListener(this);
		}
		~SensorSectionPaneImplBase() override
		{
			appController->getSensorTask()->removeListener(this);
		}
		void create(LvObj parent)
		{
			SectionPaneImplBase::create(parent, SensorTask::MetadataList[sensorIndex].title);
			setFlexFlow(LV_FLEX_FLOW_COLUMN);
			LvObj contentpane = LvObj::create(ptr);
			contentpane.setStyleBgOpa(LV_OPA_0, LV_PART_MAIN);
			contentpane.setStyleBorderWidth(0, LV_PART_MAIN);
			contentpane.setStyleOutlineWidth(0, LV_PART_MAIN);
			contentpane.setStylePadAll(0, LV_PART_MAIN);
			contentpane.setFlexGrow(1);
			contentpane.setFlexFlow(LV_FLEX_FLOW_COLUMN);
			contentpane.setWidth(LV_PCT(100));
			contentpane.setStyleFlexMainPlace(LV_FLEX_ALIGN_SPACE_EVENLY, LV_PART_MAIN);
			// label
			LvObj hstack = LvObj::create(contentpane);
			hstack.setStyleBgOpa(LV_OPA_0, LV_PART_MAIN);
			hstack.setStyleBorderWidth(0, LV_PART_MAIN);
			hstack.setStyleOutlineWidth(0, LV_PART_MAIN);
			hstack.setStylePadAll(0, LV_PART_MAIN);
			hstack.setFlexFlow(LV_FLEX_FLOW_ROW);
			hstack.setFlexAlign(LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
			hstack.setSize(LV_PCT(100), LV_SIZE_CONTENT);
			const lv_font_t* fontv = sharedRes->font3;
			const lv_font_t* fontu = sharedRes->font4;
			int32_t hv = fontv ? (fontv->line_height - fontv->base_line) : 0;
			int32_t hu = fontu ? (fontu->line_height - fontu->base_line) : 0;
			valueLabel = LvLabel::create(hstack);
			valueLabel.setStyleTextAlign(LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
			valueLabel.setStyleTextColor(sharedRes->colorDashboardRegularText, LV_PART_MAIN);
			valueLabel.setStyleTextFont(fontv, LV_PART_MAIN);
			unitLabel = LvLabel::create(hstack);
			unitLabel.setStyleTextAlign(LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
			unitLabel.setStyleTextColor(sharedRes->colorDashboardRegularText, LV_PART_MAIN);
			unitLabel.setStyleTextFont(fontu, LV_PART_MAIN);
			unitLabel.setStylePadTop(hv - hu, LV_PART_MAIN); // align with the baseline
			unitLabel.setTextStatic(SensorTask::MetadataList[sensorIndex].unit);
			// bar
			valueBar = LvBar::create(contentpane);
			valueBar.setStyleRadius(0, LV_PART_MAIN);
			valueBar.setStyleRadius(0, LV_PART_INDICATOR);
			valueBar.setSize(LV_PCT(100), Metrics::BarGraphThickness);
			lv_grad_horizontal_init(&barGrad);
			updateBarColors();
			setValue(std::numeric_limits<float>::quiet_NaN());
		}
		void updateBarColors()
		{
			const auto& rng = SensorTask::Thresholds::Ranges.array[sensorIndex];
			const auto& th = appController->getSensorTask()->getThresholds().array[sensorIndex];
			if(SensorTask::MetadataList[sensorIndex].proportional)
			{
				// the entire range is visible
				valueBar.setRange((int32_t)rng.under, (int32_t)rng.over);
				uint8_t under = (uint8_t)std::max(1.0f, 255.0f * (th.under - rng.under) / (rng.over - rng.under));
				uint8_t over = (uint8_t)std::min(254.0f, 255.0f * (th.over - rng.under) / (rng.over - rng.under));
				lv_color_t colors[] = { colorUnder, colorGood, colorGood, colorOver };
				uint8_t stops[] = { 0, under, over, 255 };
				static_assert(std::size(colors) <= LV_GRADIENT_MAX_STOPS, "too many gradient stops");
				lv_grad_init_stops(&barGrad, colors, nullptr, stops, std::size(colors));
			}
			else
			{
				// only the comfortable range is visible
				valueBar.setRange((int32_t)th.under, (int32_t)th.over);
				lv_color_t colors[] = { colorUnder, colorGood, colorGood, colorOver };
				uint8_t stops[] = { 0, 64, 192, 255 };
				static_assert(std::size(colors) <= LV_GRADIENT_MAX_STOPS, "too many gradient stops");
				lv_grad_init_stops(&barGrad, colors, nullptr, stops, std::size(colors));
			}
			valueBar.setStyleBgGrad(&barGrad, LV_PART_INDICATOR);
		}
		void setAlert(bool v)
		{
			setStyleBgColor(v ? sharedRes->colorDashboardSensorAlert : sharedRes->colorDashboardSectionBackground, LV_PART_MAIN);
		}
		void setValue(float v)
		{
			if(std::isnan(v))
			{
				valueLabel.setTextStatic("--");
				valueBar.setValue(0, false);
			}
			else
			{
				formatValue(v, &valueBuf);
				valueLabel.setTextStatic(valueBuf);
				valueBar.setValue((int32_t)(v + 0.5f), false);
			}
		}
		virtual void formatValue(float v, strutil::Str<8>* buf) = 0;
		// SensorTask::Listener
		void sensorTaskThresholdsDidChange(SensorTask*, const SensorTask::Thresholds&) override
		{
			ScopedLvLock sl;
			updateBarColors();
		}
		void sensorTaskValuesDidChange(SensorTask* p, SensorTask::Fupdates updates, const SensorTask::Values& values) override
		{
			int m = 1 << sensorIndex;
			if(updates & m)
			{
				ScopedLvLock sl;
				setAlert(p->getAlertFlags() & m);
				setValue(values[sensorIndex]);
			}
		}
	};

	struct TemperatureSectionPane : public SensorSectionPaneImplBase
	{
		static constexpr lv_color_t ColorUnder = LV_COLOR_MAKE(61, 115, 178);
		static constexpr lv_color_t ColorGood = LV_COLOR_MAKE(0, 192, 0);
		static constexpr lv_color_t ColorOver = LV_COLOR_MAKE(255, 128, 0);
		TemperatureSectionPane() = delete;
		TemperatureSectionPane(AppController* ac, SharedRes* sr) : SensorSectionPaneImplBase(ac, sr, SensorTask::Ibme680Temp, ColorUnder, ColorGood, ColorOver)
		{
		}
		void formatValue(float v, strutil::Str<8>* buf) override
		{
			buf->format("%.1f", v);
		}
	};

	struct HumiditySectionPane : public SensorSectionPaneImplBase
	{
		static constexpr lv_color_t ColorUnder = LV_COLOR_MAKE(208, 170, 135);
		static constexpr lv_color_t ColorGood = LV_COLOR_MAKE(0, 192, 0);
		static constexpr lv_color_t ColorOver = LV_COLOR_MAKE(0, 224, 255);
		HumiditySectionPane() = delete;
		HumiditySectionPane(AppController* ac, SharedRes* sr) : SensorSectionPaneImplBase(ac, sr, SensorTask::Ibme680Hum, ColorUnder, ColorGood, ColorOver)
		{
		}
		void formatValue(float v, strutil::Str<8>* buf) override
		{
			buf->format("%d", (int)(v + 0.5f));
		}
	};

	struct AirPressureSectionPane : public SensorSectionPaneImplBase
	{
		static constexpr lv_color_t ColorUnder = LV_COLOR_MAKE(61, 115, 178);
		static constexpr lv_color_t ColorGood = LV_COLOR_MAKE(0, 192, 0);
		static constexpr lv_color_t ColorOver = LV_COLOR_MAKE(255, 128, 0);
		AirPressureSectionPane() = delete;
		AirPressureSectionPane(AppController* ac, SharedRes* sr) : SensorSectionPaneImplBase(ac, sr, SensorTask::Ibme680Pres, ColorUnder, ColorGood, ColorOver)
		{
		}
		void formatValue(float v, strutil::Str<8>* buf) override
		{
			buf->format("%d", (int)(v + 0.5f));
		}
	};

	struct Co2LevelSectionPane : public SensorSectionPaneImplBase
	{
		static constexpr lv_color_t ColorUnder = LV_COLOR_MAKE(0, 192, 0);
		static constexpr lv_color_t ColorGood = LV_COLOR_MAKE(224, 224, 0);
		static constexpr lv_color_t ColorOver = LV_COLOR_MAKE(255, 128, 0);
		Co2LevelSectionPane() = delete;
		Co2LevelSectionPane(AppController* ac, SharedRes* sr) : SensorSectionPaneImplBase(ac, sr, SensorTask::Iscd4xCo2, ColorUnder, ColorGood, ColorOver)
		{
		}
		void formatValue(float v, strutil::Str<8>* buf) override
		{
			buf->format("%d", (int)(v + 0.5f));
		}
	};

	struct IaqSectionPane : public SensorSectionPaneImplBase
	{
		static constexpr lv_color_t ColorUnder = LV_COLOR_MAKE(224, 224, 0);
		static constexpr lv_color_t ColorGood = LV_COLOR_MAKE(0, 192, 0);
		static constexpr lv_color_t ColorOver = LV_COLOR_MAKE(0, 192, 0);
		IaqSectionPane() = delete;
		IaqSectionPane(AppController* ac, SharedRes* sr) : SensorSectionPaneImplBase(ac, sr, SensorTask::Ibme680Iaq, ColorUnder, ColorGood, ColorOver)
		{
		}
		void formatValue(float v, strutil::Str<8>* buf) override
		{
			buf->format("%d", (int)(v + 0.5f));
		}
	};

	// ================================================================================
	// container pane

	struct ContainerPane : public LvObj
	{
		AppController* appController;
		SharedRes* sharedRes;
		ClockSectionPane clockSectionPane;
		WeatherSectionPane weatherSectionPane;
		TemperatureSectionPane temperatureSectionPane;
		HumiditySectionPane humiditySectionPane;
		AirPressureSectionPane airPressureSectionPane;
		Co2LevelSectionPane co2LevelSectionPane;
		IaqSectionPane iaqSectionPane;
		ContainerPane() = delete;
		ContainerPane(AppController* ac, SharedRes* sr)
			: appController(ac)
			, sharedRes(sr)
			, clockSectionPane(ac, sr)
			, weatherSectionPane(ac, sr)
			, temperatureSectionPane(ac, sr)
			, humiditySectionPane(ac, sr)
			, airPressureSectionPane(ac, sr)
			, co2LevelSectionPane(ac, sr)
			, iaqSectionPane(ac, sr)
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
			static const int32_t GridColDsc[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
			static const int32_t GridRowDsc[] = { LV_GRID_FR(4), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
			setGridDscArray(GridColDsc, GridRowDsc);
			clockSectionPane.create(ptr);
			weatherSectionPane.create(ptr);
			temperatureSectionPane.create(ptr);
			humiditySectionPane.create(ptr);
			airPressureSectionPane.create(ptr);
			co2LevelSectionPane.create(ptr);
			iaqSectionPane.create(ptr);
			clockSectionPane.setGridCell(LV_GRID_ALIGN_STRETCH, 0, 4, LV_GRID_ALIGN_STRETCH, 0, 1);
			weatherSectionPane.setGridCell(LV_GRID_ALIGN_STRETCH, 4, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
			temperatureSectionPane.setGridCell(LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
			humiditySectionPane.setGridCell(LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
			airPressureSectionPane.setGridCell(LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
			co2LevelSectionPane.setGridCell(LV_GRID_ALIGN_STRETCH, 3, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
			iaqSectionPane.setGridCell(LV_GRID_ALIGN_STRETCH, 4, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
		}

	};

} // namespace gui::dashboard
