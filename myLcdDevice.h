//----------------------------------------
// myLcdDevice.h
//----------------------------------------
// formerly LCDWIKI_KBD
// pubic API limited to minimum necessary
//
// This class is derived from myLcd and provides
// the device specific implementation of low level
// methods.
//
// Initially implemented to support 8 or 16 bit parallel
// via #defines, I would REALLY like to change this into
// a RUNTIME object that separates the PORT from the DEVICE,
// starting with using SPI for the ILI9488 as stolen from
// teensy ILI9488_t3 library.
//
// The higher level drawing routines should be completely
// abstracted on top of the DEVICE layer.  The DEVICE layer
// should be abstracted on top of the PORT layer.


#pragma once


#include "myLcd.h"
#include <SPI.h>


// LCD controller chip identifiers

#define ID_932X    0
#define ID_7575    1
#define ID_9341    2
#define ID_HX8357D 3
#define ID_4535    4
#define ID_9486    5
#define ID_7735    6
#define ID_9488    7
#define ID_9481    8
#define ID_UNKNOWN 0xFF

//LCD controller chip mode identifiers

#define ILI9325 0
#define ILI9328 1
#define ILI9341 2
#define HX8357D 3
#define HX8347G 4
#define HX8347I 5
#define ILI9486 6
#define ST7735S 7
#define ILI9488 8
#define ILI9481 9

//if using the lcd breakout board,comment out this next line.
//if using the lcd shield,leave the line enable:

// #define USE_ADAFRUIT_SHIELD_PIN 1


extern void setTFTDataPins(int p0,int p1,int p2,int p3,int p4,int p5,int p6,int p7);
	// prh addition is really teensy only


typedef struct _lcd_info
{
	uint16_t lcd_id;
	int16_t lcd_wid;
	int16_t lcd_heg;
} lcd_info;



class myLcdDevice : public myLcd
{
public:

	myLcdDevice(SPIClass *spi_ptr, uint8_t _CS, uint8_t _DC, uint8_t _RST = 255, uint8_t _MOSI=255, uint8_t _SCLK=255, uint8_t _MISO=255);
	bool isSPIDevice()  { return spi ? 1 : 0; }

	myLcdDevice(uint16_t model,uint8_t cs, uint8_t cd, uint8_t wr, uint8_t rd, uint8_t reset);

	void begin(void);

	void setRotation(uint8_t r);

	int16_t width(void) const    { return _width; }
	int16_t height(void) const   { return _height; }
	uint8_t getRotation(void) const { return _rotation; }
		// get current _rotation
		// 0  :  0 degree
		// 1  :  90 degree
		// 2  :  180 degree
		// 3  :  270 degree


	virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;

private:	// available to base class via virtual calls

	virtual uint16_t color565(uint8_t r, uint8_t g, uint8_t b) override;
	virtual void 	 drawPixel(int16_t x, int16_t y, uint16_t color) override;
	virtual void 	 setAddrWindow(int16_t x1, int16_t y1, int16_t x2, int16_t y2) override;
	virtual void 	 pushAnyColor(uint16_t * block, int16_t n, bool first, uint8_t flags) override;
	virtual int16_t  readRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *block) override;


private:  // was public

	myLcdDevice(int16_t wid,int16_t heg,uint8_t cs, uint8_t cd, uint8_t wr, uint8_t rd, uint8_t reset);

	void reset(void);
	void start(uint16_t ID);
	void init_table8(const void *table, int16_t size);
	void init_table16(const void *table, int16_t size);

	void setLR(void);
	void invertDisplay(boolean i);
	void vertScroll(int16_t top, int16_t scrollines, int16_t offset);	// useless
	void dimScreen(); // prh addition = set all pixels to half their value

	void Push_Command(uint16_t cmd, uint8_t *block, int8_t N);
	void pushAnyColor(uint8_t * block, int16_t n, bool first, uint8_t flags);

	uint16_t readID(void);
	uint16_t readReg(uint16_t reg, int8_t index);


private:	// was protected

    uint16_t WIDTH;
	uint16_t HEIGHT;
	uint16_t _width;
	uint16_t _height;
	uint16_t _rotation;
	uint16_t lcd_driver;
	uint16_t lcd_model;

private:

	SPIClass *spi;

	uint16_t XC;
	uint16_t YC;
	uint16_t CC;
	uint16_t RC;
	uint16_t SC1;
	uint16_t SC2;
	uint16_t MD;
	uint16_t VL;
	uint16_t R24BIT;

	#if __LCD_TEENSY__
		// prh mod for teensy
		uint8_t _reset;
		uint8_t _cs;
		uint8_t _cd;
		uint8_t _wr;
		uint8_t _rd;
	#else

		#ifndef USE_ADAFRUIT_SHIELD_PIN

			#ifdef __AVR__
				volatile uint8_t *csPort;
				volatile uint8_t *cdPort;
				volatile uint8_t *wrPort;
				volatile uint8_t *rdPort;

				uint8_t csPinSet;
				uint8_t cdPinSet;
				uint8_t wrPinSet;
				uint8_t rdPinSet;
				uint8_t csPinUnset;
				uint8_t cdPinUnset;
				uint8_t wrPinUnset;
				uint8_t rdPinUnset;
				uint8_t _reset;
			#endif

			#if defined(__SAM3X8E__)
				Pio *csPort;
				Pio *cdPort;
				Pio *wrPort;
				Pio *rdPort;

				uint32_t csPinSet;
				uint32_t cdPinSet;
				uint32_t wrPinSet;
				uint32_t rdPinSet;
				uint32_t csPinUnset;
				uint32_t cdPinUnset;
				uint32_t wrPinUnset;
				uint32_t rdPinUnset;
				uint32_t _reset;
			#endif

		#endif

	#endif	// !__LCD_TEENSY__
};

// end of myLcdDevice.h
