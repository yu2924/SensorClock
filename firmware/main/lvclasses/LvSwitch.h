//
//  LvSwitch.h
//
//  created by yu2924 on 2025-11-17
//  This file was generated mostly by GitHub Copilot and partially edited manually.
//

#pragma once

#include "LvObj.h"

#if defined(__cplusplus)

struct LvSwitch : public LvObj
{
	constexpr LvSwitch() noexcept = default;
	constexpr LvSwitch(lv_obj_t* p) noexcept : LvObj(p) {}
	constexpr LvSwitch(std::nullptr_t) noexcept : LvObj(nullptr) {}
	static LvSwitch create(lv_obj_t* parent) noexcept { return LvSwitch(lv_switch_create(parent)); }
	// setters
	void setOrientation(lv_switch_orientation_t orientation) noexcept { lv_switch_set_orientation(ptr, orientation); }
	// getters
	lv_switch_orientation_t getOrientation() const noexcept { return lv_switch_get_orientation(ptr); }
	// extensions
	bool isChecked() const noexcept { return hasState(LV_STATE_CHECKED); }
	void setCheck(bool v) noexcept { setState(LV_STATE_CHECKED, v); }
};

#endif // defined(__cplusplus)
