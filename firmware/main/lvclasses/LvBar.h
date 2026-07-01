//
//  LvBar.h
//
//  created by yu2924 on 2025-11-22
//  This file was generated mostly by GitHub Copilot and partially edited manually.
//

#pragma once

#include "LvObj.h"

#if defined(__cplusplus)

struct LvBar : public LvObj
{
    constexpr LvBar() noexcept = default;
    constexpr LvBar(lv_obj_t* p) noexcept : LvObj(p) {}
    constexpr LvBar(std::nullptr_t) noexcept : LvObj(nullptr) {}
    static LvBar create(lv_obj_t* parent) noexcept { return LvBar(lv_bar_create(parent)); }
    // setters
    void setValue(int32_t value, lv_anim_enable_t anim) noexcept { lv_bar_set_value(ptr, value, anim); }
    void setStartValue(int32_t startValue, lv_anim_enable_t anim) noexcept { lv_bar_set_start_value(ptr, startValue, anim); }
    void setRange(int32_t min, int32_t max) noexcept { lv_bar_set_range(ptr, min, max); }
    void setMinValue(int32_t min) noexcept { lv_bar_set_min_value(ptr, min); }
    void setMaxValue(int32_t max) noexcept { lv_bar_set_max_value(ptr, max); }
    void setMode(lv_bar_mode_t mode) noexcept { lv_bar_set_mode(ptr, mode); }
    void setOrientation(lv_bar_orientation_t orientation) noexcept { lv_bar_set_orientation(ptr, orientation); }
    // getters
    int32_t getValue() const noexcept { return lv_bar_get_value(ptr); }
    int32_t getStartValue() const noexcept { return lv_bar_get_start_value(ptr); }
    int32_t getMinValue() const noexcept { return lv_bar_get_min_value(ptr); }
    int32_t getMaxValue() const noexcept { return lv_bar_get_max_value(ptr); }
    lv_bar_mode_t getMode() const noexcept { return lv_bar_get_mode(ptr); }
    lv_bar_orientation_t getOrientation() const noexcept { return lv_bar_get_orientation(ptr); }
    bool isSymmetrical() const noexcept { return lv_bar_is_symmetrical(ptr); }
#if LV_USE_OBSERVER
    lv_observer_t* bindValue(lv_subject_t* subject) noexcept { return lv_bar_bind_value(ptr, subject); }
#endif
};

#endif // defined(__cplusplus)
