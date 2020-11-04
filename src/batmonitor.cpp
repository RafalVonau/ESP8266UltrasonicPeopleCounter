#include "batmonitor.h"

static const int __bm_vtable[11] = { 4200, 4100, 4000, 3900, 3800, 3700, 3600, 3500, 3400, 3300, 3200 };
static const int __bm_ptable[11] = {  100,  94,   83,   72,   59,   50,   33,   15,   6,     0,   0  };

/*!
 * \brief Interpolate battery percent from voltage.
 */
int BatMonitor::bat_interpolate(int x)
{
	int n = 11;
	int const *tabx = __bm_vtable;
	int const *taby = __bm_ptable;
	int index;
	int y;

	if (x >= tabx[0]) {
		y = taby[0];
	} else if (x <= tabx[n - 1]) {
		y = taby[n - 1];
	} else {
		for (index = 1; index < n; index++)
		if (x > tabx[index]) break;
		/*  interpolate */
		y = (taby[index - 1] - taby[index]) * (x - tabx[index]) /(tabx[index - 1] - tabx[index]);
		y += taby[index];
	}
	return y;
}
//===========================================================================================

/*!
 * \brief Get data from ADC (average 16 measurements for lower ADC noise).
 * Results are quite linear above 0.1V and bellow 0.9V.
 * Measurement time = 2.125 [ms] (17 * 125us)
 */
int BatMonitor::get_from_adc()
{
	volatile int i;
	i = system_adc_read();
	i = 0; /* Discard first measurement */
	i = system_adc_read();
	i += system_adc_read();
	i += system_adc_read();
	i += system_adc_read();
	i += system_adc_read();
	i += system_adc_read();
	i += system_adc_read();
	i += system_adc_read();
	i += system_adc_read();
	i += system_adc_read();
	i += system_adc_read();
	i += system_adc_read();
	i += system_adc_read();
	i += system_adc_read();
	i += system_adc_read();
	i += system_adc_read();
	return (i>>4);
}
//===========================================================================================


/*!
 * \brief Measure and process data.
 */
void BatMonitor::process(void)
{
	int val, volt, percent;

	switch (m_average) {
		case 0: { val = system_adc_read();              } break;
		case 1: { val = BatMonitor::get_from_adc();     } break;
		default:{ val = BatMonitor::get_from_adc();     } break;
	}
	volt = (((int)val) * m_mpy)/m_div;
	percent = bat_interpolate(volt);
	if (m_CB) m_CB(this, val, volt, percent);
}
//===========================================================================================

