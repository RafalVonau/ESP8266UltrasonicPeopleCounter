#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <functional>
#include <memory>
#include <ESP8266WiFi.h>
extern "C" {
  #include <espnow.h>
  #include "user_interface.h"
}

class ProgressBar {
public:
	ProgressBar(int x, int y, int w, int h, int col1, int col2, int col3) {
		m_x = x;
		m_y = y;
		m_w = w;
		m_h = h;
		m_col1 = col1;
		m_col2 = col2;
		m_col3 = col3;
		m_needUpdate = 1;
	}
	~ProgressBar() {}
	void set(int v) {if (v<0) v=0; if (v>100) v=100; if (v != m_val) {m_val = v;m_needUpdate = 1;}}
	int get() const {return m_val;}
	
	void inc() {if (m_val < 100) {m_val++;m_needUpdate = 1;}}
	void dec() {if (m_val > 0) {m_val--;m_needUpdate = 1;}}
	
	void drawDigit(int v, int x, int y);
	void drawDigitSmall(int v, int x, int y);
	void draw();
	void invalidate() {m_needUpdate = 1;}
private:
	int           m_x;
	int           m_y;
	int           m_w;
	int           m_h;
	int           m_col1;
	int           m_col2;
	int           m_col3;
	int           m_val;
	int           m_needUpdate;
};
//==========================================================================================


#endif
