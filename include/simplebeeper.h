#ifndef SIMPLEBIPPER_H
#define SIMPLEBIPPER_H

#include <functional>
#include <memory>
#include <ESP8266WiFi.h>
extern "C" {
  #include <espnow.h>
  #include "user_interface.h"
}
#include "Ticker.h"

class SimpleBeeper
{
public:
	SimpleBeeper(int gpio) {
		m_gpio           = gpio;
		m_bussy          = 0;
		m_enabled        = 1;
		pinMode(gpio, OUTPUT);
	}
	~SimpleBeeper() { m_ticker.detach(); }

	void ICACHE_RAM_ATTR beepOff() {
		digitalWrite(m_gpio, HIGH);
		m_ticker.detach();
		m_bussy = 0;
	}

	void beep(int ms = 50) {
		if ((m_bussy) || (!m_enabled)) return;
		m_bussy = 1;
		digitalWrite(m_gpio, LOW);
		m_ticker.attach_ms(ms, [this]() ICACHE_RAM_ATTR { beepOff();} );
	}

	void setEnabled(int i) {m_enabled = i;}
public:
	int          m_enabled;
	int          m_gpio;
	volatile int m_bussy;
	Ticker       m_ticker;
};
//==========================================================================================


#endif

