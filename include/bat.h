#ifndef BAT_H
#define BAT_H

#include <functional>
#include <memory>
#include <ESP8266WiFi.h>
extern "C" {
  #include <espnow.h>
  #include "user_interface.h"
}

#define BAT_MAX (29)

class Bat {
public:
	Bat(int x, int y) {
		m_x          = x;
		m_y          = y;
		m_needUpdate = 1;
	}
	~Bat() {}
	void set(int v) {v = ((v*BAT_MAX)+50)/100; if (v != m_val) {m_val = v;m_needUpdate = 1;}}
	int get() const {return (m_val*100)/BAT_MAX;}
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
