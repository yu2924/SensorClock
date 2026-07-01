//
//  LvObj.h
//
//  created by yu2924 on 2025-11-16
//  This file was generated mostly by GitHub Copilot and partially edited manually.
//

#pragma once

#include "lvgl.h"

#if defined(__cplusplus)

struct LvObj
{
	lv_obj_t* ptr = nullptr;
	constexpr LvObj(lv_obj_t* p = nullptr) noexcept : ptr(p) {}
	constexpr lv_obj_t* get() const noexcept { return ptr; }
	constexpr lv_obj_t* operator->() const noexcept { return ptr; }
	constexpr operator lv_obj_t* () const noexcept { return ptr; }
	explicit constexpr operator bool() const noexcept { return ptr != nullptr; }
	// --------------------------------------------------------------------------------
	// lv_obj.h
	static LvObj create(lv_obj_t* parent) { return LvObj(lv_obj_create(parent)); }
	// lv_obj flag / state / user-data
	void addFlag(lv_obj_flag_t f) noexcept { lv_obj_add_flag(ptr, f); }
	void removeFlag(lv_obj_flag_t f) noexcept { lv_obj_remove_flag(ptr, f); }
	void setFlag(lv_obj_flag_t f, bool v) noexcept { lv_obj_set_flag(ptr, f, v); }
	void addState(lv_state_t state) noexcept { lv_obj_add_state(ptr, state); }
	void removeState(lv_state_t state) noexcept { lv_obj_remove_state(ptr, state); }
	void setState(lv_state_t state, bool v) noexcept { lv_obj_set_state(ptr, state, v); }
	void setUserData(void* user_data) noexcept { lv_obj_set_user_data(ptr, user_data); }
	void* getUserData() const noexcept { return lv_obj_get_user_data(ptr); }
	// getters
	bool hasFlag(lv_obj_flag_t f) const noexcept { return lv_obj_has_flag(ptr, f); }
	bool hasFlag_any(lv_obj_flag_t f) const noexcept { return lv_obj_has_flag_any(ptr, f); }
	lv_state_t getState() const noexcept { return lv_obj_get_state(ptr); }
	bool hasState(lv_state_t state) const noexcept { return lv_obj_has_state(ptr, state); }
	lv_group_t* getGroup() const noexcept { return lv_obj_get_group(ptr); }
	// other utilities
	void allocateSpecAttr() noexcept { lv_obj_allocate_spec_attr(ptr); }
	bool checkType(const lv_obj_class_t* class_p) const noexcept { return lv_obj_check_type(ptr, class_p); }
	bool hasClass(const lv_obj_class_t* class_p) const noexcept { return lv_obj_has_class(ptr, class_p); }
	const lv_obj_class_t* getClass() const noexcept { return lv_obj_get_class(ptr); }
	bool isValid() const noexcept { return lv_obj_is_valid(ptr); }
	// utility to null an external LvObj when the object is deleted
	static void nullOnDelete(LvObj* obj_ptr) noexcept { lv_obj_null_on_delete(&obj_ptr->ptr); }
	// screen / create events helpers
	void addScreenLoadEvent(lv_event_code_t trigger, LvObj screen, lv_screen_load_anim_t anim_type, uint32_t duration, uint32_t delay) noexcept { lv_obj_add_screen_load_event(ptr, trigger, screen.ptr, anim_type, duration, delay); }
	void addScreenCreateEvent(lv_event_code_t trigger, lv_screen_create_cb_t screen_create_cb, lv_screen_load_anim_t anim_type, uint32_t duration, uint32_t delay) noexcept { lv_obj_add_screen_create_event(ptr, trigger, screen_create_cb, anim_type, duration, delay); }
	void addPlayTimelineEvent(lv_event_code_t trigger, lv_anim_timeline_t* at, uint32_t delay, bool reverse) noexcept { lv_obj_add_play_timeline_event(ptr, trigger, at, delay, reverse); }
#if LV_USE_OBJ_ID
	// ID related (guarded by LV_USE_OBJ_ID)
	void setId(void* id) noexcept { lv_obj_set_id(ptr, id); }
	void* getId() const noexcept { return lv_obj_get_id(ptr); }
	LvObj findById(const void* id) const noexcept { return LvObj(lv_obj_find_by_id(ptr, id)); }
	// assign/free id - keep as member for convenience
	void assignId(const lv_obj_class_t* class_p) noexcept { lv_obj_assign_id(class_p, ptr); }
	void freeId() noexcept { lv_obj_free_id(ptr); }
	// static/global helpers
	static int idCompare(const void* id1, const void* id2) noexcept { return lv_obj_id_compare(id1, id2); }
	const char* stringifyId(char buf[], uint32_t len) noexcept { return lv_obj_stringify_id(ptr, buf, len); }
#if LV_USE_OBJ_ID_BUILTIN
	static void destroyBuiltinId() noexcept { lv_objid_builtin_destroy(); }
#endif
#endif // LV_USE_OBJ_ID
	// --------------------------------------------------------------------------------
	// lv_obj_tree.h
	void deleteObject() noexcept { lv_obj_delete(ptr); }
	void clean() noexcept { lv_obj_clean(ptr); }
	void deleteDelayed(uint32_t delay_ms) noexcept { lv_obj_delete_delayed(ptr, delay_ms); }
	void deleteAsync() noexcept { lv_obj_delete_async(ptr); }
	void setParent(LvObj parent) noexcept { lv_obj_set_parent(ptr, parent.ptr); }
	void swapWith(LvObj other) noexcept { lv_obj_swap(ptr, other.ptr); }
	void moveToIndex(int32_t index) noexcept { lv_obj_move_to_index(ptr, index); }
	LvObj getParent() const noexcept { return LvObj(lv_obj_get_parent(ptr)); }
	LvObj getScreen() const noexcept { return lv_obj_get_screen(ptr); }
	lv_display_t* getDisplay() const noexcept { return lv_obj_get_display(ptr); }
	LvObj getChild(int32_t idx) const noexcept { return LvObj(lv_obj_get_child(ptr, idx)); }
	LvObj getChildByType(int32_t idx, const lv_obj_class_t* class_p) const noexcept { return LvObj(lv_obj_get_child_by_type(ptr, idx, class_p)); }
	LvObj getSibling(int32_t idx) const noexcept { return LvObj(lv_obj_get_sibling(ptr, idx)); }
	LvObj getSiblingByType(int32_t idx, const lv_obj_class_t* class_p) const noexcept { return LvObj(lv_obj_get_sibling_by_type(ptr, idx, class_p)); }
	uint32_t getChildCount() const noexcept { return lv_obj_get_child_count(ptr); }
	uint32_t getChildCountByType(const lv_obj_class_t* class_p) const noexcept { return lv_obj_get_child_count_by_type(ptr, class_p); }
	int32_t getIndex() const noexcept { return lv_obj_get_index(ptr); }
	int32_t getIndexByType(const lv_obj_class_t* class_p) const noexcept { return lv_obj_get_index_by_type(ptr, class_p); }
#if LV_USE_OBJ_NAME
	void setName(const char* name) noexcept { lv_obj_set_name(ptr, name); }
	void setNameStatic(const char* name) noexcept { lv_obj_set_name_static(ptr, name); }
	const char* getName() const noexcept { return lv_obj_get_name(ptr); }
	void getNameResolved(char buf[], size_t buf_size) const noexcept { lv_obj_get_name_resolved(ptr, buf, buf_size); }
	LvObj findByName(const char* name) const noexcept { return LvObj(lv_obj_find_by_name(ptr, name)); }
	LvObj getChildByName(const char* name_path) const noexcept { return LvObj(lv_obj_get_child_by_name(ptr, name_path)); }
#endif // LV_USE_OBJ_NAME
	void treeWalk(lv_obj_tree_walk_cb_t cb, void* user_data) const noexcept { lv_obj_tree_walk(ptr, cb, user_data); }
	void dumpTree() const noexcept { lv_obj_dump_tree(ptr); }
	// --------------------------------------------------------------------------------
	// lv_obj_pos.h
	void setPos(int32_t x, int32_t y) noexcept { lv_obj_set_pos(ptr, x, y); }
	void setX(int32_t x) noexcept { lv_obj_set_x(ptr, x); }
	void setY(int32_t y) noexcept { lv_obj_set_y(ptr, y); }
	void setSize(int32_t w, int32_t h) noexcept { lv_obj_set_size(ptr, w, h); }
	void setWidth(int32_t w) noexcept { lv_obj_set_width(ptr, w); }
	void setHeight(int32_t h) noexcept { lv_obj_set_height(ptr, h); }
	void setContentWidth(int32_t w) noexcept { lv_obj_set_content_width(ptr, w); }
	void setContentHeight(int32_t h) noexcept { lv_obj_set_content_height(ptr, h); }
	bool refrSize() noexcept { return lv_obj_refr_size(ptr); }
	void markLayoutDirty() noexcept { lv_obj_mark_layout_as_dirty(ptr); }
	void updateLayout() const noexcept { lv_obj_update_layout(ptr); }
	void setLayout(uint32_t layout) noexcept { lv_obj_set_layout(ptr, layout); }
	bool isLayoutPositioned() const noexcept { return lv_obj_is_layout_positioned(ptr); }
	void setAlign(lv_align_t align) noexcept { lv_obj_set_align(ptr, align); }
	void align(lv_align_t align, int32_t x_ofs, int32_t y_ofs) noexcept { lv_obj_align(ptr, align, x_ofs, y_ofs); }
	void alignTo(const LvObj& base, lv_align_t align, int32_t x_ofs, int32_t y_ofs) noexcept { lv_obj_align_to(ptr, base.ptr, align, x_ofs, y_ofs); }
	void center() noexcept { lv_obj_center(ptr); }
	void set_Transform(const lv_matrix_t* matrix) noexcept { lv_obj_set_transform(ptr, matrix); }
	void resetTransform() noexcept { lv_obj_reset_transform(ptr); }
	void getCoords(lv_area_t* coords) const noexcept { lv_obj_get_coords(ptr, coords); }
	int32_t getX() const noexcept { return lv_obj_get_x(ptr); }
	int32_t getX2() const noexcept { return lv_obj_get_x2(ptr); }
	int32_t getY() const noexcept { return lv_obj_get_y(ptr); }
	int32_t getY2() const noexcept { return lv_obj_get_y2(ptr); }
	int32_t getXAligned() const noexcept { return lv_obj_get_x_aligned(ptr); }
	int32_t getYAligned() const noexcept { return lv_obj_get_y_aligned(ptr); }
	int32_t getWidth() const noexcept { return lv_obj_get_width(ptr); }
	int32_t getHeight() const noexcept { return lv_obj_get_height(ptr); }
	int32_t getContentWidth() const noexcept { return lv_obj_get_content_width(ptr); }
	int32_t getContentHeight() const noexcept { return lv_obj_get_content_height(ptr); }
	void getContentCoords(lv_area_t* area) const noexcept { lv_obj_get_content_coords(ptr, area); }
	void refrPos() noexcept { lv_obj_refr_pos(ptr); }
	void moveTo(int32_t x, int32_t y) noexcept { lv_obj_move_to(ptr, x, y); }
	void moveChildrenBy(int32_t x_diff, int32_t y_diff, bool ignore_floating) noexcept { lv_obj_move_children_by(ptr, x_diff, y_diff, ignore_floating); }
	const lv_matrix_t* getTransform() const noexcept { return lv_obj_get_transform(ptr); }
	// invalidation / visibility
	void invalidateArea(const lv_area_t* area) const noexcept { lv_obj_invalidate_area(ptr, area); }
	void invalidate() const noexcept { lv_obj_invalidate(ptr); }
	bool areaIsVisible(lv_area_t* area) const noexcept { return lv_obj_area_is_visible(ptr, area); }
	bool isVisible() const noexcept { return lv_obj_is_visible(ptr); }
	// click area / hit test
	void set_ext_click_area(int32_t size) noexcept { lv_obj_set_ext_click_area(ptr, size); }
	void get_click_area(lv_area_t* area) const noexcept { lv_obj_get_click_area(ptr, area); }
	bool hit_test(const lv_point_t* point) noexcept { return lv_obj_hit_test(ptr, point); }
	// utility
	static int32_t clampWidth(int32_t width, int32_t min_width, int32_t max_width, int32_t ref_width) noexcept { return lv_clamp_width(width, min_width, max_width, ref_width); }
	static int32_t clampHeight(int32_t height, int32_t min_height, int32_t max_height, int32_t ref_height) noexcept { return lv_clamp_height(height, min_height, max_height, ref_height); }
	// extensions
	lv_area_t getBounds() const { return { getX(), getY(), getX2(), getY2() }; }
	void setBounds(int x, int y, int w, int h) { setPos(x, y); setSize(w, h); }
	void setBounds(const lv_area_t& rc) { setPos(rc.x1, rc.y1); setSize(lv_area_get_width(&rc), lv_area_get_height(&rc)); }
	// --------------------------------------------------------------------------------
	// lv_obj_scroll.h
	void setScrollbarMode(lv_scrollbar_mode_t mode) noexcept { lv_obj_set_scrollbar_mode(ptr, mode); }
	void setScrollDir(lv_dir_t dir) noexcept { lv_obj_set_scroll_dir(ptr, dir); }
	void setScrollSnap_x(lv_scroll_snap_t align) noexcept { lv_obj_set_scroll_snap_x(ptr, align); }
	void setScrollSnap_y(lv_scroll_snap_t align) noexcept { lv_obj_set_scroll_snap_y(ptr, align); }
	lv_scrollbar_mode_t getScrollbarMode() const noexcept { return lv_obj_get_scrollbar_mode(ptr); }
	lv_dir_t getScrollDir() const noexcept { return lv_obj_get_scroll_dir(ptr); }
	lv_scroll_snap_t getScrollSnapX() const noexcept { return lv_obj_get_scroll_snap_x(ptr); }
	lv_scroll_snap_t getScrollSnapY() const noexcept { return lv_obj_get_scroll_snap_y(ptr); }
	int32_t getScrollX() const noexcept { return lv_obj_get_scroll_x(ptr); }
	int32_t getScrollY() const noexcept { return lv_obj_get_scroll_y(ptr); }
	int32_t getScrollTop() const noexcept { return lv_obj_get_scroll_top(ptr); }
	int32_t getScrollBottom() const noexcept { return lv_obj_get_scroll_bottom(ptr); }
	int32_t getScrollLeft() const noexcept { return lv_obj_get_scroll_left(ptr); }
	int32_t getScrollRight() const noexcept { return lv_obj_get_scroll_right(ptr); }
	void getScrollEnd(lv_point_t* end) noexcept { lv_obj_get_scroll_end(ptr, end); }
	void scrollBy(int32_t dx, int32_t dy, lv_anim_enable_t anim_en) noexcept { lv_obj_scroll_by(ptr, dx, dy, anim_en); }
	void scrollByBounded(int32_t dx, int32_t dy, lv_anim_enable_t anim_en) noexcept { lv_obj_scroll_by_bounded(ptr, dx, dy, anim_en); }
	void scrollTo(int32_t x, int32_t y, lv_anim_enable_t anim_en) noexcept { lv_obj_scroll_to(ptr, x, y, anim_en); }
	void scrollToX(int32_t x, lv_anim_enable_t anim_en) noexcept { lv_obj_scroll_to_x(ptr, x, anim_en); }
	void scrollToY(int32_t y, lv_anim_enable_t anim_en) noexcept { lv_obj_scroll_to_y(ptr, y, anim_en); }
	void scrollToView(lv_anim_enable_t anim_en) noexcept { lv_obj_scroll_to_view(ptr, anim_en); }
	void scrollToViewRecursive(lv_anim_enable_t anim_en) noexcept { lv_obj_scroll_to_view_recursive(ptr, anim_en); }
	bool isScrolling() const noexcept { return lv_obj_is_scrolling(ptr); }
	void stopScrollAnim() const noexcept { lv_obj_stop_scroll_anim(ptr); }
	void updateSnap(lv_anim_enable_t anim_en) noexcept { lv_obj_update_snap(ptr, anim_en); }
	void getScrollbarArea(lv_area_t* hor, lv_area_t* ver) noexcept { lv_obj_get_scrollbar_area(ptr, hor, ver); }
	void scrollbarInvalidate() noexcept { lv_obj_scrollbar_invalidate(ptr); }
	void readjustScroll(lv_anim_enable_t anim_en) noexcept { lv_obj_readjust_scroll(ptr, anim_en); }
	// --------------------------------------------------------------------------------
	// lv_obj_style.h
	void addStyle(const lv_style_t* style, lv_style_selector_t selector) noexcept { lv_obj_add_style(ptr, style, selector); }
	bool replaceStyle(const lv_style_t* old_style, const lv_style_t* new_style, lv_style_selector_t selector) noexcept { return lv_obj_replace_style(ptr, old_style, new_style, selector); }
	void removeStyle(const lv_style_t* style, lv_style_selector_t selector) noexcept { lv_obj_remove_style(ptr, style, selector); }
	void removeStyleAll() noexcept { lv_obj_remove_style_all(ptr); }
	static void reportStyleChange(lv_style_t* style) noexcept { lv_obj_report_style_change(style); }
	void refreshStyle(lv_part_t part, lv_style_prop_t prop) noexcept { lv_obj_refresh_style(ptr, part, prop); }
	void styleSetDisabled(const lv_style_t* style, lv_style_selector_t selector, bool dis) noexcept { lv_obj_style_set_disabled(ptr, style, selector, dis); }
	bool styleGetDisabled(const lv_style_t* style, lv_style_selector_t selector) const noexcept { return lv_obj_style_get_disabled(ptr, style, selector); }
	static void enableStyleRefresh(bool en) noexcept { lv_obj_enable_style_refresh(en); }
	lv_style_value_t getStyleProp(lv_part_t part, lv_style_prop_t prop) const noexcept { return lv_obj_get_style_prop(ptr, part, prop); }
	bool hasStyleProp(lv_style_selector_t selector, lv_style_prop_t prop) const noexcept { return lv_obj_has_style_prop(ptr, selector, prop); }
	void setLocalStyleProp(lv_style_prop_t prop, lv_style_value_t value, lv_style_selector_t selector) noexcept { lv_obj_set_local_style_prop(ptr, prop, value, selector); }
	lv_style_res_t getLocalStyleProp(lv_style_prop_t prop, lv_style_value_t* value, lv_style_selector_t selector) noexcept { return lv_obj_get_local_style_prop(ptr, prop, value, selector); }
	bool removeLocalStyleProp(lv_style_prop_t prop, lv_style_selector_t selector) noexcept { return lv_obj_remove_local_style_prop(ptr, prop, selector); }
	lv_opa_t getStyleOpaRecursive(lv_part_t part) const noexcept { return lv_obj_get_style_opa_recursive(ptr, part); }
	lv_color32_t styleApplyRecolor(lv_part_t part, lv_color32_t color) const noexcept { return lv_obj_style_apply_recolor(ptr, part, color); }
	lv_color32_t getStyleRecolorRecursive(lv_part_t part) const noexcept { return lv_obj_get_style_recolor_recursive(ptr, part); }
	void fadeIn(uint32_t time, uint32_t delay) noexcept { lv_obj_fade_in(ptr, time, delay); }
	void fadeOut(uint32_t time, uint32_t delay) noexcept { lv_obj_fade_out(ptr, time, delay); }
	static lv_state_t styleGetSelectorStateCmp(lv_style_selector_t selector) noexcept { return lv_obj_style_get_selector_state(selector); }
	static lv_part_t styleGetSelectorPart(lv_style_selector_t selector) noexcept { return lv_obj_style_get_selector_part(selector); }
	// --------------------------------------------------------------------------------
	// lv_obj_draw.h
	void initDrawRectDsc(lv_part_t part, lv_draw_rect_dsc_t* draw_dsc) noexcept { lv_obj_init_draw_rect_dsc(ptr, part, draw_dsc); }
	void initDrawLabelDsc(lv_part_t part, lv_draw_label_dsc_t* draw_dsc) noexcept { lv_obj_init_draw_label_dsc(ptr, part, draw_dsc); }
	void initDrawImageDsc(lv_part_t part, lv_draw_image_dsc_t* draw_dsc) noexcept { lv_obj_init_draw_image_dsc(ptr, part, draw_dsc); }
	void initDrawLineDsc(lv_part_t part, lv_draw_line_dsc_t* draw_dsc) noexcept { lv_obj_init_draw_line_dsc(ptr, part, draw_dsc); }
	void initDrawArcDsc(lv_part_t part, lv_draw_arc_dsc_t* draw_dsc) noexcept { lv_obj_init_draw_arc_dsc(ptr, part, draw_dsc); }
	int32_t calculateExtDrawSize(lv_part_t part) const noexcept { return lv_obj_calculate_ext_draw_size(ptr, part); }
	void refreshExtDrawSize() noexcept { lv_obj_refresh_ext_draw_size(ptr); }
	// --------------------------------------------------------------------------------
	// lv_obj_event.h
	lv_result_t sendEvent(lv_event_code_t event_code, void* param) noexcept { return lv_obj_send_event(ptr, event_code, param); }
	lv_event_dsc_t* addEventCb(lv_event_cb_t event_cb, lv_event_code_t filter, void* user_data) noexcept { return lv_obj_add_event_cb(ptr, event_cb, filter, user_data); }
	uint32_t getEventCount() noexcept { return lv_obj_get_event_count(ptr); }
	lv_event_dsc_t* getEventDsc(uint32_t index) noexcept { return lv_obj_get_event_dsc(ptr, index); }
	bool removeEvent(uint32_t index) noexcept { return lv_obj_remove_event(ptr, index); }
	bool removeEventDsc(lv_event_dsc_t* dsc) noexcept { return lv_obj_remove_event_dsc(ptr, dsc); }
	uint32_t removeEventCb(lv_event_cb_t event_cb) noexcept { return lv_obj_remove_event_cb(ptr, event_cb); }
	uint32_t removeEventCbWithUserData(lv_event_cb_t event_cb, void* user_data) noexcept { return lv_obj_remove_event_cb_with_user_data(ptr, event_cb, user_data); }
	// --------------------------------------------------------------------------------
	// lv_obj_style_gen.h
	// style_gen getters
	int32_t getStyleWidth(lv_part_t part) const noexcept { return lv_obj_get_style_width(ptr, part); }
	int32_t getStyleMinWidth(lv_part_t part) const noexcept { return lv_obj_get_style_min_width(ptr, part); }
	int32_t getStyleMaxWidth(lv_part_t part) const noexcept { return lv_obj_get_style_max_width(ptr, part); }
	int32_t getStyleHeight(lv_part_t part) const noexcept { return lv_obj_get_style_height(ptr, part); }
	int32_t getStyleMinHeight(lv_part_t part) const noexcept { return lv_obj_get_style_min_height(ptr, part); }
	int32_t getStyleMaxHeight(lv_part_t part) const noexcept { return lv_obj_get_style_max_height(ptr, part); }
	int32_t getStyleLength(lv_part_t part) const noexcept { return lv_obj_get_style_length(ptr, part); }
	int32_t getStyleX(lv_part_t part) const noexcept { return lv_obj_get_style_x(ptr, part); }
	int32_t getStyleY(lv_part_t part) const noexcept { return lv_obj_get_style_y(ptr, part); }
	lv_align_t getStyleAlign(lv_part_t part) const noexcept { return lv_obj_get_style_align(ptr, part); }
	int32_t getStyleTransformWidth(lv_part_t part) const noexcept { return lv_obj_get_style_transform_width(ptr, part); }
	int32_t getStyleTransformHeight(lv_part_t part) const noexcept { return lv_obj_get_style_transform_height(ptr, part); }
	int32_t getStyleTranslateX(lv_part_t part) const noexcept { return lv_obj_get_style_translate_x(ptr, part); }
	int32_t getStyleTranslateY(lv_part_t part) const noexcept { return lv_obj_get_style_translate_y(ptr, part); }
	int32_t getStyleTranslateRadial(lv_part_t part) const noexcept { return lv_obj_get_style_translate_radial(ptr, part); }
	int32_t getStyleTransformScaleX(lv_part_t part) const noexcept { return lv_obj_get_style_transform_scale_x(ptr, part); }
	int32_t getStyleTransformScaleY(lv_part_t part) const noexcept { return lv_obj_get_style_transform_scale_y(ptr, part); }
	int32_t getStyleTransformRotation(lv_part_t part) const noexcept { return lv_obj_get_style_transform_rotation(ptr, part); }
	int32_t getStyleTransformPivotX(lv_part_t part) const noexcept { return lv_obj_get_style_transform_pivot_x(ptr, part); }
	int32_t getStyleTransformPivotY(lv_part_t part) const noexcept { return lv_obj_get_style_transform_pivot_y(ptr, part); }
	int32_t getStyleTransformSkewX(lv_part_t part) const noexcept { return lv_obj_get_style_transform_skew_x(ptr, part); }
	int32_t getStyleTransformSkewY(lv_part_t part) const noexcept { return lv_obj_get_style_transform_skew_y(ptr, part); }
	int32_t getStylePadTop(lv_part_t part) const noexcept { return lv_obj_get_style_pad_top(ptr, part); }
	int32_t getStylePadBottom(lv_part_t part) const noexcept { return lv_obj_get_style_pad_bottom(ptr, part); }
	int32_t getStylePadLeft(lv_part_t part) const noexcept { return lv_obj_get_style_pad_left(ptr, part); }
	int32_t getStylePadRight(lv_part_t part) const noexcept { return lv_obj_get_style_pad_right(ptr, part); }
	int32_t getStylePadRow(lv_part_t part) const noexcept { return lv_obj_get_style_pad_row(ptr, part); }
	int32_t getStylePadColumn(lv_part_t part) const noexcept { return lv_obj_get_style_pad_column(ptr, part); }
	int32_t getStylePadRadial(lv_part_t part) const noexcept { return lv_obj_get_style_pad_radial(ptr, part); }
	int32_t getStyleMarginTop(lv_part_t part) const noexcept { return lv_obj_get_style_margin_top(ptr, part); }
	int32_t getStyleMarginBottom(lv_part_t part) const noexcept { return lv_obj_get_style_margin_bottom(ptr, part); }
	int32_t getStyleMarginLeft(lv_part_t part) const noexcept { return lv_obj_get_style_margin_left(ptr, part); }
	int32_t getStyleMarginRight(lv_part_t part) const noexcept { return lv_obj_get_style_margin_right(ptr, part); }
	lv_color_t getStyleBgColor(lv_part_t part) const noexcept { return lv_obj_get_style_bg_color(ptr, part); }
	lv_color_t getStyleBgColorFiltered(lv_part_t part) const noexcept { return lv_obj_get_style_bg_color_filtered(ptr, part); }
	lv_opa_t getStyleBgOpa(lv_part_t part) const noexcept { return lv_obj_get_style_bg_opa(ptr, part); }
	lv_color_t getStyleBgGradColor(lv_part_t part) const noexcept { return lv_obj_get_style_bg_grad_color(ptr, part); }
	lv_color_t getStyleBgGradColorFiltered(lv_part_t part) const noexcept { return lv_obj_get_style_bg_grad_color_filtered(ptr, part); }
	lv_grad_dir_t getStyleBgGradDir(lv_part_t part) const noexcept { return lv_obj_get_style_bg_grad_dir(ptr, part); }
	int32_t getStyleBgMainStop(lv_part_t part) const noexcept { return lv_obj_get_style_bg_main_stop(ptr, part); }
	int32_t getStyleBgGradStop(lv_part_t part) const noexcept { return lv_obj_get_style_bg_grad_stop(ptr, part); }
	lv_opa_t getStyleBgMainOpa(lv_part_t part) const noexcept { return lv_obj_get_style_bg_main_opa(ptr, part); }
	lv_opa_t getStyleBgGradOpa(lv_part_t part) const noexcept { return lv_obj_get_style_bg_grad_opa(ptr, part); }
	const lv_grad_dsc_t* getStyleBgGrad(lv_part_t part) const noexcept { return lv_obj_get_style_bg_grad(ptr, part); }
	const void* getStyleBgImageSrc(lv_part_t part) const noexcept { return lv_obj_get_style_bg_image_src(ptr, part); }
	lv_opa_t getStyleBgImageOpa(lv_part_t part) const noexcept { return lv_obj_get_style_bg_image_opa(ptr, part); }
	lv_color_t getStyleBgImageRecolor(lv_part_t part) const noexcept { return lv_obj_get_style_bg_image_recolor(ptr, part); }
	lv_color_t getStyleBgImageRecolorFiltered(lv_part_t part) const noexcept { return lv_obj_get_style_bg_image_recolor_filtered(ptr, part); }
	lv_opa_t getStyleBgImageRecolorOpa(lv_part_t part) const noexcept { return lv_obj_get_style_bg_image_recolor_opa(ptr, part); }
	bool getStyleBgImageTiled(lv_part_t part) const noexcept { return lv_obj_get_style_bg_image_tiled(ptr, part); }
	lv_color_t getStyleBorderColor(lv_part_t part) const noexcept { return lv_obj_get_style_border_color(ptr, part); }
	lv_color_t getStyleBorderColorFiltered(lv_part_t part) const noexcept { return lv_obj_get_style_border_color_filtered(ptr, part); }
	lv_opa_t getStyleBorderOpa(lv_part_t part) const noexcept { return lv_obj_get_style_border_opa(ptr, part); }
	int32_t getStyleBorderWidth(lv_part_t part) const noexcept { return lv_obj_get_style_border_width(ptr, part); }
	lv_border_side_t getStyleBorderSide(lv_part_t part) const noexcept { return lv_obj_get_style_border_side(ptr, part); }
	bool getStyleBorderPost(lv_part_t part) const noexcept { return lv_obj_get_style_border_post(ptr, part); }
	int32_t getStyleOutlineWidth(lv_part_t part) const noexcept { return lv_obj_get_style_outline_width(ptr, part); }
	lv_color_t getStyleOutlineColor(lv_part_t part) const noexcept { return lv_obj_get_style_outline_color(ptr, part); }
	lv_color_t getStyleOutlineColorFiltered(lv_part_t part) const noexcept { return lv_obj_get_style_outline_color_filtered(ptr, part); }
	lv_opa_t getStyleOutlineOpa(lv_part_t part) const noexcept { return lv_obj_get_style_outline_opa(ptr, part); }
	int32_t getStyleOutlinePad(lv_part_t part) const noexcept { return lv_obj_get_style_outline_pad(ptr, part); }
	int32_t getStyleShadowWidth(lv_part_t part) const noexcept { return lv_obj_get_style_shadow_width(ptr, part); }
	int32_t getStyleShadowOffsetX(lv_part_t part) const noexcept { return lv_obj_get_style_shadow_offset_x(ptr, part); }
	int32_t getStyleShadowOffsetY(lv_part_t part) const noexcept { return lv_obj_get_style_shadow_offset_y(ptr, part); }
	int32_t getStyleShadowSpread(lv_part_t part) const noexcept { return lv_obj_get_style_shadow_spread(ptr, part); }
	lv_color_t getStyleShadowColor(lv_part_t part) const noexcept { return lv_obj_get_style_shadow_color(ptr, part); }
	lv_color_t getStyleShadowColorFiltered(lv_part_t part) const noexcept { return lv_obj_get_style_shadow_color_filtered(ptr, part); }
	lv_opa_t getStyleShadowOpa(lv_part_t part) const noexcept { return lv_obj_get_style_shadow_opa(ptr, part); }
	lv_opa_t getStyleImageOpa(lv_part_t part) const noexcept { return lv_obj_get_style_image_opa(ptr, part); }
	lv_color_t getStyleImageRecolor(lv_part_t part) const noexcept { return lv_obj_get_style_image_recolor(ptr, part); }
	lv_color_t getStyleImageRecolorFiltered(lv_part_t part) const noexcept { return lv_obj_get_style_image_recolor_filtered(ptr, part); }
	lv_opa_t getStyleImageRecolorOpa(lv_part_t part) const noexcept { return lv_obj_get_style_image_recolor_opa(ptr, part); }
	const lv_image_colorkey_t* getStyleImageColorkey(lv_part_t part) const noexcept { return lv_obj_get_style_image_colorkey(ptr, part); }
	int32_t getStyleLineWidth(lv_part_t part) const noexcept { return lv_obj_get_style_line_width(ptr, part); }
	int32_t getStyleLineDashWidth(lv_part_t part) const noexcept { return lv_obj_get_style_line_dash_width(ptr, part); }
	int32_t getStyleLineDashGap(lv_part_t part) const noexcept { return lv_obj_get_style_line_dash_gap(ptr, part); }
	bool getStyleLineRounded(lv_part_t part) const noexcept { return lv_obj_get_style_line_rounded(ptr, part); }
	lv_color_t getStyleLineColor(lv_part_t part) const noexcept { return lv_obj_get_style_line_color(ptr, part); }
	lv_color_t getStyleLineColorFiltered(lv_part_t part) const noexcept { return lv_obj_get_style_line_color_filtered(ptr, part); }
	lv_opa_t getStyleLineOpa(lv_part_t part) const noexcept { return lv_obj_get_style_line_opa(ptr, part); }
	int32_t getStyleArcWidth(lv_part_t part) const noexcept { return lv_obj_get_style_arc_width(ptr, part); }
	bool getStyleArcRounded(lv_part_t part) const noexcept { return lv_obj_get_style_arc_rounded(ptr, part); }
	lv_color_t getStyleArcColor(lv_part_t part) const noexcept { return lv_obj_get_style_arc_color(ptr, part); }
	lv_color_t getStyleArcColorFiltered(lv_part_t part) const noexcept { return lv_obj_get_style_arc_color_filtered(ptr, part); }
	lv_opa_t getStyleArcOpa(lv_part_t part) const noexcept { return lv_obj_get_style_arc_opa(ptr, part); }
	const void* getStyleArcImageSrc(lv_part_t part) const noexcept { return lv_obj_get_style_arc_image_src(ptr, part); }
	lv_color_t getStyleTextColor(lv_part_t part) const noexcept { return lv_obj_get_style_text_color(ptr, part); }
	lv_color_t getStyleTextColorFiltered(lv_part_t part) const noexcept { return lv_obj_get_style_text_color_filtered(ptr, part); }
	lv_opa_t getStyleTextOpa(lv_part_t part) const noexcept { return lv_obj_get_style_text_opa(ptr, part); }
	const lv_font_t* getStyleTextFont(lv_part_t part) const noexcept { return lv_obj_get_style_text_font(ptr, part); }
	int32_t getStyleTextLetterSpace(lv_part_t part) const noexcept { return lv_obj_get_style_text_letter_space(ptr, part); }
	int32_t getStyleTextLineSpace(lv_part_t part) const noexcept { return lv_obj_get_style_text_line_space(ptr, part); }
	lv_text_decor_t getStyleTextDecor(lv_part_t part) const noexcept { return lv_obj_get_style_text_decor(ptr, part); }
	lv_text_align_t getStyleTextAlign(lv_part_t part) const noexcept { return lv_obj_get_style_text_align(ptr, part); }
	lv_color_t getStyleTextOutlineStrokeColor(lv_part_t part) const noexcept { return lv_obj_get_style_text_outline_stroke_color(ptr, part); }
	lv_color_t getStyleTextOutlineStrokeColorFiltered(lv_part_t part) const noexcept { return lv_obj_get_style_text_outline_stroke_color_filtered(ptr, part); }
	int32_t getStyleTextOutlineStrokeWidth(lv_part_t part) const noexcept { return lv_obj_get_style_text_outline_stroke_width(ptr, part); }
	lv_opa_t getStyleTextOutlineStrokeOpa(lv_part_t part) const noexcept { return lv_obj_get_style_text_outline_stroke_opa(ptr, part); }
	int32_t getStyleRadius(lv_part_t part) const noexcept { return lv_obj_get_style_radius(ptr, part); }
	int32_t getStyleRadialOffset(lv_part_t part) const noexcept { return lv_obj_get_style_radial_offset(ptr, part); }
	bool getStyleClipCorner(lv_part_t part) const noexcept { return lv_obj_get_style_clip_corner(ptr, part); }
	lv_opa_t getStyleOpa(lv_part_t part) const noexcept { return lv_obj_get_style_opa(ptr, part); }
	lv_opa_t getStyleOpaLayered(lv_part_t part) const noexcept { return lv_obj_get_style_opa_layered(ptr, part); }
	const lv_color_filter_dsc_t* getStyleColorFilterDsc(lv_part_t part) const noexcept { return lv_obj_get_style_color_filter_dsc(ptr, part); }
	lv_opa_t getStyleColorFilterOpa(lv_part_t part) const noexcept { return lv_obj_get_style_color_filter_opa(ptr, part); }
	lv_color_t getStyleRecolor(lv_part_t part) const noexcept { return lv_obj_get_style_recolor(ptr, part); }
	lv_opa_t getStyleRecolorOpa(lv_part_t part) const noexcept { return lv_obj_get_style_recolor_opa(ptr, part); }
	const lv_anim_t* getStyleAnim(lv_part_t part) const noexcept { return lv_obj_get_style_anim(ptr, part); }
	uint32_t getStyleAnimDuration(lv_part_t part) const noexcept { return lv_obj_get_style_anim_duration(ptr, part); }
	const lv_style_transition_dsc_t* getStyleTransition(lv_part_t part) const noexcept { return lv_obj_get_style_transition(ptr, part); }
	lv_blend_mode_t getStyleBlendMode(lv_part_t part) const noexcept { return lv_obj_get_style_blend_mode(ptr, part); }
	uint16_t getStyleLayout(lv_part_t part) const noexcept { return lv_obj_get_style_layout(ptr, part); }
	lv_base_dir_t getStyleBaseDir(lv_part_t part) const noexcept { return lv_obj_get_style_base_dir(ptr, part); }
	const void* getStyleBitmapMaskSrc(lv_part_t part) const noexcept { return lv_obj_get_style_bitmap_mask_src(ptr, part); }
	uint32_t getStyleRotarySensitivity(lv_part_t part) const noexcept { return lv_obj_get_style_rotary_sensitivity(ptr, part); }
#if LV_USE_FLEX
	lv_flex_flow_t getStyleFlexFlow(lv_part_t part) const noexcept { return lv_obj_get_style_flex_flow(ptr, part); }
	lv_flex_align_t getStyleFlexMainPlace(lv_part_t part) const noexcept { return lv_obj_get_style_flex_main_place(ptr, part); }
	lv_flex_align_t getStyleFlexCrossPlace(lv_part_t part) const noexcept { return lv_obj_get_style_flex_cross_place(ptr, part); }
	lv_flex_align_t getStyleFlexTrackPlace(lv_part_t part) const noexcept { return lv_obj_get_style_flex_track_place(ptr, part); }
	uint8_t getStyleFlexGrow(lv_part_t part) const noexcept { return lv_obj_get_style_flex_grow(ptr, part); }
#endif // LV_USE_FLEX
#if LV_USE_GRID
	const int32_t* getStyleGridColumnDscArray(lv_part_t part) const noexcept { return lv_obj_get_style_grid_column_dsc_array(ptr, part); }
	lv_grid_align_t getStyleGridColumnAlign(lv_part_t part) const noexcept { return lv_obj_get_style_grid_column_align(ptr, part); }
	const int32_t* getStyleGridRowDscArray(lv_part_t part) const noexcept { return lv_obj_get_style_grid_row_dsc_array(ptr, part); }
	lv_grid_align_t getStyleGridRowAlign(lv_part_t part) const noexcept { return lv_obj_get_style_grid_row_align(ptr, part); }
	int32_t getStyleGridCellColumnPos(lv_part_t part) const noexcept { return lv_obj_get_style_grid_cell_column_pos(ptr, part); }
	lv_grid_align_t getStyleGridCellXAlign(lv_part_t part) const noexcept { return lv_obj_get_style_grid_cell_x_align(ptr, part); }
	int32_t getStyleGridCellColumnSpan(lv_part_t part) const noexcept { return lv_obj_get_style_grid_cell_column_span(ptr, part); }
	int32_t getStyleGridCellRowPos(lv_part_t part) const noexcept { return lv_obj_get_style_grid_cell_row_pos(ptr, part); }
	lv_grid_align_t getStyleGridCellYAlign(lv_part_t part) const noexcept { return lv_obj_get_style_grid_cell_y_align(ptr, part); }
	int32_t getStyleGridCellRowSpan(lv_part_t part) const noexcept { return lv_obj_get_style_grid_cell_row_span(ptr, part); }
#endif // LV_USE_GRID
	// style_gen setters
	void setStyleWidth(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_width(ptr, value, selector); }
	void setStyleMinWidth(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_min_width(ptr, value, selector); }
	void setStyleMaxWidth(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_max_width(ptr, value, selector); }
	void setStyleHeight(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_height(ptr, value, selector); }
	void setStyleMinHeight(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_min_height(ptr, value, selector); }
	void setStyleMaxHeight(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_max_height(ptr, value, selector); }
	void setStyleLength(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_length(ptr, value, selector); }
	void setStyleX(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_x(ptr, value, selector); }
	void setStyleY(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_y(ptr, value, selector); }
	void setStyleAlign(lv_align_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_align(ptr, value, selector); }
	void setStyleTransformWidth(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_transform_width(ptr, value, selector); }
	void setStyleTransformHeight(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_transform_height(ptr, value, selector); }
	void setStyleTranslateX(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_translate_x(ptr, value, selector); }
	void setStyleTranslateY(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_translate_y(ptr, value, selector); }
	void setStyleTranslateRadial(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_translate_radial(ptr, value, selector); }
	void setStyleTransformScaleX(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_transform_scale_x(ptr, value, selector); }
	void setStyleTransformScaleY(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_transform_scale_y(ptr, value, selector); }
	void setStyleTransformRotation(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_transform_rotation(ptr, value, selector); }
	void setStyleTransformPivotX(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_transform_pivot_x(ptr, value, selector); }
	void setStyleTransformPivotY(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_transform_pivot_y(ptr, value, selector); }
	void setStyleTransformSkewX(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_transform_skew_x(ptr, value, selector); }
	void setStyleTransformSkewY(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_transform_skew_y(ptr, value, selector); }
	void setStylePadTop(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_pad_top(ptr, value, selector); }
	void setStylePadBottom(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_pad_bottom(ptr, value, selector); }
	void setStylePadLeft(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_pad_left(ptr, value, selector); }
	void setStylePadRight(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_pad_right(ptr, value, selector); }
	void setStylePadRow(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_pad_row(ptr, value, selector); }
	void setStylePadColumn(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_pad_column(ptr, value, selector); }
	void setStylePadRadial(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_pad_radial(ptr, value, selector); }
	void setStyleMarginTop(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_margin_top(ptr, value, selector); }
	void setStyleMarginBottom(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_margin_bottom(ptr, value, selector); }
	void setStyleMarginLeft(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_margin_left(ptr, value, selector); }
	void setStyleMarginRight(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_margin_right(ptr, value, selector); }
	void setStyleBgColor(lv_color_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_bg_color(ptr, value, selector); }
	void setStyleBgOpa(lv_opa_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_bg_opa(ptr, value, selector); }
	void setStyleBgGradColor(lv_color_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_bg_grad_color(ptr, value, selector); }
	void setStyleBgGradDir(lv_grad_dir_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_bg_grad_dir(ptr, value, selector); }
	void setStyleBgMainStop(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_bg_main_stop(ptr, value, selector); }
	void setStyleBgGradStop(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_bg_grad_stop(ptr, value, selector); }
	void setStyleBgMainOpa(lv_opa_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_bg_main_opa(ptr, value, selector); }
	void setStyleBgGradOpa(lv_opa_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_bg_grad_opa(ptr, value, selector); }
	void setStyleBgGrad(const lv_grad_dsc_t* value, lv_style_selector_t selector) noexcept { lv_obj_set_style_bg_grad(ptr, value, selector); }
	void setStyleBgImageSrc(const void* value, lv_style_selector_t selector) noexcept { lv_obj_set_style_bg_image_src(ptr, value, selector); }
	void setStyleBgImageOpa(lv_opa_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_bg_image_opa(ptr, value, selector); }
	void setStyleBgImageRecolor(lv_color_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_bg_image_recolor(ptr, value, selector); }
	void setStyleBgImageRecolorOpa(lv_opa_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_bg_image_recolor_opa(ptr, value, selector); }
	void setStyleBgImageTiled(bool value, lv_style_selector_t selector) noexcept { lv_obj_set_style_bg_image_tiled(ptr, value, selector); }
	void setStyleBorderColor(lv_color_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_border_color(ptr, value, selector); }
	void setStyleBorderOpa(lv_opa_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_border_opa(ptr, value, selector); }
	void setStyleBorderWidth(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_border_width(ptr, value, selector); }
	void setStyleBorderSide(lv_border_side_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_border_side(ptr, value, selector); }
	void setStyleBorderPost(bool value, lv_style_selector_t selector) noexcept { lv_obj_set_style_border_post(ptr, value, selector); }
	void setStyleOutlineWidth(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_outline_width(ptr, value, selector); }
	void setStyleOutlineColor(lv_color_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_outline_color(ptr, value, selector); }
	void setStyleOutlineOpa(lv_opa_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_outline_opa(ptr, value, selector); }
	void setStyleOutlinePad(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_outline_pad(ptr, value, selector); }
	void setStyleShadowWidth(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_shadow_width(ptr, value, selector); }
	void setStyleShadowOffsetX(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_shadow_offset_x(ptr, value, selector); }
	void setStyleShadowOffsetY(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_shadow_offset_y(ptr, value, selector); }
	void setStyleShadowSpread(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_shadow_spread(ptr, value, selector); }
	void setStyleShadowColor(lv_color_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_shadow_color(ptr, value, selector); }
	void setStyleShadowOpa(lv_opa_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_shadow_opa(ptr, value, selector); }
	void setStyleImageOpa(lv_opa_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_image_opa(ptr, value, selector); }
	void setStyleImageRecolor(lv_color_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_image_recolor(ptr, value, selector); }
	void setStyleImageRecolorOpa(lv_opa_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_image_recolor_opa(ptr, value, selector); }
	void setStyleImageColorkey(const lv_image_colorkey_t* value, lv_style_selector_t selector) noexcept { lv_obj_set_style_image_colorkey(ptr, value, selector); }
	void setStyleLineWidth(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_line_width(ptr, value, selector); }
	void setStyleLineDashWidth(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_line_dash_width(ptr, value, selector); }
	void setStyleLineDashGap(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_line_dash_gap(ptr, value, selector); }
	void setStyleLineRounded(bool value, lv_style_selector_t selector) noexcept { lv_obj_set_style_line_rounded(ptr, value, selector); }
	void setStyleLineColor(lv_color_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_line_color(ptr, value, selector); }
	void setStyleLineOpa(lv_opa_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_line_opa(ptr, value, selector); }
	void setStyleArcWidth(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_arc_width(ptr, value, selector); }
	void setStyleArcRounded(bool value, lv_style_selector_t selector) noexcept { lv_obj_set_style_arc_rounded(ptr, value, selector); }
	void setStyleArcColor(lv_color_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_arc_color(ptr, value, selector); }
	void setStyleArcOpa(lv_opa_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_arc_opa(ptr, value, selector); }
	void setStyleArcImageSrc(const void* value, lv_style_selector_t selector) noexcept { lv_obj_set_style_arc_image_src(ptr, value, selector); }
	void setStyleTextColor(lv_color_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_text_color(ptr, value, selector); }
	void setStyleTextOpa(lv_opa_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_text_opa(ptr, value, selector); }
	void setStyleTextFont(const lv_font_t* value, lv_style_selector_t selector) noexcept { lv_obj_set_style_text_font(ptr, value, selector); }
	void setStyleTextLetterSpace(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_text_letter_space(ptr, value, selector); }
	void setStyleTextLineSpace(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_text_line_space(ptr, value, selector); }
	void setStyleTextDecor(lv_text_decor_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_text_decor(ptr, value, selector); }
	void setStyleTextAlign(lv_text_align_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_text_align(ptr, value, selector); }
	void setStyleTextOutlineStrokeColor(lv_color_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_text_outline_stroke_color(ptr, value, selector); }
	void setStyleTextOutlineStrokeWidth(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_text_outline_stroke_width(ptr, value, selector); }
	void setStyleTextOutlineStrokeOpa(lv_opa_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_text_outline_stroke_opa(ptr, value, selector); }
	void setStyleRadius(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_radius(ptr, value, selector); }
	void setStyleRadialOffset(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_radial_offset(ptr, value, selector); }
	void setStyleClipCorner(bool value, lv_style_selector_t selector) noexcept { lv_obj_set_style_clip_corner(ptr, value, selector); }
	void setStyleOpa(lv_opa_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_opa(ptr, value, selector); }
	void setStyleOpaLayered(lv_opa_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_opa_layered(ptr, value, selector); }
	void setStyleColorFilterDsc(const lv_color_filter_dsc_t* value, lv_style_selector_t selector) noexcept { lv_obj_set_style_color_filter_dsc(ptr, value, selector); }
	void setStyleColorFilterOpa(lv_opa_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_color_filter_opa(ptr, value, selector); }
	void setStyleRecolor(lv_color_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_recolor(ptr, value, selector); }
	void setStyleRecolorOpa(lv_opa_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_recolor_opa(ptr, value, selector); }
	void setStyleAnim(const lv_anim_t* value, lv_style_selector_t selector) noexcept { lv_obj_set_style_anim(ptr, value, selector); }
	void setStyleAnimDuration(uint32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_anim_duration(ptr, value, selector); }
	void setStyleTransition(const lv_style_transition_dsc_t* value, lv_style_selector_t selector) noexcept { lv_obj_set_style_transition(ptr, value, selector); }
	void setStyleBlendMode(lv_blend_mode_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_blend_mode(ptr, value, selector); }
	void setStyleLayout(uint16_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_layout(ptr, value, selector); }
	void setStyleBaseDir(lv_base_dir_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_base_dir(ptr, value, selector); }
	void setStyleBitmapMaskSrc(const void* value, lv_style_selector_t selector) noexcept { lv_obj_set_style_bitmap_mask_src(ptr, value, selector); }
	void setStyleRotarySensitivity(uint32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_rotary_sensitivity(ptr, value, selector); }
#if LV_USE_FLEX
	void setStyleFlexFlow(lv_flex_flow_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_flex_flow(ptr, value, selector); }
	void setStyleFlexMainPlace(lv_flex_align_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_flex_main_place(ptr, value, selector); }
	void setStyleFlexCrossPlace(lv_flex_align_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_flex_cross_place(ptr, value, selector); }
	void setStyleFlexTrackPlace(lv_flex_align_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_flex_track_place(ptr, value, selector); }
	void setStyleFlexGrow(uint8_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_flex_grow(ptr, value, selector); }
	// lv_flax.h
	void setFlexFlow(lv_flex_flow_t flow) noexcept { lv_obj_set_flex_flow(ptr, flow); }
	void setFlexAlign(lv_flex_align_t main_place, lv_flex_align_t cross_place, lv_flex_align_t track_cross_place) noexcept { lv_obj_set_flex_align(ptr, main_place, cross_place, track_cross_place); }
	void setFlexGrow(uint8_t grow) noexcept { lv_obj_set_flex_grow(ptr, grow); }
#endif // LV_USE_FLEX
#if LV_USE_GRID
	void setStyleGridColumnDscArray(const int32_t* value, lv_style_selector_t selector) noexcept { lv_obj_set_style_grid_column_dsc_array(ptr, value, selector); }
	void setStyleGridColumnAlign(lv_grid_align_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_grid_column_align(ptr, value, selector); }
	void setStyleGridRowDscArray(const int32_t* value, lv_style_selector_t selector) noexcept { lv_obj_set_style_grid_row_dsc_array(ptr, value, selector); }
	void setStyleGridRowAlign(lv_grid_align_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_grid_row_align(ptr, value, selector); }
	void setStyleGridCellColumnPos(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_grid_cell_column_pos(ptr, value, selector); }
	void setStyleGridCellXAlign(lv_grid_align_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_grid_cell_x_align(ptr, value, selector); }
	void setStyleGridCellColumnSpan(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_grid_cell_column_span(ptr, value, selector); }
	void setStyleGridCellRowPos(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_grid_cell_row_pos(ptr, value, selector); }
	void setStyleGridCellYAlign(lv_grid_align_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_grid_cell_y_align(ptr, value, selector); }
	void setStyleGridCellRowSpan(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_grid_cell_row_span(ptr, value, selector); }
	// lv_grid.h
	void setGridDscArray(const int32_t col_dsc[], const int32_t row_dsc[]) noexcept { lv_obj_set_grid_dsc_array(ptr, col_dsc, row_dsc); }
	void setGridAlign(lv_grid_align_t column_align, lv_grid_align_t row_align) noexcept { lv_obj_set_grid_align(ptr, column_align, row_align); }
	void setGridCell(lv_grid_align_t column_align, int32_t col_pos, int32_t col_span, lv_grid_align_t row_align, int32_t row_pos, int32_t row_span) noexcept { lv_obj_set_grid_cell(ptr, column_align, col_pos, col_span, row_align, row_pos, row_span); }
#endif // LV_USE_GRID
	// lv_style.h
	lv_style_value_t styleApplyColorFilter(lv_part_t part, lv_style_value_t v) noexcept { return lv_obj_style_apply_color_filter(ptr, part, v); }
	void setStylePadAll(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_pad_all(ptr, value, selector); }
	void setStylePadHor(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_pad_hor(ptr, value, selector); }
	void setStylePadVer(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_pad_ver(ptr, value, selector); }
	void setStyleMarginAll(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_margin_all(ptr, value, selector); }
	void setStyleMarginHor(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_margin_hor(ptr, value, selector); }
	void setStyleMarginVer(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_margin_ver(ptr, value, selector); }
	void setStylePadGap(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_pad_gap(ptr, value, selector); }
	void setStyleSize(int32_t width, int32_t height, lv_style_selector_t selector) noexcept { lv_obj_set_style_size(ptr, width, height, selector); }
	void setStyleTransformScale(int32_t value, lv_style_selector_t selector) noexcept { lv_obj_set_style_transform_scale(ptr, value, selector); }
	int32_t getStyleSpaceLeft(lv_part_t part) const noexcept { return lv_obj_get_style_space_left(ptr, part); }
	int32_t getStyleSpaceRight(lv_part_t part) const noexcept { return lv_obj_get_style_space_right(ptr, part); }
	int32_t getStyleSpaceTop(lv_part_t part) const noexcept { return lv_obj_get_style_space_top(ptr, part); }
	int32_t getStyleSpaceBottom(lv_part_t part) const noexcept { return lv_obj_get_style_space_bottom(ptr, part); }
	lv_text_align_t calculateStyleTextAlign(lv_part_t part, const char* txt) const noexcept { return lv_obj_calculate_style_text_align(ptr, part, txt); }
	int32_t getStyleTransformScaleXSafe(lv_part_t part) const noexcept { return lv_obj_get_style_transform_scale_x_safe(ptr, part); }
	int32_t getStyleTransformScaleYSafe(lv_part_t part) const noexcept { return lv_obj_get_style_transform_scale_y_safe(ptr, part); }
//	lv_opa_t getStyleOpaRecursive(lv_part_t part) const noexcept { return lv_obj_get_style_opa_recursive(ptr, part); }
	lv_color32_t styleApplyRecolor(lv_part_t part, lv_color32_t color) noexcept { return lv_obj_style_apply_recolor(ptr, part, color); }
//	lv_color32_t getStyleRecolorRecursive(lv_part_t part) const noexcept { return lv_obj_get_style_recolor_recursive(ptr, part); }
	// extensions
	void setStylePadLRTB(int32_t l, int32_t r, int32_t t, int32_t b, lv_style_selector_t selector)
	{
		lv_obj_set_style_pad_left(ptr, l, selector);
		lv_obj_set_style_pad_right(ptr, r, selector);
		lv_obj_set_style_pad_top(ptr, t, selector);
		lv_obj_set_style_pad_bottom(ptr, b, selector);
	}
	void setStylePadColRow(int32_t c, int32_t r, lv_style_selector_t selector)
	{
		lv_obj_set_style_pad_column(ptr, c, selector);
		lv_obj_set_style_pad_row(ptr, r, selector);
	}
	bool isCheckable() const noexcept { return hasFlag(LV_OBJ_FLAG_CHECKABLE); }
	void setCheckable(bool v) noexcept { setFlag(LV_OBJ_FLAG_CHECKABLE, v); }
	bool isChecked() const noexcept { return hasState(LV_STATE_CHECKED); }
	void setCheck(bool v) noexcept { setState(LV_STATE_CHECKED, v); }
	bool isDisabled() const noexcept { return hasState(LV_STATE_DISABLED); }
	void setDisabled(bool v) noexcept { setState(LV_STATE_DISABLED, v); }
	bool isHidden() const noexcept { return hasFlag(LV_OBJ_FLAG_HIDDEN); }
	void setHidden(bool v) noexcept { setFlag(LV_OBJ_FLAG_HIDDEN, v); }
	// typed event callback
	// event receivers must implement the following methods:
	//   void onLvEventCallback(lv_event_t* e);
	template<typename T> static void typedEventHandler(lv_event_t* e) noexcept
	{
		if(T* p = static_cast<T*>(lv_event_get_user_data(e)))
		{
			p->onLvEventCallback(e);
		}
	}
	template<typename T> lv_event_dsc_t* addTypedEventCb(T* pt, lv_event_code_t filter)
	{
		return lv_obj_add_event_cb(ptr, typedEventHandler<T>, filter, pt);
	}
	template<typename T> uint32_t removeTypedEventCb(T* pt)
	{
		return lv_obj_remove_event_cb_with_user_data(ptr, typedEventHandler<T>, pt);
	}
};

#endif // defined(__cplusplus)
