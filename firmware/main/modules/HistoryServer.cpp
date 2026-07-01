//
//  HistoryServer.cpp
//  SensorClock
//
//  created by yu2924 on 2026-04-21
//

#include "HistoryServer.h"

#include "utilities/heapbuffer.h"
#include "utilities/listenerlist.h"
#include "utilities/platform.h"
#include "utilities/strutil.h"

#if defined(ESP_PLATFORM)
static constexpr uint16_t DefaultServerPort = 80;
#include "modules/HttpServerCore_esp32.h"
#else
static constexpr uint16_t DefaultServerPort = 8080;
#include "modules/HttpServerCore_proto.h"
#endif

class HistoryServerImpl : public HistoryServer, private IHTTPContentHandler
{
private:
	struct RespBuf
	{
		HeapBufferT<char> buf;
		RespBuf() = delete;
		RespBuf(size_t len) noexcept
		{
			if(buf.resize(len) && (0 < len)) { buf[0] = 0; }
			else { DEBUGPRINTW("HistoryServer: malloc(%zu) failed", len); }
		}
		operator const char* () const noexcept { return buf.data(); }
		size_t length() const noexcept { return !buf.empty() ? strlen(buf.data()) : 0; }
		int format(const char* fmt, ...) noexcept { if(buf.empty()) { return 0; }  va_list va; va_start(va, fmt); int rl = vsnprintf(buf.data(), buf.size(), fmt, va); va_end(va); return rl; }
		int addFormat(const char* fmt, ...) noexcept { if(buf.empty()) { return 0; }  va_list va; va_start(va, fmt); size_t len = strlen(buf.data()); int rl = vsnprintf(buf.data() + len, buf.size() - len, fmt, va); va_end(va); return rl; }
	};
	ListenerListT<Listener> listenerList;
	Config config{};
	DataLogger* dataLogger;
	HttpServerCore httpServerCore;
public:
	HistoryServerImpl() = delete;
	HistoryServerImpl(DataLogger* p) : dataLogger(p)
	{
		config.hostname = "SensorClock";
		config.port = DefaultServerPort;
		httpServerCore.setCredentials(config.username, config.password);
		httpServerCore.addContentPathAndMethod("/sensor", HTTP_GET);
		httpServerCore.addContentPathAndMethod("/weather", HTTP_GET);
	}
	~HistoryServerImpl() override
	{
		httpServerCore.stop();
	}
	void addListener(Listener* p) override
	{
		listenerList.add(p);
	}
	void removeListener(Listener* p) override
	{
		listenerList.remove(p);
	}
	const Config& getConfig() const override
	{
		return config;
	}
	void setConfig(const Config& v) override
	{
		if(config == v) { return; }
		bool needsrestart = httpServerCore.isRunning() && ((config.hostname != v.hostname) || (config.port != v.port));
		config = v;
		httpServerCore.setCredentials(config.username, config.password);
		listenerList.call(&Listener::historyServerConfigDidChange, this, config);
		if(needsrestart)
		{
			httpServerCore.stop();
			httpServerCore.start(config.hostname, config.port, this);
		}
	}
	bool isActive() const override
	{
		return httpServerCore.isRunning();
	}
	bool activate() override
	{
		return httpServerCore.start(config.hostname, config.port, this);
	}
	void deactivate() override
	{
		httpServerCore.stop();
	}
private:
	bool sendStaticResponse(HTTPContentContext* ctx, const char* statustext, const char* mimetype, const char* body)
	{
		ctx->setResponseHeader(statustext, mimetype);
		return ctx->sendSimpleResponse(body, strlen(body));
	}
	// IHTTPContentHandler
	bool handleContent(HTTPContentContext* ctx) override
	{
		if(ctx->getRequestMethod() == HTTP_GET)
		{
			if(strutil::areEqualNoCase(ctx->getRequestPath(), "/sensor"))
			{
				const char* pfrom = ctx->lookupRequestQueryParameter("from");
				time_t tfrom = pfrom ? strtoull(pfrom, nullptr, 0) : 0;
				const char* pto = ctx->lookupRequestQueryParameter("to");
				time_t tto = pto ? strtoull(pto, nullptr, 0) : std::numeric_limits<time_t>::max();
				auto snapshot = dataLogger->createSensorSnapshot(tfrom, tto);
				RespBuf resp(1024);
				if(!snapshot || !resp)
				{
					return sendStaticResponse(ctx, "500 Internal Server Error", "application/json", R"({"status":"500 Internal Server Error"})");
				}
				ctx->setResponseHeader("200 OK", "application/json");
				size_t numrec = snapshot->getNumRecords();
				resp.format(R"({"status":"200 OK", "path":"/sensor", )");
				if(pfrom) resp.addFormat(R"("from":"%lld", )", tfrom);
				if(pto) resp.addFormat(R"("to":"%lld", )", tto);
				resp.addFormat(R"("length":"%zu", "data":[)", numrec);
				if(!ctx->sendChunkedResponse(resp, resp.length())) { return false; }
				for(size_t i = 0; i < numrec; ++i)
				{
					DataLogger::SensorHistoryRecord rec;
					if(!snapshot->getRecordAt(i, &rec)) { break; }
					const char* fmt = R"(%s)"
						R"({"dt":"%lld", )"
						R"("temp":"%.1f", "humidity":"%d", "pressure":"%d", "co2":"%d", "iaq":"%d"})";
					resp.format(fmt,
						(0 < i) ? ", " : "",
						rec.dt,
						rec.values[DataLogger::IsfTemp], (int)rec.values[DataLogger::IsfHum], (int)rec.values[DataLogger::IsfPres], (int)rec.values[DataLogger::IsfCo2], (int)rec.values[DataLogger::IsfIaq]);
					if(!ctx->sendChunkedResponse(resp, resp.length())) { return false; }
				}
				resp.format("]}");
				if(!ctx->sendChunkedResponse(resp, resp.length())) { return false; }
				if(!ctx->sendChunkedResponse(nullptr, 0)) { return false; }
				return true;
			}
			else if(strutil::areEqualNoCase(ctx->getRequestPath(), "/weather"))
			{
				const char* pfrom = ctx->lookupRequestQueryParameter("from");
				time_t tfrom = pfrom ? strtoull(pfrom, nullptr, 0) : 0;
				const char* pto = ctx->lookupRequestQueryParameter("to");
				time_t tto = pto ? strtoull(pto, nullptr, 0) : std::numeric_limits<time_t>::max();
				auto snapshot = dataLogger->createWeatherSnapshot(tfrom, tto);
				RespBuf resp(1024);
				if(!snapshot || !resp)
				{
					return sendStaticResponse(ctx, "500 Internal Server Error", "application/json", R"({"status":"500 Internal Server Error"})");
				}
				ctx->setResponseHeader("200 OK", "application/json");
				size_t numrec = snapshot->getNumRecords();
				resp.format(R"({"status":"200 OK", "path":"/weather", )");
				if(pfrom) resp.addFormat(R"("from":"%lld", )", tfrom);
				if(pto) resp.addFormat(R"("to":"%lld", )", tto);
				resp.addFormat(R"("length":"%zu", "data":[)", numrec);
				if(!ctx->sendChunkedResponse(resp, resp.length())) { return false; }
				for(size_t i = 0; i < numrec; ++i)
				{
					DataLogger::WeatherHistoryRecord rec;
					if(!snapshot->getRecordAt(i, &rec)) { break; }
					const char* fmt = R"(%s)"
						R"({"dt":"%lld", )"
						R"("temp":"%.1f", "pressure":"%d", "humidity":"%d", "cloud":"%d", )"
						R"("wind_speed":"%.2f", "wind_deg":"%d", "weather_id":"%d", "weather_icon":"%s"})";
					resp.format(fmt,
						(0 < i) ? ", " : "",
						rec.dt,
						rec.tempC, rec.pressureHpa, rec.humidityPct, rec.cloudsPct,
						rec.windSpeedMps, rec.windDeg, rec.weatherId, (const char*)rec.weatherIcon);
					if(!ctx->sendChunkedResponse(resp, resp.length())) { return false; }
				}
				resp.format("]}");
				if(!ctx->sendChunkedResponse(resp, resp.length())) { return false; }
				if(!ctx->sendChunkedResponse(nullptr, 0)) { return false; }
				return true;
			}
			else
			{
				return sendStaticResponse(ctx, "404 Not Found", "application/json", R"({"status":"404 Not Found"})");
			}
		}
		else
		{
			return sendStaticResponse(ctx, "405 Method Not Allowed", "application/json", R"({"status":"405 Method Not Allowed"})");
		}
	}
};

xheap::uniquePtr<HistoryServer> HistoryServer::createInstance(DataLogger* p)
{
	return xheap::makeUnique<HistoryServer, HistoryServerImpl>(p);
}
