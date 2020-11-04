#include "pgmspace.h"

// memcpy_P(buf, SETMAXWINDOW_IMAGE, 20480);

#define SETMAXWINDOW_IMAGE_LEN 20480
#define SETMAXWINDOW_IMAGE_W 160
#define SETMAXWINDOW_IMAGE_H 128

extern const uint16_t SETMAXWINDOW_IMAGE[20480] PROGMEM;

#define SETMAXWINDOW_draw(x,y) TFT_drawPixmap(x, y, SETMAXWINDOW_IMAGE_W, SETMAXWINDOW_IMAGE_H,SETMAXWINDOW_IMAGE)


