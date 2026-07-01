//
//  LvLine.h
//
//  created by yu2924 on 2025-05-14
//  This file was generated mostly by GitHub Copilot and partially edited manually.
//

#pragma once

#include "LvObj.h"

#if defined(__cplusplus)

struct LvLine : public LvObj
{
	constexpr LvLine() noexcept = default;
	constexpr LvLine(lv_obj_t* p) noexcept : LvObj(p) {}
	constexpr LvLine(std::nullptr_t) noexcept : LvObj(nullptr) {}
	static LvLine create(lv_obj_t* parent) noexcept { return LvLine(lv_line_create(parent)); }
	// setter methods
 	void setPoints(const lv_point_precise_t points[], uint32_t point_num) noexcept { lv_line_set_points(ptr, points, point_num); }
	void setPointsMutable(lv_point_precise_t points[], uint32_t point_num) noexcept { lv_line_set_points_mutable(ptr, points, point_num); }
	void setYInvert(bool en) noexcept { lv_line_set_y_invert(ptr, en); }
	// getter methods
	const lv_point_precise_t* getPoints() const noexcept { return lv_line_get_points(ptr); }
	uint32_t getPointCount() const noexcept { return lv_line_get_point_count(ptr); }
	bool isPointArrayMutable() const noexcept { return lv_line_is_point_array_mutable(ptr); }
	lv_point_precise_t* getPointsMutable() const noexcept {return lv_line_get_points_mutable(ptr); }
	bool getYInvert() const noexcept {return lv_line_get_y_invert(ptr); }
};

#endif // defined(__cplusplus)
