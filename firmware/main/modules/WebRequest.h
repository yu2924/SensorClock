//
//  WebRequest.h
//  SensorClock
//
//  created by yu2924 on 2025-11-23
//

#pragma once

#include <functional>
#include "utilities/heapbuffer.h"
#include "utilities/xheap.h"

class WebRequest
{
public:
	struct Response
	{
		int status;
		HeapBufferT<uint8_t> body;
	};
	virtual ~WebRequest() {}
	virtual bool isRunning() const = 0;
	virtual bool start(const char* url, std::function<void(const Response&)> onfinish) = 0;
	virtual bool isAborted() const = 0;
	virtual void abort() = 0;
	// factory
	static xheap::uniquePtr<WebRequest> createInstance();
};
