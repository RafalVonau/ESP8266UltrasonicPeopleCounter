#include "us100.h"

/*!
 * \brief Send trig signal.
 */
void ICACHE_RAM_ATTR US100::trigSend()
{
	volatile uint8_t dummy = 0;
	
	m_ticker.detach();
	while (Serial.available()) { dummy = Serial.read(); }
	Serial.write(0x55);
	m_echoState = 1;
	m_trigTime = millis();
	if (m_mode == 0) {
		/* Wait m_waitTime [ms] and read results. */
		m_ticker.once_ms(m_waitTime, [this]() ICACHE_RAM_ATTR { echoTimeout(); });
	} else {
		/* poll for serial data . */
		m_ticker.attach_ms(m_pollTime, [this]() ICACHE_RAM_ATTR {
			uint32_t m = millis();
			if (m_trigTime) {
				if ((Serial.available() > 1) || ((m - m_trigTime) > m_waitTime)) {
					m_lastMeasurementTime = (m - m_trigTime);
					m_trigTime = 0;
					echoTimeout();
				}
			} else {
				m_ticker.detach();
			}
		});
	}
}
//====================================================================================

/*!
 * \brief Serial data avail or mesurement timeout.
 */
void ICACHE_RAM_ATTR US100::echoTimeout()
{
	uint8_t  HighLen = 0;
	uint8_t  LowLen  = 0;
	uint32_t Len_mm  = 0;

	m_ticker.detach();
	if (m_echoState != 1) return;
	m_lastMeasurementTime = millis() - m_trigTime;
	if (Serial.available() >= 2) {
		HighLen = Serial.read();                   // High byte of distance
		LowLen  = Serial.read();                   // Low byte of distance
		Len_mm  = ((uint32_t)HighLen)*256 + ((uint32_t)LowLen);            // Calculate the distance
		if ((Len_mm > 1) && (Len_mm < m_maxDist)) {
			m_echoState = 2;
			if (m_CB) m_CB(this, Len_mm);
			return;
		}
	}
	/* Timeout */
	m_echoState = 3;
	if (m_CB) m_CB(this, m_maxDist);
}
//====================================================================================

