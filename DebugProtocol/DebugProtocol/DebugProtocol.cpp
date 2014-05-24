// DebugProtocol.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h> 
#define NON_STANDART_MODEM 1

typedef unsigned short int UWORD; 

void nop(void)
{
}
typedef struct BTUnit
{
    FILE *ComOut;
    unsigned char EEPROM[512];

    unsigned char eeprom_read(unsigned char addr)
    {
        return(EEPROM[addr]);
    };
    void eeprom_write(unsigned char addr, unsigned char value)
    {
        EEPROM[addr] = value;
    };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#include "..\..\Var001.H"
//// [1 GYRO]->[2 MEM]-> [3 POW] -> [4 STM] -> [5 BT] -| 
//  A                                                |
//  --------------------------------------------------
#define MY_UNIT '5' 

#define BT_TIMER1 1
#define BT_TIMER3 1

//// for 18F2321
////#define DELAY_BTW_NEXT_DIAL 0xe001
//// for 18F25K20
//#define DELAY_BTW_NEXT_DIAL 0xe001
//
//// for 18F2321
////#define TO_BTW_CHARS 0xff00
//// for 18F25K20
//#define TO_BTW_CHARS 0xffDE
////#define 0xff58
//
//// 442mks+xxx=2247(1799??)/898mks == 2693/1340 = count 169/83 
//// addon needs to be added
//// for 18F2321
////#define TIME_FOR_PACKET 0xfef4
//// for 18F25K20
//#define TIME_FOR_PACKET 0xffad
////#define TIME_FOR_PACKET 0xff52
////#define TIME_FOR_PACKET 0xff42
//// for 18F2321
////#define DELAY_BTW_SEND_PACKET 0xffa3
//// for 18F25K20
//#define DELAY_BTW_SEND_PACKET 0xffd0

//////////////////////////////////////////////////////
// to properly process Timer0 interrupt needs to has all timer set values different

#define MEDIAN_TIME      0x1000
#define MEDIAN_TIME_LOW  0x0010
#define MEDIAN_TIME_HIGH 0x2000

//#define MEDIAN_TIME      0x2000
//#define MEDIAN_TIME_LOW  0x0010
//#define MEDIAN_TIME_HIGH 0x4000



//#define DELAY_BTW_NEXT_DIAL 0xfeec
#define DELAY_BTW_NEXT_DIAL 0xe00c
#define PING_DELAY 4
#define DEBUG_LED_COUNT 1
#define TO_BTW_CHARS 0xff00

#define TIME_FOR_PACKET 0xff98
#define TIME_FOR_PACKET0 0xff97

//#define DELAY_BTW_SEND_PACKET 0xfe03
#define DELAY_BTW_SEND_PACKET 0xff73
//#define DELAY_BTW_SEND_PACKET 0xffa3
//#define DELAY_BTW_SEND_PACKET 0xffd1

//#define MAX_TX_POSSIBLE 0xE0bf
#define MAX_TX_POSSIBLE 0xD8EF
//#define MIN_TX_POSSIBLE 0xB9AF
#define MIN_TX_POSSIBLE 0xB1DF
                        // value 0xffff-8000 =0xE0bf - that is max value when TX will be possible
                        // (TO= 93*128 = 11904 op = 0.001488)+(Packet prep = 7472=0.000467)
                        // 1 char = 0.0002sec TO= 93*128 = 11904 == 3.75 char
                        // allow to get 3 char 0.0002*3 *16,000,000= 9600 cycles
                        // 0xffff - (8000+10000) = 0xB9AF
                        // all time btw FQ1 FQ2 is 26807 = 0.0016754375 = 8.3 char, all 3 freq = 25 char

/////////////////////////////////////////////////////
//#define __DEBUG


// this include commands to switch off flush 
//#define FLASH_POWER_DOWN 1

// for debugging TX in simulation debugger - it requare stimulus file
//#define DEBUG_SIM 1
/////////////////////////////////////////////////////////////////////////////////////////////////////
// define blinking LED on pin 14 (RC3)
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define DEBUG_LED
#ifdef DEBUG_LED
 #define DEBUG_LED_OFF bitclr(LATA,5)
 #define DEBUG_LED_ON bitset(LATA,5)
///////////////////////////////////////////////////////////////
//   for a blinking LED behive like CUBESAT/CRAFT
//   it is waiting for connection, wait for p/kt, and when pkt is Ok it send back to earth reply packet, and blinks
///////////////////////////////////////////////////////////////
//#define DEBUG_LED_CALL_EARTH
// for test sequence 
//// "5atsx=...CBabbcgg
/// atdtl
// 5/"

///////////////////////////////////////////////////////////////
//   for a blinking LED behive like Ground Station, it is constantly sends pktm if received pkt, then it blinks
///////////////////////////////////////////////////////////////
#define DEBUG_LED_CALL_LUNA
// for test sequence 
// "5atsx=...CBabbcgg
// atdtl
// 5"
#endif


 // next line uncommented == by default it will be cubesat (calling earth)
//#define DEFAULT_CALL_EARTH 1

//////////////////////////re-arrange//////////////////////////////////////////
#ifdef DEBUG_LED_CALL_EARTH
#define DEFAULT_CALL_EARTH 1
#undef DEBUG_LED_CALL_LUNA
#endif

// next line uncommented == by default it will be ground station (calling luna)
//#define DEFAULT_CALL_LUNA 1

//////////////////////////re-arrange//////////////////////////////////////////
#ifdef DEBUG_LED_CALL_LUNA
#define DEFAULT_CALL_LUNA 1
#undef DEBUG_LED_CALL_EARTH
#endif 

//////////////////////////re-arrang///////////////////////////////////////////
#ifdef DEFAULT_CALL_EARTH
#undef DEFAULT_CALL_LUNA
#endif 
#ifdef DEFAULT_CALL_LUNA
#undef DEFAULT_CALL_EARTH
#endif

// needs to spec processor - C and ASM code is different
#ifdef __18CXX
#ifdef __16F88
#define _16F88 1
#endif
#ifdef __16F884
#define _16F884 1
#endif
#ifdef __18F2321
#define _18F2321 1
#endif
#ifdef __18F25K20
#define _18F25K20 1
#endif
#endif

// will be responce on command "=<unit><cmd>"
#define RESPONCE_ON_EQ

// sync clock / timeral  support
#define SYNC_CLOCK_TIMER  

// next line define non standart modem implementation
// next line define command:
// *<send data> over connected link
#define NON_STANDART_MODEM 1




////////////////////////////////////////////
// listing in C30
// -Wa,-ahlsnd="$(BINDIR_)$(INFILEBASE).lst"
// -g - debugging information
// -O1 - optimization looks good
// -02 and -O3 does something that code is not recognizable - can be dangerouse
// -fpack-struct pack struct
// -save-temps 
// -funroll-loops this will make loops big, ugly but fast
////////////////////////////////////////////
// -g -Wall -save-temps -O1 -Wa,-ahlsnd="$(BINDIR_)$(INFILEBASE).lst"
////////////////////////////////////////////

///////////////////////////////////////////////////////////////
// disable I2C processing
///////////////////////////////////////////////////////////////
#define NO_I2C_PROC 1
// it can be only master support: pic works in master mode only=> uncomment this line if 
//     no multimaster support on a bus
//#define I2C_ONLY_MASTER 1

// master support done via interrupts and firmware - commenting next line and I2C will be a software work
//#define I2C_INT_SUPPORT 1

// different processors:
#ifdef _16F88
#endif

#ifdef _16F884
#endif

#ifdef _18F2321
#endif

#ifdef _18F25K20
#endif

////////////////////////////////////////////////////////////////////////////////////
// for additional (separated from SyncUART) support 
// FLASH MEMORY support
///////////////////////////////////////////////////////////////////////////////////
#define EXT_INT 1
#ifdef __PIC24H__
// SSCLOCK RA0(pin2), SSDATA_IN RA1(pin3), SSDATA_OUT RA2(pin9), SSCS RA3(pin10)
//#define SSPORT  LATAbits
//#define SSCLOCK LATA0
//#define SSDATA_IN LATA1
//#define SSDATA_OUT LATA2
//#define SSCS       LATA3

#define SSPORT  PORTAbits
#define SSCLOCK RA0
#define SSDATA_IN RA1
#define SSDATA_OUT RA2
#define SSCS       RA3
#else
// SSCLOCK RA2(pin4), SSDATA_IN RA3(pin5), SSDATA_OUT RA4(pin6), SSCS RA5(pin7)

#ifdef _18F25K20
#define SSPORT LATC
#define SSPORT_READ PORTC
#define SSCLOCK 4
#define SSDATA_IN 3
#define SSDATA_OUT 0
#define SSDATA_OUT_READ 0
#define SSCS       5

// this is for Cubesat version - 3 FLASH memory processing
#ifdef DEBUG_LED_CALL_LUNA
#define SSPORT2      LATC
#define SSPORT2_READ  PORTC
#define SSDATA_OUT2 1
#define SSDATA_OUT3 2
#else
// deppend on scematics
//#define SSPORT2      LATC
//#define SSPORT2_READ  PORTC
//#define SSDATA_OUT2 1
//#define SSDATA_OUT3 2

#endif

#else
#define SSPORT PORTA
#define SSCLOCK 2
#define SSDATA_IN 3
#define SSDATA_OUT 4
#define SSCS       5

// this is for Cubesat version - 3 FLASH memory processing
//#define SSPORT2  PORTC
//#define SSDATA_OUT2 0
//#define SSDATA_OUT3 1
#endif
#endif

// carefull!!! on SST25VF032 present write protection bits which are 111 by default
// and operation write bytes 
#define SST25VF032 
#ifdef SST25VF032
#else
#endif
/////////////////////////////////////////////////////////////////////////////////
//   BT definitions
/////////////////////////////////////////////////////////////////////////////////
#define PORT_BT LATA
#define PORT_BT_READ PORTA
#define Tx_CE      0	// RA0 pin 2 // Chip Enable Activates RX or TX mode
#define Tx_CSN     1	// RA1 pin 3 // SPI Chip Select
#define Tx_SCK     2    // RA2 pin 4  // SPI Clock
#define Tx_MOSI    3	// RA3 pin 5  // SPI Slave Data Input
#define Rx_MISO    4	// RA4 pin 6  // SPI Slave Data Output, with tri-state option
#define Rx_IRQ     0    // RB0 pin 21 // Maskable interrupt pin. Active low
#define PORT_AMPL LATB
#define BT_TX      1   // RB1 pin 22 BT in transmit mode
#define BT_RX      2   // RB2 pin 23 BT in receive mode

#define SETUP_RX_MODE 0
#define SETUP_TX_MODE 1

///////////////////////////////////////////////////////////////////////////////////
//   serial port semaphores
//     RX_FULL     signals to prev unit to send data
//     TX_NOT_READY   check next in loop for ready to receive data
////////////////////////////////////////////////////////////////////////////////////
#define RX_FULL LATB.4
#define TX_NOT_READY LATB.3



// redifine output buffer size
#define BUFFER_LEN 40
#define OUT_BUFFER_LEN 40





//#include "..\..\commc0.h"
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// begin of COPY 0
///////////////////////////////////////////////////////////////////////
#ifdef _18F2321
#define _18F2321_18F25K20
#endif

#ifdef _18F25K20
#define _18F2321_18F25K20
#endif

#ifdef _16F884
#define _16F884_16F724
#endif

#ifdef _16F724
#define _16F884_16F724
#endif


// preprocessing different MASKS for different units
#if (MY_UNIT == '1')
    #define UNIT_MASK   0b00000010
    #define UNIT_MASK_H 0b11111110
#endif 

#if (MY_UNIT == '2')
    #define UNIT_MASK   0b00000100
    #define UNIT_MASK_H 0b11111100
#endif 

#if (MY_UNIT == '3')
    #define UNIT_MASK   0b00001000
    #define UNIT_MASK_H 0b11111000
#endif 

#if (MY_UNIT == '4')
    #define UNIT_MASK   0b00010000
    #define UNIT_MASK_H 0b11110000
#endif 

#if (MY_UNIT == '5')
    #define UNIT_MASK   0b00100000
    #define UNIT_MASK_H 0b11100000
#endif 

#if (MY_UNIT == '6')
    #define UNIT_MASK   0b01000000
    #define UNIT_MASK_H 0b11000000
#endif 

#if (MY_UNIT == '7')
    #define UNIT_MASK   0b10000000
    #define UNIT_MASK_H 0b10000000
#endif 

#if (MY_UNIT == '8')
    #define UNIT_MASK   0b10000000
    #define UNIT_MASK_H 0b00000000
#endif 

#ifdef _18F2321
//#define pAddInc\
//            FSR_REGISTER ++;\
//            PTR_FSR+=CARRY_BYTE;\
//            if (Carry)\
//                CARRY_BYTE = 1;\
//            else\
//                CARRY_BYTE = 0;
#define pSubInc\
            FSR_REGISTER ++;\
            PTR_FSR-=CARRY_BYTE;\
            CARRY_BYTE = !Carry;

#define pSub___(x,y,z)\
            PTR_FSR-=x;\
            if (!Carry)\
                CARRY_BYTE++;\
            PTR_FSR-=y;\
            if (!Carry)\
                CARRY_BYTE++;\
            PTR_FSR-=z;\
            if (!Carry)\
                CARRY_BYTE++;\
            pSubInc;


#define pSub__(x,y)\
            PTR_FSR-=x;\
            if (!Carry)\
                CARRY_BYTE++;\
            PTR_FSR-=y;\
            if (!Carry)\
                CARRY_BYTE++;\
            pSubInc;

#define pSub_(x)\
            PTR_FSR-=x;\
            if (!Carry)\
                CARRY_BYTE++;\
            pSubInc;

#define pSubS___(x,y,z)\
            if (bittest(x,7))\
            {\
                W =0xff;\
                PTR_FSR-=W;\
                if (!Carry)\
                    CARRY_BYTE++;\
            }\
            if (bittest(y,7))\
            {\
                W =0xff;\
                PTR_FSR-=W;\
                if (!Carry)\
                    CARRY_BYTE++;\
            }\
            if (bittest(z,7))\
            {\
                W =0xff;\
                PTR_FSR-=W;\
                if (!Carry)\
                    CARRY_BYTE++;\
            }\
            pSubInc;


#define pSubS__(x,y)\
            if (bittest(x,7))\
            {\
                W =0xff;\
                PTR_FSR-=W;\
                if (!Carry)\
                    CARRY_BYTE++;\
            }\
            if (bittest(y,7))\
            {\
                W =0xff;\
                PTR_FSR-=W;\
                if (!Carry)\
                    CARRY_BYTE++;\
            }\
            pSubInc;

#define pSubS_(x)\
            if (bittest(x,7)) \
            {\
                W =0xff;\
                PTR_FSR-=W;\
                if (!Carry)\
                    CARRY_BYTE++;\
            }\
            pSubInc;

#define pSubSEnd___(x,y,z)\
            if (bittest(x,7))\
            {\
                PTR_FSR++;\
            }\
            if (bittest(y,7))\
            {\
                PTR_FSR++;\
            }\
            if (bittest(z,7))\
            {\
                PTR_FSR++;\
            }


#define pSubSEnd__(x,y)\
            if (bittest(x,7))\
            {\
                PTR_FSR++;\
            }\
            if (bittest(y,7))\
            {\
                PTR_FSR++;\
            }

#define pSubSEnd_(x)\
            if (bittest(x,7))\
            {\
                PTR_FSR++;\
            }





#define pAddInc\
            FSR_REGISTER ++;\
            PTR_FSR+=CARRY_BYTE;\
            CARRY_BYTE = Carry;

#define pAdd___(x,y,z)\
            PTR_FSR+=x;\
            if (Carry)\
                CARRY_BYTE=++;\
            PTR_FSR+=y;\
            if (Carry)\
                CARRY_BYTE++;\
            PTR_FSR+=z;\
            if (Carry)\
                CARRY_BYTE++;\
            pAddInc;

#define pAdd__(x,y)\
            PTR_FSR+=x;\
            if (Carry)\
                CARRY_BYTE++;\
            PTR_FSR+=y;\
            if (Carry)\
                CARRY_BYTE++;\
            pAddInc;

#define pAdd_(x)\
            PTR_FSR+=x;\
            if (Carry)\
                CARRY_BYTE++;\
            pAddInc;

#define pAddS__(x,y)\
            if (bittest(x,7))\
            {\
                W =0xff;\
                PTR_FSR+=W;\
                if (Carry)\
                    CARRY_BYTE++;\
            }\
            if (bittest(y,7))\
            {\
                W =0xff;\
                PTR_FSR+=W;\
                if (Carry)\
                    CARRY_BYTE++;\
            }\
            pAddInc;

#define pAddS_(x)\
            if (bittest(x,7)) \
            {\
                W =0xff;\
                PTR_FSR+=W;\
                if (Carry)\
                    CARRY_BYTE++;\
            }\
            pAddInc;

#define pAddSEnd__(x,y)\
            if (bittest(x,7))\
            {\
                PTR_FSR--;\
            }\
            if (bittest(y,7))\
            {\
                PTR_FSR--;\
            }

#define pAddSEnd_(x)\
            if (bittest(x,7))\
            {\
                PTR_FSR--;\
            }
#endif //#ifdef _18F2321


#define SSPBUF_RX SSPBUF
#define SSPBUF_TX SSPBUF


#define VOLATILE
// on 88,884,2321 I2C_BRG=SSPADD
#define I2C_BRG SSPADD


// to be compatible with VS2010
#ifdef WIN32
    #ifdef _16F88
    #endif
    #ifdef _16F884
    #endif

    #ifdef _18F2321
    //#include "18F2321.H"
    #endif
#endif
// on small processors it is 1 byte
#define WORD unsigned char
#define UWORD unsigned char
// different processors different includes and different RAM BANKS
//////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifdef _16F88
#include "int16CXX.H"
#define RAM_BANK_0 0
#define RAM_BANK_1 1
#define RAM_BANK_2 2
#define RAM_BANK_3 3
#define RAM_BANK_4 2
#define RAM_BANK_5 3
#define TIMER0_CONTROL_REG OPTION_REG
#define TIMER0_BYTE TMR0
#define TIMER0_INT_ENBL TMR0IE
#define TIMER0_INT_FLG  TMR0IF
#define FSR_REGISTER FSR
#define PTR_FSR INDF
#define INT0_EDG INTEDG
#define INT0_FLG INT0IF
#define INT0_ENBL INT0IE
#define I2CPORT PORTB
#define I2CTRIS TRISB
#define I2C_SDA 1
#define I2C_SCL 4
#define TIMER0_INT_ENBL TMR0IE
#define TIMER0_INT_FLG  TMR0IF
#undef _Q_PROCESS
#ifdef I2C_INT_SUPPORT
#warning "No I2C wirmware support for PIC16F88 device"
#undef I2C_INT_SUPPORT
#endif
#define _TRMT TRMT
#endif
//////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifdef _16F884
#include "int16CXX.H"
#define RAM_BANK_0 0
#define RAM_BANK_1 1
#define RAM_BANK_2 2
#define RAM_BANK_3 3
#define RAM_BANK_4 2
#define RAM_BANK_5 3

#define TIMER0_CONTROL_REG OPTION_REG
#define TIMER0_BYTE TMR0
#define TIMER0_INT_ENBL T0IE
#define TIMER0_INT_FLG  T0IF
#define FSR_REGISTER FSR
#define PTR_FSR INDF
#define INT0_EDG INTEDG
#define INT0_FLG INTF
#define INT0_ENBL INTE
#define I2CPORT PORTB
#define I2CTRIS TRISB
#define I2C_SDA 1
#define I2C_SCL 4
#ifdef I2C_INT_SUPPORT
//#warning "No I2C wirmware support for PIC16F884 device"
//#undef I2C_INT_SUPPORT
#endif
#undef _Q_PROCESS
#define _TRMT TRMT
#endif

//////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifdef _16F724
#include "int16CXX.H"
#define RAM_BANK_0 0
#define RAM_BANK_1 1
#define RAM_BANK_2 2
#define RAM_BANK_3 3
#define RAM_BANK_4 2
#define RAM_BANK_5 3

#define TIMER0_CONTROL_REG OPTION_REG
#define TIMER0_BYTE TMR0
#define TIMER0_INT_ENBL T0IE
#define TIMER0_INT_FLG  T0IF
#define FSR_REGISTER FSR
#define PTR_FSR INDF
#define INT0_EDG INTEDG
#define INT0_FLG INTF
#define INT0_ENBL INTE
#define I2CPORT PORTB
#define I2CTRIS TRISB
#define I2C_SDA 1
#define I2C_SCL 4
#ifdef I2C_INT_SUPPORT
//#warning "No I2C wirmware support for PIC16F884 device"
//#undef I2C_INT_SUPPORT
#endif
#undef _Q_PROCESS
#define _TRMT TRMT
#endif


//////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifdef _18F25K20
#undef _Q_PROCESS
#ifdef __18CXX
//char PORTE    @ 0xF84;
#include "p18f2321.h"
#define RCIF PIR1bits.RCIF
#define RCIE PIE1bits.RCIE
#define CREN RCSTAbits.CREN
#define TXIE PIE1bits.TXIE
#define TXIF PIR1bits.TXIF
#define TRMT TXSTAbits.TRMT
#define BCLIF PIR2bits.BCLIF
#define SEN SSPCON2bits.SEN
#define PEN SSPCON2bits.PEN
#define SSPIF PIR1bits.SSPIF
#define SSPOV SSPCON1bits.SSPOV
#define BF SSPSTATbits.BF
#define ACKEN SSPCON2bits.ACKEN
#define ACKDT SSPCON2bits.ACKDT
#define RCEN SSPCON2bits.RCEN
#define RSEN SSPCON2bits.RSEN
#define P SSPSTATbits.P
#define S SSPSTATbits.S
#define DA_ SSPSTATbits.D_A
#define BCLIE PIE2bits.BCLIE 
#define TMR0IF INTCONbits.TMR0IF
#define T0SE T0CONbits.T0SE
#define TMR0ON T0CONbits.TMR0ON
#define TMR0IE INTCONbits.TMR0IE
#define TMR1IF PIR1bits.TMR1IF
#define INT0IF INTCONbits.INT0IF
#define INT0IE INTCONbits.INT0IE
#define GIE INTCONbits.GIE 
#define POR_ RCONbits.POR
#define PEIE INTCONbits.PEIE
#define RBIF INTCONbits.RBIF
#define TMR1IE PIE1bits.TMR1IE
#define IDLEN OSCCONbits.IDLEN 
#define TXEN TXSTAbits.TXEN
#define SMP SSPSTATbits.SMP
#define CKE SSPSTATbits.CKE
#define WCOL SSPCON1bits.WCOL
#define SSPEN SSPCON1bits.SSPEN
#define Carry STATUSbits.C
#define T08BIT T0CONbits.T08BIT
#define PLLEN OSCTUNEbits.PLLEN
#define INTEDG0 INTCON2bits.INTEDG0
#define RD EECON1bits.RD
#define WREN EECON1bits.WREN
#define WR EECON1bits.WR
#define TX9 TXSTAbits.TX9
#define RX9 RCSTAbits.RX9
#define SPEN RCSTAbits.SPEN
#define BRGH TXSTAbits.BRGH
#define SYNC TXSTAbits.SYNC
#define ADDEN RCSTAbits.ADDEN
#define SSPIE PIE1bits.SSPIE
#define nop()    {_asm nop _endasm}
//#define ClrWdt() {_asm clrwdt _endasm}
#define sleep()  {_asm sleep _endasm}
#define seset()  {_asm reset _endasm}
#define FSR_REGISTER ptr_FSR
unsigned char *ptr_FSR;
#define PTR_FSR (*ptr_FSR)
#else
//#include "18f25fk20.h"
#include "int18XXX.H"
char PORTE    @ 0xF84;
#define FSR_REGISTER FSR0
#define PTR_FSR INDF0
#endif
#define RAM_BANK_0 0
#define RAM_BANK_1 1
#define RAM_BANK_2 2
#define RAM_BANK_3 3
#define RAM_BANK_4 4
#define RAM_BANK_5 5
#define TIMER0_CONTROL_REG T0CON
#define TIMER0_BYTE TMR0L
#define TIMER0_INT_ENBL TMR0IE
#define TIMER0_INT_FLG  TMR0IF

#define INT0_EDG INTEDG0
#define INT0_FLG INT0IF
#define INT0_ENBL INT0IE
#define I2CPORT PORTC
#define I2CTRIS TRISC
#define I2C_SDA 4
#define I2C_SCL 3
#define _TRMT TRMT
#undef UWORD  
#define UWORD unsigned long
#endif
//////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifdef _18F2321
#undef _Q_PROCESS
#ifdef __18CXX
//char PORTE    @ 0xF84;
#include "p18f2321.h"
#define RCIF PIR1bits.RCIF
#define RCIE PIE1bits.RCIE
#define CREN RCSTAbits.CREN
#define TXIE PIE1bits.TXIE
#define TXIF PIR1bits.TXIF
#define TRMT TXSTAbits.TRMT
#define BCLIF PIR2bits.BCLIF
#define SEN SSPCON2bits.SEN
#define PEN SSPCON2bits.PEN
#define SSPIF PIR1bits.SSPIF
#define SSPOV SSPCON1bits.SSPOV
#define BF SSPSTATbits.BF
#define ACKEN SSPCON2bits.ACKEN
#define ACKDT SSPCON2bits.ACKDT
#define RCEN SSPCON2bits.RCEN
#define RSEN SSPCON2bits.RSEN
#define P SSPSTATbits.P
#define S SSPSTATbits.S
#define DA_ SSPSTATbits.D_A
#define BCLIE PIE2bits.BCLIE 
#define TMR0IF INTCONbits.TMR0IF
#define T0SE T0CONbits.T0SE
#define TMR0ON T0CONbits.TMR0ON
#define TMR0IE INTCONbits.TMR0IE
#define TMR1IF PIR1bits.TMR1IF
#define INT0IF INTCONbits.INT0IF
#define INT0IE INTCONbits.INT0IE
#define GIE INTCONbits.GIE 
#define POR_ RCONbits.POR
#define PEIE INTCONbits.PEIE
#define RBIF INTCONbits.RBIF
#define TMR1IE PIE1bits.TMR1IE
#define IDLEN OSCCONbits.IDLEN 
#define TXEN TXSTAbits.TXEN
#define SMP SSPSTATbits.SMP
#define CKE SSPSTATbits.CKE
#define WCOL SSPCON1bits.WCOL
#define SSPEN SSPCON1bits.SSPEN
#define Carry STATUSbits.C
#define T08BIT T0CONbits.T08BIT
#define PLLEN OSCTUNEbits.PLLEN
#define INTEDG0 INTCON2bits.INTEDG0
#define RD EECON1bits.RD
#define WREN EECON1bits.WREN
#define WR EECON1bits.WR
#define TX9 TXSTAbits.TX9
#define RX9 RCSTAbits.RX9
#define SPEN RCSTAbits.SPEN
#define BRGH TXSTAbits.BRGH
#define SYNC TXSTAbits.SYNC
#define ADDEN RCSTAbits.ADDEN
#define SSPIE PIE1bits.SSPIE
#define nop()    {_asm nop _endasm}
//#define ClrWdt() {_asm clrwdt _endasm}
#define sleep()  {_asm sleep _endasm}
#define seset()  {_asm reset _endasm}
#define FSR_REGISTER ptr_FSR
unsigned char *ptr_FSR;
#define PTR_FSR (*ptr_FSR)
#else
//#include "18f2321.h"
#include "int18XXX.H"
char PORTE    @ 0xF84;
#define FSR_REGISTER FSR0
#define PTR_FSR INDF0
#endif
#define RAM_BANK_0 0
#define RAM_BANK_1 1
#define RAM_BANK_2 0
#define RAM_BANK_3 1
#define RAM_BANK_4 0
#define RAM_BANK_5 1
#define TIMER0_CONTROL_REG T0CON
#define TIMER0_BYTE TMR0L
#define TIMER0_INT_ENBL TMR0IE
#define TIMER0_INT_FLG  TMR0IF

#define INT0_EDG INTEDG0
#define INT0_FLG INT0IF
#define INT0_ENBL INT0IE
#define I2CPORT PORTC
#define I2CTRIS TRISC
#define I2C_SDA 4
#define I2C_SCL 3
#define _TRMT TRMT
#undef UWORD  
#define UWORD unsigned long
#endif
//////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifdef __PIC24H__
#undef WORD
#ifdef HI_TECH_C
   #include "htc.h"
   #include "pic24HJ64GP502.h"
#else // end of HI_TECH
   #include "p24hxxxx.h"
   #ifdef __PIC24HJ64GP502__
      #include "p24HJ64GP502.h"
   #endif
   #ifdef __PIC24HJ64GP504__
      #include "p24HJ64GP504.h"
   #endif
   #include "rtcc.h"

   #include "pps.h"
   #include "string.h"
#endif
// bugs in C30 compiler!! this can create unpredictable code
#define VOLATILE volatile 
//#define VOLATILE
#define SSPIF IFS1bits.MI2C1IF
#define BF I2C1STATbits.RBF
//#define BF bittest(I2C1STAT,2)
#define ACKEN I2C1CONbits.ACKEN
#define DA_ I2C1STATbits.D_A
#define SSPBUF_RX I2C1RCV
#define SSPBUF_TX I2C1TRN
#define P I2C1STATbits.P
#define ACKDT I2C1CONbits.ACKDT
#define RCEN I2C1CONbits.RCEN
#define ACKSTAT I2C1STATbits.ACKSTAT
#define SSPADD I2C1BRG
#define I2C_BRG I2C1BRG
#define RSEN I2C1CONbits.RSEN
#define PEN I2C1CONbits.PEN
#define S I2C1STATbits.S
#define BCLIF I2C1STATbits.BCL
#define SSPOV I2C1STATbits.I2COV
#define RCIE IEC0bits.U1RXIE
#define RCIF IFS0bits.U1RXIF
#define RCSTA U1STA
#define RCREG U1RXREG
#define _TRMT U1STAbits.TRMT
#define TXREG U1TXREG
#define TXIE IEC0bits.U1TXIE

#ifdef USE_COM2
#define RCIECOM2 IEC1bits.U2RXIE
#define RCIFCOM2 IFS1bits.U2RXIF
#define RCSTACOM2 U2STA
#define RCREGCOM2 U2RXREG
#define TRMTCOM2 U2STAbits.TRMT
#define TXREGCOM2 U2TXREG
#define TXIECOM2 IEC1bits.U2TXIE
#define TXENCOM2 U2STAbits.UTXEN
#define TXIFCOM2 IFS1bits.U2TXIF
#endif

#define TIMER0_INT_FLG IFS0bits.T1IF
#define TIMER0_INT_ENBL IEC0bits.T1IE
#define TMR0ON T1CONbits.TON
#define TMR1IF IFS0bits.T2IF
#define TMR1IE IEC0bits.T2IE
#define TMR1ON T2CONbits.TON
#define INT0_FLG IFS0bits.INT0IF
#define INT0_ENBL IEC0bits.INT0IE
#define INT1IF IFS1bits.INT1IF
#define INT1IE IEC1bits.INT1IE
#define INT2IF IFS1bits.INT2IF
#define INT2IE IEC1bits.INT2IE
#define POR_ RCONbits.POR
#define FSR_REGISTER ptr_FSR
unsigned char *ptr_FSR;
#define PTR_FSR (*ptr_FSR)
#define TXEN U1STAbits.UTXEN
#define TXIF IFS0bits.U1TXIF
#define WCOL I2C1STATbits.IWCOL
#define SEN I2C1CONbits.SEN
#define TIMER0_BYTE TMR1
#define INT0_EDG INTCON2bits.INT0EP
#define INTEDG1 INTCON2bits.INT1EP
#define INTEDG2 INTCON2bits.INT2EP
#define Carry SRbits.C
#ifdef HI_TECH_C
#define nop() asm("nop");
#define __builtin_write_OSCCONH(arg) OSCCONH=arg;
#define __builtin_write_OSCCONL(arg) OSCCONL=arg;
#else
#define nop Nop
#endif
    #ifndef WORD
    #define WORD unsigned int
    #endif
#endif
//////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////


#ifdef SSPORT
#ifdef FLASH_POWER_DOWN
void CsLow(void);
void CsHigh(void);
#define CS_LOW  CsLow(); // set low Chip Select
#define CS_HIGH CsHigh(); // set high Chip Select
#else
#define CS_LOW  bclr(SSPORT,SSCS); // set low Chip Select
#define CS_HIGH bset(SSPORT,SSCS); // set high Chip Select
#endif
#endif 


#define _NOT_SIMULATOR 1
#define _OPTIMIZED_ 1



#ifdef _18F2321_18F25K20
//#undef _OPTIMIZED_
//#define DELAY_I2C 14
//#define DELAY_1_I2C W = DELAY_I2C; while(--W);
#define DELAY_START_I2C nop();nop();nop();
#define DELAY_1_I2C nop();nop();nop();

// on 18F232l in Fosc=32MHz Fcy= 8MIPS BRG=0x05 for 800kHz I2C operation
// on 18F232l in Fosc=32MHz Fcy= 8MIPS BRG=0x09 for 800kHz I2C operation
//#define FAST_DELAY 0x5
//#define SLOW_DELAY 0x9

// on 18F232l in Fosc=32MHz Fcy= 8MIPS BRG=0x1f for 250kHz I2C operation
#define FAST_DELAY 0x1f
#define SLOW_DELAY 0x1f

// on 18F232l in Fosc=32MHz Fcy = 8MIPS BRG=0x12 for 400kHz I2C operation
//#define FAST_DELAY 0x13
//#define SLOW_DELAY 0x13


      //nop();nop();nop();nop();nop();nop();nop();nop();nop();
#else
   #ifdef __PIC24H__
#define FAST_DELAY 98
#define SLOW_DELAY 98 
   #else
// on 16LF88 in Fosc=8MHz Fcy= 2MIPS 3 nop() = 666kHz
#define DELAY_START_I2C nop();nop();nop();
#define DELAY_1_I2C nop();nop();nop();

// on 16LF884 in Fosc=8MHz Fcy= 2MIPS BRG=0x06 for 285kHz I2C operation
// on 16LF884 in Fosc=8MHz Fcy= 2MIPS BRG=0x0a for 181kHz I2C operation
#define FAST_DELAY 0x6
#define SLOW_DELAY 0xa 
   #endif
#endif

////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _16F88
///////////////////////////////////////////////////////////////////////////////////////////
// this line for debug and may be normal operations
// on pin 15 (RA6) no oscilator output  
#pragma config PWRTE=off, WDTE=off, FOSC=4, BODEN=off
//#pragma config PWRTE=on, WDTE=off, FOSC=4, BODEN=off
//#define DEBUG_OSC_PIN15 1

// this line for pin 15 (RA6) oscilator output 2MHz:
//#pragma config PWRTE=off, WDTE=off, FOSC=5, BODEN=off
//#pragma config |= 0x2f20
// for debuging by pickit 3
#pragma config |= 0x2720
// 0010 1111 0010 0000
//   1                = 1 = Code protection off
//    0               = 0 = CCP1 function on RB3
//      1             = 1 = In-Circuit Debugger disabled, RB6 and RB7 are general purpose I/O pins
//       11           = 11 = Write protection off
//         1          = 1 = Code protection off;
//           0        = 1 = RB3/PGM pin has PGM function, Low-Voltage Programming enabled
//            0       = 0 = BOR disabled
//             1      = 0 = RA5/MCLR/VPP pin function is digital I/O, MCLR internally tied to VDD
//                0   = 0 = PWRT enabled
//                 0  = 0 = WDT disabled
//              1   00= FOSC=4 =100 = INTRC oscillator; port I/O function on both RA6/OSC2/CLKO pin and RA7/OSC1/CLKI pin
// bit 13 CP: Flash Program Memory Code Protection bits
//            1 = Code protection off; 0 = 0000h to 0FFFh code-protected (all protected)
// bit 12 CCPMX: CCP1 Pin Selection bit
//            1 = CCP1 function on RB0; 0 = CCP1 function on RB3
// bit 11 DEBUG: In-Circuit Debugger Mode bit
//            1 = In-Circuit Debugger disabled, RB6 and RB7 are general purpose I/O pins
//            0 = In-Circuit Debugger enabled, RB6 and RB7 are dedicated to the debugger
// bit 10-9 WRT<1:0>: Flash Program Memory Write Enable bits
//           11 = Write protection off
//           10 = 0000h to 00FFh write-protected, 0100h to 0FFFh may be modified by EECON control
//           01 = 0000h to 07FFh write-protected, 0800h to 0FFFh may be modified by EECON control
//           00 = 0000h to 0FFFh write-protected
// bit 8 CPD: Data EE Memory Code Protection bit
//            1 = Code protection off; 0 = Data EE memory code-protected
// bit 7 LVP: Low-Voltage Programming Enable bit
//            1 = RB3/PGM pin has PGM function, Low-Voltage Programming enabled
//            0 = RB3 is digital I/O, HV on MCLR must be used for programming
// bit 6 BOREN: Brown-out Reset Enable bit
//            1 = BOR enabled ; 0 = BOR disabled
// bit 5 MCLRE: RA5/MCLR/VPP Pin Function Select bit
//            1 = RA5/MCLR/VPP pin function is MCLR ; 0 = RA5/MCLR/VPP pin function is digital I/O, MCLR internally tied to VDD
// bit 3 PWRTEN: Power-up Timer Enable bit
//            1 = PWRT disabled; 0 = PWRT enabled
// bit 2 WDTEN: Watchdog Timer Enable bit
//            1 = WDT enabled ; 0 = WDT disabled
// bit 4, 1-0 FOSC<2:0>: Oscillator Selection bits
//         111 = EXTRC oscillator; CLKO function on RA6/OSC2/CLKO
//         110 = EXTRC oscillator; port I/O function on RA6/OSC2/CLKO
//         101 = INTRC oscillator; CLKO function on RA6/OSC2/CLKO pin and port I/O function on RA7/OSC1/CLKI pin
//         100 = INTRC oscillator; port I/O function on both RA6/OSC2/CLKO pin and RA7/OSC1/CLKI pin
//         011 = ECIO; port I/O function on RA6/OSC2/CLKO
//         010 = HS oscillator
//         001 = XT oscillator
//         000 = LP oscillator
#pragma config reg2 = 0
#endif
//////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifdef _16F884
/////////////////////////////////////////////////////////////////////////////////////////
// this line for debug and may be normal operations
// on pin 15 (RA6) no oscilator output  
//#pragma config PWRTE=on, WDTE=off, FOSC=4, BODEN=off
//#define DEBUG_OSC_PIN15 1

// this line for pin 15 (RA6) oscilator output 2MHz
#pragma config PWRTE=off, WDTE=off
//#pragma config |= 0x30f4
// for debuging by pickit 3
#pragma config |= 0x10f4
// 0001 0000 1110 0000
// 00                  == not in use
//   1                 = 1 = In-Circuit Debugger disabled, RB6 and RB7 are general purpose I/O pins
//    1                == LVP (1) = RB3/PGM pin has PGM function, Low-Voltage Programming enabled, (0) disabled
//      0              == FCMEN Fail-Safe Clock Monitor (1) Enabled (0) disabled
//       0             == IESO Internal - external Swithcover bit (1) eabled (0) disabled
//        00           == BOREN - brown-out reset selection
//                       11 - BOR enable
//                       10 - BOR enabled at operation and disabled at sleep
//                       01 - BOR controlled by SBOREN bit in PCON register
//                       00 - BOR disabled
//           1         == CPD (data code protection (1) disabled (0) enabled
//            1        == CP Data Code protection (1) program memory code protection disabled (0) enabled
//             1       == MCLRE - (1) RE3 is MCLR is on (0) RE3 - digital input
//              1      == PWRTE - (1) power up timer disabled (0) powerup timer enabled
//                0    == WTDE watchdog timer (1) enabled (0) disabled
//                 100 =100 = INTRC oscillator; port I/O function on both RA6/OSC2/CLKO pin and RA7/OSC1/CLKI pin
// bit 13 DEBUG: In-Circuit Debugger Mode bit
//            1 = In-Circuit Debugger disabled, RB6 and RB7 are general purpose I/O pins
//            0 = In-Circuit Debugger enabled, RB6 and RB7 are dedicated to the debugger
// bit 12 LVP: Low-Voltage Programming Enable bit
//            1 = RB3/PGM pin has PGM function, Low-Voltage Programming enabled
//            0 = RB3 is digital I/O, HV on MCLR must be used for programming
// bit 11 FCMEN
// bit 10 IESO
// bit 9-8 BOREN: Brown-out Reset Enable bits
// bit 7   CPD Data EE Memory Code Protection bit
//            1 = Code protection off; 0 = Data EE memory code-protected
// bit 6 CP: Flash Program Memory Code Protection bits
//            1 = Code protection off; 0 = 0000h to 0FFFh code-protected (all protected)
// bit 5 MCLRE: RA5/MCLR/VPP Pin Function Select bit
//            1 = RE3/MCLR/VPP pin function is MCLR ; 0 = RE3/MCLR/VPP pin function is digital I/O, MCLR internally tied to VDD
// bit 4 PWRTEN: Power-up Timer Enable bit
//            1 = PWRT disabled; 0 = PWRT enabled
// bit 3 WDTEN: Watchdog Timer Enable bit
//            1 = WDT enabled ; 0 = WDT disabled
// bit 2-0: Oscillator Selection bits
//         111 = RC oscillator; CLKO function on RA6/OSC2/CLKO, RC on RA7
//         110 = RCIO oscillator; port I/O function on RA6/OSC2/CLKO, RC on RA7
//         101 = INTRC oscillator; CLKOUT function on RA6/OSC2/CLKO pin and port I/O function on RA7/OSC1/CLKI pin
//         100 = INTRC oscillator; port I/O function on both RA6/OSC2/CLKO pin and RA7/OSC1/CLKI pin
//         011 = EC: port I/O function on RA6/OSC2/CLKO, CLKIN on RA7
//         010 = HS oscillator; Highspeed cristal/resonator on RA6 and RA7
//         001 = XT oscillator; Crystal/resonator on RA6 and RA7
//         000 = LP oscillator; low power crystal on RA6 and RA7
#pragma config reg2 = 0x0700
// CONFIG2
//  bit 10-9  WRT Flash program memory self write enable bit
//  bit 8     BOR4V  (1) brownout reset 4.0v (1_ brownout reset 2.1V
// 0000 0111 0000 0000
#endif

//////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifdef _16F724
/////////////////////////////////////////////////////////////////////////////////////////
// this line for debug and may be normal operations
// on pin 15 (RA6) no oscilator output  
//#pragma config PWRTE=on, WDTE=off, FOSC=4, BODEN=off
//#define DEBUG_OSC_PIN15 1

// this line for pin 15 (RA6) oscilator output 2MHz
#pragma config PWRTE=off, WDTE=off
//#pragma config |= 0x30f4
// for debuging by pickit 3
#pragma config |= 0x10f4
// 0001 0000 1110 0000
// 00                  == not in use
//   1                 = 1 = In-Circuit Debugger disabled, RB6 and RB7 are general purpose I/O pins
//    1                == LVP (1) = RB3/PGM pin has PGM function, Low-Voltage Programming enabled, (0) disabled
//      0              == FCMEN Fail-Safe Clock Monitor (1) Enabled (0) disabled
//       0             == IESO Internal - external Swithcover bit (1) eabled (0) disabled
//        00           == BOREN - brown-out reset selection
//                       11 - BOR enable
//                       10 - BOR enabled at operation and disabled at sleep
//                       01 - BOR controlled by SBOREN bit in PCON register
//                       00 - BOR disabled
//           1         == CPD (data code protection (1) disabled (0) enabled
//            1        == CP Data Code protection (1) program memory code protection disabled (0) enabled
//             1       == MCLRE - (1) RE3 is MCLR is on (0) RE3 - digital input
//              1      == PWRTE - (1) power up timer disabled (0) powerup timer enabled
//                0    == WTDE watchdog timer (1) enabled (0) disabled
//                 100 =100 = INTRC oscillator; port I/O function on both RA6/OSC2/CLKO pin and RA7/OSC1/CLKI pin
// bit 13 DEBUG: In-Circuit Debugger Mode bit
//            1 = In-Circuit Debugger disabled, RB6 and RB7 are general purpose I/O pins
//            0 = In-Circuit Debugger enabled, RB6 and RB7 are dedicated to the debugger
// bit 12 LVP: Low-Voltage Programming Enable bit
//            1 = RB3/PGM pin has PGM function, Low-Voltage Programming enabled
//            0 = RB3 is digital I/O, HV on MCLR must be used for programming
// bit 11 FCMEN
// bit 10 IESO
// bit 9-8 BOREN: Brown-out Reset Enable bits
// bit 7   CPD Data EE Memory Code Protection bit
//            1 = Code protection off; 0 = Data EE memory code-protected
// bit 6 CP: Flash Program Memory Code Protection bits
//            1 = Code protection off; 0 = 0000h to 0FFFh code-protected (all protected)
// bit 5 MCLRE: RA5/MCLR/VPP Pin Function Select bit
//            1 = RE3/MCLR/VPP pin function is MCLR ; 0 = RE3/MCLR/VPP pin function is digital I/O, MCLR internally tied to VDD
// bit 4 PWRTEN: Power-up Timer Enable bit
//            1 = PWRT disabled; 0 = PWRT enabled
// bit 3 WDTEN: Watchdog Timer Enable bit
//            1 = WDT enabled ; 0 = WDT disabled
// bit 2-0: Oscillator Selection bits
//         111 = RC oscillator; CLKO function on RA6/OSC2/CLKO, RC on RA7
//         110 = RCIO oscillator; port I/O function on RA6/OSC2/CLKO, RC on RA7
//         101 = INTRC oscillator; CLKOUT function on RA6/OSC2/CLKO pin and port I/O function on RA7/OSC1/CLKI pin
//         100 = INTRC oscillator; port I/O function on both RA6/OSC2/CLKO pin and RA7/OSC1/CLKI pin
//         011 = EC: port I/O function on RA6/OSC2/CLKO, CLKIN on RA7
//         010 = HS oscillator; Highspeed cristal/resonator on RA6 and RA7
//         001 = XT oscillator; Crystal/resonator on RA6 and RA7
//         000 = LP oscillator; low power crystal on RA6 and RA7
#pragma config reg2 = 0x0700
// CONFIG2
//  bit 10-9  WRT Flash program memory self write enable bit
//  bit 8     BOR4V  (1) brownout reset 4.0v (1_ brownout reset 2.1V
// 0000 0111 0000 0000
#endif

//////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifdef _18F2321
#ifdef __18CXX
#include "p18f2321.h"
#define rambank varlocate
#endif
///////////////////////////////////////////////////////////////////////////////////////////
// this line for debug and may be normal operations
// on pin 15 (RA6) no oscilator output  
//#pragma config PWRTE=on, WDTE=off, FOSC=4, BODEN=off
//#define DEBUG_OSC_PIN15 1

// this line for pin 15 (RA6) oscilator output 2MHz
//#pragma config PWRTE=off, WDTE=off, FOSC=5, BODEN=off
//#pragma config |= 0x2f20
// for debuging by pickit 3
//#pragma config |= 0x27
// CONFIG1H: CONFIGURATION REGISTER 1 HIGH (BYTE ADDRESS 300001h)
// bit 7 IESO: Internal/External Oscillator Switchover bit
// 1         = Oscillator Switchover mode enabled
// 0         = Oscillator Switchover mode disabled
// bit 6 FCMEN: Fail-Safe Clock Monitor Enable bit
//  1        = Fail-Safe Clock Monitor enabled
//  0        = Fail-Safe Clock Monitor disabled
// bit 5-4 Unimplemented: Read as ‘0’
//   00
// bit 3-0 FOSC<3:0>: Oscillator Selection bits
//      11xx = External RC oscillator, CLKO function on RA6
//      101x = External RC oscillator, CLKO function on RA6
//      1001 = Internal oscillator block, CLKO function on RA6, port function on RA7
//      1000 = Internal oscillator block, port function on RA6 and RA7
//      0111 = External RC oscillator, port function on RA6
//      0110 = HS oscillator, PLL enabled (Clock Frequency = 4 x FOSC1)
//      0101 = EC oscillator, port function on RA6
//      0100 = EC oscillator, CLKO function on RA6
//      0011 = External RC oscillator, CLKO function on RA6
//      0010 = HS oscillator
//      0001 = XT oscillator
//      0000 = LP oscillator
// 0000 1000
#ifdef __18CXX
#pragma config OSC=INTIO2
#else
#pragma config[1] = 0x08
#endif
// CONFIG2L: CONFIGURATION REGISTER 2 LOW (BYTE ADDRESS 300002h)
//  bit 7-5 Unimplemented: Read as ‘0’
// 000
//  bit 4-3 BORV<1:0>: Brown-out Reset Voltage bits(1)
//    1 1   = Minimum setting 
//    0 0   = Maximum setting
// bit 2-1 BOREN<1:0>: Brown-out Reset Enable bits(2)
//       11 = Brown-out Reset enabled in hardware only (SBOREN is disabled)
//       10 = Brown-out Reset enabled in hardware only and disabled in Sleep mode(SBOREN is disabled)
//       01 = Brown-out Reset enabled and controlled by software (SBOREN is enabled)
//       00 = Brown-out Reset disabled in hardware and software
// bit 0 PWRTEN: Power-up Timer Enable bit(2)
//         1 = PWRT disabled 0 = PWRT enabled
//
// 0001 1000
#ifdef __18CXX
#pragma config BOR=OFF
#else
#pragma config[2] = 0x18
#endif
// CONFIG2H: CONFIGURATION REGISTER 2 HIGH (BYTE ADDRESS 300003h)
// bit 7-5 Unimplemented: Read as ‘0’
// bit 4-1 WDTPS<3:0>: Watchdog Timer Postscale Select bits
//    1111 = 1:32,768  1110 = 1:16,384   1101 = 1:8,192 1100 = 1:4,096
//    1011 = 1:2,048   1010 = 1:1,024    1001 = 1:512   1000 = 1:256
//    0111 = 1:128     0110 = 1:64       0101 = 1:32    0100 = 1:16
//    0011 = 1:8       0010 = 1:4        0001 = 1:2     0000 = 1:1
// bit 0 WDTEN: Watchdog Timer Enable bit 1 = WDT enabled 0 = WDT disabled (control is placed on the SWDTEN bit)
// 0000 0000
#ifdef __18CXX
#pragma config WDT=OFF
#else
#pragma config[3] = 0x00
#endif
//  CONFIG3H: CONFIGURATION REGISTER 3 HIGH (BYTE ADDRESS 300005h)
// bit 7 MCLRE: MCLR Pin Enable bit
// 1        = MCLR pin enabled; RE3 input pin disabled 
// 0        = RE3 input pin enabled; MCLR disabled
// bit 6-3 Unimplemented: Read as ‘0’
//  000 0
// bit 2 LPT1OSC: Low-Power Timer1 Oscillator Enable bit 
//       1   = Timer1 configured for low-power operation
//       0   = Timer1 configured for higher power operation
// bit 1 PBADEN: PORTB A/D Enable bit (Affects ADCON1 Reset state. ADCON1 controls PORTB<4:0> pin configuration.)
//        1  = PORTB<4:0> pins are configured as analog input channels on Reset
//        0  = PORTB<4:0> pins are configured as digital I/O on Reset
// bit 0 CCP2MX: CCP2 MUX bit 
//         1 = CCP2 input/output is multiplexed with RC1 
//         0 = CCP2 input/output is multiplexed with RB3
// 1000 0001


#ifdef __18CXX
   //#pragma config MCLRE=ON, CCP2MX=ON
   #pragma config MCLRE=ON
#else
   #pragma config[5] = 0x81
#endif
// CONFIG4L: CONFIGURATION REGISTER 4 LOW (BYTE ADDRESS 300006h)
// bit 7 DEBUG: Background Debugger Enable bit
//  1        = Background debugger disabled, RB6 and RB7 configured as general purpose I/O pins
//  0        = Background debugger enabled, RB6 and RB7 are dedicated to in-circuit debug
// bit 6 XINST: Extended Instruction Set Enable bit
//   1       = Instruction set extension and Indexed Addressing mode enabled
//   0       = Instruction set extension and Indexed Addressing mode disabled (Legacy mode)
// bit 5-4 BBSIZ<1:0>: Boot Block Size Select bits
//         PIC18F4221/4321 Devices:
//    1x     = 1024 Words
//    01     = 512 Words
//    00     = 256 Words
//         PIC18F2221/2321 Devices:
//    1x     = 512 Words
//    x1     = 512 Words
//    00     = 256 Words
// bit 3 Reserved: Maintain as ‘0’
// bit 2 LVP: Single-Supply ICSP™ Enable bit 
//       1   = Single-Supply ICSP enabled 
//       0   = Single-Supply ICSP disabled
// bit 1 Unimplemented: Read as ‘0’
//        0
// bit 0 STVREN: Stack Full/Underflow Reset Enable bit 
//         1 = Stack full/underflow will cause Reset
//         0 = Stack full/underflow will not cause Reset
// 0100 0100
#ifdef __DEBUG
    #ifdef __18CXX
        #pragma config XINST=ON, LVP=ON
    #else
        #pragma config[6] = 0x00
    #endif
#else
    #ifdef __18CXX
        #pragma config XINST=ON, LVP=ON
    #else
        #pragma config[6] = 0x80
    #endif
#endif
// CONFIG5L: CONFIGURATION REGISTER 5 LOW (BYTE ADDRESS 300008h)
// bit 7-2 Unimplemented: Read as ‘0’
// bit 1 CP1: Code Protection bit 
//        1  = Block 1 not code-protected 
//        0  = Block 1 code-protected(1)
// bit 0 CP0: Code Protection bit 
//         1 = Block 0 not code-protected 
//         0 = Block 0 code-protected(1)
// 0000 0011
#ifdef __18CXX
#pragma config CP0=OFF, CP1=OFF
#else
#pragma config[8] = 0x03
#endif
// CONFIG5H: CONFIGURATION REGISTER 5 HIGH (BYTE ADDRESS 300009h)
// bit 7 CPD: Data EEPROM Code Protection bit 
// 1          = Data EEPROM not code-protected 
// 0          = Data EEPROM code-protected
// bit 6 CPB: Boot Block Code Protection bit 
//  1         = Boot block not code-protected 
//  0         = Boot block code-protected(1)
// bit 5-0 Unimplemented: Read as ‘0’
// 1100 0000
#ifdef __18CXX
#pragma config CPD=OFF, CPB=OFF
#else
#pragma config[9] = 0xc0
#endif
// CONFIG6L: CONFIGURATION REGISTER 6 LOW (BYTE ADDRESS 30000Ah)
// bit 7-2 Unimplemented: Read as ‘0’
// bit 1 WRT1: Write Protection bit 
//         1  = Block 1 not write-protected 
//         0  = Block 1 write-protected
// bit 0 WRT0: Write Protection bit 
//          1 = Block 0 not write-protected 
//          0 = Block 0 write-protected
// 00000 0011
#ifdef __18CXX
#pragma config WRT1=OFF, WRT0=OFF
#else
#pragma config[0xa] = 0x03
#endif
// CONFIG6H: CONFIGURATION REGISTER 6 HIGH (BYTE ADDRESS 30000Bh)
// bit 7 WRTD: Data EEPROM Write Protection bit 
// 1        = Data EEPROM not write-protected 
// 0        = Data EEPROM write-protected
// bit 6 WRTB: Boot Block Write Protection bit 
//  1       = Boot block not write-protected 
//  0       = Boot block write-protected(2)
// bit 5 WRTC: Configuration Register Write Protection bit 
//   1      = Configuration registers (300000-3000FFh) not write-protected 
//   0      = Configuration registers (300000-3000FFh) write-protected
// bit 4-0 Unimplemented: Read as ‘0’
//    0 0000
// 1110 0000
#ifdef __18CXX
#pragma config WRTD=OFF, WRTB=OFF, WRTC=OFF
#else
#pragma config[0xb] = 0xe3
#endif
//
// CONFIG7L: CONFIGURATION REGISTER 7 LOW (BYTE ADDRESS 30000Ch)
// bit 7-2 Unimplemented: Read as ‘0’
// bit 1 EBTR1: Table Read Protection bit 
//        1  = Block 1 not protected from table reads executed in other blocks 
//        0  = Block 1 protected from table reads executed in other blocks
// bit 0 EBTR0: Table Read Protection bit 
//         1 = Block 0 not protected from table reads executed in other blocks 
//         0 = Block 0 protected from table reads executed in other blocks
// 0000 0011
#ifdef __18CXX
#pragma config EBTR1=OFF, EBTR0=OFF
#else
#pragma config[0xc] = 0x03
#endif
// CONFIG7H: CONFIGURATION REGISTER 7 HIGH (BYTE ADDRESS 30000Dh)
// bit 7 Unimplemented: Read as ‘0’
// bit 6 EBTRB: Boot Block Table Read Protection bit 
//  1      = Boot block not protected from table reads executed in other blocks 
//  0      = Boot block protected from table reads executed in other blocks(1)
// bit 5-0 Unimplemented: Read as ‘0’
// 0100 0000
#ifdef __18CXX
#pragma config EBTRB=OFF
#else
#pragma config[0xd] = 0x40
#endif
#endif
//////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifdef _18F25K20
#ifdef __18CXX
#include "p18f25k20.h"
#define rambank varlocate
#endif
///////////////////////////////////////////////////////////////////////////////////////////
// this line for debug and may be normal operations
// on pin 15 (RA6) no oscilator output  
//#pragma config PWRTE=on, WDTE=off, FOSC=4, BODEN=off
//#define DEBUG_OSC_PIN15 1

// this line for pin 15 (RA6) oscilator output 2MHz
//#pragma config PWRTE=off, WDTE=off, FOSC=5, BODEN=off
//#pragma config |= 0x2f20
// for debuging by pickit 3
//#pragma config |= 0x27
// CONFIG1H: CONFIGURATION REGISTER 1 HIGH (BYTE ADDRESS 300001h)
// bit 7 IESO: Internal/External Oscillator Switchover bit
//   1         = Oscillator Switchover mode enabled
//   0         = Oscillator Switchover mode disabled
// bit 6 FCMEN: Fail-Safe Clock Monitor Enable bit
//    1        = Fail-Safe Clock Monitor enabled
//    0        = Fail-Safe Clock Monitor disabled
// bit 5-4 Unimplemented: Read as ‘0’
// bit 3-0 FOSC<3:0>: Oscillator Selection bits
//        11xx = External RC oscillator, CLKOUT function on RA6
//        101x = External RC oscillator, CLKOUT function on RA6
//        1001 = Internal oscillator block, CLKOUT function on RA6, port function on RA7
//        1000 = Internal oscillator block, port function on RA6 and RA7
//        0111 = External RC oscillator, port function on RA6
//        0110 = HS oscillator, PLL enabled (Clock Frequency = 4 x FOSC1)
//        0101 = EC oscillator, port function on RA6
//        0100 = EC oscillator, CLKOUT function on RA6
//        0011 = External RC oscillator, CLKOUT function on RA6
//        0010 = HS oscillator
//        0001 = XT oscillator
//        0000 = LP oscillator
//   0000 0110
#ifdef __18CXX
#pragma config OSC=INTIO2
#else
//#pragma config[1] = 0x08
#ifdef DEBUG_SIM
#pragma config[1] = 0x08
#else
#pragma config[1] = 0x06
#endif
#endif
// CONFIG2L: CONFIGURATION REGISTER 2 LOW (BYTE ADDRESS 300002h)
// bit 7-5 Unimplemented: Read as ‘0’
//    000
// bit 4-3 BORV<1:0>: Brown-out Reset Voltage bits(1)
//       11    = VBOR set to 1.8V nominal
//       10    = VBOR set to 2.2V nominal
//       01    = VBOR set to 2.7V nominal
//       00    = VBOR set to 3.0V nominal
// bit 2-1 BOREN<1:0>: Brown-out Reset Enable bits(2)
//          11 = Brown-out Reset enabled in hardware only (SBOREN is disabled)
//          10  = Brown-out Reset enabled in hardware only and disabled in Sleep mode 
//                   (SBOREN is disabled) 
//          01  = Brown-out Reset enabled and controlled by software (SBOREN is enabled) 
//          00  = Brown-out Reset disabled in hardware and software
// bit 0 PWRTEN: Power-up Timer Enable bit(2)
//            1 = PWRT disabled 
//            0 = PWRT enabled 
//    0001 1000
#ifdef __18CXX
#pragma config BOR=OFF
#else
#pragma config[2] = 0x11
#endif
// CONFIG2H: CONFIGURATION REGISTER 2 HIGH (BYTE ADDRESS 300003h)
// bit 7-5      Unimplemented: Read as ‘0’
// 000
// bit 4-1 WDTPS<3:0>: Watchdog Timer Postscale Select bits
//    1111 = 1:32,768    1110 = 1:16,384    1101 = 1:8,192    1100 = 1:4,096
//    1011 = 1:2,048     1010 = 1:1,024     1001 = 1:512      1000 = 1:256
//    0111 = 1:128       0110 = 1:64        0101 = 1:32       0100 = 1:16
//    0011 = 1:8         0010 = 1:4         0001 = 1:2
//    0000 = 1:1
// bit 0 WDTEN: Watchdog Timer Enable bit
//         1 = WDT is always enabled. SWDTEN bit has no effect
//         0 = WDT is controlled by SWDTEN bit of the WDTCON register
// 0000 0000
#ifdef __18CXX
#pragma config WDT=OFF
#else
#pragma config[3] = 0x00
#endif
//  CONFIG3H: CONFIGURATION REGISTER 3 HIGH (BYTE ADDRESS 300005h)
// bit 7 MCLRE: MCLR Pin Enable bit 
// 1 = MCLR pin enabled; RE3 input pin disabled
// 0 = RE3 input pin enabled; MCLR disabled
// bit 6-4 Unimplemented: Read as ‘0’
//  000
// bit 3 HFOFST: HFINTOSC Fast Start-up
//      1    = HFINTOSC starts clocking the CPU without waiting for the oscillator to stabilize.
//      0    = The system clock is held off until the HFINTOSC is stable.
// bit 2 LPT1OSC: Low-Power Timer1 Oscillator Enable bit
//       1   = Timer1 configured for low-power operation
//       0   = Timer1 configured for higher power operation
// bit 1 PBADEN: PORTB A/D Enable bit (Affects ANSELH Reset state. ANSELH controls PORTB<4:0> pin configuration.)
//        1  = PORTB<4:0> pins are configured as analog input channels on Reset
//        0  = PORTB<4:0> pins are configured as digital I/O on Reset
// bit 0 CCP2MX: CCP2 MUX bit
//         1 = CCP2 input/output is multiplexed with RC1 
//         0 = CCP2 input/output is multiplexed with RB3 
// 1000 1000


#ifdef __18CXX
   //#pragma config MCLRE=ON, CCP2MX=ON
   #pragma config MCLRE=ON
#else
   #pragma config[5] = 0x88
#endif
// CONFIG4L: CONFIGURATION REGISTER 4 LOW (BYTE ADDRESS 300006h)
// bit 7 DEBUG: Background Debugger Enable bit
// 1         = Background debugger disabled, RB6 and RB7 configured as general purpose I/O pins 
// 0         = Background debugger enabled, RB6 and RB7 are dedicated to In-Circuit Debug
// bit 6 XINST: Extended Instruction Set Enable bit
//  1        = Instruction set extension and Indexed Addressing mode enabled 
//  0        = Instruction set extension and Indexed Addressing mode disabled (Legacy mode)
// bit 5-3 Unimplemented: Read as ‘0’ 
//   00 0
// bit 2 LVP: Single-Supply ICSP Enable bit
//       1   = Single-Supply ICSP enabled 
//       0   = Single-Supply ICSP disabled 
// bit 1 Unimplemented: Read as ‘0’ 
//        0
// bit 0 STVREN: Stack Full/Underflow Reset Enable bit
//         1 = Stack full/underflow will cause Reset 
//         0 = Stack full/underflow will not cause Reset
// 0100 0100
#ifdef __DEBUG
    #ifdef __18CXX
        #pragma config XINST=ON, LVP=ON
    #else
        #pragma config[6] = 0x00
    #endif
#else
    #ifdef __18CXX
        #pragma config XINST=ON, LVP=ON
    #else
        #pragma config[6] = 0x80
    #endif
#endif
// CONFIG5L: CONFIGURATION REGISTER 5 LOW (BYTE ADDRESS 300008h)
// bit 7-4 Unimplemented: Read as ‘0’
// 0000
// bit 3 CP3: Code Protection bit(1)
//      1    = Block 3 not code-protected 
//      0    = Block 3 code-protected
// bit 2 CP2: Code Protection bit(1)
//       1   = Block 2 not code-protected 
//       0   = Block 2 code-protected 
// bit 1 CP1: Code Protection bit
//        1  = Block 1 not code-protected 
//        0  = Block 1 code-protected 
// bit 0 CP0: Code Protection bit
//         1 = Block 0 not code-protected 
//         0 = Block 0 code-protected
// 0000 1111
#ifdef __18CXX
#pragma config CP0=OFF, CP1=OFF
#else
#pragma config[8] = 0x0f
#endif
// CONFIG5H: CONFIGURATION REGISTER 5 HIGH (BYTE ADDRESS 300009h)
// bit 7 CPD: Data EEPROM Code Protection bit
// 1         = Data EEPROM not code-protected
// 0         = Data EEPROM code-protected
// bit 6 CPB: Boot Block Code Protection bit
//  1        = Boot Block not code-protected
//  0        = Boot Block code-protected
// bit 5-0 Unimplemented: Read as ‘0’
//   00 0000
// 1100 0000
#ifdef __18CXX
#pragma config CPD=OFF, CPB=OFF
#else
#pragma config[9] = 0xc0
#endif
// CONFIG6L: CONFIGURATION REGISTER 6 LOW (BYTE ADDRESS 30000Ah)
// bit 7-4 Unimplemented: Read as ‘0’
// 0000
// bit 3 WRT3: Write Protection bit(1)
//      1    = Block 3 not write-protected 
//      0    = Block 3 write-protected
// bit 2 WRT2: Write Protection bit(1)
//       1   = Block 2 not write-protected
//       0   = Block 2 write-protected
// bit 1 WRT1: Write Protection bit
//        1  = Block 1 not write-protected
//        0  = Block 1 write-protected
// bit 0 WRT0: Write Protection bit
//         1 = Block 0 not write-protected
//         0 = Block 0 write-protected
// 00000 1111
#ifdef __18CXX
#pragma config WRT1=OFF, WRT0=OFF
#else
#pragma config[0xa] = 0x0f
#endif
// CONFIG6H: CONFIGURATION REGISTER 6 HIGH (BYTE ADDRESS 30000Bh)
// bit 7 WRTD: Data EEPROM Write Protection bit
// 1         = Data EEPROM not write-protected
// 0         = Data EEPROM write-protected
// bit 6 WRTB: Boot Block Write Protection bit
//  1        = Boot Block not write-protected
//  0        = Boot Block write-protected
// bit 5 WRTC: Configuration Register Write Protection bit(1)
//   1       = Configuration registers not write-protected
//   0       = Configuration registers write-protected
// bit 4-0 Unimplemented: Read as ‘0’
//    0 0000

// 1110 0000
#ifdef __18CXX
#pragma config WRTD=OFF, WRTB=OFF, WRTC=OFF
#else
#pragma config[0xb] = 0xe0
#endif
//
// CONFIG7L: CONFIGURATION REGISTER 7 LOW (BYTE ADDRESS 30000Ch)
// bit 7-4 Unimplemented: Read as ‘0’
// 0000
// bit 3 EBTR3: Table Read Protection bit(1)
//      1    = Block 3 not protected from table reads executed in other blocks
//      0    = Block 3 protected from table reads executed in other blocks
// bit 2 EBTR2: Table Read Protection bit(1)
//       1   = Block 2 not protected from table reads executed in other blocks 
//       0   = Block 2 protected from table reads executed in other blocks 
// bit 1 EBTR1: Table Read Protection bit
//        1  = Block 1 not protected from table reads executed in other blocks 
//        0  = Block 1 protected from table reads executed in other blocks 
// bit 0 EBTR0: Table Read Protection bit
//         1 = Block 0 not protected from table reads executed in other blocks 
//         0 = Block 0 protected from table reads executed in other blocks
// 0000 1111
#ifdef __18CXX
#pragma config EBTR1=OFF, EBTR0=OFF
#else
#pragma config[0xc] = 0x0f
#endif
// CONFIG7H: CONFIGURATION REGISTER 7 HIGH (BYTE ADDRESS 30000Dh)
// bit 7 Unimplemented: Read as ‘0’
// 0
// bit 6 EBTRB: Boot Block Table Read Protection bit
//  1 = Boot Block not protected from table reads executed in other blocks
//  0 = Boot Block protected from table reads executed in other blocks
// bit 5-0 Unimplemented: Read as ‘0
//   00 0000
// bit 5-0 Unimplemented: Read as ‘0’
// 0100 0000
#ifdef __18CXX
#pragma config EBTRB=OFF
#else
#pragma config[0xd] = 0x40
#endif
#endif
//////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifdef __PIC24HJ64GP502__
   // configuration
   _FBS( RBS_NO_RAM & BSS_NO_BOOT_CODE & BWRP_WRPROTECT_OFF);
   // Boot Segment RAM Code Protection Size
   //   11        = No Boot RAM defined
   //   10        = Boot RAM is 128 bytes
   //   01        = Boot RAM is 256 bytes
   //   00        = Boot RAM is 1024 bytes
   // Boot Segment Program Flash Code Protection Size
   //       X11   = No Boot program Flash segment 
   //            Boot space is 1K Instruction Words (except interrupt vectors)
   //       110   = Standard security; boot program Flash segment ends at 0x0007FE
   //       010   = High security; boot program Flash segment ends at 0x0007FE 
   //            Boot space is 4K Instruction Words (except interrupt vectors)
   //       101   = Standard security; boot program Flash segment, ends at 0x001FFE
   //       001   = High security; boot program Flash segment ends at 0x001FFE
   //            Boot space is 8K Instruction Words (except interrupt vectors)
   //       100   = Standard security; boot program Flash segment ends at 0x003FFE
   //       000   = High security; boot program Flash segment ends at 0x003FFE
   // Boot Segment Program Flash Write Protection 
   //          1  = Boot segment can be written 
   //          0  = Boot segment is write-protected
   _FSS(RSS_NO_RAM & SSS_NO_FLASH & SWRP_WRPROTECT_OFF); 
   // Secure Segment RAM Code Protection
   //   11        = No Secure RAM defined
   //   10        = Secure RAM is 256 Bytes less BS RAM
   //   01        = Secure RAM is 2048 Bytes less BS RAM
   //   00        = Secure RAM is 4096 Bytes less BS RAM
   // Secure Segment Program Flash Code Protection Size (Secure segment is not implemented on 32K devices)
   //       X11   = No Secure program flash segment 
   //            Secure space is 4K IW less BS
   //       110   = Standard security; secure program flash segment startsat End of BS, ends at 0x001FFE
   //       010   = High security; secure program flash segment starts at End of BS, ends at 0x001FFE
   //            Secure space is 8K IW less BS
   //       101   = Standard security; secure program flash segment starts at End of BS, ends at 0x003FFE
   //       001   = High security; secure program flash segment starts at End of BS, ends at 0x003FFE
   //            Secure space is 16K IW less BS
   //       100   = Standard security; secure program flash segment starts at End of BS, ends at 007FFEh
   //       000   = High security; secure program flash segment starts at End of BS, ends at 0x007FFE
   // Secure Segment Program Flash Write-Protect bit
   //          1  = Secure Segment can bet written
   //          0  = Secure Segment is write-protected
   _FGS(GSS_OFF & GCP_OFF & GWRP_OFF);
   // General Segment Code-Protect bit
   //        11   = User program memory is not code-protected
   //        10   = Standard security
   //        0x   = High security
   // General Segment Write-Protect bit
   //          1  = User program memory is not write-protected
   //          0  = User program memory is write-protected
   _FOSCSEL(FNOSC_FRCPLL);
// & IESO_ON);
   // Two-speed Oscillator Start-up Enable bit
   //   1         = Start-up device with FRC, then automatically switch to the user-selected oscillator source when ready
   //   0         = Start-up device with user-selected oscillator source			
   // Initial Oscillator Source Selection bits
   //        111  = Internal Fast RC (FRC) oscillator with postscaler
   //        110  = Internal Fast RC (FRC) oscillator with divide-by-16
   //        101  = LPRC oscillator
   //        100  = Secondary (LP) oscillator
   //        011  = Primary (XT, HS, EC) oscillator with PLL
   //        010  = Primary (XT, HS, EC) oscillator
   //        001  = Internal Fast RC (FRC) oscillator with PLL
   //        000  = FRC oscillator
   _FOSC(FCKSM_CSECME & IOL1WAY_OFF & OSCIOFNC_ON & POSCMD_NONE);
   // Clock Switching Mode bits
   //   1x        = Clock switching is disabled, Fail-Safe Clock Monitor is disabled
   //   01        = Clock switching is enabled, Fail-Safe Clock Monitor is disabled
   //   00        = Clock switching is enabled, Fail-Safe Clock Monitor is enabled
   // Peripheral pin select configuration
   //     1       = Allow only one reconfiguration
   //     0       = Allow multiple reconfigurations
   // OSC2 Pin Function bit (except in XT and HS modes)
   //        1    = OSC2 is clock output
   //        0    = OSC2 is general purpose digital I/O pin
   // Primary Oscillator Mode Select bits
   //         11  = Primary oscillator disabled
   //         10  = HS Crystal Oscillator mode
   //         01  = XT Crystal Oscillator mode
   //         00  = EC (External Clock) mode
   _FWDT(FWDTEN_OFF);
   // Watchdog Timer Enable bit
   //   1         = Watchdog Timer always enabled (LPRC oscillator cannot be disabled. Clearing the SWDTEN bit in the RCON register has no effect.)
   //   0         = Watchdog Timer enabled/disabled by user software (LPRC can be disabled by clearing the SWDTEN bit in the RCON register)
   // Watchdog Timer Window Enable bit
   //    1        = Watchdog Timer in Non-Window mode
   //    0        = Watchdog Timer in Window mode
   // Watchdog Timer Prescaler bit
   //      1       = 1:128
   //      0       = 1:32
   // Watchdog Timer Postscaler bits
   //       1111   = 1:32,768
   //       1110   = 1:16,384
   //       0001   = 1:2
   //       0000   = 1:1
   _FPOR(ALTI2C_OFF & FPWRT_PWR1);
   // Alternate I2C™ pins
   //      1       = I2C mapped to SDA1/SCL1 pins
   //      0       = I2C mapped to ASDA1/ASCL1 pins
   // Power-on Reset Timer Value Select bits
   //       111    = PWRT = 128 ms
   //       110    = PWRT = 64 ms
   //       101    = PWRT = 32 ms
   //       100    = PWRT = 16 ms
   //       011    = PWRT = 8 ms
   //       010    = PWRT = 4 ms
   //       001    = PWRT = 2 ms
   //       000    = PWRT = Disabled
   _FICD(ICS_PGD1 & JTAGEN_OFF)
   // JTAG Enable bit
   //     1        = JTAG enabled
   //     0        = JTAG disabled
   // ICD Communication Channel Select bits
   //         11   = Communicate on PGEC1 and PGED1
   //         10   = Communicate on PGEC2 and PGED2
   //         01   = Communicate on PGEC3 and PGED3
   //         00   = Reserved, do not use

   _FUID0('G');
   _FUID1('Y');
   _FUID2('R');
   _FUID3('O');
   
#endif
//////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifdef __PIC24HJ64GP504__
   // configuration
   _FBS( RBS_NO_RAM & BSS_NO_BOOT_CODE & BWRP_WRPROTECT_OFF);
   // Boot Segment RAM Code Protection Size
   //   11        = No Boot RAM defined
   //   10        = Boot RAM is 128 bytes
   //   01        = Boot RAM is 256 bytes
   //   00        = Boot RAM is 1024 bytes
   // Boot Segment Program Flash Code Protection Size
   //       X11   = No Boot program Flash segment 
   //            Boot space is 1K Instruction Words (except interrupt vectors)
   //       110   = Standard security; boot program Flash segment ends at 0x0007FE
   //       010   = High security; boot program Flash segment ends at 0x0007FE 
   //            Boot space is 4K Instruction Words (except interrupt vectors)
   //       101   = Standard security; boot program Flash segment, ends at 0x001FFE
   //       001   = High security; boot program Flash segment ends at 0x001FFE
   //            Boot space is 8K Instruction Words (except interrupt vectors)
   //       100   = Standard security; boot program Flash segment ends at 0x003FFE
   //       000   = High security; boot program Flash segment ends at 0x003FFE
   // Boot Segment Program Flash Write Protection 
   //          1  = Boot segment can be written 
   //          0  = Boot segment is write-protected
   _FSS(RSS_NO_RAM & SSS_NO_FLASH & SWRP_WRPROTECT_OFF); 
   // Secure Segment RAM Code Protection
   //   11        = No Secure RAM defined
   //   10        = Secure RAM is 256 Bytes less BS RAM
   //   01        = Secure RAM is 2048 Bytes less BS RAM
   //   00        = Secure RAM is 4096 Bytes less BS RAM
   // Secure Segment Program Flash Code Protection Size (Secure segment is not implemented on 32K devices)
   //       X11   = No Secure program flash segment 
   //            Secure space is 4K IW less BS
   //       110   = Standard security; secure program flash segment startsat End of BS, ends at 0x001FFE
   //       010   = High security; secure program flash segment starts at End of BS, ends at 0x001FFE
   //            Secure space is 8K IW less BS
   //       101   = Standard security; secure program flash segment starts at End of BS, ends at 0x003FFE
   //       001   = High security; secure program flash segment starts at End of BS, ends at 0x003FFE
   //            Secure space is 16K IW less BS
   //       100   = Standard security; secure program flash segment starts at End of BS, ends at 007FFEh
   //       000   = High security; secure program flash segment starts at End of BS, ends at 0x007FFE
   // Secure Segment Program Flash Write-Protect bit
   //          1  = Secure Segment can bet written
   //          0  = Secure Segment is write-protected
   _FGS(GSS_OFF & GCP_OFF & GWRP_OFF);
   // General Segment Code-Protect bit
   //        11   = User program memory is not code-protected
   //        10   = Standard security
   //        0x   = High security
   // General Segment Write-Protect bit
   //          1  = User program memory is not write-protected
   //          0  = User program memory is write-protected
   _FOSCSEL(FNOSC_FRCPLL);
// & IESO_ON);
   // Two-speed Oscillator Start-up Enable bit
   //   1         = Start-up device with FRC, then automatically switch to the user-selected oscillator source when ready
   //   0         = Start-up device with user-selected oscillator source			
   // Initial Oscillator Source Selection bits
   //        111  = Internal Fast RC (FRC) oscillator with postscaler
   //        110  = Internal Fast RC (FRC) oscillator with divide-by-16
   //        101  = LPRC oscillator
   //        100  = Secondary (LP) oscillator
   //        011  = Primary (XT, HS, EC) oscillator with PLL
   //        010  = Primary (XT, HS, EC) oscillator
   //        001  = Internal Fast RC (FRC) oscillator with PLL
   //        000  = FRC oscillator
   _FOSC(FCKSM_CSECME & IOL1WAY_OFF & OSCIOFNC_ON & POSCMD_NONE);
   // Clock Switching Mode bits
   //   1x        = Clock switching is disabled, Fail-Safe Clock Monitor is disabled
   //   01        = Clock switching is enabled, Fail-Safe Clock Monitor is disabled
   //   00        = Clock switching is enabled, Fail-Safe Clock Monitor is enabled
   // Peripheral pin select configuration
   //     1       = Allow only one reconfiguration
   //     0       = Allow multiple reconfigurations
   // OSC2 Pin Function bit (except in XT and HS modes)
   //        1    = OSC2 is clock output
   //        0    = OSC2 is general purpose digital I/O pin
   // Primary Oscillator Mode Select bits
   //         11  = Primary oscillator disabled
   //         10  = HS Crystal Oscillator mode
   //         01  = XT Crystal Oscillator mode
   //         00  = EC (External Clock) mode
   _FWDT(FWDTEN_OFF);
   // Watchdog Timer Enable bit
   //   1         = Watchdog Timer always enabled (LPRC oscillator cannot be disabled. Clearing the SWDTEN bit in the RCON register has no effect.)
   //   0         = Watchdog Timer enabled/disabled by user software (LPRC can be disabled by clearing the SWDTEN bit in the RCON register)
   // Watchdog Timer Window Enable bit
   //    1        = Watchdog Timer in Non-Window mode
   //    0        = Watchdog Timer in Window mode
   // Watchdog Timer Prescaler bit
   //      1       = 1:128
   //      0       = 1:32
   // Watchdog Timer Postscaler bits
   //       1111   = 1:32,768
   //       1110   = 1:16,384
   //       0001   = 1:2
   //       0000   = 1:1
   _FPOR(ALTI2C_OFF & FPWRT_PWR1);
   // Alternate I2C™ pins
   //      1       = I2C mapped to SDA1/SCL1 pins
   //      0       = I2C mapped to ASDA1/ASCL1 pins
   // Power-on Reset Timer Value Select bits
   //       111    = PWRT = 128 ms
   //       110    = PWRT = 64 ms
   //       101    = PWRT = 32 ms
   //       100    = PWRT = 16 ms
   //       011    = PWRT = 8 ms
   //       010    = PWRT = 4 ms
   //       001    = PWRT = 2 ms
   //       000    = PWRT = Disabled
   _FICD(ICS_PGD1 & JTAGEN_OFF)
   // JTAG Enable bit
   //     1        = JTAG enabled
   //     0        = JTAG disabled
   // ICD Communication Channel Select bits
   //         11   = Communicate on PGEC1 and PGED1
   //         10   = Communicate on PGEC2 and PGED2
   //         01   = Communicate on PGEC3 and PGED3
   //         00   = Reserved, do not use

   _FUID0('M');
   _FUID1('A');
   _FUID2('I');
   _FUID3('N');
   
#endif
//////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////

      
#define bitset(i,j) i |= 1U << j
#define bitclr(i,j) i &= ~(1U << j)
#define bittest(i,j) (i & (1U<<j))
#define bset(i,j) i.j=1
#define bclr(i,j) i.j=0
#define btest(i,j) (i.j)

#pragma rambank RAM_BANK_2
///////////////////////////////////////BANK 2//////////////////////
#ifdef BT_TIMER1
unsigned long Timer1Cnt;
#else
unsigned int TIMER1_ADJUST;
unsigned char TIMER1_ADJ0;

unsigned char TMR130;
unsigned char TMR1SEC;
unsigned char TMR1MIN;
unsigned char TMR1HOUR;
unsigned char TMR1DAY;
unsigned char TMR1YEAR;

//#ifdef __18CXX
struct _TimerB1{
unsigned SetSyncTime:1;
unsigned SetSleep:1;
} TimerB1;
//#else
//bit SetSyncTime;
//bit SetSleep;
//#endif

unsigned char setTMR130;
unsigned char setTMR1SEC;
unsigned char setTMR1MIN;
unsigned char setTMR1HOUR;
unsigned char setTMR1DAY;
#endif

#pragma rambank RAM_BANK_0
/////////////////////////////////////BANK 0///////////////////////
#define CRITICAL_BUF_SIZE 4
#ifndef BUFFER_LEN
 #ifdef _18F2321_18F25K20
   #define BUFFER_LEN 18
   #define OUT_BUFFER_LEN 18
 #else
   #ifdef __PIC24H__
     #define BUFFER_LEN 20
     #define OUT_BUFFER_LEN 20
   #else
     #define BUFFER_LEN 14
     #define OUT_BUFFER_LEN 14
   #endif
 #endif
#endif

// 0 byte == iInQuSize
// 1 byte == iPtr1InQu
// 2 byte == iPtr2InQu
// 3-BUFFER_LEN queue
struct AQueue
{
    WORD iEntry;
    WORD iQueueSize;
    WORD iExit;
    unsigned char Queue[BUFFER_LEN];
};
struct BQueue
{
    WORD iEntry;
    WORD iQueueSize;
    WORD iExit;
    unsigned char Queue[OUT_BUFFER_LEN];
};
VOLATILE struct AQueue AInQu;
VOLATILE struct BQueue AOutQu;
#ifdef RX_FULL
#define OVER_BUFFER_LEN 5
VOLATILE struct OLQueue
{
    WORD iEntry;
    WORD iQueueSize;
    WORD iExit;
    unsigned char Queue[OVER_BUFFER_LEN];
} OverLoad;
#endif
#ifdef USE_COM2
VOLATILE struct AQueue AInQuCom2;
VOLATILE struct BQueue AOutQuCom2;
#endif

#define MAX_ADR '9'
#define MIN_ADR '1'
//#define ESC_SYMB '#'
#define ESC_SYMB 0x23

unsigned char UnitADR;
unsigned char UnitFrom;
unsigned char SendCMD;


struct _MainB2{
unsigned RetransmitTo:1;
#ifdef NON_STANDART_MODEM
unsigned SendOverLink:1;
unsigned SendOverLinkAndProc:1;
unsigned SendOverLinkStarted:1;
unsigned SendOverlinkWasESC:1;
unsigned FlashRQ:1;
unsigned PingRQ:1;
unsigned PingRSPRQ:1;
VOLATILE unsigned DoPing:1;
VOLATILE unsigned ConstantPing:1;
#endif
unsigned SendComOneByte:1;
VOLATILE unsigned OutPacket:1;
VOLATILE unsigned OutPacketESC:1;
VOLATILE unsigned OutPacketLenNull:1;

VOLATILE unsigned GetPkt:1;
VOLATILE unsigned GetPktESC:1;
VOLATILE unsigned GetPktLenNull:1;
VOLATILE unsigned RelayPktLenNull:1;
VOLATILE unsigned RelayGranted:1;
VOLATILE unsigned RelayPktOverload:1;
VOLATILE unsigned RelayPktESC:1;
unsigned getCMD:1;
unsigned ESCNextByte:1;
unsigned LastWasUnitAddr:1;
VOLATILE unsigned LockToQueue:1;
unsigned PrepI2C:1;
unsigned CommLoopOK:1;
unsigned SetFromAddr:1;
unsigned SetSendCMD:1;
unsigned SendWithEsc:1;
#ifdef USE_COM2
unsigned SendCom2WithEsc:1;
#endif

unsigned DoneWithCMD:1;
#ifndef NO_I2C_PROC
unsigned ComNotI2C:1;
#endif

//VOLATILE unsigned prepStream:1;
//VOLATILE unsigned prepCmd:1;
//VOLATILE unsigned InComRetransmit:1;
//VOLATILE unsigned InComZeroLenMsg:1;
#ifdef EXT_INT
VOLATILE unsigned ExtInterrupt:1;
VOLATILE unsigned ExtInterrupt1:1;
VOLATILE unsigned ExtInterrupt2:1;
VOLATILE unsigned InDoneNoSleep:1;
VOLATILE unsigned ExtFirst:1;
#endif
} Main;


unsigned char PingAttempts;
VOLATILE unsigned char OutPacketUnit;
VOLATILE unsigned char RelayPkt;
VOLATILE unsigned char AllowMask;
VOLATILE unsigned char UnitMask1;
VOLATILE unsigned char UnitMask2;

//bit BlockComm;
// this is in BANK0
struct _I2CB3_B4{
unsigned LockToI2C:1;
unsigned ESCI2CChar:1;
unsigned WaitQuToEmp:1;
unsigned NextI2CRead:1;
unsigned SetI2CYesNo:1;
unsigned EchoWhenI2C:1;

//} I2C_B3;
//struct _I2CB4{
VOLATILE unsigned I2CGettingPKG:1;
unsigned I2CReplyExpected:1;
unsigned RetransI2ComCSet:1;
unsigned Timer0Fired:1;


unsigned AddresWasSend:1;
} I2C;

#pragma rambank RAM_BANK_1
///////////////////////////////////////BANK 1//////////////////////

// this is in BANK1
struct _I2CB4_B5{
unsigned I2Cread:1;
VOLATILE unsigned I2CBusBusy:1;
#ifdef I2C_INT_SUPPORT
unsigned NeedMaster:1;
unsigned NeedRestart:1;
unsigned NeedStop:1;
unsigned NeedReceive:1;
unsigned NeedSend:1;
unsigned SendACK_NAK:1;
#endif
VOLATILE unsigned I2CMasterDone:1;
} I2C_B1;

#ifndef NO_I2C_PROC
unsigned char I2Caddr;
VOLATILE struct AQueue AInI2CQu;
VOLATILE struct BQueue AOutI2CQu;
#endif

#pragma rambank RAM_BANK_0
///////////////////////////////////////BANK 0//////////////////////

unsigned char LenI2CRead;
unsigned char I2CReplyCMD;
#ifdef __PIC24H__
rtccTimeDate RtccTimeDate;
rtccTimeDate RtccTimeDateVal;
#endif

//bit ReTransToI2C;
#ifndef _WIN32
unsigned char eeprom_read(unsigned char addr);
void eeprom_write(unsigned char addr, unsigned char value);
#endif


#define SPBRG_4800_40MIPS 2064

#define SPBRG_9600 51
#define SPBRG_9600_40MIPS 1040

#define SPBRG_19200 25

#define SPBRG_19200_8MHZ 25
#define SPBRG_19200_16MHZ 51
#define SPBRG_19200_32MHZ 103
#define SPBRG_19200_64MHZ 207
#define SPBRG_19200_40MIPS 519

#define SPBRG_38400_8MHZ 13
#define SPBRG_38400_16MHZ 25
#define SPBRG_38400_32MHZ 51
#define SPBRG_38400_64MHZ 103
#define SPBRG_38400_40MIPS 259

#define SPBRG_38400 12

#define SPBRG_57600 8
#define SPBRG_57600_16MHZ 16
#define SPBRG_57600_32MHZ 34
#define SPBRG_57600_64MHZ 68
#define SPBRG_57600_40MIPS 172

#define SPBRG_115200_16MHZ 8
#define SPBRG_115200_32MHZ 16
#define SPBRG_115200_64MHZ 34
#define SPBRG_115200_40MIPS 85



//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//      END COPY 0
/////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#include "..\..\Var002.H"
#pragma rambank RAM_BANK_4
///////////////////////////////////////BANK 4//////////////////////////





struct _Data_B0{
unsigned Timer1Meausre:1;
unsigned Timer1Count:1;
unsigned Timer1LoadLowOpponent:1;
unsigned Timer1DoTX:1;
unsigned Tmr3DoMeausreFq1_Fq2:1;
unsigned Tmr3Run:1;
unsigned Tmr3Inturrupt:1;
unsigned Tmr3DoneMeasureFq1Fq2:1;
unsigned Timer3Ready2Sync:1;
unsigned Timer3OutSyncRQ:1;
unsigned TransmitESC:1;
unsigned RXLoopAllowed:1;
unsigned RXLoopBlocked:1;
unsigned RXMessageWasOK:1;
unsigned RXPktIsBad:1;
unsigned RXPkt2IsBad:1;
unsigned IntitialTmr3OffsetDone:1;// set 1  -> set 0 on Tmr3 measure and set 1 again on next interrupr all done to skip AdjustTimer3 on first (FQ2) RX 
unsigned BTExternalWasStarted:1;
unsigned EnableFlashWrite:1;
unsigned GIE:1;
unsigned UpgradeProgFlags:1;
} DataB0;

#ifdef DEBUG_LED
int PingDelay;
unsigned char DebugLedCount;
#endif

unsigned char FqTXCount;
unsigned char FqTX;
//unsigned char TXSendOverFQ;
#ifdef _18F2321_18F25K20
UWORD TIMER0 @ 0xFD6;
UWORD TIMER1 @ 0xFCE;
UWORD TIMER3 @ 0xFB2;
#endif
UWORD Time1Left;

UWORD Timer1Id;

UWORD Tmr1High; // to count for a 536 sec with presision of 0.000000125s== 1/4mks == 37.5m
                // UWORD in TMR1 counts till 0.008192 s = 8.192ms

UWORD Tmr1LoadLow;  // timer1 interupt reload values 
UWORD Tmr1LoadLowCopy;  // timer1 interupt reload values 

UWORD Tmr1LoadHigh; // this will 
UWORD Tmr1TOHigh;   // this overload value will generate interrupts and reload timer

UWORD INTTimer1;
UWORD INTTimer1HCount;

UWORD INTTimer3;
UWORD INTTimer3HCount;


UWORD Timer1HCount;
UWORD Timer3HCount;



UWORD AdjRX;
int iAdjRX;

unsigned char FqRXRealCount;
unsigned char IntRXCount;
unsigned char FqRXCount;
unsigned char FqRX;


UWORD Tmr3High; // to count for a 536 sec with presision of 0.000000125s== 1/4mks == 37.5m
                // UWORD in TMR3 counts till 0.008192 s = 8.192ms

UWORD Tmr3LoadLow;  // timer3 interupt reload values 
UWORD Tmr3LoadHigh;
UWORD Tmr3TOHigh; 
unsigned char SkipRXTmr3;
UWORD AdjustTimer3;
UWORD Tmr3LoadLowCopy;
unsigned char OutSyncCounter;
unsigned char ESCCount;
unsigned char AfterESCChar;

#pragma rambank RAM_BANK_1
///////////////////////////////////////BANK 1//////////////////////////

#pragma rambank RAM_BANK_3
/////////////////////////////////////////////BANK 3//////////////////////
struct _DataB3{
unsigned FlashCmd:1;
unsigned FlashCmdLen:1;
unsigned FlashRead:1;
unsigned FlashWas1byteWrite:1;
} DataB3;
unsigned char CountWrite;
unsigned char FlashCmdPos;
unsigned char OldFlashCmd;
unsigned char CurFlashCmd;
unsigned char AdrFlash1;
unsigned char AdrFlash2;
unsigned char AdrFlash3;
UWORD TMR2Count;

UWORD T2Byte0;

UWORD T2Count1;
UWORD T2Byte1;

UWORD T2Count2;
UWORD T2Byte2;

UWORD T2Count3;
UWORD T2Byte3;
unsigned char AdrBH;
UWORD wAddr;

unsigned char ProgAdrBH;
UWORD wProgAddr;
UWORD wProgMaxAddr;

// structure of a packet acsepted from BT
unsigned char NextLen;
unsigned char TypePkt;

#pragma rambank RAM_BANK_5
///////////////////////////////////////BANK 5//////////////////////////
#define BT_BUF 120
//unsigned char ByteInbank5;
struct _BtInternal
{
    WORD iEntry;
    WORD iQueueSize;
    WORD iExit;
    unsigned char Queue[BT_BUF];
} BTInternal;

struct _BtExternal
{
    WORD iEntry;
    WORD iQueueSize;
    WORD iExit;
    unsigned char Queue[BT_BUF];
} BTExternal;


#pragma rambank RAM_BANK_1
///////////////////////////////////////BANK 1//////////////////////////



//#define LEN_OFFSET_INPUT_BUF 28
#define LEN_OFFSET_INPUT_BUF 32

#define BT_TX_MAX_LEN (LEN_OFFSET_INPUT_BUF-7-2)

unsigned char BTqueueOut[BT_TX_MAX_LEN];
unsigned char BTqueueOutLen;

unsigned char BTpktCopy;
unsigned char BTqueueOutCopy[BT_TX_MAX_LEN];
unsigned char BTqueueOutCopyLen;

unsigned char BTqueueIn[LEN_OFFSET_INPUT_BUF];
unsigned char BTqueueInLen;

unsigned char BTqueueIn2[LEN_OFFSET_INPUT_BUF];
unsigned char BTqueueInLen2;

unsigned char BTqueueIn3[LEN_OFFSET_INPUT_BUF];
unsigned char BTqueueInLen3;

unsigned char OutputMsg[LEN_OFFSET_INPUT_BUF];
unsigned char OutputMsgLen;
unsigned char iTotalLen; // int -> char
unsigned char iCrc;
unsigned char *ptrOut;
unsigned char *ptrTemp;
unsigned char mask;
unsigned char CRCM8TX;
unsigned char CRCM8TX2;
unsigned char CRCM8TX3;


UWORD CRC;
UWORD CRC16TX;
struct _BTFLAGS
{
    unsigned BT3fqProcessed:1;
    unsigned BTFirstInit:1;
    unsigned RxInProc:1;
    unsigned CRCM8F:1;
    unsigned Check01:1;
} BTFlags;


UWORD CRCcmp;

unsigned char i;
unsigned char j;
unsigned char res;
   
unsigned char iShift;
//unsigned char bByte;

unsigned char bByte1;
unsigned char bByte2;
unsigned char bByte3;
unsigned char bByteOut;
unsigned char Vote;
unsigned char CmpCount;

unsigned char FisrtR;
unsigned char SecondR;
unsigned char FisrtR2;
unsigned char SecondR2;

unsigned char NextByte1;
unsigned char NextByte2;

unsigned char CRCM8TXNext;
unsigned char CRCM8TX2Next;
struct _DistMeasure{
UWORD RXaTmr1;
UWORD RXaTmr1H;
UWORD RXbTmr1;
UWORD RXbTmr1H;
UWORD TXaTmr1;
UWORD TXaTmr1H;
UWORD TXbTmr1;
UWORD TXbTmr1H;
UWORD TXPeriod;
} DistMeasure;
UWORD PossibleRXTmr1;
UWORD PossibleRXTmr1H;
#pragma rambank RAM_BANK_0

// hex decimal assignment:
// A == J
// B == K
// C == L
// D == M
// E == N
// F == O
//////////////////////////////////////////////////////////////////////////////////////////
// from ATSX=LunMMF1F2F3 // set 
// i.e. ATSX=LunM0020J3k
//  Addr =   Lun
//              M         = mode
//             ' '(space)  1 frequency mode on RX and 1 frequency to TX
//              a          3 frequency mode on RX and 1 frequency to TX
//              b          1 frequency mode on RX and 3 frequency to TX
//              c          3 frequency mode on RX and 3 frequency to TX
//              e          3 frequency mode on RX and 1 frequency to TX, RX allow to switch from 1->3->1 frequency
//              g          3 frquency mode on RX and 3 frequency to TX, RX allow to switch from 1->3->1 frequency
//               0       - unused
//                02     - channel 1 frequency == 02
//                  0J   = channel 2 frequency = 0x0A = 10
//                    3k = channel 3 frequency = 0x3B = 59
// all numbers has to be escaped by simbol '#'
unsigned char BTStatus;
//unsigned char BTInturrupt;
unsigned char Config01;
//  parameter 4 in ATSX=LunMMF1F2F3
//
////////////////////////////////////////////////////////////////////////////
// ATDTl<una> - connect to CubeSat
// ATDTL<UNA>
// when connection established it relpy with "CONNECT" and com will be in relay state (no unit wrapping)
////////////////////////////////////////////////////////////////////////////
// ATSZ=Z set module in listen state on FQ1 when first message receved it reply with "CONNECT" to a UNIT
////////////////////////////////////////////////////////////////////////////
// ATDTE<ARTH> - set module in listen mode on FQ1 and when connection esatblished it reply with "CONNECT" to a unit
// 
////////////////////////////////////////////////////////////////////////////
// not implemented, but good idea:
// ATSY=<pkt><len><data> set into a FLASH memory <pkt> with a <data>
//    all pakets has to be exchanged before sending "CONNECT" message
//    (1) first send <pkt> from device issued ATSX or ATDTEARTH command (all <pkt>s)
//    (2)  this pakages first stored in second device memory
//    (3) then transfer from unit issued command ATDTLUNA/ATDTluna to CubSat device
//    (4) after exchange done both units generats "CONNECT" message and follow "connect" message 
//    (5) <pkt>s "processed" to a data loop
//   <pkt> is that types
//   <pktA> without sequence send firsts
//   <pktB> with sequence - on each reply message data can be stored as a sequence+1 number for continue
// transfer or 
//   <pktC> reply on that paket can be ignored (it has to be spesified length of a reply message)
//   this will allow selected TCP/IP messages to be skipped from unnnessery tranfer
//   for support of a "frozen" session
/////////////////////////////////////////////////////////////////////////// 

#define MODE_CALL_EARTH 1
#ifndef NO_I2C_PROC
#define MODE_CALL_LUNA_I2C 2
#endif
#define MODE_CALL_LUNA_COM 4
#define MODE_CONNECT 8
#define MODE_DIAL 0x10
#define SOME_DATA_OUT_BUFFER 0x20
#define RESPONCE_WAS_SENT 0x40
#define INIT_BT_NOT_DONE 0x80

unsigned char ATCMD;
unsigned char ATCMDStatus;
unsigned char ATCMDStatusAddon;

unsigned char UpgradeProgStatus;

#define PCKT_DIAL 0xf0
#define PCKT_DATA 0xf1
unsigned char BTpkt;
//unsigned char RXreceiveFQ;
//unsigned char BTFQcurr;
unsigned char BTType;
unsigned char BTokMsg;


unsigned char Freq1;
unsigned char Freq2;
unsigned char Freq3;

unsigned char Addr1;
unsigned char Addr2;
unsigned char Addr3;

unsigned char OpponentFreq1;
unsigned char OpponentFreq2;
unsigned char OpponentFreq3;

unsigned char OpponentAddr1;
unsigned char OpponentAddr2;
unsigned char OpponentAddr3;
#pragma rambank RAM_BANK_1


/*
typedef struct PacketDial
{
    //unsigned char Preamb;
    unsigned char Preamb1;
    unsigned char Preamb2;
    unsigned char Preamb3;
    unsigned char BTpacket;
    unsigned char FqCur;
#define PACKET_LEN_OFFSET 5
    unsigned char BTpacketLen;
    unsigned char Type1; // 'L' , 'l', 'E'
    unsigned char Type2; // 'u'        'a'
    unsigned char Type3; // 'n'        'r'
    unsigned char Type4; // 'a'        'z'
    unsigned char OppoentAdr1;
    unsigned char OppoentAdr2;
    unsigned char OppoentAdr3;
    unsigned char OpponentFq1;
    unsigned char OpponentFq2;
    unsigned char OpponentFq3;
};
*/
typedef struct PacketStart
{
    //unsigned char Preamb;
    unsigned char Preamb1;
    unsigned char Preamb2;
    unsigned char Preamb3;
    unsigned char BTpacket;
    unsigned char FqCur;
#define PACKET_LEN_OFFSET 5
    unsigned char BTpacketLen;

    unsigned char Type; //  'l'        'e'
    unsigned char Res1;  // 'u'        'a'
    unsigned char Res2;  // 'n'        'r'
    unsigned char Res3;  // 'a'        'z'
    unsigned char Adr1;
    unsigned char Adr2;
    unsigned char Adr3;
    unsigned char Fq1;
    unsigned char Fq2;
    unsigned char Fq3;
    UWORD Tmr1LLowOpponent;
};

void wCRCupdt(int bByte);
unsigned char SendBTcmd(unsigned char cmd);
void SendBTbyte(unsigned char cmd);
unsigned char GetBTbyte(void);

////////////////////////////////////////////////////////////////////
// simulation functions, vars
///////////////////////////////////////////////////////////////////
    void PrgUnit(unsigned char ProgAdrBH, UWORD wProgAddr, UWORD wProgMaxAddr)
    {
    };
    void InitModem(void)
    {
    };
BTUnit * OtherUnit;
unsigned char *FlashMemory;
int FlashMemoryLen;
unsigned int FlashWriteByteN;
unsigned int FlashPointerByteN;
unsigned int FlashCMD;
unsigned int FlashAdr;
unsigned int FlashAdr1;
unsigned int FlashAdr2;
unsigned int FlashAdr3;
unsigned int FlasSSCS;
#define CS_LOW  CS_Low();
#define CS_HIGH CS_High();
unsigned int FlashStatus;
#define btest(SSPORT,SSCS) (FlasSSCS!=0)
int INT0_ENBL;
void CS_Low(void)
{
    FlasSSCS = 0;
    FlashWriteByteN = 0;
};
void CS_High(void)
{
    FlasSSCS = 1;
};
unsigned char GetSSByte(void)
{
    unsigned bByte;
    if (FlashCMD == 0x03) // read
    {
        bByte = FlashMemory[FlashAdr+FlashPointerByteN];
        FlashPointerByteN++;
        return bByte;
    }
    else if (FlashCMD == 0x05) // status
        return 0;
    return 0;
};
            // F\x01\x06              == write enable (flash command 06) -> send 0x06
            // F\x01\0xc7             == erase all flash                 -> send =0xc7
            // F\x05\x03\x00\x12\x34@\x04 == read 4 bytes from a address 0x001234  -> send 0x03 0x00 0x12 0x34 <- read 4 bytes (must not to cross boundary)
            // F\x01\x06F\x0c\x02\x00\x11\x22\x00\x00\x00\x00\x00\x00\x00\x00 == write 8 bytes to address 0x001122
            // F\x01\x06F\x04\x20\x00\x04\x00 == erase sector (4K) starting from address 0x000400

void SendSSByte(unsigned char bByte)
{
    switch(FlashWriteByteN)
    {
    case 0:FlashCMD = bByte;
           if (FlashCMD == 0x06) // write enable
           {
           }
           else if (FlashCMD == 0xc7) // erase all flash
           {
               memset(FlashMemory, 0xff, FlashMemoryLen);
           }
        break;
    case 1:FlashAdr1 = bByte;break;
    case 2:FlashAdr2 = bByte;break;
    case 3:FlashAdr3 = bByte;FlashPointerByteN = 0;FlashAdr = FlashAdr1 * 256*256 + FlashAdr2*256+FlashAdr3;break;
    default:
        switch(FlashCMD)
        {
        case 0x03: // read operation
        case 0x02: // write operation
            FlashMemory[FlashAdr+FlashPointerByteN] &= bByte; FlashPointerByteN++;
            break;
        case 0x04: // erase sector 4K
            memset(&FlashMemory[FlashAdr&0xfffff000], 0xff, 4096);
            break;
        }
        break;
    }
    FlashWriteByteN++;
};
void putch_main(void)
{
    unsigned char bWorkByte;
    if (AOutQu.iQueueSize)
    {
        bWorkByte = AOutQu.Queue[AOutQu.iExit];
        if (++AOutQu.iExit >= OUT_BUFFER_LEN)
            AOutQu.iExit = 0;
        AOutQu.iQueueSize--;
        fwrite(&bWorkByte,1,1,ComOut);
    }
};
void putch(unsigned char simbol)
{
    if (OutPacketUnit != 0)
    {
        if (Main.OutPacketESC)
            Main.OutPacketESC = 0;
        else if (simbol == ESC_SYMB)
            Main.OutPacketESC = 1;
        else if (simbol == OutPacketUnit)
        {
            if (!Main.OutPacketLenNull)
                OutPacketUnit = 0;
        }
        else
            Main.OutPacketLenNull = 0;
    }
    else
    {
        if (simbol <= MAX_ADR)
        {            
            if (simbol >= MIN_ADR) // msg to relay
            {
                OutPacketUnit = simbol;
                Main.OutPacketLenNull = 1;
            }
        }
        else
            return;   // fixing output packet
    }
    while (AOutQu.iQueueSize >= (OUT_BUFFER_LEN-1)) // dose output queue full ?? then wait till some space will be avalable in output queue
    {
        putch_main();
    }
SEND_BYTE_TO_QU:
    AOutQu.Queue[AOutQu.iEntry] = simbol; // add bytes to a queue
    if (++AOutQu.iEntry >= OUT_BUFFER_LEN)
        AOutQu.iEntry = 0;
    AOutQu.iQueueSize++; // this is unar operation == it does not interfere with interrupt service decrement
    //if (!Main.PrepI2C)      // and allow transmit interrupt
    //    TXIE = 1;  // placed simol will be pushed out of the queue by interrupt
}
void putchWithESC(unsigned char simbol)
{
    if (Main.SendWithEsc)
    {
WAIT_SPACE_Q:
        if (AOutQu.iQueueSize >= (OUT_BUFFER_LEN-3)) // is enought space to output ??
            goto WAIT_SPACE_Q;
        if (simbol == ESC_SYMB)
           goto PUT_ESC;

        if (simbol >= MIN_ADR)
        {
            if (simbol <= MAX_ADR)
            {
PUT_ESC:
                putch(ESC_SYMB);
            }
        }
    }
    putch(simbol);
}
void putmsg(const char *s, unsigned char len)
{
    while(len)
    {
        putch(*s);
		s++;
        len--;
    }
}




//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#include "..\..\ProcCMD.H"
void ProcessCMD(unsigned char bByte)
{
    #ifdef __PIC24H__
    char * ptrlFrsCopy; // attention C30 compiled do not understand pointers anything but char* !!!! needs to make a cast!!!!
    char * ptrlCorrection;
    #else
    unsigned char bFrsCopyL;
    unsigned char bFrsCopyH;
    unsigned char bFrsCopyHH;
    #endif
    unsigned char bCarry;
    unsigned char bWork;

    
    unsigned char bWorkL;
    unsigned char bWorkH;
    unsigned char bWS;
    
//////////////////////////////////////////////////////////////////////
// done area with no difference as a command or stream distingvishing
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
    if (!Main.getCMD)
    {
/////////////////////////////////////////////////////////////////////
// this is a STREAM area of processing
// yes, yes, it is ugly == but processor is PIC, and stack for calls can be limited

//#include "commc3.h"
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
// begin COPY 3
/////////////////////////////////////////////////////////////////////
    //if (!Main.getCMD) // outside of the include was if == unit in "stream" relay mode
    //{
        // getCMD == 0
        // in stream was ESC char and now needs to echo that char to loop
        if (Main.ESCNextByte)
            Main.ESCNextByte = 0;
        else
        {  
            // if this is addressed to this unit then process it and switch "stream" -> "command" mode
            if (bByte == MY_UNIT)
            {
                Main.getCMD = 1; //next will be: <CMD>
                Main.SetFromAddr = 0;
                Main.SetSendCMD = 0;
                I2C.ESCI2CChar = 0;
                Main.LastWasUnitAddr = 1;
                Main.LockToQueue =0; //that will allow to switch back ability to retansmit
                return;
            }
            else if (bByte == ESC_SYMB)   // ESC char - needs to echo next simbol to loop
                Main.ESCNextByte = 1;
        }
        // relay char to the loop, bcs now it is "stream" mode      
        putch(bByte); //ok
SKIP_ECHO_BYTE: ;
    }
    else    // now unit in command mode == processing all data
    {

        if (Main.RetransmitTo) // command =X* was entered - all packet till end was retransmitted to different unit
        {
            if (Main.ESCNextByte)
            {
                Main.ESCNextByte = 0;
                goto RETRANSMIT;
            }
            else
            {
                if (bByte == ESC_SYMB)
                {
                    Main.LastWasUnitAddr = 0;
RETRANSMIT:                    
                    putch(bByte);
                    return;
                }
                else if (bByte == MY_UNIT)
                {
                    if (Main.LastWasUnitAddr)  // pakets can not travel with 0 length - it is definetly was a lost packet and
                        goto RETRANSMIT;

                     Main.RetransmitTo = 0;
                     bByte = UnitFrom;
                     goto RETRANSMIT;
                }
                goto RETRANSMIT;
            }
        }
 
        // getCMD == 1 
        // stream addressing this unit
        if (Main.ESCNextByte)
            Main.ESCNextByte = 0;
        else
        {
            if (bByte == ESC_SYMB)
            {
                Main.LastWasUnitAddr = 0;
#ifndef NO_I2C_PROC

                if (!Main.PrepI2C)
                    Main.ESCNextByte = 1;
                else
                    I2C.ESCI2CChar = 1;
#else
                Main.ESCNextByte = 1;
#endif
                return;
            }
            else if (bByte == MY_UNIT)
            {
                if (Main.LastWasUnitAddr)  // pakets can not travel with 0 length - it is definetly was a lost packet and
                    return;           // needs to continue CMD mode  

                Main.getCMD = 0; // CMD stream done 
#ifndef NO_I2C_PROC
                if (Main.PrepI2C) // execute I2C if CMD stream done 
                {
                    bByte = '@';
                    goto END_I2C_MSG_WAIT;
                }
#endif
                return;
            }
            Main.LastWasUnitAddr = 0;
        }
#ifndef NO_I2C_PROC
//////////////////////////////////////////////////////////////////////////////////
//  I2C command processing:
//     "<"<I2CAddr><DATA>@ or "<"<I2C addr><data><unit> 
//     "<"<I2Caddr><data>">"L@   or "<"<I2Caddr><data>">"L<unit> 
//         where L is a length data to read
//     ">"<I2C addr>L@  or ">"<I2C addr>L<unit> 
//         where L is a length bytes to read
//////////////////////////////////////////////////////////////////////////////////
        if (Main.PrepI2C) // stream addressing I2C 
        {
I2C_PROCESS:
            if (I2C.WaitQuToEmp)      // out queue was to be emptied before any next operation with another I2C
            {
                if (I2Caddr == 0xff)  
                {
                    I2Caddr = bByte;  // first after '<' is address 
                    return;
                }
WAIT_QU_EMP:
                if (AOutI2CQu.iQueueSize) // wait untill ouput I2C queue will be empty to start communication to I2C
                    goto WAIT_QU_EMP;
                //if (TXIE)
                //    goto WAIT_QU_EMP;
                I2C.WaitQuToEmp = 0;
                //BlockComm = 1;
            }
            if (I2C.NextI2CRead)
            {
                LenI2CRead = bByte;
                Main.PrepI2C = 0;
                goto END_I2C_MSG_WAIT;
            }
            if (I2C.ESCI2CChar)
            {
                I2C.ESCI2CChar = 0;
                goto PUT_CHAR;
            }
            if (bByte == '@') // this is end of the message
            {
                Main.PrepI2C = 0;
                goto END_I2C_MSG_WAIT;
            }
            if (bByte == '>') // this is end of the message and start read from same I2C device
            {
                I2C.NextI2CRead = 1;
                return;
            }
PUT_CHAR:

            putchI2C(bByte);
            if (AOutI2CQu.iQueueSize < 14) // packet can be long
                return;
END_I2C_MSG_WAIT:              // TBD this loop has to have limitation - bus can be dead

#ifdef I2C_INT_SUPPORT ////////////////////////////////////////////////////////
            InitI2cMaster();
WAIT_I2C_DONE:
            if (!I2C_B1.I2CMasterDone)  // needs to wait
                goto WAIT_I2C_DONE;
            if (bByte == '@')
                Main.PrepI2C = 0;
#else // not I2C_INT_SUPPORT /////////////////////////////////////////////////
            if (AOutI2CQu.iQueueSize) // this is a case when something in a queue and needs to send it
            {
                //if (I2C_B1.I2CBusBusy) // needs to wait when I2C will be not busy
                //    goto END_I2C_MSG_WAIT;
WAIT_I2C_START:
                if (InitI2cMaster()) // TBD I2C line busy by somebody else what to do?
                    goto WAIT_I2C_START;
                sendI2C();
                //if (sendI2C())  // if return not Zero == error in send everything else has to be skipped
                //    goto DONE_DONE_I2C;
                // out qu must be cleaned (in sendI2C)
                //AOutI2CQu.iQueueSize = 0;
                //AOutI2CQu.iEntry = 0;
                //AOutI2CQu.iExit = 0;
         
                if (bByte == '@')
                {
DONE_I2C:                
                    //I2Caddr = 0xff;
                    Main.PrepI2C = 0;
                    //BlockComm = 0;
                }
                ReleseI2cMaster();
            }
            if (I2C.NextI2CRead)
            {
                I2C_B1.I2Cread = 1;
I2C_WAIT_READ:              // TBD this loop has to have limitation - bus can be dead
                if (InitI2cMaster()) // if in restart somebody uses line then collision
                    goto DONE_DONE_I2C;
                sendI2C();
                //if (sendI2C()) // send address only - TBD needs to check how was ACK on address
                //    goto DONE_DONE_I2C;
                receiveI2C();
DONE_DONE_I2C:
                I2C.NextI2CRead = 0;
                goto DONE_I2C;
            }

#endif // not I2C_INT_SUPPORT/////////////////////////////////////////////////////////////
            Main.DoneWithCMD = 1; // long command ends
            return;
        }  // end if a adressing I2C stream
#endif
//////////////////////////////////////////////////////////////////////////////
// FLASH command processing
// set by external comman like F
//        F<length-of-packet><CMD><data>
//            send and receive responce from FLASH
//        F<length-of-packet><CMD><data>@<length-to-read>
//            in last case <length-of-packet> must include simbol '@'
//////////////////////////////////////////////////////////////////////////////
#ifdef SSPORT
        if (DataB3.FlashCmd)
        {
            if (DataB3.FlashCmdLen) // store length of a flash command
            {
                DataB3.FlashCmdLen = 0;
                CountWrite = bByte;
                DataB3.FlashRead = 0;
                FlashCmdPos = 0;
                //CS_LOW;
            }
            else
            {
                if (DataB3.FlashRead)
                {
                    goto SEND_AGAIN;
CONTINUE_READ: 
                    Main.SendWithEsc = 1;
                    do 
                    {
                        putchWithESC(GetSSByte()); // read byte from FLASh will goes to Com
                                                   // if size bigger then 13 bytes it can be delay (putchWithESC waits out queue avalable space)
                        putch_main(); // that will initiate send 
                    } while(--bByte);
                    DataB3.FlashRead = 0;
                    Main.SendWithEsc = 0;
                    if (UnitFrom)
                        putch(UnitFrom);
                    goto DONE_WITH_FLASH;
                }
                else if (CountWrite == 1) // this will be last byte to write or it can be symb=@ request to read
                {
                    if (OldFlashCmd == 0)  // no prev command "write enable" == read command
                    {
                        if (bByte == '@') // without CS_HIGH will be next read
                        {
                            DataB3.FlashRead = 1;
                            {
                                if (UnitFrom)
                                {
                                    putch(UnitFrom);
                                    if (SendCMD)
                                        putch(SendCMD);
                                }
                            }
                            return;
                        }
                    }
                }
                if (FlashCmdPos == 0)
                {
                   if (CountWrite== 1)
                   {
                       if (OldFlashCmd!=0) // two 1 byte commands in a row
                       {
                            CS_LOW;
                            SendSSByte(OldFlashCmd);
                            CS_HIGH;
                            nop();nop();
WRITE_FLASH_CMD:
                            CS_LOW;
                            SendSSByte(bByte);
                            CS_HIGH;
                            OldFlashCmd = 0;
                            DataB3.FlashCmd = 0;
                            Main.DoneWithCMD = 1; // long command flash manipulation done
                            CountWrite = 0;
                            return;
                       }
                       if (bByte != 0x06) // only command 06 allow to be in stack
                           goto WRITE_FLASH_CMD;
                       OldFlashCmd = bByte;
                       DataB3.FlashWas1byteWrite = 1;
                   }
                   else
                   {
                       CurFlashCmd = bByte;
                       if (!DataB3.FlashWas1byteWrite)
                       {
                           OldFlashCmd = 0;
                       }
                       DataB3.FlashWas1byteWrite = 0;
                   }
                } 
                else if (FlashCmdPos == 1)
                    AdrFlash1 = bByte;
                else if (FlashCmdPos == 2)
                    AdrFlash2 = bByte;
                else if (FlashCmdPos == 3)
                    AdrFlash3 = bByte;
                else
                {
SEND_AGAIN:         // that is equivalent of a function -- just on some PIC it is not enought stack
                    if (btest(SSPORT,SSCS)) // another FLASH write can pause FLASH write intiated by serial 
                    {
                        if (OldFlashCmd != 0)
                        {
                            CS_LOW;
                            SendSSByte(OldFlashCmd);
                            CS_HIGH;
                            nop();nop();
                        }
                        CS_LOW;
                        SendSSByte(CurFlashCmd);
                        SendSSByte(AdrFlash1);
                        SendSSByte(AdrFlash2);
                        SendSSByte(AdrFlash3);
                    }
                    if (!DataB3.FlashRead)
                        SendSSByte(bByte);
                    if (++AdrFlash1 ==0)
                    {
                        if (++AdrFlash2 ==0)
                            ++AdrFlash3;
                        CS_HIGH;          // that is extention == FLASH can not read/write over same page
                        goto SEND_AGAIN;
                    }
                    if (DataB3.FlashRead)
                        goto CONTINUE_READ;
                }
                FlashCmdPos++;
               
                //SendSSByte(bByte);
                //SendSSByteFAST(bByte); //for testing only
                if (--CountWrite)
                    return;
DONE_WITH_FLASH:
                OldFlashCmd = 0;
                CurFlashCmd = 0;
                DataB3.FlashCmd = 0;
                CS_HIGH;
                Main.DoneWithCMD = 1; // long command flash manipulation done 
            }
            return;
        }
#endif // SSPORT
/////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// end COPY 3
////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// additional code proceessed as Cmd :
/////////////////////////////////////////////////////////////////////
    if (ATCMDStatusAddon)
    {
        ATCMDStatusAddon++; 
        if (ATCMDStatusAddon == 2) // AT
        {
            if (bByte != 'T')
            {
DONE_ADDON_CMD:
                ATCMDStatusAddon = 0;
                Main.DoneWithCMD = 1; // long command done
            }
            return;
        }
        if (bByte == 0x0a)
            goto DONE_ADDON_CMD;
        if (ATCMDStatusAddon == 3) // ATD
        {
            if (bByte == 'D')      // ATD - repeat send DIAL from Earth to Luna
            {
                 if (ATCMD & MODE_CONNECT)
                 {
                     Main.PingRQ = 1;
                 }
                 goto DONE_ADDON_CMD;
            }
            if (bByte == 'P')      // ATP - repeat 10 times send PING from Earth to Luna
            {
                Main.DoPing =1;
                PingAttempts = 20;
                Main.ConstantPing = 0;
            }
            if (bByte == 'C')      // ATC - constant PING from Earth to Luna
            {
                Main.DoPing =1;
                PingAttempts = 2;
                Main.ConstantPing = 1;
            }
            if (bByte == 'X')      // ATX - stop pinging
            {
                Main.DoPing =0;
                PingAttempts = 2;
                Main.ConstantPing = 0;
            }
        }
    }
    if (ATCMDStatus)
    {
         ATCMDStatus++;
         if (ATCMDStatus == 2) // At
         {
             if (bByte == 't') // at command starts == othervise commane done
             {
                 return;
             }
             else
             {
                  ATCMDStatus = 0;
                  Main.DoneWithCMD = 1; // long command done
                  goto CONTINUE_NOT_AT;
             }
         }
         else if (ATCMDStatus == 3) // ATd // ATs // ATh
         {
             if (bByte == 'd') // atd command
             {
                 ATCMDStatus = 4;        // on next entry will be 5
             }
             else if (bByte == 's') // ats command
             { 
                 ATCMDStatus = 9;        // on next entry will be 10
             }
             else if (bByte == 'h') // ath command
             { 
                 ATCMDStatus = 3;        // on next entry will be 4
                 InitModem();
             }
             else 
                ATCMDStatus = 3;         // on next entry will be 4
             return;
         }
         else if (ATCMDStatus == 4) // some AT command == skip till 0x0a
         {
              if (bByte == 0x0a) // CR == AT command done
              {
                  ATCMDStatus = 0;
                  Main.DoneWithCMD = 1; // long command done
              }
              else
                  ATCMDStatus = 3; // on next entry will be 4
              return;
         }
         if (bByte == 0x0a) // CR == AT command done
         {
             ATCMDStatus = 0;
             Main.DoneWithCMD = 1; // long command done
             //if (ATCMD)
             //{
             //    ATCMD |= INIT_BT_NOT_DONE;  // execute mode
             //    BTFlags.BTFirstInit = 0;
             //}
             return;
         }
         if (ATCMDStatus == 5) // atdt
         {
             if (bByte != 't')
                ATCMDStatus = 3;
             return;
         }
         else if (ATCMDStatus == 6) // atdt
         {
              // ATDTl - connect to CubeSat and use Comm for tranfering data
              // when connection established it relpy with "CONNECT" and com will be in relay state (no unit wrapping)
              ////////////////////////////////////////////////////////////////////////////
              // ATDTE - set module in listen mode on FQ1 and when connection esatblished it reply with "CONNECT" to a unit
              ////////////////////////////////////////////////////////////////////////////
             ATCMD = 0;
             if (bByte == 'e') // atdtEARTH
                 ATCMD = MODE_CALL_EARTH;//1;
             else if (bByte == 'l') // atdtluna
                 ATCMD = MODE_CALL_LUNA_COM;//4;
             ATCMD |= INIT_BT_NOT_DONE;  // execute mode
             BTFlags.BTFirstInit = 0;

             ATCMDStatus = 6; // on next entry will be 7
             //SetupBT();
             INT0_ENBL = 1;
             return;
         }
         else if (ATCMDStatus == 7) // skip till 0x0a
         {
              ATCMDStatus = 6; // on next entry will be 7
              return;
         }
         else if (ATCMDStatus == 10) // ats command: atsx= atsy= atsy=
         {
              ATCMD = 0;
              // from ATSX=LunBMF1F2F3 // set 
              // i.e. ATSX=Lun00020J3k
              //  Addr =   Lun
              //              0         = 0 = 9600 1 = 19200 3 = 38400 5 = 57600
              //               0       - mode = 0 == 250kb check summ on chip
              //                                1 == 250kb no check summ on chip check sum in software
              //                                2 == 250kb 3 channels FQ1->FQ2->FQ3 internal check summ on individual msg
              //                                3 == 128kb
              //                02     - channel 1 frequency == 02
              //                  0J   = channel 2 frequency = 0x0A = 10
              //                    3k = channel 3 frequency = 0x3B = 59
              ////////////////////////////////////////////////////////////////////////////
              // ATSZ=Z set module in listen state on FQ1 when first message receved it reply with "CONNECT" to a UNIT
              ////////////////////////////////////////////////////////////////////////////
              // ATSY=<pkt><len><data> set into a FLASH memory <pkt> with a <data>
              ////////////////////////////////////////////////////////////////////////////

              if (bByte == 'z') // ATSZ=Z set module in listen state on FQ1 when first message receved it reply with "CONNECT" to a UNIT
              {
                 ATCMD = MODE_CALL_LUNA_COM;//4;
                 ATCMDStatus = 6; // on next entry will be 7

              }
              if (bByte == 'x') // from ATSX=LunBMF1F2F3 // set 
                 ATCMDStatus = 10;      // on next entry will be 11
              if (bByte == 'y') // ATSY=<pkt><len><data> set into a FLASH memory <pkt> with a <data>
                 ATCMDStatus = 23;  // on next entry will be 24
              return;
         }
         else if (ATCMDStatus == 11) // ATSX=LunBMF1F2F3
              return;                //     A
         else if (ATCMDStatus == 12) // ATSX=LunBMF1F2F3
         {                           //      A
                 Addr1 = bByte;
                 if (Addr1 == '.')
                     Addr1 = 0xaa;    
                 eeprom_write(0x34, Addr1);
                 return;
          }
          else if (ATCMDStatus == 13) // ATSX=LunBMF1F2F3
          {                           //        A
                 Addr2 = bByte;
                 if (Addr2 == '.')
                     Addr2 = 0xaa;    
                 eeprom_write(0x35, Addr2);
                 return;
          }
          else if (ATCMDStatus == 14) // ATSX=LunBMF1F2F3
          {                           //         A
                 Addr3 = bByte;
                 if (Addr3 == '.')
                     Addr3 = 0xaa;    
                 eeprom_write(0x36, Addr3);
                 return;
          }
          else if (ATCMDStatus == 15) // ATSX=LunBMF1F2F3
          {                           //         A
                Config01 = bByte;
//             ' '(space)  1 frequency mode on RX and 1 frequency to TX
//              a          3 frequency mode on RX and 1 frequency to TX
//              b          1 frequency mode on RX and 3 frequency to TX
//              c          3 frequency mode on RX and 3 frequency to TX
//              e          3 frequency mode on RX and 1 frequency to TX, RX allow to switch from 1->3->1 frequency
//              g          3 frquency mode on RX and 3 frequency to TX, RX allow to switch from 1->3->1 frequency
                eeprom_write(0x30, Config01);
                return;
          }
          else if (ATCMDStatus == 16) // ATSX=LunBMF1F2F3
          {                           //          A
                //Config01 |= bByte<<2;
                return;
          }
          else if (ATCMDStatus == 17) // ATSX=LunBMF1F2F3
          {                           //           A
                Freq1 = bByte<<4;
                 return;
          }
          else if (ATCMDStatus == 18) // ATSX=LunBMF1F2F3
          {                           //            A
                Freq1 |= bByte & 0x0f;
                //BTFQcurr = Freq1;
                FqTX = Freq1;
                FqTXCount = 0;
                eeprom_write(0x31, Freq1); // slow operation == 4 simbols at one write
                return;
          }
          else if (ATCMDStatus == 19) // ATSX=LunBMF1F2F3
          {                           //             A
                Freq2 = bByte<<4;
                return;
          }
          else if (ATCMDStatus == 20) // ATSX=LunBMF1F2F3
          {                           //              A
                Freq2 |= bByte & 0x0f;
                eeprom_write(0x32, Freq2); // slow operation == 4 simbols at one write
                return;
          }
          else if (ATCMDStatus == 21) // ATSX=LunBMF1F2F3
          {                           //               A
                Freq3 = bByte<<4;
                return;
          }
          else if (ATCMDStatus == 22) // ATSX=LunBMF1F2F3
          {                           //                A
                Freq3 |= bByte & 0x0f;
                eeprom_write(0x33, Freq3); // slow operation == 4 simbols at one write
                ATCMDStatus = 3; // on next entry will be 4
                //SetupBT();
                //INT0_ENBL = 1;
                return;
          }
          else if (ATCMDStatus == 24) 
          {
          }
          // something wrong == done with AT
          ATCMDStatus = 0;
          Main.DoneWithCMD = 1; // long command done
          return;
    }
CONTINUE_NOT_AT:
    if (DataB0.UpgradeProgFlags)
    {
        // CMD upgrade PIC from storage
        //<U><XXXXXX>=<xxxxxx>=<llll>
        // load from XXXXXX (FLASH) to xxxxxx (prog memory) llll bytes and restart processor
        UpgradeProgStatus++;
        if (UpgradeProgStatus == 1) //<U><XXxxxx>=<xxxxxx>=<llll>
            AdrBH= bByte;
        else if (UpgradeProgStatus == 2)//<U><xxXXxx>=<xxxxxx>=<llll>
            wAddr = ((UWORD)bByte)<<8;
        else if (UpgradeProgStatus == 3) //<U><xxxxXX>=<xxxxxx>=<llll>
            wAddr += bByte;
        else if (UpgradeProgStatus == 4) //<U><......>=<......>.<....> 
            ;        
        else if (UpgradeProgStatus == 5) //<U><xxxxxx>=<XXxxxx>=<llll> 
            ProgAdrBH = bByte;
        else if (UpgradeProgStatus == 6) //<U><xxxxxx>=<xxXXxx>=<llll> 
            wProgAddr =  ((UWORD)bByte)<<8;
        else if (UpgradeProgStatus == 7) //<U><xxxxxx>=<xxxxXX>=<llll> 
            wProgAddr += bByte;

        else if (UpgradeProgStatus == 8) //<U><......>.<......>=<....> 
            ;
        else if (UpgradeProgStatus == 9) //<U><xxxxxx>=<xxxxxx>=<LLll> 
            wProgMaxAddr = ((UWORD)bByte)<<8;        
        else if (UpgradeProgStatus == 10) //<U><xxxxxx>=<xxxxxx>=<llLL> 
        {
            wProgMaxAddr += bByte;
            Main.DoneWithCMD = 1; // long command done
            DataB0.UpgradeProgFlags = 0;
            // reset after upgrade
            PrgUnit(ProgAdrBH, wProgAddr, wProgMaxAddr);
 
        }
    }
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//#include "commc4.h"
////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// begin COPY 4
///////////////////////////////////////////////////////////////////////   

// standart commands:
//<unit>=Xci<unit> - set responce unit(X) and command (c)
//<unit>=X*<unit> and all data transfers to X till end of the packet
//<unit>=X?<unit>   - timer sync message
//<unit>~<unit>   - message from itself from other side of the loop    
        if (Main.SetFromAddr) //<unit>=Xc            //<unit>=Xci<unit> 
        {                     //       |         if ' ' than responce unit is not set
            if (bByte == ' ')
                bByte = 0;
            UnitFrom = bByte;
            Main.SetFromAddr = 0;
            Main.SetSendCMD = 1;

            return;
        }
        else if (Main.SetSendCMD) //<unit>=xC                 ///<unit>=xCi<unit> 
        {                         //        |        if ' ' than SendCMD is not set
            if (bByte == ' ')
                bByte = 0;
            SendCMD = bByte;
            Main.SetSendCMD = 0;
            I2C.SetI2CYesNo = 1;
            Main.DoneWithCMD = 1; // long command "=XC" done

            if (bByte == '*')  // "=X*" and all data transfers to X till end of the packet
            {
                 //if (UnitFrom) // assuming that unit was specified 
                 {
                     putch(UnitFrom);putch(UnitFrom); // twice to avoid lost bytes
                     Main.RetransmitTo = 1;
                     return;
                 }
            }
#ifdef SYNC_CLOCK_TIMER
            if (bByte == '?')
            {
#ifdef __PIC24H__
                Tmr4Count =TMR4;  // it is possible to count delays from interrupt to recorded time
                Tmr4CountH = TMR5HLD;
                TAfter.Timer = (((unsigned long)Tmr4CountH)<<16) | ((unsigned long)Tmr4Count);
                TAfter.Second = Tmr4CountOld;
                RtccReadTimeDate(&TAfter.Rtcc);
                putch(UnitFrom);putch(UnitFrom);
                FSR_REGISTER = &Tdelta;
                Main.SendWithEsc = 1;
                for (bWork = 0; bWork < 4*sizeof(Ttilad);bWork++)
                {
                    putchWithESC(PTR_FSR);FSR_REGISTER++;
                }
                putch(UnitFrom); // close packet
                Main.SendWithEsc = 0;
#else  // not __PIC24H__
#endif // __PIC24H__
                return;
            }
#endif // SYNC_CLOCK_TIMER

#ifdef RESPONCE_ON_EQ
			if (UnitFrom) // basically that is ACK
            {
            	putch(UnitFrom);
                if (SendCMD)
                	putch(SendCMD);
                putch('~');
                putch(UnitFrom);
            }
#endif
            return;
        }
        else if (bByte == '=') // new version   "=XC" where X - unit to responce and C - one byte command to responce 
                               // if command is "=X*" than all packet till end has to be send over com to device X with closing packet byte (X at the end)
                               
                               // old verion
                               // <unit>=XCI<unit> from unit = X, CMD to send =C (space = no CMD) I = expect retransmit over I2C
        {                      //  '=5CC' == to unit=5 with CMD=C over Type=C (Com) (operation SET)
                               //  '=5CI' == to unit=5 with CMD=C over Type=I (I2C) (opeartion SET) equivalent of <5C<DATA>@ 
                               //  '=*'   == to unit=5 with CMD=C over I2C == starting next byte all stream goes from com to I2C (retransmit)
                               //  '=*'   == to unit=5 with CMD=C over Com == starting next byte all stream goes from I2C to com (retransmit)
                               //  '=<NBIT+LEN>' (LEN < 128) next LEN bytes will goes to previously set device
                               //  high bit has to be set
            Main.DoneWithCMD = 0; // long command
            Main.SetFromAddr = 1;
        }
        // processing CMD
        else if (bByte == '~') // reseved test message from itself
        {
            Main.CommLoopOK = 1;
#ifdef SYNC_CLOCK_TIMER
#ifdef __PIC24H__
            memcpy(&Tdelta,&Ttilad,sizeof(Tdelta));
#else
#ifdef      _18F2321_18F25K20
#endif 
#endif // __PIC24H__
#endif // SYNC_CLOCK_TIMER
        }
#ifndef NO_I2C_PROC
        else if (bByte == '<') // "<"<I2CAddr><DATA>@ or "<"<I2C addr><data><unit> 
        {                      // "<"<I2Caddr><data>">"L@   or "<"<I2Caddr><data>">"L<unit> 
                               // where L is a length data to read
            Main.DoneWithCMD = 0; // long command
            I2Caddr = 0xff;
            Main.PrepI2C = 1;
            I2C_B1.I2Cread = 0;
            I2C.WaitQuToEmp = 1;
            I2C.NextI2CRead = 0;
       }
        else if (bByte == '>') // ><I2C addr>L@  or ><I2C addr>L<unit> where L is a length bytes to read
        {                      
            Main.DoneWithCMD = 0; // long command
            I2Caddr = 0xff;
            Main.PrepI2C = 1;
            I2C.WaitQuToEmp =  1;
            I2C.NextI2CRead = 1;
        }
#endif // NO_I2C_PROC
#ifdef SSPORT
        else if (bByte == 'F') // manipulation with FLASH memory: read/write/erase/any flash command
        {
            Main.DoneWithCMD = 0; // long command
            DataB3.FlashCmd = 1;
            DataB3.FlashCmdLen = 1;
            
            // send something to FLASH
            // F<length-of-packet><CMD><data>
            // send and receive responce from FLASH
            // F<length-of-packet><CMD><data>@<length-to-read>
            // in last case <length-of-packet> must include simbol '@'
            // F\x01\x06              == write enable (flash command 06) -> send 0x06
            // F\x01\0xc7             == erase all flash                 -> send =0xc7
            // F\x05\x03\x00\x12\x34@\x04 == read 4 bytes from a address 0x001234  -> send 0x03 0x00 0x12 0x34 <- read 4 bytes (must not to cross boundary)
            // F\x01\x06F\x0c\x02\x00\x11\x22\x00\x00\x00\x00\x00\x00\x00\x00 == write 8 bytes to address 0x001122
            // F\x01\x06F\x04\x20\x00\x04\x00 == erase sector (4K) starting from address 0x000400
        }
#endif
/////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// end COPY 4
////////////////////////////////////////////////////////////////////////


// additional code:
//
#ifdef NON_STANDART_MODEM
        else if (bByte == '$') // send everything to connected BT till end of the packet and ask to retransmit data to link
        {
            if (ATCMD & MODE_CONNECT) // was connection esatblished?
            {
                 Main.SendOverLink = 1;
                 Main.SendOverLinkAndProc = 1;
                 Main.SendOverlinkWasESC = 0;
            }
            // TBD: on no connection established - needs to do something with uplink data - it is going to nowhere
        }
        else if (bByte == '*') // send everything to connected BT till end of the packet and process inside BT 
        {
            if (ATCMD & MODE_CONNECT) // was connection esatblished?
            {
                 Main.SendOverLink = 1;
                 Main.SendOverlinkWasESC = 0;
            }
            // TBD: on no connection established - needs to do something with uplink data - it is going to nowhere
        }
#endif
        else if (bByte == 'a')
        {
            ATCMDStatus = 1;
            Main.DoneWithCMD = 0; // long command
        }
        else if (bByte == 'A')  // additional command
        {
            ATCMDStatusAddon = 1;
            Main.DoneWithCMD = 0; // long command
        }
        else if (bByte == 'U')  // CMD upgrade PIC from storage <U><XXXXXX>=<xxxxxx>=<llll>// additional command
        {
            DataB0.UpgradeProgFlags =1;
            UpgradeProgStatus = 0;
            Main.DoneWithCMD = 0; // long command
        }
        else if (bByte == 'C')  // CRC flash <C><XXXXXX>=<xxxxxx>=<CRC1>=<CRC2>=<CRC3>=<CRC4> 
        {                       // if CRCX is OK respond "O000"
                                // if CRCX is not OK respond o<CRC1>=<CRC2>=<CRC3>=<CRC4>
                                // on each 1/4 of the 1/4 
                                // i.e C008000=00a000=1234=5678=9abc=def0
                                // CRC of 008000-008800 = 1234
                                // CRC    008800-009000 = 5678
                                //        009000-009800 = 9abc
                                //        009800-00a000 = def0
                                //   responce OOOO in case of all 4 CRC are OK
                                //   responce o0101=2345=5432=9876 on each not matched CRC
        }
        else if (bByte == 'c')  // CRC prog  <c><XXXXXX>=<xxxxxx>=<CRC1>=<CRC2>=<CRC3>=<CRC4>
        {
        }
SKIP_BYTE:;
    } // do not confuse: this is a else from getCMD == 1
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
};

int iStat;
unsigned char ConstByte;
unsigned char WriteFileComHex(unsigned char bInByte, BOOL &CharReady)
{
    CharReady = FALSE;
    switch(iStat)
    {
    case 0: 
        if (bInByte == '%')
           iStat = 1;
        else
        {
            CharReady = TRUE;
            return bInByte;
        }
        break;
    case 1:
        if (bInByte <= '9')
            ConstByte = (bInByte - '0')<<4;
        else if (bInByte <= 'F')
            ConstByte = (bInByte - 'A' + 10)<<4;
        else
            ConstByte = (bInByte - 'a' + 10)<<4;
        iStat = 2;
        break;
    case 2:
        if (bInByte <= '9')
            ConstByte |= (bInByte - '0');
        else if (bInByte <= 'F')
            ConstByte |= (bInByte - 'A' + 10);
        else
            ConstByte |= (bInByte - 'a' + 10);
        iStat = 0;
        CharReady = TRUE;
        return ConstByte;
        break;
    }
    return 0;
}
int EarthLunaPktN=0;
int LunaEarthPktN=0;
BOOL Simulation(BTUnit *BTLuna, BTUnit *BTEarth, FILE *FileComOutEarth, FILE *FileComOutLuna)
{
    BOOL bRet = FALSE;
    unsigned char bWorkByte;
    if (BTEarth->AOutQu.iQueueSize)
    {
        bWorkByte = BTEarth->AOutQu.Queue[BTEarth->AOutQu.iExit];
        if (++BTEarth->AOutQu.iExit >= OUT_BUFFER_LEN)
            BTEarth->AOutQu.iExit = 0;
        BTEarth->AOutQu.iQueueSize--;
        fwrite(&bWorkByte,1,1,FileComOutEarth);
    }
    if (BTEarth->ATCMD & SOME_DATA_OUT_BUFFER) // BTEarth send something to the BTLuna
    {
        bRet = TRUE;
        // earth to luna TX
        BOOL LostPacket = FALSE;
        EarthLunaPktN++;
        if (BTEarth->BTqueueOut[0] == '*') // process by Luna BT unit
        {
            for (int ii = 1; ii <BTEarth->BTqueueOutLen;ii++)
            {
                if (!LostPacket)
                    BTLuna->ProcessCMD(BTEarth->BTqueueOut[ii]);
            }
        }
        else
        {
            for (int ii = 0; ii <BTEarth->BTqueueOutLen;ii++)
            {
                if (!LostPacket)
                    fwrite(&BTEarth->BTqueueOut[ii],1,1,FileComOutLuna);
            }
        }
        BTEarth->BTqueueOutLen = 0;
        BTEarth->ATCMD &= (0xff ^SOME_DATA_OUT_BUFFER);
    }
    if (BTLuna->AOutQu.iQueueSize)
    {
        bWorkByte = BTLuna->AOutQu.Queue[BTLuna->AOutQu.iExit];
        if (++BTLuna->AOutQu.iExit >= OUT_BUFFER_LEN)
            BTLuna->AOutQu.iExit = 0;
        BTLuna->AOutQu.iQueueSize--;
        fwrite(&bWorkByte,1,1,FileComOutLuna);
    }
    if (BTLuna->ATCMD & SOME_DATA_OUT_BUFFER) // BTEarth send something to the BTLuna
    {
        bRet = TRUE;
        // luna to earth communication
        // 
        BOOL LostPacket = FALSE;
        LunaEarthPktN++;
        if (BTLuna->BTqueueOut[0] == '*') // process by Luna BT unit
        {
            for (int ii = 1; ii <BTLuna->BTqueueOutLen;ii++)
            {
                if (LostPacket)
                     BTLuna->ProcessCMD(BTLuna->BTqueueOut[ii]);
            }
        }
        else
        {
            for (int ii = 0; ii <BTLuna->BTqueueOutLen;ii++)
            {
                if (LostPacket)
                    fwrite(&BTLuna->BTqueueOut[ii],1,1,FileComOutEarth);
            }
        }
        BTLuna->BTqueueOutLen = 0;
        BTLuna->ATCMD &= (0xff ^SOME_DATA_OUT_BUFFER);
    }
    
    return (bRet);
}

int _tmain(int argc, _TCHAR* argv[])
{
    unsigned char bWorkByte;
    BTUnit BTLuna;
    BTUnit BTEarth;
    memset(&BTLuna, 0, sizeof(BTLuna));
    memset(&BTEarth, 0, sizeof(BTEarth));
    BTLuna.OtherUnit = &BTEarth;
    BTEarth.OtherUnit = &BTLuna;
    BTLuna.FlashMemoryLen = 1*1024*1024;
    BTLuna.FlashMemory = (unsigned char*) GlobalAlloc( GMEM_FIXED, BTLuna.FlashMemoryLen);// 1 MB
    memset(BTLuna.FlashMemory,0xff,BTLuna.FlashMemoryLen);
    memset(BTLuna.EEPROM, 0xff, sizeof(BTLuna.EEPROM));
    BTEarth.FlashMemoryLen = 1*1024*1024;
    BTEarth.FlashMemory = (unsigned char*) GlobalAlloc( GMEM_FIXED, BTLuna.FlashMemoryLen);// 1 MB
    memset(BTEarth.FlashMemory,0xff,BTEarth.FlashMemoryLen);
    memset(BTEarth.EEPROM, 0xff, sizeof(BTEarth.EEPROM));
    FILE *FileComOutEarth = fopen(argv[3], "wb");
    if (FileComOutEarth)
    {
        BTEarth.ComOut = FileComOutEarth;
        FILE *FileComOutLuna = fopen(argv[2], "wb");
        if (FileComOutLuna)
        {
            BTLuna.ComOut = FileComOutLuna;
            FILE *FileComEarth = fopen(argv[1], "rb");
            if (FileComEarth)
            {
                unsigned char bByff[1];
                BOOL CharReady;
                unsigned char OutPutByte;
                iStat=0;
                // initial commands
                while(fread(bByff,1,1,FileComEarth)==1)
                {
                    // main simulation - Luna & Earth are talking
                    OutPutByte = WriteFileComHex(bByff[0], CharReady);
                    if (CharReady)
                    {
                        BTEarth.ProcessCMD(OutPutByte);
                        Simulation(&BTLuna, &BTEarth, FileComOutEarth, FileComOutLuna);
                    }
                }

                fclose(FileComEarth);
                // initial transfer done - now simulate process of restoration
                while(Simulation(&BTLuna, &BTEarth, FileComOutEarth, FileComOutLuna))
                {

                }
            }
            fclose(FileComOutLuna);
        }
        fclose(FileComOutEarth);
    }
    GlobalFree(BTEarth.FlashMemory);
    GlobalFree(BTLuna.FlashMemory);
	return 0;
}

