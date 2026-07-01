//
//  AlarmSounder.h
//  SensorClock
//
//  created by yu2924 on 2026-04-08
//

#pragma once

#include "utilities/xheap.h"

class AlarmSounder
{
public:
	struct Listener
	{
		virtual ~Listener() {}
		virtual void alertSounderSettingsDidChange(AlarmSounder*, int vol, bool loop, bool en) {}
		virtual void alertSounderReachedToEnd(AlarmSounder*) {}
	};
	virtual ~AlarmSounder() {}
	virtual void addListener(Listener* p) = 0;
	virtual void removeListener(Listener* p) = 0;
	// settings
	virtual int getVolume() const = 0;
	virtual void setVolume(int v) = 0;
	virtual bool isLooped() const = 0;
	virtual void setLooped(bool v) = 0;
	virtual bool isEnabled() const = 0;
	virtual void setEnabled(bool v) = 0;
	// transport
	virtual const char* getMediaFile() const = 0;
	virtual void setMediaFile(const char* v) = 0;
	virtual bool isPlayOn() const = 0;
	virtual void setPlayOn(bool v) = 0;
	virtual bool isAuditionOn() const = 0;
	virtual void setAuditionOn(bool v) = 0;
	// factory
	static void platformInit();
	static xheap::uniquePtr<AlarmSounder> createInstance();
};
