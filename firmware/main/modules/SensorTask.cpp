//
//  SensorTask.cpp
//  SensorClock
//
//  created by yu2924 on 2025-12-20
//

#include "SensorTask.h"

#include <cmath>

const std::array<SensorTask::Metadata, SensorTask::Icount> SensorTask::MetadataList =
{
	Metadata{ "CO2 Level"			, "ppm"			, "sco2", false },
	Metadata{ "Temperature"			, "\xc2\xb0""C"	, "stmp", false },
	Metadata{ "Humidity"			, "%"			, "shum", true  },
	Metadata{ "Temperature"			, "\xc2\xb0""C"	, "btmp", false },
	Metadata{ "Air Pressure"		, "hPa"			, "bprs", false },
	Metadata{ "Humidity"			, "%"			, "bhum", true },
	Metadata{ "Indoor Air Quarity"	, "%"			, "biaq", true }
};

const SensorTask::Thresholds SensorTask::Thresholds::Ranges
{
	{
		{ 400, 2000 }, // scd4x co2
		{ -10,   60 }, // scd4x temp
		{   0,  100 }, // scd4x hum
		{ -40,   85 }, // bme680 temp
		{ 300, 1100 }, // bme680 pres
		{   0,  100 }, // bme680 hum
		{   0,  100 }, // bme680 iaq
	},
	0
};

const SensorTask::Thresholds SensorTask::Thresholds::Defaults
{
	{
		{ 400, 1500 }, // scd4x co2
		{  25,   35 }, // scd4x temp
		{  40,   60 }, // scd4x hum
		{  25,   35 }, // bme680 temp
		{  980, 1050 }, // bme680 pres
		{  40,   60 }, // bme680 hum
		{  75,  100 }, // bme680 iaq
	},
	0
};

SensorTask::Falerts SensorTask::calculateAlertFlags(const Thresholds& th, const Values& val)
{
	Falerts falerts = 0;
	for(int i = 0; i < Icount; ++i)
	{
		const auto& rng = th.array[i];
		float v = val[i];
		Falerts ma = 1 << i;
		if((th.alerts & ma) && !std::isnan(v) && ((v < rng.under) || (rng.over < v))) falerts |= ma;
	}
	return falerts;
}

#if defined(_WINDOWS)

// ================================================================================
// Prototype code for Windows
// It simply outputs dummy data.

#include <random>
#include "utilities/listenerlist.h"
#include "modules/Dispatcher.h"

class SensorTaskImpl : public SensorTask, private DispatchTimer::Listener
{
public:
	xheap::uniquePtr<DispatchTimer> timer = DispatchTimer::createInstance();
	ListenerListT<SensorTask::Listener> listenerList;
	Thresholds thresholds = Thresholds::Defaults;
	Values values{};
	Falerts alertFlags = 0;
	SensorTaskImpl()
	{
		timer->addListener(this);
	}
	~SensorTaskImpl() override
	{
		timer->removeListener(this);
	}
	void addListener(SensorTask::Listener* p) override
	{
		listenerList.add(p);
	}
	void removeListener(SensorTask::Listener* p) override
	{
		listenerList.remove(p);
	}
	const Thresholds& getThresholds() const override
	{
		return thresholds;
	}
	void setThresholds(const Thresholds& v) override
	{
		if(thresholds == v) { return; }
		thresholds = v;
		alertFlags = calculateAlertFlags(thresholds, values);
		listenerList.call(&SensorTask::Listener::sensorTaskThresholdsDidChange, this, thresholds);
	}
	bool isActive() const override
	{
		return timer->isRunning();
	}
	bool activate() override
	{
		dispatchTimerDidElapse(timer.get());
		timer->start(5000);
		srand(0);
		return true;
	}
	void deactivate() override
	{
		timer->stop();
	}
	const Values& getValues() const override
	{
		return values;
	}
	Falerts getAlertFlags() const override
	{
		return alertFlags;
	}
	// DispatchTimer::Listener
	void dispatchTimerDidElapse(DispatchTimer*) override
	{
		// dummy data
		static std::random_device seed_gen;
		static std::uint32_t seed = seed_gen();
		static std::mt19937 engine(seed);
		static std::uniform_real_distribution<float> dist(-1, 1);
		values = {};
		for(int isensor = 0; isensor < Icount; ++isensor)
		{
			const auto& rng = Thresholds::Ranges.array[isensor];
			const auto& th = thresholds.array[isensor];
			float center = (th.over + th.under) * 0.5f;
			float amp = (th.over - th.under) * 0.5f * 1.5f;
			float v = center + amp * dist(engine);
			values[isensor] = std::max(rng.under, std::min(rng.over, v));
		}
		alertFlags = calculateAlertFlags(thresholds, values);
		const Fupdates fupdates = Fscd4xAll | Fbme680All;
		listenerList.call(&SensorTask::Listener::sensorTaskValuesDidChange, this, fupdates, values);
	}
};

void SensorTask::platformInit()
{
}

xheap::uniquePtr<SensorTask> SensorTask::createInstance()
{
	return xheap::makeUnique<SensorTask, SensorTaskImpl>();
}

#elif defined(ESP_PLATFORM)

// ================================================================================
// Production code for ESP32

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_task.h>
#include <bme680.h>
#include <scd4x.h>
#include "utilities/listenerlist.h"
#include "modules/bme6801IAQ_esp32.h"
#include "modules/Dispatcher.h"

static constexpr i2c_port_t SENSORTASK_I2CPORT = I2C_NUM_0;
static constexpr gpio_num_t SENSORTASK_SDAPIN = (gpio_num_t)CONFIG_I2CDEV_DEFAULT_SDA_PIN;
static constexpr gpio_num_t SENSORTASK_SCLPIN = (gpio_num_t)CONFIG_I2CDEV_DEFAULT_SCL_PIN;

static const char* TagScd4x = "SCD4x";

struct Scd4xHandler
{
	static constexpr uint8_t Scd4xHandler_I2C_ADDR = SCD4X_I2C_ADDR; // 0x62
	static constexpr int IntervalPeriodicMs = 5000;
	static constexpr int IntervalLowPowerPeriodicMs = 30000;
	i2c_dev_t dev{};
	TickType_t tkMeasureInterval = 0;
	TickType_t tkMeasureLast = 0;
	bool initialized = false;
	Scd4xHandler() = delete;
	Scd4xHandler(i2c_port_t port, gpio_num_t pinsda, gpio_num_t pinscl)
	{
		// scd4x default: dev.cfg.master.clk_speed = 100000;
		ESP_ERROR_CHECK_WITHOUT_ABORT(scd4x_init_desc(&dev, port, pinsda, pinscl));
	}
	~Scd4xHandler()
	{
		stop();
		scd4x_free_desc(&dev);
	}
	bool start(bool lowpower)
	{
		tkMeasureInterval = 0;
		tkMeasureLast = 0;
		initialized = false;
		esp_err_t r = scd4x_wake_up(&dev);
		if(r != ESP_OK) { ESP_LOGW(TagScd4x, "scd4x_wake_up() failed"); }
		r = scd4x_stop_periodic_measurement(&dev);
		if(r != ESP_OK) { ESP_LOGW(TagScd4x, "scd4x_stop_periodic_measurement() failed"); }
		r = scd4x_reinit(&dev);
		if(r != ESP_OK) { ESP_LOGW(TagScd4x, "scd4x_reinit() failed"); return false; }
		uint16_t sn[3]{};
		scd4x_get_serial_number(&dev, sn, sn + 1, sn + 2);
		ESP_LOGI(TagScd4x, "get_serial_number SN=0x%04x%04x%04x", sn[0], sn[1], sn[2]);
		if(lowpower) scd4x_start_low_power_periodic_measurement(&dev);
		else		 scd4x_start_periodic_measurement(&dev);
		int measintms = lowpower ? IntervalLowPowerPeriodicMs : IntervalPeriodicMs;
		tkMeasureInterval = pdMS_TO_TICKS(measintms);
		tkMeasureLast = xTaskGetTickCount();
		initialized = true;
		return true;
	}
	void stop()
	{
		if(!initialized) { return; }
		scd4x_stop_periodic_measurement(&dev);
		initialized = false;
	}
	bool tryRead(float* pco2, float* ptemp, float* phum)
	{
		if(!initialized) { return false; }
		TickType_t tkread = xTaskGetTickCount();
		if(tkread < tkMeasureLast + tkMeasureInterval) { return false; }
		uint16_t co2 = 0; float temp = 0, hum = 0;
		if(scd4x_read_measurement(&dev, &co2, &temp, &hum) != ESP_OK) { return false; }
		tkMeasureLast = tkread;
		// ESP_LOGI(TagScd4x, "read_measurement co2=%u temp=%.1f hum=%.0f", (uint32_t)co2, temp, hum);
		if(pco2) { *pco2 = (float)co2; }
		if(ptemp) { *ptemp = temp; }
		if(phum) { *phum = hum; }
		return true;
	}
};

// --------------------------------------------------------------------------------

static const char* TagBme680 = "BME680";

struct Bme680Handler
{
	static constexpr uint8_t Bme680Handler_I2C_ADDR = BME680_I2C_ADDR_1; // 0x77
	static constexpr int MeasureIntervalMs = 1000;
	static constexpr int IntervalThinner = 5;
	bme680_t dev{};
	IAQTracker iaqTracker;
	TickType_t tkMeasureInterval = 0;
	TickType_t tkMeasureLast = 0;
	TickType_t tkReadDelay = 0;
	bool initialized = false;
	Bme680Handler() = delete;
	Bme680Handler(i2c_port_t port, gpio_num_t pinsda, gpio_num_t pinscl)
	{
		// bme680 default: dev.i2c_dev.cfg.master.clk_speed = 1000000;
		ESP_ERROR_CHECK_WITHOUT_ABORT(bme680_init_desc(&dev, Bme680Handler_I2C_ADDR, port, pinsda, pinscl));
	}
	~Bme680Handler()
	{
		stop();
		bme680_free_desc(&dev);
	}
	bool start()
	{
		tkMeasureInterval = 0;
		tkMeasureLast = 0;
		tkReadDelay = 0;
		initialized = false;
		esp_err_t r = bme680_init_sensor(&dev);
		if(r != ESP_OK) { ESP_LOGW(TagBme680, "bme680_init_sensor failed"); return false; }
		iaqTracker.init(300 / IntervalThinner, 3600 / IntervalThinner, 0.03f);
		// the initialization parameters are as follows: "IAQ_example.py"
		// cf. https://github.com/thstielow/raspi-bme680-iaq/blob/main/IAQ_example.py
		bme680_set_oversampling_rates(&dev, BME680_OSR_8X, BME680_OSR_4X, BME680_OSR_2X); // t,p,h
		bme680_set_filter_size(&dev, BME680_IIR_SIZE_3);
		bme680_set_heater_profile(&dev, 0, 320, 150);
		bme680_use_heater_profile(&dev, 0);
		//bme680_set_ambient_temperature(&dev, 25);
		bme680_get_measurement_duration(&dev, &tkReadDelay);
		ESP_LOGI(TagBme680, "measurement_duration=%u ms", pdTICKS_TO_MS(tkReadDelay));
		int measintms = MeasureIntervalMs * IntervalThinner;
		tkMeasureInterval = pdMS_TO_TICKS(measintms);
		tkMeasureLast = xTaskGetTickCount();
		initialized = true;
		return true;
	}
	void stop()
	{
		initialized = false;
	}
	bool tryRead(float* ptemp, float* ppres, float* phum, float* piaq)
	{
		if(!initialized) { return false; }
		TickType_t tkread = xTaskGetTickCount();
		if(tkread < (tkMeasureLast + tkMeasureInterval)) { return false; }
		if(bme680_force_measurement(&dev) != ESP_OK) return false;
		vTaskDelay(tkReadDelay);
		bme680_values_float_t values{};
		if(bme680_get_results_float(&dev, &values) != ESP_OK) { return false; }
		bme680_set_ambient_temperature(&dev, (int16_t)values.temperature);
		float iaq = iaqTracker.getIAQ(values);
		tkMeasureLast = tkread;
		// ESP_LOGI(TagBme680, "get_results_float temp=%.1f pres=%.0f hum=%.0f iaq=%.0f", values.temperature, values.pressure, values.humidity, iaq);
		if(ptemp) { *ptemp = values.temperature; }
		if(ppres) { *ppres = values.pressure; }
		if(phum) { *phum = values.humidity; }
		if(piaq) { *piaq = iaq; }
		return true;
	}
};

// --------------------------------------------------------------------------------

static const char* TagSensorTask = "SensorTask";

class SensorTaskImpl : public SensorTask
{
private:
	static constexpr UBaseType_t TaskPriority = ESP_TASK_MAIN_PRIO + 1;
	static constexpr uint32_t TaskStackDepth = 4096;
	static constexpr uint32_t PollingIntervalMs = 500;
	TaskHandle_t hTask = NULL;
	static constexpr uint32_t BitReqQuit = 0x01;
	static constexpr uint32_t BitDidQuit = 0x02;
	EventGroupHandle_t hEventGroup = NULL;
	ListenerListT<Listener> listenerList;
	Thresholds thresholds = Thresholds::Defaults;
	Values values{};
	Falerts alertFlags = 0;
public:
	SensorTaskImpl()
	{
		hEventGroup = xEventGroupCreate();
	}
	~SensorTaskImpl() override
	{
		deactivate();
		if(hEventGroup) { vEventGroupDelete(hEventGroup); }
	}
	static void taskProc(void* arg)
	{
		if(SensorTaskImpl* p = (SensorTaskImpl*)arg) { p->runTask(); }
	}
	void runTask()
	{
		// ESP_LOGI(TagSensorTask, "sensor thread begin");
		{
			ESP_LOGI(TagSensorTask, "port=%d sdapin=%d sclpin=%d", SENSORTASK_I2CPORT, SENSORTASK_SDAPIN, SENSORTASK_SCLPIN);
			Scd4xHandler scd4x(SENSORTASK_I2CPORT, SENSORTASK_SDAPIN, SENSORTASK_SCLPIN);
			Bme680Handler bme680(SENSORTASK_I2CPORT, SENSORTASK_SDAPIN, SENSORTASK_SCLPIN);
			bool rs = scd4x.start(false);
			bool rb = bme680.start();
			if(rs || rb)
			{
				constexpr TickType_t tkpoll = pdMS_TO_TICKS(PollingIntervalMs);
				TickType_t tklast = xTaskGetTickCount();
				while(!(xEventGroupGetBits(hEventGroup) & BitReqQuit))
				{
					Fupdates fupdates = 0;
					if(scd4x.tryRead(&values[Iscd4xCo2], &values[Iscd4xTemp], &values[Iscd4xHum])) { fupdates |= Fscd4xAll; }
					if(bme680.tryRead(&values[Ibme680Temp], &values[Ibme680Pres], &values[Ibme680Hum], &values[Ibme680Iaq])) { fupdates |= Fbme680All; }
					if(xEventGroupGetBits(hEventGroup) & BitReqQuit) { break; }
					if(fupdates)
					{
						alertFlags = calculateAlertFlags(thresholds, values);
						Dispatcher::getMainDispatcher()->callAsync([this, fupdates]()
						{
							listenerList.call(&Listener::sensorTaskValuesDidChange, this, fupdates, values);
						});
					}
					xTaskDelayUntil(&tklast, tkpoll);
				}
			}
			bme680.stop();
			scd4x.stop();
		}
		// ESP_LOGI(TagSensorTask, "sensor thread end");
		xEventGroupSetBits(hEventGroup, BitDidQuit);
		vTaskSuspend(NULL);
	}
	void addListener(Listener* p) override
	{
		listenerList.add(p);
	}
	void removeListener(Listener* p) override
	{
		listenerList.remove(p);
	}
	const Thresholds& getThresholds() const override
	{
		return thresholds;
	}
	void setThresholds(const Thresholds& v) override
	{
		if(thresholds == v) { return; }
		thresholds = v;
		alertFlags = calculateAlertFlags(thresholds, values);
		listenerList.call(&SensorTask::Listener::sensorTaskThresholdsDidChange, this, thresholds);
	}
	bool isActive() const override
	{
		return hTask != NULL;
	}
	bool activate() override
	{
		deactivate();
		values = {};
		xEventGroupClearBits(hEventGroup, BitReqQuit | BitDidQuit);
		if(xTaskCreate(&taskProc, "sensor", TaskStackDepth, this, TaskPriority, &hTask) != pdPASS)
		{
			ESP_LOGW(TagSensorTask, "xTaskCreate() failed");
			return false;
		}
		return true;
	}
	void deactivate() override
	{
		if(hTask)
		{
			xEventGroupSetBits(hEventGroup, BitReqQuit);
			xTaskAbortDelay(hTask);
			xEventGroupWaitBits(hEventGroup, BitDidQuit, pdFALSE, pdFALSE, pdMS_TO_TICKS(100));
			vTaskDelete(hTask);
			hTask = NULL;
		}
	}
	const Values& getValues() const override
	{
		return values;
	}
	Falerts getAlertFlags() const override
	{
		return alertFlags;
	}
};

void SensorTask::platformInit()
{
	ESP_ERROR_CHECK(i2cdev_init());
}

xheap::uniquePtr<SensorTask> SensorTask::createInstance()
{
	return xheap::makeUnique<SensorTask, SensorTaskImpl>();
}

#else
#error unknown platform
#endif // defined(_WINDOWS) / defined(ESP_PLATFORM)
