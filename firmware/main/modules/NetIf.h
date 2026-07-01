//
//  Network.h
//  SensorClock
//
//  created by yu2924 on 2025-12-28
//

#pragma once

#include <array>
#include <functional>
#include "utilities/strutil.h"
#include "utilities/xheap.h"

static constexpr size_t MaxSSID = 32;

struct WifiAp
{
	strutil::Str<(MaxSSID + 1 + 3) & ~0x03> ssid;
	int authMode;
	int pairwiseCipher;
	int groupCipher;
};

struct WifiConfig
{
	strutil::Str<(MaxSSID + 1 + 3) & ~0x03> ssid;
	strutil::Str<64> password;
	bool operator!=(const WifiConfig& r) const { return (ssid != r.ssid) || (password != r.password); }
	bool operator==(const WifiConfig& r) const { return !(*this != r); }
};

const char* wifiAuthModeText(int authmode);
const char* wifiCipherTypeText(int cipher);

class WifiConnection
{
public:
	struct Listener
	{
		virtual ~Listener() {}
		virtual void wifiConnectionConfigDidChange(WifiConnection*, const WifiConfig&) {}
		virtual void wifiConnectionStatusDidUpdate(WifiConnection*, bool) {}
	};
	virtual ~WifiConnection() {}
	virtual void addListener(Listener* p) = 0;
	virtual void removeListener(Listener* p) = 0;
	virtual const WifiConfig& getConfig() const = 0;
	virtual void setConfig(const WifiConfig& v) = 0;
	virtual bool isActive() const = 0;
	virtual bool activate() = 0;
	virtual void deactivate() = 0;
	virtual bool isConnected() const = 0;
	virtual const char* getIpAddress() const = 0;
	// factory
	static void platformInit();
	static xheap::uniquePtr<WifiConnection> createInstance();
};

class WifiApScanner
{
public:
	static constexpr size_t MaxApScanList = 16;
	struct Result
	{
		std::array<WifiAp, MaxApScanList> arrWifiAp;
		size_t numWifiAp;
	};
	virtual ~WifiApScanner() {}
	virtual bool isRunning() const = 0;
	virtual bool start(WifiConnection* wificon, std::function<void(const Result&)> onfinish) = 0;
	virtual void abort() = 0;
	// factory
	static xheap::uniquePtr<WifiApScanner> createInstance();
};

class SntpService
{
public:
	virtual ~SntpService() {}
	virtual const char* getServer() const = 0;
	virtual void setServer(const char* v) = 0;
	virtual bool isActive() const = 0;
	virtual bool activate() = 0;
	virtual void deactivate() = 0;
	// factory
	static xheap::uniquePtr<SntpService> createInstance();
};
