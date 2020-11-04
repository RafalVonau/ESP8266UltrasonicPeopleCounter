#ifndef SWITCH_H
#define SWITCH_H

#include <functional>
#include <memory>
#include <ESP8266WiFi.h>
extern "C" {
  #include <espnow.h>
  #include "user_interface.h"
}

class Switch {
public:
	Switch(int x, int y) {
		m_x          = x;
		m_y          = y;
		m_needUpdate = 1;
	}
	~Switch() {}
	void set(int v) {if (v != m_val) {m_val = v;m_needUpdate = 1;}}
	int get() const {return m_val;}
	
	void inc() {if (m_val == 1) {m_val=0;} else {m_val=1;} m_needUpdate = 1;}
	void dec() {inc();}
	void draw();
	void invalidate() {m_needUpdate = 1;}
private:
	int           m_x;
	int           m_y;
	int           m_val;
	int           m_needUpdate;
};
//==========================================================================================


#endif
