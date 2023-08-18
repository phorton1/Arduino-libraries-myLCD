//-----------------------------------------------
// myLcd.cpp
//-----------------------------------------------

#include "myLcd.h"
#include "myLcdFont.c"


#define swap(a, b) { int16_t t = a; a = b; b = t; }


myLcd::myLcd(void)
{
	text_bgcolor = 0xF800; //default red
	text_color = 0x07E0; //default green
	draw_color = 0xF800; //default red
	text_size = 1;

	m_use_bc = true;

	#if WITH_ILI9431_FONTS
		font = 0;
	#endif
}

//----------------------------------------
// public API
//----------------------------------------

void myLcd::Set_Text_Size(uint8_t s)
{
	text_size = s;
}

void myLcd::Set_Text_Cursor(int16_t x, int16_t y)
{
	text_x = x;
	text_y = y;
}

void myLcd::Set_Text_colour(uint16_t color)
{
	text_color = color;
}

void myLcd::Set_Text_Back_colour(uint16_t color)
{
	text_bgcolor = color;
}

void myLcd::Set_Draw_color(uint16_t color)
{
	draw_color = color;
}

void myLcd::Fill_Screen(uint16_t color)
{
	Fill_Rect(0, 0, Get_Width(), Get_Height(), color);
}


void myLcd::Fill_Circle(int16_t x, int16_t y, int16_t radius)
{
	Draw_Fast_VLine(x, y-radius, 2*radius+1);
	Fill_Circle_Helper(x, y, radius, 3, 0);
}


void myLcd::Draw_Circle(int16_t x, int16_t y, int16_t radius)
{
	int16_t f = 1 - radius;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * radius;
	int16_t x1= 0;
	int16_t y1= radius;

	Draw_Pixel(x, y+radius, draw_color);
 	Draw_Pixel(x, y-radius, draw_color);
	Draw_Pixel(x+radius, y, draw_color);
	Draw_Pixel(x-radius, y, draw_color);

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

		Draw_Pixel(x + x1, y + y1, draw_color);
    	Draw_Pixel(x - x1, y + y1, draw_color);
		Draw_Pixel(x + x1, y - y1, draw_color);
		Draw_Pixel(x - x1, y - y1, draw_color);
		Draw_Pixel(x + y1, y + x1, draw_color);
		Draw_Pixel(x - y1, y + x1, draw_color);
		Draw_Pixel(x + y1, y - x1, draw_color);
		Draw_Pixel(x - y1, y - x1, draw_color);
 	}
}


void myLcd::Draw_Line(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
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
      		Draw_Pixel(y1, x1, draw_color);
    	}
		else
		{
			// Serial.printf("DrawPixel(%d,%d)\n",x1,y1);
      		Draw_Pixel(x1, y1, draw_color);
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



// prh - talk about a weird amateur API
// as well as polluting namespace ..
// these constants are no longer effing public
#define LEFT 0
#define RIGHT 9999
#define CENTER 9998


//print string
size_t myLcd::Print_String(const uint8_t *st, int16_t x, int16_t y)
{
	int16_t pos;
	uint16_t len;
	const char * p = (const char *)st;
	size_t n = 0;
	if (x == CENTER || x == RIGHT)
	{
		len = strlen((const char *)st) * 6 * text_size;
		pos = (Get_Width() - len);
		if (x == CENTER)
		{
			x = pos/2;
		}
		else
		{
			x = pos - 1;
		}
	}
    Set_Text_Cursor(x, y);
	while(1)
	{
		unsigned char ch = *(p++);//pgm_read_byte(p++);
		if(ch == 0)
		{
			break;
		}
		if(write(ch))
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


void myLcd::Draw_Fast_VLine(int16_t x, int16_t y, int16_t h)
{
	Fill_Rect(x, y, 1, h, draw_color);
}


void myLcd::Fill_Circle_Helper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername,int16_t delta)
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
      		Draw_Fast_VLine(x0+x, y0-y, 2*y+1+delta);
      		Draw_Fast_VLine(x0+y, y0-x, 2*x+1+delta);
    	}
    	if (cornername & 0x2)
		{
      		Draw_Fast_VLine(x0-x, y0-y, 2*y+1+delta);
      		Draw_Fast_VLine(x0-y, y0-x, 2*x+1+delta);
    	}
  	}
}



// draw a char
// - assumes a 6x8 full box font
// - font's are stored as bytes containing columns
// - every 6 bytes is another character
// - size is a simple integer multiplier
// - background only drawn IFF bg is different than color
// - "use_bc" forces background drawing off


void myLcd::Draw_Char(
	int16_t x,
	int16_t y,
	uint8_t c,
	uint16_t color,
	uint16_t bg,
	uint8_t size,
	boolean use_bc)	// prh changed from "mode"
{
	if((x >= Get_Width()) || (y >= Get_Height()) || ((x + 6 * size - 1) < 0) || ((y + 8 * size - 1) < 0))
	{
    	return;
	}
  	if(c >= 176) // prh huh?
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
        			Draw_Pixel(x+i, y+j, color);
        		}
        		else
				{
					Fill_Rect(x+(i*size), y+(j*size), size, size, color);
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
					Draw_Pixel(x+i, y+j, bg);
				}
				else
				{
					Fill_Rect(x+i*size, y+j*size, size, size, bg);
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
	#if WITH_ILI9431_FONTS
		if (font)
		{
			if (c == '\n')
			{
				text_y += font->line_space;
				text_x = 0;
			}
			else if(c == '\r')
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
    	text_y += text_size*8;
    	text_x  = 0;
 	}
	else if(c == '\r')
	{
	}
	else
	{
    	Draw_Char(text_x, text_y, c, text_color, text_bgcolor, text_size,m_use_bc);		// prh text_mode);
    	text_x += text_size*6;
    }
  	return 1;
}
