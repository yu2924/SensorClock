//
//  DataLogger.cpp
//  SensorClock
//
//  created by yu2924 on 2026-04-15
//

#include "DataLogger.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <ctime>
#include <mutex>
#include "utilities/platform.h"
#include "utilities/xheap.h"
#include "modules/BoardFs.h"
#include "modules/Dispatcher.h"

namespace fcc
{
	static constexpr uint32_t fccFromStr(const char(&sz)[5])
	{
		return sz[3] * 0x1000000u + sz[2] * 0x10000u + sz[1] * 0x100u + sz[0];
	}
	static constexpr std::array<char, 5> strFromFcc(uint32_t fcc)
	{
		return std::array<char, 5>{ (char)(fcc), (char)(fcc >> 8), (char)(fcc >> 16), (char)(fcc >> 24), 0 };
	}
};

template<typename ExtRecordT, typename IntRecordT, typename ISnapshot, size_t BufferSize, uint32_t Signature, uint32_t Version> struct HistoryBufferBaseT
{
	struct FileHeader
	{
		uint32_t sig;
		uint32_t ver;
		uint32_t wlen;
		uint32_t reserved;
	};
	static constexpr size_t TotalBytes = BufferSize * sizeof(IntRecordT);
	IntRecordT* bufferPtr = nullptr;
	uint32_t whead = 0, wlen = 0;
	std::recursive_mutex mtx;
	HistoryBufferBaseT()
	{
		bufferPtr = (IntRecordT*)xheap::memAlloc(TotalBytes);
		if(!bufferPtr) { DEBUGPRINTW("HistoryBuffer(%s): malloc(%zu) failed", fcc::strFromFcc(Signature).data(), TotalBytes); }
	}
	~HistoryBufferBaseT()
	{
		xheap::memFree(bufferPtr);
	}
	size_t getNumRecordsInternal() const
	{
		return wlen;
	}
	IntRecordT& getRecordAtInternal(size_t idx) const
	{
		assert(bufferPtr && (idx < wlen));
		int rhead = (int)whead - (int)wlen + (int)idx; if(rhead < 0) { rhead += BufferSize; }
		return bufferPtr[rhead];
	}
	void reset()
	{
		std::lock_guard sl(mtx);
		whead = wlen = 0;
	}
	bool load(const char* path)
	{
		auto loadproc = [this](FILE* pf)->bool
		{
			FileHeader hdr{};
			if(fread(&hdr, 1, sizeof(hdr), pf) != sizeof(hdr)) { return false; }
			if((hdr.sig != Signature) || (hdr.ver != Version) || (BufferSize < hdr.wlen)) { return false; }
			for(size_t c = hdr.wlen, i = 0; i < c; ++i)
			{
				IntRecordT& rec = bufferPtr[i];
				if(fread(&rec, 1, sizeof(IntRecordT), pf) != sizeof(IntRecordT)) { return false; }
			}
			whead = wlen = hdr.wlen;
			if(BufferSize <= whead) { whead = 0; }
			DEBUGPRINTI("HistoryBuffer(%s): load %zu records", fcc::strFromFcc(Signature).data(), wlen);
			return true;
		};
		std::lock_guard sl(mtx);
		if(!bufferPtr) { return false; }
		bool r = false;
		if(FILE* pf = fopen(path, "rb"))
		{
			r = loadproc(pf);
			fclose(pf);
		}
		if(!r) { reset(); }
		return r;
	}
	bool save(const char* path)
	{
		auto saveproc = [this](FILE* pf)->bool
		{
			FileHeader hdr = { Signature, Version, wlen, 0 };
			if(fwrite(&hdr, 1, sizeof(hdr), pf) != sizeof(hdr)) { return false; }
			for(size_t c = wlen, i = 0; i < c; ++i)
			{
				const IntRecordT& rec = getRecordAtInternal(i);
				if(fwrite(&rec, 1, sizeof(IntRecordT), pf) != sizeof(IntRecordT)) { return false; }
			}
			DEBUGPRINTI("HistoryBuffer(%s): save %zu records", fcc::strFromFcc(Signature).data(), wlen);
			return true;
		};
		std::lock_guard sl(mtx);
		if(!bufferPtr) { return false; }
		bool r = false;
		if(FILE* pf = fopen(path, "wb"))
		{
			r = saveproc(pf);
			fclose(pf);
		}
		return r;
	}
	void push(const ExtRecordT& rec)
	{
		std::lock_guard sl(mtx);
		if(!bufferPtr) { return; }
		bufferPtr[whead] = rec; // convert ExtRecordT into IntRecordT
		++whead; if(BufferSize <= whead) { whead = 0; }
		if(wlen < BufferSize) { ++wlen; }
		DEBUGPRINTI("HistoryBuffer(%s): push(), totalrecords=%u", fcc::strFromFcc(Signature).data(), wlen);
	}
	void replaceLast(const ExtRecordT& rec)
	{
		std::lock_guard sl(mtx);
		if(!bufferPtr) { return; }
		if(0 < wlen)
		{
			IntRecordT& reclast = getRecordAtInternal(wlen - 1);
			reclast = rec; // convert ExtRecordT into IntRecordT
			DEBUGPRINTI("HistoryBuffer(%s): replaceLast(), totalrecords=%u", fcc::strFromFcc(Signature).data(), wlen);
		}
		else
		{
			push(rec);
		}
	}
	xheap::uniquePtr<ISnapshot> createSnapshot(time_t tfrom, time_t tto)
	{
		struct SnapshotImpl : public ISnapshot
		{
			IntRecordT* ptrRecords = nullptr;
			size_t numRecords = 0;
			bool good = false;
			SnapshotImpl() = delete;
			SnapshotImpl(HistoryBufferBaseT& host, time_t tfrom, time_t tto)
			{
				size_t wlen = host.getNumRecordsInternal();
				if(0 < wlen)
				{
					ptrRecords = (IntRecordT*)xheap::memAlloc(wlen * sizeof(IntRecordT));
					if(!ptrRecords) { DEBUGPRINTW("Snapshot(%s): malloc(%zu) failed", fcc::strFromFcc(Signature).data(), wlen * sizeof(IntRecordT)); return; }
					size_t irec = 0;
					for(size_t i = 0; i < wlen; ++i)
					{
						const IntRecordT& rec = host.getRecordAtInternal(i);
						if((rec.dt < tfrom) || (tto < rec.dt)) { continue; }
						ptrRecords[irec] = rec; // convert IntRecordT into ExtRecordT
						++irec;
					}
					numRecords = irec;
				}
				good = true;
			}
			virtual ~SnapshotImpl()
			{
				xheap::memFree(ptrRecords);
			}
			virtual size_t getNumRecords() const override
			{
				return numRecords;
			}
			virtual bool getRecordAt(size_t i, ExtRecordT* rec) override
			{
				if(numRecords <= i) { return false; }
				*rec = ptrRecords[i];
				return true;
			}
		};
		std::lock_guard sl(mtx);
		if(!bufferPtr) { return {}; }
		auto snapshot = xheap::makeUnique<ISnapshot, SnapshotImpl>(*this, tfrom, tto);
		if(snapshot && !((SnapshotImpl*)snapshot.get())->good) { snapshot.reset(); }
		return snapshot;
	}
};

template<typename Record, size_t MaxQueue> struct AsyncQueue
{
	std::recursive_mutex mtx;
	Dispatcher* dispatcher = nullptr;
	std::array<Record, MaxQueue> buffer{};
	size_t whead = 0, wlen = 0;
	std::function<void()> signalCallback;
	bool trigger = false;
	AsyncQueue() = delete;
	AsyncQueue(Dispatcher* disp, std::function<void()> cb) : dispatcher(disp), signalCallback(cb) {}
	size_t size() const
	{
		return wlen;
	}
	void push(const Record& rec)
	{
		std::lock_guard sl(mtx);
		if(buffer.size() <= wlen) { return; }
		buffer[whead] = rec;
		++wlen;
		++whead; if(buffer.size() <= whead) { whead -= buffer.size(); }
		if(!trigger)
		{
			trigger = true;
			dispatcher->callAsync([this]() { std::lock_guard sl(mtx); if(signalCallback) { signalCallback(); } trigger = false; });
		}
	}
	bool pop(Record* rec)
	{
		std::lock_guard sl(mtx);
		if(!wlen) { return false; }
		int rhead = (int)whead - (int)wlen; if(rhead < 0) { rhead += (int)buffer.size(); }
		*rec = buffer[rhead];
		--wlen;
		return true;
	}
};

// ================================================================================
// sensor history

namespace sensorhistory
{
	// data flow and threads:
	// 
	// thread:               main                 queue dispatch        queue dispatch             external
	// block : [SensorTask] ----> [Logger [AsyncQueue] ----> [AverageBuffer] ----> [HistoryBuffer]] ----> [external snapshot client]

	using SensorFrame = std::array<float, DataLogger::IsfCount>;
	static SensorFrame SensorFrameDenan(const SensorFrame& vi) { SensorFrame vo; for(size_t i = 0; i < vi.size(); ++i) { vo[i] = std::isnan(vi[i]) ? 0.0f : vi[i]; } return vo; }
	static SensorFrame operator*(const SensorFrame& vi, float m) { SensorFrame vo; for(size_t i = 0; i < vi.size(); ++i) { vo[i] = vi[i] * m; } return vo; }
	static SensorFrame& operator+=(SensorFrame& vi, const SensorFrame& va) { for(size_t i = 0; i < vi.size(); ++i) { vi[i] += va[i]; } return vi; }
	static SensorFrame& operator-=(SensorFrame& vi, const SensorFrame& va) { for(size_t i = 0; i < vi.size(); ++i) { vi[i] -= va[i]; } return vi; }

	// AverageBuffer
	// average sensor inputs occurring every 5 seconds over a 10-minute period
	struct AverageBuffer
	{
		static constexpr size_t BufferSize = 10 * 60 / 5; // capcity: 10min, 1event/5sec
		static constexpr size_t TotalBytes = BufferSize * sizeof(SensorFrame);
		SensorFrame* bufferPtr = nullptr;
		uint32_t whead = 0, wlen = 0;
		SensorFrame sum{};
		std::recursive_mutex mtx;
		AverageBuffer()
		{
			bufferPtr = (SensorFrame*)xheap::memAlloc(TotalBytes);
			if(!bufferPtr) { DEBUGPRINTW("AverageBuffer: malloc(%zu) failed", TotalBytes); }
		}
		~AverageBuffer()
		{
			xheap::memFree(bufferPtr);
		}
		void reset()
		{
			std::lock_guard sl(mtx);
			whead = wlen = 0;
			sum = {};
		}
		void push(const SensorFrame& frm)
		{
			std::lock_guard sl(mtx);
			if(!bufferPtr) { return; }
			bufferPtr[whead] = SensorFrameDenan(frm);
			sum += bufferPtr[whead];
			++whead; if(BufferSize <= whead) { whead = 0; }
			++wlen; if(BufferSize <= wlen) { popInternal(); }
		}
		void popInternal()
		{
			if(!bufferPtr) { return; }
			if(!wlen) { return; }
			int rhead = (int)whead - (int)wlen; if(rhead < 0) { rhead += BufferSize; }
			sum -= bufferPtr[rhead];
			--wlen;
		}
		SensorFrame getAverage()
		{
			std::lock_guard sl(mtx);
			if(!bufferPtr || !wlen) { return {}; }
			return sum * (1.0f / (float)wlen);
		}
	};

	// HistoryBuffer
	// stores 90 days' worth of data recoded at 10-minute intervals
	// estimation of the amount of data to be written: 207360B/10min i.e. 28.5MB/day, 10.2GB/year
	// NOTE: if an RTC reset occurs, the timestamps may not be monotonically increasing
	using ExtRecordT = DataLogger::SensorHistoryRecord;
	struct IntRecordT
	{
		time_t dt;
		int16_t tempx100; // 0
		int16_t pres; // 2
		int16_t co2; // 3
		int8_t hum; // 1
		int8_t iaq; // 4
		IntRecordT() = default;
		IntRecordT(const ExtRecordT& x)
		{
			dt = x.dt;
			tempx100 = (int16_t)(x.values[DataLogger::IsfTemp] * 100 + 0.5f);
			hum = (int8_t)(x.values[DataLogger::IsfHum] + 0.5f);
			pres = (int16_t)(x.values[DataLogger::IsfPres] + 0.5f);
			co2 = (int16_t)(x.values[DataLogger::IsfCo2] + 0.5f);
			iaq = (int8_t)(x.values[DataLogger::IsfIaq] + 0.5f);
		}
		operator ExtRecordT() const
		{
			ExtRecordT x{};
			x.dt = dt;
			x.values[DataLogger::IsfTemp] = (float)tempx100 * 0.01f;
			x.values[DataLogger::IsfHum] = (float)hum;
			x.values[DataLogger::IsfPres] = (float)pres;
			x.values[DataLogger::IsfCo2] = (float)co2;
			x.values[DataLogger::IsfIaq] = (float)iaq;
			return x;
		}
	};
	static constexpr size_t BufferSize = 90 * 24 * 60 / 10; // capacity: 90days, 144events/day
	static constexpr size_t TotalBytes = BufferSize * sizeof(IntRecordT);
	static constexpr uint32_t Signature = fcc::fccFromStr("sshb");
	static constexpr uint32_t Version = 1;
	using HistoryBuffer = HistoryBufferBaseT<ExtRecordT, IntRecordT, DataLogger::SensorSnapshot, BufferSize, Signature, Version>;

	// --------------------------------------------------------------------------------

	class Logger : private SensorTask::Listener
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
		SensorTask* sensorTask;
		AsyncQueue<DataLogger::SensorHistoryRecord, 8> asyncQueue;
		AverageBuffer averageBuffer;
		HistoryBuffer historyBuffer;
		strutil::Str<128> persistencePath;
		time_t lastTime = 0;
		const bool autoPersist = false;
		bool active = false;
	public:
		Logger() = delete;
		Logger(Dispatcher* queuedisp, SensorTask* p, bool autopersist)
			: sensorTask(p)
			, asyncQueue(queuedisp, [this]() { processPendingRecordsAsync(); })
			, autoPersist(autopersist)
		{
			persistencePath << BoardFs::sdcardGetRoot()<< "/srhb.dat";
			if(autoPersist && BoardFs::sdcardEnsureMounted()) { historyBuffer.load(persistencePath); }
			if(sensorTask) { sensorTask->addListener(this); }
		}
		~Logger() override
		{
			if(sensorTask) { sensorTask->removeListener(this); }
		}
		void reset()
		{
			averageBuffer.reset();
			historyBuffer.reset();
			lastTime = 0;
		}
		bool load(const char* path)
		{
			return historyBuffer.load(path);
		}
		bool save(const char* path)
		{
			return historyBuffer.save(path);
		}
		void activate()
		{
			active = true;
		}
		void deactivate()
		{
			active = false;
		}
		xheap::uniquePtr<DataLogger::SensorSnapshot> createSnapshot(time_t tfrom, time_t tto)
		{
			return historyBuffer.createSnapshot(tfrom, tto);
		}
		void processRecord(const DataLogger::SensorHistoryRecord& rec)
		{
			averageBuffer.push(rec.values);
			time_t trec = normalizeTime(rec.dt);
			if(lastTime <= 0) { lastTime = trec; }
			if(lastTime != trec)
			{
				if(autoPersist && BoardFs::sdcardEnsureMounted()) { historyBuffer.save(persistencePath); }
				lastTime = trec;
				historyBuffer.push(DataLogger::SensorHistoryRecord{ trec, averageBuffer.getAverage() });
				// DEBUGPRINTI("Logger(Sensor): record new temp=%.1f hum=%d pres=%d co2=%d iaq=%d", rec.values[DataLogger::IsfTemp], (int)rec.values[DataLogger::IsfHum], (int)rec.values[DataLogger::IsfPres], (int)rec.values[DataLogger::IsfCo2], (int)rec.values[DataLogger::IsfIaq]);
			}
		}
	private:
		void processPendingRecordsAsync()
		{
			DataLogger::SensorHistoryRecord rec;
			while(asyncQueue.pop(&rec)) { processRecord(rec); }
		}
		// SensorTask::Listener
		void sensorTaskValuesDidChange(SensorTask*, SensorTask::Fupdates, const SensorTask::Values& val) override
		{
			if(!active) { return; }
			DataLogger::SensorHistoryRecord rec{};
			rec.dt = time(nullptr);
			rec.values[DataLogger::IsfTemp] = val[SensorTask::Ibme680Temp];
			rec.values[DataLogger::IsfHum ] = val[SensorTask::Ibme680Hum ];
			rec.values[DataLogger::IsfPres] = val[SensorTask::Ibme680Pres];
			rec.values[DataLogger::IsfCo2 ] = val[SensorTask::Iscd4xCo2  ];
			rec.values[DataLogger::IsfIaq ] = val[SensorTask::Ibme680Iaq ];
			asyncQueue.push(rec);
		}
	};

} // namespace sensorhistory

// ================================================================================
// weather history

namespace weatherhistory
{
	// data flow and threads:
	// 
	// thread:                main                 queue dispatch             external
	// block : [WeatherTask] ----> [Logger [AsyncQueue] ----> [HistoryBuffer]] ----> [external snapshot client]

	// HistoryBuffer
	// stores 90 days' worth of data recoded at 1-hour intervals
	// estimation of the amount of data to be written: 69120B/hour i.e. 1.58MB/day, 577MB/year
	using ExtRecordT = DataLogger::WeatherHistoryRecord;
	struct IntRecordT
	{
		time_t dt;
		int16_t tempX100;
		int16_t pressureHpa;
		int8_t humidityPct;
		int8_t cloudsPct;
		int16_t windDeg;
		int16_t windSpeedX100;
		int16_t weatherId;
		strutil::Str<12> weatherIcon;
		IntRecordT(const ExtRecordT& x)
		{
			dt = x.dt;
			tempX100 = (int16_t)(x.tempC * 100 + 0.5);
			pressureHpa = x.pressureHpa;
			humidityPct = (int8_t)x.humidityPct;
			cloudsPct = (int8_t)x.cloudsPct;
			windDeg = x.windDeg;
			windSpeedX100 = (int16_t)(x.windSpeedMps * 100 + 0.5f);
			weatherId = x.weatherId;
			weatherIcon = x.weatherIcon;
		}
		operator ExtRecordT() const
		{
			ExtRecordT x{};
			x.dt = dt;
			x.tempC = (float)tempX100 * 0.01f;
			x.pressureHpa = pressureHpa;
			x.humidityPct = humidityPct;
			x.cloudsPct = cloudsPct;
			x.windDeg = windDeg;
			x.windSpeedMps = (float)windSpeedX100 * 0.01f;
			x.weatherId = weatherId;
			x.weatherIcon = weatherIcon;
			return x;
		}
	};
	static constexpr size_t BufferSize = 90 * 24; // capacity: 90days, 24event/day
	static constexpr size_t TotalBytes = BufferSize * sizeof(IntRecordT);
	static constexpr uint32_t Signature = fcc::fccFromStr("wthb");
	static constexpr uint32_t Version = 1;
	using HistoryBuffer = HistoryBufferBaseT<ExtRecordT, IntRecordT, DataLogger::WeatherSnapshot, BufferSize, Signature, Version>;

	// --------------------------------------------------------------------------------

	class Logger : private WeatherTask::Listener
	{
	private:
		// quantize in 1-hours intervals
		static time_t normalizeTime(time_t t)
		{
			tm stm{};
			const tm* ptm = platform::localtime(&t, &stm);
			if(!ptm) { return t; }
			tm tm = *ptm;
			tm.tm_min = tm.tm_sec = 0;
			return mktime(&tm);
		}
		WeatherTask* weatherTask;
		AsyncQueue<DataLogger::WeatherHistoryRecord, 8> asyncQueue;
		HistoryBuffer historyBuffer;
		strutil::Str<128> persistencePath;
		time_t lastTime = 0;
		const bool autoPersist = false;
		bool active = false;
	public:
		Logger() = delete;
		Logger(Dispatcher* queuedisp, WeatherTask* p, bool autopersist)
			: weatherTask(p)
			, asyncQueue(queuedisp, [this]() { processPendingRecordsAsync(); })
			, autoPersist(autopersist)
		{
			persistencePath << BoardFs::sdcardGetRoot() << "/wrhb.dat";
			if(autoPersist && BoardFs::sdcardEnsureMounted()) { historyBuffer.load(persistencePath); }
			if(weatherTask) { weatherTask->addListener(this); }
		}
		~Logger() override
		{
			if(weatherTask) { weatherTask->removeListener(this); }
		}
		void reset()
		{
			historyBuffer.reset();
			lastTime = 0;
		}
		bool load(const char* path)
		{
			return historyBuffer.load(path);
		}
		bool save(const char* path)
		{
			return historyBuffer.save(path);
		}
		void activate()
		{
			active = true;
		}
		void deactivate()
		{
			active = false;
		}
		xheap::uniquePtr<DataLogger::WeatherSnapshot> createSnapshot(time_t tfrom, time_t tto)
		{
			return historyBuffer.createSnapshot(tfrom, tto);
		}
		void processRecord(const DataLogger::WeatherHistoryRecord& rrec)
		{
			DataLogger::WeatherHistoryRecord rec(rrec);
			rec.dt = normalizeTime(rec.dt);
			if(lastTime <= 0) { lastTime = rec.dt; }
			if(lastTime != rec.dt)
			{
				if(autoPersist && BoardFs::sdcardEnsureMounted()) { historyBuffer.save(persistencePath); }
				lastTime = rec.dt;
				historyBuffer.push(rec);
				// DEBUGPRINTI("Logger(Wheather): record new temp=%.1f pres=%d hum=%d clouds=%d", rec.tempC, rec.pressureHpa, rec.humidityPct, rec.cloudsPct);
			}
			else
			{
				historyBuffer.replaceLast(rec);
				// DEBUGPRINTI("Logger(Wheather): record update temp=%.1f pres=%d hum=%d clouds=%d", rec.tempC, rec.pressureHpa, rec.humidityPct, rec.cloudsPct);
			}
		}
	private:
		void processPendingRecordsAsync()
		{
			DataLogger::WeatherHistoryRecord rec;
			while(asyncQueue.pop(&rec)) { processRecord(rec); }
		}
		// WeatherTask::Listener
		void weatherTaskResultDidChange(WeatherTask*, const WeatherQuery::Result& res) override
		{
			if(!active) { return; }
			if(res.apiStatus != 200) { return; }
			DataLogger::WeatherHistoryRecord rec{};
			rec.dt = res.current.dt;
			rec.tempC = res.current.tempC;
			rec.pressureHpa = res.current.pressureHpa;
			rec.humidityPct = res.current.humidityPct;
			rec.cloudsPct = rec.cloudsPct;
			rec.windDeg = rec.windDeg;
			rec.windSpeedMps = res.current.windSpeedMps;
			rec.weatherId = res.current.weatherId;
			rec.weatherIcon = res.current.weatherIcon;
			asyncQueue.push(rec);
		}
	};

} // namespace weatherhistory

// ================================================================================
// DataLogger

class DataLoggerImpl : public DataLogger
{
private:
	xheap::uniquePtr<Dispatcher> queueDispatcher = Dispatcher::createInstance();
	sensorhistory::Logger sensorLogger;
	weatherhistory::Logger weatherLogger;
	bool active = false;
public:
	DataLoggerImpl() = delete;
	DataLoggerImpl(SensorTask* st, WeatherTask* wt) : sensorLogger(queueDispatcher.get(), st, true), weatherLogger(queueDispatcher.get(), wt, true)
	{
	}
	~DataLoggerImpl() override
	{
	}
	void resetHistory() override
	{
		sensorLogger.reset();
		weatherLogger.reset();
	}
	bool isActive() const override
	{
		return active;
	}
	bool activate() override
	{
		active = true;
		sensorLogger.activate();
		weatherLogger.activate();
		return true;
	}
	void deactivate() override
	{
		active = false;
		weatherLogger.deactivate();
		sensorLogger.deactivate();
	}
	xheap::uniquePtr<SensorSnapshot> createSensorSnapshot(time_t tfrom, time_t tto) override
	{
		return sensorLogger.createSnapshot(tfrom, tto);
	}
	virtual xheap::uniquePtr<WeatherSnapshot> createWeatherSnapshot(time_t tfrom, time_t tto) override
	{
		return weatherLogger.createSnapshot(tfrom, tto);
	}
};

xheap::uniquePtr<DataLogger> DataLogger::createInstance(SensorTask* st, WeatherTask* wt)
{
	return xheap::makeUnique<DataLogger, DataLoggerImpl>(st, wt);
}

// ================================================================================

#if __has_include("modules/DataLogger.test.h")
#include "modules/DataLogger.test.h"
#endif
