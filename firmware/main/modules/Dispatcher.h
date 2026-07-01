//
//  Dispatcher.h
//  SensorClock
//
//  created by yu2924 on 2025-11-23
//

#pragma once

#include <functional>
#include "utilities/xheap.h"

class Dispatcher
{
public:
	virtual ~Dispatcher() {}
	virtual void callAsync(std::function<void()> proc) = 0;
	virtual void runEventLoop() = 0;
	// factory
	static xheap::uniquePtr<Dispatcher> createInstance(uint32_t stackdepth = 0);
	// provides the main event loop
	static Dispatcher* getMainDispatcher();
};

class DispatchTimer
{
public:
	struct Listener
	{
		virtual ~Listener() {}
		virtual void dispatchTimerDidElapse(DispatchTimer*) = 0;
	};
	virtual ~DispatchTimer() {}
	virtual void addListener(Listener* p) = 0;
	virtual void removeListener(Listener* p) = 0;
	virtual bool isRunning() const = 0;
	virtual void start(int tms, std::function<void()> proc = nullptr) = 0;
	virtual void stop() = 0;
	// factory
	static xheap::uniquePtr<DispatchTimer> createInstance();
	// single shot
	static void callAfterDelay(int tms, std::function<void()> proc);
};
