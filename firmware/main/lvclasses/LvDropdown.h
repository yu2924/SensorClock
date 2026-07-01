//
//  LvDropdown.h
//
//  created by yu2924 on 2025-11-16
//  This file was generated mostly by GitHub Copilot and partially edited manually.
//

#pragma once

#include "LvObj.h"

#if defined(__cplusplus)

struct LvDropdown : public LvObj
{
	constexpr LvDropdown() noexcept = default;
	constexpr LvDropdown(lv_obj_t* p) noexcept : LvObj(p) {}
	constexpr LvDropdown(std::nullptr_t) noexcept : LvObj(nullptr) {}
	static LvDropdown create(lv_obj_t* parent) noexcept { return LvDropdown(lv_dropdown_create(parent)); }
	// setters
	void setText(const char* txt) noexcept { lv_dropdown_set_text(ptr, txt); }
	void setOptions(const char* options) noexcept { lv_dropdown_set_options(ptr, options); }
	void setOptionsStatic(const char* options) noexcept { lv_dropdown_set_options_static(ptr, options); }
	void addOption(const char* option, uint32_t pos) noexcept { lv_dropdown_add_option(ptr, option, pos); }
	void clearOptions() noexcept { lv_dropdown_clear_options(ptr); }
	void setSelected(uint32_t sel_opt) noexcept { lv_dropdown_set_selected(ptr, sel_opt); }
	void setDir(lv_dir_t dir) noexcept { lv_dropdown_set_dir(ptr, dir); }
	void setSymbol(const void* symbol) noexcept { lv_dropdown_set_symbol(ptr, symbol); }
	void setSelectedHighlight(bool en) noexcept { lv_dropdown_set_selected_highlight(ptr, en); }
	// getters
	LvObj getList() const noexcept { return LvObj(lv_dropdown_get_list(ptr)); }
	const char* getText() const noexcept { return lv_dropdown_get_text(ptr); }
	const char* getOptions() const noexcept { return lv_dropdown_get_options(ptr); }
	uint32_t getSelected() const noexcept { return lv_dropdown_get_selected(ptr); }
	uint32_t getOptionCount() const noexcept { return lv_dropdown_get_option_count(ptr); }
	void getSelectedStr(char* buf, uint32_t buf_size) const noexcept { lv_dropdown_get_selected_str(ptr, buf, buf_size); }
	int32_t getOptionIndex(const char* option) noexcept { return lv_dropdown_get_option_index(ptr, option); }
	const char* getSymbol() const noexcept { return lv_dropdown_get_symbol(ptr); }
	bool getSelectedHighlight() const noexcept { return lv_dropdown_get_selected_highlight(ptr); }
	lv_dir_t getDir() const noexcept { return lv_dropdown_get_dir(ptr); }
	// others
	void open() noexcept { lv_dropdown_open(ptr); }
	void close() noexcept { lv_dropdown_close(ptr); }
	bool isOpen() const noexcept { return lv_dropdown_is_open(ptr); }
#if LV_USE_OBSERVER
	// Bind an integer Subject to the dropdown's value. Returns the created observer.
	lv_observer_t* bindValue(lv_subject_t* subject) noexcept { return lv_dropdown_bind_value(ptr, subject); }
#endif // LV_USE_OBSERVER
};

#endif // defined(__cplusplus)
