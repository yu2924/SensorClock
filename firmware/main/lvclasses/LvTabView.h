//
//  LvTabView.h
//
//  created by yu2924 on 2025-11-21
//  This file was generated mostly by GitHub Copilot and partially edited manually.
//

#pragma once

#include "LvObj.h"
#include "LvButton.h"

#if defined(__cplusplus)

struct LvTabView : public LvObj
{
    constexpr LvTabView() noexcept = default;
    constexpr LvTabView(lv_obj_t* p) noexcept : LvObj(p) {}
    constexpr LvTabView(std::nullptr_t) noexcept : LvObj(nullptr) {}
    static LvTabView create(lv_obj_t* parent) noexcept { return LvTabView(lv_tabview_create(parent)); }
    LvObj addTab(const char* name) noexcept { return LvObj(lv_tabview_add_tab(ptr, name)); }
    void renameTab(uint32_t idx, const char* newName) noexcept { lv_tabview_rename_tab(ptr, idx, newName); }
    void setActive(uint32_t idx, lv_anim_enable_t animEn) noexcept { lv_tabview_set_active(ptr, idx, animEn); }
    void setTabBarPosition(lv_dir_t dir) noexcept { lv_tabview_set_tab_bar_position(ptr, dir); }
    void setTabBarSize(int32_t size) noexcept { lv_tabview_set_tab_bar_size(ptr, size); }
    uint32_t getTabCount() const noexcept { return lv_tabview_get_tab_count(ptr); }
    uint32_t getActiveTab() const noexcept { return lv_tabview_get_tab_active(ptr); }
    LvButton getTabButton(int32_t idx) const noexcept { return LvButton(lv_tabview_get_tab_button(ptr, idx)); }
    LvObj getContent() const noexcept { return LvObj(lv_tabview_get_content(ptr)); }
    LvObj getTabBar() const noexcept { return LvObj(lv_tabview_get_tab_bar(ptr)); }
};

#endif // defined(__cplusplus)
