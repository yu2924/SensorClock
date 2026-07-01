//
//  LvTextArea.h
//
//  created by yu2924 on 2025-11-17
//  This file was generated mostly by GitHub Copilot and partially edited manually.
//

#pragma once

#include "LvObj.h"

#if defined(__cplusplus)

struct LvTextArea : public LvObj
{
	constexpr LvTextArea() noexcept = default;
	constexpr LvTextArea(lv_obj_t* p) noexcept : LvObj(p) {}
	constexpr LvTextArea(std::nullptr_t) noexcept : LvObj(nullptr) {}
	static LvTextArea create(lv_obj_t* parent) noexcept { return LvTextArea(lv_textarea_create(parent)); }
	// add / remove
	void addChar(uint32_t c) noexcept { lv_textarea_add_char(ptr, c); }
	void addText(const char* txt) noexcept { lv_textarea_add_text(ptr, txt); }
	void deleteChar() noexcept { lv_textarea_delete_char(ptr); }
	void deleteCharForward() noexcept { lv_textarea_delete_char_forward(ptr); }
	// setters
	void setText(const char* txt) noexcept { lv_textarea_set_text(ptr, txt); }
	void setPlaceholderText(const char* txt) noexcept { lv_textarea_set_placeholder_text(ptr, txt); }
	void setCursorPos(int32_t pos) noexcept { lv_textarea_set_cursor_pos(ptr, pos); }
	void setCursorClickPos(bool en) noexcept { lv_textarea_set_cursor_click_pos(ptr, en); }
	void setPasswordMode(bool en) noexcept { lv_textarea_set_password_mode(ptr, en); }
	void setPasswordBullet(const char* bullet) noexcept { lv_textarea_set_password_bullet(ptr, bullet); }
	void setOneLine(bool en) noexcept { lv_textarea_set_one_line(ptr, en); }
	void setAcceptedChars(const char* list) noexcept { lv_textarea_set_accepted_chars(ptr, list); }
	void setMaxLength(uint32_t num) noexcept { lv_textarea_set_max_length(ptr, num); }
	void setInsertReplace(const char* txt) noexcept { lv_textarea_set_insert_replace(ptr, txt); }
	void setTextSelection(bool en) noexcept { lv_textarea_set_text_selection(ptr, en); }
	void setPasswordShowTime(uint32_t time) noexcept { lv_textarea_set_password_show_time(ptr, time); }
	void setAlign(lv_text_align_t align) noexcept { lv_textarea_set_align(ptr, align); }
	// getters
	const char* getText() const noexcept { return lv_textarea_get_text(ptr); }
	const char* getPlaceholderText() const noexcept { return lv_textarea_get_placeholder_text(ptr); }
	LvObj getLabel() const noexcept { return LvObj(lv_textarea_get_label(ptr)); }
	uint32_t getCursorPos() const noexcept { return lv_textarea_get_cursor_pos(ptr); }
	bool getCursorClickPos() const noexcept { return lv_textarea_get_cursor_click_pos(ptr); }
	bool getPasswordMode() const noexcept { return lv_textarea_get_password_mode(ptr); }
	const char* getPasswordBullet() const noexcept { return lv_textarea_get_password_bullet(ptr); }
	bool getOneLine() const noexcept { return lv_textarea_get_one_line(ptr); }
	const char* getAcceptedChars() const noexcept { return lv_textarea_get_accepted_chars(ptr); }
	uint32_t getMaxLength() const noexcept { return lv_textarea_get_max_length(ptr); }
	bool textIsSelected() const noexcept { return lv_textarea_text_is_selected(ptr); }
	bool getTextSelection() const noexcept { return lv_textarea_get_text_selection(ptr); }
	uint32_t getPasswordShowTime() const noexcept { return lv_textarea_get_password_show_time(ptr); }
	uint32_t getCurrentChar() const noexcept { return lv_textarea_get_current_char(ptr); }
	// others
	void clearSelection() noexcept { lv_textarea_clear_selection(ptr); }
	void cursorRight() noexcept { lv_textarea_cursor_right(ptr); }
	void cursorLeft() noexcept { lv_textarea_cursor_left(ptr); }
	void cursorDown() noexcept { lv_textarea_cursor_down(ptr); }
	void cursorUp() noexcept { lv_textarea_cursor_up(ptr); }
};

#endif // defined(__cplusplus)
