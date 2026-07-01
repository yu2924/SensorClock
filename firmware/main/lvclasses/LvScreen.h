//
//  LvScreen.h
//
//  created by yu2924 on 2025-11-16
//  This file was generated mostly by GitHub Copilot and partially edited manually.
//

#pragma once

#include "LvObj.h"

#if defined(__cplusplus)

struct LvScreen : public LvObj
{
	constexpr LvScreen() noexcept = default;
	constexpr LvScreen(lv_obj_t* p) noexcept : LvObj(p) {}
	constexpr LvScreen(std::nullptr_t) noexcept : LvObj(nullptr) {}
	static LvScreen create() noexcept { return LvScreen(lv_obj_create(nullptr)); }
	void load() noexcept { lv_screen_load(ptr); }
	void loadAnim(lv_screen_load_anim_t anim_type, uint32_t time, uint32_t delay, bool auto_del) noexcept { lv_screen_load_anim(ptr, anim_type, time, delay, auto_del); }
};

#endif // defined(__cplusplus)
