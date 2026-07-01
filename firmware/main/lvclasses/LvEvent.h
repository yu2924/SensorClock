//
//  LvEvent.h
//
//  created by yu2924 on 2025-11-16
//  This file was generated mostly by GitHub Copilot and partially edited manually.
//

#pragma once

#include "lvgl.h"

#if defined(__cplusplus)

struct LvEvent
{
	lv_event_t* ptr = nullptr;
	constexpr LvEvent(lv_event_t* p = nullptr) noexcept : ptr(p) {}
	operator const lv_event_t* () const noexcept { return ptr; }
	operator lv_event_t* () noexcept { return ptr; }
	void* getTarget() const noexcept { return lv_event_get_target(ptr); }
	template<typename Tp> Tp getTargetOfType() const noexcept { return static_cast<Tp>(getTarget()); }
	void* getCurrentTarget() const noexcept { return lv_event_get_current_target(ptr); }
	template<typename Tp> Tp getCurrentTargetOfType() const noexcept { return static_cast<Tp>(getCurrentTarget()); }
	lv_event_code_t getCode() const noexcept { return lv_event_get_code(ptr); }
	void* getParam() const noexcept { return lv_event_get_param(ptr); }
	void* getUserData() const noexcept { return lv_event_get_user_data(ptr); }
	template<typename Tp> Tp getUserDataOfType() const { return static_cast<Tp>(getUserData()); }
	void stopBubbling() noexcept { lv_event_stop_bubbling(ptr); }
	void stopTrickling() noexcept { lv_event_stop_trickling(ptr); }
	void stopProcessing() noexcept { lv_event_stop_processing(ptr); }
	void freeUserDataCb() noexcept { lv_event_free_user_data_cb(ptr); }
	static uint32_t registerId() noexcept { return lv_event_register_id(); }
	static const char* codeGetName(lv_event_code_t code) noexcept { return lv_event_code_get_name(code); }
	// lv_obj_event.h
	lv_obj_t* getCurrentTargetObj() noexcept { return lv_event_get_current_target_obj(ptr); }
	lv_obj_t* getTargetObj() noexcept { return lv_event_get_target_obj(ptr); }
	// extension
	bool testTargetAndCode(lv_obj_t* obj, lv_event_code_t code) { return (getTarget() == obj) && (getCode() == code); }
};

#endif // defined(__cplusplus)
