//
//  WeatherTask.h
//  SensorClock
//
//  created by yu2924 on 2025-11-23
//

#pragma once

#include <array>
#include <functional>
#include "utilities/strutil.h"
#include "utilities/xheap.h"

struct WeatherLocation
{
	strutil::Str<32> cityName;
	strutil::Str<8> country;
	double lat, lon;
	bool isValid() const noexcept { return !cityName.isEmpty() && !country.isEmpty(); }
	bool operator!=(const WeatherLocation& r) const { return (cityName != r.cityName) || (country != r.country) || (lat != r.lat) || (lon != r.lon); }
	bool operator==(const WeatherLocation& r) const { return !(*this != r); }
};

struct WeatherCurrentStatus
{
	time_t dt, sunrise, sunset;
	float tempC;
	int16_t pressureHpa;
	int16_t humidityPct;
	int16_t cloudsPct;
	int16_t windDeg;
	float windSpeedMps;
	int16_t weatherId;
	strutil::Str<14> weatherIcon;
};

struct WeatherDailyStatus
{
	time_t dt, sunrise, sunset, moonrise, moonset;
	float moonphase;
	float tempC, tempMinC, tempMaxC;
	int16_t pressureHpa;
	int16_t humidityPct;
	int16_t cloudsPct;
	int16_t windDeg;
	float windSpeedMps;
	int16_t weatherId;
	strutil::Str<14> weatherIcon;
};

class WeatherLocationQuery
{
public:
	static constexpr size_t MaxLocationList = 5;
	struct Result
	{
		int apiStatus;
		std::array<WeatherLocation, MaxLocationList> arrLocations;
		size_t numLocations;
	};
	virtual ~WeatherLocationQuery() {}
	virtual bool isRunning() const = 0;
	virtual bool query(const char* apikey, const char* name, std::function<void(const Result&)> onfinish) = 0;
	virtual void abort() = 0;
	// factory
	static xheap::uniquePtr<WeatherLocationQuery> createInstance();
};

class WeatherQuery
{
public:
	static constexpr size_t MaxForecastList = 8;
	struct Result
	{
		int apiStatus;
		WeatherCurrentStatus current;
		std::array<WeatherDailyStatus, MaxForecastList> arrForecasts;
		size_t numforecasts;
	};
	virtual ~WeatherQuery() {}
	virtual bool isRunning() const = 0;
	virtual bool query(const char* apikey, double lat, double lon, std::function<void(const Result&)> onfinish) = 0;
	virtual void abort() = 0;
	// factory
	static xheap::uniquePtr<WeatherQuery> createInstance();
};

class WeatherTask
{
public:
	struct Listener
	{
		virtual ~Listener() {}
		virtual void weatherTaskConfigDidChange(WeatherTask*, const char*, const WeatherLocation&) {}
		virtual void weatherTaskResultDidChange(WeatherTask*, const WeatherQuery::Result&) {}
	};
	virtual ~WeatherTask() {}
	virtual void addListener(Listener* p) = 0;
	virtual void removeListener(Listener* p) = 0;
	virtual bool isActive() const = 0;
	virtual bool activate() = 0;
	virtual void deactivate() = 0;
	virtual const char* getApiKey() const = 0;
	virtual void setApiKey(const char* v) = 0;
	virtual const WeatherLocation& getLocation() const = 0;
	virtual void setLocation(const WeatherLocation& v) = 0;
	virtual const WeatherQuery::Result& getResult() const = 0;
	// factory
	static xheap::uniquePtr<WeatherTask> createInstance();
};
