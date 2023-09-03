//-----------------------------------------------
// myLcdUnused.cpp
//-----------------------------------------------
// These methods are NOT used by my program.
// You can prove that by setting the following #ifdef to 0
// Nonetheless, I renamed them all consistently with my other changes

#if 0		// entire file ifdef'd out

#include "myLcd.h"


#define swap(a, b) { int16_t t = a; a = b; b = t; }


//set 8bits r,g,b color
void myLcd::setDrawColor(uint8_t r, uint8_t g, uint8_t b)
{
	_draw_color = color565(r, g, b);
}

//get draw color
uint16_t myLcd::getDrawColor(void) const
{
	return _draw_color;
}


//read color data for point(x,y)
uint16_t myLcd::readPixel(int16_t x, int16_t y)
{
	uint16_t colour;
	readRect(x, y, 1, 1, &colour);
	return colour;
}

//fill a rectangle
void myLcd::fillRect(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
	int w = x2 - x1 + 1, h = y2 - y1 + 1;
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
	fillRect(x1, y1, w, h, _draw_color);
}




//Fill the full screen with r,g,b
void myLcd::fillScreen(uint8_t r, uint8_t g, uint8_t b)
{
	uint16_t color = color565(r, g, b);
	fillRect(0, 0, width(), height(), color);
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

//draw a round rectangle
void myLcd::drawRoundRect(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t radius)
{
	int w = x2 - x1 + 1, h = y2 - y1 + 1;
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
	drawFastHLine(x1+radius, y1, w-2*radius);
	drawFastHLine(x1+radius, y1+h-1, w-2*radius);
	drawFastVLine(x1, y1+radius, h-2*radius);
  	drawFastVLine(x1+w-1, y1+radius, h-2*radius);
	drawCircleHelper(x1+radius, y1+radius, radius, 1);
	drawCircleHelper(x1+w-radius-1, y1+radius, radius, 2);
	drawCircleHelper(x1+w-radius-1, y1+h-radius-1, radius, 4);
	drawCircleHelper(x1+radius, y1+h-radius-1, radius, 8);
}

//fill a round rectangle
void myLcd::fillRoundRect(int16_t x1, int16_t y1, int16_t x2,int16_t y2, int16_t radius)
{
	int w = x2 - x1 + 1, h = y2 - y1 + 1;
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
	fillRect(x1+radius, y1, w-2*radius, h, _draw_color);
	fillCircleHelper(x1+w-radius-1, y1+radius, radius, 1, h-2*radius-1);
	fillCircleHelper(x1+radius, y1+radius, radius, 2, h-2*radius-1);
}



//draw a circular bead
void myLcd::drawCircleHelper(int16_t x0, int16_t y0, int16_t radius, uint8_t cornername)
{
	int16_t f     = 1 - radius;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * radius;
	int16_t x     = 0;
	int16_t y     = radius;
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
	    if (cornername & 0x4)
		{
			drawPixel(x0 + x, y0 + y);
			drawPixel(x0 + y, y0 + x);
	    }
	    if (cornername & 0x2)
		{
			drawPixel(x0 + x, y0 - y);
			drawPixel(x0 + y, y0 - x);
	    }
	    if (cornername & 0x8)
		{
			drawPixel(x0 - y, y0 + x);
			drawPixel(x0 - x, y0 + y);
	    }
	    if (cornername & 0x1)
		{
			drawPixel(x0 - y, y0 - x);
	 		drawPixel(x0 - x, y0 - y);
	    }
  	}
}

//uint8_t LCDKIWI_GUI::Get_Screen_Rotation(void) const
//{
//	return LCDKIWI_KBV::getRotation();
//}

//void LCDKIWI_GUI::Push_Colors(uint16_t * block, int16_t n, boolean first, uint8_t flags)
//{
//	LCDKIWI_KBV::pushAnyColor(block, n, first, flags);
//}

//void LCDKIWI_GUI::Push_Colors(uint8_t * block, int16_t n, boolean first, uint8_t flags)
//{
//	LCDKIWI_KBV::pushAnyColor(block, n, first, flags);
//}

//draw a bit map
void myLcd::drawBitmap(int16_t x, int16_t y, int16_t sx, int16_t sy, const uint16_t *data, int16_t scale)
{
	int16_t color;
	setAddrWindow(x, y, x + sx*scale - 1, y + sy*scale - 1);
	if(1 == scale)
	{

		pushAnyColor((uint16_t *)data, sx * sy, 1, 0);
	}
	else
	{
		for (int16_t row = 0; row < sy; row++)
		{
			for (int16_t col = 0; col < sx; col++)
			{
				color = *(data + (row*sx + col)*1);//pgm_read_word(data + (row*sx + col)*1);
				fillRect(x+col*scale, y+row*scale, scale, scale, color);
			}
		}
	}
}

//get text x coordinate
int16_t myLcd::getCursorX(void) const
{
	return _text_x;
}

//get text y coordinate
int16_t myLcd::getCursorY(void) const
{
	return _text_y;
}

//set text colour with 8bits r,g,b
void myLcd::setTextColor(uint8_t r, uint8_t g, uint8_t b)
{
	_text_color = color565(r, g, b);
}

//get text colour
uint16_t myLcd::getTextColor(void) const
{
	return _text_color;
}


//set text background colour with 8bits r,g,b
void myLcd::setTextBackColor(uint8_t r, uint8_t g, uint8_t b)
{
	_text_bgcolor = color565(r, g, b);
}

//get text background colour
uint16_t myLcd::getTextBackColor(void) const
{
	return _text_bgcolor;
}



//get text size
uint8_t myLcd::getTextSize(void) const
{
	return _text_size;
}


// prh - removed stupid text_mode concept
// void myLcd::Set_Text_Mode(boolean mode)
// {
// 	text_mode = mode;
// }
//
// boolean myLcd::Get_Text_Mode(void) const
// {
// 	return text_mode;
// }




//print string
void myLcd::drawString(String st, int16_t x, int16_t y)
{
	drawString((uint8_t *)(st.c_str()), x, y);
}

//print int number
void myLcd::drawNumber(long num, int16_t x, int16_t y, int16_t length, uint8_t filler, int16_t system)
{
	uint8_t st[27] = {0};
	uint8_t *p = st+26;
	boolean flag = false;
	int16_t len = 0,nlen = 0,left_len = 0,i = 0;
	*p = '\0';
	if(0 == num)
	{
		*(--p) = '0';
		len = 1;
	}
	else
	{
		if(num < 0)
		{
			num = -num;
			flag = true;
		}
	}
	while((num > 0) && (len < 10))
	{
		if(num%system > 9)
		{
			*(--p) = 'A' + (num%system-10);
		}
		else
		{
			*(--p) = '0' + num%system;
		}
		num = num/system;
		len++;
	}
	if(flag)
	{
		*(--p) = '-';
	}
	if(length > (len + flag + 1))
	{
		if(length > (int16_t) sizeof(st))
		{
			nlen = sizeof(st) - len - flag - 1;
		}
		else
		{
			nlen = length - len - flag - 1;
		}
		for(i = 0;i< nlen;i++)
		{
			*(--p) = filler;
		}
		left_len = sizeof(st) - nlen - len - flag - 1;
	}
	else
	{
		left_len = sizeof(st) - len - flag - 1;
	}
	for(i = 0; i < (int16_t)(sizeof(st) - left_len);i++)
	{
		st[i] = st[left_len + i];
	}
	st[i] = '\0';
	drawString(st, x, y);
}


//print float number
void myLcd::drawFloat(double num, uint8_t dec, int16_t x, int16_t y, uint8_t divider, int16_t length, uint8_t filler)
{
	uint8_t st[27] = {0};
	uint8_t * p = st;
	boolean flag = false;
	int16_t i = 0;
	if(dec<1)
	{
		dec=1;
	}
	else if(dec>5)
	{
		dec=5;
	}
	if(num<0)
	{
		flag = true;
	}
	dtostrf(num, length, dec, (char *)st);
	if(divider != '.')
	{
		while(i < (int16_t)sizeof(st))
		{
			if('.' == *(p+i))
			{
				*(p+i) = divider;
			}
			i++;
		}
	}
	if(filler != ' ')
	{
		if(flag)
		{
			*p = '-';
			i = 1;
			while(i < (int16_t)sizeof(st))
			{
				if((*(p+i) == ' ') || (*(p+i) == '-'))
				{
					*(p+i) = filler;
				}
				i++;
			}
		}
		else
		{
			i = 0;
			while(i < (int16_t) sizeof(st))
			{
				if(' ' == *(p+i))
				{
					*(p+i) = filler;
				}
			}
		}
	}
	drawString(st, x, y);
}





//uint16_t LCDKIWI_GUI::Change_To_565(uint8_t r, uint8_t g, uint8_t b)
//{
//	return LCDKIWI_KBV::color565(r, g, b);
//}

//uint16_t LCDKIWI_GUI::Read_Dev_ID(void)
//{
//	return LCDKIWI_KBV::readID();

//}

//void LCDKIWI_GUI::Dev_Invert_Display(boolean i)
//{
//	LCDKIWI_KBV::invertDisplay(i);
//}

//void LCDKIWI_GUI::Dev_Vert_Scroll(int16_t top, int16_t scrollines, int16_t offset)
//{
//	LCDKIWI_KBV::vertScroll(top, scrollines, offset);
//}


#endif	// entire file ifdef'd out
