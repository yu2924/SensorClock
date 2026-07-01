//
//  BoardFs.h
//  SensorClock
//
//  created by yu2924 on 2026-01-25
//

#pragma once

#include "utilities/heapbuffer.h"

namespace BoardFs
{
	// string loader methods
	size_t loadStringFromFile(const char* path, HeapBufferT<char>* buf) noexcept;
	// file systems
	const char* assetfsGetRoot() noexcept;
	const char* spifsGetRoot() noexcept;
	const char* sdcardGetRoot() noexcept;
	bool sdcardMount() noexcept;
	void sdcardUnmount() noexcept;
	bool sdcardIsMouted() noexcept;
	bool sdcardIsStatusGood() noexcept;
	bool sdcardEnsureMounted() noexcept;
	// asset support methods
	void resolveAssetFilePath(const char* filename, char* pabspath, size_t cabspath) noexcept;
#if __has_include(<lvgl.h>)
	void resolveLvglAssetFilePath(const char* filename, char* pabspath, size_t cabspath) noexcept;
#endif
} // namespace BoardFs
