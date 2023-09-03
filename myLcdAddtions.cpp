//-----------------------------------------------------
// myLcdAdditions.cpp
//------------------------------------------------------
// Additional myLcd methods

#include "myLcd.h"
#include <myDebug.h>

#define dbg_la		1

#define DEBUG_DRAW_FONT_CHAR   0

#define MAX_PRINT_LEN  1023
	// 1k buffer on the stack!!
	// Note that var-arg formats can only be half of this ?!?

#ifndef __LCD_TEENSY__
	// prh - already defined in Paul's stuff
	#define swap(a, b) { int16_t t = a; a = b; b = t; }
#endif


//------------------------------------------------
// public API
//------------------------------------------------

void myLcd::drawBorder(int x, int y, int w, int h, int b, int color)
	// draw a frigin border
{
	fillRect(x,		y,		b,		h,	color);
	fillRect(x+w-b,	y,		b,		h,	color);
	fillRect(x,		y,		w,		b,	color);
	fillRect(x,		y+h-b,	w,		b,	color);
}


void myLcd::printfJustified(
	int x,
	int y,
	int w,
	int h,
	int just,
	uint16_t fc,
	uint16_t bc,
	bool use_bc,
	const char *format,
	...)
{
	va_list args;
	va_start(args, format);
	printfvJustified(x,y,w,h,just,fc,bc,use_bc,format,args);
}


void myLcd::printfvJustified(
	int x,
	int y,
	int w,
	int h,
	int just,
	uint16_t fc,
	uint16_t bc,
	bool use_bc,
	const char *format,
	va_list args)
{
	char display_buffer[MAX_PRINT_LEN+1];
	if (strlen(format) >= MAX_PRINT_LEN/2)
	{
		my_error("error - MAX_PRINT_LEN overflow",0);
		return;
	}
	vsprintf(display_buffer,format,args);
	printJustified(x,y,w,h,just,fc,bc,use_bc,display_buffer);
}



void myLcd::printJustified(
	int x,
	int start_y,
	int w,
	int h,
	int just,
	uint16_t fc,
	uint16_t bc,
	bool use_bc,
	const char *text)
{
	// prints \n delimited lines

	char cut_buf[MAX_PRINT_LEN+1];
		// another 1K buffer on the stack, just so that we can
		// we can break the passed in string into lines at \n's
		// for calling drawString ...


	display(dbg_la,"printJustified(%d,%d,%d,%d,  %d, 0x%04x, 0x%04x, %d, \"%s\")", x, start_y, w, h, just, fc, bc, use_bc, text);

	setTextColor(fc);
	if (use_bc)
		fillRect(x,start_y,w,h,bc);

	int y = start_y + 1;
	int yoffset = getFontHeight();

	// We clip the string in the X direction by pushing characters to the next line.
	// Until such a time as we implement actual clipping in drawChar/Strings,
	// we currently print any characters whose TOPS are IN the rectangle, without regard
	// if they overflow the rectangle at the bottom.

	while (*text && y < start_y + h - 1) // while the top of the next line is in the rectangle
	{
		int len = 0;
		int pixel_len = 0;

		while (*text)
		{
			char c = *text;
			int pix = getCharWidth(c);

			// if cr, or the whole character won't fit,
			// or the unlikely case they tried to print more than 1025 characters,
			// push the next character to the next line

			if (c == '\n')
			{
				text++;
				break;
			}
			else if (pixel_len + pix > w ||
					len >= MAX_PRINT_LEN)
			{
				break;
			}

			text++;
			pixel_len += pix;
			cut_buf[len++] = c;

		}	// inner while *text

		cut_buf[len++] = 0;

		int use_x = x;
		if (just != LCD_JUST_LEFT)
		{
			int xoffset = (w - pixel_len);
			if (xoffset < 0) xoffset = 0;
			if (just == LCD_JUST_CENTER)
				xoffset /= 2;
			use_x += xoffset;
		}

		drawString(cut_buf,use_x,y);
		y += yoffset;

	}	// outer while *text
}




//------------------------------------------------------
// ILI9431_FONTS protected API
//------------------------------------------------------

#if __LCD_TEENSY__

	static uint32_t fetchbit(const uint8_t *p, uint32_t index)
	{
		if (p[index >> 3] & (1 << (7 - (index & 7)))) return 1;
		return 0;
	}

	static uint32_t fetchbits_unsigned(const uint8_t *p, uint32_t index, uint32_t required)
	{
		uint32_t val = 0;
		do {
			uint8_t b = p[index >> 3];
			uint32_t avail = 8 - (index & 7);
			if (avail <= required) {
				val <<= avail;
				val |= b & ((1 << avail) - 1);
				index += avail;
				required -= avail;
			} else {
				b >>= avail - required;
				val <<= required;
				val |= b & ((1 << required) - 1);
				break;
			}
		} while (required);
		return val;
	}

	static uint32_t fetchbits_signed(const uint8_t *p, uint32_t index, uint32_t required)
	{
		uint32_t val = fetchbits_unsigned(p, index, required);
		if (val & (1 << (required - 1))) {
			return (int32_t)val - (1 << required);
		}
		return (int32_t)val;
	}


	void myLcd::drawFontChar(unsigned int c)
	{
		uint32_t bitoffset;
		const uint8_t *data;

		display(dbg_la+1,"drawFontChar %d='%c' at x=%d y=%d", c, (c>32?c:32), _text_x, _text_y);

		if (c >= _font->index1_first && c <= _font->index1_last)
		{
			bitoffset = c - _font->index1_first;
			bitoffset *= _font->bits_index;
		}
		else if (c >= _font->index2_first && c <= _font->index2_last)
		{
			bitoffset = c - _font->index2_first + _font->index1_last - _font->index1_first + 1;
			bitoffset *= _font->bits_index;
		}
		else if (_font->unicode)
		{
			return; // TODO: implement sparse unicode
		}
		else
		{
			return;
		}

		#if DEBUG_DRAW_FONT_CHAR
			Serial.printf("  index =  %d\n", fetchbits_unsigned(_font->index, bitoffset, _font->bits_index));
		#endif

		data = _font->data + fetchbits_unsigned(_font->index, bitoffset, _font->bits_index);
		uint32_t encoding = fetchbits_unsigned(data, 0, 3);
		if (encoding != 0) return;

		uint32_t _width = fetchbits_unsigned(data, 3, _font->bits_width);
		bitoffset = _font->bits_width + 3;
		uint32_t _height = fetchbits_unsigned(data, bitoffset, _font->bits_height);
		bitoffset += _font->bits_height;

		#if DEBUG_DRAW_FONT_CHAR
			Serial.printf("  size =   %d,%d\n", _width, _height);
		#endif

		int32_t xoffset = fetchbits_signed(data, bitoffset, _font->bits_xoffset);
		bitoffset += _font->bits_xoffset;
		int32_t yoffset = fetchbits_signed(data, bitoffset, _font->bits_yoffset);
		bitoffset += _font->bits_yoffset;
		#if DEBUG_DRAW_FONT_CHAR
			Serial.printf("  offset = %d,%d\n", xoffset, yoffset);
		#endif

		uint32_t delta = fetchbits_unsigned(data, bitoffset, _font->bits_delta);
		bitoffset += _font->bits_delta;

		#if DEBUG_DRAW_FONT_CHAR
			Serial.printf("  delta =  %d\n", delta);
			// Serial.printf("  cursor = %d,%d\n", cursor_x, cursor_y);
		#endif

		// horizontally, we draw every pixel, or none at all

		if (_text_x < 0) _text_x = 0;
		int32_t origin_x = _text_x + xoffset;
		if (origin_x < 0)
		{
			_text_x -= xoffset;
			origin_x = 0;
		}
		if (origin_x + (int)_width > width())
		{
			// if (!wrap) return;  prh - wrap

			origin_x = 0;
			if (xoffset >= 0) {
				_text_x = 0;
			} else {
				_text_x = -xoffset;
			}
			_text_y += _font->line_space;
		}
		if (_text_y >= height()) return;
		_text_x += delta;

		// vertically, the top and/or bottom can be clipped
		int32_t origin_y = _text_y + _font->cap_height - _height - yoffset;
		#if DEBUG_DRAW_FONT_CHAR
			Serial.printf("  origin = %d,%d\n", origin_x, origin_y);
		#endif

		// TODO: compute top skip and number of lines
		int32_t linecount = _height;
		//uint32_t loopcount = 0;
		uint32_t y = origin_y;
		while (linecount)
		{
			#if DEBUG_DRAW_FONT_CHAR
				Serial.printf("    linecount = %d\n", linecount);
			#endif

			uint32_t b = fetchbit(data, bitoffset++);
			if (b == 0)
			{
				#if DEBUG_DRAW_FONT_CHAR
					Serial.println("    single line");
				#endif

				uint32_t x = 0;
				do
				{
					uint32_t xsize = _width - x;
					if (xsize > 32) xsize = 32;
					uint32_t bits = fetchbits_unsigned(data, bitoffset, xsize);
					drawFontBits(bits, xsize, origin_x + x, y, 1);
					bitoffset += xsize;
					x += xsize;
				} while (x < _width);
				y++;
				linecount--;
			}
			else
			{
				uint32_t n = fetchbits_unsigned(data, bitoffset, 3) + 2;
				bitoffset += 3;
				uint32_t x = 0;
				do
				{
					uint32_t xsize = _width - x;
					if (xsize > 32) xsize = 32;
					#if DEBUG_DRAW_FONT_CHAR
						Serial.printf("    multi line %d\n", n);
					#endif
					uint32_t bits = fetchbits_unsigned(data, bitoffset, xsize);
					drawFontBits(bits, xsize, origin_x + x, y, n);
					bitoffset += xsize;
					x += xsize;
				} while (x < _width);
				y += n;
				linecount -= n;
			}
			//if (++loopcount > 100) {
				//Serial.println("     abort draw loop");
				//break;
			//}
		}
	}

	void myLcd::drawFontBits(uint32_t bits, uint32_t numbits, uint32_t x, uint32_t y, uint32_t repeat)
	{
		if (bits == 0)
			return;

		while (repeat--)
		{
			uint32_t x1 = x;
			uint32_t n = numbits;
			while (n--)
			{
				if (bits & (1 << n))
				{
					drawPixel(x1, y, _text_color);
					#if DEBUG_DRAW_FONT_CHAR > 1
						Serial.printf("        pixel at %d,%d\n", x1, y);
					#endif
				}
				x1++;
			}
			y++;
		}
	}

#endif	// __LCD_TEENSY__




//---------------------------------------------------
// protected API
//---------------------------------------------------

int myLcd::getFontHeight()
{
	#if __LCD_TEENSY__
		if (_font)
			return _font->line_space;
	#endif

	return _text_size * 8;
}


int myLcd::getCharWidth(unsigned int c)
{
	#if __LCD_TEENSY__
		if (_font)
		{
			uint32_t bitoffset;
			if (c >= _font->index1_first && c <= _font->index1_last)
			{
				bitoffset = c - _font->index1_first;
				bitoffset *= _font->bits_index;
			}
			else if (c >= _font->index2_first && c <= _font->index2_last)
			{
				bitoffset = c - _font->index2_first + _font->index1_last - _font->index1_first + 1;
				bitoffset *= _font->bits_index;
			}
			else
			{
				warning(0,"WARNING: chr(%d)=%c cannot be mapped",c,c);
				return 0;
			}

			const uint8_t *data = _font->data + fetchbits_unsigned(_font->index, bitoffset, _font->bits_index);
			uint32_t encoding = fetchbits_unsigned(data, 0, 3);
			if (encoding != 0)
			{
				warning(0,"WARNING: chr(%d)=%c bad encoding(%d)",c,c,encoding);
				return 0;
			}

			bitoffset = _font->bits_width + 3;
			bitoffset += _font->bits_height;
			bitoffset += _font->bits_xoffset;
			bitoffset += _font->bits_yoffset;

			uint32_t delta = fetchbits_unsigned(data, bitoffset, _font->bits_delta);
			return (int) delta;
		}
	#endif

	return _text_size * 6;
}


int myLcd::getTextExtent(const char *text)
{
	int len = strlen(text);

	#if __LCD_TEENSY__
		if (_font)
		{
			int strlen = 0;
			for (int i=0; i<len; i++)
				strlen += getCharWidth(text[i]);
			return strlen;
		}
	#endif

	return len * _text_size * 6;
}
