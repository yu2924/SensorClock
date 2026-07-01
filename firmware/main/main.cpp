//
//  main.cpp
//  SensorClock
//
//  created by yu2924 on 2025-11-16
//

#if defined(_WINDOWS)

// ================================================================================
// Prototype code for Windows

#define STRICT
#if !defined NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <crtdbg.h>
#include <LvglWindowsIconResource.h>
#include <lvgl.h>
#include "modules/AppController.h"
#include "modules/Dispatcher.h"
#include "gui/GuiInit.h"

struct ScopedCoinitialize
{
	ScopedCoinitialize() { if(FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED))) { OutputDebugStringW(L"CoInitializeEx() failed\n"); } }
	~ScopedCoinitialize() { CoUninitialize(); }
};

struct ScopedWsaInit
{
	ScopedWsaInit() { WSADATA wsaData; if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) { OutputDebugStringW(L"WSAStartup() failed\n"); } }
	~ScopedWsaInit() { WSACleanup(); }
};

static void lvLogPrintCb(lv_log_level_t level, const char* buf)
{
	char s[1024]; snprintf(s, std::size(s), "%s\n", buf);
	OutputDebugStringA(s);
}

struct AppContainer
{
	xheap::uniquePtr<AppController> appController = AppController::createInstance();
	xheap::uniquePtr<gui::GuiInit> guiInit = gui::GuiInit::createInstance(appController.get());
	AppContainer()
	{
	}
	void startup()
	{
		guiInit->create();
		appController->activate();
	}
};

xheap::uniquePtr<AppContainer> gAppContainer;

static void __cdecl OnExitProc()
{
	OutputDebugStringW(L"OnExitProc\n");
	if(gAppContainer) gAppContainer.reset();
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int)
{
	ScopedCoinitialize coinit;
	ScopedWsaInit wsaInit;
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	lv_init();
	lv_log_register_print_cb(lvLogPrintCb);
	lv_display_t* display = lv_windows_create_display(L"SensorClock Prototype", gui::getScreenWidth(), gui::getScreenHeight(), 100, false, false);
	if(!display) { return -1; }
	HWND hwnd = lv_windows_get_display_window_handle(display);
	if(!hwnd) { return -1; }
	lv_indev_t* pointer_indev = lv_windows_acquire_pointer_indev(display);
	if(!pointer_indev) { return -1; }
	lv_indev_t* keypad_indev = lv_windows_acquire_keypad_indev(display);
	if(!keypad_indev) { return -1; }
	lv_indev_t* encoder_indev = lv_windows_acquire_encoder_indev(display);
	if(!encoder_indev) { return -1; }
	if(HICON hicon = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_LVGL_WINDOWS)))
	{
		SendMessageW(hwnd, WM_SETICON, TRUE, (LPARAM)hicon);
		SendMessageW(hwnd, WM_SETICON, FALSE, (LPARAM)hicon);
	}
	atexit(OnExitProc);
	Dispatcher* disp = Dispatcher::getMainDispatcher();
	gAppContainer = xheap::makeUnique<AppContainer>();
	gAppContainer->startup();
	disp->runEventLoop();
	return 0;
}

#elif defined(ESP_PLATFORM)

// ================================================================================
// Production code for ESP32

#include <esp_err.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <i2cdev.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <lvgl.h>

#include <bsp/display.h>
#include <bsp/esp-bsp.h>

#include "utilities/cjsonwrap.h"
#include "utilities/platform.h"
#include "modules/AppController.h"
#include "modules/Dispatcher.h"
#include "gui/GuiInit.h"

static const char* TagApp = "App";
/*
struct ScopedDisplayLock
{
	ScopedDisplayLock(uint32_t t = -1) { bsp_display_lock(t); }
	~ScopedDisplayLock() { bsp_display_unlock(); }
};
*/

static void lvLogPrintCb(lv_log_level_t lvlev, const char* buf)
{
	static const char* TagLvga = "LVGA";
	int esplev = ESP_LOG_NONE;
	char levchr = 'I';
	switch(lvlev)
	{
		case LV_LOG_LEVEL_TRACE	: esplev = ESP_LOG_VERBOSE	; levchr = 'V'; break;
		case LV_LOG_LEVEL_INFO	: esplev = ESP_LOG_INFO		; levchr = 'I'; break;
		case LV_LOG_LEVEL_WARN	: esplev = ESP_LOG_WARN		; levchr = 'W'; break;
		case LV_LOG_LEVEL_ERROR	: esplev = ESP_LOG_ERROR	; levchr = 'E'; break;
		case LV_LOG_LEVEL_USER	: esplev = ESP_LOG_DEBUG	; levchr = 'D'; break;
	}
	esp_log(ESP_LOG_CONFIG_INIT(esplev), TagLvga, "%c (%ld) %s: ", levchr, esp_log_timestamp(), TagLvga);
	printf("%s\n", buf);
}

void debugPrintHeapStat()
{
	static const struct { uint32_t cap; const char* label; } Table[] =
	{
		{ MALLOC_CAP_SPIRAM		, "spi" },
		{ MALLOC_CAP_INTERNAL	, "int" },
		{ MALLOC_CAP_DMA		, "dma" },
		{ MALLOC_CAP_DEFAULT	, "def" },
	};
	printf("%-6s %8s %8s %8s\n", "caps", "allfree", "largest", "total");
	for(const auto& ent : Table)
	{
		size_t allfree = heap_caps_get_free_size(ent.cap);
		size_t largest = heap_caps_get_largest_free_block(ent.cap);
		size_t total   = heap_caps_get_total_size(ent.cap);
		printf("%-6s %8u %8u %8u\n", ent.label, allfree, largest, total);
	}
}

// custom allocator using PSRAM for LVGL
// cf. https://github.com/lvgl/lvgl/issues/8191
extern "C"
{
	void lv_mem_init(void) {}
	void lv_mem_deinit(void) {}
	lv_mem_pool_t lv_mem_add_pool(void*, size_t) { return NULL; }
	void lv_mem_remove_pool(lv_mem_pool_t) {}
	void* lv_malloc_core(size_t size) { return heap_caps_malloc(size, MALLOC_CAP_SPIRAM); }
	void* lv_realloc_core(void* p, size_t new_size) { return heap_caps_realloc(p, new_size, MALLOC_CAP_SPIRAM); }
	void lv_free_core(void* p) { heap_caps_free(p); }
	void lv_mem_monitor_core(lv_mem_monitor_t*) {}
	lv_result_t lv_mem_test_core(void) { return LV_RESULT_OK; }
}

// --------------------------------------------------------------------------------

struct AppContainer
{
	xheap::uniquePtr<AppController> appController;
	xheap::uniquePtr<gui::GuiInit> guiInit;
	void startup()
	{
		ESP_LOGI(TagApp, "AppContainer startup begin");
		appController = AppController::createInstance();
		ESP_LOGI(TagApp, "AppController construction done");
		guiInit = gui::GuiInit::createInstance(appController.get());
		guiInit->create();
		bsp_display_backlight_on();
		ESP_LOGI(TagApp, "gui creation done");
		appController->activate();
		ESP_LOGI(TagApp, "AppController activation done");
		debugPrintHeapStat();
		ESP_LOGI(TagApp, "AppContainer startup end");
	}
};

xheap::uniquePtr<AppContainer> gAppContainer;

extern "C" void app_main(void)
{
	// bsp
	// NOTE: bsp_display_start_with_config() includes the following steps:
	//   setup both the lcd and the touch devices
	//   call lvgl_port_init() that create the its own LVGL task, call lv_init() within the task
	bsp_display_cfg_t cfg =
	{
		.lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
		.buffer_size = BSP_LCD_DRAW_BUFF_SIZE,
		.double_buffer = BSP_LCD_DRAW_BUFF_DOUBLE,
		.flags =
		{
			.buff_dma = true,
			.buff_spiram = true,
			.sw_rotate = true,
		}
	};
	if(lv_display_t* display = bsp_display_start_with_config(&cfg))
	{
		// bsp_display_backlight_on();
		bsp_display_rotate(display, LV_DISPLAY_ROTATION_180);
	}
	// ----------------------------------------
	// idf
	esp_err_t r = nvs_flash_init();
	if((r == ESP_ERR_NVS_NO_FREE_PAGES) || (r == ESP_ERR_NVS_NEW_VERSION_FOUND))
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		r = nvs_flash_init();
	}
	ESP_ERROR_CHECK(r);
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	// ----------------------------------------
	// app
	lv_log_register_print_cb(lvLogPrintCb);
	cjsonwrap::initWithHeapAllocators();
	WifiConnection::platformInit();
	SensorTask::platformInit();
	AlarmSounder::platformInit();
	Dispatcher* disp = Dispatcher::getMainDispatcher();
	gAppContainer = xheap::makeUnique<AppContainer>();
	gAppContainer->startup();
	disp->runEventLoop();
}

#else
#error unknown platform
#endif // defined(_WINDOWS) / defined(ESP_PLATFORM)
