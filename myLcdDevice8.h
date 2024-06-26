//----------------------------------------------
// myLcdDevice8.h
//----------------------------------------------
#pragma once

// if using 8bit mode on Mega2560 and the data pin is from 22 to 29, uncomment the below macro definition
// if using 8bit mode on UNO and the data pin is from 2 to 9, comment the below macro definition

// #define USE_8BIT_SHIELD_ON_MEGA


#if __LCD_TEENSY__	// teensy specific

	// implementations of following methods in myLcdDeviceTeensy.cpp

    extern void  	setReadDir();
    extern void  	setWriteDir();
    extern uint8_t  _read8();
    extern void 	_write8(uint8_t v);

	#define DELAY_MIN // delayMicroseconds(2)
		// 2023-08-17 - I was getting blanks in lines in teensyExpression
		// and determined empirically that I needed a delay between WR_ACTIVE
		// and WR_IDLE, so took out the call to the WR_STROBE macro, put the
		// WR_ACTIVE and WR_IDLE directly into the write() macro and messed
		// with delay. 2 microseconds seems to be the minimum that worked.
		//
		// 2023-08-21 - I now think it's that the timer_handler() is the problem.

    #define write8(v)   { _write8(v); WR_ACTIVE; DELAY_MIN; WR_IDLE; }	// WR_STROBE; }
    #define read8(v)    { RD_ACTIVE; DELAYN; *(&v) = _read8(); RD_IDLE; }

    #define DELAYN     // delayMicroseconds(7);	// was DELAY7
		// Just to be sure, I made a new define DELAYN and then
		// determined this delay was not needed.

    #define RD_ACTIVE  digitalWrite(_rd, 0)    // *rdPort &=  rdPinUnset
    #define RD_IDLE    digitalWrite(_rd, 1)    // *rdPort |=  rdPinSet
    #define WR_ACTIVE  digitalWrite(_wr, 0)    // *wrPort &=  wrPinUnset
    #define WR_IDLE    digitalWrite(_wr, 1)    // *wrPort |=  wrPinSet
    #define CD_COMMAND digitalWrite(_cd, 0)    // *cdPort &=  cdPinUnset
    #define CD_DATA    digitalWrite(_cd, 1)    // *cdPort |=  cdPinSet
    #define CS_ACTIVE  digitalWrite(_cs, 0)    // *csPort &=  csPinUnset
    #define CS_IDLE    digitalWrite(_cs, 1)    // *csPort |=  csPinSet

#else   // prh - original stuff

	#define DELAY7        \
		asm volatile(       \
		"rjmp .+0" "\n\t" \
		"rjmp .+0" "\n\t" \
		"rjmp .+0" "\n\t" \
		"nop"      "\n"   \
		::);

	#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined (__AVR_ATmega328__) || defined(__AVR_ATmega8__)

		#ifdef USE_ADAFRUIT_SHIELD_PIN

			// Arduino Uno, Duemilanove, etc.

			// LCD control lines:
			// RD (read), WR (write), CD (command/data), CS (chip select)
			#define RD_PORT PORTC				/*pin A0 */
			#define WR_PORT PORTC				/*pin A1 */
			#define CD_PORT PORTC				/*pin A2 */
			#define CS_PORT PORTC				/*pin A3 */
			#define RD_MASK B00000001
			#define WR_MASK B00000010
			#define CD_MASK B00000100
			#define CS_MASK B00001000

			#define BMASK 0x2F
			#define DMASK 0xD0

			// These are macros for I/O operations...

			// Write 8-bit value to LCD data lines
			#define write8(d) { PORTD = (PORTD & ~DMASK) | ((d) & DMASK); PORTB = (PORTB & ~BMASK) | ((d) & BMASK); WR_STROBE; }
			// #define write16(d) {uint8_t h = (d)>>8, l = d; write8(d); write8(l);}

			// Read 8-bit value from LCD data lines.  The signle argument
			// is a destination variable; this isn't a function and doesn't
			// return a value in the conventional sense.
			#define read8(dst) { RD_ACTIVE; DELAY7; dst = (PIND & DMASK) | (PINB & BMASK); RD_IDLE; }
			// #define read16(dst) { uint8_t hi; read8(hi); read8(dst); dst |= (hi << 8); }


			// These set the PORT directions as required before the write and read
			// operations.  Because write operations are much more common than reads,
			// the data-reading functions in the library code set the PORT(s) to
			// input before a read, and restore them back to the write state before
			// returning.  This avoids having to set it for output inside every
			// drawing method.  The default state has them initialized for writes.
			#define setWriteDir() { DDRD |=  DMASK; DDRB |=  BMASK; }
			#define setReadDir()  { DDRD &= ~DMASK; DDRB &= ~BMASK; }

		#else // Uno w/Breakout board

			#define BMASK         0x03
			#define DMASK         0xFC
			#define write8(d) { PORTD = (PORTD & ~DMASK) | ((d) & DMASK); PORTB = (PORTB & ~BMASK) | ((d) & BMASK); WR_STROBE; }
			//  #define write16(d) {uint8_t h = (d)>>8, l = d; write8(d); write8(l);}
			#define read8(dst) { RD_ACTIVE; DELAY7; dst = (PIND & DMASK) | (PINB & BMASK); RD_IDLE; }
			//  #define read16(dst) { uint8_t hi; read8(hi); read8(dst); dst |= (hi << 8); }

			#define setWriteDir() { DDRD |=  DMASK; DDRB |=  BMASK; }
			#define setReadDir()  { DDRD &= ~DMASK; DDRB &= ~BMASK; }

		#endif

	#elif defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2561__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)

		#ifdef USE_ADAFRUIT_SHIELD_PIN

			// Arduino Mega, ADK, etc.

			#define RD_PORT PORTF
			#define WR_PORT PORTF
			#define CD_PORT PORTF
			#define CS_PORT PORTF
			#define RD_MASK B00000001
			#define WR_MASK B00000010
			#define CD_MASK B00000100
			#define CS_MASK B00001000

			#define BMASK         0xB0
			#define GMASK         0x20
			#define HMASK         0x78

			#define write8(d) { \
				PORTH = (PORTH&~HMASK)|(((d)&(0x03<<6)>>3)|(((d)&0x03)<<5); \
				PORTB = (PORTB&~BMASK)|(((d)&((0x01<<5)|(0x03<<2)))<<2); \
				PORTG = (PORTG&~HMASK)|(((d)&(0x01<<4)<<1); WR_STROBE; }

			//  #define write16(d) { uint8_t h = (x)>>8, l = x; write8(h); write8(l); }

			#define read8(dst) { \
				RD_ACTIVE; \
				DELAY7; \
				dst = ((PINH & (0x03<<3)) << 3) | ((PINB & BMASK) >> 2) | \
				((PING & GMASK) >> 1) | ((PINH & (0x03<<5) >> 5); RD_IDLE; }

			//  #define read16(dst) { uint8_t hi; read8(hi); read8(dst); dst |= (hi << 8); }

			#define setWriteDir() { DDRH |=  HMASK; DDRB |=  BMASK; DDRG |=  GMASK; }
			#define setReadDir()  { DDRH &= ~HMASK; DDRB &= ~BMASK; DDRG &= ~GMASK; }

		#else // Mega w/Breakout board

			#ifndef USE_8BIT_SHIELD_ON_MEGA

				#define EMASK         0x38
				#define GMASK         0x20
				#define HMASK         0x78

				#define write8(d) { \
					PORTH &= ~HMASK; \
					PORTH |= ((d&(0x03<<6))>>3) | ((d&0x03) << 5); \
					PORTE &= ~EMASK; \
					PORTE |= ((d & (0x03<<2)) << 2) | ((d & (0x01<<5)) >> 2); \
					PORTG &= ~GMASK; \
					PORTG |= (d & (0x01<<4)) << 1; WR_STROBE; }

				//  #define write16(d) { uint8_t h = (x)>>8, l = x; write8(h); write8(l); }

				#define read8(dst) { \
					RD_ACTIVE; DELAY7; \
					dst = (PINH & (0x03<<5)) >> 5; \
					dst |= (PINH & (0x03<<3)) << 3; \
					dst |= (PINE & (0x01<<3)) << 2; \
					dst |= (PINE & (0x03<<4)) >> 2; \
					dst |= (PING & (0x01<<5)) >> 1;RD_IDLE;}

				//  #define read16(dst) { uint8_t hi; read8(hi); read8(dst); dst |= (hi << 8); }

				#define setWriteDir() { DDRH |= HMASK;DDRE |= EMASK;DDRG |= GMASK; }
				#define setReadDir()  { DDRH &= ~HMASK;DDRE &= ~EMASK;DDRG &= ~(GMASK); }

			#else

				#define AMASK    0xFF

				#define write8(d) 		{ PORTA = d; WR_STROBE; }
				#define read8(dst) 		{ RD_ACTIVE; DELAY7; dst = PINA;RD_IDLE;}
				#define setWriteDir() 	{DDRA |= AMASK;}
				#define setReadDir()  	{DDRA &= ~AMASK;}

			#endif

		#endif

	#elif defined(__AVR_ATmega32U4__)

		// Arduino Leonardo

		#ifdef USE_ADAFRUIT_SHIELD_PIN

			#define RD_PORT		PORTF
			#define WR_PORT		PORTF
			#define CD_PORT		PORTF
			#define CS_PORT		PORTF
			#define RD_MASK		B10000000
			#define WR_MASK		B01000000
			#define CD_MASK		B00100000
			#define CS_MASK		B00010000

			#define BMASK 		0xF0
			#define CMASK 		0x80
			#define DMASK 		0x90
			#define EMASK 		0x40

			// #define write16(d) { uint8_t h = (x)>>8, l = x; write8(h); write8(l); }
			// #define read16(dst) { uint8_t hi; read8(hi); read8(dst); dst |= (hi << 8); }


			#define write8(d) { \
				PORTE = (PORTE & ~EMASK) | (((d) & (0x1<<7))>>1); \
				PORTD = (PORTD & ~DMASK) | (((d) & (0x1<<6))<<1) | ((d) & (0x1<<4)); \
				PORTC = (PORTC & ~CMASK) | (((d) & (0x1<<5))<<2); \
				PORTB = (PORTB & ~BMASK) | (((d) & ~BMASK)<<4); \
				WR_STROBE; }

			#define read8(dst) {  \
				RD_ACTIVE; \
				DELAY7; \
				dst = ((PINE & EMASK) << 1) | ((PIND & (0x1<<7)) >> 1) | \
				((PINC & CMASK) >> 2) | ((PINB & BMASK) >> 4) | \
				(PIND & (1<<4));RD_IDLE; }

			#define setWriteDir() { DDRE |=  EMASK; DDRD |=  DMASK; DDRC |=  CMASK; DDRB |=  BMASK; }
			#define setReadDir()  { DDRE &= ~EMASK; DDRD &= ~DMASK; DDRC &= ~CMASK; DDRB &= ~BMASK; }

		#else // Leonardo w/Breakout board

			#define BMASK 	0x30
			#define CMASK 	0x40
			#define DMASK 	0x93
			#define EMASK 	0x40

			// #define write16(d) { uint8_t h = (x)>>8, l = x; write8(h); write8(l); }
			// #define read16(dst) { uint8_t hi; read8(hi); read8(dst); dst |= (hi << 8); }

			#define write8(d) { \
				uint8_t dr1 = (d) >> 1, dl1 = (d) << 1; \
				PORTE = (PORTE & ~EMASK) | (dr1 & (0x1<<6)); \
				PORTD = (PORTD & ~DMASK) | (dl1 & (0x1<<7)) | (((d) & (0x1<<3))>>3) | (dr1 & (0x1<<1)) |  ((d) & (0x1<<4)); \
				PORTC = (PORTC & ~CMASK) | (dl1 & (0x1<<6));   \
				PORTB = (PORTB & ~BMASK) |(((d) & 0x3)<<4); WR_STROBE; }

			#define read8(dst) {  \
				RD_ACTIVE; \
				DELAY7; \
				dst = (((PINE & EMASK) | (PIND & (0x1<<1)) << 1) | \
				(((PINC & CMASK) | (PIND & (0x1<<7)) >> 1) | \
				((PIND & 0x1) << 3) | ((PINB & BMASK) >> 4) | \
				(PIND & (0x1<<4)); RD_IDLE; }

			#define setWriteDir() { DDRE |=  EMASK; DDRD |=  DMASK; DDRC |=  CMASK; DDRB |=  BMASK; }
			#define setReadDir()  { DDRE &= ~EMASK; DDRD &= ~DMASK; DDRC &= ~CMASK; DDRB &= ~BMASK; }

		#endif

	#elif defined(__SAM3X8E__)

		// Arduino Due

		#ifdef USE_ADAFRUIT_SHIELD_PIN

			#define RD_PORT PIOA				/*pin A0 */
			#define WR_PORT PIOA				/*pin A1 */
			#define CD_PORT PIOA				/*pin A2 */
			#define CS_PORT PIOA				/*pin A3 */
			#define RD_MASK 0x00010000
			#define WR_MASK 0x01000000
			#define CD_MASK 0x00800000
			#define CS_MASK 0x00400000

			// #define write16(d) { uint8_t h = (x)>>8, l = x; write8(h); write8(l); }
			// #define read16(dst) { uint8_t hi; read8(hi); read8(dst); dst |= (hi << 8); }

			#define write8(d) { \
				PIO_Set(PIOD, (((d) & 0x08)<<(7-3))); \
				PIO_Clear(PIOD, (((~d) & 0x08)<<(7-3))); \
				PIO_Set(PIOC, (((d) & 0x01)<<(22-0)) | (((d) & 0x02)<<(21-1))| (((d) & 0x04)<<(29-2))| (((d) & 0x10)<<(26-4))| (((d) & 0x40)<<(24-6))| (((d) & 0x80)<<(23-7))); \
				PIO_Clear(PIOC, (((~d) & 0x01)<<(22-0)) | (((~d) & 0x02)<<(21-1))| (((~d) & 0x04)<<(29-2))| (((~d) & 0x10)<<(26-4))| (((~d) & 0x40)<<(24-6))| (((~d) & 0x80)<<(23-7))); \
				PIO_Set(PIOB, (((d) & 0x20)<<(27-5))); \
				PIO_Clear(PIOB, (((~d) & 0x20)<<(27-5))); \
				WR_STROBE; }

			#define read8(result) { \
				RD_ACTIVE; \
				delayMicroseconds(1); \
				result = (((PIOC->PIO_PDSR & (1<<23)) >> (23-7)) | ((PIOC->PIO_PDSR & (1<<24)) >> (24-6)) | \
				((PIOB->PIO_PDSR & (1<<27)) >> (27-5)) | ((PIOC->PIO_PDSR & (1<<26)) >> (26-4)) | \
				((PIOD->PIO_PDSR & (1<< 7)) >> ( 7-3)) | ((PIOC->PIO_PDSR & (1<<29)) >> (29-2)) | \
				((PIOC->PIO_PDSR & (1<<21)) >> (21-1)) | ((PIOC->PIO_PDSR & (1<<22)) >> (22-0))); \
				RD_IDLE;}

			#define setWriteDir() { \
				PIOD->PIO_MDDR |=  0x00000080; /*PIOD->PIO_SODR =  0x00000080;*/ PIOD->PIO_OER |=  0x00000080; PIOD->PIO_PER |=  0x00000080; \
				PIOC->PIO_MDDR |=  0x25E00000; /*PIOC->PIO_SODR =  0x25E00000;*/ PIOC->PIO_OER |=  0x25E00000; PIOC->PIO_PER |=  0x25E00000; \
				PIOB->PIO_MDDR |=  0x08000000; /*PIOB->PIO_SODR =  0x08000000;*/ PIOB->PIO_OER |=  0x08000000; PIOB->PIO_PER |=  0x08000000; }

			#define setReadDir() { \
				pmc_enable_periph_clk( ID_PIOD ) ;	  pmc_enable_periph_clk( ID_PIOC ) ;	  pmc_enable_periph_clk( ID_PIOB ) ; \
				PIOD->PIO_PUDR |=  0x00000080; PIOD->PIO_IFDR |=  0x00000080; PIOD->PIO_ODR |=  0x00000080; PIOD->PIO_PER |=  0x00000080; \
				PIOC->PIO_PUDR |=  0x25E00000; PIOC->PIO_IFDR |=  0x25E00000; PIOC->PIO_ODR |=  0x25E00000; PIOC->PIO_PER |=  0x25E00000; \
				PIOB->PIO_PUDR |=  0x08000000; PIOB->PIO_IFDR |=  0x08000000; PIOB->PIO_ODR |=  0x08000000; PIOB->PIO_PER |=  0x08000000; }

			// Control signals are ACTIVE LOW (idle is HIGH)
			// Command/Data: LOW = command, HIGH = data
			// These are single-instruction operations and always inline

			#define RD_ACTIVE  RD_PORT->PIO_CODR = RD_MASK
			#define RD_IDLE    RD_PORT->PIO_SODR = RD_MASK
			#define WR_ACTIVE  WR_PORT->PIO_CODR = WR_MASK
			#define WR_IDLE    WR_PORT->PIO_SODR = WR_MASK
			#define CD_COMMAND CD_PORT->PIO_CODR = CD_MASK
			#define CD_DATA    CD_PORT->PIO_SODR = CD_MASK
			#define CS_ACTIVE  CS_PORT->PIO_CODR = CS_MASK
			#define CS_IDLE    CS_PORT->PIO_SODR = CS_MASK

		#else // Due w/Breakout board

			// #define write16(d) { uint8_t h = (x)>>8, l = x; write8(h); write8(l); }
			//#define read16(dst) { uint8_t hi; read8(hi); read8(dst); dst |= (hi << 8); }

			#define write8(d) 		{ PIO_Set(PIOC, (((d) & 0xFF)<<1)); PIO_Clear(PIOC, (((~d) & 0xFF)<<1)); WR_STROBE; }
			#define read8(result) 	{ RD_ACTIVE; delayMicroseconds(1); result = ((PIOC->PIO_PDSR & 0x1FE) >> 1); RD_IDLE;}
			#define setWriteDir() 	{ PIOC->PIO_MDDR |=  0x000001FE; /*PIOC->PIO_SODR |=  0x000001FE;*/ PIOC->PIO_OER |=  0x000001FE; PIOC->PIO_PER |=  0x000001FE; }
			#define setReadDir() 	{ pmc_enable_periph_clk( ID_PIOC ) ; PIOC->PIO_PUDR |=  0x000001FE; PIOC->PIO_IFDR |=  0x000001FE; PIOC->PIO_ODR |=  0x000001FE; PIOC->PIO_PER |=  0x000001FE; }

			// When using the TFT breakout board, control pins are configurable.

			#define RD_ACTIVE	rdPort->PIO_CODR = rdPinSet		//PIO_Clear(rdPort, rdPinSet)
			#define RD_IDLE		rdPort->PIO_SODR = rdPinSet		//PIO_Set(rdPort, rdPinSet)
			#define WR_ACTIVE	wrPort->PIO_CODR = wrPinSet		//PIO_Clear(wrPort, wrPinSet)
			#define WR_IDLE		wrPort->PIO_SODR = wrPinSet		//PIO_Set(wrPort, wrPinSet)
			#define CD_COMMAND	cdPort->PIO_CODR = cdPinSet		//PIO_Clear(cdPort, cdPinSet)
			#define CD_DATA		cdPort->PIO_SODR = cdPinSet		//PIO_Set(cdPort, cdPinSet)
			#define CS_ACTIVE	csPort->PIO_CODR = csPinSet		//PIO_Clear(csPort, csPinSet)
			#define CS_IDLE		csPort->PIO_SODR = csPinSet		//PIO_Set(csPort, csPinSet)

		#endif

	#else
		#error "Board type unsupported / not recognized"
	#endif

	//-----------------------------------------------------

	#if !defined(__SAM3X8E__)

		// Stuff common to all Arduino AVR board types:

		#ifdef USE_ADAFRUIT_SHIELD_PIN

			// Control signals are ACTIVE LOW (idle is HIGH)
			// Command/Data: LOW = command, HIGH = data
			// These are single-instruction operations and always inline

			#define RD_ACTIVE  RD_PORT &= ~RD_MASK
			#define RD_IDLE    RD_PORT |=  RD_MASK
			#define WR_ACTIVE  WR_PORT &= ~WR_MASK
			#define WR_IDLE    WR_PORT |=  WR_MASK
			#define CD_COMMAND CD_PORT &= ~CD_MASK
			#define CD_DATA    CD_PORT |=  CD_MASK
			#define CS_ACTIVE  CS_PORT &= ~CS_MASK
			#define CS_IDLE    CS_PORT |=  CS_MASK

		#else // Breakout board

			// When using the TFT breakout board, control pins are configurable.

			#define RD_ACTIVE  *rdPort &=  rdPinUnset
			#define RD_IDLE    *rdPort |=  rdPinSet
			#define WR_ACTIVE  *wrPort &=  wrPinUnset
			#define WR_IDLE    *wrPort |=  wrPinSet
			#define CD_COMMAND *cdPort &=  cdPinUnset
			#define CD_DATA    *cdPort |=  cdPinSet
			#define CS_ACTIVE  *csPort &=  csPinUnset
			#define CS_IDLE    *csPort |=  csPinSet

		#endif	// !Breakout board
	#endif	// !defined(__SAM3X8E__)
#endif  // ! prh __LCD_TEENSY__


// Data write strobe, ~2 instructions and always inline

#define WR_STROBE 		{ WR_ACTIVE; WR_IDLE; }
#define RD_STROBE 		{ RD_IDLE; RD_ACTIVE;RD_ACTIVE;RD_ACTIVE; }
#define write16(d) 		{ uint8_t h = (d)>>8, l = d; write8(h); write8(l); }
#define read16(dst) 	{ uint8_t hi; read8(hi); read8(dst); dst |= (hi << 8); }
#define writeCmd8(x)	{ CD_COMMAND; write8(x); CD_DATA;  }
#define writeData8(x)	{ write8(x) }
#define writeCmd16(x)	{ CD_COMMAND; write16(x); CD_DATA; }
#define writeData16(x)	{ write16(x) }


// These higher-level operations are usually functionalized,
// except on Mega where's there's gobs and gobs of program space.

// Set value of TFT register: 8-bit address, 8-bit value

#define writeCmdData8(a, d) { CD_COMMAND; write8(a); CD_DATA; write8(d); }

// Set value of TFT register: 16-bit address, 16-bit value
// See notes at top about macro expansion, hence hi & lo temp vars

#define writeCmdData16(a, d) { \
	uint8_t hi, lo; \
	hi = (a) >> 8; lo = (a); CD_COMMAND; write8(hi); write8(lo); \
	hi = (d) >> 8; lo = (d); CD_DATA   ; write8(hi); write8(lo); }


// end of myLcdDevice8.h
