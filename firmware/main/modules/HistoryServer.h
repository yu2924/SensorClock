//
//  HistoryServer.h
//  SensorClock
//
//  created by yu2924 on 2026-04-21
//

#pragma once

#include "utilities/strutil.h"
#include "utilities/xheap.h"
#include "modules/DataLogger.h"

class HistoryServer
{
public:
	struct Config
	{
		strutil::Str<16> username;
		strutil::Str<16> password;
		strutil::Str<16> hostname;
		uint16_t port;
		bool operator!=(const Config& r) const { return (username != r.username) || (password != r.password) || (hostname != r.hostname) || (port != r.port); }
		bool operator==(const Config& r) const { return !(*this != r); }
	};
	struct Listener
	{
		virtual ~Listener() {}
		virtual void historyServerConfigDidChange(HistoryServer*, const Config&) {}
	};
	virtual ~HistoryServer() {}
	virtual void addListener(Listener* p) = 0;
	virtual void removeListener(Listener* p) = 0;
	virtual const Config& getConfig() const = 0;
	virtual void setConfig(const Config& v) = 0;
	virtual bool isActive() const = 0;
	virtual bool activate() = 0;
	virtual void deactivate() = 0;
	// factory
	static xheap::uniquePtr<HistoryServer> createInstance(DataLogger* p);
};
