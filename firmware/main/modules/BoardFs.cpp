//
//  BoardFs.cpp
//  SensorClock
//
//  created by yu2924 on 2026-01-25
//

#include "BoardFs.h"
#if __has_include("BoardFs.config.h")
#include "BoardFs.config.h"
#endif

#include <cstdio>

namespace BoardFs
{
	size_t loadStringFromFile(const char* path, HeapBufferT<char>* buf) noexcept
	{
		if(!buf) { return 0; }
		if(FILE* pf = fopen(path, "rb"))
		{
			fseek(pf, 0, SEEK_END);
			long len = ftell(pf);
			if(len < 0) { len = 0; }
			buf->resize(len + 1);
			fseek(pf, 0, SEEK_SET);
			fread(buf->data(), 1, len, pf);
			buf->data()[len] = 0;
			fclose(pf);
			return len;
		}
		else
		{
			buf->resize(1);
			buf->data()[0] = 0;
			return 0;
		}
	}
}

#if defined(_WINDOWS)

// ================================================================================
// Prototype code for Windows
// It treats the current directory of the process as the root of the file system.

#define STRICT
#define WIN32_LEAN_AND_MEAN
#if !defined NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#if __has_include(<lvgl.h>)
#include <lvgl.h>
#endif
#include "utilities/strutil.h"

namespace BoardFs
{
	const char* assetfsGetRoot() noexcept
	{
		static strutil::Str<MAX_PATH> dir;
		if(dir.isEmpty())
		{
			::GetCurrentDirectoryA((DWORD)dir.capacity(), dir.getBuffer());
			PathAppendA(dir.getBuffer(), "assets");
		}
		return dir;
	}
	const char* spifsGetRoot() noexcept
	{
		static strutil::Str<MAX_PATH> dir;
#if ENABLE_SPIFS
		if(dir.isEmpty())
		{
			::GetCurrentDirectoryA((DWORD)dir.capacity(), dir.getBuffer());
			PathAppendA(dir.getBuffer(), "spifs");
		}
#endif
		return dir;
	}
	const char* sdcardGetRoot() noexcept
	{
		static strutil::Str<MAX_PATH> dir;
		if(dir.isEmpty())
		{
			::GetCurrentDirectoryA((DWORD)dir.capacity(), dir.getBuffer());
			PathAppendA(dir.getBuffer(), "sdcard");
		}
		return dir;
	}
	bool sdcardMount() noexcept { return true; }
	void sdcardUnmount() noexcept {}
	bool sdcardIsMouted() noexcept { return true; }
	bool sdcardIsStatusGood() noexcept { return true; }
	bool sdcardEnsureMounted() noexcept { return true; }
	// asset support methods
	void resolveAssetFilePath(const char* filename, char* pabspath, size_t cabspath) noexcept
	{
		if(filename[0] == '/') { ++filename; }
		const char* pp[] = { assetfsGetRoot(), "/", filename};
		strutil::join(pabspath, cabspath, pp, std::size(pp));
	}
#if __has_include(<lvgl.h>)
	void resolveLvglAssetFilePath(const char* filename, char* pabspath, size_t cabspath) noexcept
	{
		resolveAssetFilePath(filename, pabspath, cabspath);
	}
#endif
} // namespace BoardFs

#elif defined(ESP_PLATFORM)

// ================================================================================
// Production code for ESP32

#include <esp_log.h>
#include <esp_partition.h>
#include <esp_vfs_fat.h>
#include <bsp/esp-bsp.h>
#include <frogfs/frogfs.h>
#include <frogfs/vfs.h>
#include <sd_pwr_ctrl_by_on_chip_ldo.h>
#include <sdmmc_cmd.h>
#if __has_include(<lvgl.h>)
#include <lvgl.h>
#endif
#include "utilities/strutil.h"

// NOTE: to use asset files with LVGL
// - set CONFIG_LV_USE_FS_FROGFS=y
// - set CONFIG_LV_FS_FROGFS_LETTER=87('W')
// - call lv_fs_frogfs_register_blob(frogfs_bin, "/assets"):
// then, pass absolute paths as "W:/assets/filename.ext"

#if USE_EMBEDDED_ASSETS
extern const uint8_t frogfs_bin[];
extern const size_t frogfs_bin_len;
#endif

namespace BoardFs
{

	static const char* TagBoardFs = "BoardFs";

	// ================================================================================
	// FrogFS for read-only assets

	static const char* AssetFsMountPoint = "/assets";
#if !USE_EMBEDDED_ASSETS
	static const char* AssetFsPartition = "assets";
#endif

	struct AssetfsInit
	{
		frogfs_fs_t* frogFs = nullptr;
		AssetfsInit()
		{
			frogfs_config_t cfg{};
#if USE_EMBEDDED_ASSETS
			cfg.addr = frogfs_bin;
#else
			cfg.part_label = AssetFsPartition;
#endif
			frogFs = frogfs_init(&cfg);
			if(frogFs)
			{
				frogfs_vfs_conf_t vfsconf{};
				vfsconf.base_path = AssetFsMountPoint;
				vfsconf.fs = frogFs;
				vfsconf.max_files = 5;
				frogfs_vfs_register(&vfsconf);
			}
			else
			{
				ESP_LOGW(TagBoardFs, "frogfs_init() failed, returned nullptr");
			}
#if __has_include(<lvgl.h>)
			if(lv_fs_frogfs_register_blob(frogfs_bin, AssetFsMountPoint) != LV_RESULT_OK)
			{
				ESP_LOGW(TagBoardFs, "lv_fs_frogfs_register_blob() failed");
			}
#endif
		}
		~AssetfsInit()
		{
			if(frogFs)
			{
				frogfs_vfs_deregister(AssetFsMountPoint);
				frogfs_deinit(frogFs);
			}
		}
		static void initOnce()
		{
			static xheap::uniquePtr<AssetfsInit> gInst;
			if(!gInst) { gInst = xheap::makeUnique<AssetfsInit>(); }
		}
		static const char* getMountPoint()
		{
			initOnce();
			return AssetFsMountPoint;
		}
	};

	// ================================================================================
	// SPI flash

	static const char* SpiFsMountPoint = CONFIG_BSP_SPIFFS_MOUNT_POINT;
#if ENABLE_SPIFS
	static const char* SpiFsPartition = CONFIG_BSP_SPIFFS_PARTITION_LABEL;
#endif

	struct SpifsInit
	{
#if ENABLE_SPIFS
		wl_handle_t wl_handle;
		SpifsInit()
		{
			esp_vfs_fat_mount_config_t cfg{};
			cfg.format_if_mount_failed = true;
			cfg.max_files = 4;
			esp_err_t r = esp_vfs_fat_spiflash_mount_rw_wl(SpiFsMountPoint, SpiFsPartition, &cfg, &wl_handle);
			if(r != ESP_OK) { ESP_LOGW(TagBoardFs, "esp_vfs_fat_spiflash_mount_rw_wl() failed, %s", esp_err_to_name(r)); }
		}
		~SpifsInit()
		{
			esp_vfs_fat_spiflash_unmount_rw_wl(SpiFsMountPoint, wl_handle);
		}
		static void initOnce()
		{
			static xheap::uniquePtr<SpifsInit> gInst;
			if(!gInst) { gInst = xheap::makeUnique<SpifsInit>(); }
		}
#else
		static void initOnce() {}
#endif
		static const char* getMountPoint()
		{
			initOnce();
			return SpiFsMountPoint;
		}
	};

	// ================================================================================
	// SD cards

	static const char* SdcardMountPoint = CONFIG_BSP_SD_MOUNT_POINT;

	// While it is largely based on the BSP implementation, the VFS mounting part and the LDO control part have been separated to allow for flexible control over mounting and unmounting.
	// cf. "esp32_p4_wifi6_touch_lcd_7b.c"
	struct SdcardMounter
	{
		struct SdPwrCtrl
		{
			sd_pwr_ctrl_handle_t pwrCtrlHandle = NULL;
			SdPwrCtrl() {}
			~SdPwrCtrl() { destroy(); }
			bool create()
			{
				if(pwrCtrlHandle) { return true; }
				sd_pwr_ctrl_ldo_config_t cfg =
				{
					.ldo_chan_id = 4,
				};
				esp_err_t r = sd_pwr_ctrl_new_on_chip_ldo(&cfg, &pwrCtrlHandle);
				if(r != ESP_OK) { ESP_LOGW(TagBoardFs, "sd_pwr_ctrl_new_on_chip_ldo() failed, %s", esp_err_to_name(r)); }
				else { ESP_LOGI(TagBoardFs, "sd_pwr_ctrl_new_on_chip_ldo() succeeded"); }
				return r == ESP_OK;
			}
			void destroy()
			{
				if(!pwrCtrlHandle) { return; }
				sd_pwr_ctrl_del_on_chip_ldo(pwrCtrlHandle);
				pwrCtrlHandle = NULL;
			}
			operator sd_pwr_ctrl_handle_t() { return pwrCtrlHandle; }
			bool isCreated() const { return pwrCtrlHandle != NULL; }
		} sdPwrCtrl;
		struct VfsFatSdmmcCard
		{
			sdmmc_card_t* sdmmcCard = NULL;
			VfsFatSdmmcCard() {}
			~VfsFatSdmmcCard() { unmount(); }
			bool mount(sd_pwr_ctrl_handle_t pwrctrlhandle)
			{
				if(sdmmcCard) { return true; }
				sdmmc_host_t host = SDMMC_HOST_DEFAULT();
				host.slot = SDMMC_HOST_SLOT_0;
				host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;
				host.pwr_ctrl_handle = pwrctrlhandle;
				// SD card is connected to Slot 0 pins. Slot 0 uses IO MUX, so not specifying the pins here
				sdmmc_slot_config_t slot_config = {};
				slot_config.cd = SDMMC_SLOT_NO_CD;
				slot_config.wp = SDMMC_SLOT_NO_WP;
				slot_config.width = 4;
				slot_config.flags = 0;
				esp_vfs_fat_sdmmc_mount_config_t mount_config = {};
				mount_config.format_if_mount_failed = false;
				mount_config.max_files = 5;
				mount_config.allocation_unit_size = 64 * 1024;
				esp_err_t r = esp_vfs_fat_sdmmc_mount(BSP_SD_MOUNT_POINT, &host, &slot_config, &mount_config, &sdmmcCard);
				if(r != ESP_OK) { ESP_LOGW(TagBoardFs, "esp_vfs_fat_sdmmc_mount() failed, %s", esp_err_to_name(r)); }
				else { ESP_LOGI(TagBoardFs, "esp_vfs_fat_sdmmc_mount() succeeded"); }
				return r == ESP_OK;
			}
			void unmount()
			{
				if(!sdmmcCard) { return; }
				esp_err_t r = esp_vfs_fat_sdcard_unmount(BSP_SD_MOUNT_POINT, sdmmcCard);
				if(r != ESP_OK) { ESP_LOGW(TagBoardFs, "esp_vfs_fat_sdcard_unmount() failed, %s", esp_err_to_name(r)); }
				else { ESP_LOGI(TagBoardFs, "esp_vfs_fat_sdcard_unmount() succeeded"); }
				sdmmcCard = NULL;
			}
			bool isMounted() const
			{
				return sdmmcCard != NULL;
			}
			bool isStatusGood() const
			{
				if(!sdmmcCard) { return false; }
				return sdmmc_get_status(sdmmcCard) == ESP_OK;
			}
		} vfsFatSdmmcCard;
		bool isMounted() const
		{
			return vfsFatSdmmcCard.isMounted();
		}
		bool mount()
		{
			if(!sdPwrCtrl.create()) { return false; }
			return vfsFatSdmmcCard.mount(sdPwrCtrl);
		}
		void unmount()
		{
			vfsFatSdmmcCard.unmount();
		}
		bool isStatusGood() const
		{
			return vfsFatSdmmcCard.isStatusGood();
		}
		bool ensureMounted()
		{
			if(!sdPwrCtrl.create()) { return false; }
			if(vfsFatSdmmcCard.isMounted() && !vfsFatSdmmcCard.isStatusGood()) { vfsFatSdmmcCard.unmount(); }
			vfsFatSdmmcCard.mount(sdPwrCtrl);
			return vfsFatSdmmcCard.isMounted() && vfsFatSdmmcCard.isStatusGood();
		}
		static SdcardMounter* getInstance()
		{
			static xheap::uniquePtr<SdcardMounter> gInst;
			if(!gInst) { gInst = xheap::makeUnique<SdcardMounter>(); }
			return gInst.get();
		}
		static const char* getMountPoint()
		{
			return SdcardMountPoint;
		}
	};

	// ================================================================================
	// public APIs

	const char* assetfsGetRoot() noexcept
	{
		return AssetfsInit::getMountPoint();
	}
	const char* spifsGetRoot() noexcept
	{
		return SpifsInit::getMountPoint();
	}
	const char* sdcardGetRoot() noexcept
	{
		return SdcardMounter::getMountPoint();
	}
	bool sdcardIsMouted() noexcept
	{
		return SdcardMounter::getInstance()->isMounted();
	}
	bool sdcardMount() noexcept
	{
		return SdcardMounter::getInstance()->mount();
	}
	void sdcardUnmount() noexcept
	{
		SdcardMounter::getInstance()->unmount();
	}
	bool sdcardIsStatusGood() noexcept
	{
		return SdcardMounter::getInstance()->isStatusGood();
	}
	bool sdcardEnsureMounted() noexcept
	{
		return SdcardMounter::getInstance()->ensureMounted();
	}

	// --------------------------------------------------------------------------------
	// asset support methods

	void resolveAssetFilePath(const char* filename, char* pabspath, size_t cabspath) noexcept
	{
		if(filename[0] == '/') { ++filename; }
		const char* pp[] = { assetfsGetRoot(), "/", filename };
		strutil::join(pabspath, cabspath, pp, std::size(pp));
	}

#if __has_include(<lvgl.h>)
	void resolveLvglAssetFilePath(const char* filename, char* pabspath, size_t cabspath) noexcept
	{
		if(3 <= cabspath)
		{
			const char prefix[] = { CONFIG_LV_FS_FROGFS_LETTER, ':', 0 };
			strcpy(pabspath, prefix);
			pabspath += 2;
			cabspath -= 2;
		}
		resolveAssetFilePath(filename, pabspath, cabspath);
	}
#endif

} // namespace BoardFs

#else
#error unknown platform
#endif // defined(_WINDOWS) / defined(ESP_PLATFORM)
