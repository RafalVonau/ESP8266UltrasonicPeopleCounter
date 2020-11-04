#ifndef SIMPLEROTARY_H
#define SIMPLEROTARY_H

#include <functional>
#include <memory>
#include <ESP8266WiFi.h>
extern "C" {
  #include <espnow.h>
  #include "user_interface.h"
}
#include "Ticker.h"

class SimpleRotary;

typedef std::function<void(SimpleRotary *, int butonEvent, int rotEvent)> SimpleRotaryCb;

class SimpleRotary
{
public:
	SimpleRotary(int gpioA, int gpioB, int gpioButton, SimpleRotaryCb cb, int longHoldTimeMS = 1000) {
		/* Initialize variables */
		m_CB             = cb;
		m_gpioA          = (1<<gpioA);
		m_gpioB          = (1<<gpioB);
		m_gpioButton     = (1<<gpioButton);
		m_lastEvent      = 0;
		gauge_phase      = 0;
		gauge_counter    = 0;
		gauge_last       = 0;
		bt_phase         = 0;
		bt_last          = 0;
		bt_hold          = 0;
		m_longHoldTimeMS = longHoldTimeMS;
		/* Setup GPIO pins */
		pinMode(gpioA, INPUT_PULLUP);
		pinMode(gpioB, INPUT_PULLUP);
		pinMode(gpioButton, INPUT_PULLUP);
	}
	~SimpleRotary() {stop();}
	void ICACHE_RAM_ATTR process();
	/* Run in loop */
	void loop(uint32_t m) { if ((m - m_lastEvent) > 1) { m_lastEvent = m; process(); } }
	/* Use timer */
	void start(int mili) { m_ticker.attach_ms(mili, [this]() ICACHE_RAM_ATTR { process();} ); }
	void stop()          { m_ticker.detach(); }
	int32_t getCounter() const { return gauge_counter; }
	int32_t getLastCounter() const { return gauge_last; }
public:
	SimpleRotaryCb  m_CB;
	int      m_gpioA;
	int      m_gpioB;
	int      m_gpioButton;
	Ticker   m_ticker;
	uint32_t m_lastEvent;
	/* State */
	uint32_t gauge_phase;      /*!< Gauge phase for fatate machine.                */
	int32_t  gauge_counter;    /*!< Current gauge counter.                         */
	int32_t  gauge_last;       /*!< Last gauge counter.                            */
	uint32_t bt_phase;         /*!< Button phase counter for software filtering.   */
	uint32_t bt_last;          /*!< Last button state.                             */
	uint32_t bt_hold;          /*!< Button hold counter (long time press detector. */
	int      m_longHoldTimeMS; /*!< Long press detection time in [ms].             */
};
//==========================================================================================


#endif

