//-----------------------------------------------
// myLcd.cpp
//-----------------------------------------------

#include "myLcd.h"
#include "myLcdFont.h"


#define swap(a, b) { int16_t t = a; a = b; b = t; }


myLcd::myLcd(void)
{
	_text_bgcolor = 0xF800; //default red
	_text_color = 0x07E0; //default green
	_draw_color = 0xF800; //default red
	_text_size = 1;

	_use_bc = true;

	#if __LCD_TEENSY__
		_font = 0;
	#endif
}

//----------------------------------------
// public API
//----------------------------------------

void myLcd::setTextSize(uint8_t s)
{
	_text_size = s;
}

void myLcd::setCursor(int16_t x, int16_t y)
{
	_text_x = x;
	_text_y = y;
}

void myLcd::getCursor(int16_t *x, int16_t *y)
{
	*x = _text_x;
	*y = _text_y;
}



void myLcd::setTextColor(uint16_t color)
{
	_text_color = color;
}

void myLcd::setTextBackColor(uint16_t color)
{
	_text_bgcolor = color;
}

void myLcd::setDrawColor(uint16_t color)
{
	_draw_color = color;
}

void myLcd::fillScreen(uint16_t color)
{
	fillRect(0, 0, width(), height(), color);
}


void myLcd::fillCircle(int16_t x, int16_t y, int16_t radius)
{
	drawFastVLine(x, y-radius, 2*radius+1);
	fillCircleHelper(x, y, radius, 3, 0);
}


void myLcd::drawCircle(int16_t x, int16_t y, int16_t radius)
{
	int16_t f = 1 - radius;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * radius;
	int16_t x1= 0;
	int16_t y1= radius;

	drawPixel(x, y+radius, _draw_color);
 	drawPixel(x, y-radius, _draw_color);
	drawPixel(x+radius, y, _draw_color);
	drawPixel(x-radius, y, _draw_color);

	while (x1<y1)
	{
    	if (f >= 0)
		{
      		y1--;
      		ddF_y += 2;
      		f += ddF_y;
    	}
    	x1++;
    	ddF_x += 2;
    	f += ddF_x;

		drawPixel(x + x1, y + y1, _draw_color);
    	drawPixel(x - x1, y + y1, _draw_color);
		drawPixel(x + x1, y - y1, _draw_color);
		drawPixel(x - x1, y - y1, _draw_color);
		drawPixel(x + y1, y + x1, _draw_color);
		drawPixel(x - y1, y + x1, _draw_color);
		drawPixel(x + y1, y - x1, _draw_color);
		drawPixel(x - y1, y - x1, _draw_color);
 	}
}


void myLcd::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
	int16_t steep = abs(y2 - y1) > abs(x2 - x1);
  	if (steep)
	{
    	swap(x1, y1);
    	swap(x2, y2);
	}
	if (x1 > x2)
	{
    	swap(x1, x2);
    	swap(y1, y2);
  	}

  	int16_t dx, dy;
  	dx = x2 - x1;
  	dy = abs(y2 - y1);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y1 < y2)
	{
    	ystep = 1;
  	}
	else
	{
    	ystep = -1;
	}

	for (; x1<=x2; x1++)
	{
    	if (steep)
		{
			// Serial.printf("DrawPixel(%d,%d)\n",x1,y1);
      		drawPixel(y1, x1, _draw_color);
    	}
		else
		{
			// Serial.printf("DrawPixel(%d,%d)\n",x1,y1);
      		drawPixel(x1, y1, _draw_color);
    	}
    	err -= dy;
    	if (err < 0)
		{
			y1 += ystep;
			err += dx;
    	}
		// delay(20);
  	}
}


//draw a rectangle
void myLcd::drawRect(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
	int16_t w = x2 - x1 + 1, h = y2 - y1 + 1;
	if (w < 0)
	{
		x1 = x2;
		w = -w;
	}
	if (h < 0)
	{
		y1 = y2;
		h = -h;
	}
	drawFastHLine(x1, y1, w);
  	drawFastHLine(x1, y2, w);
	drawFastVLine(x1, y1, h);
	drawFastVLine(x2, y1, h);
}



//draw a triangle
void myLcd::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,int16_t x2, int16_t y2)
{
	drawLine(x0, y0, x1, y1);
	drawLine(x1, y1, x2, y2);
	drawLine(x2, y2, x0, y0);
}

//fill a triangle
void myLcd::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,int16_t x2, int16_t y2)
{
	int16_t a, b, y, last;
	if (y0 > y1)
	{
		swap(y0, y1);
		swap(x0, x1);
	}
	if (y1 > y2)
	{
		swap(y2, y1);
		swap(x2, x1);
	}
	if (y0 > y1)
	{
		swap(y0, y1);
		swap(x0, x1);
	}

	if (y0 == y2)
	{
		a = b = x0;
		if (x1 < a)
		{
			a = x1;
		}
		else if (x1 > b)
		{
			b = x1;
		}
		if (x2 < a)
		{
			a = x2;
		}
		else if (x2 > b)
		{
			b = x2;
		}
		drawFastHLine(a, y0, b-a+1);
		return;
	}

	int16_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0, dx12 = x2 - x1, dy12 = y2 - y1;
	int32_t sa = 0, sb = 0;
	if (y1 == y2)
	{
		last = y1;
	}
	else
	{
		last = y1-1;
	}

	for (y=y0; y<=last; y++)
	{
		a   = x0 + sa / dy01;
		b   = x0 + sb / dy02;
		sa += dx01;
		sb += dx02;
		if (a > b)
		{
			swap(a,b);
		}
		drawFastHLine(a, y, b-a+1);
	}

	sa = dx12 * (y - y1);
	sb = dx02 * (y - y0);
	for (; y<=y2; y++)
	{
		a   = x1 + sa / dy12;
		b   = x0 + sb / dy02;
		sa += dx12;
		sb += dx02;
		if (a > b)
		{
			swap(a,b);
		}
		drawFastHLine(a, y, b-a+1);
	}
}




// prh - talk about a weird amateur API
// as well as polluting namespace ..
// these constants are no longer effing public
#define LEFT 0
#define RIGHT 9999
#define CENTER 9998


//print string
size_t myLcd::drawString(const char *st, int16_t x, int16_t y)
{
	int16_t pos;
	uint16_t len;
	const char *p = st;
	size_t n = 0;
	if (x == CENTER || x == RIGHT)
	{
		len = strlen(st) * 6 * _text_size;
		pos = (width() - len);
		if (x == CENTER)
		{
			x = pos/2;
		}
		else
		{
			x = pos - 1;
		}
	}
    setCursor(x, y);
	while (1)
	{
		char ch = *(p++);
		if (ch == 0)
		{
			break;
		}
		if (write(ch))
		{
			n++;
		}
		else
		{
			break;
		}
	}
	return n;
}


//-------------------------------------------------------------
// implementation methods
//-------------------------------------------------------------


void myLcd::drawFastVLine(int16_t x, int16_t y, int16_t h)
{
	fillRect(x, y, 1, h, _draw_color);
}

void myLcd::drawFastHLine(int16_t x, int16_t y, int16_t w)
{
	fillRect(x, y, w, 1, _draw_color);
}

void myLcd::fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername,int16_t delta)
	// fill a semi-circle
{
	int16_t f     = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x     = 0;
	int16_t y     = r;

	while (x<y)
	{
    	if (f >= 0)
		{
      		y--;
      		ddF_y += 2;
      		f += ddF_y;
    	}
    	x++;
    	ddF_x += 2;
    	f += ddF_x;

    	if (cornername & 0x1)
		{
      		drawFastVLine(x0+x, y0-y, 2*y+1+delta);
      		drawFastVLine(x0+y, y0-x, 2*x+1+delta);
    	}
    	if (cornername & 0x2)
		{
      		drawFastVLine(x0-x, y0-y, 2*y+1+delta);
      		drawFastVLine(x0-y, y0-x, 2*x+1+delta);
    	}
  	}
}



// draw a char
// - assumes a 6x8 full box _font
// - _font's are stored as bytes containing columns
// - every 6 bytes is another character
// - size is a simple integer multiplier
// - background only drawn IFF bg is different than color
// - "use_bc" forces background drawing off


void myLcd::drawChar(
	int16_t x,
	int16_t y,
	uint8_t c,
	uint16_t color,
	uint16_t bg,
	uint8_t size,
	boolean use_bc)	// prh changed from "mode"
{
	if ((x >= width()) || (y >= height()) || ((x + 6 * size - 1) < 0) || ((y + 8 * size - 1) < 0))
	{
    	return;
	}
	if (c >= 176) // prh huh?
  	{
		c++;
  	}

	for (int8_t i=0; i<6; i++) 		// for each column
	{
    	uint8_t line;
    	if (i == 5)
    	{
      		line = 0x0;
    	}
    	else
    	{
      		line = pgm_read_byte(lcd_font+(c*5)+i);
    	}

    	for (int8_t j = 0; j<8; j++) 	// for each row
		{
      		if (line & 0x1)
			{
        		if (size == 1)
        		{
        			drawPixel(x+i, y+j, color);
        		}
        		else
				{
					fillRect(x+(i*size), y+(j*size), size, size, color);
        		}
        	}


			// prh and got rid of this weird if
			// else if (bg != color)
			// {
			//	   if (!mode)
			//	   {

			else if (use_bc)
			{
				if (size == 1)
				{
					drawPixel(x+i, y+j, bg);
				}
				else
				{
					fillRect(x+i*size, y+j*size, size, size, bg);
				}
			}

			// }

      		line >>= 1;
    	}
    }
}


//-----------------------------------------------
// write() - implements base Print class
//-----------------------------------------------


size_t myLcd::write(uint8_t c)
{
	#if __LCD_TEENSY__
		if (_font)
		{
			if (c == '\n')
			{
				_text_y += _font->line_space;
				_text_x = 0;
			}
			else if (c == '\r')
			{
			}
			else
			{
				drawFontChar(c);
			}
			return 1;
		}
	#endif

	if (c == '\n')
	{
    	_text_y += _text_size*8;
    	_text_x  = 0;
 	}
	else if (c == '\r')
	{
	}
	else
	{
    	drawChar(_text_x, _text_y, c, _text_color, _text_bgcolor, _text_size,_use_bc);		// prh text_mode);
    	_text_x += _text_size*6;
    }
  	return 1;
}
