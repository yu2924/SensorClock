//
//  strutil.h
//  SensorClock
//
//  created by yu2924 on 2025-12-04
//

#pragma once

#include <cstdarg>
#include <cstring>
#include <iterator>

namespace strutil
{
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif
	static bool isEmpty(const char* src) noexcept
	{
		return !src || (*src == 0);
	}
	static bool areEqual(const char* a, const char* b) noexcept
	{
		return (a != nullptr) && (b != nullptr) && (std::strcmp(a, b) == 0);
	}
	static bool areEqualNoCase(const char* a, const char* b) noexcept
	{
#if defined(_WIN32)
		return (a != nullptr) && (b != nullptr) && (_stricmp(a, b) == 0);
#else
		return (a != nullptr) && (b != nullptr) && (strcasecmp(a, b) == 0);
#endif
	}
	static size_t length(const char* src) noexcept
	{
		return src ? std::strlen(src) : 0;
	}
	static size_t copy(char* buf, size_t buflen, const char* src) noexcept
	{
		if(!buflen) return 0;
		size_t l = std::min(buflen - 1, length(src));
		if(src) std::memcpy(buf, src, l);
		buf[l] = 0;
		return l;
	};
	static size_t concat(char* buf, size_t buflen, const char* src) noexcept
	{
		size_t off = length(buf);
		if(buflen <= off) return 0;
		return copy(buf + off, buflen - off, src);
	}
	static void join(char* buf, size_t buflen, const char* const* textlist, size_t numlist, const char* delim = nullptr) noexcept
	{
		size_t len = buflen, off = 0;
		for(size_t i = 0; i < numlist; ++i)
		{
			if(len <= (1 + off)) break;
			off += copy(buf + off, len - off, textlist[i]);
			if(len <= (1 + off)) break;
			if((i < (numlist - 1)) && delim && *delim) { off += copy(buf + off, len - off, delim); }
		}
	}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
	template<size_t L> struct Str
	{
		char buffer[L]{};
		Str() noexcept = default;
		Str(const char* p) noexcept { strutil::copy(buffer, L, p); }
		constexpr size_t capacity() const noexcept { return L; }
		bool isEmpty() const noexcept { return strutil::isEmpty(buffer); }
		size_t length() const noexcept { return strutil::length(buffer); }
		operator const char* () const noexcept { return buffer; }
		char operator[](size_t i) const noexcept { return (i < std::size(buffer)) ? buffer[i] : 0; }
		Str& operator=(const char* p) noexcept { strutil::copy(buffer, L, p); return *this; }
		Str& operator+=(const char* p) noexcept { strutil::concat(buffer, L, p); return *this; }
		Str& operator<<(const char* p) noexcept { strutil::concat(buffer, L, p); return *this; }
		Str operator+(const char* p) const noexcept { Str out = *this; out += p; return out; }
		bool operator==(const char* p) const noexcept { return areEqual(buffer, p); }
		bool operator!=(const char* p) const noexcept { return !areEqual(buffer, p); }
		char* getBuffer() noexcept { return buffer; }
		void clear() { memset(buffer, 0, std::size(buffer)); }
		void join(const char* const* textlist, size_t numlist, const char* delim = nullptr) noexcept { strutil::join(buffer, L, textlist, numlist, delim); }
		int format(const char* fmt, ...) noexcept { va_list va; va_start(va, fmt); int lr = vsnprintf(buffer, std::size(buffer), fmt, va); va_end(va); return lr; }
		int addFormat(const char* fmt, ...) noexcept { size_t len = length(); va_list va; va_start(va, fmt); int lr = vsnprintf(buffer + len, std::size(buffer) - len, fmt, va); va_end(va); return lr; }
		static Str formatted(const char* fmt, ...) noexcept { char s[L]; va_list va; va_start(va, fmt); vsnprintf(s, std::size(s), fmt, va); va_end(va); return s; }
	};
} // namespace strutil
