//
//  SensorTask.h
//  SensorClock
//
//  created by yu2924 on 2025-12-20
//

#pragma once

#include <array>
#include <functional>
#include "utilities/xheap.h"

class SensorTask
{
public:
	enum
	{
		Iscd4xCo2 = 0,
		Iscd4xTemp,
		Iscd4xHum,
		Ibme680Temp,
		Ibme680Pres,
		Ibme680Hum,
		Ibme680Iaq,
		Icount,
	};
	enum
	{
		Fscd4xCo2 = 1 << Iscd4xCo2,
		Fscd4xTemp = 1 << Iscd4xTemp,
		Fscd4xHum = 1 << Iscd4xHum,
		Fscd4xAll = Fscd4xCo2 | Fscd4xTemp | Fscd4xHum,
		Fbme680Temp = 1 << Ibme680Temp,
		Fbme680Pres = 1 << Ibme680Pres,
		Fbme680Hum = 1 << Ibme680Hum,
		Fbme680Iaq = 1 << Ibme680Iaq,
		Fbme680All = Fbme680Temp | Fbme680Pres | Fbme680Hum | Fbme680Iaq,
	};
	struct Metadata
	{
		const char* title;
		const char* unit;
		const char* mnemonic;
		bool proportional;
	};
	static const std::array<Metadata, Icount> MetadataList;
	using Fupdates = uint32_t;
	using Falerts = uint32_t;
	struct Thresholds
	{
		struct { float under, over; } array[Icount];
		Falerts alerts;
		bool operator!=(const Thresholds& r) const { for(size_t i = 0; i < Icount; ++i) { if((array[i].under != r.array[i].under) || (array[i].over != r.array[i].over)) { return true; } } return alerts != r.alerts; }
		bool operator==(const Thresholds& r) const { return !(*this != r); }
		static const Thresholds Ranges;
		static const Thresholds Defaults;
	};
	using Values = std::array<float, Icount>;
	static Falerts calculateAlertFlags(const Thresholds& th, const Values& val);
	struct Listener
	{
		virtual ~Listener() {}
		virtual void sensorTaskThresholdsDidChange(SensorTask*, const Thresholds&) {}
		virtual void sensorTaskValuesDidChange(SensorTask*, Fupdates, const Values&) {}
	};
	virtual ~SensorTask() {}
	virtual void addListener(Listener* p) = 0;
	virtual void removeListener(Listener* p) = 0;
	virtual const Thresholds& getThresholds() const = 0;
	virtual void setThresholds(const Thresholds& v) = 0;
	virtual bool isActive() const = 0;
	virtual bool activate() = 0;
	virtual void deactivate() = 0;
	virtual const Values& getValues() const = 0;
	virtual Falerts getAlertFlags() const = 0;
	// factory
	static void platformInit();
	static xheap::uniquePtr<SensorTask> createInstance();
};
