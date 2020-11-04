#ifndef __ST7735__
#define __ST7735__

#include <functional>
#include <memory>
#include <ESP8266WiFi.h>
extern "C" {
  #include <espnow.h>
  #include "user_interface.h"
}

/* Use in memory Frame buffer or not */
//#define TFT_FB_EN

#define TFT_TFTWIDTH  128
#define TFT_TFTHEIGHT 160

// Some used command definitions kept from original
#define TFT_INVOFF  0x20
#define TFT_INVON   0x21
#define TFT_DISPOFF 0x28
#define TFT_DISPON  0x29
#define TFT_CASET   0x2A
#define TFT_RASET   0x2B
#define TFT_RAMWR   0x2C
#define TFT_RAMRD   0x2E

#define TFT_PTLAR   0x30
#define TFT_COLMOD  0x3A
#define TFT_MADCTL  0x36

// Basic colour definitions
#define	TFT_BLACK   0x0000
#define	TFT_RED     0xF800
#define	TFT_GREEN   0x07E0
#define	TFT_BLUE    0x001F
#define TFT_YELLOW  0xFFE0
#define TFT_MAGENTA 0xF81F
#define TFT_CYAN    0x07FF  
#define TFT_WHITE   0xFFFF
#define TFT_GREY    0x632C

typedef struct _lv_font_struct
{
	uint32_t first_ascii;
	uint32_t last_ascii;
	uint8_t height_row;
	const uint8_t * bitmap;
	const uint32_t * map;
	const uint8_t * width;
	struct _lv_font_struct * next_page;    /*Pointer to a font extension*/
} lv_font_t;

#define TFT_HW_SPI

void TFT_init(uint8_t RS, uint8_t CS,uint8_t qwidth,uint8_t qheight );
void TFT_setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void TFT_setRotation(uint8_t r);
void TFT_invertDisplay(uint8_t i);
void TFT_transmitCmdData(uint8_t cmd, const uint8_t *data, uint8_t numDataByte);
void TFT_commandList(const uint8_t *addr);
void TFT_commonInit(const uint8_t *cmdList);


/* GFX */
uint16_t TFT_Color565(uint8_t r, uint8_t g, uint8_t b);
void TFT_fillScreen(uint16_t color);

void TFT_drawPixel(int16_t x, int16_t y, uint16_t color);

void TFT_drawPixmap(int16_t x, int16_t y, int16_t w, int16_t h,const uint16_t *data);
// Draw pixmap repeat line horizontaly
void TFT_drawPixmapH(int16_t x, int16_t y, int16_t w, int16_t h,const uint16_t *data);

void TFT_drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
void TFT_drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
void TFT_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,uint16_t color);


void TFT_drawCircle(int16_t x0, int16_t y0, int16_t r,uint16_t color);
void TFT_fillCircle(int16_t x0, int16_t y0, int16_t r,uint16_t color);

void TFT_drawRect(int16_t x, int16_t y, int16_t w, int16_t h,uint16_t color);
void TFT_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

void TFT_drawRoundRect(int16_t x, int16_t y, int16_t w,int16_t h, int16_t r, uint16_t color);
void TFT_fillRoundRect(int16_t x, int16_t y, int16_t w,int16_t h, int16_t r, uint16_t color);

uint8_t TFT_drawChar(lv_font_t *f, uint8_t x, uint8_t y, uint32_t letter, uint16_t color, uint16_t bgColor);
uint8_t TFT_drawString(lv_font_t *f, uint8_t x, uint8_t y, const char *str, uint16_t color, uint16_t bgColor);

void TFT_drawTriangle(int16_t x0, int16_t y0,int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void TFT_fillTriangle(int16_t x0, int16_t y0,int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);

uint8_t TFT_display_temperature_to_buf(uint8_t temp0, uint8_t temp1 , char *buf);
uint8_t TFT_display_temperature(lv_font_t *f,  uint8_t x, uint8_t y, uint8_t temp0, uint8_t temp1, uint16_t color, uint16_t bgColor);
uint8_t TFT_display_time(lv_font_t *f,  uint8_t x, uint8_t y, uint8_t tmin, uint8_t tsec, uint16_t color, uint16_t bgColor);
uint8_t TFT_display_set(lv_font_t *f,  uint8_t x, uint8_t y, int8_t tmin, uint8_t tsec, uint16_t color, uint16_t color0, uint16_t color1, uint16_t bgColor);

#endif
