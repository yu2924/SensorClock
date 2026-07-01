//
//  WeatherTask.cpp
//  SensorClock
//
//  created by yu2924 on 2025-11-23
//

#include "WeatherTask.h"

#include "utilities/cjsonwrap.h"
#include "utilities/listenerlist.h"
#include "utilities/platform.h"
#include "utilities/strutil.h"
#include "modules/Dispatcher.h"
#include "modules/WebRequest.h"
#if __has_include("modules/WeatherTask.config.h")
#include "modules/WeatherTask.config.h"
#endif

// ================================================================================

class WeatherLocationQueryImpl : public WeatherLocationQuery
{
private:
	xheap::uniquePtr<WebRequest> webRequest = WebRequest::createInstance();
	Result result{};
	strutil::Str<256> urlBuf;
public:
	WeatherLocationQueryImpl()
	{
	}
	~WeatherLocationQueryImpl() override
	{
		abort();
	}
	static bool parseJson(const char* pjson, size_t ljson, Result* pr) noexcept
	{
		cjsonwrap::cJSONUniquePtr rootarr(cJSON_ParseWithLength(pjson, ljson));
		if(!rootarr) return false;
		int carr = cJSON_GetArraySize(rootarr.get());
		carr = std::min(carr, (int)MaxLocationList);
		pr->numLocations = carr;
		for(int i = 0; i < carr; ++i)
		{
			WeatherLocation& loc = pr->arrLocations[i];
			cJSON* dic = cJSON_GetArrayItem(rootarr.get(), i);
			loc.cityName = cJSON_GetStringValue(cJSON_GetObjectItem(dic, "name"));
			loc.lat = cJSON_GetNumberValue(cJSON_GetObjectItem(dic, "lat"));
			loc.lon = cJSON_GetNumberValue(cJSON_GetObjectItem(dic, "lon"));
			loc.country = cJSON_GetStringValue(cJSON_GetObjectItem(dic, "country"));
		}
		return true;
	}
	bool isRunning() const
	{
		return webRequest->isRunning();
	}
	bool query(const char* apikey, const char* name, std::function<void(const Result&)> onfinish)
	{
		if(isRunning()) return false;
		result = {};
		urlBuf = {};
		urlBuf <<"https://api.openweathermap.org/geo/1.0/direct?q=" << name << "&limit=" << strutil::Str<16>::formatted("%zu", MaxLocationList) << "&appid=" << apikey;
		return webRequest->start(urlBuf, [this, onfinish](const WebRequest::Response& resp)
		{
			if(webRequest->isAborted()) return;
			if((resp.status == 200) && parseJson((const char*)resp.body.data(), resp.body.size(), &result)) { result.apiStatus = resp.status; }
			if(onfinish) onfinish(result);
		});
	}
	void abort()
	{
		webRequest->abort();
	}
};

xheap::uniquePtr<WeatherLocationQuery> WeatherLocationQuery::createInstance()
{
	return xheap::makeUnique<WeatherLocationQuery, WeatherLocationQueryImpl>();
}

// ================================================================================

class WeatherQueryImpl : public WeatherQuery
{
private:
	xheap::uniquePtr<WebRequest> webRequest = WebRequest::createInstance();
	Result result{};
	strutil::Str<256> urlBuf;
public:
	WeatherQueryImpl()
	{
	}
	~WeatherQueryImpl() override
	{
		abort();
	}
	static bool parseJson(const char* pjson, size_t ljson, Result* pr) noexcept
	{
		cjsonwrap::cJSONUniquePtr rootdic(cJSON_ParseWithLength(pjson, ljson));
		if(!rootdic) return false;
		// current
		cJSON* curdic = cJSON_GetObjectItem(rootdic.get(), "current");
		WeatherCurrentStatus& cstat = pr->current;
		cstat.dt = (time_t)cJSON_GetNumberValue(cJSON_GetObjectItem(curdic, "dt"));
		cstat.sunrise = (time_t)cJSON_GetNumberValue(cJSON_GetObjectItem(curdic, "sunrise"));
		cstat.sunset = (time_t)cJSON_GetNumberValue(cJSON_GetObjectItem(curdic, "sunset"));
		cstat.tempC = (float)cJSON_GetNumberValue(cJSON_GetObjectItem(curdic, "temp"));
		cstat.pressureHpa = (int16_t)cJSON_GetNumberValue(cJSON_GetObjectItem(curdic, "pressure"));
		cstat.humidityPct = (int16_t)cJSON_GetNumberValue(cJSON_GetObjectItem(curdic, "humidity"));
		cstat.cloudsPct = (int16_t)cJSON_GetNumberValue(cJSON_GetObjectItem(curdic, "clouds"));
		cstat.windDeg = (int16_t)cJSON_GetNumberValue(cJSON_GetObjectItem(curdic, "wind_deg"));
		cstat.windSpeedMps = (float)cJSON_GetNumberValue(cJSON_GetObjectItem(curdic, "wind_speed"));
		cJSON* wdic = cJSON_GetArrayItem(cJSON_GetObjectItem(curdic, "weather"), 0);
		cstat.weatherId = (int16_t)cJSON_GetNumberValue(cJSON_GetObjectItem(wdic, "id"));
		cstat.weatherIcon = cJSON_GetStringValue(cJSON_GetObjectItem(wdic, "icon"));
		// forecast
		cJSON* dailyarr = cJSON_GetObjectItem(rootdic.get(), "daily");
		int carr = cJSON_GetArraySize(dailyarr);
		carr = std::min(carr, (int)MaxForecastList);
		pr->numforecasts = carr;
		for(int i = 0; i < carr; ++i)
		{
			cJSON* daydic = cJSON_GetArrayItem(dailyarr, i);
			WeatherDailyStatus& fstat = pr->arrForecasts[i];
			fstat.dt = (time_t)cJSON_GetNumberValue(cJSON_GetObjectItem(daydic, "dt"));
			fstat.sunrise = (time_t)cJSON_GetNumberValue(cJSON_GetObjectItem(daydic, "sunrise"));
			fstat.sunset = (time_t)cJSON_GetNumberValue(cJSON_GetObjectItem(daydic, "sunset"));
			fstat.moonrise = (time_t)cJSON_GetNumberValue(cJSON_GetObjectItem(daydic, "moonrise"));
			fstat.moonset = (time_t)cJSON_GetNumberValue(cJSON_GetObjectItem(daydic, "moonset"));
			fstat.moonphase = (float)cJSON_GetNumberValue(cJSON_GetObjectItem(daydic, "moonphase"));
			cJSON* tdic = cJSON_GetObjectItem(daydic, "temp");
			fstat.tempC = (float)cJSON_GetNumberValue(cJSON_GetObjectItem(tdic, "day"));
			fstat.tempMinC = (float)cJSON_GetNumberValue(cJSON_GetObjectItem(tdic, "min"));
			fstat.tempMaxC = (float)cJSON_GetNumberValue(cJSON_GetObjectItem(tdic, "max"));
			fstat.pressureHpa = (int16_t)cJSON_GetNumberValue(cJSON_GetObjectItem(daydic, "pressure"));
			fstat.humidityPct = (int16_t)cJSON_GetNumberValue(cJSON_GetObjectItem(daydic, "humidity"));
			fstat.cloudsPct = (int16_t)cJSON_GetNumberValue(cJSON_GetObjectItem(daydic, "clouds"));
			fstat.windDeg = (int16_t)cJSON_GetNumberValue(cJSON_GetObjectItem(daydic, "wind_deg"));
			fstat.windSpeedMps = (float)cJSON_GetNumberValue(cJSON_GetObjectItem(daydic, "wind_speed"));
			cJSON* wdic = cJSON_GetArrayItem(cJSON_GetObjectItem(daydic, "weather"), 0);
			fstat.weatherId = (int16_t)cJSON_GetNumberValue(cJSON_GetObjectItem(wdic, "id"));
			fstat.weatherIcon = cJSON_GetStringValue(cJSON_GetObjectItem(wdic, "icon"));
		}
		return true;
	}
	bool isRunning() const override
	{
		return webRequest->isRunning();
	}
	bool query(const char* apikey, double lat, double lon, std::function<void(const Result&)> onfinish) override
	{
		if(isRunning()) return false;
		result = {};
#if DUMMYTEST
		DispatchTimer::callAfterDelay(200, [this, onfinish]()
		{
			if(parseJson(WeatherForecastDummyResponce, strlen(WeatherForecastDummyResponce), &result)) { result.apiStatus = 200; }
			if(onfinish) { onfinish(result); }
		});
		return true;
#else
		urlBuf = {};
		urlBuf << "https://api.openweathermap.org/data/3.0/onecall?lat=" << strutil::Str<16>::formatted("%g", lat) << "&lon=" << strutil::Str<16>::formatted("%g", lon) << "&units=metric&exclude=minutely,hourly,alerts&appid=" << apikey;
		return webRequest->start(urlBuf, [this, onfinish](const WebRequest::Response& resp)
		{
			if(webRequest->isAborted()) return;
			if((resp.status == 200) && parseJson((const char*)resp.body.data(), resp.body.size(), &result)) { result.apiStatus = resp.status; }
			if(onfinish) { onfinish(result); }
		});
#endif
	}
	void abort() override
	{
		webRequest->abort();
	}
};

xheap::uniquePtr<WeatherQuery> WeatherQuery::createInstance()
{
	return xheap::makeUnique<WeatherQuery, WeatherQueryImpl>();
}

// ================================================================================

class WeatherTaskImpl : public WeatherTask, private DispatchTimer::Listener
{
private:
	// quantize in 10-minute intervals
	static time_t normalizeTime(time_t t)
	{
		tm stm{};
		const tm* ptm = platform::localtime(&t, &stm);
		if(!ptm) { return t; }
		tm tm = *ptm;
		tm.tm_min -= tm.tm_min % 10;
		tm.tm_sec = 0;
		return mktime(&tm);
	}
	static constexpr int PollingPeriodMs = 10 * 1000;
	xheap::uniquePtr<DispatchTimer> timer = DispatchTimer::createInstance();
	xheap::uniquePtr<WeatherQuery> weatherQuery = WeatherQuery::createInstance();
	strutil::Str<128> apiKey;
	WeatherLocation location{};
	ListenerListT<WeatherTask::Listener> listenerList;
	WeatherQuery::Result result{};
	time_t lastQueryTime = 0;
	bool active = false;
public:
	WeatherTaskImpl()
	{
		timer->addListener(this);
	}
	~WeatherTaskImpl() override
	{
		deactivate();
		timer->removeListener(this);
	}
private:
	void restartTask()
	{
		stopTask();
#if !DUMMYTEST
		if(!apiKey.isEmpty() && location.isValid() && active && !timer->isRunning())
#endif
		{
			performQuery(true);
			timer->start(PollingPeriodMs);
		}
	}
	void stopTask()
	{
		if(timer->isRunning()) { timer->stop(); }
		if(weatherQuery->isRunning()) { weatherQuery->abort(); }
	}
	void performQuery(bool force)
	{
		time_t tcur = normalizeTime(time(nullptr));
		if(!force && (lastQueryTime == tcur)) { return; }
		weatherQuery->query(apiKey, location.lat, location.lon, [this, tcur](const WeatherQuery::Result& r)
		{
			result = r;
			if(result.apiStatus == 200) { lastQueryTime = tcur; }
			listenerList.call(&WeatherTask::Listener::weatherTaskResultDidChange, this, result);
		});
	}
public:
	void addListener(WeatherTask::Listener* p) override
	{
		listenerList.add(p);
	}
	void removeListener(WeatherTask::Listener* p) override
	{
		listenerList.remove(p);
	}
	bool isActive() const override
	{
		return active;
	}
	bool activate() override
	{
		active = true;
		restartTask();
		return true;
	}
	void deactivate() override
	{
		active = false;
		stopTask();
	}
	const char* getApiKey() const override
	{
		return apiKey;
	}
	void setApiKey(const char* v) override
	{
		if(apiKey == v) { return; }
		apiKey = v;
		restartTask();
		listenerList.call(&WeatherTask::Listener::weatherTaskConfigDidChange, this, apiKey, location);
	}
	const WeatherLocation& getLocation() const override
	{
		return location;
	}
	void setLocation(const WeatherLocation& v) override
	{
		if(location == v) { return; }
		location = v;
		restartTask();
		listenerList.call(&WeatherTask::Listener::weatherTaskConfigDidChange, this, apiKey, location);
	}
	const WeatherQuery::Result& getResult() const
	{
		return result;
	}
private:
	// DispatchTimer::Listener
	void dispatchTimerDidElapse(DispatchTimer*) override
	{
		performQuery(false);
	}
};

xheap::uniquePtr<WeatherTask> WeatherTask::createInstance()
{
	return xheap::makeUnique<WeatherTask, WeatherTaskImpl>();
}
