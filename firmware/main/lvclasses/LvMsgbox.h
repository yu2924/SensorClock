//
//  LvMsgbox.h
//
//  created by yu2924 on 2025-05-14
//  This file was generated mostly by GitHub Copilot and partially edited manually.
//

#pragma once

#include "LvObj.h"

#if defined(__cplusplus)

struct LvMsgbox : public LvObj
{
	constexpr LvMsgbox() noexcept = default;
	constexpr LvMsgbox(lv_obj_t* p) noexcept : LvObj(p) {}
	constexpr LvMsgbox(std::nullptr_t) noexcept : LvObj(nullptr) {}
	static LvMsgbox create(lv_obj_t* parent) noexcept { return LvMsgbox(lv_msgbox_create(parent)); }
	LvObj addTitle(const char* title) noexcept { return lv_msgbox_add_title(ptr, title); }
	LvObj addHeaderButton(const void* icon) noexcept { return lv_msgbox_add_header_button(ptr, icon); }
	LvObj addText(const char* text) noexcept { return lv_msgbox_add_text(ptr, text); }
	LvObj addFooterButton(const char* text) noexcept { return lv_msgbox_add_footer_button(ptr, text); }
	LvObj addCloseButton() noexcept { return lv_msgbox_add_close_button(ptr); }
	LvObj getHeader() const noexcept { return lv_msgbox_get_header(ptr); }
	LvObj getFooter() const noexcept { return lv_msgbox_get_footer(ptr); }
	LvObj getContent() const noexcept { return lv_msgbox_get_content(ptr); }
	LvObj getTitle() const noexcept { return lv_msgbox_get_title(ptr); }
	void close() noexcept { lv_msgbox_close(ptr); }
	void closeAsync() noexcept { lv_msgbox_close_async(ptr); }
};

#endif // defined(__cplusplus)
