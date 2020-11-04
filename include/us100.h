#ifndef US100_H
#define US100_H

#include <functional>
#include <memory>
#include <ESP8266WiFi.h>
extern "C" {
  #include <espnow.h>
  #include "user_interface.h"
}
#include "Ticker.h"

class US100;

typedef std::function<void(US100 *, int distanceMM)> US100Cb;

class US100
{
public:
	/*
	 * maxDist - maximum valid distance in milimeters 1 - 4500 [mm] ,
	 * mode:
	 *   0 - constant time (every waitTime [ms]),
	 *   1 - event when data available on serial port (poll every poolTime).
	 * waitTime - time to wait for measurement (default 100ms).
	 *
	 * Information from https://github.com/stoduk/PingSerial/blob/master/PingSerial.cpp:
	 * - around ~11ms overhead for any measurement to be returned
	 *   [3ms of that will be sending/receiving 3 chars @ 9600 baud]
	 * - sound takes 5.7us per mm round trip
	 * The above combined should give us an upper bound on how long an
	 * operation can take, but it seems we have two long-distance-timeout
	 * modes - one where no value is returned (ie. genuine timeout), and one
	 * where after 91ms a bogus large number is returned (11090 or 11110mm).
	 *
	 * Given a max range of 3.5m, this gives 11ms + 20ms = 31ms.
	 *
	 * Set the timeout to be the max of these two, ie. 91ms plus a bit.
	 * Why US-100 takes so long to timeout I don't know..
	 */
	US100(US100Cb cb, uint32_t maxDist = 3000, int mode = 0, uint32_t waitTime = 100, uint32_t poolTime = 20) {
		m_CB        = cb;
		m_echoState = 0;
		m_echoDist  = maxDist;
		m_trigTime  = 0;
		m_maxDist   = maxDist;
		m_mode      = mode;
		m_waitTime  = waitTime;
		m_pollTime  = poolTime;
	}
	
	~US100() {m_ticker.detach(); m_sndticker.detach();}
	
	/*!
	 * \btief Start measurement (send trigger).
	 */
	void ICACHE_RAM_ATTR trigSend();
	/*!
	 * \btief Start measurement after ms miliceconds from now.
	 */
	void trigSendDelayed(int ms) { m_sndticker.once_ms(ms, [this]() ICACHE_RAM_ATTR { trigSend();} ); }
	/*!
	 * \brief Read results and parse data.
	 */
	void ICACHE_RAM_ATTR echoTimeout();
	/*!
	 * \brief Get last readed distance in [mm].
	 */
	uint32_t getDistance() {
		if (m_echoState == 2) return m_echoDist;
		return m_maxDist;
	}
	/*!
	 * \brief Get maximum allowed distance in [mm].
	 */
	int getMaxDist() const {return m_maxDist;}
	/*!
	 * \brief Get last measurement time.
	 */
	uint32 getLastMeasureTime() const {return m_lastMeasurementTime; }
public:
	US100Cb  m_CB;
	int      m_echoState;
	int      m_mode;
	uint32_t m_maxDist;
	uint32_t m_waitTime;
	uint32_t m_pollTime;
	uint32_t m_echoDist;
	uint32_t m_trigTime;
	uint32_t m_lastMeasurementTime;
	/* Timers */
	Ticker   m_ticker;         /*!< Measurement read timer */
	Ticker   m_sndticker;      /*!< Trig send timmer.      */
};
//==========================================================================================


#endif
