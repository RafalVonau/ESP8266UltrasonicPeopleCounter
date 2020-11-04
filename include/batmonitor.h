#ifndef BATMONITOR_H
#define BATMONITOR_H

#include <functional>
#include <memory>
#include <ESP8266WiFi.h>
extern "C" {
  #include <espnow.h>
  #include "user_interface.h"
}

class BatMonitor;

typedef std::function<void(BatMonitor *, int adu, int volt, int percent)> BatMonitorCb;

class BatMonitor {
public:
	/*!
	 * \brief Create new battery monitor.
	 * \param cb - callback function,
	 * \param t_mpy ( for adu to voltage conversion volt = (adu * t_mpy)/t_div,
	 * \param t_div ( for adu to voltage conversion volt = (adu * t_mpy)/t_div,
	 * \param average - Average method (
	 *    0 - no average 125 [us],
	 *    1 - mean from 16 samples 2.2 [ms],
	 *  )
	 * \param scanIntervalMs - battery measure interval in [ms].
	 */
	BatMonitor(BatMonitorCb cb, int t_mpy = 3710, int t_div = 732, int average = 1, int scanIntervalMs = 20000) {
		m_CB               = cb;
		m_div              = t_div;
		m_mpy              = t_mpy;
		m_lastEvent        = millis() - scanIntervalMs + 100u;
		m_average          = average;
		m_scanIntervalMs   = scanIntervalMs;
	}
	~BatMonitor() {}
	void loop(uint32_t m) { if ((m - m_lastEvent) > m_scanIntervalMs) { m_lastEvent = m; process(); } }
	void process();

	/*!
	 * \brief Interpolate battery percent.
	 * \param voltMV - battery voltage in [mV]
	 * \return battery percent 0-100
	 */
	static int bat_interpolate(int voltMV);
	/*!
	 * \brief Get data from ADC (average 16 measurements for lower ADC noise).
	 * Results are quite linear above 0.1V and bellow 0.9V.
	 * Measurement time = 2.125 [ms] (17 * 125us)
	 */
	static int get_from_adc();
public:
	BatMonitorCb  m_CB;
	int           m_mpy;
	int           m_div;
	int           m_average;
	uint32_t      m_scanIntervalMs;
	uint32_t      m_lastEvent;
};
//==========================================================================================


#endif
