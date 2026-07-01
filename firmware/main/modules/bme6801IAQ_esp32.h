//
//  bme680IAQ_esp32.h
//  SensorClock
//
//  created by yu2924 on 2026-01-18
//
//  This C++ code is a straightforward port from the Python code "bme680IAQ.py" written by thstielow.
//  https://github.com/thstielow/raspi-bme680-iaq
//

#include <bme680.h>
#include <array>
#include <cmath>

class IAQTracker
{
private:
	static float square(float v) { return v * v; }
	// C++ translation of the Python list and numpy.mean()
	struct MeanBuffer
	{
		std::array<float, 101> arr;
		size_t wpos = 0, rpos = 0, len = 0;
		float sum = 0;
		void init()
		{
			sum = 0;
			wpos = rpos = len = 0;
		}
		void init(float v)
		{
			init();
			append(v);
		}
		void append(float v)
		{
			assert(len < arr.size());
			arr[wpos++] = v;
			sum += v;
			if(arr.size() <= wpos) wpos = 0;
			++len;
		}
		void del_head()
		{
			assert(0 < len);
			int v = arr[rpos++];
			sum -= v;
			if(arr.size() <= rpos) rpos = 0;
			--len;
		}
		bool about_to_overflow() const
		{
			return arr.size() <= len;
		}
		float mean() const
		{
			return sum / (float)len;
		}
	};
	float slope = 0;
	int burn_in_cycles = 0;
	MeanBuffer gas_cal_data;
	float gas_ceil = 0;
	int gas_recal_period = 0;
	int gas_recal_step = 0;
public:
	IAQTracker() {}
	void init(int burn_in_cycles_ = 300, int gas_recal_period_ = 3600, float ph_slope_ = 0.03f)
	{
		slope = ph_slope_;
		burn_in_cycles = burn_in_cycles_; // determines burn-in-time, usually 5 minutes, equal to 300 cycles of 1s duration
		gas_cal_data.init();
		gas_ceil = 0;
		gas_recal_period = gas_recal_period_; // number of cycles after which to drop last entry of the gas calibration list. Here: 1h
		gas_recal_step = 0;
	}
	static float waterSatDensity(float temp)
	{
		float rho_max = (6.112f * 100 * std::exp((17.62f * temp)/(243.12f + temp)))/(461.52f * (temp + 273.15f));
		return rho_max;
	}
	float getIAQ(const bme680_values_float_t& bme_data)
	{
		float temp = bme_data.temperature;
		float hum = bme_data.humidity;
		float R_gas = bme_data.gas_resistance;

		// calculate stauration density and absolute humidity
		float rho_max = waterSatDensity(temp);
		float hum_abs = hum * 10 * rho_max;

		// compensate exponential impact of humidity on resistance
		float comp_gas = R_gas * std::exp(slope * hum_abs);

		if(burn_in_cycles > 0)
		{
			// check if burn-in-cycles are recorded
			--burn_in_cycles;		// count down cycles
			if(comp_gas > gas_ceil)	// if value exceeds current ceiling, add to calibration list and update ceiling
			{
				gas_cal_data.init(comp_gas);
				gas_ceil = comp_gas;
			}
			return std::numeric_limits<float>::quiet_NaN(); // return None type as sensor burn-in is not yet completed
		}
		else
		{
			// adapt calibration
			if(comp_gas > gas_ceil)
			{
				gas_cal_data.append(comp_gas);
				if(gas_cal_data.about_to_overflow())
				{
					gas_cal_data.del_head();
				}
				gas_ceil = gas_cal_data.mean();
			}

			// calculate and print relative air quality on a scale of 0-100%
			// use quadratic ratio for steeper scaling at high air quality
			// clip air quality at 100%
			float AQ = std::min(square(comp_gas / gas_ceil), 1.0f) * 100;

			// for compensating negative drift (dropping resistance) of the gas sensor:
			// delete oldest value from calibration list and add current value
			++gas_recal_step;
			if(gas_recal_step >= gas_recal_period)
			{
				gas_recal_step = 0;
				gas_cal_data.append(comp_gas);
				gas_cal_data.del_head();
				gas_ceil = gas_cal_data.mean();
			}
			return AQ;
		}
	}
};
