#ifndef COUNTER_H
#define COUNTER_H

#include <functional>
#include <memory>
#include <ESP8266WiFi.h>
extern "C" {
  #include <espnow.h>
  #include "user_interface.h"
}

class Counter {
public:
	/*!
	 * \brief Create new counter.
	 * \param x - position x,
	 * \param y - position y,
	 * \param size - size and layout:
	 *     0 - big size   - centered,
	 *     1 - small size - allign to left,
	 *     2 - small size - centered.
	 * \param lazy - lazy update (0 - disabled, x - update if change bigger than x or timeout),
	 * \param lazyDelayMS - lazy update timeout in [ms] use loop method to update timeout calculations.
	 */
	Counter(int x, int y, int size, int lazy=0, uint32_t lazyDelayMS=500) {
		m_x           = x;
		m_y           = y;
		m_size        = size;
		m_lazy        = lazy;
		m_lazyDelayMS = lazyDelayMS;
		m_needUpdate  = 1;
	}
	~Counter() {}
	
	/*!
	 * \brief Check needupdate flag.
	 */
	void inline checkUpdate() {
		if (m_lazy == 0) {
			m_needUpdate = 1;
		} else if (m_val != m_curval) {
			if (m_val > m_curval) {
				if ((m_val - m_curval) > m_lazy) invalidate();
			} else {
				if ((m_curval - m_val) > m_lazy) invalidate();
			}
		}
	}
	int set(int v) {if (v != m_val) {m_val = v;checkUpdate();return 1;} return 0; }
	int get() const {return m_val;}
	
	void inc() {if (m_val < 999) {m_val++;checkUpdate();}}
	void dec() {if (m_val > 0) {m_val--;checkUpdate();}}
	
	void drawDigit(int v, int x, int y);
	void drawDigitSmall(int v, int x, int y);
	void draw();
	void invalidate() {m_needUpdate = 1;}
	/*!
	 * \brief Handle lazy update timeout.
	 */
	int loop(uint32_t m) {
		if ((m_lazy) && (!m_needUpdate) && (m_val != m_curval) && ((m - m_lastUpdate)>m_lazyDelayMS)) {
			m_needUpdate = 1;
			return 1;
		}
		return 0;
	}
private:
	int           m_x;
	int           m_y;
	int           m_size;
	int           m_val;
	int           m_lazy;
	int           m_needUpdate;
	/* Lazy update */
	uint32_t      m_lazyDelayMS;
	int           m_curval;
	uint32_t      m_lastUpdate;
};
//==========================================================================================


#endif
