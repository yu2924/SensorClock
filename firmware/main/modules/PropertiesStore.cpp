//
//  PropertiesStore.cpp
//  SensorClock
//
//  created by yu2924 on 2025-12-07
//

#include "PropertiesStore.h"

#if defined(_WINDOWS)

// ================================================================================
// Prototype code for Windows
// It uses an INI-style text file located in the current folder for storage.

#include <map>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include "utilities/platform.h"
#include "modules/Dispatcher.h"

static std::string StringTrim(const std::string& s, const std::string& delim)
{
	if(s.length() == 0) return s;
	size_t i0 = s.find_first_not_of(delim);
	if(i0 == std::string::npos) return std::string();
	size_t i1 = s.find_last_not_of(delim);
	return std::string(s, i0, i1 - i0 + 1);
}

class PropertiesStoreImpl : public PropertiesStore
{
public:
	std::string path;
	std::map<std::string, std::string> kvmap;
	xheap::uniquePtr<DispatchTimer> timer = DispatchTimer::createInstance();
	bool dirty = false;
	PropertiesStoreImpl()
	{
	}
	~PropertiesStoreImpl() override
	{
		close();
	}
	void setDirty()
	{
		dirty = true;
		timer->start(3000, [this]() { timer->stop(); flush(); });
	}
	bool open(const char* sect) override
	{
		close();
		bool r = true;
		path = std::filesystem::absolute(std::string(sect) + ".txt").string();
		try
		{
			std::fstream str(path, std::ios::binary | std::ios::in);
			std::string line;
			while(std::getline(str, line))
			{
				line = StringTrim(line, "\r\n");
				if(line.empty() || (line[0] == ';') || (line[0] == '#')) continue;
				if(line.find('=') == std::string::npos) continue;
				std::stringstream linestr(line);
				std::string key; std::getline(linestr, key, '=');
				if(key.empty()) continue;
				std::string val; std::getline(linestr, val, '=');
				kvmap[key] = val;
			}
		}
		catch(std::exception&)
		{
			r = false;
		}
		return r;
	}
	bool flush() override
	{
		if(!dirty) return true;
		bool r = true;
		try
		{
			std::fstream str(path, std::ios::binary | std::ios::out | std::ios::trunc);
			str << "#\r\n";
			for(const auto& kv : kvmap)
			{
				str << kv.first << "=" << kv.second << "\r\n";
			}
			dirty = false;
			DEBUGPRINTI("PropertiesStore: flush");
		}
		catch(std::exception&)
		{
			r = false;
		}
		return r;
	}
	void close() override
	{
		flush();
		kvmap.clear();
		path.clear();
	}
	bool getBoolValue(const char* key, bool vdef) const override
	{
		return getIntValue(key, vdef ? 1 : 0) ? true : false;
	}
	void setBoolValue(const char* key, bool v)
	{
		setIntValue(key, v ? 1 : 0);
	}
	int getIntValue(const char* key, int vdef) const override
	{
		auto it = kvmap.find(key);
		if(it == kvmap.end()) return vdef;
		return atoi(it->second.c_str());
	}
	void setIntValue(const char* key, int v)
	{
		char s[16]{}; snprintf(s, std::size(s), "%d", v);
		kvmap[key] = s;
		setDirty();
	}
	double getDoubleValue(const char* key, double vdef) const override
	{
		auto it = kvmap.find(key);
		if(it == kvmap.end()) return vdef;
		return strtod(it->second.c_str(), nullptr);
	}
	void setDoubleValue(const char* key, double v) override
	{
		char s[16]{}; snprintf(s, std::size(s), "%g", v);
		kvmap[key] = s;
		setDirty();
	}
	size_t getStringValue(const char* key, const char* vdef, char* buf, size_t buflen) const override
	{
		auto it = kvmap.find(key);
		return strutil::copy(buf, buflen, (it != kvmap.end()) ? it->second.c_str() : vdef);
	}
	void setStringValue(const char* key, const char* v) override
	{
		kvmap[key] = v;
		setDirty();
	}
};

xheap::uniquePtr<PropertiesStore> PropertiesStore::createInstance()
{
	return xheap::makeUnique<PropertiesStore, PropertiesStoreImpl>();
}

#elif defined(CONFIG_IDF_TARGET)

// ================================================================================
// Production code for ESP32

#include <esp_log.h>
#include <nvs_flash.h>
#include "modules/Dispatcher.h"

static const char* TagPropertiesStore = "PropertiesStore";

class PropertiesStoreImpl : public PropertiesStore
{
private:
	static constexpr nvs_handle_t HNVS_NULL = 0;
	nvs_handle_t hNvs = HNVS_NULL;
	xheap::uniquePtr<DispatchTimer> timer = DispatchTimer::createInstance();
	bool dirty = false;
public:
	PropertiesStoreImpl()
	{
	}
	~PropertiesStoreImpl() override
	{
		close();
	}
	void setDirty()
	{
		dirty = true;
		timer->start(3000, [this]() { timer->stop(); flush(); });
	}
	bool open(const char* sect) override
	{
		close();
		return nvs_open(sect, NVS_READWRITE, &hNvs) == ESP_OK;
	}
	bool flush() override
	{
		if(!hNvs) { return false; }
		if(dirty) { if(nvs_commit(hNvs) != ESP_OK) return false; }
		dirty = false;
		ESP_LOGI(TagPropertiesStore, "flush");
		return true;
	}
	void close() override
	{
		flush();
		if(hNvs) { nvs_close(hNvs); }
		hNvs = HNVS_NULL;
	}
	bool getBoolValue(const char* key, bool vdef) const override
	{
		if(!hNvs) { return vdef; }
		uint8_t v = vdef ? 1 : 0;
		nvs_get_u8(hNvs, key, &v);
		return v ? true : false;
	}
	void setBoolValue(const char* key, bool v) override
	{
		if(!hNvs) return;
		if(nvs_set_u8(hNvs, key, v ? 1 : 0) == ESP_OK) { setDirty(); }
	}
	int getIntValue(const char* key, int vdef) const override
	{
		if(!hNvs) { return vdef; }
		int32_t v = vdef;
		nvs_get_i32(hNvs, key, &v);
		return v;
	}
	void setIntValue(const char* key, int v) override
	{
		if(!hNvs) return;
		if(nvs_set_i32(hNvs, key, v) == ESP_OK) { setDirty(); }
	}
	double getDoubleValue(const char* key, double vdef) const override
	{
		if(!hNvs) { return vdef; }
		double v = vdef;
		strutil::Str<28> s; size_t l = s.capacity();
		if(nvs_get_str(hNvs, key, s.getBuffer(), &l) == ESP_OK) { v = std::strtod(s, nullptr); }
		return v;
	}
	void setDoubleValue(const char* key, double v) override
	{
		if(!hNvs) { return; }
		strutil::Str<28> s; s.format("%g", v);
		if(nvs_set_str(hNvs, key, s) == ESP_OK) { setDirty(); }
	}
	size_t getStringValue(const char* key, const char* vdef, char* buf, size_t buflen) const override
	{
		size_t rl = strutil::copy(buf, buflen, vdef);
		if(!hNvs) { return rl; }
		size_t l = buflen; if(nvs_get_str(hNvs, key, buf, &l) == ESP_OK) { rl = l - 1; }
		return rl;
	}
	void setStringValue(const char* key, const char* v) override
	{
		if(!hNvs) { return; }
		if(nvs_set_str(hNvs, key, v) == ESP_OK) { setDirty(); }
	}
};

xheap::uniquePtr<PropertiesStore> PropertiesStore::createInstance()
{
	return xheap::makeUnique<PropertiesStore, PropertiesStoreImpl>();
}

#else
#error unknown platform
#endif // defined(_WINDOWS) / defined(ESP_PLATFORM)
