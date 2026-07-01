//
//  LvLabel.h
//
//  created by yu2924 on 2025-11-16
//  This file was generated mostly by GitHub Copilot and partially edited manually.
//

#pragma once

#include "LvObj.h"
#include <cstdarg>

#if defined(__cplusplus)

struct LvLabel : public LvObj
{
	constexpr LvLabel() noexcept = default;
	constexpr LvLabel(lv_obj_t* p) noexcept : LvObj(p) {}
	constexpr LvLabel(std::nullptr_t) noexcept : LvObj(nullptr) {}
	static LvLabel create(lv_obj_t* parent) noexcept { return LvLabel(lv_label_create(parent)); }
	// setters
	void setText(const char * txt) noexcept { lv_label_set_text(ptr, txt); }
	void setTextFmt(const char * fmt, ...) noexcept { va_list args; va_start(args, fmt); lv_label_set_text_vfmt(ptr, fmt, args); va_end(args); }
	void setTextVfmt(const char * fmt, va_list args) noexcept { lv_label_set_text_vfmt(ptr, fmt, args); }
	void setTextStatic(const char * txt) noexcept { lv_label_set_text_static(ptr, txt); }
	void setLongMode(lv_label_long_mode_t long_mode) noexcept { lv_label_set_long_mode(ptr, long_mode); }
	void setTextSelectionStart(uint32_t index) noexcept { lv_label_set_text_selection_start(ptr, index); }
	void setTextSelectionEnd(uint32_t index) noexcept { lv_label_set_text_selection_end(ptr, index); }
	void setRecolor(bool en) noexcept { lv_label_set_recolor(ptr, en); }
#if LV_USE_TRANSLATION
	void set_translation_tag(const char * tag) noexcept { lv_label_set_translation_tag(ptr, tag); }
#endif // LV_USE_TRANSLATION
	// getters
	char * getText() const noexcept { return lv_label_get_text(ptr); }
	lv_label_long_mode_t getLongMode() const noexcept { return lv_label_get_long_mode(ptr); }
	void getLetterPos(uint32_t char_id, lv_point_t * pos) const noexcept { lv_label_get_letter_pos(ptr, char_id, pos); }
	uint32_t getLetterOn(lv_point_t * pos_in, bool bidi) const noexcept { return lv_label_get_letter_on(ptr, pos_in, bidi); }
	bool isCharUnderPos(lv_point_t * pos) const noexcept { return lv_label_is_char_under_pos(ptr, pos); }
	uint32_t getTextSelectionStart() const noexcept { return lv_label_get_text_selection_start(ptr); }
	uint32_t getTextSelectionEnd() const noexcept { return lv_label_get_text_selection_end(ptr); }
	bool getRecolor() const noexcept { return lv_label_get_recolor(ptr); }

#if LV_USE_OBSERVER
	lv_observer_t * bindText(lv_subject_t * subject, const char * fmt) noexcept { return lv_label_bind_text(ptr, subject, fmt); }
#endif
	// edit operations
	void insText(uint32_t pos, const char * txt) noexcept { lv_label_ins_text(ptr, pos, txt); }
	void cutText(uint32_t pos, uint32_t cnt) noexcept { lv_label_cut_text(ptr, pos, cnt); }
};

#endif // defined(__cplusplus)
