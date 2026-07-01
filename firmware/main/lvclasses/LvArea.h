//
//  LvArea.h
//
//  created by yu2924 on 2025-11-16
//  This file was generated mostly by GitHub Copilot and partially edited manually.
//

#pragma once

#include "lvgl.h"
#include <algorithm>

#if defined(__cplusplus)

struct LvArea : public lv_area_t
{
	LvArea(const lv_area_t& r) noexcept : lv_area_t(r) {}
	LvArea(int32_t x1, int32_t y1, int32_t x2, int32_t y2) { lv_area_set(this, x1, y1, x2, y2); }
	LvArea& operator=(const lv_area_t& r) noexcept { *this = r; return *this; }
	operator const lv_area_t* () const noexcept { return this; }
	operator lv_area_t* () noexcept { return this; }
	// initializers / copy
	void set(int32_t x1, int32_t y1, int32_t x2, int32_t y2) noexcept { lv_area_set(this, x1, y1, x2, y2); }
	void copyFrom(const lv_area_t* src) noexcept { lv_area_copy(this, src); }
	// size / geometry
	int32_t getWidth() const noexcept { return lv_area_get_width(this); }
	int32_t getHeight() const noexcept { return lv_area_get_height(this); }
	void setWidth(int32_t w) noexcept { lv_area_set_width(this, w); }
	void setHeight(int32_t h) noexcept { lv_area_set_height(this, h); }
	uint32_t getSize() const noexcept { return lv_area_get_size(this); }
	// mutations
	void increase(int32_t w_extra, int32_t h_extra) noexcept { lv_area_increase(this, w_extra, h_extra); }
	void move(int32_t x_ofs, int32_t y_ofs) noexcept { lv_area_move(this, x_ofs, y_ofs); }
	// Alignment
	void alignTo(const lv_area_t& base, lv_align_t align, int32_t ofs_x = 0, int32_t ofs_y = 0) noexcept { lv_area_align(&base, this, align, ofs_x, ofs_y); }
	// extensions
	lv_point_t getCenter() const noexcept { return { (x1 + x2) / 2, (y1 + y2) / 2 }; }
	bool contains(int32_t x, int32_t y) const noexcept { return (x1 <= x) && (x <= x2) && (y1 <= y) && (y <= y2); }
	bool intersects(const lv_area_t& other) const noexcept { return !((other.x1 < x2) || (other.x2 < x1) || (y2 < other.y1) || (y1 > other.y2)); }
	void intersectWith(const lv_area_t& other) noexcept
	{
		// if no intersection, sets area to an empty rect (x1=0,y1=0,x2=-1,y2=-1)
		if(!intersects(other))
		{
			x1 = 0; y1 = 0; x2 = -1; y2 = -1;
			return;
		}
		int32_t nx1 = std::max(x1, other.x1);
		int32_t ny1 = std::max(y1, other.y1);
		int32_t nx2 = std::min(x2, other.x2);
		int32_t ny2 = std::min(y2, other.y2);
		x1 = nx1; y1 = ny1; x2 = nx2; y2 = ny2;
	}
	LvArea removeFromLeft(int32_t d) noexcept { LvArea rco = *this; d = std::min(getWidth(), std::max((int32_t)0, d)); x1 += d; rco.x2 = x1; return rco; }
	LvArea removeFromRight(int32_t d) noexcept { LvArea rco = *this; d = std::min(getWidth(), std::max((int32_t)0, d)); x2 -= d; rco.x1 = x2; return rco; }
	LvArea removeFromTop(int32_t d) noexcept { LvArea rco = *this; d = std::min(getHeight(), std::max((int32_t)0, d)); y1 += d; rco.y2 = y1; return rco; }
	LvArea removeFromBottom(int32_t d) noexcept { LvArea rco = *this; d = std::min(getHeight(), std::max((int32_t)0, d)); y2 -= d; rco.y1 = y2; return rco; }
	LvArea withX(int32_t x) const noexcept { LvArea rco = *this; rco.x1 = x; return rco; }
	LvArea withY(int32_t y) const noexcept { LvArea rco = *this; rco.y1 = y; return rco; }
	LvArea withWidth(int32_t w) const noexcept { LvArea rco = *this; rco.setWidth(w); return rco; }
	LvArea withHeight(int32_t h) const noexcept { LvArea rco = *this; rco.setHeight(h); return rco; }
	LvArea increased(int32_t w_extra, int32_t h_extra) const noexcept { LvArea rco = *this; rco.increase(w_extra, h_extra); return rco; }
	LvArea moved(int32_t x_ofs, int32_t y_ofs) const noexcept { LvArea rco = *this; rco.move(x_ofs, y_ofs); return rco; }
	static LvArea withPositionAndSize(int32_t x, int32_t y, int32_t w, int32_t h) { return { x, y, x + w, y + h }; }
};

struct LvPoint : public lv_point_t
{
	LvPoint(const lv_point_t& r) : lv_point_t(r) {}
	LvPoint& operator=(const lv_point_t& r) { *this = r; return *this; }
	operator const lv_point_t* () const { return this; }
	operator lv_point_t* () { return this; }
	void set(int32_t nx, int32_t ny) { lv_point_set(this, nx, ny); }
	void transform(int32_t angle, int32_t scale_x, int32_t scale_y, const lv_point_t* pivot = nullptr, bool zoom_first = true) { lv_point_transform(this, angle, scale_x, scale_y, pivot, zoom_first); }
	static lv_point_t fromPrecise(const lv_point_precise_t* p) { return lv_point_from_precise(p); }
	lv_point_precise_t toPrecise() const { return lv_point_to_precise(this); }
	void swap(LvPoint& other) { lv_point_swap(this, &other); }
};

#endif // defined(__cplusplus)
