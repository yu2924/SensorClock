//
//  LvKeyboard.h
//
//  created by yu2924 on 2025-11-17
//  This file was generated mostly by GitHub Copilot and partially edited manually.
//

#pragma once

#include "LvObj.h"

#if defined(__cplusplus)

struct LvKeyboard : public LvObj
{
	constexpr LvKeyboard() noexcept = default;
	constexpr LvKeyboard(lv_obj_t* p) noexcept : LvObj(p) {}
	constexpr LvKeyboard(std::nullptr_t) noexcept : LvObj(nullptr) {}
	static LvKeyboard create(lv_obj_t* parent) noexcept { return LvKeyboard(lv_keyboard_create(parent)); }
	// setters
	void setTextarea(lv_obj_t* ta) noexcept { lv_keyboard_set_textarea(ptr, ta); }
	void setMode(lv_keyboard_mode_t mode) noexcept { lv_keyboard_set_mode(ptr, mode); }
	void setPopovers(bool en) noexcept { lv_keyboard_set_popovers(ptr, en); }
	void setMap(lv_keyboard_mode_t mode, const char * const map[], const lv_buttonmatrix_ctrl_t ctrl_map[]) noexcept { lv_keyboard_set_map(ptr, mode, map, ctrl_map); }
	// getters
	lv_obj_t* getTextarea() const noexcept { return lv_keyboard_get_textarea(ptr); }
	lv_keyboard_mode_t getMode() const noexcept { return lv_keyboard_get_mode(ptr); }
	bool getPopovers() const noexcept { return lv_keyboard_get_popovers(ptr); }
	const char * const * getMapArray() const noexcept { return lv_keyboard_get_map_array(ptr); }
	uint32_t getSelectedButton() const noexcept { return lv_keyboard_get_selected_button(ptr); }
	const char* getButtonText(uint32_t btn_id) const noexcept { return lv_keyboard_get_button_text(ptr, btn_id); }
	// others
	static void defEventCb(lv_event_t* e) noexcept { lv_keyboard_def_event_cb(e); }
};

#endif // defined(__cplusplus)
