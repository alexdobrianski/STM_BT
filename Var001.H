/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////Var001.h//////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

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
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////end of var001.h//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////