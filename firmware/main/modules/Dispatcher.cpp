//
//  Dispatcher.cpp
//  SensorClock
//
//  created by yu2924 on 2025-11-23
//

#include "Dispatcher.h"

#if defined(_WINDOWS)

// ================================================================================
// Prototype code for Windows
// Unlike the production code for the ESP32, the prototype version of the event loop dispatcher does not create its own thread.

#define STRICT
#define WIN32_LEAN_AND_MEAN
#if !defined NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <map>
#include <memory>
#include <lvgl.h>
#include "utilities/listenerlist.h"

static bool hasBeenShutdown = false;

struct Mutex : public CRITICAL_SECTION
{
	Mutex() { InitializeCriticalSection(this); }
	~Mutex() { DeleteCriticalSection(this); }
	void lock() { EnterCriticalSection(this); }
	void unlock() { LeaveCriticalSection(this); }
};

struct ScopedLock
{
	Mutex& m;
	ScopedLock() = delete;
	ScopedLock(Mutex& rm) : m(rm) { m.lock(); }
	~ScopedLock() { m.unlock(); }
};

class DispatchWindow
{
public:
	static constexpr UINT WM_DW_INVOKEASYNC = WM_USER + 128;
	HWND hWnd = NULL;
	Mutex mutex;
	std::list<std::function<void()>> pendingQueue;
	std::unordered_map<int, std::function<void()>> timerList;
	DispatchWindow()
	{
		OutputDebugStringW(L"DispatchWindow construct\n");
		HINSTANCE hinst = ::GetModuleHandleW(nullptr);
		static const WCHAR WindowName[] = L"Dispatcher";
		WNDCLASSEX wcex{};
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = 0;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(void*);
		wcex.hInstance = hinst;
		wcex.hIcon = NULL;
		wcex.hCursor = NULL;
		wcex.hbrBackground = 0;
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = WindowName;
		::RegisterClassExW(&wcex);
		hWnd = ::CreateWindowExW(0, WindowName, WindowName, WS_CHILD, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_MESSAGE, NULL, hinst, NULL);
		if(hWnd) { ::SetWindowLongPtrW(hWnd, 0, (LONG_PTR)this); }
		else { OutputDebugStringW(L"[Dispatcher] WARNING: CreateWindowExW() failed\n"); }
	}
	~DispatchWindow()
	{
		hasBeenShutdown = true;
		pendingQueue.clear();
		::DestroyWindow(hWnd);
		OutputDebugStringW(L"DispatchWindow destruct\n");
	}
	static LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l)
	{
		switch(m)
		{
			case WM_DW_INVOKEASYNC:
				if(DispatchWindow* p = (DispatchWindow*)::GetWindowLongPtrW(h, 0))
				{
					p->processPendings();
				}
				return 0;
			case WM_TIMER:
				if(DispatchWindow* p = (DispatchWindow*)::GetWindowLongPtrW(h, 0))
				{
					int idt = (int)w;
					auto it = p->timerList.find(idt);
					if(it != p->timerList.end()) { it->second(); }
				}
				return 0;
			default: break;
		}
		return DefWindowProcW(h, m, w, l);
	}
	void processPendings()
	{
		while(1)
		{
			std::function<void()> cb;
			{
				ScopedLock sl(mutex);
				if(pendingQueue.empty()) return;
				cb = pendingQueue.front();
				pendingQueue.pop_front();
			}
			if(cb) cb();
		}

		auto it = pendingQueue.begin();
		while(it != pendingQueue.end())
		{
			std::function<void()> cb;
			{
				ScopedLock sl(mutex);
				cb = *it;
			}
			it = pendingQueue.erase(it);
			if(cb) cb();
		}
	}
	int findNextIdt()
	{
		for(int idt = 1; idt < 65536; ++idt)
		{
			if(timerList.find(idt) == timerList.end()) return idt;
		}
		return 0;
	}
	void callAsync(std::function<void()> proc)
	{
		ScopedLock sl(mutex);
		pendingQueue.push_back(proc);
		if(pendingQueue.size() == 1) PostMessageW(hWnd, WM_DW_INVOKEASYNC, 0, 0);
	}
	void runEventLoop()
	{
		while(true)
		{
			MSG msg; while(PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) { DispatchMessageW(&msg); }
			uint32_t time_till_next = ::lv_timer_handler();
			::lv_delay_ms(time_till_next);
		}
	}
	int setTimer(int tms, std::function<void()> proc)
	{
		int idt = findNextIdt();
		if(idt <= 0) return 0;
		timerList.insert({ idt, proc });
		::SetTimer(hWnd, idt, tms, NULL);
		return idt;
	}
	void killTimer(int idt)
	{
		if(idt <= 0) return;
		::KillTimer(hWnd, idt);
		timerList.erase(idt);
	}
	static DispatchWindow& getGlobalDispatchWindow()
	{
		static xheap::uniquePtr<DispatchWindow> gDispatchWindow;
		if(!gDispatchWindow) gDispatchWindow = xheap::makeUnique<DispatchWindow>();
		return *gDispatchWindow;
	}
};

class DispatcherImpl : public Dispatcher
{
public:
	~DispatcherImpl() override {}
	void callAsync(std::function<void()> proc) override { DispatchWindow::getGlobalDispatchWindow().callAsync(proc); }
	void runEventLoop() override { DispatchWindow::getGlobalDispatchWindow().runEventLoop(); }
};

xheap::uniquePtr<Dispatcher> Dispatcher::createInstance(uint32_t)
{
	return xheap::makeUnique<Dispatcher, DispatcherImpl>();
}

Dispatcher* Dispatcher::getMainDispatcher()
{
	static xheap::uniquePtr<Dispatcher> gInst;
	if(!gInst) { gInst = Dispatcher::createInstance(); }
	return gInst.get();
}

// --------------------------------------------------------------------------------

class DispatchTimerImpl : public DispatchTimer
{
public:
	ListenerListT<Listener> listenerList;
	std::function<void()> callbackProc;
	int idTimer = 0;
	DispatchTimerImpl() {}
	~DispatchTimerImpl() override
	{
		stop();
	}
	void addListener(Listener* p) override
	{
		listenerList.add(p);
	}
	void removeListener(Listener* p) override
	{
		listenerList.remove(p);
	}
	bool isRunning() const override
	{
		return 0 < idTimer;
	}
	void start(int tms, std::function<void()> proc) override
	{
		if(isRunning()) stop();
		callbackProc = proc;
		idTimer = DispatchWindow::getGlobalDispatchWindow().setTimer(tms, [this]()
		{
			if(callbackProc) { callbackProc(); }
			listenerList.call(&Listener::dispatchTimerDidElapse, this);
		});
	}
	void stop() override
	{
		if(!isRunning()) return;
		if(hasBeenShutdown) return;
		DispatchWindow::getGlobalDispatchWindow().killTimer(idTimer);
		idTimer = 0;
		callbackProc = nullptr;
	}
};

xheap::uniquePtr<DispatchTimer> DispatchTimer::createInstance()
{
	return xheap::makeUnique<DispatchTimer, DispatchTimerImpl>();
}

void DispatchTimer::callAfterDelay(int tms, std::function<void()> proc)
{
	struct Entry
	{
		std::function<void()> timerHandler;
		int idTimer = 0;
	};
	std::shared_ptr<Entry> ent = std::make_shared<Entry>();
	ent->timerHandler = proc;
	ent->idTimer = DispatchWindow::getGlobalDispatchWindow().setTimer(tms, [ent]()
	{
		if(ent->timerHandler) { ent->timerHandler(); }
		DispatchWindow::getGlobalDispatchWindow().killTimer(ent->idTimer);
	});
}

#elif defined(ESP_PLATFORM)

// ================================================================================
// Production code for ESP32

#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_task.h>
#include "utilities/listenerlist.h"
#include "utilities/strutil.h"

struct Mutex
{
	StaticSemaphore_t sSem{};
	SemaphoreHandle_t hSem;
	Mutex() { hSem = xSemaphoreCreateRecursiveMutexStatic(&sSem); }
	~Mutex() { vSemaphoreDelete(hSem); }
	void lock() { xSemaphoreTake(hSem, portMAX_DELAY); }
	void unlock() { xSemaphoreGive(hSem); }
};

struct ScopedLock
{
	Mutex& mtx;
	ScopedLock() = delete;
	ScopedLock(Mutex& m) : mtx(m) { mtx.lock(); }
	~ScopedLock() { mtx.unlock(); }
};

//ESP_EVENT_DECLARE_BASE(EventBaseDispatch);
ESP_EVENT_DEFINE_BASE(EventBaseDispatch);
static constexpr int32_t EventIdDispatchAsync = 1;

static const char* TagDispatcher = "Dispatcher";

class DispatcherImpl : public Dispatcher
{
private:
	static constexpr UBaseType_t TaskPriority = ESP_TASK_MAIN_PRIO;
	static constexpr uint32_t TaskStackDepthDefault = 4096;
	static constexpr uint32_t EventLoopQueueSize = 32;
	esp_event_loop_handle_t hEventLoop = NULL;
	esp_event_handler_instance_t eventHandlerInstance = NULL;
	struct CallQueue
	{
		Mutex mutex;
		static constexpr size_t QueueSize = 32;
		static constexpr size_t PosMask = 0x1f;
		std::array<std::function<void()>, QueueSize> queueList;
		size_t posPush = 0;
		size_t posPop = 0;
		CallQueue() {}
		size_t count() const { return (posPush - posPop) & PosMask; }
		size_t room() const { return queueList.size() - count(); }
		size_t push(std::function<void()> proc)
		{
			ScopedLock sl(mutex);
			if(!proc) { ESP_LOGW(TagDispatcher, "null dispatch function"); return count(); }
			if(!room()) { ESP_LOGW(TagDispatcher, "queue overflow"); return count(); }
			queueList[posPush & PosMask] = proc;
			++posPush;
			return count();
		}
		std::function<void()> pop()
		{
			ScopedLock sl(mutex);
			if(!count()) { return {}; }
			std::function<void()> proc = queueList[posPop & PosMask];
			queueList[posPop & PosMask] = nullptr;
			++posPop;
			return proc;
		}
	} callQueue;
public:
	DispatcherImpl() = delete;
	DispatcherImpl(uint32_t stackdepth, bool createtask)
	{
		ESP_LOGI(TagDispatcher, "construct: %s", createtask ? "with task" : "without task");
		esp_event_loop_args_t args{};
		args.queue_size = EventLoopQueueSize;
		if(createtask)
		{
			strutil::Str<16> name;
			name.format("disp%08zx", (size_t)this);
			args.task_name = name;
			args.task_priority = TaskPriority;
			args.task_stack_size = (stackdepth != 0) ? stackdepth : TaskStackDepthDefault;
			args.task_core_id = tskNO_AFFINITY; // ESP_TASK_MAIN_CORE;
		}
		ESP_ERROR_CHECK(esp_event_loop_create(&args, &hEventLoop));
		ESP_ERROR_CHECK(esp_event_handler_instance_register_with(hEventLoop, EventBaseDispatch, ESP_EVENT_ANY_ID, eventHandler, this, &eventHandlerInstance));
	}
	~DispatcherImpl() override
	{
		esp_event_handler_instance_unregister_with(hEventLoop, EventBaseDispatch, ESP_EVENT_ANY_ID, eventHandlerInstance);
		esp_event_loop_delete(hEventLoop);
		// ESP_LOGI(TagDispatcher, "destruct");
	}
private:
	static void eventHandler(void* arg, esp_event_base_t base, int32_t evid, void* evdata)
	{
		if(DispatcherImpl* p = (DispatcherImpl*)arg) { p->onEvent(base, evid, evdata); }
	}
	void onEvent(esp_event_base_t base, int32_t evid, void* evdata)
	{
		if((base == EventBaseDispatch) && (evid == EventIdDispatchAsync))
		{
			while(std::function<void()> proc = callQueue.pop()) { proc(); }
		}
	}
public:
	void callAsync(std::function<void()> proc) override
	{
		if(!proc) { return; }
		if(callQueue.push(proc) == 1) { esp_event_post_to(hEventLoop, EventBaseDispatch, EventIdDispatchAsync, nullptr, 0, portMAX_DELAY); }
	}
	void runEventLoop() override
	{
		// cf. esp_event_loop_run_task(), esp_event.c
		esp_err_t r = ESP_OK;
		while(true)
		{
			r = esp_event_loop_run(hEventLoop, portMAX_DELAY);
			if(r != ESP_OK) { break; }
		}
	}
};

xheap::uniquePtr<Dispatcher> Dispatcher::createInstance(uint32_t stackdepth)
{
	return xheap::makeUnique<Dispatcher, DispatcherImpl>(stackdepth, true);
}

Dispatcher* Dispatcher::getMainDispatcher()
{
	static xheap::uniquePtr<Dispatcher> gInst;
	if(!gInst) { gInst = xheap::makeUnique<Dispatcher, DispatcherImpl>(0, false); }
	return gInst.get();
}

// ================================================================================

static const char* TagDispatchTimer = "DispatchTimer";

class DispatchTimerImpl : public DispatchTimer
{
private:
	strutil::Str<16> timerName;
	TimerHandle_t hTimer = NULL;
	StaticTimer_t sTimer{};
	std::function<void()> clientCallback;
	ListenerListT<Listener> listenerList;
public:
	DispatchTimerImpl()
	{
		ESP_LOGI(TagDispatchTimer, "construct");
		timerName.format("timer%x", (uintptr_t)this);
	}
	~DispatchTimerImpl() override
	{
		if(hTimer) { xTimerDelete(hTimer, 500); }
		ESP_LOGI(TagDispatchTimer, "destruct");
	}
private:
	static void timerHandler(TimerHandle_t htimer)
	{
		if(DispatchTimerImpl* p = (DispatchTimerImpl*)pvTimerGetTimerID(htimer)) { p->onTimerElapsed(); }
	}
	void onTimerElapsed()
	{
		Dispatcher::getMainDispatcher()->callAsync([this]()
		{
			if(clientCallback) { clientCallback(); }
			listenerList.call(&Listener::dispatchTimerDidElapse, this);
		});
	}
public:
	void addListener(Listener* p) override
	{
		listenerList.add(p);
	}
	void removeListener(Listener* p) override
	{
		listenerList.remove(p);
	}
	bool isRunning() const override
	{
		return hTimer && (xTimerIsTimerActive(hTimer) != pdFALSE);
	}
	void start(int tms, std::function<void()> proc) override
	{
		clientCallback = proc;
		TickType_t period = pdMS_TO_TICKS(tms);
		if(!hTimer)
		{
			hTimer = xTimerCreateStatic(timerName, period, pdTRUE, this, timerHandler, &sTimer);
			xTimerStart(hTimer, 500);
		}
		else
		{
			xTimerReset(hTimer, 500);
			if(xTimerGetPeriod(hTimer) != period) { xTimerChangePeriod(hTimer, period, 500); }
		}
	}
	void stop() override
	{
		if(hTimer && xTimerIsTimerActive(hTimer)) { xTimerStop(hTimer, 500); }
		clientCallback = nullptr;
	}
};

xheap::uniquePtr<DispatchTimer> DispatchTimer::createInstance()
{
	return xheap::makeUnique<DispatchTimer, DispatchTimerImpl>();
}

void DispatchTimer::callAfterDelay(int tms, std::function<void()> proc)
{
	static constexpr size_t MaxOneshotTimers = 4;
	struct Entry
	{
		strutil::Str<16> timerName;
		TimerHandle_t hTimer = NULL;
		StaticTimer_t sTimer{};
		std::function<void()> clientCallback;
		void launch(int tms, std::function<void()> proc)
		{
			if(hTimer) return;
			clientCallback = proc;
			timerName.format("oneshot%x", (uintptr_t)this);
			hTimer = xTimerCreateStatic(timerName, pdMS_TO_TICKS(tms), pdFALSE, this, timerHandler, &sTimer);
			xTimerStart(hTimer, 500);
		}
		static void timerHandler(TimerHandle_t htimer)
		{
			if(Entry* p = (Entry*)pvTimerGetTimerID(htimer)) { p->onTimerElapsed(); }
		}
		void onTimerElapsed()
		{
			Dispatcher::getMainDispatcher()->callAsync([this]()
			{
				xTimerDelete(hTimer, 500);
				if(clientCallback) { clientCallback(); }
				*this = {};
			});
		}
	};
	static std::array<Entry, MaxOneshotTimers> StaticEntries;
	auto it = std::find_if(StaticEntries.begin(), StaticEntries.end(), [](const Entry& ent) { return ent.hTimer == NULL; });
	if(it == StaticEntries.end()) return;
	it->launch(tms, proc);
}

#else
#error unknown platform
#endif // defined(_WINDOWS) / defined(ESP_PLATFORM)
