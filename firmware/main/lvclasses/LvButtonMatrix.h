//
//  LvButtonMatrix.h
//
//  created by yu2924 on 2025-05-14
//  This file was generated mostly by GitHub Copilot and partially edited manually.
//

#pragma once

#include "LvObj.h"

#if defined(__cplusplus)

struct LvButtonMatrix : public LvObj
{
	constexpr LvButtonMatrix() noexcept = default;
	constexpr LvButtonMatrix(lv_obj_t* p) noexcept : LvObj(p) {}
	constexpr LvButtonMatrix(std::nullptr_t) noexcept : LvObj(nullptr) {}
	static LvButtonMatrix create(lv_obj_t* parent) noexcept { return LvButtonMatrix(lv_buttonmatrix_create(parent)); }
	// Setter functions
 	void setMap(const char* const map[]) noexcept { lv_buttonmatrix_set_map(ptr, map); }
	void setCtrlMap(const lv_buttonmatrix_ctrl_t ctrl_map[]) noexcept { lv_buttonmatrix_set_ctrl_map(ptr, ctrl_map); }
	void setSelectedButton(uint32_t btn_id) noexcept { lv_buttonmatrix_set_selected_button(ptr, btn_id); }
	void setButtonCtrl(uint32_t btn_id, lv_buttonmatrix_ctrl_t ctrl) noexcept { lv_buttonmatrix_set_button_ctrl(ptr, btn_id, ctrl); }
	void clearButtonCtrl(uint32_t btn_id, lv_buttonmatrix_ctrl_t ctrl) noexcept { lv_buttonmatrix_clear_button_ctrl(ptr, btn_id, ctrl); }
	void setButtonCtrlAll(lv_buttonmatrix_ctrl_t ctrl) noexcept { lv_buttonmatrix_set_button_ctrl_all(ptr, ctrl); }
	void clearButtonCtrlAll(lv_buttonmatrix_ctrl_t ctrl) noexcept { lv_buttonmatrix_clear_button_ctrl_all(ptr, ctrl); }
	void setButtonWidth(uint32_t btn_id, uint32_t width) noexcept { lv_buttonmatrix_set_button_width(ptr, btn_id, width); }
	void setOneChecked(bool en) noexcept { lv_buttonmatrix_set_one_checked(ptr, en); }
	// Getter functions
	const char* const* getMap() const noexcept {return lv_buttonmatrix_get_map(ptr); }
	uint32_t getSelectedButton() const noexcept { return lv_buttonmatrix_get_selected_button(ptr); }
	const char* getButtonText(uint32_t btn_id) const noexcept { return lv_buttonmatrix_get_button_text(ptr, btn_id); }
	bool hasButtonCtrl(uint32_t btn_id, lv_buttonmatrix_ctrl_t ctrl) const noexcept { return lv_buttonmatrix_has_button_ctrl(ptr, btn_id, ctrl); }
	bool getOneChecked() const noexcept { return lv_buttonmatrix_get_one_checked(ptr); }
};

#endif // defined(__cplusplus)
