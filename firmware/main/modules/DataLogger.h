//
//  DataLogger.h
//  SensorClock
//
//  created by yu2924 on 2026-04-15
//

#pragma once

#include <array>
#include "utilities/xheap.h"
#include "modules/SensorTask.h"
#include "modules/WeatherTask.h"

class DataLogger
{
public:
	enum
	{
		IsfTemp = 0,
		IsfHum,
		IsfPres,
		IsfCo2,
		IsfIaq,
		IsfCount,
	};
	struct SensorHistoryRecord
	{
		time_t dt;
		std::array<float, 5> values;
	};
	struct SensorSnapshot
	{
		virtual ~SensorSnapshot() {}
		virtual size_t getNumRecords() const = 0;
		virtual bool getRecordAt(size_t i, SensorHistoryRecord* p) = 0;
	};
	struct WeatherHistoryRecord
	{
		time_t dt;
		float tempC;
		int16_t pressureHpa;
		int16_t humidityPct;
		int16_t cloudsPct;
		int16_t windDeg;
		float windSpeedMps;
		int16_t weatherId;
		strutil::Str<14> weatherIcon;
	};
	struct WeatherSnapshot
	{
		virtual ~WeatherSnapshot() {}
		virtual size_t getNumRecords() const = 0;
		virtual bool getRecordAt(size_t i, WeatherHistoryRecord* p) = 0;
	};
	virtual ~DataLogger() {}
	virtual void resetHistory() = 0;
	virtual bool isActive() const = 0;
	virtual bool activate() = 0;
	virtual void deactivate() = 0;
	virtual xheap::uniquePtr<SensorSnapshot> createSensorSnapshot(time_t tfrom, time_t tto) = 0;
	virtual xheap::uniquePtr<WeatherSnapshot> createWeatherSnapshot(time_t tfrom, time_t tto) = 0;
	// factory
	static xheap::uniquePtr<DataLogger> createInstance(SensorTask* st, WeatherTask* wt);
};
