//---------------------------------------------
// myLcdDevice.cpp
//----------------------------------------------
// IMPORTANT: LIBRARY MUST BE SPECIFICALLY CONFIGURED FOR EITHER TFT SHIELD
// OR BREAKOUT BOARD USAGE.

// if using 8bit mode,set the below macro definition to 1
// if using 16bit mode,set the below macro definition to 0

#define CONFIG_USE_8BIT_BUS 1


#define dbg_ld  0

#include "myLcdDevice.h"
#include "myLcdDeviceRegs.h"
#include <myDebug.h>


#if defined(__SAM3X8E__)
	#include <include/pio.h>
    #define PROGMEM
    #define pgm_read_byte(addr) (*(const unsigned char *)(addr))
    #define pgm_read_word(addr) (*(const unsigned short *)(addr))
#endif


#include "pins_arduino.h"

#if !__LCD_TEENSY__
	#include "wiring_private.h"
#endif



#if (CONFIG_USE_8BIT_BUS==1)
	#include "myLcdDevice8.h"
#elif (CONFIG_USE_8BIT_BUS==0)
	#include "myLcdDevice16.h"
#endif

#define TFTLCD_DELAY16  0xFFFF
#define TFTLCD_DELAY8   0x7F
#define MAX_REG_NUM     24



// #define LEFT_SHIFT(x) (1<<x) //x value 0:mipi dcs rev1
	//  1: auto readinc
	//  2: read bgr
	//  3: read low and high
	//  4: read 24 bits
	//  5: xsa and xea 16 bit
	//  6: read no dummy
	//  8: invert gs
	//  9: invert ss
	// 10: mv axis
	// 11: invert rgb
	// 12: rev screen
	// 13: filp vert
	// 14: flip horiz

lcd_info current_lcd_info[] = {
	0x9325,240,320,
	0x9328,240,320,
	0x9341,240,320,
	0x9090,320,480,
	0x7575,240,320,
	0x9595,240,320,
	0x9486,320,480,
	0x7735,128,160,
	0x9488,320,480,
	0x9481,320,480 };



myLcdDevice::myLcdDevice(SPIClass *spi_ptr, uint8_t _CS, uint8_t _DC, uint8_t _RST /*=255*/, uint8_t _MOSI /*=255*/, uint8_t _SCLK /*=255*/, uint8_t _MISO /*=255*/)
{
	spi = spi_ptr;
}


myLcdDevice::myLcdDevice(uint16_t model,uint8_t cs, uint8_t cd, uint8_t wr, uint8_t rd, uint8_t reset)
{
	#if __LCD_TEENSY__

		_reset = reset;			// 8
		_cs = cs;				// 9
		_cd = cd;				// 10
		_wr = wr;				// 11
		_rd = rd;				// 12

		pinMode(_reset,	OUTPUT);
		pinMode(_cs, 	OUTPUT);
		pinMode(_cd, 	OUTPUT);
		pinMode(_wr, 	OUTPUT);
		pinMode(_rd, 	OUTPUT);

		digitalWrite(cs, 	LOW);
		digitalWrite(cd, 	LOW);
		digitalWrite(wr, 	LOW);
		digitalWrite(rd, 	LOW);
		digitalWrite(reset, HIGH);

	#else	// !__LCD_TEENSY__ prh

		#ifndef USE_ADAFRUIT_SHIELD_PIN
			// Convert pin numbers to registers and bitmasks
			_reset	 = reset;
			#ifdef __AVR__
				csPort	   = portOutputRegister(digitalPinToPort(cs));
				cdPort	   = portOutputRegister(digitalPinToPort(cd));
				wrPort	   = portOutputRegister(digitalPinToPort(wr));
				rdPort	   = portOutputRegister(digitalPinToPort(rd));
			#endif
			#if defined(__SAM3X8E__)
				csPort	   = digitalPinToPort(cs);
				cdPort	   = digitalPinToPort(cd);
				wrPort	   = digitalPinToPort(wr);
				rdPort	   = digitalPinToPort(rd);
			#endif
			csPinSet	 = digitalPinToBitMask(cs);
			cdPinSet	 = digitalPinToBitMask(cd);
			wrPinSet	 = digitalPinToBitMask(wr);
			rdPinSet	 = digitalPinToBitMask(rd);
			csPinUnset = ~csPinSet;
			cdPinUnset = ~cdPinSet;
			wrPinUnset = ~wrPinSet;
			rdPinUnset = ~rdPinSet;
			#ifdef __AVR__
				*csPort   |=  csPinSet; // Set all control bits to HIGH (idle)
				*cdPort   |=  cdPinSet; // Signals are ACTIVE LOW
				*wrPort   |=  wrPinSet;
				*rdPort   |=  rdPinSet;
			#endif
			#if defined(__SAM3X8E__)
				csPort->PIO_SODR  |=  csPinSet; // Set all control bits to HIGH (idle)
				cdPort->PIO_SODR  |=  cdPinSet; // Signals are ACTIVE LOW
				wrPort->PIO_SODR  |=  wrPinSet;
				rdPort->PIO_SODR  |=  rdPinSet;
			#endif
			pinMode(cs, OUTPUT);	  // Enable outputs
			pinMode(cd, OUTPUT);
			pinMode(wr, OUTPUT);
			pinMode(rd, OUTPUT);
			if (reset)
			{
				digitalWrite(reset, HIGH);
				pinMode(reset, OUTPUT);
			}
		#else	// USE_ADAFRUIT_SHIELD_PIN
			CS_IDLE; 	// Set all control bits to idle state
			WR_IDLE;
			RD_IDLE;
			CD_DATA;
			digitalWrite(5, HIGH); // Reset line
			pinMode(A3, OUTPUT);   // Enable outputs
			pinMode(A2, OUTPUT);
			pinMode(A1, OUTPUT);
			pinMode(A0, OUTPUT);
			pinMode(5, OUTPUT);
		#endif

		// added to begin() in my version:

		setWriteDir();

	#endif	// !__LCD_TEENSY__  prh

 	_rotation = 0;
 	lcd_model = current_lcd_info[model].lcd_id;

	WIDTH = current_lcd_info[model].lcd_wid;
	HEIGHT = current_lcd_info[model].lcd_heg;

	_width = WIDTH;
	_height = HEIGHT;
}



#if !__LCD_TEENSY__	// prh teensy mods

	myLcdDevice::myLcdDevice(int16_t wid,int16_t heg,uint8_t cs, uint8_t cd, uint8_t wr, uint8_t rd, uint8_t reset)
	{
		#ifndef USE_ADAFRUIT_SHIELD_PIN
			// Convert pin numbers to registers and bitmasks
			_reset	 = reset;
			#ifdef __AVR__
				csPort	   = portOutputRegister(digitalPinToPort(cs));
				cdPort	   = portOutputRegister(digitalPinToPort(cd));
				wrPort	   = portOutputRegister(digitalPinToPort(wr));
				rdPort	   = portOutputRegister(digitalPinToPort(rd));
			#endif
			#if defined(__SAM3X8E__)
				csPort	   = digitalPinToPort(cs);
				cdPort	   = digitalPinToPort(cd);
				wrPort	   = digitalPinToPort(wr);
				rdPort	   = digitalPinToPort(rd);
			#endif
			csPinSet	 = digitalPinToBitMask(cs);
			cdPinSet	 = digitalPinToBitMask(cd);
			wrPinSet	 = digitalPinToBitMask(wr);
			rdPinSet	 = digitalPinToBitMask(rd);
			csPinUnset = ~csPinSet;
			cdPinUnset = ~cdPinSet;
			wrPinUnset = ~wrPinSet;
			rdPinUnset = ~rdPinSet;
			#ifdef __AVR__
				*csPort   |=  csPinSet; // Set all control bits to HIGH (idle)
				*cdPort   |=  cdPinSet; // Signals are ACTIVE LOW
				*wrPort   |=  wrPinSet;
				*rdPort   |=  rdPinSet;
			#endif
			#if defined(__SAM3X8E__)
				csPort->PIO_SODR  |=  csPinSet; // Set all control bits to HIGH (idle)
				cdPort->PIO_SODR  |=  cdPinSet; // Signals are ACTIVE LOW
				wrPort->PIO_SODR  |=  wrPinSet;
				rdPort->PIO_SODR  |=  rdPinSet;
			#endif
			pinMode(cs, OUTPUT);	  // Enable outputs
			pinMode(cd, OUTPUT);
			pinMode(wr, OUTPUT);
			pinMode(rd, OUTPUT);
			if (reset)
			{
				digitalWrite(reset, HIGH);
				pinMode(reset, OUTPUT);
			}
		#else	// USE_ADAFRUIT_SHIELD_PIN
			CS_IDLE; // Set all control bits to idle state
			WR_IDLE;
			RD_IDLE;
			CD_DATA;
			digitalWrite(5, HIGH); // Reset line
			pinMode(A3, OUTPUT);   // Enable outputs
			pinMode(A2, OUTPUT);
			pinMode(A1, OUTPUT);
			pinMode(A0, OUTPUT);
			pinMode(5, OUTPUT);
		#endif
		_rotation = 0;
		lcd_model = 0xFFFF;
		setWriteDir();
		WIDTH = wid;
		HEIGHT = heg;
		_width = WIDTH;
		_height = HEIGHT;
	}

#endif	// !__LCD_TEENSY__  prh teensy mods


//-----------------------------
// init && reset
//-----------------------------

void myLcdDevice::begin(void)
{
	setWriteDir();	// prh added in my version, should be benign
	reset();
	if (lcd_model == 0xFFFF)
	{
		lcd_model = readID();
	}
	// prh - for debuggin
	readID();
	start(lcd_model);
//	setRotation(0);
}


void myLcdDevice::reset(void)
{
	// have_reset = 1;
	// setWriteDir();
	// Set all control bits to idle state

    CS_IDLE;
    RD_IDLE;
    WR_IDLE;

	#ifdef USE_ADAFRUIT_SHIELD_PIN
		digitalWrite(5, LOW);
		delay(2);
		digitalWrite(5, HIGH);
		// delay(100);
		// digitalWrite(5, LOW);
		// delay(100);
	#else
		if (_reset)
		{
			digitalWrite(_reset, LOW);
			delay(2);
			digitalWrite(_reset, HIGH);
			// delay(100);
			// digitalWrite(_reset, LOW);
			// delay(100);
		}
	#endif

	CS_ACTIVE;
	CD_COMMAND;
	write8(0x00);
	for(uint8_t i=0; i<3; i++)
	{
		WR_STROBE; // Three extra 0x00s
	}
	CS_IDLE;
}


//---------------------------------------------
// start
//---------------------------------------------

void myLcdDevice:: init_table8(const void *table, int16_t size)
{
	uint8_t i;
    uint8_t *p = (uint8_t *) table, dat[MAX_REG_NUM];            //R61526 has GAMMA[22]
    while (size > 0)
	{
        uint8_t cmd = pgm_read_byte(p++);
        uint8_t len = pgm_read_byte(p++);
        if (cmd == TFTLCD_DELAY8)
		{
            delay(len);
            len = 0;
        }
		else
		{
            for (i = 0; i < len; i++)
            {
                dat[i] = pgm_read_byte(p++);
            }
			Push_Command(cmd,dat,len);
        }
        size -= len + 2;
    }
}

void myLcdDevice:: init_table16(const void *table, int16_t size)
{
    uint16_t *p = (uint16_t *) table;
    while (size > 0)
	{
        uint16_t cmd = pgm_read_word(p++);
        uint16_t d = pgm_read_word(p++);
        if (cmd == TFTLCD_DELAY16)
        {
            delay(d);
        }
        else
		{
			writeCmdData16(cmd, d);                      //static function
        }
        size -= 2 * sizeof(int16_t);
    }
}



void myLcdDevice::start(uint16_t ID)
{
	reset();
	delay(200);
	switch(ID)
	{
		case 0x9325:
		case 0x9328:
			lcd_driver = ID_932X;
			XC = 0;
			YC = 0;
			CC = ILI932X_RW_GRAM;
			RC = ILI932X_RW_GRAM;
			SC1 = ILI932X_GATE_SCAN_CTRL2;
			SC2 = ILI932X_GATE_SCAN_CTRL3;
			MD = 0x0003;
			VL = 1;
			R24BIT = 0;
			static const uint16_t ILI932x_regValues[] PROGMEM =
			{
		  		ILI932X_START_OSC 	   		, 0x0001, // Start oscillator
		  		TFTLCD_DELAY16				, 50,	 // 50 millisecond delay
		  		ILI932X_DRIV_OUT_CTRL		, 0x0100,
		  		ILI932X_DRIV_WAV_CTRL		, 0x0700,
		  		ILI932X_ENTRY_MOD			, 0x1030,
		  		ILI932X_RESIZE_CTRL			, 0x0000,
		  		ILI932X_DISP_CTRL2			, 0x0202,
		  		ILI932X_DISP_CTRL3			, 0x0000,
		  		ILI932X_DISP_CTRL4			, 0x0000,
		  		ILI932X_RGB_DISP_IF_CTRL1	, 0x0,
		  		ILI932X_FRM_MARKER_POS   	, 0x0,
		  		ILI932X_RGB_DISP_IF_CTRL2	, 0x0,
		  		ILI932X_POW_CTRL1			, 0x0000,
		  		ILI932X_POW_CTRL2			, 0x0007,
		  		ILI932X_POW_CTRL3			, 0x0000,
		  		ILI932X_POW_CTRL4			, 0x0000,
		  		TFTLCD_DELAY16				, 200,
		  		ILI932X_POW_CTRL1			, 0x1690,
		  		ILI932X_POW_CTRL2			, 0x0227,
		  		TFTLCD_DELAY16				, 50,
		  		ILI932X_POW_CTRL3			, 0x001A,
		  		TFTLCD_DELAY16				, 50,
		  		ILI932X_POW_CTRL4			, 0x1800,
		  		ILI932X_POW_CTRL7			, 0x002A,
		  		TFTLCD_DELAY16				, 50,
		  		ILI932X_GAMMA_CTRL1			, 0x0000,
		  		ILI932X_GAMMA_CTRL2			, 0x0000,
		  		ILI932X_GAMMA_CTRL3			, 0x0000,
		  		ILI932X_GAMMA_CTRL4			, 0x0206,
		  		ILI932X_GAMMA_CTRL5			, 0x0808,
		  		ILI932X_GAMMA_CTRL6			, 0x0007,
		  		ILI932X_GAMMA_CTRL7			, 0x0201,
		  		ILI932X_GAMMA_CTRL8			, 0x0000,
		  		ILI932X_GAMMA_CTRL9			, 0x0000,
		 		ILI932X_GAMMA_CTRL10		, 0x0000,
		  		ILI932X_GRAM_HOR_AD			, 0x0000,
		  		ILI932X_GRAM_VER_AD			, 0x0000,
		  		ILI932X_HOR_START_AD		, 0x0000,
		  		ILI932X_HOR_END_AD			, 0x00EF,
		  		ILI932X_VER_START_AD		, 0X0000,
		  		ILI932X_VER_END_AD			, 0x013F,
		  		ILI932X_GATE_SCAN_CTRL1		, 0xA700, // Driver Output Control (R60h)
		  		ILI932X_GATE_SCAN_CTRL2		, 0x0003, // Driver Output Control (R61h)
		  		ILI932X_GATE_SCAN_CTRL3		, 0x0000, // Driver Output Control (R62h)
		  		ILI932X_PANEL_IF_CTRL1		, 0X0010, // Panel Interface Control 1 (R90h)
		  		ILI932X_PANEL_IF_CTRL2		, 0X0000,
		  		ILI932X_PANEL_IF_CTRL3		, 0X0003,
		  		ILI932X_PANEL_IF_CTRL4		, 0X1100,
		  		ILI932X_PANEL_IF_CTRL5		, 0X0000,
		  		ILI932X_PANEL_IF_CTRL6		, 0X0000,
		  		ILI932X_DISP_CTRL1			, 0x0133 // Main screen turn on
			};
			init_table16(ILI932x_regValues, sizeof(ILI932x_regValues));
			break;
		case 0x9341:
			lcd_driver = ID_9341;
			XC = ILI9341_COLADDRSET;
			YC = ILI9341_PAGEADDRSET;
			CC = ILI9341_MEMORYWRITE;
			RC = HX8357_RAMRD;
			SC1 = 0x33;
			SC2 = 0x37;
			MD = ILI9341_MADCTL;
			VL = 0;
			R24BIT = 1;
			static const uint8_t ILI9341_regValues[] PROGMEM =
			{   // BOE 2.4
				ILI9341_SOFTRESET,			0,                 //Soft Reset
				TFTLCD_DELAY8,				50,
				ILI9341_DISPLAYOFF,			0,            						// Display Off
				// ILI9341_PIXELFORMAT, 	1, 0x55,      						// Pixel read=565, write=565.
            	ILI9341_INTERFACECONTROL, 	3, 0x01, 0x01, 0x00,  				// Interface Control needs EXTC=1 MV_EOR=0, TM=0, RIM=0
            	ILI9341_POWERCONTROLB, 		3, 0x00, 0x81, 0x30,  				// Power Control B [00 81 30]
            	ILI9341_POWERONSEQ, 		4, 0x64, 0x03, 0x12, 0x81,    		// Power On Seq [55 01 23 01]
            	ILI9341_DRIVERTIMINGA, 		3, 0x85, 0x10, 0x78,  				// Driver Timing A [04 11 7A]
            	ILI9341_POWERCONTROLA, 		5, 0x39, 0x2C, 0x00, 0x34, 0x02,    // Power Control A [39 2C 00 34 02]
            	ILI9341_RUMPRATIO, 			1, 0x20,      						// Pump Ratio [10]
            	ILI9341_DRIVERTIMINGB, 		2, 0x00, 0x00,        				// Driver Timing B [66 00]
            	ILI9341_RGBSIGNAL, 			1, 0x00,      						// RGB Signal [00]
				// ILI9341_FRAMECONTROL, 	2, 0x00, 0x1B,        				// Frame Control [00 1B]
            	// 0xB6, 					2, 0x0A, 0xA2, 0x27, 				// Display Function [0A 82 27 XX]    .kbv SS=1
            	ILI9341_INVERSIONCONRTOL, 	1, 0x00,      						// Inversion Control [02] .kbv NLA=1, NLB=1, NLC=1
            	ILI9341_POWERCONTROL1, 		1, 0x21,      						// Power Control 1 [26]
            	ILI9341_POWERCONTROL2, 		1, 0x11,      						// Power Control 2 [00]
            	ILI9341_VCOMCONTROL1, 		2, 0x3F, 0x3C,        				// VCOM 1 [31 3C]
            	ILI9341_VCOMCONTROL2, 		1, 0xB5,      						// VCOM 2 [C0]
            	ILI9341_MEMCONTROL, 		1, ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR,
            	ILI9341_PIXELFORMAT, 		1, 0x55,      						// Pixel read=565, write=565.
            	ILI9341_FRAMECONTROL, 		2, 0x00, 0x1B,        				// Frame Control [00 1B]
            	ILI9341_MEMORYACCESS, 		1, 0x48,      						// Memory Access [00]
            	ILI9341_ENABLE3G, 			1, 0x00,      						// Enable 3G [02]
            	ILI9341_GAMMASET, 			1, 0x01,      						// Gamma Set [01]
            	ILI9341_UNDEFINE0, 			15, 0x0f, 0x26, 0x24, 0x0b, 0x0e, 0x09, 0x54, 0xa8, 0x46, 0x0c, 0x17, 0x09, 0x0f, 0x07, 0x00,
            	ILI9341_UNDEFINE1, 			15, 0x00, 0x19, 0x1b, 0x04, 0x10, 0x07, 0x2a, 0x47, 0x39, 0x03, 0x06, 0x06, 0x30, 0x38, 0x0f,
				ILI9341_ENTRYMODE, 			1,0x07,
            	ILI9341_SLEEPOUT, 			0,            						// Sleep Out
            	TFTLCD_DELAY8, 				150,
            	ILI9341_DISPLAYON, 			0          							// Display On
            };
			init_table8(ILI9341_regValues, sizeof(ILI9341_regValues));
			break;
		case 0x9090:
			lcd_driver = ID_HX8357D;
			XC = ILI9341_COLADDRSET;
			YC = ILI9341_PAGEADDRSET;
			CC = HX8357_RAMWR;
			RC = HX8357_RAMRD;
			SC1 = 0x33;
			SC2  =0x37;
			MD = HX8357_MADCTL;
			VL = 1;
			R24BIT = 1;
			static const uint8_t HX8357D_regValues[] PROGMEM =
			{
  				HX8357_SWRESET, 	0,
  				HX8357D_SETC, 		3, 0xFF, 0x83, 0x57,
  				TFTLCD_DELAY8, 		250,
				HX8357_SETRGB, 		4, 0x00, 0x00, 0x06, 0x06,
				HX8357D_SETCOM, 	1, 0x25,  // -1.52V
  				HX8357_SETOSC, 		1, 0x68,  // Normal mode 70Hz, Idle mode 55 Hz
  				HX8357_SETPANEL, 	1, 0x05,  // BGR, Gate direction swapped
  				HX8357_SETPWR1, 	6, 0x00, 0x15, 0x1C, 0x1C, 0x83, 0xAA,
  				HX8357D_SETSTBA, 	6, 0x50, 0x50, 0x01, 0x3C, 0x1E, 0x08,
  				// MEME GAMMA HERE
  				HX8357D_SETCYC, 	7, 0x02, 0x40, 0x00, 0x2A, 0x2A, 0x0D, 0x78,
  				HX8357_COLMOD, 		1, 0x55,
  				HX8357_MADCTL, 		1, 0xC0,
  				HX8357_TEON, 		1, 0x00,
  				HX8357_TEARLINE, 	2, 0x00, 0x02,
  				HX8357_SLPOUT, 		0,
  				TFTLCD_DELAY8, 		150,
  				HX8357_DISPON, 		0,
  				TFTLCD_DELAY8, 		50
			};
			init_table8(HX8357D_regValues, sizeof(HX8357D_regValues));
			break;
		case 0x7575:
		case 0x9595:
			lcd_driver = ID_7575;
			XC = 0;
			YC = 0;
			CC = 0x22;
			RC = ILI932X_RW_GRAM;
			SC1 = 0x0E;
			SC2 = 0x14;
			MD = HX8347G_MEMACCESS;
			VL = 1;
			R24BIT = 1;
			static const uint8_t HX8347G_regValues[] PROGMEM =
			{
				// 0xEA, 2, 0x00, 0x20,		// PTBA[15:0]
				// 0xEC, 2, 0x0C, 0xC4,
				0x2E, 1, 0x89,
				0x29, 1, 0x8F,
				0x2B, 1, 0x02,
				0xE2, 1, 0x00,
				0xE4, 1, 0x01,
				0xE5, 1, 0x10,
				0xE6, 1, 0x01,
				0xE7, 1, 0x10,
				0xE8, 1, 0x70,
				0xF2, 1, 0x00,
				0xEA, 1, 0x00,
				0xEB, 1, 0x20,
				0xEC, 1, 0x3C,
				0xED, 1, 0xC8,
				0xE9, 1, 0x38,
				0xF1, 1, 0x01,
				// 0x40, 13, 0x01, 0x00, 0x00, 0x10, 0x0E, 0x24, 0x04, 0x50, 0x02, 0x13, 0x19, 0x19, 0x16,
				// 0x50, 14, 0x1B, 0x31, 0x2F, 0x3F, 0x3F, 0x3E, 0x2F, 0x7B, 0x09, 0x06, 0x06, 0x0C, 0x1D, 0xCC,
				// skip gamma, do later
				0x1B, 1, 0x1A,
				0x1A, 1, 0x01,
				0x24, 1, 0x61,
				0x25, 1, 0x5C,
				0x23, 1, 0x88,
				0x18, 1, 0x36,
				0x19, 1, 0x01,
				0x1F, 1, 0x88,
				TFTLCD_DELAY8, 5, // delay 5 ms
				0x1F, 1, 0x80,
				TFTLCD_DELAY8, 5,
				0x1F, 1, 0x90,
				TFTLCD_DELAY8, 5,
				0x1F, 1, 0xD4,
				TFTLCD_DELAY8, 5,
				0x17, 1, 0x05,
				0x36, 1, 0x00, //0x09
				0x28, 1, 0x38,
				TFTLCD_DELAY8, 40,
				0x28, 1, 0x3C,
				0x02, 1, 0x00,
				0x03, 1, 0x00,
				0x04, 1, 0x00,
				0x05, 1, 0xEF,
				0x06, 1, 0x00,
				0x07, 1, 0x00,
				0x08, 1, 0x01,
				0x09, 1, 0x3F
			};
		    init_table8(HX8347G_regValues, sizeof(HX8347G_regValues));
			break;
		case 0x9486:
			lcd_driver = ID_9486;
			XC = ILI9341_COLADDRSET;
			YC = ILI9341_PAGEADDRSET;
			CC = ILI9341_MEMORYWRITE;
			RC = HX8357_RAMRD;
			SC1 = 0x33;
			SC2 = 0x37;
			MD = ILI9341_MADCTL;
			VL = 0;
			R24BIT = 0;
			static const uint8_t ILI9486_regValues[] PROGMEM =
			{
				0xF1, 6,	0x36, 0x04, 0x00, 0x3C, 0x0F, 0x8F,
				0xF2, 9,	0x18, 0xA3, 0x12, 0x02, 0xB2, 0x12, 0xFF, 0x10, 0x00,
				0xF8, 2,	0x21, 0x04,
				0xF9, 2,	0x00, 0x08,
				0x36, 1,	0x08,
				0xB4, 1,	0x00,
				0xC1, 1,	0x41,
				0xC5, 4,	0x00, 0x91, 0x80, 0x00,
				0xE0, 15,	0x0F, 0x1F, 0x1C, 0x0C, 0x0F, 0x08, 0x48, 0x98, 0x37, 0x0A, 0x13, 0x04, 0x11, 0x0D, 0x00,
				0xE1, 15,	0x0F, 0x32, 0x2E, 0x0B, 0x0D, 0x05, 0x47, 0x75, 0x37, 0x06, 0x10 ,0x03, 0x24, 0x20, 0x00,
				0x3A, 1,	0x55,
				0x11, 0,
				0x36, 1,	0x28,
				TFTLCD_DELAY8, 120,
				0x29, 0
				/*
				0x01, 0,            				// Soft Reset
            	TFTLCD_DELAY8, 150,  				// .kbv will power up with ONLY reset, sleep out, display on
            	0x28, 0,            				// Display Off
            	0x3A, 1, 	0x55,      				// Pixel read=565, write=565.
            	0xC0, 2, 	0x0d, 0x0d,        		// Power Control 1 [0E 0E]
            	0xC1, 2, 	0x43, 0x00,        		// Power Control 2 [43 00]
            	0xC2, 1, 	0x00,      				// Power Control 3 [33]
            	0xC5, 4, 	0x00, 0x48, 0x00, 0x48, // VCOM  Control 1 [00 40 00 40]
            	0xB4, 1, 	0x00,      				// Inversion Control [00]
            	0xB6, 3, 	0x02, 0x02, 0x3B,  		// Display Function Control [02 02 3B]
            	0xE0, 15,	0x0F, 0x24, 0x1C, 0x0A, 0x0F, 0x08, 0x43, 0x88, 0x32, 0x0F, 0x10, 0x06, 0x0F, 0x07, 0x00,
            	0xE1, 15,	0x0F, 0x38, 0x30, 0x09, 0x0F, 0x0F, 0x4E, 0x77, 0x3C, 0x07, 0x10, 0x05, 0x23, 0x1B, 0x00,
            	0x11, 0,            				// Sleep Out
            	TFTLCD_DELAY8, 150,
            	0x29, 0         					// Display On
				*/
			};
			init_table8(ILI9486_regValues, sizeof(ILI9486_regValues));
			break;
		case 0x9488:
			lcd_driver = ID_9488;
			XC = ILI9341_COLADDRSET;
			YC = ILI9341_PAGEADDRSET;
			CC = ILI9341_MEMORYWRITE;
			RC = HX8357_RAMRD;
			SC1 = 0x33;
			SC2 = 0x37;
			MD = ILI9341_MADCTL;
			VL = 0;
			R24BIT = 1;
			static const uint8_t ILI9488_regValues[] PROGMEM =
			{
				0xF7, 4, 	0xA9, 0x51, 0x2C, 0x82,
				0xC0, 2, 	0x11, 0x09,
				0xC1, 1, 	0x41,
				0xC5, 3, 	0x00, 0x0A, 0x80,
				0xB1, 2, 	0xB0, 0x11,
				0xB4, 1, 	0x02,
				0xB6, 2, 	0x02, 0x22,
				0xB7, 1, 	0xC6,
				0xBE, 2, 	0x00, 0x04,
				0xE9, 1, 	0x00,
				0x36, 1, 	0x08,
				0x3A, 1, 	0x55,
				0xE0, 15, 	0x00, 0x07, 0x10, 0x09, 0x17, 0x0B, 0x41, 0x89, 0x4B, 0x0A, 0x0C, 0x0E, 0x18, 0x1B, 0x0F,
				0xE1, 15, 	0x00, 0x17, 0x1A, 0x04, 0x0E, 0x06, 0x2F, 0x45, 0x43, 0x02, 0x0A, 0x09, 0x32, 0x36, 0x0F,
				0x11, 0,
				TFTLCD_DELAY8, 120,
				0x29, 0
			};
			init_table8(ILI9488_regValues, sizeof(ILI9488_regValues));
			break;
		case 0x9481:
			lcd_driver = ID_9481;
			XC=ILI9341_COLADDRSET,YC=ILI9341_PAGEADDRSET,CC=ILI9341_MEMORYWRITE,RC=HX8357_RAMRD,SC1=0x33,SC2=0x37,MD=ILI9341_MADCTL,VL=0,R24BIT=0;
			static const uint8_t ILI9481_regValues[] PROGMEM =
			{
				0x11, 0,
				TFTLCD_DELAY8, 20,
				0xD0, 3, 0x07, 0x42, 0x18,
				0xD1, 3, 0x00, 0x07, 0x10,
				0xD2, 2, 0x01, 0x02,
				0xC0, 5, 0x10, 0x3B, 0x00, 0x02, 0x11,
				0xC5, 1, 0x03,
				0xC8, 12, 0x00, 0x32, 0x36, 0x45, 0x06, 0x16, 0x37, 0x75, 0x77, 0x54, 0x0C, 0x00,
				0x36, 1, 0x0A,
				0x3A, 1, 0x55,
				0x2A, 4, 0x00, 0x00, 0x01, 0x3F,
				0x2B, 4, 0x00, 0x00, 0x01, 0xE0,
				TFTLCD_DELAY8, 120,
				0x29, 0
			};
			init_table8(ILI9481_regValues, sizeof(ILI9481_regValues));
			break;
		case 0x7735:
			lcd_driver = ID_7735;
			XC = ILI9341_COLADDRSET;
			YC = ILI9341_PAGEADDRSET;
			CC = ILI9341_MEMORYWRITE;
			RC = HX8357_RAMRD;
			SC1 = 0x33;
			SC2 = 0x37;
			MD = ILI9341_MADCTL;
			VL = 0;
			R24BIT = 0;
			static const uint8_t ST7735S_regValues[] PROGMEM =
			{
				0x011, 0,            			// Soft Reset
            	TFTLCD_DELAY8, 120,  			// .kbv will power up with ONLY reset, sleep out, display on
            	0xB1, 3,	0x05, 0x3C, 0x3C,   // Display Off
            	0xB2, 3,	0x05, 0x3C, 0x3C,   // Pixel read=565, write=565.
            	0xB3, 6,	0x05, 0x3C, 0x3C, 0x05, 0x3C, 0x3C,        //Power Control 1 [0E 0E]
            	0xB4, 1,	0x03,         		// Power Control 2 [43 00]
            	0xC0, 3,	0x28, 0x08, 0x04,   // Power Control 3 [33]
            	0xC1, 1,	0xC0,     			// VCOM  Control 1 [00 40 00 40]
            	0xC2, 2,	0x0D, 0x00,      	// Inversion Control [00]
            	0xC3, 2,	0x8D, 0x2A,   		// Display Function Control [02 02 3B]
            	0xC4, 2,	0x8D, 0xEE,
            	0xC5, 1,	0x1A,
            	// 0x36, 1, 0xC0,
            	0xE0, 16,0x03, 0x22, 0x07, 0x0A, 0x2E, 0x30, 0x25, 0x2A, 0x28, 0x26, 0x2E, 0x3A, 0x00, 0x01, 0x03, 0x13,
            	0xE1, 16,0x04, 0x16, 0x06, 0x0D, 0x2D, 0x26, 0x23, 0x27, 0x27, 0x25, 0x2D, 0x3B, 0x00, 0x01, 0x04, 0x13,
            	0x3A, 1, 0x05,            		// Sleep Out
            	TFTLCD_DELAY8, 150,
            	0x29, 0         				// Display On
			};
			init_table8(ST7735S_regValues, sizeof(ST7735S_regValues));
			break;
		default:
			lcd_driver = ID_UNKNOWN;
			break;
	}
	setRotation(_rotation);
	invertDisplay(false);
}



//-----------------------------------------------
// Public API - setRotation
//-----------------------------------------------

void myLcdDevice::setRotation(uint8_t r)
{
    _rotation = r & 3;           // just perform the operation ourselves on the protected variables
    _width = (_rotation & 1) ? HEIGHT : WIDTH;
    _height = (_rotation & 1) ? WIDTH : HEIGHT;
	CS_ACTIVE;
	// prh - removed compiler warnings by settint val to zero in all cases below
	if (lcd_driver == ID_932X)
	{
		uint16_t val = 0;
		switch(_rotation)
		{
			default:
				val = 0x1030;  //0 degree
				break;
		 	case 1 :
				val = 0x1028;  //90 degree
				break;
		 	case 2 :
				val = 0x1000;  //180 degree
				break;
		 	case 3 :
				val = 0x1018;  //270 degree
				break;
		}
		writeCmdData16(MD, val);
	}
	else if (lcd_driver == ID_7735)
	{
		uint8_t val = 0;
		switch(_rotation)
		{
			case 0:
				val = 0xD0; //0 degree
				break;
		 	case 1:
				val = 0xA0; //90 degree
				break;
		 	case 2:
				val = 0x00; //180 degree
				break;
		 	case 3:
				val = 0x60; //270 degree
				break;
		}
		writeCmdData8(MD, val);
	}
	else if (lcd_driver == ID_9481)
	{
		uint8_t val = 0;
		switch (_rotation)
		{
		   	case 0:
		     	val = 0x09; //0 degree PAO=0,CAO=0,P/CO=0,VO=0,RGBO=1,DO=0,HF=0,VF=1
		     	break;
		   	case 1:
		     	val = 0x2B; //90 degree PAO=0,CAO=0,P/CO=1,VO=0,RGBO=1,DO=0,HF=1,VF=1
		     	break;
		 	case 2:
		    	val = 0x0A; //180 degree PAO=0,CAO=0,P/CO=0,VO=0,RGBO=1,DO=0,HF=1,VF=0
		    	break;
		   	case 3:
		     	val = 0x28; //270 degree PAO=0,CAO=0,P/CO=1,VO=0,RGBO=1,DO=0,HF=0,VF=0
		     	break;
		 }
		 writeCmdData8(MD, val);

	}
	else if (lcd_driver == ID_9486)
	{
		uint8_t val = 0;
		switch (_rotation)
		{
		   	case 0:
		     	val = ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR; //0 degree
		     	break;
		   	case 1:
		     	val = ILI9341_MADCTL_MV | ILI9341_MADCTL_ML | ILI9341_MADCTL_BGR ; //90 degree
		     	break;
		 	case 2:
		    	val = ILI9341_MADCTL_MY |ILI9341_MADCTL_BGR; //180 degree
		    	break;
		   	case 3:
		     	val = ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR; //270 degree
		     	break;
		 }
		 writeCmdData8(MD, val);
	}
	else if (lcd_driver == ID_9488)
	{
		uint8_t val = 0;
		switch (_rotation)
		{
			case 0:
		     	val = ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR ; //0 degree
		     	break;
		   	case 1:
		     	val = ILI9341_MADCTL_MV | ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR ; //90 degree
		     	break;
		 	case 2:
		    	val = ILI9341_MADCTL_ML | ILI9341_MADCTL_BGR; //180 degree
		    	break;
		   	case 3:
		     	val = ILI9341_MADCTL_MX | ILI9341_MADCTL_ML | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR; //270 degree
		     	break;
		 }
		 writeCmdData8(MD, val);
	}
	else
	{
		uint8_t val = 0;
		switch (_rotation)
		{
		   	case 0:
		     	val = ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR; //0 degree
		     	break;
		   	case 1:
		     	val = ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR; //90 degree
		     	break;
		 	case 2:
		    	val = ILI9341_MADCTL_MY | ILI9341_MADCTL_ML |ILI9341_MADCTL_BGR; //180 degree
		    	break;
		   	case 3:
		     	val = ILI9341_MADCTL_MX | ILI9341_MADCTL_MY| ILI9341_MADCTL_ML | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR; //270 degree
		     	break;
		 }
		 writeCmdData8(MD, val);
	}
 	setAddrWindow(0, 0, _width - 1, _height - 1);
	vertScroll(0, HEIGHT, 0);
	CS_IDLE;
}



//---------------------------------------------------------
// Virtual API
//---------------------------------------------------------

// virtual public
void myLcdDevice::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
	int16_t end;
	if (w < 0)
	{
        w = -w;
        x -= w;
    }                           // + ve w
    end = x + w;
    if (x < 0)
        x = 0;
    if (end > _width)
        end = _width;
    w = end - x;
    if (h < 0)
	{
        h = -h;
        y -= h;
    }                           // + ve h
    end = y + h;
    if (y < 0)
        y = 0;
    if (end > _height)
        end = _height;
    h = end - y;

    setAddrWindow(x, y, x + w - 1, y + h - 1);//set area
	CS_ACTIVE;
    if (lcd_driver == ID_932X)
		writeCmd8(ILI932X_START_OSC);
	writeCmd8(CC);
	if (h > w)
	{
        end = h;
        h = w;
        w = end;
    }
	while (h-- > 0)
	{
		end = w;
		do
		{
   			writeData16(color);//set color data
        } while (--end != 0);
	}
	if (lcd_driver == ID_932X)
		setAddrWindow(0, 0, _width - 1, _height - 1);
	else if (lcd_driver == ID_7575)
		setLR();

	CS_IDLE;

}	// fillRect()



// virtual
uint16_t myLcdDevice::color565(uint8_t r, uint8_t g, uint8_t b)
	// convert 8-bit R,G,B to 16-bit packed color
{
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
}


// virtual
void myLcdDevice::drawPixel(int16_t x, int16_t y, uint16_t color)
{
	if ((x < 0) || (y < 0) || (x > _width) || (y > _height))
		return;
	setAddrWindow(x, y, x, y);
	CS_ACTIVE;
	writeCmdData16(CC, color);
	CS_IDLE;
}


// virtual
void myLcdDevice::setAddrWindow(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
	CS_ACTIVE;
	if (lcd_driver == ID_932X)
	{
	    // Values passed are in current (possibly rotated) coordinate
	    // system.  932X requires hardware-native coords regardless of
	    // MADCTL, so rotate inputs as needed.  The address counter is
	    // set to the top-left corner -- although fill operations can be
	    // done in any direction, the current screen _rotation is applied
	    // because some users find it disconcerting when a fill does not
	    // occur top-to-bottom.
	    int x, y, t;
	    switch(_rotation)
		{
			default:
				x  = x1;
				y  = y1;
				break;
		    case 1:
				t  = y1;
				y1 = x1;
				x1 = WIDTH  - 1 - y2;
				y2 = x2;
				x2 = WIDTH  - 1 - t;
				x  = x2;
				y  = y1;
				break;
			case 2:
				t  = x1;
				x1 = WIDTH  - 1 - x2;
				x2 = WIDTH  - 1 - t;
				t  = y1;
				y1 = HEIGHT - 1 - y2;
				y2 = HEIGHT - 1 - t;
				x  = x2;
				y  = y2;
				break;
			case 3:
				t  = x1;
				x1 = y1;
				y1 = HEIGHT - 1 - x2;
				x2 = y2;
				y2 = HEIGHT - 1 - t;
				x  = x1;
				y  = y2;
				break;
    	}
		writeCmdData16(ILI932X_HOR_START_AD, x1); // Set address window
		writeCmdData16(ILI932X_HOR_END_AD, x2);
		writeCmdData16(ILI932X_VER_START_AD, y1);
		writeCmdData16(ILI932X_VER_END_AD, y2);
		writeCmdData16(ILI932X_GRAM_HOR_AD, x ); // Set address counter to top left
		writeCmdData16(ILI932X_GRAM_VER_AD, y );
 	}
	else if (lcd_driver == ID_7575)
	{
		writeCmdData8(HX8347G_COLADDRSTART_HI,x1>>8);
		writeCmdData8(HX8347G_COLADDRSTART_LO,x1);
		writeCmdData8(HX8347G_ROWADDRSTART_HI,y1>>8);
		writeCmdData8(HX8347G_ROWADDRSTART_LO,y1);
		writeCmdData8(HX8347G_COLADDREND_HI,x2>>8);
		writeCmdData8(HX8347G_COLADDREND_LO,x2);
		writeCmdData8(HX8347G_ROWADDREND_HI,y2>>8);
		writeCmdData8(HX8347G_ROWADDREND_LO,y2);
	}
	else
	{
		#if  1	// prh mods to eliminate warnings
			uint8_t x_buf[4];
			x_buf[0] = x1>>8;
			x_buf[1] = x1&0xFF;
			x_buf[2] = x2>>8;
			x_buf[3] = x2&0xFF;

			uint8_t y_buf[4];
			y_buf[0] = y1>>8;
			y_buf[1] = y1&0xFF;
			y_buf[2] = y2>>8;
			y_buf[3] = y2&0xFF;
		#else
			uint8_t x_buf[] = {x1>>8,x1&0xFF,x2>>8,x2&0xFF};
			uint8_t y_buf[] = {y1>>8,y1&0xFF,y2>>8,y2&0xFF};
		#endif

		Push_Command(XC, x_buf, 4); //set x address
		Push_Command(YC, y_buf, 4); //set y address
	}
	CS_IDLE;
}	// setAddrWindow()



// virtual
void myLcdDevice::pushAnyColor(uint16_t * block, int16_t n, bool first, uint8_t flags)
	// push block of 16 bit colors
{
	uint16_t color;
    // uint8_t h, l;     // prh - teensy mods remove unused var warnings
	bool isconst = flags & 1;
	// bool isbigend = (flags & 2) != 0;
    CS_ACTIVE;
    if (first)
	{
		if (lcd_driver == ID_932X)
		{
			writeCmd8(ILI932X_START_OSC);
		}
		writeCmd8(CC);
    }
    while (n-- > 0)
	{
        if (isconst)
		{
			color = pgm_read_word(block++);
        }
		else
		{
			color = (*block++);
		}
        writeData16(color);
    }
    CS_IDLE;
}	// pushAnyColor()


// virtual
int16_t myLcdDevice::readRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *block)
{
	// prh - teensy mods remove unused var warnings
	// uint16_t ret, dummy;
    // int16_t n = w * h;
    // uint8_t r, g, b, tmp;
	uint16_t ret = 0;
    // int16_t n = w * h;
    uint32_t n = w * h;		// prh
    uint8_t r, g, b;

    setAddrWindow(x, y, x + w - 1, y + h - 1);

	while (n > 0)
	{
        CS_ACTIVE;
		writeCmd16(RC);
        setReadDir();
		if (lcd_driver == ID_932X)
		{
			while(n)
			{
				for(int i =0; i< 2; i++)
				{
					read8(r);
					read8(r);
					read8(r);
					read8(g);
				}
				*block++ = (r<<8 | g);
				n--;
			}
			setAddrWindow(0, 0, _width - 1, _height - 1);
		}
		else
		{
			read8(r);
        	while (n)
			{
				if (R24BIT == 1)
				{
        			read8(r);
         			read8(g);
        			read8(b);
            		ret = color565(r, g, b);
				}
				else if (R24BIT == 0)	// prh could if (R24BIT) and else
				{
					read16(ret);
				}
            	*block++ = ret;
            	n--;
        	}
        }
		// RD_IDLE;
        CS_IDLE;
        setWriteDir();
    }
	return 0;
}


//-------------------------------------------
// Other Internal Methods
//-------------------------------------------

void myLcdDevice::setLR(void)
	// Unlike the 932X drivers that set the address window to the full screen
	// by default (using the address counter for drawPixel operations), the
	// 7575 needs the address window set on all graphics operations.  In order
	// to save a few register writes on each pixel drawn, the lower-right
	// corner of the address window is reset after most fill operations, so
	// that drawPixel only needs to change the upper left each time.
{
	CS_ACTIVE;
	writeCmdData8(HX8347G_COLADDREND_HI,(_width -1)>>8);
	writeCmdData8(HX8347G_COLADDREND_LO,_width -1);
	writeCmdData8(HX8347G_ROWADDREND_HI,(_height -1)>>8);
	writeCmdData8(HX8347G_ROWADDREND_LO,_height -1);
	CS_IDLE;
}


void myLcdDevice::invertDisplay(boolean i)
{
	CS_ACTIVE;
	uint8_t val = VL^i;
	if (lcd_driver == ID_932X)
	{
		writeCmdData8(0x61, val);
	}
	else if (lcd_driver == ID_7575)
	{
		writeCmdData8(0x01, val ? 8 : 10);
	}
	else
	{
		writeCmd8(val ? 0x21 : 0x20);
	}
	CS_IDLE;
}


void myLcdDevice::vertScroll(int16_t top, int16_t scrollines, int16_t offset)
{
    int16_t bfa = HEIGHT - top - scrollines;
    int16_t vsp;
	// prh mods remove unused var warnings
    // int16_t sea = top;
    if (offset <= -scrollines || offset >= scrollines)
		offset = 0; //valid scroll

	vsp = top + offset; // vertical start position
    if (offset < 0)
        vsp += scrollines;          //keep in unsigned range

	// prh mods remove unused var warnings
    // sea = top + scrollines - 1;
	if (lcd_driver == ID_932X)
	{
		writeCmdData8(SC1, (1 << 1) | 0x1);        //!NDL, VLE, REV
        writeCmdData8(SC2, vsp);        //VL#
	}
	else
	{
  		uint8_t d[6];           // for multi-byte parameters
  		d[0] = top >> 8;        //TFA
  		d[1] = top;
  		d[2] = scrollines >> 8; //VSA
  		d[3] = scrollines;
  		d[4] = bfa >> 8;        //BFA
  		d[5] = bfa;
		Push_Command(SC1, d, 6);
		d[0] = vsp >> 8;        //VSP
  		d[1] = vsp;
		Push_Command(SC2, d, 2);
		if (lcd_driver == ID_7575)
		{
			d[0] = (offset != 0) ? 0x08:0;
			Push_Command(0x01, d, 1);
		}
		else if (offset == 0)
		{
			Push_Command(0x13, NULL, 0);
		}
	}
}	// vertScroll()





// prh addition = added myLcdDevice::dimScreen() method

#define NUM_BUF_LINES   1
	// must divide evenly into 320
	// and I had to change an int16_t to a uint32_t in Read_DGRAM
	// and it's too slow, and clunky with NUM_BUF_LINES>1
	// and it doesn't restore colors correctly
	// and it takes about 80% of the teensy memory at NUM_BUF_LINES=160
	//
	// So maybe dialogs that pop up over the rest of the program should be
	// distinctly different, like black on white or black on yellow writing.
	// ....

uint16_t line_buf[480 * NUM_BUF_LINES];

void myLcdDevice::dimScreen()		// set all pixels to half their value
{
	static bool toggle = false;

	for (int y=0; y<320; y += NUM_BUF_LINES)
	{
		readRect(0,y,480,NUM_BUF_LINES,line_buf);

		setAddrWindow(0, y, 479, y+NUM_BUF_LINES-1);
		CS_ACTIVE;
		writeCmd8(CC);

		for (uint32_t i=0; i<480 * NUM_BUF_LINES; i++)
		{
			// color FROM 565
			// the bits are as follows and get moved to the high order bits of the R G B bytes
			//      15 14 13 12 11       10 9 8 7 6 5         4 3 2 1 0
			//       7  6  5  4  3 210   7  6 5 4 3 2 10      7 6 5 4 3 210

			uint16_t color = line_buf[i];
			int r = (color >> 8) & 0xF8;
			int g = (color >> 3) & 0xFC;
			int b = (color << 4) * 0xF8;

			uint16_t new_color = toggle ?
				color565(r*4,g*4,b*4) :
				color565(r/4,g/4,b/4);

			writeData16(new_color);
		}
	}

	CS_IDLE;
	toggle = !toggle;

}	// dimScreen()


//-------------------------------------------------
// Primitives
//-------------------------------------------------


void myLcdDevice::Push_Command(uint16_t cmd, uint8_t *block, int8_t N)
	// Write a command and N datas
{
  	CS_ACTIVE;
    writeCmd16(cmd);
    while (N-- > 0)
	{
        uint8_t u8 = *block++;
        writeData8(u8);
		if (N && (lcd_driver == ID_7575))
		{
			cmd++;
			writeCmd16(cmd);
		}
    }
    CS_IDLE;
}


void myLcdDevice::pushAnyColor(uint8_t * block, int16_t n, bool first, uint8_t flags)
	// push block of 8 bit colors
{
	uint16_t color;
    uint8_t h, l;
	bool isconst = flags & 1;
	bool isbigend = (flags & 2) != 0;
    CS_ACTIVE;
    if (first)
	{
		if (lcd_driver == ID_932X)
		{
			writeCmd8(ILI932X_START_OSC);
		}
		writeCmd8(CC);
    }
    while (n-- > 0)
	{
        if (isconst)
		{
            h = pgm_read_byte(block++);
            l = pgm_read_byte(block++);
        }
		else
		{
		    h = (*block++);
            l = (*block++);
		}
        color = (isbigend) ? (h << 8 | l) :  (l << 8 | h);
        writeData16(color);
    }
    CS_IDLE;
}


//--------------------------------------
// ReadXXX methods
//--------------------------------------

uint16_t myLcdDevice::readID(void)
{
	uint32_t ret = 0;

	if ((readReg(0x04,0) == 0x00) && (readReg(0x04,1) == 0x8000))
	{
		display(dbg_ld,"readID() got 0x00 0x8000",0);

		uint8_t buf[] = {0xFF, 0x83, 0x57};
		Push_Command(HX8357D_SETC, buf, sizeof(buf));

		ret = readReg(0xD0,0);
		ret <<= 16;				// this shift cannot be correct
		ret = readReg(0xD0,1);
		if ((ret == 0x990000) || (ret == 0x900000))
		{
			display(dbg_ld,"readID() got 0x9090",0);
			ret = 0x9090;
		}
	}
	if (ret != 0x9090)
		ret = readReg(0xD3,1); //0x9341 0x9486

	if (ret != 0x9341 &&
		ret != 0x9486 &&
		ret != 0x9488)
		ret = readReg(0, 0); //others

	display(dbg_ld,"readID()=0x%08x",ret);

	// return low 32 bits
	return ret;
}


uint16_t myLcdDevice::readReg(uint16_t reg, int8_t index)
{
	// prh - teensy mods remove unused var warnings
	// uint16_t ret,high;
    // uint8_t low;
	uint16_t ret = 0;
	// if (!have_reset)
	// {
	//     reset();
	// }
	CS_ACTIVE;
    writeCmd16(reg);
    setReadDir();
    delay(1);
	do
	{
		// read8(high);
		// read8(low);
		// ret = (high << 8) | lowc
		read16(ret);  //read 16bits
	} while (--index >= 0);
	// RD_IDLE;
    CS_IDLE;
    setWriteDir();
    return ret;
}
