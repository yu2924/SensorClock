//
//  AppController.h
//  SensorClock
//
//  created by yu2924 on 2025-11-26
//

#pragma once

#include "utilities/strutil.h"
#include "utilities/xheap.h"
#include "modules/AlarmSounder.h"
#include "modules/HistoryServer.h"
#include "modules/NetIf.h"
#include "modules/PropertiesStore.h"
#include "modules/SensorTask.h"
#include "modules/WeatherTask.h"

class AppController
{
public:
	struct TzEnt
	{
		const char* cityName;
		const char* utcoffset;
		const char* posixstring;
	};
	struct TzArray
	{
		const TzEnt* ptr;
		size_t size;
	};
	struct Listener
	{
		virtual ~Listener() {}
		virtual void appControllerSntpServerDidChange(AppController*, const char*) {}
		virtual void appControllerTimeZoneIndexDidChange(AppController*, size_t) {}
	};
	virtual ~AppController() {}
	virtual void addListener(Listener* p) = 0;
	virtual void removeListener(Listener* p) = 0;
	// containing objects
	virtual PropertiesStore* getPropertiesStore() = 0;
	virtual WifiConnection* getWifiConnection() = 0;
	virtual WeatherTask* getWeatherTask() = 0;
	virtual SensorTask* getSensorTask() = 0;
	virtual HistoryServer* getHistoryServer() = 0;
	virtual AlarmSounder* getAlarmSounder() = 0;
	// status control
	virtual bool isActive() const = 0;
	virtual void activate() = 0;
	virtual void deactivate() = 0;
	// system config
	virtual const char* getSntpServer() const = 0;
	virtual void setSntpServer(const char* v) = 0;
	virtual TzArray getTimeZoneList() const = 0;
	virtual size_t getTimeZoneIndex() const = 0;
	virtual void setTimeZoneIndex(size_t v) = 0;
	// factory
	static xheap::uniquePtr<AppController> createInstance();
};
