//
//  AppController.cpp
//  SensorClock
//
//  created by yu2924 on 2025-11-26
//

#include "AppController.h"

#include "utilities/cjsonwrap.h"
#include "utilities//heapbuffer.h"
#include "utilities/listenerlist.h"
#include "utilities/platform.h"
#include "utilities/strutil.h"
#include "modules/BoardFs.h"
#include "modules/DataLogger.h"
#include "modules/Dispatcher.h"

class AppControllerImpl
	: public AppController
	, private WifiConnection::Listener
	, private WeatherTask::Listener
	, private SensorTask::Listener
	, private HistoryServer::Listener
	, private AlarmSounder::Listener
{
private:
	ListenerListT<AppController::Listener> listenerList;
	const struct TimeZoneList
	{
		cjsonwrap::cJSONUniquePtr jsonRoot;
		HeapBufferT<TzEnt> tzEntList;
		TimeZoneList()
		{
			strutil::Str<128> path;
			BoardFs::resolveAssetFilePath("timezones50.json", path.getBuffer(), path.capacity());
			HeapBufferT<char> json;
			BoardFs::loadStringFromFile(path, &json);
			jsonRoot.reset(cJSON_Parse(json.data()));
			int numtz = cJSON_GetArraySize(jsonRoot.get());
			tzEntList.resize(numtz);
			for(int i = 0; i < numtz; ++i)
			{
				cJSON* entdic = cJSON_GetArrayItem(jsonRoot.get(), i);
				TzEnt& ent = tzEntList[i];
				ent.cityName	= cJSON_GetStringValue(cJSON_GetObjectItem(entdic, "city"));
				ent.utcoffset	= cJSON_GetStringValue(cJSON_GetObjectItem(entdic, "utcOffset"));
				ent.posixstring	= cJSON_GetStringValue(cJSON_GetObjectItem(entdic, "posixString"));
			}
		}
		size_t getDefaultIndex() const
		{
			for(size_t c = tzEntList.size(), i = 0; i < c; ++i)
			{
				if(strstr(tzEntList[i].posixstring, "JST")) { return i; }
			}
			return 0;
		}
		size_t size() const { return tzEntList.size(); }
		const TzEnt& getAtIndex(size_t i) const { return tzEntList[i]; }
		const TzEnt* getData() const { return tzEntList.data(); }
	} timeZoneList;
	xheap::uniquePtr<PropertiesStore> propertiesStore = PropertiesStore::createInstance();
	xheap::uniquePtr<WifiConnection> wifiConnection = WifiConnection::createInstance();
	xheap::uniquePtr<WeatherTask> weatherTask = WeatherTask::createInstance();
	xheap::uniquePtr<SensorTask> sensorTask = SensorTask::createInstance();
	xheap::uniquePtr<AlarmSounder> alarmSounder = AlarmSounder::createInstance();
	xheap::uniquePtr<SntpService> sntpService = SntpService::createInstance();
	xheap::uniquePtr<DataLogger> dataLogger = DataLogger::createInstance(sensorTask.get(), weatherTask.get());
	xheap::uniquePtr<HistoryServer> historyServer = HistoryServer::createInstance(dataLogger.get());
	size_t timeZoneIndex = (size_t)-1;
	bool active = false;
public:
	AppControllerImpl()
	{
		propertiesStore->open("preferences");
		// system config
		{
			strutil::Str<128> sntpsvr;
			propertiesStore->getStringValue("sntpsvr", "ntp.nict.jp", sntpsvr.getBuffer(), sntpsvr.capacity());
			setSntpServerInternal(sntpsvr);
			size_t tzi = propertiesStore->getIntValue("tzindex", -1);
			setTimeZoneIndexInternal(tzi);
		}
		// wifi
		{
			WifiConfig wcfg{};
			propertiesStore->getStringValue("wifissid", "", wcfg.ssid.getBuffer(), wcfg.ssid.capacity());
			propertiesStore->getStringValue("wifipass", "", wcfg.password.getBuffer(), wcfg.password.capacity());
			wifiConnection->setConfig(wcfg);
		}
		// weather api
		{
			strutil::Str<128> key;
			propertiesStore->getStringValue("wapikey", "", key.getBuffer(), key.capacity());
			weatherTask->setApiKey(key);
		}
		{
			WeatherLocation loc{};
			propertiesStore->getStringValue("wapicity", "Tokyo", loc.cityName.getBuffer(), loc.cityName.capacity());
			propertiesStore->getStringValue("wapicntr", "JP", loc.country.getBuffer(), loc.country.capacity());
			loc.lat = propertiesStore->getDoubleValue("wapilat", 35.6828387);
			loc.lon = propertiesStore->getDoubleValue("wapilon", 139.7594549);
			weatherTask->setLocation(loc.isValid() ? loc : WeatherLocation{});
		}
		// sensors
		{
			strutil::Str<16> k;
			SensorTask::Thresholds th = SensorTask::Thresholds::Defaults;
			for(int i = 0; i < SensorTask::Icount; ++i)
			{
				const char* mnem = SensorTask::MetadataList[i].mnemonic;
				k.format("sth%su", mnem);
				th.array[i].under = (float)propertiesStore->getDoubleValue(k, (double)th.array[i].under);
				k.format("sth%so", mnem);
				th.array[i].over  = (float)propertiesStore->getDoubleValue(k, (double)th.array[i].over);
				k.format("sth%sw", mnem);
				SensorTask::Falerts ma = 1 << i;
				th.alerts = (th.alerts & ~ma) | (propertiesStore->getBoolValue(k, th.alerts & ma) ? ma : 0);
			}
			sensorTask->setThresholds(th);
		}
		// history server
		{
#if defined(_WINDOWS)
			constexpr uint16_t DefaultPort = 8080;
#else
			constexpr uint16_t DefaultPort = 80;
#endif
			HistoryServer::Config cfg{};
			propertiesStore->getStringValue("hsvrhost", "SensorClock", cfg.hostname.getBuffer(), cfg.hostname.capacity());
			propertiesStore->getStringValue("hsvruser", "SensorClock", cfg.username.getBuffer(), cfg.username.capacity());
			propertiesStore->getStringValue("hsvrpass", "password", cfg.password.getBuffer(), cfg.password.capacity());
			cfg.port = (uint16_t)propertiesStore->getIntValue("hsvrport", DefaultPort);
			historyServer->setConfig(cfg);
		}
		// alert
		{
			strutil::Str<128> path;
			BoardFs::resolveAssetFilePath("Alert.ogg", path.getBuffer(), path.capacity());
			alarmSounder->setMediaFile(path);
			alarmSounder->setLooped(true);
			alarmSounder->setVolume(propertiesStore->getIntValue("alarmvol", 60));
			alarmSounder->setEnabled(propertiesStore->getBoolValue("alarmen", false));
		}
		// events
		wifiConnection->addListener(this);
		weatherTask->addListener(this);
		sensorTask->addListener(this);
		historyServer->addListener(this);
		alarmSounder->addListener(this);
	}
	~AppControllerImpl() override
	{
		alarmSounder->removeListener(this);
		historyServer->removeListener(this);
		sensorTask->removeListener(this);
		weatherTask->removeListener(this);
		wifiConnection->removeListener(this);
		propertiesStore->flush();
	}
	void addListener(AppController::Listener* p) override
	{
		listenerList.add(p);
	}
	void removeListener(AppController::Listener* p) override
	{
		listenerList.remove(p);
	}
	// --------------------------------------------------------------------------------
	// containing objects
	PropertiesStore* getPropertiesStore() override
	{
		return propertiesStore.get();
	}
	WifiConnection* getWifiConnection() override
	{
		return wifiConnection.get();
	}
	WeatherTask* getWeatherTask() override
	{
		return weatherTask.get();
	}
	SensorTask* getSensorTask() override
	{
		return sensorTask.get();
	}
	AlarmSounder* getAlarmSounder() override
	{
		return alarmSounder.get();
	}
	HistoryServer* getHistoryServer() override
	{
		return historyServer.get();
	}
	// --------------------------------------------------------------------------------
	// status control
	bool isActive() const override
	{
		return active;
	}
	void activate() override
	{
		// NOTE: these { sntpService, weatherTask, historyServer } are activated automatically when the wifiConnection is connected
		active = true;
		wifiConnection->activate();
		sensorTask->activate();
		dataLogger->activate();
	}
	void deactivate() override
	{
		dataLogger->deactivate();
		sensorTask->deactivate();
		wifiConnection->deactivate();
		active = false;
	}
	// --------------------------------------------------------------------------------
	// system config
	const char* getSntpServer() const override
	{
		return sntpService->getServer();
	}
	void setSntpServerInternal(const char* urlnew)
	{
		sntpService->setServer(urlnew);
	}
	void setSntpServer(const char* urlnew) override
	{
		if(strutil::areEqual(sntpService->getServer(), urlnew)) { return; }
		setSntpServerInternal(urlnew);
		propertiesStore->setStringValue("sntpsvr", urlnew);
		listenerList.call(&AppController::Listener::appControllerSntpServerDidChange, this, urlnew);
	}
	TzArray getTimeZoneList() const override
	{
		return { timeZoneList.getData(), timeZoneList.size()};
	}
	size_t getTimeZoneIndex() const override
	{
		return timeZoneIndex;
	}
	void setTimeZoneIndexInternal(size_t tzinew)
	{
		timeZoneIndex = (tzinew < timeZoneList.size()) ? tzinew : timeZoneList.getDefaultIndex();
		platform::setSystemTimeZone(timeZoneList.getAtIndex(timeZoneIndex).posixstring);
	}
	void setTimeZoneIndex(size_t tzinew) override
	{
		if(timeZoneIndex == tzinew) { return; }
		setTimeZoneIndexInternal(tzinew);
		propertiesStore->setIntValue("tzindex", (int)timeZoneIndex);
		listenerList.call(&AppController::Listener::appControllerTimeZoneIndexDidChange, this, tzinew);
	}
private:
	// --------------------------------------------------------------------------------
	// incoming events
	// WifiConnection::Listener
	void wifiConnectionConfigDidChange(WifiConnection*, const WifiConfig& cfg) override
	{
		propertiesStore->setStringValue("wifissid", cfg.ssid);
		propertiesStore->setStringValue("wifipass", cfg.password);
		DEBUGPRINTI("AppController: set ssid=%s", cfg.ssid);
	}
	void wifiConnectionStatusDidUpdate(WifiConnection*, bool connected) override
	{
		if(connected)
		{
			DEBUGPRINTI("AppController: network connected");
			sntpService->activate();
			weatherTask->activate();
			historyServer->activate();
		}
		else
		{
			DEBUGPRINTI("AppController: network disconnected");
			historyServer->deactivate();
			weatherTask->deactivate();
			sntpService->deactivate();
		}
	}
	// WeatherTask::Listener
	void weatherTaskConfigDidChange(WeatherTask*, const char* key, const WeatherLocation& loc) override
	{
		propertiesStore->setStringValue("wapikey", key);
		propertiesStore->setStringValue("wapicity", loc.cityName);
		propertiesStore->setStringValue("wapicntr", loc.country);
		propertiesStore->setDoubleValue("wapilat", loc.lat);
		propertiesStore->setDoubleValue("wapilon", loc.lon);
		DEBUGPRINTI("AppController: set apikey=%s location=%s", key, (const char*)loc.cityName);
	}
	// SensorTask::Listener
	void sensorTaskThresholdsDidChange(SensorTask* p, const SensorTask::Thresholds& th) override
	{
		strutil::Str<16> k;
		for(int i = 0; i < SensorTask::Icount; ++i)
		{
			const char* mnem = SensorTask::MetadataList[i].mnemonic;
			k.format("sth%su", mnem);
			propertiesStore->setDoubleValue(k, (double)th.array[i].under);
			k.format("sth%so", mnem);
			propertiesStore->setDoubleValue(k, (double)th.array[i].over);
			k.format("sth%sw", mnem);
			SensorTask::Falerts ma = 1 << i;
			propertiesStore->setBoolValue(k, th.alerts & ma);
		}
		alarmSounder->setPlayOn(p->getAlertFlags() != 0);
	}
	void sensorTaskValuesDidChange(SensorTask* p, SensorTask::Fupdates, const SensorTask::Values&) override
	{
		alarmSounder->setPlayOn(p->getAlertFlags() != 0);
	}
	// HistoryServer::Listener
	void historyServerConfigDidChange(HistoryServer*, const HistoryServer::Config& cfg) override
	{
		propertiesStore->setStringValue("hsvrhost", cfg.hostname);
		propertiesStore->setStringValue("hsvruser", cfg.username);
		propertiesStore->setStringValue("hsvrpass", cfg.password);
		propertiesStore->setIntValue("hsvrport", cfg.port);
	}
	// AlarmSounder::Listener
	void alertSounderSettingsDidChange(AlarmSounder*, int vol, bool, bool en) override
	{
		propertiesStore->setIntValue("alarmvol", alarmSounder->getVolume());
		propertiesStore->setBoolValue("alarmen", alarmSounder->isEnabled());
	}
};

xheap::uniquePtr<AppController> AppController::createInstance()
{
	return xheap::makeUnique<AppController, AppControllerImpl>();
}
