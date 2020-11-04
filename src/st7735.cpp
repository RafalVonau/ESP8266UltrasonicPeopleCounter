/*
	TFT_init(4,5,15,12,0,160,128);
	TFT_setRotation(1);
	TFT_setAddrWindow(0,0,160,128);
	TFT_fillScreen(TFT_Color565(255,0,0));
	TFT_fillRect(30,30,90,90,TFT_Color565(0,255,0));
	for (int qq=0; qq<160;qq++) TFT_drawPixel(qq,qq,TFT_Color565(0,0,255));
*/

#include "st7735.h"
#include "hspi.h"

//#define USE_DIRECTIO
//#define DEBUG_ENABLE

#ifdef DEBUG_ENABLE
#define EUDEBUG( ... ) {printf( __VA_ARGS__ );}
#else
#define EUDEBUG( ... )
#endif


#define DELAY 0x80

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif

uint8_t  _rs, _cs, _rs_mask, _cs_mask, colstart, rowstart,_width,_height,rotation; // some displays need this changed
uint8_t  tabcolor;

static const uint8_t TFT[] = { // TFT support only now
	9,
	0x011,	0+DELAY,10,
	0x013,	0,
	0x20,	0,
	0x03a,	1,	0x005,
	0x29,	0,
	0x036,	1,	0x000,
	0x02a,	4,	0,0,0,0x07f,
	0x02b,	4,	0,0,0,0x09f,
	0x2c,	0,
};



#ifndef USE_DIRECTIO
#define TFT_DC_COMMAND {digitalWrite(_rs,0);}
#define TFT_DC_DATA    {digitalWrite(_rs,1);}
#define TFT_CS_LOW     {digitalWrite(_cs,0);}
#define TFT_CS_HIGH    {hspi_wait_ready();digitalWrite(_cs,1);}
#else
#include "esp8266_gpio_direct.h"
#define TFT_DC_COMMAND {gpio_r->out_w1tc = _rs_mask;}
#define TFT_DC_DATA    {gpio_r->out_w1ts = _rs_mask;}
#define TFT_CS_LOW     {gpio_r->out_w1tc = _cs_mask;}
#define TFT_CS_HIGH    {hspi_wait_ready();gpio_r->out_w1ts = _cs_mask;}
#endif


inline uint16_t TFT_swapcolor(uint16_t x) { return (x << 11) | (x & 0x07E0) | (x >> 11); }

// Init when using software SPI.  All output pins are configurable.
void TFT_init(uint8_t rs, uint8_t cs, uint8_t qwidth,uint8_t qheight)
{
	_rs        = rs;
	_cs        = cs;
	_rs_mask   = (1u<<rs);
	_cs_mask   = (1u<<cs);
	hspi_init();
	_width  = qwidth;
	_height = qheight;
	TFT_commonInit(TFT);
}
//===========================================================================================

void TFT_transmitCmdData(uint8_t cmd, const uint8_t *data, uint8_t numDataByte)
{
	// Hardware implementation
	hspi_wait_ready();
	TFT_DC_COMMAND;
	TFT_CS_LOW;
	hspi_send_uint8(cmd);
	if (data) {
		hspi_wait_ready();
		TFT_DC_DATA;
		hspi_send_data(data, numDataByte);
	}
	TFT_CS_HIGH;
}
//===========================================================================================

void TFT_transmitCmdData2(uint8_t cmd, const uint8_t *data, uint8_t numDataByte)
{
	uint8_t i;
	// Hardware implementation
	hspi_wait_ready();
	TFT_DC_COMMAND;
	TFT_CS_LOW;
	hspi_send_uint8(cmd);
	hspi_wait_ready();
	TFT_DC_DATA;
	for (i = 0;i<numDataByte;++i) {
		hspi_wait_ready();
		hspi_send_uint8(data[i]);
	}
	TFT_CS_HIGH;
}
//===========================================================================================


// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void TFT_commandList(const uint8_t *addr) 
{
	uint8_t  numCommands, numArgs;
	uint16_t ms;
	uint8_t cmd;

	numCommands = *addr++;   // Number of commands to follow
	EUDEBUG("Number of commands = %d\n",numCommands);
	while (numCommands--) {                 // For each command...
		cmd      = *addr++;                  //   Read, issue command
		numArgs  = *addr++;                  //   Number of args to follow
		ms       = numArgs & DELAY;          //   If hibit set, delay follows args
		numArgs &= ~DELAY;                   //   Mask out delay bit
		EUDEBUG("Command = 0x%x, args=%d\n",cmd, numArgs);
		TFT_transmitCmdData2(cmd, addr, numArgs);
		addr += numArgs;
		if (ms) {
			ms = *addr++; // Read post-command delay time (ms)
			if(ms == 255) ms = 500;     // If 255, delay for 500 ms
			EUDEBUG("Wait(%d ms)\n",ms);
			delay(ms);
		}
	}
}
//===========================================================================================

// Initialization code for  TFT displays
void TFT_commonInit(const uint8_t *cmdList) 
{
	colstart  = rowstart = 0; // May be overridden in init func

	pinMode(_rs, OUTPUT);
	pinMode(_cs, OUTPUT);
	digitalWrite(_cs,1);
	delay(2);
	digitalWrite(_cs,0);
	if(cmdList) TFT_commandList(cmdList);
}
//===========================================================================================

void TFT_setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1,uint8_t y1) 
{
	uint8_t buf[4];
	
	buf[0] = 0x00;
	buf[1] = x0+colstart;
	buf[2] = 0x00;
	buf[3] = x1+colstart;
	TFT_transmitCmdData(TFT_CASET, buf, 4);
	
	buf[0] = 0x00;
	buf[1] = y0+rowstart;
	buf[2] = 0x00;
	buf[3] = y1+rowstart;
	TFT_transmitCmdData(TFT_RASET, buf, 4);
	
	TFT_transmitCmdData(TFT_RAMWR, NULL, 0); // write to RAM
}
//===========================================================================================

// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t TFT_Color565(uint8_t r, uint8_t g, uint8_t b) 
{
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
//===========================================================================================

#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

// Generally 0 - Portrait 1 - Landscape
void TFT_setRotation(uint8_t m) 
{
	uint8_t buf[4];
	rotation = m % 4; // can't be higher than 3
	switch (rotation) {
		case 0: {
			buf[0] = (MADCTL_MX | MADCTL_MY | MADCTL_BGR);
			_width  = TFT_TFTWIDTH;
			 _height = TFT_TFTHEIGHT;
		} break;
		case 1: {
			buf[0] = (MADCTL_MY | MADCTL_MV | MADCTL_BGR);
			_width  = TFT_TFTHEIGHT;
			_height = TFT_TFTWIDTH;
		} break;
		case 2: {
			buf[0] = (MADCTL_BGR);
			_width  = TFT_TFTWIDTH;
			_height = TFT_TFTHEIGHT;
		} break;
		case 3: {
			buf[0] = (MADCTL_MX | MADCTL_MV | MADCTL_BGR);
			_width  = TFT_TFTHEIGHT;
			_height = TFT_TFTWIDTH;
		} break;
	}
	TFT_transmitCmdData(TFT_MADCTL, buf, 1);
}
//===========================================================================================


void TFT_invertDisplay(uint8_t i) 
{
	TFT_transmitCmdData(i ? TFT_INVON : TFT_INVOFF, NULL, 0);
}
//===========================================================================================


/**
 * Return with the bitmap of a font.
 * @param font_p pointer to a font
 * @param letter a letter
 * @return  pointer to the bitmap of the letter
 */
const uint8_t *TFT_font_get_bitmap(const lv_font_t *font_p, uint32_t letter, uint8_t *w)
{
	const lv_font_t * font_i = font_p;
	while(font_i != NULL) {
		if(letter >= font_i->first_ascii && letter <= font_i->last_ascii) {
			uint32_t index = (letter - font_i->first_ascii);
			*w=font_i->width[index];
			return &font_i->bitmap[font_i->map[index]];
		}
		font_i = font_i->next_page;
	}
	return NULL;
}
//===========================================================================================


#define writePixel(x,y,c) TFT_drawPixel(x,y,c)

void TFT_drawPixel(int16_t x, int16_t y, uint16_t color) 
{
	if ((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) return;
	TFT_setAddrWindow(x,y,x+1,y+1);
	hspi_wait_ready();
	TFT_DC_DATA;
	TFT_CS_LOW;
	hspi_send_uint16(color);
	TFT_CS_HIGH;
}
//===========================================================================================

void TFT_drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
	// Rudimentary clipping
	if((x >= _width) || (y >= _height)) return;
	if((y+h-1) >= _height) h = _height-y;
	/* Direct write to TFT */
	TFT_setAddrWindow(x, y, x, y+h-1);
	hspi_wait_ready();
	TFT_DC_DATA;
	TFT_CS_LOW;
	hspi_send_uint16_r(color, h);
	TFT_CS_HIGH;
}
//===========================================================================================

void TFT_drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
	// Rudimentary clipping
	if((x >= _width) || (y >= _height)) return;
	if((x+w-1) >= _width)  w = _width-x;
	/* Direct write to TFT */
	TFT_setAddrWindow(x, y, x+w-1, y);
	hspi_wait_ready();
	TFT_DC_DATA;
	TFT_CS_LOW;
	hspi_send_uint16_r(color, w);
	TFT_CS_HIGH;
}
//===========================================================================================


void TFT_fillScreen(uint16_t color) 
{
	TFT_fillRect(0, 0,  _width, _height, color);
}
//===========================================================================================

// Draw a rectangle
void TFT_drawRect(int16_t x, int16_t y, int16_t w, int16_t h,uint16_t color) 
{
	TFT_drawFastHLine(x, y, w, color);
	TFT_drawFastHLine(x, y+h-1, w, color);
	TFT_drawFastVLine(x, y, h, color);
	TFT_drawFastVLine(x+w-1, y, h, color);
}
//===========================================================================================

// fill a rectangle
void TFT_fillRect(int16_t x, int16_t y, int16_t w, int16_t h,uint16_t color) 
{
	if((x >= _width) || (y >= _height)) return;
	if((x + w - 1) >= _width)  w = _width  - x;
	if((y + h - 1) >= _height) h = _height - y;
	TFT_setAddrWindow(x, y, x+w-1, y+h-1);
	hspi_wait_ready();
	TFT_DC_DATA;
	TFT_CS_LOW;
	hspi_send_uint16_r(color, h*w);
	TFT_CS_HIGH;
}
//===========================================================================================

// Draw pixmap
void TFT_drawPixmap(int16_t x, int16_t y, int16_t w, int16_t h,const uint16_t *data)
{
	int i,s;
	// rudimentary clipping (drawChar w/big text requires this)
	if((x >= _width) || (y >= _height)) return;
	if((x + w - 1) >= _width)  w = _width  - x;
	if((y + h - 1) >= _height) h = _height - y;
	s = ((int)w)*((int)h);
	TFT_setAddrWindow(x, y, x+w-1, y+h-1);
	hspi_wait_ready();
	TFT_DC_DATA;
	TFT_CS_LOW;
#if 1
	/* Send word by word */
	for (i=0;i < s;++i) {
		hspi_send_uint16(pgm_read_word(&data[i]));
		hspi_wait_ready();
	}
#else
	hspi_send_data_uint16(data, s);
#endif
	TFT_CS_HIGH;
}
//===========================================================================================

// Draw pixmap repeat line horizontaly
void TFT_drawPixmapH(int16_t x, int16_t y, int16_t w, int16_t h,const uint16_t *data)
{
	int i,j;
	// rudimentary clipping (drawChar w/big text requires this)
	if((x >= _width) || (y >= _height)) return;
	if((x + w - 1) >= _width)  w = _width  - x;
	if((y + h - 1) >= _height) h = _height - y;
	TFT_setAddrWindow(x, y, x+w-1, y+h-1);
	hspi_wait_ready();
	TFT_DC_DATA;
	TFT_CS_LOW;
	/* Send word by word */
	for (j=0;j<h;++j) {
		for (i=0;i<w;++i) {
			hspi_send_uint16(pgm_read_word(&data[j]));
			hspi_wait_ready();
		}
	}
	TFT_CS_HIGH;
}
//===========================================================================================



// draw a character
uint8_t TFT_drawChar(lv_font_t *f, uint8_t x, uint8_t y, uint32_t letter, uint16_t color, uint16_t bgColor)
{
	uint8_t h,w,col,col_sub,row;
	const uint8_t *bitmap_p;

	bitmap_p = TFT_font_get_bitmap(f, letter, &w);
	if (!bitmap_p) return 0;
	h = f->height_row;
	if((x >= _width) || (y >= _height)) return 0;
	if((x + w - 1) >= _width)  return 0;
	if((y + h - 1) >= _height) return 0;

	TFT_setAddrWindow(x, y, x+w-1, y+h-1);
	hspi_wait_ready();
	TFT_DC_DATA;
	TFT_CS_LOW;
	/* Send line by line */
	for (row = 0; row < h; row++) {
		for(col = 0, col_sub = 7; col < w; col ++, col_sub--) {
			if (*bitmap_p & (1 << col_sub)) {
				hspi_send_uint16(color);
			} else {
				hspi_send_uint16(bgColor);
			}
			if(col_sub == 0) {
				bitmap_p++;
				col_sub = 8;
			}
		}
		/*Go to the next row*/
		if(col_sub != 7) bitmap_p++;   /*Go to the next byte if it not done in the last step*/
	}
	TFT_CS_HIGH;
	return w;
}
//===========================================================================================

uint8_t TFT_drawString(lv_font_t *f, uint8_t x, uint8_t y, const char *str, uint16_t color, uint16_t bgColor)
{
	char ch = *str++;
	
	while (ch) {
		x += TFT_drawChar(f, x, y, ch, color, bgColor);
		ch = *str++;
	}
	return x;
}
//===========================================================================================

// Draw a circle outline
void TFT_drawCircle(int16_t x0, int16_t y0, int16_t r,uint16_t color)
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	writePixel(x0  , y0+r, color);
	writePixel(x0  , y0-r, color);
	writePixel(x0+r, y0  , color);
	writePixel(x0-r, y0  , color);

	while (x<y) {
		if (f >= 0) {y--;ddF_y += 2;f += ddF_y;}
		x++;
		ddF_x += 2;
		f += ddF_x;
		writePixel(x0 + x, y0 + y, color);
		writePixel(x0 - x, y0 + y, color);
		writePixel(x0 + x, y0 - y, color);
		writePixel(x0 - x, y0 - y, color);
		writePixel(x0 + y, y0 + x, color);
		writePixel(x0 - y, y0 + x, color);
		writePixel(x0 + y, y0 - x, color);
		writePixel(x0 - y, y0 - x, color);
	}
}
//===========================================================================================

void TFT_drawCircleHelper( int16_t x0, int16_t y0,int16_t r, uint8_t cornername, uint16_t color);
void TFT_drawCircleHelper( int16_t x0, int16_t y0,int16_t r, uint8_t cornername, uint16_t color)
{
	int16_t f     = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x     = 0;
	int16_t y     = r;

	while (x<y) {
		if (f >= 0) {y--;ddF_y += 2;f+= ddF_y;}
		x++;
		ddF_x += 2;
		f     += ddF_x;
		if (cornername & 0x4) {
			writePixel(x0 + x, y0 + y, color);
			writePixel(x0 + y, y0 + x, color);
		}
		if (cornername & 0x2) {
			writePixel(x0 + x, y0 - y, color);
			writePixel(x0 + y, y0 - x, color);
		}
		if (cornername & 0x8) {
			writePixel(x0 - y, y0 + x, color);
			writePixel(x0 - x, y0 + y, color);
		}
		if (cornername & 0x1) {
			writePixel(x0 - y, y0 - x, color);
			writePixel(x0 - x, y0 - y, color);
		}
	}
}
//===========================================================================================

// Used to do circles and roundrects
void TFT_fillCircleHelper(int16_t x0, int16_t y0, int16_t r,uint8_t cornername, int16_t delta, uint16_t color);
void TFT_fillCircleHelper(int16_t x0, int16_t y0, int16_t r,uint8_t cornername, int16_t delta, uint16_t color)
{
	int16_t f     = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x     = 0;
	int16_t y     = r;

	while (x<y) {
		if (f >= 0) {y--;ddF_y += 2;f += ddF_y;}
		x++;
		ddF_x += 2;
		f     += ddF_x;
		if (cornername & 0x1) {
			TFT_drawFastVLine(x0+x, y0-y, 2*y+1+delta, color);
			TFT_drawFastVLine(x0+y, y0-x, 2*x+1+delta, color);
		}
		if (cornername & 0x2) {
			TFT_drawFastVLine(x0-x, y0-y, 2*y+1+delta, color);
			TFT_drawFastVLine(x0-y, y0-x, 2*x+1+delta, color);
		}
	}
}
//===========================================================================================

void TFT_fillCircle(int16_t x0, int16_t y0, int16_t r,uint16_t color) 
{
	TFT_drawFastVLine(x0, y0-r, 2*r+1, color);
	TFT_fillCircleHelper(x0, y0, r, 3, 0, color);
}
//===========================================================================================

// Draw a rounded rectangle
void TFT_drawRoundRect(int16_t x, int16_t y, int16_t w,int16_t h, int16_t r, uint16_t color) 
{
	TFT_drawFastHLine(x+r  , y    , w-2*r, color); // Top
	TFT_drawFastHLine(x+r  , y+h-1, w-2*r, color); // Bottom
	TFT_drawFastVLine(x    , y+r  , h-2*r, color); // Left
	TFT_drawFastVLine(x+w-1, y+r  , h-2*r, color); // Right
	// draw four corners
	TFT_drawCircleHelper(x+r    , y+r    , r, 1, color);
	TFT_drawCircleHelper(x+w-r-1, y+r    , r, 2, color);
	TFT_drawCircleHelper(x+w-r-1, y+h-r-1, r, 4, color);
	TFT_drawCircleHelper(x+r    , y+h-r-1, r, 8, color);
}
//===========================================================================================

// Fill a rounded rectangle
void TFT_fillRoundRect(int16_t x, int16_t y, int16_t w,int16_t h, int16_t r, uint16_t color) 
{
	TFT_fillRect(x+r, y, w-2*r, h, color);
	// draw four corners
	TFT_fillCircleHelper(x+w-r-1, y+r, r, 1, h-2*r-1, color);
	TFT_fillCircleHelper(x+r    , y+r, r, 2, h-2*r-1, color);
}
//===========================================================================================

// Bresenham's algorithm - thx wikpedia
void TFT_writeLineHelper(int16_t x0, int16_t y0, int16_t x1, int16_t y1,uint16_t color);
void TFT_writeLineHelper(int16_t x0, int16_t y0, int16_t x1, int16_t y1,uint16_t color)
{
	int16_t steep = abs(y1 - y0) > abs(x1 - x0);
	int16_t dx, dy;
	
	if (steep) {_swap_int16_t(x0, y0);_swap_int16_t(x1, y1);}
	if (x0 > x1) {_swap_int16_t(x0, x1);_swap_int16_t(y0, y1);}
	
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1) {ystep = 1;} else {ystep = -1;}

	for (; x0<=x1; x0++) {
		if (steep) {writePixel(y0, x0, color);} else {writePixel(x0, y0, color);}
		err -= dy;
		if (err < 0) {y0 += ystep;err += dx;}
	}
}
//===========================================================================================

void TFT_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,uint16_t color) 
{
	// Update in subclasses if desired!
	if(x0 == x1){
		if(y0 > y1) _swap_int16_t(y0, y1);
		TFT_drawFastVLine(x0, y0, y1 - y0 + 1, color);
	} else if(y0 == y1){
		if(x0 > x1) _swap_int16_t(x0, x1);
		TFT_drawFastHLine(x0, y0, x1 - x0 + 1, color);
	} else {
		TFT_writeLineHelper(x0, y0, x1, y1, color);
	}
}
//===========================================================================================

// Draw a triangle
void TFT_drawTriangle(int16_t x0, int16_t y0,int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) 
{
	TFT_drawLine(x0, y0, x1, y1, color);
	TFT_drawLine(x1, y1, x2, y2, color);
	TFT_drawLine(x2, y2, x0, y0, color);
}
//===========================================================================================

// Fill a triangle
void TFT_fillTriangle(int16_t x0, int16_t y0,int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) 
{
	int16_t a, b, y, last;

	// Sort coordinates by Y order (y2 >= y1 >= y0)
	if (y0 > y1) {_swap_int16_t(y0, y1); _swap_int16_t(x0, x1);}
	if (y1 > y2) {_swap_int16_t(y2, y1); _swap_int16_t(x2, x1);}
	if (y0 > y1) {_swap_int16_t(y0, y1); _swap_int16_t(x0, x1);}
	
	if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
		a = b = x0;
		if(x1 < a)      a = x1;
		else if(x1 > b) b = x1;
		if(x2 < a)      a = x2;
		else if(x2 > b) b = x2;
		TFT_drawFastHLine(a, y0, b-a+1, color);
		return;
	}
	int16_t
	dx01 = x1 - x0,
	dy01 = y1 - y0,
	dx02 = x2 - x0,
	dy02 = y2 - y0,
	dx12 = x2 - x1,
	dy12 = y2 - y1;
	int32_t
	sa   = 0,
	sb   = 0;
	if(y1 == y2) last = y1;   // Include y1 scanline
	else         last = y1-1; // Skip it
	
	for(y=y0; y<=last; y++) {
		a   = x0 + sa / dy01;
		b   = x0 + sb / dy02;
		sa += dx01;
		sb += dx02;
		if(a > b) _swap_int16_t(a,b);
		TFT_drawFastHLine(a, y, b-a+1, color);
	}
	sa = dx12 * (y - y1);
	sb = dx02 * (y - y0);
	for(; y<=y2; y++) {
		a   = x1 + sa / dy12;
		b   = x0 + sb / dy02;
		sa += dx12;
		sb += dx02;
		if(a > b) _swap_int16_t(a,b);
		TFT_drawFastHLine(a, y, b-a+1, color);
	}
}
//===========================================================================================

uint8_t TFT_display_temperature_to_buf(uint8_t temp0, uint8_t temp1 , char *buf)
{
	uint16_t k,d=100;
	uint16_t i,first = 0;

	for (i = 0; i < 3; ++i) {
		k = temp0/d;
		temp0%=d;
		d/=10;
		if (k) {
			buf[i] = (char)('0'+k);
			first = 1;
		} else {
			if ((first) || (i==2)) {
				buf[i] = '0';
			} else {
				buf[i] = ' ';
			}
		}
	}
	buf[3] = '.';
	buf[4] = (char)('0'+(temp1/10));
	buf[5] = (char)('0'+(temp1%10));
	buf[6] = 176;
	buf[7] = 'C';
	buf[8] = '\0';
	return 8;
}
//===========================================================================================

uint8_t TFT_display_temperature(lv_font_t *f,  uint8_t x, uint8_t y, uint8_t temp0, uint8_t temp1, uint16_t color, uint16_t bgColor)
{
	char buf[16];
	TFT_display_temperature_to_buf(temp0, temp1, buf);
	return TFT_drawString(f, x, y, buf, color, bgColor);
}
//===========================================================================================

uint8_t TFT_display_time(lv_font_t *f,  uint8_t x, uint8_t y, uint8_t tmin, uint8_t tsec, uint16_t color, uint16_t bgColor)
{
	char buf[8];
	buf[0] = '0'+(tmin/10);
	buf[1] = '0'+(tmin%10);
	buf[2] = ':';
	buf[3] = '0'+(tsec/10);
	buf[4] = '0'+(tsec%10);
	buf[5] = 's';
	buf[6] = '\0';
	return TFT_drawString(f, x, y, buf, color, bgColor);
}
//===========================================================================================

uint8_t TFT_display_set(lv_font_t *f,  uint8_t x, uint8_t y, int8_t tmin, uint8_t tsec, uint16_t color, uint16_t color0, uint16_t color1, uint16_t bgColor)
{
	char buf[4];
	if (tmin > -1) {
		buf[0] = '0'+(tmin/10);
		buf[1] = '0'+(tmin%10);
		buf[2] = '\0';
		x = TFT_drawString(f, x, y, buf, color0, bgColor);
		buf[0] = ':';
		buf[1] = '\0';
		x = TFT_drawString(f, x, y, buf, color, bgColor);
		buf[0] = '0'+(tsec/10);
		buf[1] = '0'+(tsec%10);
		buf[2] = '\0';
		x = TFT_drawString(f, x, y, buf, color1, bgColor);
		buf[0] = 's';
		buf[1] = '\0';
		return TFT_drawString(f, x, y, buf, color, bgColor);
	} else {
		buf[0] = ' ';
		buf[1] = ' ';
		buf[2] = ' ';
		buf[3] = '\0';
		x = TFT_drawString(f, x, y, buf, color, bgColor);
		buf[0] = '0'+(tsec/10);
		buf[1] = '0'+(tsec%10);
		buf[2] = '\0';
		x = TFT_drawString(f, x, y, buf, color1, bgColor);
		buf[0] = 176;
		buf[1] = 'C';
		buf[2] = '\0';
		return TFT_drawString(f, x, y, buf, color, bgColor);
	}
}
//===========================================================================================

