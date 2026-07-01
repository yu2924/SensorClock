//
//  Common.h
//  SensorClock
//
//  created by yu2924 on 2025-11-21
//

#pragma once

#if !LV_USE_LODEPNG
#error requires LV_USE_LODEPNG=1 for the LodePng decoder
#endif
#if !LV_USE_TINY_TTF || !LV_TINY_TTF_FILE_SUPPORT
#error requires LV_USE_TINY_TTF=1 && LV_TINY_TTF_FILE_SUPPORT=1 for TTF files
#endif

#if SCREENRES_800x480
#define SCREENRES_1024x600	0
static constexpr int32_t GuiScreenWidth = 800;
static constexpr int32_t GuiScreenHeight = 480;
extern "C"
{
	LV_FONT_DECLARE(lv_font_montserrat_latin1x_12)
	LV_FONT_DECLARE(lv_font_montserrat_semibold_latin1x_12)
}
#else
#define SCREENRES_800x480	0
#define SCREENRES_1024x600 1
static constexpr int32_t GuiScreenWidth = 1024;
static constexpr int32_t GuiScreenHeight = 600;
extern "C"
{
	LV_FONT_DECLARE(lv_font_montserrat_latin1x_14)
	LV_FONT_DECLARE(lv_font_montserrat_semibold_latin1x_14)
}
#endif

namespace gui
{

	static constexpr int32_t WeatherIconResourceSize = 72;

	namespace Metrics
	{
#if SCREENRES_800x480
		static constexpr int32_t WeatherIconScaleNum = 8;
		static constexpr int32_t WeatherIconScaleDen = 10;
		static constexpr int32_t Margin = 3;
		static constexpr int32_t Padding = 6;
		static constexpr int32_t CornerRadius = 3;
		static constexpr int32_t HeaderHeight = 20;
		static constexpr int32_t BarGraphThickness = 9;
		static constexpr int32_t FieldRowHeight = 28;
		static constexpr int32_t FieldLabelWidth = 112;
		static constexpr int32_t FieldValueWidth = 224;
		static constexpr int32_t Font1Height = 128;
		static constexpr int32_t Font2Height = 51;
		static constexpr int32_t Font3Height = 28;
		static constexpr int32_t Font4Height = 14;
		static constexpr int32_t Font5Height = 12;
#elif SCREENRES_1024x600
		static constexpr int32_t WeatherIconScaleNum = 1;
		static constexpr int32_t WeatherIconScaleDen = 1;
		static constexpr int32_t Margin = 4;
		static constexpr int32_t Padding = 8;
		static constexpr int32_t CornerRadius = 4;
		static constexpr int32_t HeaderHeight = 24;
		static constexpr int32_t BarGraphThickness = 12;
		static constexpr int32_t FieldRowHeight = 36;
		static constexpr int32_t FieldLabelWidth = 144;
		static constexpr int32_t FieldValueWidth = 280;
		static constexpr int32_t Font1Height = 160;
		static constexpr int32_t Font2Height = 64;
		static constexpr int32_t Font3Height = 36;
		static constexpr int32_t Font4Height = 18;
		static constexpr int32_t Font5Height = 14;
#endif
	}

	struct SharedRes
	{
		static lv_font_t* loadTtfFromFile(int32_t fontheight) noexcept
		{
			strutil::Str<128> path;
			BoardFs::resolveLvglAssetFilePath("Montserrat-Medium.ttf", path.getBuffer(), path.capacity());
			lv_font_t* font = lv_tiny_ttf_create_file(path, fontheight);
			if(!font) { LV_LOG_WARN("loading font failed \"%s\"", (const char*)path); }
			return font;
		}
		struct SafeTtfPtr
		{
			lv_font_t* font = nullptr;
			SafeTtfPtr() = delete;
			SafeTtfPtr(lv_font_t* f) : font(f) {}
			~SafeTtfPtr() { if(font) { lv_tiny_ttf_destroy(font); } }
			operator const lv_font_t* () const { return font ? font : LV_FONT_DEFAULT; }
		};
		struct SafeFontPtr
		{
			const lv_font_t* font = nullptr;
			SafeFontPtr() = delete;
			SafeFontPtr(const lv_font_t* f) : font(f) {}
			operator const lv_font_t* () const { return font ? font : LV_FONT_DEFAULT; }
		};
#if SCREENRES_800x480
		const SafeTtfPtr font1{ loadTtfFromFile(Metrics::Font1Height) };
		const SafeTtfPtr font2{ loadTtfFromFile(Metrics::Font2Height) };
		const SafeFontPtr font3{ &lv_font_montserrat_28 };
		const SafeFontPtr font4{ &lv_font_montserrat_14 };
		const SafeFontPtr font5{ &lv_font_montserrat_latin1x_12 };
		const SafeFontPtr fontLatin1{ &lv_font_montserrat_latin1x_12 };
		const SafeFontPtr fontLatin1Bold{ &lv_font_montserrat_semibold_latin1x_12 };
#elif SCREENRES_1024x600
		const SafeTtfPtr	font1{ loadTtfFromFile(Metrics::Font1Height) };
		const SafeTtfPtr	font2{ loadTtfFromFile(Metrics::Font2Height) };
		const SafeFontPtr	font3{ &lv_font_montserrat_36 };
		const SafeFontPtr	font4{ &lv_font_montserrat_18 };
		const SafeFontPtr	font5{ &lv_font_montserrat_latin1x_14 };
		const SafeFontPtr	fontLatin1{ &lv_font_montserrat_latin1x_14 };
		const SafeFontPtr	fontLatin1Bold{ &lv_font_montserrat_semibold_latin1x_14 };
#endif
		const lv_color_t colorScreenBackground				= LV_COLOR_MAKE(32, 32, 32);
		const lv_color_t colorHeaderRegularText				= LV_COLOR_MAKE(160, 160, 160);
		const lv_color_t colorHeaderLightText				= LV_COLOR_MAKE(64, 128, 255);
		const lv_color_t colorDashboardSectionBackground	= LV_COLOR_MAKE(40, 44, 48);
		const lv_color_t colorDashboardRegularText			= LV_COLOR_MAKE(240, 255, 248);
		const lv_color_t colorDashboardThinText				= LV_COLOR_MAKE(144, 160, 156);
		const lv_color_t colorDashboardSaturdayText			= LV_COLOR_MAKE(160, 192, 240);
		const lv_color_t colorDashboardSundayText			= LV_COLOR_MAKE(240, 192, 160);
		const lv_color_t colorDashboardHourHand				= LV_COLOR_MAKE(240, 255, 248);
		const lv_color_t colorDashboardMinuteHand			= LV_COLOR_MAKE(144, 160, 156);
		const lv_color_t colorDashboardSecondHand			= LV_COLOR_MAKE(255, 128, 0);
		const lv_color_t colorDashboardSensorAlert			= LV_COLOR_MAKE(93, 32, 35);
		const lv_color_t colorSettingsSectionBackground		= LV_COLOR_MAKE(192, 192, 192);
		const lv_color_t colorSettingsTextAreaBgNormal		= LV_COLOR_MAKE(255, 255, 255);
		const lv_color_t colorSettingsTextAreaBgEdit		= LV_COLOR_MAKE(240, 255, 240);
		const lv_color_t colorSettingsRegularText			= LV_COLOR_MAKE(0, 0, 0);
		const lv_color_t colorSettingsThinText				= LV_COLOR_MAKE(128, 128, 128);
		const lv_event_code_t EventActivePageDidChange		= (lv_event_code_t)LvEvent::registerId();
	};

	struct ScopedLvLock
	{
		ScopedLvLock() { lv_lock(); }
		~ScopedLvLock() { lv_unlock(); }
	};

	void callOnMain(std::function<void()> proc)
	{
		Dispatcher::getMainDispatcher()->callAsync(proc);
	}

	void lvObjSetVisibility(LvObj& obj, bool show, bool anim) noexcept
	{
		static constexpr uint32_t MaxAnimObj = 8;
		static struct ObjFader
		{
			struct Entry { LvObj obj{}; bool show{}; };
			std::array<Entry, MaxAnimObj> buffer;
			size_t nextPos = 0;
			static void animExecProc(void* var, int32_t v) noexcept
			{
				Entry* ent = (Entry*)var;
				ent->obj.setStyleOpa(v, LV_PART_MAIN);
			}
			static void animCompletedProc(lv_anim_t* anim) noexcept
			{
				Entry* ent = (Entry*)anim->var;
				if(!ent->show) { ent->obj.setHidden(true); }
				*ent = {};
			}
			void startFade(LvObj& obj, int32_t opastart, int32_t opastop, uint32_t t, bool show) noexcept
			{
				Entry& ent = buffer[nextPos++];
				ent = { obj, show };
				if(buffer.size() <= nextPos) { nextPos = 0; }
				lv_anim_t anim;
				lv_anim_init(&anim);
				lv_anim_set_var(&anim, &ent);
				lv_anim_set_values(&anim, opastart, opastop);
				lv_anim_set_exec_cb(&anim, animExecProc);
				lv_anim_set_completed_cb(&anim, animCompletedProc);
				lv_anim_set_duration(&anim, t);
				lv_anim_start(&anim);
			}
		} sObjFader;
		if(obj.isHidden() == !show) { return; }
		if(anim)
		{
			if(show) obj.setHidden(false);
			sObjFader.startFade(obj, show ? LV_OPA_0 : LV_OPA_100, show ? LV_OPA_100 : LV_OPA_0, 100, show);
		}
		else
		{
			obj.setHidden(!show);
			obj.setStyleOpa(show ? LV_OPA_100 : LV_OPA_0, LV_PART_MAIN);
		}
	}

	void lvObjPresent(LvObj& obj, bool anim)
	{
		lvObjSetVisibility(obj, true, anim);
	}

	void lvObjDismiss(LvObj& obj, bool anim)
	{
		lvObjSetVisibility(obj, false, anim);
	}

} // namespace gui
