//
//  LvList.h
//
//  created by yu2924 on 2025-11-21
//  This file was generated mostly by GitHub Copilot and partially edited manually.
//

#pragma once

#include "LvObj.h"

#if defined(__cplusplus)

struct LvList : public LvObj
{
    constexpr LvList() noexcept = default;
    constexpr LvList(lv_obj_t* p) noexcept : LvObj(p) {}
    constexpr LvList(std::nullptr_t) noexcept : LvObj(nullptr) {}
    static LvList create(lv_obj_t* parent) noexcept { return LvList(lv_list_create(parent)); }
    LvObj addText(const char* txt) noexcept { return lv_list_add_text(ptr, txt); }
    LvObj addButton(const void* icon, const char* txt) noexcept { return lv_list_add_button(ptr, icon, txt); }
    const char* getButtonText(LvObj btn) const noexcept { return lv_list_get_button_text(ptr, btn); }
    void setButtonText(LvObj btn, const char* txt) noexcept { lv_list_set_button_text(ptr, btn, txt); }
};

#endif // defined(__cplusplus)
