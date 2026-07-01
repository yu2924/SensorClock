//
//  LvCheckbox.h
//
//  created by yu2924 on 2025-11-17
//  This file was generated mostly by GitHub Copilot and partially edited manually.
//

#pragma once

#include "LvObj.h"

#if defined(__cplusplus)

struct LvCheckbox : public LvObj
{
	constexpr LvCheckbox() noexcept = default;
	constexpr LvCheckbox(lv_obj_t* p) noexcept : LvObj(p) {}
	constexpr LvCheckbox(std::nullptr_t) noexcept : LvObj(nullptr) {}
	static LvCheckbox create(lv_obj_t* parent) noexcept { return LvCheckbox(lv_checkbox_create(parent)); }
	// setters
	void setText(const char* txt) noexcept { lv_checkbox_set_text(ptr, txt); }
	// getters
	const char* getText() const noexcept { return lv_checkbox_get_text(ptr); }
	// extensions
	bool isChecked() const noexcept { return hasState(LV_STATE_CHECKED); }
	void setCheck(bool v) noexcept { setState(LV_STATE_CHECKED, v); }
};

#endif // defined(__cplusplus)
