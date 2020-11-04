#include "simplerotary.h"
#include "esp8266_gpio_direct.h"

/*!
 * \brief Process data - calculate button and rotary state.
 */
void ICACHE_RAM_ATTR SimpleRotary::process()
{
	char gp,gn,btn,v,Key,mv;

	Key = gpio_r->in;
	v =  (Key&m_gpioA)?0:1;
	v += (Key&m_gpioB)?0:2;
	btn = (Key&m_gpioButton)?0:1;

	gn = gp = 0;
	/* Simple key press filter */
	if (btn) {
		bt_hold++;
		if (bt_phase < 100) bt_phase++;
	} else {
		bt_hold = 0;
		if (bt_phase) bt_phase--;
	}
	btn = 0;
	if (bt_phase > 75) {
		if (bt_last == 0) {
			btn     = 1;
			bt_last = 1;
		} else {
			btn     = 0;
		}
	} else if (bt_phase < 25) {
		bt_last = 0;
	}
	/* Gauge state machine */
	switch (gauge_phase) {
		case 1: {
			/* Phase 1 (A = 0, B = 0) */
			if (v == 1) {gauge_counter++; gauge_phase = 2;}
			if (v == 2) {gauge_counter--; gauge_phase = 4;}
		} break;
		case 2: {
			/* Phase 2 (A = 0, B = 1) */
			if (v == 3) {gauge_counter++; gauge_phase = 3;}
			if (v == 0) {gauge_counter--; gauge_phase = 1;}
		} break;
		case 3: {
			/* Phase 3 (A = 1, B = 1) */
			if (v == 2) {gauge_counter++; gauge_phase = 4;}
			if (v == 1) {gauge_counter--; gauge_phase = 2;}
		} break;
		case 4: {
			/* Phase 4 (A = 1, B = 0) */
			if (v == 0) {gauge_counter++; gauge_phase = 1;}
			if (v == 3) {gauge_counter--; gauge_phase = 3;}
		} break;
		default: {
			/* Calculate phase first time */
			if (v == 0) gauge_phase = 1;
			else if (v == 1) gauge_phase = 2;
			else if (v == 3) gauge_phase = 3;
			else if (v == 2) gauge_phase = 4;
		} break;
	}
	if (bt_hold > m_longHoldTimeMS) {
		m_CB(this, 2, 0);
		bt_hold = 0;
	} else {
		if (btn) {
			/* Button pressed */
			m_CB(this, 1, 0);
		} else {
			/* Analize gauge event */
			if (gauge_counter > (gauge_last + 3)) {
				m_CB(this, 0, 1);
				gauge_last += 4;
			}
			if (gauge_counter < (gauge_last - 3)) {
				m_CB(this, 0, -1);
				gauge_last -= 4;
			}
		}
	}
}
//===========================================================================================

