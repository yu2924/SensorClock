//
//  Network.cpp
//  SensorClock
//
//  created by yu2924 on 2025-12-28
//

#include "NetIf.h"

#if defined(_WINDOWS)

// ================================================================================
// Prototype code for Windows
// These simply output dummy data, or does nothing.

#if defined(_WIN32)
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define STRICT
#if !defined NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Winsock2.h>
#pragma comment(lib, "Ws2_32.lib")
typedef int socklen_t;
#endif
#include <string>
#include "utilities/listenerlist.h"
#include "utilities/platform.h"
#include "modules/Dispatcher.h"

static std::string GetPrimaryHostAddress(uint16_t port)
{
	std::string saddr;
	SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if(s != INVALID_SOCKET)
	{
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr("192.0.2.4"); // RFC3330 TEST-NET
		addr.sin_port = htons(port);
		if(connect(s, (sockaddr*)&addr, sizeof(addr)) != SOCKET_ERROR)
		{
			socklen_t cbaddr = sizeof(addr);
			if((getsockname(s, (sockaddr*)&addr, &cbaddr) != SOCKET_ERROR) && (addr.sin_addr.s_addr != inet_addr("127.0.0.1"))) { saddr = inet_ntoa(addr.sin_addr); }
		}
		closesocket(s);
	}
	return saddr;
}

const char* wifiAuthModeText([[maybe_unused]] int authmode)
{
	return "WIFI_AUTH_UNKNOWN";
}

const char* wifiCipherTypeText([[maybe_unused]] int cipher)
{
	return "WIFI_CIPHER_TYPE_UNKNOWN";
}

// --------------------------------------------------------------------------------

class WifiConnectionImpl : public WifiConnection
{
public:
	WifiConfig config{};
	ListenerListT<Listener> listenerList;
	std::string ipAddress;
	bool active = false;
	bool connected = false;
	WifiConnectionImpl()
	{
		ipAddress = GetPrimaryHostAddress(80);
		DEBUGPRINTI("WifiConnection: ip=%s", ipAddress.c_str());
	}
	~WifiConnectionImpl() override
	{
	}
	void update()
	{
		bool newconnected = !config.ssid.isEmpty() && active;
		if(connected != newconnected)
		{
			connected = newconnected;
			Dispatcher::getMainDispatcher()->callAsync([this]()
			{
				listenerList.call(&Listener::wifiConnectionStatusDidUpdate, this, connected);
			});
		}
	}
	void addListener(Listener* p) override
	{
		listenerList.add(p);
	}
	void removeListener(Listener* p) override
	{
		listenerList.remove(p);
	}
	const WifiConfig& getConfig() const override
	{
		return config;
	}
	void setConfig(const WifiConfig& v) override
	{
		if(config == v) { return; }
		config = v;
		update();
		listenerList.call(&Listener::wifiConnectionConfigDidChange, this, config);
	}
	bool isActive() const override
	{
		return active;
	}
	bool activate() override
	{
		active = true;
		update();
		return true;
	}
	void deactivate() override
	{
		active = false;
		update();
	}
	bool isConnected() const override
	{
		return connected;
	}
	const char* getIpAddress() const override
	{
		return ipAddress.c_str();
	}
};

void WifiConnection::platformInit()
{
}

xheap::uniquePtr<WifiConnection> WifiConnection::createInstance()
{
	return xheap::makeUnique<WifiConnection, WifiConnectionImpl>();
}

// --------------------------------------------------------------------------------

class WifiApScannerImpl : public WifiApScanner
{
public:
	Result result{};
	bool running = false;
	WifiApScannerImpl()
	{
	}
	~WifiApScannerImpl() override
	{
	}
	bool isRunning() const override
	{
		return running;
	}
	bool start(WifiConnection* wificon, std::function<void(const Result&)> onfinish) override
	{
		WifiConnection* wifiConRestore = nullptr;
		if(wificon && wificon->isActive()) { wifiConRestore = wificon; wifiConRestore->deactivate(); }
		running = true;
		DEBUGPRINTI("WifiApScan: start");
		DispatchTimer::callAfterDelay(500, [this, wifiConRestore, onfinish]()
		{
			result = {};
			result.numWifiAp = 4;
			for(size_t c = result.numWifiAp, i = 0; i < c; ++i)
			{
				result.arrWifiAp[i].ssid.format("Dummy AP %c", 'A' + (int)i);
			}
			if(onfinish) onfinish(result);
			running = false;
			if(wifiConRestore) { wifiConRestore->activate(); }
			DEBUGPRINTI("WifiApScan: stop");
		});
		return true;
	}
	void abort() override
	{
	}
};

xheap::uniquePtr<WifiApScanner> WifiApScanner::createInstance()
{
	return xheap::makeUnique<WifiApScanner, WifiApScannerImpl>();
}

// --------------------------------------------------------------------------------

class SntpServiceImpl : public SntpService
{
public:
	strutil::Str<128> serverUrl;
	bool active = false;
	SntpServiceImpl()
	{
	}
	~SntpServiceImpl() override
	{
	}
	void update()
	{
		// setup SNTP
	}
	const char* getServer() const override
	{
		return serverUrl;
	}
	void setServer(const char* v) override
	{
		if(serverUrl == v) { return; }
		serverUrl = v;
		update();
	}
	bool isActive() const override
	{
		return active;
	}
	bool activate() override
	{
		active = true;
		update();
		return true;
	}
	void deactivate() override
	{
		active = false;
	}
};

xheap::uniquePtr<SntpService> SntpService::createInstance()
{
	return xheap::makeUnique<SntpService, SntpServiceImpl>();
}

#elif defined(ESP_PLATFORM)

// ================================================================================
// Production code for ESP32

#include <esp_event.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <esp_netif_sntp.h>
#include <esp_sntp.h>
#include <esp_wifi.h>
#include "utilities/listenerlist.h"
#include "utilities/platform.h"
#include "modules/Dispatcher.h"

const char* wifiAuthModeText(int authmode)
{
	static const struct { int authmode; const char* text; } Table[] =
	{
		{ WIFI_AUTH_OPEN, "OPEN" },
		{ WIFI_AUTH_OWE, "OWE" },
		{ WIFI_AUTH_WEP, "WEP" },
		{ WIFI_AUTH_WPA_PSK, "WPA-PSK" },
		{ WIFI_AUTH_WPA2_PSK, "WPA2-PSK" },
		{ WIFI_AUTH_WPA_WPA2_PSK, "WPA/WPA2-PSK" },
		{ WIFI_AUTH_ENTERPRISE, "ENTERPRISE" },
		{ WIFI_AUTH_WPA3_PSK, "WPA3-PSK" },
		{ WIFI_AUTH_WPA2_WPA3_PSK, "WPA2/WPA3-PSK" },
		{ WIFI_AUTH_WPA3_ENTERPRISE, "WPA3-ENTERPRISE" },
		{ WIFI_AUTH_WPA2_WPA3_ENTERPRISE, "WPA2/WPA3-ENTERPRISE" },
		{ WIFI_AUTH_WPA3_ENT_192, "WPA3-ENT 192" },
	};
	for(const auto& ent : Table) { if(ent.authmode == authmode) return ent.text; }
	return "UNKNOWN";
}

const char* wifiCipherTypeText(int cipher)
{
	static const struct { int cipher; const char* text; } Table[] =
	{
		{ WIFI_CIPHER_TYPE_NONE, "NONE" },
		{ WIFI_CIPHER_TYPE_WEP40, "WEP40" },
		{ WIFI_CIPHER_TYPE_WEP104, "WEP104" },
		{ WIFI_CIPHER_TYPE_TKIP, "TKIP" },
		{ WIFI_CIPHER_TYPE_CCMP, "CCMP" },
		{ WIFI_CIPHER_TYPE_TKIP_CCMP, "TKIP/CCMP" },
		{ WIFI_CIPHER_TYPE_AES_CMAC128, "AES-CMAC128" },
		{ WIFI_CIPHER_TYPE_SMS4, "SMS4" },
		{ WIFI_CIPHER_TYPE_GCMP, "GCMP" },
		{ WIFI_CIPHER_TYPE_GCMP256, "GCMP256" },
	};
	for(const auto& ent : Table) { if(ent.cipher == cipher) return ent.text; }
	return "UNKNOWN";
}

// --------------------------------------------------------------------------------

static const char* TagWifiConnection = "WifiConnection";

class WifiConnectionImpl : public WifiConnection
{
private:
	static constexpr int MaxNumRetry = 5;
	WifiConfig config;
	ListenerListT<Listener> listenerList;
	strutil::Str<20> ipAddress;
	int iRetry = 0;
	bool active = false;
	bool connected = false;
public:
	WifiConnectionImpl()
	{
		ESP_LOGI(TagWifiConnection, "construct");
		esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, eventHandler, this);
		esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, eventHandler, this);
	}
	~WifiConnectionImpl() override
	{
		deactivate();
		esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, eventHandler);
		esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, eventHandler);
		ESP_LOGI(TagWifiConnection, "destruct");
	}
private:
	static void eventHandler(void* arg, esp_event_base_t base, int32_t evid, void* evdata)
	{
		if(WifiConnectionImpl* p = (WifiConnectionImpl*)arg) { p->onEvent(base, evid, evdata); }
	}
	void onEvent(esp_event_base_t base, int32_t evid, void* evdata)
	{
		if((base == WIFI_EVENT) && (evid == WIFI_EVENT_STA_START))
		{
			ESP_LOGI(TagWifiConnection, "WIFI_EVENT_STA_START");
		}
		else if((base == WIFI_EVENT) && (evid == WIFI_EVENT_STA_DISCONNECTED))
		{
			ESP_LOGI(TagWifiConnection, "WIFI_EVENT_STA_DISCONNECTED");
			ipAddress.clear();
			connected = false;
			if(isConnectionNeeded() && (iRetry < MaxNumRetry))
			{
				++iRetry;
				esp_wifi_connect();
				ESP_LOGI(TagWifiConnection, "retry=%d", iRetry);
			}
			else
			{
				Dispatcher::getMainDispatcher()->callAsync([this]() { listenerList.call(&Listener::wifiConnectionStatusDidUpdate, this, connected); });
			}
		}
		else if((base == WIFI_EVENT) && (evid == WIFI_EVENT_STA_CONNECTED))
		{
			ESP_LOGI(TagWifiConnection, "WIFI_EVENT_STA_CONNECTED");
		}
		else if((base == IP_EVENT) && (evid == IP_EVENT_STA_GOT_IP))
		{
			ESP_LOGI(TagWifiConnection, "IP_EVENT_STA_GOT_IP");
			ip_event_got_ip_t* event = (ip_event_got_ip_t*)evdata;
			esp_ip4_addr_t ip = event->ip_info.ip;
			ESP_LOGI(TagWifiConnection, "  ip=%d.%d.%d.%d", esp_ip4_addr1(&ip), esp_ip4_addr2(&ip), esp_ip4_addr3(&ip), esp_ip4_addr4(&ip));
			ipAddress.format("%d.%d.%d.%d", esp_ip4_addr1(&ip), esp_ip4_addr2(&ip), esp_ip4_addr3(&ip), esp_ip4_addr4(&ip));
			connected = true;
			iRetry = 0;
			Dispatcher::getMainDispatcher()->callAsync([this]() { listenerList.call(&Listener::wifiConnectionStatusDidUpdate, this, connected); });
		}
	}
	bool isConnectionNeeded() const
	{
		return !config.ssid.isEmpty() && active;
	}
	bool updateConnection()
	{
		if(isConnectionNeeded())
		{
			if(!connected)
			{
				iRetry = 0;
				wifi_config_t cfg = {};
				strutil::copy((char*)cfg.sta.ssid, std::size(cfg.sta.ssid), config.ssid);
				strutil::copy((char*)cfg.sta.password, std::size(cfg.sta.password), config.password);
				cfg.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
				cfg.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;
				strutil::copy((char*)cfg.sta.sae_h2e_identifier, std::size(cfg.sta.sae_h2e_identifier), "");
				esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &cfg);
				if(err != ESP_OK) { ESP_LOGW(TagWifiConnection, "esp_wifi_set_config() failed"); }
				else
				{
					err = esp_wifi_connect();
					if(err != ESP_OK) { ESP_LOGW(TagWifiConnection, "esp_wifi_connect() failed"); }
				}
				return err == ESP_OK;
			}
			return true;
		}
		else
		{
			if(connected)
			{
				esp_wifi_disconnect();
			}
			return true;
		}
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
	const WifiConfig& getConfig() const override
	{
		return config;
	}
	void setConfig(const WifiConfig& cfg) override
	{
		if(config == cfg) { return; }
		config = cfg;
		listenerList.call(&Listener::wifiConnectionConfigDidChange, this, config);
		updateConnection();
	}
	bool isActive() const override
	{
		return active;
	}
	bool activate() override
	{
		active = true;
		return updateConnection();
	}
	void deactivate() override
	{
		active = false;
		updateConnection();
	}
	bool isConnected() const override
	{
		return connected;
	}
	const char* getIpAddress() const override
	{
		return ipAddress;
	}
};

void WifiConnection::platformInit()
{
	ESP_ERROR_CHECK(esp_netif_init());
	esp_netif_t* netif = esp_netif_create_default_wifi_sta();
	assert(netif);
	wifi_init_config_t initcfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&initcfg));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_start());
}

xheap::uniquePtr<WifiConnection> WifiConnection::createInstance()
{
	return xheap::makeUnique<WifiConnection, WifiConnectionImpl>();
}

// --------------------------------------------------------------------------------

static const char* TagWifiApScanner = "WifiApScanner";

static_assert(CONFIG_WIFI_PROV_SCAN_MAX_ENTRIES <= WifiApScanner::MaxApScanList, "CONFIG_WIFI_PROV_SCAN_MAX_ENTRIES <= WifiApScanner::MaxApScanList");

class WifiApScannerImpl : public WifiApScanner
{
private:
	WifiConnection* wifiConRestore = nullptr;
	std::function<void(const Result&)> onFinish;
	std::array<wifi_ap_record_t, MaxApScanList> wifiApRecordList;
	Result result{};
	bool running = false;
public:
	WifiApScannerImpl()
	{
		esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_SCAN_DONE, eventHandler, this);
	}
	~WifiApScannerImpl() override
	{
		esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_SCAN_DONE, eventHandler);
	}
private:
	static void eventHandler(void* arg, esp_event_base_t base, int32_t evid, void* evdata)
	{
		if(WifiApScannerImpl* p = (WifiApScannerImpl*)arg) { p->onEvent(base, evid, evdata); }
	}
	void onEvent(esp_event_base_t base, int32_t evid, void* evdata)
	{
		if((base == WIFI_EVENT) && (evid == WIFI_EVENT_SCAN_DONE))
		{
			ESP_LOGI(TagWifiApScanner, "WIFI_EVENT_SCAN_DONE");
			running = false;
			uint16_t numap = 0;
			if(esp_wifi_scan_get_ap_num(&numap) == ESP_OK)
			{
				numap = std::min(numap, (uint16_t)wifiApRecordList.size());
				if(esp_wifi_scan_get_ap_records(&numap, wifiApRecordList.data()) == ESP_OK)
				{
					result = {};
					result.numWifiAp = numap;
					for(size_t c = numap, i = 0; i < c; ++i)
					{
						result.arrWifiAp[i].ssid = (const char*)wifiApRecordList[i].ssid;
						result.arrWifiAp[i].authMode = wifiApRecordList[i].authmode;
						result.arrWifiAp[i].groupCipher = wifiApRecordList[i].group_cipher;
						result.arrWifiAp[i].pairwiseCipher = wifiApRecordList[i].pairwise_cipher;
					}
				}
			}
			Dispatcher::getMainDispatcher()->callAsync([this]()
			{
				if(wifiConRestore) { wifiConRestore->activate(); }
				wifiConRestore = nullptr;
				if(onFinish) { onFinish(result); }
				onFinish = nullptr;
			});
		}
	}
public:
	bool isRunning() const override
	{
		return running;
	}
	bool start(WifiConnection* wificon, std::function<void(const Result&)> onfinish) override
	{
		if(running) { return false; }
		wifiConRestore = nullptr;
		if(wificon && wificon->isActive()) { wifiConRestore = wificon; wifiConRestore->deactivate(); }
		onFinish = onfinish;
		result = {};
		running = true;
		esp_err_t err = esp_wifi_scan_start(nullptr, false);
		if(err != ESP_OK)
		{
			if(wifiConRestore) { wifiConRestore->activate(); }
			wifiConRestore = nullptr;
			if(onFinish) { onFinish(result); }
			onFinish = nullptr;
			running = false;
			return false;
		}
		return true;
	}
	void abort() override
	{
		if(running) { esp_wifi_scan_stop(); }
	}
};

xheap::uniquePtr<WifiApScanner> WifiApScanner::createInstance()
{
	return xheap::makeUnique<WifiApScanner, WifiApScannerImpl>();
}

// --------------------------------------------------------------------------------

static const char* TagSntpService = "SntpService";

class SntpServiceImpl : public SntpService
{
private:
	strutil::Str<32> serverUrl;
	bool active = false;
	bool running = false;
public:
	SntpServiceImpl()
	{
		ESP_LOGI(TagSntpService, "syncinterval=%u ms", sntp_get_sync_interval());
	}
	~SntpServiceImpl() override
	{
		deactivate();
	}
private:
	static void onTimeSync(timeval* tv)
	{
		tm stm{};
		const tm* ptm = platform::localtime(&tv->tv_sec, &stm);
		char s[32]{}; strftime(s, std::size(s), "%Y-%m-%d %H:%M:%S", ptm);
		ESP_LOGI(TagSntpService, "synchronized, currenttime=%s", s);
	}
	bool updateRunning()
	{
		if(active && !serverUrl.isEmpty())
		{
			if(!running)
			{
				esp_sntp_config_t cfg = ESP_NETIF_SNTP_DEFAULT_CONFIG((const char*)serverUrl);
				cfg.wait_for_sync = false;
				cfg.sync_cb = onTimeSync;
				esp_err_t err = esp_netif_sntp_init(&cfg);
				if(err != ESP_OK) { return false; }
				running = true;
			}
			else
			{
				const char* cururl = esp_sntp_getservername(0);
				if(!strutil::areEqual(cururl, serverUrl)) { esp_sntp_setservername(0, serverUrl); }
			}
		}
		else if(!active || serverUrl.isEmpty())
		{
			if(running)
			{
				esp_netif_sntp_deinit();
				running = false;
			}
		}
		return true;
	}
public:
	const char* getServer() const override
	{
		return serverUrl;
	}
	void setServer(const char* v) override
	{
		if(serverUrl == v) { return; }
		serverUrl = v;
		updateRunning();
	}
	bool isActive() const override
	{
		return active;
	}
	bool activate() override
	{
		active = true;
		return updateRunning();
	}
	void deactivate() override
	{
		active = false;
		updateRunning();
	}
};

xheap::uniquePtr<SntpService> SntpService::createInstance()
{
	return xheap::makeUnique<SntpService, SntpServiceImpl>();
}

#else
#error unknown platform
#endif // defined(_WINDOWS) / defined(ESP_PLATFORM)
