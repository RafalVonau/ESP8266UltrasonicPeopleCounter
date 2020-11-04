#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "st7735.h"
#include "progressbar.h"

void ProgressBar::draw()
{
	int a,b;
	if (m_needUpdate == 0) return;
	m_needUpdate = 0;
	
	a = (m_w * m_val)/100;
	b = m_w - a;
	if (a) {
		if (m_val > 94) {
			TFT_fillRect(m_x, m_y, a, m_h, m_col2);
		} else {
			TFT_fillRect(m_x, m_y, a, m_h, m_col1);
		}
	}
	if (b) TFT_fillRect(m_x + a, m_y, b, m_h, m_col3);
}
//===========================================================================================


