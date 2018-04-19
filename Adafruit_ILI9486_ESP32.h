/*
See rights and use declaration in License.h
This library has been modified for the Maple Mini

-----------
modified by IOXhop (www.ioxhop.com)
 - Add micro MISO MOSI and SCK pin
-----------
modified by Yoann Darche
 - Add the updated to support SPI-4wire mode / 18bit Color mode
*/

#ifndef _ADAFRUIT_ILI9486H_
#define _ADAFRUIT_ILI9486H_

//#include "Arduino.h"
//#include "Print.h"
#include <Adafruit_GFX.h>
#include <SPI.h>

#define TFTWIDTH	320
#define TFTHEIGHT	480

#define ILI9486_INVOFF	0x20
#define ILI9486_INVON	0x21
#define ILI9486_CASET	0x2A
#define ILI9486_PASET	0x2B
#define ILI9486_RAMWR	0x2C
#define ILI9486_MADCTL	0x36
#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

// Color definitions
#define BLACK       0x0000      /*   0,   0,   0 */
#define NAVY        0x000F      /*   0,   0, 128 */
#define DARKGREEN   0x03E0      /*   0, 128,   0 */
#define DARKCYAN    0x03EF      /*   0, 128, 128 */
#define MAROON      0x7800      /* 128,   0,   0 */
#define PURPLE      0x780F      /* 128,   0, 128 */
#define OLIVE       0x7BE0      /* 128, 128,   0 */
#define LIGHTGREY   0xC618      /* 192, 192, 192 */
#define DARKGREY    0x7BEF      /* 128, 128, 128 */
#define BLUE        0x001F      /*   0,   0, 255 */
#define GREEN       0x07E0      /*   0, 255,   0 */
#define CYAN        0x07FF      /*   0, 255, 255 */
#define RED         0xF800      /* 255,   0,   0 */
#define MAGENTA     0xF81F      /* 255,   0, 255 */
#define YELLOW      0xFFE0      /* 255, 255,   0 */
#define WHITE       0xFFFF      /* 255, 255, 255 */
#define ORANGE      0xFD20      /* 255, 165,   0 */
#define GREENYELLOW 0xAFE5      /* 173, 255,  47 */
#define PINK        0xF81F

/*
	Define pins and Output Data Registers
*/

// SPI Speed :
#define TFT_SPI_SPEED  60000000

#define TFT_SCK  14
#define TFT_MISO 12
#define TFT_MOSI 13

//Control pins
#define TFT_RST     26
#define TFT_CS   	15
#define TFT_DC      33   // Data/Command
#define TFT_BLK     18   // Background led control


/*
#define CD_COMMAND   gpio_set_level((gpio_num_t)TFT_RS, 0);
#define CD_DATA      gpio_set_level((gpio_num_t)TFT_RS, 1);

#define CS_ACTIVE    GPIO.out_w1tc = ((uint32_t)1 << TFT_CS);
#define CS_IDLE      GPIO.out_w1ts = ((uint32_t)1 << TFT_CS);
*/
#define swap(a, b) { int16_t t = a; a = b; b = t; }

/*****************************************************************************/
class Adafruit_ILI9486_ESP32 : public Adafruit_GFX
{
public:

	Adafruit_ILI9486_ESP32(void);

	void	begin(void);

	// tftDC    : TFT ILI9486 Data/Command pin
	// tftReset : TFT ILI9486 rest pin
	// tftCS    : TFT ILI9486 Chip select active to Low, some tft has no CS, leave it to 0
	// tftBLK   : Is the pin used to activate or desactivate the Back led of the TFT, if none let it to 0
	// spiMISO  : is actually optional (not implemented yet)
	void	begin(uint8_t spiClk, uint8_t spiMOSI, uint8_t tftDC, uint8_t tftCS,
										  uint8_t spiMISO, uint8_t tftReset, uint8_t tftBLK);

	void	setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

	void	pushColor(uint16_t color);
	void  pushColorN(uint16_t color, uint32_t n);
	
	void	drawPixel(int16_t x, int16_t y, uint16_t color);

	void	fillScreen(uint16_t color);
	void	drawLine(int16_t x0, int16_t y0,int16_t x1, int16_t y1, uint16_t color);

	void	drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
	void	drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
	void	fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

	void	setRotation(uint8_t r);
	void	invertDisplay(boolean i);


	uint16_t color565(uint8_t r, uint8_t g, uint8_t b);

	void reset(void);

protected:

	uint8_t _spiClk;
	uint8_t _spiMISO;
	uint8_t _spiMOSI;
	uint8_t _tftReset;
	uint8_t _tftDC;
	uint8_t _tftCS;
	uint8_t _tftBLK;
	uint8_t	tabcolor;

	void writeCommand(uint8_t c);
	void writeData(uint8_t d);
	void writeData16(uint16_t d);
	void writeData16(uint16_t d, uint32_t num);
	void commandList(const uint8_t *addr);

	void CD_DATA(void);
	void CD_COMMAND(void);
	void CS_ON(void);
	void CS_OFF(void);
};



#endif //endif of the header file
