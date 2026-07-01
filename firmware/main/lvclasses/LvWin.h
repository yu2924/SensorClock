//
//  LvWin.h
//
//  created by yu2924 on 2025-11-22
//  This file was generated mostly by GitHub Copilot and partially edited manually.
//

#pragma once

#include "LvObj.h"

#if defined(__cplusplus)

struct LvWin : public LvObj
{
	constexpr LvWin() noexcept = default;
	constexpr LvWin(lv_obj_t* p) noexcept : LvObj(p) {}
	constexpr LvWin(std::nullptr_t) noexcept : LvObj(nullptr) {}
	static LvWin create(lv_obj_t* parent) noexcept { return LvWin(lv_win_create(parent)); }
	LvObj addTitle(const char* txt) noexcept { return lv_win_add_title(ptr, txt); }
	LvObj addButton(const void* icon, int32_t btn_w) noexcept {return lv_win_add_button(ptr, icon, btn_w); }
	LvObj getHeader() const noexcept { return lv_win_get_header(ptr); }
	LvObj getContent() const noexcept { return lv_win_get_content(ptr); }
};

#endif // defined(__cplusplus)
