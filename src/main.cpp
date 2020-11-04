#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "simplerotary.h"
#include "us100.h"
#include "st7735.h"
#include <EEPROM_Rotate.h>
#include "batmonitor.h"
#include "simplebeeper.h"

#include "counter.h"
#include "bat.h"
#include "orientation.h"
#include "switch.h"
#include "progressbar.h"
#include "mainwindow.h"
#include "setmaxwindow.h"
#include "orientationwindow.h"
#include "soundwindow.h"
#include "limitwindow.h"
#include "setdoorwidth.h"

/*
 * - PCB CONNECTIONS -
 *
 * TFT_CS     - GPIO16 (CS)
 * TFT_SCK    - GPIO14 (SCL)
 * TFT_RS     - GPIO12 (A0)
 * TFT_MOSI   - GPIO13 (SDA)
 * TFT_RESET  - GPIO15 (RESET)
 * TFT_LED+   - 100ohm -> 3.3V
 * ROT1       - GPIO5 - filter (ESP pin - 10kohm (pullup) - 100nF to GND - 1kohm to rotary)
 * ROT2       - GPIO4 - the same filter
 * BTN        - GPIO0 - the same filter
 * LED/BUZZER - GPIO2 (Buzzer connected to 3.3V GPIO2 drives to GND)
 * VBAT - 20kohm - A0 - 4.7kohm - GND
 *                    |- 100nF -|
 */

#define GPIO_ROT_A      5
#define GPIO_ROT_B      4
#define GPIO_ROT_BUTTON 0

#define GPIO_TFT_RS     12
#define GPIO_TFT_CS     16
#define GPIO_TFT_SCK    14
#define GPIO_TFT_MOSI   13
#define GPIO_TFT_RESET  15

#define GPIO_LED        2

#define SONAR_MARGIN_CM 40
#define DEBUG_SONAR_DISTANCE
#define DISPLAY_MODES   4

#if 1
#undef DEBUG_ENABLED
#define pdebug(fmt, args...)
#define pwrite(fmt, len)
#else
#define DEBUG_ENABLED
#define pdebug(fmt, args...) Serial.printf(fmt, ## args)
#define pwrite(fmt, len) Serial.write(fmt, len)
#endif

//===========================================================================================
//==================================-- Global variables --===================================
//===========================================================================================

EEPROM_Rotate     EEPROMr;
US100            *g_us100;
SimpleRotary     *g_rot;
BatMonitor       *g_batmon;
SimpleBeeper     *g_beeper;
volatile int      g_us100_state;
volatile int      g_us100_state_last;
volatile int      g_us100_mark;
volatile int      g_us100_valid;
volatile int      g_update_display;
volatile int      g_display_mode;
volatile int      g_display_mode_last;
Counter          *g_peopleCounter;
Counter          *g_peopleMax;
Counter          *g_setMaxCounter;
Counter          *g_setDoorLength;
Bat              *g_bat;
Orientation      *g_orn;
Switch           *g_sndSW;
ProgressBar      *g_progress;
#ifdef DEBUG_SONAR_DISTANCE
Counter          *g_debugDistance;
#endif

/* settings */
int               g_orientation;

//===========================================================================================
//==================================-- The code --===========================================
//===========================================================================================
uint16_t door_length;
uint16_t last_eeprom_flags;
uint16_t last_eeprom_max_number;
uint16_t last_eeprom_door_length;

/*!
 * \brief Write configuration to eeprom.
 */
void writeIntoEEPROM(uint16_t flags, uint16_t max_number, uint16_t door_length)
{
	if ((flags != last_eeprom_flags) || (max_number != last_eeprom_max_number) || (door_length != last_eeprom_door_length)) {
		last_eeprom_flags       = flags;
		last_eeprom_max_number  = max_number;
		last_eeprom_door_length = door_length;
		pdebug("EEPROM write (max = %u, door = %u, flags=%u)\n",max_number, door_length, flags);
		EEPROMr.write(10, (last_eeprom_max_number >> 8) & 0xff);
		EEPROMr.write(11, last_eeprom_max_number & 0xff);
		EEPROMr.write(12, (last_eeprom_door_length >> 8) & 0xff);
		EEPROMr.write(13, last_eeprom_door_length & 0xff);
		EEPROMr.write(14, last_eeprom_flags & 0xff);
		EEPROMr.commit();
	}
}
//===========================================================================================

/*!
 * \brief Read configuration from eeprom.
 */
void readFromEEPROM(uint16_t *flags, uint16_t *max_number, uint16_t *door_length)
{
	uint16_t v;

	EEPROMr.size(8);
	EEPROMr.begin(4096);
	/* Read max number */
	v = (((uint16_t)EEPROMr.read(10) << 8) | ((uint16_t)EEPROMr.read(11)));
	if (v > 999) v = 220;
	*max_number = v;
	/* Read door length */
	v = (((uint16_t)EEPROMr.read(12) << 8) | ((uint16_t)EEPROMr.read(13)));
	if (v > 999) v = 0;
	*door_length = v;
	/* Flags */
	v = ((uint16_t)EEPROMr.read(14));
	if (v > 16) v = 0;
	*flags = v;

	pdebug("EEPROM read (max = %u, door = %u, flags=%u\n",*max_number, *door_length, *flags);
}
//===========================================================================================

/*!
 * \brief Increment/decrement people counter.
 */
static void incCounter(int dir)
{
	int limit;
	if (dir == 1) {
		g_peopleCounter->inc();
		limit = (g_peopleCounter->get() == g_peopleMax->get())?1:0;
		if (limit) {
			g_display_mode = -100;
			g_beeper->beep(1000);
		} else {
			g_beeper->beep();
		}
	} else {
		g_peopleCounter->dec();
	}
	g_progress->set((g_peopleCounter->get() * 100)/g_peopleMax->get());
	//g_bat->set((g_peopleCounter->get() * 100)/g_peopleMax->get());
	g_update_display = 1;
}
//===========================================================================================

/*!
 * \brief Got distance from US-100.
 */
void ICACHE_RAM_ATTR gotDistance(US100 *us100, int distanceMM)
{
	int distcm;

	/* Analize results */
	if (us100->getMaxDist() == distanceMM) { us100->trigSend(); return; }
	distcm = distanceMM/10;
#ifdef DEBUG_SONAR_DISTANCE
	g_update_display = g_debugDistance->set( distcm );
#endif
	if (door_length > SONAR_MARGIN_CM) {
		/* First algorithm - simple treshold based algorithm (door length must be set in configuration) */
		if (distcm < (door_length - SONAR_MARGIN_CM)) {
			g_us100_state = 1;
		} else {
			g_us100_state = 0;
		}
		pdebug("Got distance %d, door_length=%d, state = %d\n",distcm, door_length, g_us100_state);
		if (g_us100_state_last != g_us100_state) { /* Single measurement */
		//if ((g_us100_valid == g_us100_state) && (g_us100_state_last != g_us100_state)) { /* Double check */
			/* State change event */
			g_us100_state_last = g_us100_state;
			if (g_us100_state) {
				incCounter(1);
				us100->trigSendDelayed(100);
				return;
			}
		}
		g_us100_valid = g_us100_state;
		us100->trigSend();
	} else {
		/* Second algorithm use deltas */
		switch (g_us100_state) {
			/* Wait for enter */
			case 10: {
				int delta = (g_us100_state_last - distcm);
				if (delta > SONAR_MARGIN_CM) {
					if (g_us100_valid) {
						g_us100_state = 20;
						g_us100_state_last = g_us100_mark = distcm;
						incCounter(1);
						us100->trigSendDelayed(100);
						g_us100_valid = 0;
						return;
					} else {
						g_us100_valid = 1;
						us100->trigSend();
						return;
					}
				} else {
					g_us100_valid = 0;
				}
			} break;
			/* Wait for exit */
			case 20: {
				int delta = (distcm - g_us100_mark);
				if (delta < 0) g_us100_mark = distcm;
				if (delta > SONAR_MARGIN_CM) {
					if (g_us100_valid) {
						g_us100_state = 10;
						g_us100_valid = 0;
					} else {
						g_us100_valid = 1;
					}
				} else {
					g_us100_valid = 0;
				}
			} break;
			default: {g_us100_state = 10;g_us100_valid = 0;} break;
		}
		g_us100_state_last = distcm;
		us100->trigSend();
	}
}
//===========================================================================================

/*!
 * \brief Rotary change event, use ICACHE_RAM_ATTR for rotary event handler.
 */
void ICACHE_RAM_ATTR gotRotary(SimpleRotary *rot, int buttonEvent, int rotEvent)
{
	pdebug("Got rotary but=%d, rot=%d\n", buttonEvent, rotEvent);
	if (buttonEvent == 1) {
		/* Handle button press */
		if (g_display_mode == -100) {g_display_mode = 0;g_update_display = 1;return;}
		g_display_mode++;
		if (g_display_mode > DISPLAY_MODES) g_display_mode = 0;
		g_update_display = 1;
	} else if (buttonEvent == 2) {
		/* Long press - back to the main menu */
		g_display_mode   = 0;
		g_update_display = 1;
	} else if (rotEvent) {
		/* Rotary event - update */
		switch (g_display_mode) {
			case 0: {incCounter(rotEvent);} break;
			case 1: {if (rotEvent == 1) g_setMaxCounter->inc(); else g_setMaxCounter->dec(); g_update_display = 1;} break; /* Change people max */
			case 2: {g_orn->inc();g_display_mode_last=-1;g_update_display = 1;} break;
			case 3: {g_sndSW->inc();g_update_display = 1;} break;
			case 4: {if (rotEvent == 1) g_setDoorLength->inc(); else g_setDoorLength->dec(); g_update_display = 1;} break; /* Change door length */
			case -100: {g_display_mode = 0; g_update_display = 1;}
			default: break;
		}
	}
}
//===========================================================================================

/*!
 * \brief Setup.
 */
void setup()
{
	uint16_t flags, max_number;
	/* Disable WiFI */
	WiFi.mode(WIFI_OFF);
	WiFi.forceSleepBegin();
	delay(1); //Needed, at least in my tests WiFi doesn't power off without this for some reason
	//ESP.deepSleep(1000000, WAKE_RF_DEFAULT);
	/* Enable WIFI code */
#if 0
	/* https://www.bakke.online/index.php/2017/05/21/reducing-wifi-power-consumption-on-esp8266-part-2/ */
	WiFi.forceSleepWake();
	delay( 1 );
	// Bring up the WiFi connection
	WiFi.mode( WIFI_STA );
	WiFi.begin( WLAN_SSID, WLAN_PASSWD );
#endif
	Serial.begin(9600);
	pdebug("Setup start\n");
	/* Setup pins */
	pinMode(GPIO_TFT_RESET,OUTPUT);
	pinMode(GPIO_TFT_RS,OUTPUT);
	pinMode(GPIO_TFT_SCK,OUTPUT);
	pinMode(GPIO_TFT_MOSI,OUTPUT);
	pinMode(GPIO_TFT_CS,OUTPUT);
	pinMode(GPIO_LED,OUTPUT);
	digitalWrite(GPIO_LED, HIGH);
	digitalWrite(GPIO_TFT_RESET, HIGH);

	/* Global variables */
	g_us100            = new US100(gotDistance, 3000, 0, 100, 20);
	g_rot              = new SimpleRotary(GPIO_ROT_A, GPIO_ROT_B, GPIO_ROT_BUTTON, gotRotary);
	g_bat              = new Bat(119,3);
	g_batmon           = new BatMonitor([](BatMonitor *b, int a, int v, int p) {
		g_bat->set(p);
		g_update_display = 1;
		pdebug("Battery adu=%d, volt=%d [mV] (%d %%)\n",a,v,p);
	},3710,732);
	g_beeper           = new SimpleBeeper(GPIO_LED);
	g_peopleCounter    = new Counter(35, 31, 0);
	g_peopleMax        = new Counter(39, 7, 1);
#ifdef DEBUG_SONAR_DISTANCE
	g_debugDistance    = new Counter(69, 7, 2, 10, 500);
#endif
	g_setMaxCounter    = new Counter(35, 52, 0);
	g_setDoorLength    = new Counter(35, 66, 0);
	g_orn              = new Orientation(11,31);
	g_sndSW            = new Switch(50,60);
	g_progress         = new ProgressBar(0, 125, 160, 3, 0xDA19, 0x7B16, 0x9DEF);
	g_us100_state      = 0;
	g_us100_state_last = 0;
	g_display_mode     = 0;
	g_display_mode_last= -1;
	g_update_display   = 1;

	/* Set settings from eeprom */
	readFromEEPROM(&flags, &max_number, &door_length );
	g_peopleMax->set(max_number);
	g_setDoorLength->set(door_length);
	if (flags&1) g_orientation = 3; else g_orientation = 1;
	if (flags&2) g_sndSW->set(1); else g_sndSW->set(0);
	g_orn->setinit(flags&1);
	g_beeper->setEnabled(g_sndSW->get());

	/* Reset TFT */
	digitalWrite(GPIO_TFT_RESET, LOW);
	delay(1000);
	digitalWrite(GPIO_TFT_RESET, HIGH);
	/* Init TFT */
	TFT_init(GPIO_TFT_RS, GPIO_TFT_CS, 160, 128 );
	TFT_setRotation(g_orientation);
	/* Start */
	g_rot->start(2);
	g_us100->trigSend();
	pdebug("Setup end\n");
}
//===========================================================================================



/*!
 * \brief Event loop - battery and GUI opreations (low priority, high priority tasks are in timers).
 * draw screen         - 70ms
 * draw people counter - 8ms
 */
void loop()
{
	uint32_t m = millis();

	g_batmon->loop(m);
#ifdef DEBUG_SONAR_DISTANCE
	/* Support for lazy update */
	if (g_debugDistance->loop(m)) g_update_display=1;
#endif
	if (g_update_display) {
		g_update_display = 0;
		if (g_display_mode != g_display_mode_last) {
			switch (g_display_mode_last) {
				case 0:g_setMaxCounter->set(g_peopleMax->get());break;
				case 1:g_peopleMax->set(g_setMaxCounter->get());break;
				case 4:door_length = g_setDoorLength->get();break;
				default:break;
			}
			g_peopleCounter->invalidate();
			g_peopleMax->invalidate();
			g_bat->invalidate();
			g_setMaxCounter->invalidate();
			g_orn->invalidate();
			g_sndSW->invalidate();
			g_progress->invalidate();
			g_setDoorLength->invalidate();
#ifdef DEBUG_SONAR_DISTANCE
			g_debugDistance->invalidate();
#endif
			/* Update main screen */
			switch (g_display_mode) {
				case 0:{
					MAINWINDOW_draw(0,0);
					if (g_display_mode_last > 0) {
						uint16_t flags = (g_sndSW->get()<<1) | g_orn->get();
						/* save settings */
						writeIntoEEPROM(flags, g_peopleMax->get(), door_length);
						g_beeper->setEnabled(g_sndSW->get());
					}
				} break;
				case 1:{SETMAXWINDOW_draw(0,0);} break;
				case 2:{ORIENTATIONWINDOW_draw(0,0);} break;
				case 3:{SOUNDWINDOW_draw(0,0);} break;
				case 4:{SETDOORWIDTH_draw(0,0);} break;
				case -100:{LIMITWINDOW_draw(0,0);} break;
				default:break;
			}
			g_display_mode_last = g_display_mode;
		}
		/* Update widgets */
		switch (g_display_mode) {
			case 0:{
				g_peopleCounter->draw();
				g_peopleMax->draw();
				g_bat->draw();
				g_progress->draw();
#ifdef DEBUG_SONAR_DISTANCE
				g_debugDistance->draw();
#endif
			} break;
			case 1:{g_setMaxCounter->draw();} break;
			case 2:{g_orn->draw();} break;
			case 3:{g_sndSW->draw();} break;
			case 4:{g_setDoorLength->draw();} break;
			default:break;
		}
		pdebug("Redraw display time = %u ms\n", (uint32_t)(millis() - m));
	}
}
//===========================================================================================
