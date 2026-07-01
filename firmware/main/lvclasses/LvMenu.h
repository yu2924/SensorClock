//
//  LvMenu.h
//
//  created by yu2924 on 2025-11-20
//  This file was generated mostly by GitHub Copilot and partially edited manually.
//

#pragma once

#include "LvObj.h"

#if defined(__cplusplus)

struct LvMenu : public LvObj
{
	constexpr LvMenu() noexcept = default;
	constexpr LvMenu(lv_obj_t* p) noexcept : LvObj(p) {}
	constexpr LvMenu(std::nullptr_t) noexcept : LvObj(nullptr) {}
	static LvMenu create(lv_obj_t* parent) noexcept { return LvMenu(lv_menu_create(parent)); }
	struct Page : public LvObj
	{
		constexpr Page() noexcept = default;
		constexpr Page(lv_obj_t* p) noexcept : LvObj(p) {}
		constexpr Page(std::nullptr_t) noexcept : LvObj(nullptr) {}
		static Page create(LvMenu menu, char const* const title) noexcept { return Page(lv_menu_page_create(menu, title)); }
		void setPageTitle(char const* const title) noexcept { lv_menu_set_page_title(ptr, title); }
		void setPageTitleStatic(char const* const title) noexcept { lv_menu_set_page_title_static(ptr, title); }
	};
	struct Section : public LvObj
	{
		constexpr Section() noexcept = default;
		constexpr Section(lv_obj_t* p) noexcept : LvObj(p) {}
		constexpr Section(std::nullptr_t) noexcept : LvObj(nullptr) {}
		static Section create(Page page) noexcept { return Section(lv_menu_section_create(page)); }
	};
	struct Cont : public LvObj
	{
		constexpr Cont() noexcept = default;
		constexpr Cont(lv_obj_t* p) noexcept : LvObj(p) {}
		constexpr Cont(std::nullptr_t) noexcept : LvObj(nullptr) {}
		static Cont create(Page page) noexcept { return Cont(lv_menu_cont_create(page)); }
		static Cont create(Section sect) noexcept { return Cont(lv_menu_cont_create(sect)); }
	};
	struct Separator : public LvObj
	{
		constexpr Separator() noexcept = default;
		constexpr Separator(lv_obj_t* p) noexcept : LvObj(p) {}
		constexpr Separator(std::nullptr_t) noexcept : LvObj(nullptr) {}
		static Separator create(Page page) noexcept { return Separator(lv_menu_separator_create(page)); }
	};
	// setters
	void setPage(Page page) noexcept { lv_menu_set_page(ptr, page); }
	void setSidebarPage(Page page) noexcept { lv_menu_set_sidebar_page(ptr, page); }
	void setModeHeader(lv_menu_mode_header_t mode) noexcept { lv_menu_set_mode_header(ptr, mode); }
	void setModeRootBackButton(lv_menu_mode_root_back_button_t mode) noexcept { lv_menu_set_mode_root_back_button(ptr, mode); }
	void setLoadPageEvent(LvObj obj, Page page) noexcept { lv_menu_set_load_page_event(ptr, obj, page); }
	// getters
	Page getCurMainPage() const noexcept {return lv_menu_get_cur_main_page(ptr); }
	Page getCurSidebarPage() const noexcept { return lv_menu_get_cur_sidebar_page(ptr); }
	LvObj getMainHeader() const noexcept {return lv_menu_get_main_header(ptr); }
	LvObj getMainHeaderBackButton() const noexcept {return lv_menu_get_main_header_back_button(ptr); }
	LvObj getSidebarHeader() const noexcept {return lv_menu_get_sidebar_header(ptr); }
	LvObj getSidebarHeaderBackButton() const noexcept {return lv_menu_get_sidebar_header_back_button(ptr); }
	bool backButtonIsRoot(LvObj obj) const noexcept {return lv_menu_back_button_is_root(ptr, obj); }
	// others
	void clearHistory(lv_obj_t* obj) noexcept { lv_menu_clear_history(ptr); }
};

#endif // defined(__cplusplus)
