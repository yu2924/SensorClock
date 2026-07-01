#define DATALOGGER_TEST 0

#if DATALOGGER_TEST && defined(_WINDOWS)
#include <numbers>
#include <random>
namespace testdatalogger
{
	struct ScopedFILE
	{
		FILE* file;
		ScopedFILE(FILE* p) : file(p) {}
		~ScopedFILE() { if(file) fclose(file); }
		operator FILE* () { return file; }
	};
	bool areFilesIdentical(const char* path1, const char* path2)
	{
		bool failed = false;
		ScopedFILE file1(fopen(path1, "rb"));
		ScopedFILE file2(fopen(path2, "rb"));
		if(!file1 || !file2) return false;
		uint8_t buf1[1024], buf2[1024];
		while(true)
		{
			size_t r1 = fread(buf1, 1, sizeof(buf1), file1);
			size_t r2 = fread(buf2, 1, sizeof(buf2), file2);
			if(r1 != r2) return false;
			if(memcmp(buf1, buf2, r1) != 0) return false;
			if((r1 < sizeof(buf1)) || (r2 < sizeof(buf2))) { break; }
		}
		return true;
	}
	static time_t makeTimeWithCalendarTime(int yyyy, int mm, int dd, int h, int m, int s)
	{
		tm stm{};
		stm.tm_year = yyyy - 1900;
		stm.tm_mon = mm - 1;
		stm.tm_mday = dd;
		stm.tm_hour = h;
		stm.tm_min = m;
		stm.tm_sec = s;
		return mktime(&stm);
	}
	// This test verifies the sensorhistory::Logger's functionality by following the steps below:
	// 1. inject test data to the logger
	// 2. write snapshot to a file
	// 3. verify the persistence functionality through save and load operations
	// 4. write snapshot to a file, again
	// 5. verify whether two files match
	// This test writes the following two files to the current directory:
	// - test_hist_sensor-1.tsv
	// - test_hist_sensor-2.tsv
	// check points:
	// - Since the data input period is 120 days, but the logger's recording period is 90 days, the first 30 days' worth of data should be discarded.
	// - Since this test varies the input values in 30-day cycles, the recorded data should show fluctuations over three cycles.
	// - Temperature values should be quantized to 0.01. All other values are integers.
	bool testSensor()
	{
		xheap::uniquePtr<Dispatcher> disp = Dispatcher::createInstance();
		sensorhistory::Logger logger(disp.get(), nullptr, true);
		// 1. inject test data to the logger
		{
			logger.reset();
			logger.activate();
			std::random_device seed_gen;
			std::uint32_t seed = seed_gen();
			std::mt19937 engine(seed);
			std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
			// 120days, 5sec interval
			const size_t L = 120 * 24 * 60 * 60 / 5;
			const time_t tstep = 5; // 5sec step
			const double phstep = 2 * std::numbers::pi / (double)(30 * 24 * 60 * 60 / tstep); // 1cycle / 30days
			time_t tcur = makeTimeWithCalendarTime(2026, 1, 1, 0, 0, 0);
			double ph = 0;
			DataLogger::SensorHistoryRecord rec{};
			for(size_t i = 0; i < L; ++i)
			{
				float v = 20 + 10 * std::sinf((float)ph) + dist(engine);
				rec.dt = tcur;
				rec.values = { v + 0, v + 5, v + 10, v + 15, v + 20 }; // co2, temp, pres, hum, iaq
				logger.processRecord(rec);
				tcur += tstep;
				ph += phstep;
			}
		}
		// 2. write snapshot to a file
		{
			auto snapshot = logger.createSnapshot(0, std::numeric_limits<time_t>::max());
			ScopedFILE file(fopen("test_hist_sensor-1.tsv", "wb"));
			if(file)
			{
				fprintf(file, "datetime\ttemp\thum\tpres\tco2\tiaq\n");
				DataLogger::SensorHistoryRecord rec;
				char ts[32];
				for(size_t c = snapshot->getNumRecords(), i = 0; i < c; ++i)
				{
					if(!snapshot->getRecordAt(i, &rec)) break;
					tm stm{};
					const tm* ptm = platform::localtime(&rec.dt, &stm);
					strftime(ts, std::size(ts), "%Y-%m-%d %H:%M:%S", ptm);
					fprintf(file, "%s\t%g\t%g\t%g\t%g\t%g\n", ts, rec.values[0], rec.values[1], rec.values[2], rec.values[3], rec.values[4]);
				}
			}
		}
		// 3. verify the persistence functionality through save and load operations
		{
			logger.save("test_hist_sensor.per");
			logger.load("test_hist_sensor.per");
		}
		// 4. write snapshot to a file, again
		{
			auto snapshot = logger.createSnapshot(0, std::numeric_limits<time_t>::max());
			ScopedFILE file(fopen("test_hist_sensor-2.tsv", "wb"));
			if(file)
			{
				fprintf(file, "datetime\ttemp\thum\tpres\tco2\tiaq\n");
				DataLogger::SensorHistoryRecord rec;
				char ts[32];
				for(size_t c = snapshot->getNumRecords(), i = 0; i < c; ++i)
				{
					if(!snapshot->getRecordAt(i, &rec)) break;
					tm stm{};
					const tm* ptm = platform::localtime(&rec.dt, &stm);
					if(!ptm) { DEBUGPRINTW("TestSensor: post persistence: broken timestamp"); break; }
					strftime(ts, std::size(ts), "%Y-%m-%d %H:%M:%S", ptm);
					fprintf(file, "%s\t%g\t%g\t%g\t%g\t%g\n", ts, rec.values[0], rec.values[1], rec.values[2], rec.values[3], rec.values[4]);
				}
			}
		}
		// 5. verify whether two files match
		{
			bool ident = areFilesIdentical("test_hist_sensor-1.tsv", "test_hist_sensor-2.tsv");
			platform::debugPrint(ident ? 'I' : 'W', "sensor filecomp: %s", ident ? "succeeded" : "failed");
		}
		return true;
	}
	// This test verifies the wheatherhistory::Logger's functionality by following the steps below:
	// 1. inject test data to the logger
	// 2. write snapshot to a file
	// 3. verify the persistence functionality through save and load operations
	// 4. write snapshot to a file, again
	// 5. verify whether two TSV files match
	// This test writes the following two files to the current directory:
	// - test_hist_weather-1.tsv
	// - test_hist_weather-2.tsv
	// check points:
	// - Since the data input period is 120 days, but the logger's recording period is 90 days, the first 30 days' worth of data should be discarded.
	// - Since this test varies the input values in 30-day cycles, the recorded data should show fluctuations over three cycles.
	// - Temperature and wind speed should be quantized to 0.01. All other values are integers.
	bool testWeather()
	{
		xheap::uniquePtr<Dispatcher> disp = Dispatcher::createInstance();
		weatherhistory::Logger logger(disp.get(), nullptr, true);
		// 1. inject test data to the logger
		{
			logger.reset();
			logger.activate();
			// 120days, 30min interval
			const size_t L = 120 * 24 * 60 * 60 / (30 * 60);
			const time_t tstep = 30 * 60; // 30min step
			const double phstep = 2 * std::numbers::pi / (double)(30 * 24 * 60 * 60 / tstep); // 1cycle / 30days
			time_t tcur = makeTimeWithCalendarTime(2026, 1, 1, 0, 0, 0);
			double ph = 0;
			DataLogger::WeatherHistoryRecord rec{};
			rec.dt = tcur;
			rec.tempC = 20;
			rec.pressureHpa = 1000;
			rec.humidityPct = 50;
			rec.cloudsPct = 10;
			rec.windSpeedMps = 0;
			rec.windDeg = 0;
			rec.weatherId = 800;
			rec.weatherIcon = "01";
			for(size_t id = 0; id < L; ++id)
			{
				float v = 25 + 5 * std::sinf((float)ph);
				rec.dt = tcur;
				rec.tempC = v;
				rec.windSpeedMps = v + 5;
				rec.windDeg = (int16_t)(v + 15);
				rec.humidityPct = (int16_t)(v + 10);
				logger.processRecord(rec);
				tcur += tstep;
				ph += phstep;
			}
		}
		// 2. write snapshot to a file
		{
			auto snapshot = logger.createSnapshot(0, std::numeric_limits<time_t>::max());
			ScopedFILE file(fopen("test_hist_weather-1.tsv", "wb"));
			if(file)
			{
				fprintf(file, "datetime\ttempC\twindspeed\twinddeg\thumidity\n");
				DataLogger::WeatherHistoryRecord rec;
				char ts[32];
				for(size_t c = snapshot->getNumRecords(), i = 0; i < c; ++i)
				{
					if(!snapshot->getRecordAt(i, &rec)) break;
					tm stm{};
					const tm* ptm = platform::localtime(&rec.dt, &stm);
					strftime(ts, std::size(ts), "%Y-%m-%d %H:%M:%S", ptm);
					fprintf(file, "%s\t%g\t%g\t%d\t%d\n", ts, rec.tempC, rec.windSpeedMps, rec.windDeg, rec.humidityPct);
				}
			}
		}
		// 3. verify the persistence functionality through save and load operations
		{
			logger.save("test_hist_weather.per");
			logger.load("test_hist_weather.per");
		}
		// 4. write snapshot to a file, again
		{
			auto snapshot = logger.createSnapshot(0, std::numeric_limits<time_t>::max());
			ScopedFILE file(fopen("test_hist_weather-2.tsv", "wb"));
			if(file)
			{
				fprintf(file, "datetime\ttempC\twindspeed\twinddeg\thumidity\n");
				DataLogger::WeatherHistoryRecord rec;
				char ts[32];
				for(size_t c = snapshot->getNumRecords(), i = 0; i < c; ++i)
				{
					if(!snapshot->getRecordAt(i, &rec)) break;
					tm stm{};
					const tm* ptm = platform::localtime(&rec.dt, &stm);
					if(!ptm) { DEBUGPRINTW("WeatherTest: post persistence: broken timestamp"); break; }
					strftime(ts, std::size(ts), "%Y-%m-%d %H:%M:%S", ptm);
					fprintf(file, "%s\t%g\t%g\t%d\t%d\n", ts, rec.tempC, rec.windSpeedMps, rec.windDeg, rec.humidityPct);
				}
			}
		}
		// 5. verify whether two files match
		{
			bool ident = areFilesIdentical("test_hist_weather-1.tsv", "test_hist_weather-2.tsv");
			platform::debugPrint(ident ? 'I' : 'W', "weather filecomp: %s", ident ? "succeeded" : "failed");
		}
		return true;
	}
	static bool rs = testdatalogger::testSensor();
	static bool rw = testdatalogger::testWeather();
} // namespace testdatalogger
#endif
