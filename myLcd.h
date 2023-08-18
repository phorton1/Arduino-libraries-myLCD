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

#define WITH_ILI9431_FONTS   1
	// set to 1 to use Paul Stoffregens's ILI9431_t3 fonts

#if ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#ifdef __AVR__
	#include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(__MK66FX1M0__)	// prh - eliminate teensy warnings
	#include <pgmspace.h>
#else
	#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
	#define pgm_read_word(addr) (*(const unsigned short *)(addr))
#endif

// #if !defined(AVR)
// 	   #include <avr/dtostrf.h>
// #endif

#if WITH_ILI9431_FONTS
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

 	virtual int16_t  Get_Width(void) const=0;
	virtual int16_t  Get_Height(void) const=0;
	virtual void 	 Fill_Rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)=0;

	// following in myLcd.cpp

	void 	 Set_Text_Size(uint8_t s);
	void     Set_Text_Cursor(int16_t x, int16_t y);
	void 	 Set_Text_colour(uint16_t color);
	void 	 Set_Text_Back_colour(uint16_t color);
	void 	 Set_Draw_color(uint16_t color);

	void 	 Fill_Screen(uint16_t color);
	void 	 Fill_Circle(int16_t x, int16_t y, int16_t radius);
	void 	 Draw_Circle(int16_t x, int16_t y, int16_t radius);
	void 	 Draw_Line(int16_t x1, int16_t y1, int16_t x2, int16_t y2);

	size_t 	 Print_String(const uint8_t *st, int16_t x, int16_t y);

	#if WITH_ILI9431_FONTS
		void setFont(const ILI9341_t3_font_t &f) { font = &f; }
		void setDefaultFont(void) { font = NULL; }
	#endif

	// in myLcdAdditions.cpp

	void 	 drawBorder(int x, int y, int w, int h, int b, int color);

	void printf_justified(
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
	void printfv_justified(
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
	void print_justified(
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

	virtual void 	 Set_Addr_Window(int16_t x1, int16_t y1, int16_t x2, int16_t y2)=0;
	virtual void 	 Push_Any_Color(uint16_t * block, int16_t n, bool first, uint8_t flags)=0;
	virtual int16_t  Read_GRAM(int16_t x, int16_t y, uint16_t *block, int16_t w, int16_t h)=0;
	virtual void 	 Draw_Pixel(int16_t x, int16_t y, uint16_t color)=0;
	virtual uint16_t Color_To_565(uint8_t r, uint8_t g, uint8_t b)=0;

private:	// implementation

	int16_t  text_x;
	int16_t  text_y;
	uint16_t text_color;
	uint16_t text_bgcolor;
	uint16_t draw_color;
	uint8_t  text_size;
	boolean  m_use_bc;		// use bgcolor when writing chars

	// in myLcdAdditions.cpp

	int getFontHeight();
	int getCharWidth(unsigned int c);
	int getTextExtent(const char *text);

	#if WITH_ILI9431_FONTS	// prh addition
		const ILI9341_t3_font_t *font;
		void drawFontChar(unsigned int c);
		void drawFontBits(uint32_t bits, uint32_t numbits, uint32_t x, uint32_t y, uint32_t repeat);
	#endif

	// in myLcd.cpp

	void 	 Draw_Fast_VLine(int16_t x, int16_t y, int16_t h);
	void 	 Fill_Circle_Helper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername,int16_t delta);
    void 	 Draw_Char(int16_t x, int16_t y, uint8_t c, uint16_t color,uint16_t bg, uint8_t size, boolean mode);

	virtual size_t write(uint8_t c) override;
		// implements base Print class method


private:  // in myLcdUnused.cpp which can be ifdef'd out in it's entirety


	uint8_t  Get_Text_Size(void) const;
	int16_t  Get_Text_X_Cursor(void) const;
	int16_t  Get_Text_Y_Cursor(void) const;
	void 	 Set_Text_colour(uint8_t r, uint8_t g, uint8_t b);
	uint16_t Get_Text_colour(void) const;
	void 	 Set_Text_Back_colour(uint8_t r, uint8_t g, uint8_t b);
	uint16_t Get_Text_Back_colour(void) const;
	void 	 Set_Draw_color(uint8_t r, uint8_t g, uint8_t b);
	uint16_t Get_Draw_color(void) const;

	uint16_t Read_Pixel(int16_t x, int16_t y);
	void 	 Fill_Screen(uint8_t r, uint8_t g, uint8_t b);
	void 	 Draw_Rectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
	void 	 Fill_Rectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
	void 	 Draw_Round_Rectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t radius);
	void 	 Fill_Round_Rectangle(int16_t x1, int16_t y1, int16_t x2,int16_t y2, int16_t radius);
	void 	 Draw_Circle_Helper(int16_t x0, int16_t y0, int16_t radius, uint8_t cornername);
	void 	 Draw_Triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,int16_t x2, int16_t y2);
	void 	 Fill_Triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,int16_t x2, int16_t y2);
	void 	 Draw_Bit_Map(int16_t x, int16_t y, int16_t sx, int16_t sy, const uint16_t *data, int16_t scale);
	void 	 Draw_Fast_HLine(int16_t x, int16_t y, int16_t w);

	void 	 Print_String(String st, int16_t x, int16_t y);
	void 	 Print_Number_Int(long num, int16_t x, int16_t y, int16_t length, uint8_t filler, int16_t system);
	void 	 Print_Number_Float(double num, uint8_t dec, int16_t x, int16_t y, uint8_t divider, int16_t length, uint8_t filler);

	void 	 setTextBackgroundOn(boolean on) 	{ m_use_bc = on; }
	boolean  getTextBackgroundOn() 				{ return m_use_bc; }

};	// class myLcd


// end of myLcd.h
