//
//  LvImage.h
//
//  created by yu2924 on 2025-11-20
//  This file was generated mostly by GitHub Copilot and partially edited manually.
//

#pragma once

#include "LvObj.h"

#if defined(__cplusplus)

struct LvImage : public LvObj
{
	constexpr LvImage() noexcept = default;
	constexpr LvImage(lv_obj_t* p) noexcept : LvObj(p) {}
	constexpr LvImage(std::nullptr_t) noexcept : LvObj(nullptr) {}
	static LvImage create(lv_obj_t* parent) noexcept { return LvImage(lv_image_create(parent)); }
	// setters
	void setSrc(const void* src) noexcept { lv_image_set_src(ptr, src); }
	void setOffsetX(int32_t x) noexcept { lv_image_set_offset_x(ptr, x); }
	void setOffsetY(int32_t y) noexcept { lv_image_set_offset_y(ptr, y); }
	void setRotation(int32_t angle) noexcept { lv_image_set_rotation(ptr, angle); }
	void setPivot(int32_t x, int32_t y) noexcept { lv_image_set_pivot(ptr, x, y); }
	void setPivotX(int32_t x) noexcept { lv_image_set_pivot_x(ptr, x); }
	void setPivotY(int32_t y) noexcept { lv_image_set_pivot_y(ptr, y); }
	void setScale(uint32_t zoom) noexcept { lv_image_set_scale(ptr, zoom); }
	void setScaleX(uint32_t zoom) noexcept { lv_image_set_scale_x(ptr, zoom); }
	void setScaleY(uint32_t zoom) noexcept { lv_image_set_scale_y(ptr, zoom); }
	void setBlendMode(lv_blend_mode_t blend_mode) noexcept { lv_image_set_blend_mode(ptr, blend_mode); }
	void setAntialias(bool antialias) noexcept { lv_image_set_antialias(ptr, antialias); }
	void setInnerAlign(lv_image_align_t align) noexcept { lv_image_set_inner_align(ptr, align); }
	void setBitmapMapSrc(const lv_image_dsc_t* src) noexcept { lv_image_set_bitmap_map_src(ptr, src); }
	// getters
	const void* getSrc() const noexcept { return lv_image_get_src(ptr); }
	int32_t getOffsetX() const noexcept { return lv_image_get_offset_x(ptr); }
	int32_t getOffsetY() const noexcept { return lv_image_get_offset_y(ptr); }
	int32_t getRotation() const noexcept { return lv_image_get_rotation(ptr); }
	void getPivot(lv_point_t* pivot) const noexcept { lv_image_get_pivot(ptr, pivot); }
	int32_t getScale() const noexcept { return lv_image_get_scale(ptr); }
	int32_t getScaleX() const noexcept { return lv_image_get_scale_x(ptr); }
	int32_t getScaleY() const noexcept { return lv_image_get_scale_y(ptr); }
	int32_t getSrcWidth() const noexcept { return lv_image_get_src_width(ptr); }
	int32_t getSrcHeight() const noexcept { return lv_image_get_src_height(ptr); }
	int32_t getTransformedWidth() const noexcept { return lv_image_get_transformed_width(ptr); }
	int32_t getTransformedHeight() const noexcept { return lv_image_get_transformed_height(ptr); }
	lv_blend_mode_t getBlendMode() const noexcept { return lv_image_get_blend_mode(ptr); }
	bool getAntialias() const noexcept { return lv_image_get_antialias(ptr); }
	lv_image_align_t getInnerAlign() const noexcept { return lv_image_get_inner_align(ptr); }
	const lv_image_dsc_t* getBitmapMapSrc() const noexcept { return lv_image_get_bitmap_map_src(ptr); }
#if LV_USE_OBSERVER
	// observer binding (optional API)
	lv_observer_t* bindSrc(lv_subject_t* subject) noexcept { return lv_image_bind_src(ptr, subject); }
#endif
};

#endif // defined(__cplusplus)
