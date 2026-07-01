//
//  LvSlider.h
//
//  created by yu2924 on 2025-11-17
//  This file was generated mostly by GitHub Copilot and partially edited manually.
//

#pragma once

#include "LvObj.h"

#if defined(__cplusplus)

struct LvSlider : public LvObj
{
	constexpr LvSlider() noexcept = default;
	constexpr LvSlider(lv_obj_t* p) noexcept : LvObj(p) {}
	constexpr LvSlider(std::nullptr_t) noexcept : LvObj(nullptr) {}
	static LvSlider create(lv_obj_t* parent) noexcept { return LvSlider(lv_slider_create(parent)); }
	// setters
	void setValue(int32_t value, lv_anim_enable_t anim) noexcept { lv_slider_set_value(ptr, value, anim); }
	void setStartValue(int32_t value, lv_anim_enable_t anim) noexcept { lv_slider_set_start_value(ptr, value, anim); }
	void setRange(int32_t min, int32_t max) noexcept { lv_slider_set_range(ptr, min, max); }
	void setMinValue(int32_t min) noexcept { lv_slider_set_min_value(ptr, min); }
	void setMaxValue(int32_t max) noexcept { lv_slider_set_max_value(ptr, max); }
	void setMode(lv_slider_mode_t mode) noexcept { lv_slider_set_mode(ptr, mode); }
	void setOrientation(lv_slider_orientation_t orientation) noexcept { lv_slider_set_orientation(ptr, orientation); }
	// getters
	int32_t getValue() const noexcept { return lv_slider_get_value(ptr); }
	int32_t getLeftValue() const noexcept { return lv_slider_get_left_value(ptr); }
	int32_t getMinValue() const noexcept { return lv_slider_get_min_value(ptr); }
	int32_t getMaxValue() const noexcept { return lv_slider_get_max_value(ptr); }
	bool isDragged() const noexcept { return lv_slider_is_dragged(ptr); }
	lv_slider_mode_t getMode() const noexcept { return lv_slider_get_mode(ptr); }
	lv_slider_orientation_t getOrientation() const noexcept { return lv_slider_get_orientation(ptr); }
	bool isSymmetrical() const noexcept { return lv_slider_is_symmetrical(ptr); }
#if LV_USE_OBSERVER
	// observer binding (wrap lv_slider_bind_value)
	lv_observer_t* bindValue(lv_subject_t* subject) noexcept { return lv_slider_bind_value(ptr, subject); }
#endif
};

#endif // defined(__cplusplus)
