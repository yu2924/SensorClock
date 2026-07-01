//
//  LvButton.h
//
//  created by yu2924 on 2025-11-16
//  This file was generated mostly by GitHub Copilot and partially edited manually.
//

#pragma once

#include "LvObj.h"

#if defined(__cplusplus)

struct LvButton : public LvObj
{
	constexpr LvButton() noexcept = default;
	constexpr LvButton(lv_obj_t* p) noexcept : LvObj(p) {}
	constexpr LvButton(std::nullptr_t) noexcept : LvObj(nullptr) {}
	static LvButton create(lv_obj_t* parent) noexcept { return LvButton(lv_button_create(parent)); }
	// extensions
	static LvButton createTextButton(lv_obj_t* parent, const char* text)
	{
		LvButton btn = LvButton::create(parent);
		lv_obj_t* lbl = lv_label_create(btn);
		lv_label_set_text(lbl, text);
		lv_obj_set_align(lbl, LV_ALIGN_CENTER);
		return btn;
	}
	static LvButton createStaticTextButton(lv_obj_t* parent, const char* text)
	{
		LvButton btn = LvButton::create(parent);
		lv_obj_t* lbl = lv_label_create(btn);
		lv_label_set_text_static(lbl, text);
		lv_obj_set_align(lbl, LV_ALIGN_CENTER);
		return btn;
	}
};

#endif // defined(__cplusplus)
