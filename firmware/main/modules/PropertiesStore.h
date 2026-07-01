//
//  PropertiesStore.h
//  SensorClock
//
//  created by yu2924 on 2025-12-07
//

#pragma once

#include "utilities/strutil.h"
#include "utilities/xheap.h"

class PropertiesStore
{
public:
	virtual ~PropertiesStore() {}
	virtual bool open(const char* sect) = 0;
	virtual bool flush() = 0;
	virtual void close() = 0;
	virtual bool getBoolValue(const char* key, bool vdef) const = 0;
	virtual void setBoolValue(const char* key, bool v) = 0;
	virtual int getIntValue(const char* key, int vdef) const = 0;
	virtual void setIntValue(const char* key, int v) = 0;
	virtual double getDoubleValue(const char* key, double vdef) const = 0;
	virtual void setDoubleValue(const char* key, double v) = 0;
	virtual size_t getStringValue(const char* key, const char* vdef, char* buf, size_t buflen) const = 0;
	virtual void setStringValue(const char* key, const char* v) = 0;
	// factory
	static xheap::uniquePtr<PropertiesStore> createInstance();
};
