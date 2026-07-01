//
//  LvTinyTtf.h
//
//  created by yu2924 on 2025-11-22
//  This file was generated mostly by GitHub Copilot and partially edited manually.
//

#pragma once

#include "LvObj.h"

#if !LV_USE_TINY_TTF
#error LV_USE_TINY_TTF=1 required
#endif

#if defined(__cplusplus)

struct LvTinyTtf
{
	lv_font_t* font = nullptr;
	constexpr LvTinyTtf() noexcept = default;
	constexpr LvTinyTtf(lv_font_t* p) noexcept : font(p) {}
	constexpr LvTinyTtf(std::nullptr_t) noexcept : font(nullptr) {}
	constexpr operator lv_font_t* () const noexcept { return font; }
	explicit constexpr operator bool() const noexcept { return font != nullptr; }
#if LV_TINY_TTF_FILE_SUPPORT
	static LvTinyTtf createFile(const char* path, int32_t font_size) noexcept { return lv_tiny_ttf_create_file(path, font_size); }
	static LvTinyTtf createFileEx(const char* path, int32_t font_size, lv_font_kerning_t kerning, size_t cache_size) noexcept { return lv_tiny_ttf_create_file_ex(path, font_size, kerning, cache_size); }
#endif
	static LvTinyTtf createData(const void* data, size_t data_size, int32_t font_size) noexcept { return lv_tiny_ttf_create_data(data, data_size, font_size); }
	static LvTinyTtf createDataEx(const void* data, size_t data_size, int32_t font_size, lv_font_kerning_t kerning, size_t cache_size) noexcept { return lv_tiny_ttf_create_data_ex(data, data_size, font_size, kerning, cache_size); }
	void setSize(int32_t font_size) noexcept { lv_tiny_ttf_set_size(font, font_size); }
	void destroy() noexcept { if(font) lv_tiny_ttf_destroy(font); font = nullptr; }
};

struct LvTinyTtfOwning : public LvTinyTtf
{
	LvTinyTtfOwning() noexcept = default;
	LvTinyTtfOwning(lv_font_t* p) : LvTinyTtf(p) {}
	LvTinyTtfOwning& operator=(lv_font_t* p) { destroy(); font = p; return *this; }
	~LvTinyTtfOwning() { destroy(); }
};

#endif // defined(__cplusplus)
