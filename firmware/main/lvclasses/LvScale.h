//
//  LvScale.h
//
//  created by yu2924 on 2026-01-28
//  This file was generated mostly by GitHub Copilot and partially edited manually.
//

#pragma once

#include "LvObj.h"

#if defined(__cplusplus)

struct LvScale : public LvObj
{
	constexpr LvScale() noexcept = default;
	constexpr LvScale(lv_obj_t* p) noexcept : LvObj(p) {}
	constexpr LvScale(std::nullptr_t) noexcept : LvObj(nullptr) {}
	static LvScale create(lv_obj_t* parent) noexcept { return LvScale(lv_scale_create(parent)); }
	// Setter functions
	void setMode(lv_scale_mode_t mode) noexcept { lv_scale_set_mode(ptr, mode); }
	void setTotalTickCount(uint32_t total_tick_count) noexcept { lv_scale_set_total_tick_count(ptr, total_tick_count); }
	void setMajorTickEvery(uint32_t major_tick_every) noexcept { lv_scale_set_major_tick_every(ptr, major_tick_every); }
	void setLabelShow(bool show_label) noexcept { lv_scale_set_label_show(ptr, show_label); }
	void setRange(int32_t min, int32_t max) noexcept { lv_scale_set_range(ptr, min, max); }
	void setMinValue(int32_t min) noexcept { lv_scale_set_min_value(ptr, min); }
	void setMaxValue(int32_t max) noexcept { lv_scale_set_max_value(ptr, max); }
	void setAngleRange(uint32_t angle_range) noexcept { lv_scale_set_angle_range(ptr, angle_range); }
	void setRotation(int32_t rotation) noexcept { lv_scale_set_rotation(ptr, rotation); }
	void setLineNeedleValue(lv_obj_t* needle_line, int32_t needle_length, int32_t value) noexcept { lv_scale_set_line_needle_value(ptr, needle_line, needle_length, value); }
	void setImageNeedleValue(lv_obj_t* needle_img, int32_t value) noexcept { lv_scale_set_image_needle_value(ptr, needle_img, value); }
	void setTextSrc(const char* txt_src[]) noexcept { lv_scale_set_text_src(ptr, txt_src); }
	void setPostDraw(bool en) noexcept { lv_scale_set_post_draw(ptr, en); }
	void setDrawTicksOnTop(bool en) noexcept { lv_scale_set_draw_ticks_on_top(ptr, en); }
	lv_scale_section_t* addSection() noexcept { return lv_scale_add_section(ptr); }
	void setSectionRange(lv_scale_section_t* section, int32_t min, int32_t max) noexcept { lv_scale_set_section_range(ptr, section, min, max); }
	void setSectionMinValue(lv_scale_section_t* section, int32_t min) noexcept { lv_scale_set_section_min_value(ptr, section, min); }
	void setSectionMaxValue(lv_scale_section_t* section, int32_t max) noexcept { lv_scale_set_section_max_value(ptr, section, max); }
	void setSectionStyleMain(lv_scale_section_t* section, const lv_style_t* style) noexcept { lv_scale_set_section_style_main(ptr, section, style); }
	void setSectionStyleIndicator(lv_scale_section_t* section, const lv_style_t* style) noexcept { lv_scale_set_section_style_indicator(ptr, section, style); }
	void setSectionStyleItems(lv_scale_section_t* section, const lv_style_t* style) noexcept { lv_scale_set_section_style_items(ptr, section, style); }
	// Getter functions
	lv_scale_mode_t getMode() const noexcept { return lv_scale_get_mode(ptr); }
	int32_t getTotalTickCount() const noexcept { return lv_scale_get_total_tick_count(ptr); }
	int32_t getMajorTickEvery() const noexcept { return lv_scale_get_major_tick_every(ptr); }
	int32_t getRotation() const noexcept { return lv_scale_get_rotation(ptr); }
	bool getLabelShow() const noexcept { return lv_scale_get_label_show(ptr); }
	uint32_t getAngleRange() const noexcept { return lv_scale_get_angle_range(ptr); }
	int32_t getRangeMinValue() const noexcept { return lv_scale_get_range_min_value(ptr); }
	int32_t getRangeMaxValue() const noexcept { return lv_scale_get_range_max_value(ptr); }
	// Other functions
#if LV_USE_OBSERVER
	lv_observer_t* bindSectionMinValue(lv_scale_section_t* section, lv_subject_t* subject) noexcept { return lv_scale_bind_section_min_value(ptr, section, subject); }
	lv_observer_t* bindSectionMaxValue(lv_scale_section_t* section, lv_subject_t* subject) noexcept { return lv_scale_bind_section_max_value(ptr, section, subject); }
#endif
};

#endif // defined(__cplusplus)
