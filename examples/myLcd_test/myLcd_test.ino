//------------------------------------------------------------
// myLcd_test
//------------------------------------------------------------
// Written to verify that the myLcd library works on the Uno
// with the 3.5" shield, when it wasn't on the teensy

#include <myLcdDevice.h>
#include <myDebug.h>

// Following Comments from LCDWIKI_KBD:
//
// Set the pins to the correct ones for your development shield or breakout board.
// when using the BREAKOUT BOARD only and using these 8 data lines to the LCD,
// pin usage as follow:
//              CS  CD  WR  RD  RST  D0  D1  D2  D3  D4  D5  D6  D7
// Arduino Uno  A3  A2  A1  A0  A4   8   9   2   3   4   5   6   7
// Arduino Mega A3  A2  A1  A0  A4   8   9   2   3   4   5   6   7
//
// The 16bit mode only use in Mega.you must modify the mode in the file of lcd_mode.h
// when using the BREAKOUT BOARD only and using these 16 data lines to the LCD,
// pin usage as follow:
//              CS  CD  WR  RD  RST  D0  D1  D2  D3  D4  D5  D6  D7  D8  D9  D10  D11  D12  D13  D14  D15
// Arduino Mega 40  38  39  44  41   37  36  35  34  33  32  31  30  22  23  24   25   26   27   28   29
//
// 8 Bit Mode
// If the IC model is known or the modules is unreadable,you can use this constructed function
// 		LCDWIKI_KBV mylcd(ILI9341,A3,A2,A1,A0,A4); //model,cs,cd,wr,rd,reset
// 		LCDWIKI_KBV mylcd(ILI9325,A3,A2,A1,A0,A4); //model,cs,cd,wr,rd,reset
// 		LCDWIKI_KBV mylcd(ILI9328,A3,A2,A1,A0,A4); //model,cs,cd,wr,rd,reset
// 		LCDWIKI_KBV mylcd(HX8357D,A3,A2,A1,A0,A4); //model,cs,cd,wr,rd,reset
// 		LCDWIKI_KBV mylcd(HX8347G,A3,A2,A1,A0,A4); //model,cs,cd,wr,rd,reset
// 		LCDWIKI_KBV mylcd(HX8347I,A3,A2,A1,A0,A4); //model,cs,cd,wr,rd,reset
// 		LCDWIKI_KBV mylcd(ILI9486,A3,A2,A1,A0,A4); //model,cs,cd,wr,rd,reset
// 		LCDWIKI_KBV mylcd(ST7735S,A3,A2,A1,A0,A4); //model,cs,cd,wr,rd,reset
// if the IC model is not known and the modules is readable,you can use this constructed function
//		LCDWIKI_KBV mylcd(240,320,A3,A2,A1,A0,A4);//width,height,cs,cd,wr,rd,reset
//		LCDWIKI_KBV mylcd(320,480,A3,A2,A1,A0,A4);//width,height,cs,cd,wr,rd,reset
//
// 16 Bit Mode
// if the IC model is known or the modules is unreadable,you can use this constructed function
//		LCDWIKI_KBV mylcd(ILI9341,40,38,39,44,41); //model,cs,cd,wr,rd,reset
//		LCDWIKI_KBV mylcd(ILI9325,40,38,39,44,41); //model,cs,cd,wr,rd,reset
//		LCDWIKI_KBV mylcd(ILI9328,40,38,39,44,41); //model,cs,cd,wr,rd,reset
//		LCDWIKI_KBV mylcd(HX8357D,40,38,39,44,41); //model,cs,cd,wr,rd,reset
//		LCDWIKI_KBV mylcd(HX8347G,40,38,39,44,41); //model,cs,cd,wr,rd,reset
//		LCDWIKI_KBV mylcd(HX8347I,40,38,39,44,41); //model,cs,cd,wr,rd,reset
//		LCDWIKI_KBV mylcd(ILI9486,40,38,39,44,41); //model,cs,cd,wr,rd,reset
//		LCDWIKI_KBV mylcd(ILI9488,40,38,39,44,41); //model,cs,cd,wr,rd,reset
//		LCDWIKI_KBV mylcd(ILI9481,40,38,39,44,41); //model,cs,cd,wr,rd,reset
//		LCDWIKI_KBV mylcd(ST7735S,40,38,39,44,41); //model,cs,cd,wr,rd,reset
//if the IC model is not known and the modules is readable,you can use this constructed function
//		LCDWIKI_KBV mylcd(240,320,40,38,39,44,41);//width,height,cs,cd,wr,rd,reset for
//		LCDWIKI_KBV mylcd(320,480,40,38,39,44,41);//width,height,cs,cd,wr,rd,reset

// define some colour values

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF


#ifdef __LCD_TEENSY__		// My Teensy Configuration

	#define TFT_DATA0     34      // needed by ts
	#define TFT_DATA1     35      // needed by ts
	#define TFT_DATA2     36
	#define TFT_DATA3     37
	#define TFT_DATA4     38
	#define TFT_DATA5     39
	#define TFT_DATA6     13
	#define TFT_DATA7     19

	#define TFT_RD        14
	#define TFT_WR        15
	#define TFT_CD_RS     16      // needed by ts - labelled "RS" on board
	#define TFT_CS        17      // needed by ts
	#define TFT_RESET     18

	myLcdDevice mylcd(
		ILI9486,
		TFT_CS,
		TFT_CD_RS,
		TFT_WR,
		TFT_RD,
		TFT_RESET);

#else	// tested and works as an Arduino Uno Shield

	myLcdDevice mylcd(ILI9486,A3,A2,A1,A0,A4); //model,cs,cd,wr,rd,reset

#endif





void setup()
{
	Serial.begin(115200);
	delay(1200);
	display(0,"testLcd.ino setup() started",0);

    #ifdef __LCD_TEENSY__    // My Teensy Configuration
        setTFTDataPins(
            TFT_DATA0,
            TFT_DATA1,
            TFT_DATA2,
            TFT_DATA3,
            TFT_DATA4,
            TFT_DATA5,
            TFT_DATA6,
            TFT_DATA7);
    #endif

    mylcd.begin();
    mylcd.setRotation(1);
    mylcd.fillScreen(BLACK);

    // mylcd.Set_Text_Mode(0);
    mylcd.fillScreen(0x0000);
    mylcd.setTextBackColor(BLACK);
	mylcd.setTextColor(YELLOW);
	mylcd.setTextSize(5);
	mylcd.drawString((const uint8_t *) "Hello World!", 0, 0);

	display(0,"testLcd.ino setup() finished",0);
}


void loop()
{
}
