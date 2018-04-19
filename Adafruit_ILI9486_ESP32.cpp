/*
See rights and use declaration in License.h
This library has been modified for the Maple Mini.
Includes DMA transfers on DMA1 CH2 and CH3.

-----------
modified by IOXhop (www.ioxhop.com)
 - Edit SPI to standard SPI library.
-----------
*/
#include "Adafruit_ILI9486_ESP32.h"

uint16_t lineBuffer[1];

/*****************************************************************************/
// Constructor uses hardware SPI, the pins being specific to each device
/*****************************************************************************/
Adafruit_ILI9486_ESP32::Adafruit_ILI9486_ESP32(void) : Adafruit_GFX(TFTWIDTH, TFTHEIGHT){}


/*****************************************************************************/

/*****************************************************************************/
// https://github.com/adafruit/adafruit-rpi-fbtft/blob/35890c52f9e3eef3237b76acc295585dd93fc8cd/fb_ili9486.c
#define DELAY 0x80
/*****************************************************************************/
const uint8_t ili9486_init_sequence[] =
{
	2, 0xb0, 0x0,	// Interface Mode Control
	1, 0x11,		// Sleep OUT
	DELAY, 150,
	2, 0x3A, 0x66,	// use 18 bits per pixel color the only mode available on SPI-3wire ou SPI-4wire
	2, 0x36, 0x48,	// MX, BGR == rotation 0
	2, 0xC2, 0x44,	// Power Control 3
	// VCOM Control 1
	5, 0xC5, 0x00, 0x00, 0x00, 0x00,
	// PGAMCTRL(Positive Gamma Control)
	16, 0xE0, 0x0F, 0x1F, 0x1C, 0x0C, 0x0F, 0x08, 0x48, 0x98,
	          0x37, 0x0A, 0x13, 0x04, 0x11, 0x0D, 0x00,
	// NGAMCTRL(Negative Gamma Control)
	16, 0xE1, 0x0F, 0x32, 0x2E, 0x0B, 0x0D, 0x05, 0x47, 0x75,
	          0x37, 0x06, 0x10, 0x03, 0x24, 0x20, 0x00,
	// Digital Gamma Control 1
	16, 0xE2, 0x0F, 0x32, 0x2E, 0x0B, 0x0D, 0x05, 0x47, 0x75,
	          0x37, 0x06, 0x10, 0x03, 0x24, 0x20, 0x00,
	1, 0x11,	// Sleep OUT
	DELAY, 150, 	// wait some time
	1, 0x29,	// Display ON
	0			// end marker
};


/******************************************************************************
** Default begin                                                             **
******************************************************************************/
void Adafruit_ILI9486_ESP32::begin(void)
{


	begin(TFT_SCK, TFT_MOSI, TFT_DC, TFT_CS, TFT_MISO, TFT_RST, TFT_BLK);

}

/******************************************************************************
** Parametrized Begin                                                        **
******************************************************************************/
void Adafruit_ILI9486_ESP32::begin(uint8_t spiClk, uint8_t spiMOSI, uint8_t tftDC, uint8_t tftCS,
										  uint8_t spiMISO, uint8_t tftReset, uint8_t tftBLK)
{

	// Store pins
	// Set Default
	_spiClk  = spiClk;
	_spiMISO = spiMISO;
	_spiMOSI = spiMOSI;
	_tftDC   = tftDC;
	_tftReset= tftReset;
	_tftCS   = tftCS;
	_tftBLK  = tftBLK;


	pinMode(tftDC, OUTPUT);

	if (_tftBLK > 0) {
		pinMode(_tftBLK, OUTPUT);
		digitalWrite(_tftBLK, LOW);
	}

	if (_tftCS > 0) {
		pinMode(_tftCS, OUTPUT);
		digitalWrite(_tftCS, HIGH);
	}

	// toggle RST low to reset
	if (_tftReset > 0) {
		pinMode(_tftReset, OUTPUT);
		digitalWrite(_tftReset, HIGH);
		delay(20);
		digitalWrite(_tftReset, LOW);
		delay(20);
		digitalWrite(_tftReset, HIGH);
		delay(200);
	}

	SPI.begin(_spiClk, _spiMISO, _spiMOSI, _tftCS);

	SPI.beginTransaction(SPISettings(TFT_SPI_SPEED, MSBFIRST, SPI_MODE0));
	commandList(ili9486_init_sequence);

	if (_tftBLK > 0) {
		digitalWrite(_tftBLK, HIGH);
	}

}


/*****************************************************************************/
void Adafruit_ILI9486_ESP32::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	writeCommand(ILI9486_CASET); // Column addr set
	writeData(x0 >> 8);
	writeData(x0 & 0xFF);     // XSTART
	writeData(x1 >> 8);
	writeData(x1 & 0xFF);     // XEND

	writeCommand(ILI9486_PASET); // Row addr set
	writeData(y0 >> 8);
	writeData(y0);     // YSTART
	writeData(y1 >> 8);
	writeData(y1);     // YEND

	writeCommand(ILI9486_RAMWR); // write to RAM
}

/*****************************************************************************/
void Adafruit_ILI9486_ESP32::pushColor(uint16_t color)
{
	CD_DATA();
	CS_ON();
		SPI.transfer( (((color>>11) & 0xb0011111) << 3) & 0xfc);
		SPI.transfer( (((color>>5)  & 0xb0111111) << 2) & 0xfc);
		SPI.transfer( ((color       & 0xb0011111) << 3) & 0xfc);
	CS_OFF();
}

void Adafruit_ILI9486_ESP32::pushColorN(uint16_t color, uint32_t n)
{
	CD_DATA();
	CS_ON();

	for(uint32_t i=0; i<n;i++) {
		SPI.transfer( (((color>>11) & 0xb0011111) << 3) & 0xfc);
		SPI.transfer( (((color>>5)  & 0xb0111111) << 2) & 0xfc);
		SPI.transfer( ((color       & 0xb0011111) << 3) & 0xfc);
	}
	CS_OFF();

}

/*****************************************************************************/
void Adafruit_ILI9486_ESP32::drawPixel(int16_t x, int16_t y, uint16_t color)
{
	if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) return;

	setAddrWindow(x, y, x, y);
	pushColor(color);
}
/*****************************************************************************/
void Adafruit_ILI9486_ESP32::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
	// Rudimentary clipping
	if ((x >= _width) || (y >= _height || h < 1)) return;
	if ((y + h - 1) >= _height)	{ h = _height - y; }
	if (h < 2 ) { drawPixel(x, y, color); return; }

	setAddrWindow(x, y, x, y + h - 1);
	pushColorN(color,h);
}
/*****************************************************************************/
void Adafruit_ILI9486_ESP32::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
	// Rudimentary clipping
	if ((x >= _width) || (y >= _height || w < 1)) return;
	if ((x + w - 1) >= _width) { w = _width - x; }
	if (w < 2 ) { drawPixel(x, y, color); return; }

	setAddrWindow(x, y, x + w - 1, y);
	pushColorN(color,w);
}

/*****************************************************************************/
void Adafruit_ILI9486_ESP32::fillScreen(uint16_t color)
{
	setAddrWindow(0, 0,  _width, _height);
	pushColorN(color, (_width*_height) );
}

/*****************************************************************************/
void Adafruit_ILI9486_ESP32::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
	// rudimentary clipping (drawChar w/big text requires this)
	if ((x >= _width) || (y >= _height || h < 1 || w < 1)) return;
	if ((x + w - 1) >= _width) { w = _width  - x; }
	if ((y + h - 1) >= _height) { h = _height - y; }
	if (w == 1 && h == 1) {
		drawPixel(x, y, color);
		return;
	}

	setAddrWindow(x, y, x + w - 1, y + h - 1);
	pushColorN(color, (w*h) );
}

/*
* Draw lines faster by calculating straight sections and drawing them with fastVline and fastHline.
*/
/*****************************************************************************/
void Adafruit_ILI9486_ESP32::drawLine(int16_t x0, int16_t y0,int16_t x1, int16_t y1, uint16_t color)
{
	if ((y0 < 0 && y1 <0) || (y0 > _height && y1 > _height)) return;
	if ((x0 < 0 && x1 <0) || (x0 > _width && x1 > _width)) return;
	if (x0 < 0) x0 = 0;
	if (x1 < 0) x1 = 0;
	if (y0 < 0) y0 = 0;
	if (y1 < 0) y1 = 0;

	if (y0 == y1) {
		if (x1 > x0) {
			drawFastHLine(x0, y0, x1 - x0 + 1, color);
		}
		else if (x1 < x0) {
			drawFastHLine(x1, y0, x0 - x1 + 1, color);
		}
		else {
			drawPixel(x0, y0, color);
		}
		return;
	}
	else if (x0 == x1) {
		if (y1 > y0) {
			drawFastVLine(x0, y0, y1 - y0 + 1, color);
		}
		else {
			drawFastVLine(x0, y1, y0 - y1 + 1, color);
		}
		return;
	}

	bool steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		swap(x0, y0);
		swap(x1, y1);
	}
	if (x0 > x1) {
		swap(x0, x1);
		swap(y0, y1);
	}

	int16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1) {
		ystep = 1;
	}
	else {
		ystep = -1;
	}

	int16_t xbegin = x0;
	if (steep) {
		for (; x0 <= x1; x0++) {
			err -= dy;
			if (err < 0) {
				int16_t len = x0 - xbegin;
				if (len) {
					drawFastVLine (y0, xbegin, len + 1, color);
					//writeVLine_cont_noCS_noFill(y0, xbegin, len + 1);
				}
				else {
					drawPixel(y0, x0, color);
					//writePixel_cont_noCS(y0, x0, color);
				}
				xbegin = x0 + 1;
				y0 += ystep;
				err += dx;
			}
		}
		if (x0 > xbegin + 1) {
			//writeVLine_cont_noCS_noFill(y0, xbegin, x0 - xbegin);
			drawFastVLine(y0, xbegin, x0 - xbegin, color);
		}

	}
	else {
		for (; x0 <= x1; x0++) {
			err -= dy;
			if (err < 0) {
				int16_t len = x0 - xbegin;
				if (len) {
					drawFastHLine(xbegin, y0, len + 1, color);
					//writeHLine_cont_noCS_noFill(xbegin, y0, len + 1);
				}
				else {
					drawPixel(x0, y0, color);
					//writePixel_cont_noCS(x0, y0, color);
				}
				xbegin = x0 + 1;
				y0 += ystep;
				err += dx;
			}
		}
		if (x0 > xbegin + 1) {
			//writeHLine_cont_noCS_noFill(xbegin, y0, x0 - xbegin);
			drawFastHLine(xbegin, y0, x0 - xbegin, color);
		}
	}
}
/*****************************************************************************/
// Pass 8-bit (each) R,G,B, get back 16-bit packed color
/*****************************************************************************/
uint16_t Adafruit_ILI9486_ESP32::color565(uint8_t r, uint8_t g, uint8_t b)
{
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

/*****************************************************************************/
void Adafruit_ILI9486_ESP32::setRotation(uint8_t m)
{
	writeCommand(ILI9486_MADCTL);
	rotation = m & 3; // can't be higher than 3
	switch (rotation) {
		case 0:
			writeData(MADCTL_MX |MADCTL_BGR);
			_width  = TFTWIDTH;
			_height = TFTHEIGHT;
			break;
		case 1:
			writeData(MADCTL_MV | MADCTL_BGR);
			_width  = TFTHEIGHT;
			_height = TFTWIDTH;
			break;
		case 2:
			writeData(MADCTL_MY | MADCTL_BGR);
			_width  = TFTWIDTH;
			_height = TFTHEIGHT;
			break;
		case 3:
			writeData(MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
			_width  = TFTHEIGHT;
			_height = TFTWIDTH;
			break;
	}
}
/*****************************************************************************/
void Adafruit_ILI9486_ESP32::invertDisplay(boolean i)
{
	writeCommand(i ? ILI9486_INVON : ILI9486_INVOFF);
}


/* ===================================================================================
== UTILITIES                                                                        ==
====================================================================================*/

/*****************************************************************************/
// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
/*****************************************************************************/
void Adafruit_ILI9486_ESP32::commandList(const uint8_t *addr)
{
	uint8_t  numBytes, tmp;

	while ( (numBytes=(*addr++))>0 ) { // end marker == 0
		if ( numBytes&DELAY ) {
			tmp = *addr++;
			delay(tmp); // up to 255 millis
		} else {
			tmp = *addr++;
			writeCommand(tmp); // first byte is command
			while (--numBytes) { //   For each argument...
				tmp = *addr++;
				writeData(tmp); // all consecutive bytes are data
			}
		}
	}
}

// 	_tftDC _tftReset _tftCS	_tftBLK

void Adafruit_ILI9486_ESP32::CD_DATA(void) {
	gpio_set_level((gpio_num_t)_tftDC, 1);
}

void Adafruit_ILI9486_ESP32::CD_COMMAND(void) {
	gpio_set_level((gpio_num_t)_tftDC, 0);
}

void Adafruit_ILI9486_ESP32::CS_ON(void) {
	if(_tftCS > 0) digitalWrite(_tftCS, LOW);
}

void Adafruit_ILI9486_ESP32::CS_OFF(void) {
	if(_tftCS > 0) digitalWrite(_tftCS, HIGH);
}


/*****************************************************************************/
void Adafruit_ILI9486_ESP32:: writeData(uint8_t c)
{
	CD_DATA();
	CS_ON();
    SPI.transfer(c);
	CS_OFF();
}

void Adafruit_ILI9486_ESP32::writeData16(uint16_t c)
{
	CD_DATA();
	CS_ON();
	SPI.transfer(c>>8);
	SPI.transfer(c&0xFF);
	CS_OFF();
}

void Adafruit_ILI9486_ESP32::writeData16(uint16_t color, uint32_t num)
{
	CD_DATA();
	CS_ON();
	for (int n=0;n<num;n++) {
		SPI.transfer(color>>8);
		SPI.transfer(color&0xFF);
	}
	CS_OFF();
}

void Adafruit_ILI9486_ESP32::writeCommand(uint8_t c)
{
	CD_COMMAND();
	CS_ON();
    SPI.transfer(c);
	CS_OFF();
}
