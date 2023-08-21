//-------------------------------------------
// myLcd.h
//-------------------------------------------
// formerly LCDWIKI_GUI
// I have limited this API to calls that I actually use
//
// Usage: This is the main user level API
// 		This is the base class, that knows how to draw things.
// 		You instanstiate a subclass myLcdDevice which is knows how to draw things.
// 		This class call that one through virtual methods.
// Note also that since this is derived from Print, you
//      can use methods like println(), printf(), and so on,
//      which all go through the overriden write() method.

#pragma once

#if defined(__MK66FX1M0__) || defined(__IMXRT1062__)
	// teensy 3.6 || teemsy 4.*
	#define __LCD_TEENSY__   1
#else
	#define __LCD_TEENSY__   0
#endif


#if ARDUINO >= 100  || __LCD_TEENSY__
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#ifdef __AVR__
	#include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(__LCD_TEENSY__)
	#include <pgmspace.h>
#else
	#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
	#define pgm_read_word(addr) (*(const unsigned short *)(addr))
#endif


#if __LCD_TEENSY__
	#include <ILI9341_t3.h>
#endif


#define LCD_JUST_LEFT    0
#define LCD_JUST_CENTER  1
#define LCD_JUST_RIGHT   2



class myLcd : public Print
{
public:

	myLcd(void);

	// following in Base Class

 	virtual int16_t  width(void) const=0;
	virtual int16_t  height(void) const=0;
	virtual void 	 fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)=0;

	// following in myLcd.cpp

	void 	 setTextSize(uint8_t s);
	void     setCursor(int16_t x, int16_t y);
	void 	 setTextColor(uint16_t color);
	void 	 setTextBackColor(uint16_t color);
	void 	 setDrawColor(uint16_t color);

	void 	 fillScreen(uint16_t color);
	void 	 fillCircle(int16_t x, int16_t y, int16_t radius);
	void 	 drawCircle(int16_t x, int16_t y, int16_t radius);
	void 	 drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2);

	size_t 	 drawString(const uint8_t *st, int16_t x, int16_t y);

	#if __LCD_TEENSY__
		void setFont(const ILI9341_t3_font_t &font) { _font = &font; }
		void setDefaultFont(void) { _font = NULL; }
	#endif

	// in myLcdAdditions.cpp

	void 	 drawBorder(int x, int y, int w, int h, int b, int color);

	void printfJustified(
		int x,
		int y,
		int w,
		int h,
		int just,
		uint16_t fc,
		uint16_t bc,
		bool use_bc,
		const char *format,
		...);
	void printfvJustified(
		int x,
		int y,
		int w,
		int h,
		int just,
		uint16_t fc,
		uint16_t bc,
		bool use_bc,
		const char *format,
		va_list args);
	void printJustified(
		int x,
		int y,
		int w,
		int h,
		int just,
		uint16_t fc,
		uint16_t bc,
		bool use_bc,
		const char *text);



protected:	// defined by subclass

	virtual void 	 setAddrWindow(int16_t x1, int16_t y1, int16_t x2, int16_t y2)=0;
	virtual void 	 pushAnyColor(uint16_t * block, int16_t n, bool first, uint8_t flags)=0;
	virtual int16_t  readRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *block) =0;
	virtual void 	 drawPixel(int16_t x, int16_t y, uint16_t color)=0;
	virtual uint16_t color565(uint8_t r, uint8_t g, uint8_t b)=0;

private:	// implementation

	int16_t  _text_x;
	int16_t  _text_y;
	uint16_t _text_color;
	uint16_t _text_bgcolor;
	uint16_t _draw_color;
	uint8_t  _text_size;
	boolean  _use_bc;		// use bgcolor when writing chars

	// in myLcdAdditions.cpp

	int getFontHeight();
	int getCharWidth(unsigned int c);
	int getTextExtent(const char *text);

	#if __LCD_TEENSY__
		const ILI9341_t3_font_t *_font;
		void drawFontChar(unsigned int c);
		void drawFontBits(uint32_t bits, uint32_t numbits, uint32_t x, uint32_t y, uint32_t repeat);
	#endif

	// in myLcd.cpp

	void 	 drawFastVLine(int16_t x, int16_t y, int16_t h);
	void 	 fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername,int16_t delta);
    void 	 drawChar(int16_t x, int16_t y, uint8_t c, uint16_t color, uint16_t bg, uint8_t size, boolean mode);

	virtual size_t write(uint8_t c) override;
		// implements base Print class method


private:  // in myLcdUnused.cpp which can be ifdef'd out in it's entirety


	uint8_t  getTextSize(void) const;
	int16_t  getCursorX(void) const;
	int16_t  getCursorY(void) const;
	void 	 setTextColor(uint8_t r, uint8_t g, uint8_t b);
	uint16_t getTextColor(void) const;
	void 	 setTextBackColor(uint8_t r, uint8_t g, uint8_t b);
	uint16_t getTextBackColor(void) const;
	void 	 setDrawColor(uint8_t r, uint8_t g, uint8_t b);
	uint16_t getDrawColor(void) const;

	uint16_t readPixel(int16_t x, int16_t y);
	void 	 fillScreen(uint8_t r, uint8_t g, uint8_t b);
	void 	 drawRect(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
	void 	 fillRect(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
	void 	 drawRoundRect(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t radius);
	void 	 fillRoundRect(int16_t x1, int16_t y1, int16_t x2,int16_t y2, int16_t radius);
	void 	 drawCircleHelper(int16_t x0, int16_t y0, int16_t radius, uint8_t cornername);
	void 	 drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,int16_t x2, int16_t y2);
	void 	 fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,int16_t x2, int16_t y2);
	void 	 drawBitmap(int16_t x, int16_t y, int16_t sx, int16_t sy, const uint16_t *data, int16_t scale);
	void 	 drawFastHLine(int16_t x, int16_t y, int16_t w);

	void 	 drawString(String st, int16_t x, int16_t y);
	void 	 drawNumber(long num, int16_t x, int16_t y, int16_t length, uint8_t filler, int16_t system);
	void 	 drawFloat(double num, uint8_t dec, int16_t x, int16_t y, uint8_t divider, int16_t length, uint8_t filler);

	void 	 setTextBackgroundOn(boolean on) 	{ _use_bc = on; }
	boolean  getTextBackgroundOn() 				{ return _use_bc; }

};	// class myLcd


// end of myLcd.h
