/***********************************************************************
    2011-2014 (C) Alex Dobrianski BT communication module 2.4 GHz

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>

    Design and development by Team "Plan B" is licensed under 
    a Creative Commons Attribution-ShareAlike 3.0 Unported License.
    http://creativecommons.org/licenses/by-sa/3.0/ 

************************************************************************/
/***********************************************************************
see www.adobri.com for communication protocol spec
************************************************************************/
// BT communication module
// common for Cubesat 10-A, lunar probe Plan B, Ground station
//////////////////////////////////////////////////////////////////////////////////////////////
//  packet retransmit 3 times : first packet is XXX, second YYY, third ZZZ
//
// Mode TX1 (3 transmitters ground station only):
// send over one of 3 BT devices with each device assigned to individual Frequencies FQ1, FQ2, FQ3
//cpu  XXX    timeout   YYY    ZZZ wait 
//                                         t=(FQ2-FQ1) <-interrupt
//FQ1     tTTT<-interrupt
//FQ2                      tTTT<-interrupt
//FQ3                                      tTTT
//
// Mode TX2 (1 transmitter)
//cpu  XXX    timeout   YYY    ZZZ wait 
//                                         t=(FQ2-FQ1) <-interrupt
//FQ1     tTTT<-interrupt
//FQ2                      tTTT<-interrupt
//FQ3                                      tTTT
//
// Mode TX3 (1 transmitter)
//cpu  XXX    timeout   YYY    ZZZ wait 
//                                         t=(FQ2-FQ1) <-interrupt
//FQ1     tTTT<-interrupt  tTTT<-interrupt tTTT
//FQ2                      
//FQ3                                     
//
// Mode RX1 (3 receivers == ground station only):
//FQ1 Listen rrr
//FQ2 Listen                rrr
//FQ3 Listen                                rrr
//
// Mode RX2 (1 receiver):
//FQ1 Listen rrr
//FQ2 Listen                rrr
//FQ3 Listen                                rrr
//
// Mode RX3 (1 receiver):
//FQ1 Listen rrr            rrr             rrr
//FQ2              
//FQ3
//
// mesurement of TTT can be done on uncommenting MEASURE_EXACT_TX_TIME, need to account that it is transfer of 32 bytes in DEBUG_LED mode
// protocol diagram for mode TX3 & RX3
// for 2321 time TTT == RRR == 0x2ADD = 10973 op = 0.001371625 sec
//          time 123456789AB = (TX = 10973 op = 0.001371625)+(TO= 93*128 = 11904 op = 0.001488)+(Packet prep = 7472) = total = 30349 = 0.003793625 sec
// for 25K20time TTT == RRR ==        = 7051 op = 0.000459 sec
//          time 123456789AB = (TX = 7344 op = 0.0004406875 sec)+(TO= 47*256 = 12032 op = 0.000752 sec ) + (Packet prep = 7724) = total = 26807 = 0.0016754375sec
// 2mbt25K20time TTT == RRR ==        = 4595 op = 0.0002871875 sec
//          time 123456789AB = (TX = 4595 op = 0.0002871875 sec)+(TO= 47*256 = 12032 op = 0.000752 sec ) + (Packet prep = 7695) = total = 24322 = 0.001520125sec
// i.e. time line :
// int =         10955/7051/4595 == TTT time
// set timeout = 11220 == TO   
// before TX     0x5AB5 Transmit preperation !!!!!! == 1BAA = 7082 op
// TX set = 0x765F
// for receiver:
// time "123456789AB" - Time X = time "7654321"
// X = 0x3500= X > 0x2ACB
#define MEDIAN_TIME 0x4000
// #define DELAY_BTW_SEND_PACKET 0xffa3
// transmitter:
//    111       222        333        111        222        333        111        222        333        111        222        333        111        222        333        111
//XXXtTTToooYYYtTTTZZZ____tTTT___________XXX____tTTTYYY____tTTTZZZ____tTTT____________________________XXX_________tTTTYYY____tTTTZZZ____tTTT
//   123456789AB123456789AB123456789AB123456789AB123456789AB123456789AB123456789AB123456789AB123456789AB123456789AB123456789AB123456789AB123456789AB123456789AB123456789AB123456789AB
//                                                                                                                                                              
//                                    .         +           .          .          .          +          +          +          .          .          .          +          +          +
//1LLLrrrxxx-OK                  LLLLLLLLLLLLLLLL                 LLLLLrrrzzz                           LLLLLLLLLLL                 LLLLLrrrzzz                           LLLLLLLLLLL
//2         LLLLrrryyy-OK                        rrrxxx                      LLLLLLLLLLLLLLLL                      rrrxxx                      LLLLLLLLLLLLLLLL                      LLLLLL
//3                   LLLLLrrrzzz                      LLLLLrrryyy                           LLLLLLLLLLL                 LLLLLrrryyy                           LLLLLLLLLLL
//      123456789AB123456789AB                                                                                                                                                                    
//      123456789AB7654321|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|
//                        X          X          +          X          X          X          +          +          +          X          X          X          +          +          +          +
//1LLLRRRxxx                     LLLLLLLLLLLLLLL                  LLLLLrrrzzz                          LLLLLLLLLLL                  LLLLLrrrzzz                          LLLLLLLLLLL 
//2         LLLLRRRyyy                          LRRRxxx                      LLLLLLLLLLLLLLL                      Lrrrxxx                      LLLLLLLLLLLLLLL                      LLLLLLLLLLL
//3                   LLLLLRRRzzz                      LLLLLRRRyyy                          LLLLLLLLLLL                  LLLLLrrryyy                          LLLLLLLLLLL                      LLLL
//                 S      0   S      0          +   S      0   S      0   S      0          +          +          +   S      0   S      0   S      0          +          +          +          +          
//                 3      0   1                     2          3          1                                           2          3          1   
//                            0=1                   0=2        0=3        0=1                                         0=2        0=3        0=1                                                                                                                                             
//                                                                                                                                                              
//                                                                                                                                                              
//                                                                                                                                                              
//errors: extra noize packet 1                                                                                                                                                                                         
//1LLLRRRxxx                     LLLLLLRRRxxx-err                 LLLLLrrrzzz                          LLLLLLLLLLL                  LLLLLrrrzzz                          LLLLLLLLLLL 
//2         LLLLRRRyyy                       LLLLRRRyyy-OK=xxx               LLLLLLLLLLLLLLL                      Lrrrxxx                      LLLLLLLLLLLLLLL                      LLLLLLLLLLL
//3                   LLLLLRRRzzz                      LLLLLRRRyyy                          LLLLLLLLLLL                  LLLLLrrryyy                          LLLLLLLLLLL                      LLLL
//                 S      0   S      0    S     0   S      0   S      0   S      0          +          +          +   S      0   S      0   S      0          +          +          +          +          
//                 3      0   3           1         2          3          1            
// extra noize packet 1 + packet 2                                                                                                                                                                                                
//1LLLRRRxxx                     LLLLLrrrxxx-err                  LLLLLrrrzzz                          LLLLLLLLLLL                  LLLLLrrrzzz                          LLLLLLLLLLL 
//2         LLLLRRRyyy                      Lrrryyy-err                      LLLLLLLLLLLLLLL                      Lrrrxxx                      LLLLLLLLLLLLLLL                      LLLLLLLLLLL
//3                   LLLLLRRRzzz                  LLLLLLLLLRRRzzz-ok=yyy                   LLLLLLLLLLL                  LLLLLrrryyy                          LLLLLLLLLLL                      LLLL
//                 S      0   S      0   S      0S         0   S      0   S      0          +          +          +   S      0   S      0   S      0          +          +          +          +          
//extra noize packet1+packet2
//1LLLRRRxxx                     Lrrrxxx-errLLLL                  LLLLLrrrzzz                          LLLLLLLLLLL                  LLLLLrrrzzz                          LLLLLLLLLLL 
//2         LLLLRRRyyy                  Lrrryyy-errRxxx                      LLLLLLLLLLLLLLL                      Lrrrxxx                      LLLLLLLLLLLLLLL                      LLLLLLLLLLL
//3                   LLLLLRRRzzz              LLLLLLLLLLLLLRRRxxx=yyy                      LLLLLLLLLLL                  LLLLLrrryyy                          LLLLLLLLLLL                      LLLL
//                 S      0   S      0S     S   3          0   S      0   S      0          +          +          +   S      0   S      0   S      0          +          +          +          +          
//                 3      0   1       2                        3          1                                           2          3          1   
//                            0=1     0=2   2   3              0=3        0=1                                         0=2        0=3        0=1                                                                                                                                             
//                                          2=2,3
// extra noize pkt1+2+3                                                                                                                                                                                           
//1LLLRRRxxx                     LLLLLrrrxxx-err          LLLLLLLLLLLLLrrrxxx=ok=zzz                              LLLLLLLLLLL                  LLLLLrrrzzz                          LLLLLLLLLLL 
//2         LLLLRRRyyy                      Lrrryyy-err                      LLLLLLLLLLLLLLL                      Lrrrxxx                      LLLLLLLLLLLLLLL                      LLLLLLLLLLL
//3                   LLLLLRRRzzz                  Lrrrzzz-err                              LLLLLLLLLLL                  LLLLLrrryyy                          LLLLLLLLLLL                      LLLL
//                 S      0   S      0   S      0S     S   1          0   S      0          +          +          +   S      0   S      0   S      0          +          +          +          +          
//                 0=3        0=1        0=2    0=3    3=3,1              0=2    0
//lost first packet:
//1LLLRRRxxx                     LLLLLLLLLLLLLLL                  LLLLLrrrzzz                          LLLLLLLLLLL                  LLLLLrrrzzz                          LLLLLLLLLLL 
//2         LLLLRRRyyy                          LLLLLLLLLLL                  LLLLLLLLLLLLLLL                      Lrrrxxx                      LLLLLLLLLLLLLLL                      LLLLLLLLLLL
//3                   LLLLLRRRzzz                          LRRRxxx=yyy                      LLLLLLLLLLL                  LLLLLrrryyy                          LLLLLLLLLLL                      LLLL
//                 S      0   S      0          +          +   S      0   S      0          +          +          +   S      0   S      0   S      0          +          +          +          +          
//                 3          1                                3          1                                           2          3          1   
//                            3=1                              0=3        0=1                                         0=2        0=3        0=1                                                                                                                                             
//
//lost second packet                   
//1LLLRRRxxx                     LLLLLLLLLLLLLLL                      Lrrrzzz                          LLLLLLLLLLL                  LLLLLrrrzzz                          LLLLLLLLLLL 
//2         LLLLRRRyyy                          LRRRxxx                      LLLLLLLLLLLLLLL                      Lrrrxxx                      LLLLLLLLLLLLLLL                      LLLLLLLLLLL
//3                   LLLLLRRRzzz                      LLLLLLLLLLLLLLL                      LLLLLLLLLLL                  LLLLLrrryyy                          LLLLLLLLLLL                      LLLL
//                 S      0   S      0          +   S      0          +   S      0          +          +          +   S      0   S      0   S      0          +          +          +          +          
//                 3          1                     2                     1                                           2          3          1   
//                            3=1                   0=2                   0=1                                         0=2        0=3        0=1                                                                                                                                             
//lost third packet
//                        X          X          +          X          X          +          +          +          +          X          X          X          +          +          +          +
//1LLLRRRxxx                     LLLLLLLLLLLLLLL                  LLLLLLLLLLLLLLL                      LLLLLLLLLLL                  LLLLLrrrzzz                          LLLLLLLLLLL 
//2         LLLLRRRyyy                          LRRRxxx                          LLLLLLLLLLL                      Lrrrxxx                      LLLLLLLLLLLLLLL                      LLLLLLLLLLL
//3                   LLLLLRRRzzz                      LLLLLRRRyyy                          LLLLLLLLLLL                  LLLLLrrryyy                          LLLLLLLLLLL                      LLLL
//                 S      0   S      0          +   S      0   S      0          +          +          +          +   S      0   S      0   S      0          +          +          +          +          
//                 3      0   1                     2          3                                                      2          3          1   
//                            0=1                   0=2        0=3                                                    0=2        0=3        0=1                                                                                                                                             
//runaway packet:
//    111       222        333        111        222        333        111        222        333      ___  111        222        333        111        222        333        111
//XXXtTTToooYYYtTTTZZZ____tTTT___________XXX____tTTTYYY____tTTTZZZ____tTTT____________________________+++XXX_________tTTTYYY____tTTTZZZ____tTTT
//   123456789AB123456789AB123456789AB123456789AB123456789AB123456789AB123456789AB123456789AB123456789___AB123456789AB123456789AB123456789AB123456789AB123456789AB123456789AB123456789AB
//      123456789AB7654321|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|   
//                 b-4=7
//1LLLRRRxxx                     LLLLLLLLLLLLLLL                  LLLLLrrrzzz                          LLLLLLLLLLL                      LLLLLLLLLLL                      LLLLLLLLLLL 
//2         LLLLRRRyyy                          LRRRxxx                      LLLLLLLLLLLLLLL                      LLLLrrrxxx                      LLLLLLLLLLL                      LLLLLLLLLLL
//3                   LLLLLRRRzzz                      LLLLLRRRyyy                          LLLLLLLLLLL                     LLLLLLLLLLLL                      LLLLLLLLLLL                      LLLL
//                 S      0   S      0          +   S      0   S      0   S      0          +          +          +      s   0          +          +          +          +          +          +          
//                 3      0   1                     2          3          1                                              2                      
//                            0=1                   0=2        0=3        0=1                                                                                                                                                                                                             
//                                                                                                                1234567  7-4 = 3 offset
//
//runon packet:
//    111       222        333        111        222        333        111        222        333    .  111        222        333        111        222        333        111
//XXXtTTToooYYYtTTTZZZ____tTTT___________XXX____tTTTYYY____tTTTZZZ____tTTT___________________________XXX_________tTTTYYY____tTTTZZZ____tTTT
//   123456789AB123456789AB123456789AB123456789AB123456789AB123456789AB123456789AB123456789AB12345679AB123456789AB123456789AB123456789AB123456789AB123456789AB123456789AB123456789AB
//      123456789AB7654321|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|123456789A|   
//                 b-4=7                                                                                                                                               
//1LLLRRRxxx                     LLLLLLLLLLLLLLL                  LLLLLrrrzzz                          LLLLLLLLLLL                  LLLLrrrzzz                           LLLLLLLLLLL 
//2         LLLLRRRyyy                          LRRRxxx                      LLLLLLLLLLLLLLL                      rrrxxx                      LLLLLLLLLLLLLLLL                      LLLLLLLLLLL
//3                   LLLLLRRRzzz                      LLLLLRRRyyy                          LLLLLLLLLLL                 LLLLLLrrryyy                          LLLLLLLLLLL                      LLLL
//                 S      0   S      0          +   S      0   S      0   S      0          +          +          +  S       0   S      0  S       0          +          +          +          +          
//                 3      0   1                     2          3          1                                          3           1         2      
//                            0=1                   0=2        0=3        0=1                                        0=3                   0=2                                                                                                                                                   
//                                                         1          1                                           123  3-4 = -1 offset
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
// 
//#define SKIP_CALC_TX_TIME 1


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
#define DELAY_BTW_NEXT_DIAL 0xe001
#define TO_BTW_CHARS 0xff00

#define TIME_FOR_PACKET 0xff98
#define TIME_FOR_PACKET0 0xe123
//#define DELAY_BTW_SEND_PACKET 0xffa3
#define DELAY_BTW_SEND_PACKET 0xffd1
/////////////////////////////////////////////////////
//#define _OLD_VERSION 1
//#define __DEBUG
//#define SHOW_RX_TX
//#define SHOW_RX
//#define FLASH_POWER_DOWN 1
/////////////////////////////////////////////////////////////////////////////////////////////////////
// define blinking LED on pin 14 (RC3)
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define DEBUG_LED
#ifdef DEBUG_LED
#define DEBUG_LED_OFF bitclr(LATA,5)
#define DEBUG_LED_ON bitset(LATA,5)
///////////////////////////////////////////////////////////////
//   for a blinking LED behive like CUBESAT/CRAFT
//   it is waiting for connection, wait for pkt, and when pkt is Ok it send back to earth reply packet, and blinks
///////////////////////////////////////////////////////////////
//#define DEBUG_LED_CALL_EARTH
// for test sequence 
/// "5atsx=...CBabbcgg
// atdtl
// 5"

///////////////////////////////////////////////////////////////
//   for a blinking LED behive like Ground Station, it is constantly sends pktm if received pkt, then it blinks
///////////////////////////////////////////////////////////////
#define DEBUG_LED_CALL_LUNA
// for test sequence 
// "5atsx=...CBabbcgg
// atdtl
// 5"
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


#include "commc0.h"


#pragma rambank RAM_BANK_4
///////////////////////////////////////BANK 4//////////////////////////





#ifdef BT_TIMER1
struct _Data_B0{
unsigned Timer1Meausre:1;
unsigned Timer1Count:1;
unsigned Timer1Inturrupt:1;
unsigned Timer1DoTX:1;
unsigned Timer1Done3FQ:1;
unsigned Timer1SwitchTX:1;
#ifdef BT_TIMER3
unsigned Tmr3DoMeausreFq1_Fq2:1;
unsigned Tmr3Run:1;
unsigned Tmr3Inturrupt:1;
unsigned Tmr3DoneMeasureFq1Fq2:1;
unsigned Timer3Ready2Sync:1;
unsigned Tmr3RxFqSwitchLost:1;
unsigned Timer3OutSyncRQ:1;
unsigned Timer3SwitchRX:1;
unsigned TransmitESC:1;
unsigned AlowSwitchFq1ToFq3:1;
#endif
} DataB0;
unsigned char FqTXCount;
unsigned char FqTX;
unsigned char TXSendOverFQ;
#ifdef _18F2321_18F25K20
UWORD TIMER0 @ 0xFD6;
UWORD TIMER1 @ 0xFCE;
UWORD TIMER3 @ 0xFB2;
#endif

UWORD Timer1Id;

UWORD Tmr1High; // to count for a 536 sec with presision of 0.000000125s== 1/4mks == 37.5m
                // UWORD in TMR1 counts till 0.008192 s = 8.192ms

UWORD Tmr1LoadLow;  // timer1 interupt reload values 
UWORD Tmr1LoadHigh; // this will 
UWORD Tmr1TOHigh;   // this overload value will generate interrupts and reload timer

#ifdef BT_TIMER3
unsigned char FqRXCount;
unsigned char FqRX;

//UWORD tBtwFqRXSwitch;
//UWORD tBtwFqRXSwitchHigh;


UWORD Tmr3High; // to count for a 536 sec with presision of 0.000000125s== 1/4mks == 37.5m
                // UWORD in TMR3 counts till 0.008192 s = 8.192ms

UWORD Tmr3LoadLow;  // timer3 interupt reload values 
UWORD Tmr3LoadHigh;
UWORD Tmr3TOHigh; 
unsigned char SkipPtr;
UWORD AdjustTimer3;
UWORD Tmr3LoadLowCopy;
unsigned char OutSyncCounter;
unsigned char ESCCount;
unsigned char AfterESCChar;
unsigned char CountFQ3;
#endif
#else
struct _Data_B0{
unsigned TimerPresc:1;
unsigned Timer:1;
unsigned SetBitsCmd:1;
//unsigned Timer0Waiting:1;
} DataB0;
#endif


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
#pragma rambank RAM_BANK_1
///////////////////////////////////////BANK 1//////////////////////////


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
#define PCKT_DIAL 0xf0
#define PCKT_DATA 0xf1
unsigned char BTpkt;
unsigned char RXreceiveFQ;
unsigned char BTFQcurr;
unsigned char BTType;
unsigned char BTokMsg;
#define BT_TX_MAX_LEN (28-7-2)

unsigned char BTqueueOut[BT_TX_MAX_LEN];
unsigned char BTqueueOutLen;

unsigned char BTpktCopy;
unsigned char BTqueueOutCopy[BT_TX_MAX_LEN];
unsigned char BTqueueOutCopyLen;

#define LEN_OFFSET_INPUT_BUF 28
unsigned char BTqueueIn[28];
unsigned char BTqueueInLen;

unsigned char BTqueueIn2[28];
unsigned char BTqueueInLen2;

unsigned char BTqueueIn3[28];
unsigned char BTqueueInLen3;
struct _BTFLAGS
{
    unsigned BT3fqProcessed:1;
    unsigned BTFirstInit:1;
    unsigned BTNeedsTX:1;
    unsigned BTFixed:1;
} BTFlags;
UWORD CRC;
UWORD CRCcmp;
unsigned char PktCount;
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

};


void wCRCupdt(int bByte);
unsigned char SendBTcmd(unsigned char cmd);
void SendBTbyte(unsigned char cmd);
unsigned char GetBTbyte(void);
void SetupBT(unsigned char SetupBtMode);
void SwitchFQ(unsigned char iFQ);
void SwitchToRXdata(void);
void ProcessBTdata(void);
unsigned char ReceiveBTdata(void);
void TransmitBTdata(void);
void ReceiveBTLost(void);
void BTCE_low(void);
void BTCE_high(void);
void SetTimer3(UWORD iTime);
void SetTimer1(UWORD iTime);
//
// additional variables:
#include "commc1.h"



//
// additional code:

unsigned char CallBkComm(void); // return 1 == process queue; 0 == do not process; 
                                // 2 = do not process and finish process 3 == process and finish internal process
                                // in case 0 fucntion needs to pop queue byte by itself
#ifndef NO_I2C_PROC
unsigned char CallBkI2C(void); // return 1 == process queue; 0 == do not process; 
                               // 2 = do not process and finish process 3 == process and finish internal process
                               // in case 0 fucntion needs to pop queue byte by itself
unsigned char getchI2C(void);
#endif
unsigned char CallBkMain(void); // 0 = do continue; 1 = process queues
void Reset_device(void);
void ShowMessage(void);
void ProcessCMD(unsigned char bByte);
void putch(unsigned char simbol);
void putchWithESC(unsigned char simbol);
unsigned char getch(void);
void putch_main(void);

void InitModem(void)
{
    ATCMDStatus = 0;
    ATCMD = 0;
    FqTX = Freq1;
    FqTXCount = 0;
    BTFQcurr = Freq1;
        
    RXreceiveFQ = 0;
    TXSendOverFQ = 0;
    FqRXCount = 0;
    FqRX = Freq1;
    FqTXCount = 0;
    FqTX = Freq1;
    BTType = 0;
    BTokMsg = 0xff;
#ifdef BT_TIMER1
    DataB0.Timer1Done3FQ = 0;
    DataB0.Tmr3DoneMeasureFq1Fq2 = 0;
    //DataB0.Time3JustDone = 0;
    DataB0.Tmr3DoMeausreFq1_Fq2 = 0;
    DataB0.Tmr3RxFqSwitchLost = 0;
    DataB0.Timer3OutSyncRQ = 0;
    // 1 = round robin on FQ1->Fq2->Fq3
    // 0 = working on FQ1 only
    DataB0.Timer3SwitchRX = 1;
    DataB0.Timer1SwitchTX = 1;
    DataB0.Timer3Ready2Sync = 0;
    DataB0.AlowSwitchFq1ToFq3 = 1;
    DataB0.TransmitESC = 0;
    //DataB0.Timer3SkipFQ2 = 0;
    //DataB0.Timer3SkipFQ3 = 0;
#endif
    Timer1Id = 0;
}
#ifdef NON_STANDART_MODEM
// adr 0z000000 (0) == Adr2B = 0x00  (var Ard2BH = 0x00)
// adr 0x001000 (4K)== Adr2B = 0x10  (var Ard2BH = 0x00)
// adr 0x002000 (8k)== Adr2B = 0x20  (var Ard2BH = 0x00)
// adr 0x010000 (65536)Adr2B = 0x00  (var Adr2BH = 0x01)

unsigned char AdrBH;
unsigned int  wAddr;

unsigned char FlashEntryBH;
unsigned int  FlashEntry;

unsigned char FlashExitBH;
unsigned int  FlashExit;

unsigned char FlashQueueSizeBH;
unsigned int  FlashQueueSize;

#define FLASH_BUFFER_LEN_BH      0x01
#define FLASH_BUFFER_LEN         0x2000

#define FLASH_BUFFER_HALF_LEN_BH 0x00
#define FLASH_BUFFER_HALF_LEN    0x8100

// structure of a packet acsepted from BT
unsigned char PrevLen;
unsigned char NextLen;
unsigned char TypePkt;

unsigned char ReadPrevLen;
unsigned char ReadNextLen;
unsigned char ReadTypePkt;

void Erace4K(unsigned char Adr2B);
#endif

void main()
{
    unsigned char bWork;

    Reset_device();
    // needs to check what is it:
    //if (TO) // Power up or MCLR
    {
        // Unit == 1 (one) is ADC and unit == 2 (two) is DAC 
        // Unit == 3 Gyro
        UnitADR = MY_UNIT;//'5'; // BT communication module
        //UnitADR = '8';
        //UnitADR = '4';
#include "commc6.h"


        Config01 = eeprom_read(0x30);
        Freq1 = eeprom_read(0x31);
        Freq2 = eeprom_read(0x32);
        Freq3 = eeprom_read(0x33);
        Addr1 = eeprom_read(0x34);
        Addr2 = eeprom_read(0x35);
        Addr3 = eeprom_read(0x36);
        InitModem();
#ifdef BT_TIMER1
#else
        DataB0.TimerPresc = 0;
        DataB0.Timer = 0;
        //DataB0.Timer0Waiting = 0;
        DataB0.SetBitsCmd = 0;
        
#endif
        DataB3.FlashCmd = 0;
        DataB3.FlashCmdLen = 0;
        DataB3.FlashRead = 0;

        
#ifdef SSPORT
        CS_HIGH;
        bclr(SSPORT,SSCLOCK);
        bclr(SSPORT,SSDATA_IN);
#endif
        //SubG[0].xxL = 2;
        //SubG[0].xxH = 0;
        //FSR_REGISTER = (char*)&SubG[0].xxL;       
        //W = SubG[0].xxH;
        //PTR_FSR -= W;
        // if no borrow then Carry is set
        // if borrow then Carry is clear
    }
    Main.ExtInterrupt = 0;  // clean interrupts
    Main.ExtInterrupt1 = 0;
    Main.ExtInterrupt2 = 0;
    Main.ExtFirst = 1;
    BTqueueOutLen = 0;
    //BTInturrupt = 0;
    BTFlags.BTFirstInit = 0;
    BTFlags.BTNeedsTX = 0;
    BTokMsg = 0xff;
    PktCount = 0;
    SkipPtr = 0;
    ESCCount = 0;
#ifdef SKIP_CALC_TX_TIME
    TMR1ON = 0;              // stop timer measure it time btw send Fq1 -> Fq2
    DataB0.Timer1Done3FQ = 1; 
    DataB0.Timer1Meausre = 0;
    DataB0.Timer1Count = 1;

    Tmr1LoadHigh = 0xffff;
    Tmr1LoadLow = 0x8975;      // timer1 interupt reload values 
    Tmr1TOHigh = Tmr1LoadHigh;
    SetTimer1(Tmr1LoadLow);
#endif

#ifdef NON_STANDART_MODEM
    //Adr2BH = 0;
    //Erace4K(0);
    // TBD: search for the begining of a queue
    FlashEntryBH = 0;
    FlashEntry = 0;

    FlashExitBH = 0;
    FlashExit = 0;

    FlashQueueSizeBH = 0;
    FlashQueueSize = 0;

    PrevLen = 0;
    NextLen = 0;

    // needs to search for packets and process it if they was stored
#endif

#ifdef DEBUG_LED
    DEBUG_LED_OFF;
#endif

#ifndef __PIC24H__
    PEIE = 1;    // bit 6 PEIE: Peripheral Interrupt Enable bit
                 // 1 = Enables all unmasked peripheral interrupts
                 // 0 = Disables all peripheral interrupts

    GIE = 1;     // bit 7 GIE: Global Interrupt Enable bit
                 // 1 = Enables all unmasked interrupts
                 // 0 = Disables all interrupts
    RBIF = 0;
#endif



//    PLLEN = 1;
#if 0
            // F\x01\x06              == write enable (flash command 06) -> send 0x06
            // F\x01\0xc7             == erase all flash                 -> send =0xc7
            // F\x05\x03\x00\x12\x34@\x04 == read 4 bytes from a address 0x001234  -> send 0x03 0x00 0x12 0x34 <- read 4 bytes (must not to cross boundary)
            // F\x01\x06F\x0c\x02\x00\x11\x22\x00\x00\x00\x00\x00\x00\x00\x00 == write 8 bytes to address 0x001122
    CS_LOW;
    SendSSByte(0x06);
    CS_HIGH;
    nop();
    CS_LOW;
    SendSSByte(0xc7);
    CS_HIGH;
    nop();


    CS_LOW;
    SendSSByte(0x06);
    CS_HIGH;
    nop();
    CS_LOW;
    SendSSByte(0x02);
    //SendSSByte(0xad);
    SendSSByte(0x00);
    SendSSByte(0x00);
    SendSSByte(0x00);

    SendSSByte(0x01);
    SendSSByte(0x02);
    SendSSByte(0x03);
    SendSSByte(0x04);
    SendSSByte(0x05);
    SendSSByte(0x06);
    //SendSSByte(0x07);
    //SendSSByte(0x08);
    CS_HIGH;
#endif
#if 0
    nop();
    CS_LOW;
    SendSSByte(0x03);
    SendSSByte(0x00);
    SendSSByte(0x00);
    SendSSByte(0x00);

    bWork = GetSSByte();
    bWork = GetSSByte();
    bWork = GetSSByte();
    bWork = GetSSByte();
    bWork = GetSSByte();
    bWork = GetSSByte();
    bWork = GetSSByte();
    bWork = GetSSByte();
    bWork = GetSSByte();
    bWork = GetSSByte();
    CS_HIGH;

    nop();
    CS_LOW;
    SendSSByte(0x05);
    bWork = GetSSByte();
    CS_HIGH;
    nop();
//#ifdef SST25VF032
//    CS_LOW;
//    SendSSByte(0x50);
//    CS_HIGH;
//    nop();
//    CS_LOW;
//    SendSSByte(0x01);
//    SendSSByte(0x00);
//    CS_HIGH;
//    nop();
//#endif

#endif

    ShowMessage();
#ifdef DEBUG_LED_CALL_EARTH
                DataB0.Timer3SwitchRX = 0;
                DataB0.Timer1SwitchTX = 0;
                DataB0.AlowSwitchFq1ToFq3 = 0;

                if (Config01&0x1)
                    DataB0.Timer3SwitchRX = 1;
                if (Config01&0x2)
                    DataB0.Timer1SwitchTX = 1;
                if (Config01&0x4)
                    DataB0.AlowSwitchFq1ToFq3 = 1;

    ATCMD = MODE_CALL_EARTH;     
    ATCMD |= INIT_BT_NOT_DONE;
    INT0_ENBL = 1;
#endif
#ifdef DEBUG_LED_CALL_LUNA
                DataB0.Timer3SwitchRX = 0;
                DataB0.Timer1SwitchTX = 0;
                DataB0.AlowSwitchFq1ToFq3 = 0;

                if (Config01&0x1)
                    DataB0.Timer3SwitchRX = 1;
                if (Config01&0x2)
                    DataB0.Timer1SwitchTX = 1;
                if (Config01&0x4)
                    DataB0.AlowSwitchFq1ToFq3 = 1;

    ATCMD = MODE_CALL_LUNA_COM;
    ATCMD |= INIT_BT_NOT_DONE;
    INT0_ENBL = 1;
    
#endif
    CountFQ3 = 2;
    //bitset(PORTA,4);
    //bitset(PORTA,3);
    //bitset(SSPCON,4);  // set clock high;
#include "commc7.h"


} // at the end will be Sleep which then continue to main







// for pic18f2321
#define SPBRG_SPEED SPBRG_57600_64MHZ
//#define SPBRG_SPEED SPBRG_38400_32MHZ
//#define SPBRG_SPEED SPBRG_19200_32MHZ
// for pic24hj64gp502
//#define SPBRG_SPEED SPBRG_57600_40MIPS

#include "commc2.h"



// additional code:
#ifdef NON_STANDART_MODEM
// adr 0z000000 (0) == Adr2B = 0x00  (var Ard2BH = 0x00)
// adr 0x001000 (4K)== Adr2B = 0x10  (var Ard2BH = 0x00)
// adr 0x002000 (8k)== Adr2B = 0x20  (var Ard2BH = 0x00)
// adr 0x010000 (65536)Adr2B = 0x00  (var Adr2BH = 0x01)

void CheckStatus(void)
{
    unsigned char StatusByte = 0x80;
    CS_LOW;
    SendSSByte(0x05);
    while( StatusByte & 0x80)
    {
        StatusByte = GetSSByte();
    }
    CS_HIGH;
}
void GetFromFlash( unsigned char bByte)
{
    unsigned char bRet;
    if (btest(SSPORT,SSCS)) // is it HIGH ???
    {
        CheckStatus(); 
        SendSSByte(0x03);
        SendSSByte(AdrBH);
        SendSSByte(wAddr>>8);
        SendSSByte(wAddr&0xff);
    }
    bRet = GetSSByte();
    wAddr++;
    if ((wAddr &0xff) == 0) // page write done
    {
        CS_HIGH;
        if (wAddr  == 0) // over 64K
            AdrBH++;
    }    
}
void Push2Flash( unsigned char bByte)
{
    if (btest(SSPORT,SSCS)) // is it HIGH ???
    {
        CheckStatus();
        CS_LOW;
        SendSSByte(0x06);
        CS_HIGH;
        nop();
        CS_LOW; 
        SendSSByte(0x02);
        SendSSByte(AdrBH);
        SendSSByte(wAddr>>8);
        SendSSByte(wAddr&0xff);
    }
    SendSSByte(bByte);// 10mks after write :: in 16MHz it ig 10 command in 40MIPS it is 400 command
                      // in some FLASH memory on first read it can be 50 mks and 10 mks next
    wAddr++;
    if ((wAddr &0xff) == 0) // page write done
    {
        CS_HIGH;
        if (wAddr  == 0) // over 64K
            AdrBH++;
    }    
}
void Erace4K(unsigned char Adr2B)
{
    CheckStatus();
    // erace first 4K for buffer
    // F\x01\x06F\x04\x20\x00\x00\x00  // address 000000
    CS_LOW;
    SendSSByte(0x06);
    CS_HIGH;
    nop();
    CS_LOW; 
    SendSSByte(0x20);
    SendSSByte(AdrBH);
    SendSSByte(Adr2B);
    SendSSByte(0x00); 
    CS_HIGH;
}
#endif


void Reset_device(void);
void ShowMessage(void);

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

#include "commc3.h"

/////////////////////////////////////////////////////////////////////
// additional code proceessed as Cmd :
/////////////////////////////////////////////////////////////////////

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
              if (bByte == 0x0d) // CR == AT command done
              {
                  ATCMDStatus = 0;
                  Main.DoneWithCMD = 1; // long command done
              }
              else
                  ATCMDStatus = 3; // on next entry will be 4
              return;
         }
         if (bByte == 0x0d) // CR == AT command done
         {
             ATCMDStatus = 0;
             Main.DoneWithCMD = 1; // long command done
             if (ATCMD)
                 ATCMD |= INIT_BT_NOT_DONE;  // execute mode
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
              // ATDTlunaU - connect to CubeSat and use I2C for tunneling data to unit
              //         U = '0'-'7' data transfered to unit '0'-'7'
              // ATDTLUNAU - connect to CubeSat and use Comm for tranfering data
              //         U = 'X' to relay all data as it is, '0'-'7' to transfer to unit
              //         basically "atdtLUNAX" set visible mode to transfer
              // when connection established it relpy with "CONNECT" and com will be in relay state (no unit wrapping)
              ////////////////////////////////////////////////////////////////////////////
              // ATDTEARTH - set module in listen mode on FQ1 and when connection esatblished it reply with "CONNECT" to a unit
              ////////////////////////////////////////////////////////////////////////////
             ATCMD = 0;
             if (bByte == 'e') // atdtEARTH
                 ATCMD = MODE_CALL_EARTH;//1;

             else if (bByte == 'l') // atdtluna
                 ATCMD = MODE_CALL_LUNA_COM;//4;
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
                DataB0.Timer3SwitchRX = 0;
                DataB0.Timer1SwitchTX = 0;
                DataB0.AlowSwitchFq1ToFq3 = 0;

                if (Config01&0x1)
                    DataB0.Timer3SwitchRX = 1;
                if (Config01&0x2)
                    DataB0.Timer1SwitchTX = 1;
                if (Config01&0x4)
                    DataB0.AlowSwitchFq1ToFq3 = 1;

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
                BTFQcurr = Freq1;
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

#include "commc4.h"


// additional code:
//
#ifdef NON_STANDART_MODEM
        else if (bByte == '$') // send everything to connected BT till end of the packet and ask to retransmit data to link
        {
            if (ATCMD & MODE_CONNECT) // was connection esatblished?
            {
                 Main.SendOverLink = 1;
                 Main.SendOverLinkAndProc = 1;
            }
            // TBD: on no connection established - needs to do something with uplink data - it is going to nowhere
        }
        else if (bByte == '*') // send everything to connected BT till end of the packet and process inside BT 
        {
            if (ATCMD & MODE_CONNECT) // was connection esatblished?
            {
                 Main.SendOverLink = 1;
            }
            // TBD: on no connection established - needs to do something with uplink data - it is going to nowhere
        }
#endif
        else if (bByte == 'a')
        {
            ATCMDStatus = 1;
            Main.DoneWithCMD = 0; // long command
        }

SKIP_BYTE:;
    } // do not confuse: this is a else from getCMD == 1
}
#pragma codepage 1

void PutsToUnit(const char * s)
{
    if (UnitFrom)
    {
        Main.SendWithEsc = 1;
        putch(UnitFrom);
        if (SendCMD)
            putch(SendCMD);
    }
    Puts(s);
    if (UnitFrom)
        putch(UnitFrom);
}

void SetTimer0(UWORD iTime)
{
     Timer1Id = iTime;
     TMR0ON = 0;
     I2C.Timer0Fired = 0;
     //DataB0.Timer0Waiting = 1;
     //TIMER0_BYTE = iTime;
     TMR0H = (unsigned char)(iTime>>8);
     TMR0L = (unsigned char)(iTime&0xff);
     
     
     //TIMER0 = iTime;

     TIMER0_INT_FLG = 0; // clean timer0 interrupt
     TIMER0_INT_ENBL = 1;  // enable timer0 interrupt
     // 1         bit 7 TMR0ON: Timer0 On/Off Control bit
     //               1 = Enables Timer0
     //               0 = Stops Timer0
     //  0        bit 6 T08BIT: Timer0 8-Bit/16-Bit Control bit
     //               1 = Timer0 is configured as an 8-bit timer/counter
     //               0 = Timer0 is configured as a 16-bit timer/counter
     //   0       bit 5 T0CS: Timer0 Clock Source Select bit
     //               1 = Transition on T0CKI pin
     //               0 = Internal instruction cycle clock (CLKO)
     //    0      bit 4 T0SE: Timer0 Source Edge Select bit
     //               1 = Increment on high-to-low transition on T0CKI pin
     //               0 = Increment on low-to-high transition on T0CKI pin
     //     0     bit 3 PSA: Timer0 Prescaler Assignment bit
     //               1 = TImer0 prescaler is NOT assigned. Timer0 clock input bypasses prescaler.
     //               0 = Timer0 prescaler is assigned. Timer0 clock input comes from prescaler output.
     //      110  bit 2-0 T0PS<2:0>: Timer0 Prescaler Select bits
     //           111 = 1:256 Prescale value
     //           110 = 1:128 Prescale value
     //           101 = 1:64 Prescale value
     //           100 = 1:32 Prescale value
     //           011 = 1:16 Prescale value
     //           010 = 1:8 Prescale value
     //           001 = 1:4 Prescale value
     //           000 = 1:2 Prescale value
#if 0
     T0SE = 0;
     T0PS = 0b110;
     //T08BIT = 1; // set timer 0 as a byte counter
     T08BIT = 0; // set timer 0 as a word counter
     TMR0ON = 1;
#else
#ifdef _18F25K20
     T0CON = 0b10000111;
#else
     T0CON = 0b10000110;
#endif
#endif
}
void SetTimer1(UWORD iTime)
{
     TMR1ON = 0;
     TMR1H = (unsigned char)(iTime>>8);
     TMR1L = (unsigned char)(iTime&0xff);
     //TIMER1 = iTime;
     TMR1IF = 0; // clean timer1 interrupt
     TMR1IE = 1;  // enable timer1 interrupt
     // 1         bit 7 RD16: 16-Bit Read/Write Mode Enable bit
     //               1 = Enables register read/write of TImer1 in one 16-bit operation
     //               0 = Enables register read/write of Timer1 in two 8-bit operations
     //  0        bit 6 T1RUN: Timer1 System Clock Status bit
     //               1 = Device clock is derived from Timer1 oscillator
     //               0 = Device clock is derived from another source
     //   00      bit 5-4 T1CKPS<1:0>: Timer1 Input Clock Prescale Select bits
     //              11 = 1:8 Prescale value
     //              10 = 1:4 Prescale value
     //              01 = 1:2 Prescale value
     //              00 = 1:1 Prescale value
     //     0     bit 3 T1OSCEN: Timer1 Oscillator Enable bit
     //               1 = Timer1 oscillator is enabled
     //               0 = Timer1 oscillator is shut off
     //             The oscillator inverter and feedback resistor are turned off to eliminate power drain.
     //      0    bit 2 T1SYNC: Timer1 External Clock Input Synchronization Select bit
     //              When TMR1CS = 1:
     //                1 = Do not synchronize external clock input
     //                0 = Synchronize external clock input
     //              When TMR1CS = 0:
     //                This bit is ignored. Timer1 uses the internal clock when TMR1CS = 0.
     //       0   bit 1 TMR1CS: Timer1 Clock Source Select bit
     //               1 = External clock from pin RC0/T1OSO/T13CKI (on the rising edge)
     //               0 = Internal clock (FOSC/4)
     //        1  bit 0 TMR1ON: Timer1 On bit
     //               1 = Enables Timer1
     //               0 = Stops Timer1
     T1CON = 0b10000001;
}

void SetTimer3(UWORD iTime)
{
     TMR3ON = 0;
     TMR3H = (unsigned char)(iTime>>8);
     TMR3L = (unsigned char)(iTime&0xff);
     //TIMER3 = iTime;
     TMR3IF = 0; // clean timer1 interrupt
     TMR3IE = 1;  // enable timer1 interrupt
     // 1            bit 7 RD16: 16-Bit Read/Write Mode Enable bit
     //                1 = Enables register read/write of Timer3 in one 16-bit operation
     //                0 = Enables register read/write of Timer3 in two 8-bit operations
     //  0  0        bit 6,3 T3CCP<2:1>: Timer3 and Timer1 to CCPx Enable bits
     //                1x = Timer3 is the capture/compare clock source for the CCP modules
     //                01 = Timer3 is the capture/compare clock source for CCP2; Timer1 is the capture/compare
     //                     clock source for CCP1
     //                00 = Timer1 is the capture/compare clock source for the CCP modules
     //   00         bit 5-4 T3CKPS<1:0>: Timer3 Input Clock Prescale Select bits
     //                11 = 1:8 Prescale value
     //                10 = 1:4 Prescale value
     //                01 = 1:2 Prescale value
     //                00 = 1:1 Prescale value
     //      0       bit 2 T3SYNC: Timer3 External Clock Input Synchronization Control bit
     //                     (Not usable if the device clock comes from Timer1/Timer3.)
     //                 When TMR3CS = 1:
     //                   1 = Do not synchronize external clock input
     //                   0 = Synchronize external clock input
     //                 When TMR3CS = 0:  
     //                   This bit is ignored. Timer3 uses the internal clock when TMR3CS = 0.
     //       0      bit 1 TMR3CS: Timer3 Clock Source Select bit
     //                1 = External clock input from Timer1 oscillator or T13CKI (on the rising edge after the first
     //                    falling edge)
     //                0 = Internal clock (FOSC/4)
     //        1     bit 0 TMR3ON: Timer3 On bit
     //                1 = Enables Timer3
     //                0 = Stops Timer3
     T3CON = 0b10000001;
}
unsigned char DoFqRXSwitch(void)
{
    unsigned char bByte;
    
    //TMR3ON = 0; // stop TMR3 for a little bit
    // or may be not this call comes after INT == that mean inside timer3 it will be not advanced
    // counter SkipPtr prevents from this
    // second place it is a timer0 == this is before timer3 started to work
    if (++FqRXCount>=3)
    {
        FqRXCount = 0;
        FqRX = Freq1;
    }
    else
    {
        if (FqRXCount == 1)
            FqRX = Freq2;
        else
            FqRX = Freq3;
    }
    bByte = FqRX;
    // TBD may be needs to adjust timer 3 ???
    //TMR3ON = 1;
    return bByte;
}
unsigned char CheckSuddenRX(void)
{
    if (DataB0.Timer3SwitchRX)
    {
        BTCE_low();  // Chip Enable Activates RX (now disable) == standby
    }
    bitclr(PORT_BT,Tx_CSN); // SPI Chip Select
    BTStatus=SendBTcmd(0xff); //cmd = 0xff; = 1111 1111 command NOP to read STATUS register
    bitset(PORT_BT,Tx_CSN); // set high
    if (BTStatus & 0x40) // it is unprocessed interrupt on RX
        return 1;
    
    return 0;

}
/////////////////////////////////////////////////////////////////////////////////////////////////
//
//            COM CALL BACK data ready from serial input
//            return 1 == process queue; 0 == do not process;
//            in case of next unit is not ready it is not good idea to process
//            byte if it will generate some serial output
/////////////////////////////////////////////////////////////////////////////////////////////////
unsigned char CallBkComm(void)  
{
    unsigned char bReturn = 0;
    unsigned char bByte;
    if (TX_NOT_READY)
    {
        // commands:
        // 
        if (Main.DoneWithCMD) // long command ??
        {
            if (DataB3.FlashCmd)
                if (DataB3.FlashRead)
                    return 0;  // better do not process byte to read from flash - next unit can not acsepted it 

        }
        else
        {
            // '=', '~', 'F', '*', 'a' - OK to process
        }
    }
#ifdef NON_STANDART_MODEM
    //   Main.SendOverLink == 0 => return from call back and process com queue
    if (!Main.SendOverLink)   // if command * was send then data has to be transferred to up/down link
        return 1;             // untill end of the packet
#endif
    if (ATCMD & MODE_CONNECT) // was connection esatblished
    {
        if ((ATCMD & MODE_CALL_LUNA_COM) 

           ) // calling CubSat
        {
SEND_BT:    
            // TBD this is a place where ++++ can be checked for disconnect == needs to accumulate data in InQueu and on ++++ disconnect from remote
            
            // returning from call back 0 will block command's processing 
            if (BTqueueOutLen < BT_TX_MAX_LEN) // enought in output buffer needs to send data to CubSat
            {
                TMR0ON = 0;
                BTpkt = PCKT_DATA;
#ifndef NON_STANDART_MODEM                
                if (DataB0.TransmitESC)
                {
                    if (ESCCount)
                    {
OUT_ESC_CHARS:
                        ESCCount--;
                        bByte = '+';
                        goto PUSH_TO_BT_QUEUE;
                    }
                    else
                    {
                        bByte = AfterESCChar;
                        DataB0.TransmitESC = 0;
                        goto PUSH_TO_BT_QUEUE;
                    }
                }
#endif
                bByte = getch();
#ifdef NON_STANDART_MODEM
                if (Main.ESCNextByte)
                {
                    Main.ESCNextByte = 0;
                    goto PUSH_TO_BT_QUEUE;
                }
                else
                {
                    if (bByte == ESC_SYMB)
                    {
                        Main.ESCNextByte = 1;
                        goto PUSH_TO_BT_QUEUE;
                    }
                    else if (bByte == MY_UNIT)
                    {
                        Main.getCMD = 0; // CMD stream done
                        Main.SendOverLink = 0;
                        return 0; 
                    }
                }
#else           // standart modem implementation
                if (bByte == '+')
                {
                    if (++ESCCount >=4) // disconnect condition
                    {
                        ATCMD = 0;
                        ESCCount = 0;
                        PutsToUnit("\r\nOK\r\n");
                    }
                    return 0;
                }
                else
                {
                    if (ESCCount)
                    {
                        AfterESCChar = bByte;
                        DataB0.TransmitESC = 1;
                        goto OUT_ESC_CHARS;
                    }
                }
#endif
PUSH_TO_BT_QUEUE:       
                ATCMD |= SOME_DATA_OUT_BUFFER;
                BTqueueOut[BTqueueOutLen] = bByte;//
                if (++BTqueueOutLen >= BT_TX_MAX_LEN) // buffer full
                    goto SET_FLAG;
                else // still present space in buffer wait for a next char
                {
                    if (RXreceiveFQ == 0) // only if lstenning on FQ1
                    {
                        if (BTType & 0x01) // only in  RX mode set timer
                           SetTimer0(TO_BTW_CHARS);//0xff00);
                    }
                }
            }
            else // no space in buffer but chars is coming == left it in queue in hope that they will not owerfrlow buffer
            {
                // if BT in RX mode set request to transmit
                // in this moment (before transmit happened) input queue is blocked before TX set 
SET_FLAG:
                if (BTType & 0x01) // only in  RX mode set flag
                    BTFlags.BTNeedsTX = 1;
            }
            return 0; // this will block retreiving data from com queue
        }
        else if (ATCMD & MODE_CALL_EARTH) // calling earth (it can be relay data from a loop or communication with some devices)
        {
#ifndef NON_STANDART_MODEM
            if (UnitFrom == 0)
#endif
               goto SEND_BT;
        }
    }
    return 1;                          // do process data
}
#ifndef NO_I2C_PROC
unsigned char CallBkI2C(void)// return 1 == process queue; 0 == do not process; 
                               // 2 = do not process and finish process 3 == process and finish internal process
{                              // in case 0 fucntion needs to pop queue byte by itself
    unsigned char bReturn = 0;
    if (Main.ComNotI2C) // prev was COM processing
    {
        if (Main.DoneWithCMD)  // it is CMD processing now continue end of a CMD from com
        {
            Main.ComNotI2C = 0;
            bReturn = 1;                    // do process data   
        }
    }
    else               // prev was I2C data processing
    {
        //Main.ComNotI2C = 0;
        bReturn = 1;               // do process data
    }    
    return bReturn;
}
#endif
///////////////////////////////////////////////////////////////////////////////////////////
//      
//          PRE PROCESSING CALL BACK
//
///////////////////////////////////////////////////////////////////////////////////////////
unsigned char CallBkMain(void) // 0 = do continue; 1 = process queues
{
    unsigned char bWork;
    if (INT0_ENBL)
    {             
        if (Main.ExtInterrupt) // received interrupt == it can be TX or RX from BT
        {
            // what it was ?
            //BTCE_low();  // Chip Enable Activates RX or TX mode (now disable)

            bitclr(PORT_BT,Tx_CSN); // SPI Chip Select
            BTStatus=SendBTcmd(0xff); //cmd = 0xff; = 1111 1111 command NOP to read STATUS register
            bitset(PORT_BT,Tx_CSN); // set high
MAIN_INT:
            Main.ExtInterrupt = 0;
#if _OLD_VERSION
            if (BTStatus & 0x40) // RX interrupt
            {
                if (BTType & 0x01) // RX
                {
                    //putch('r');
                    // receve timing OK message dial
                    // <receive = 442mks><6mks IRQ> <1051/525 mks process on each FQ><ok msg 102/51mks>
                    // <receive = 442mks><6mks IRQ> <1051/525 mks process on each FQ><not msg 102/51mks>
                    // <receive = 442mks><6mks IRQ> <1051/525 mks process on each FQ><ok msg 102/51mks>|<ok msg data 1602/801mks>
                    // max process = 442mks + 6 + + 102/51 + 2653/1327 = 3207/1826 mks
                    // on FQ3 with max correction :
                    // <receive = 442mks><6mks IRQ> <1051/525 mks process on each FQ><correction 3447/1723mks>ok msg 102/51mks>|<ok msg data 1602/801mks>
                    TMR0ON = 0;
        
                    /////////////////////////////////////////////////////////
                    // TRUE on return mean:
                    // 1. FQ1==OK FQ2,FQ3 return TRUE does not matter how
                    // 2. FQ1!-OK FQ2==OK, and FQ3 return TRUE does not matter how
                    // 3. FQ1!=OK FQ2!=OK, FQ3==OK
                    // 4. FQ1!=OK FQ2!=OK, FQ3!=OK but FQ3 | FQ2 | FQ1 
                    /////////////////////////////////////////////////////////
                    if (ReceiveBTdata()) // packet CRC was OK == that mean FQ2 FQ3 can be tailored and noice canselation can be adjusted
                    {                    // on FQ3 packet will be attempt to fix errors
                        if (RXreceiveFQ == 1) // pkt was ok on FQ1 (after receive switch points of listen frequency)
                        {
                            //SetTimer0(0xff58); // 442mks+xxx=2247(1799??)/898mks == 2693/1340 = count 169/83 
SET_TIMER_AND_PROC:
                            if (!DataB0.Tmr3DoneMeasureFq1Fq2) // DataB0.Timer3DoneMeasure == 0
                                SetTimer0(TIME_FOR_PACKET);//Time4Packet); // count 220 = 3520 mks == why?
                            //putch('0');
PROCESS_DATA:
                            ProcessBTdata();
                            if (RXreceiveFQ == 0) // pkt was not ok on FQ3
                            {
AFTER_PROCESS:                            
                                if (ATCMD & MODE_CONNECT) // connection was established == earth get responce from luna
                                {
                                     // need to set timeout if input data 
                                     goto SET_TO_BTW_CHARS;
                                }
                                else // no connection yet == this is RX after transmit dial packet
                                {
                                    if ((ATCMD & MODE_CALL_LUNA_COM)) // earth calls cubsat
                                       SetTimer0(DELAY_BTW_NEXT_DIAL); // 0x0000 == 4 sec till next attempt for earth to dial luna
                                }
                            }
                            //putch(bWork);
                        }
                        else if (RXreceiveFQ == 2) // after receive FQ2 cases: 1) FQ1==OK FQ2 == does not matter 2) FQ1!=OK FQ2==OK
                        {
                            //putch('1');
                            goto SET_TIMER_AND_PROC;
                            //SetTimer0(0xff58/0xffad); // 442mks+xxx=2247(1799??)/898mks == 2693/1340 = count 169/83 
                            //if (DataB0.Tmr3DoneMeasureFq1Fq2) // adjust time btw packets
                            //{
                            //}
                            //else
                            //if (!DataB0.Tmr3DoneMeasureFq1Fq2)
                            //    SetTimer0(TIME_FOR_PACKET);//Time4Packet); // count 220 = 3520 mks
                            ////putch('1');
                            //if (BTFlags.BT3fqProcessed == 0)
                            //    goto PROCESS_DATA;
                        }
                        else if (RXreceiveFQ == 0) // after receiove FQ3 1)FQ1==OK Fq2,FQ3 doesn not matter 2) FQ1!=OK FQ2==OK FQ3 does not matter 3) FQ1!=OK FQ2!=OK FQ3==OK 4) FQ1|FQ2|FQ3 fixed
                        {
                            //putch('2');
                            //BTokMsg &= 0x7f;
                            goto PROCESS_DATA;
                            //if (BTFlags.BT3fqProcessed == 0)
                            //{
                            //    BTokMsg &= 0x7f;
                            //    goto PROCESS_DATA;
                            //}
                            //goto AFTER_PROCESS;
                        }
                    }
                    else // on receive over FQ1 FQ2 FQ3 if paket is not OK
                    {
                        //putch('-');
                        if (RXreceiveFQ == 0) // pkt was not ok on FQ3
                        {
                            // and fix did not helped == missfortune ! == but may be something can be done?
                            goto AFTER_PROCESS;
                        }
                        else 
                        {
                            if (!DataB0.Tmr3DoneMeasureFq1Fq2) // adjust time btw packets

                                SetTimer0(TIME_FOR_PACKET);//Time4Packet); // count 220 = 3520 mks
                        }
                    }
                }
                else // RX interrupt in a moment of TX operation
                {
                }
                //DataB0.Tmr3Inturrupt = 0;// switch off interrupt from timer 3 if it was fired during processing
            }
#else //////////////////////new version
            if (BTStatus & 0x40) // RX interrupt
            {
                if (BTType & 0x01) // RX
                {
                    //putch('r');
                    // receve timing OK message dial
                    // <receive = 442mks><6mks IRQ> <1051/525 mks process on each FQ><ok msg 102/51mks>
                    // <receive = 442mks><6mks IRQ> <1051/525 mks process on each FQ><not msg 102/51mks>
                    // <receive = 442mks><6mks IRQ> <1051/525 mks process on each FQ><ok msg 102/51mks>|<ok msg data 1602/801mks>
                    // max process = 442mks + 6 + + 102/51 + 2653/1327 = 3207/1826 mks
                    // on FQ3 with max correction :
                    // <receive = 442mks><6mks IRQ> <1051/525 mks process on each FQ><correction 3447/1723mks>ok msg 102/51mks>|<ok msg data 1602/801mks>
                    TMR0ON = 0;
        
                    /////////////////////////////////////////////////////////
                    // TRUE on return mean:
                    // 1. FQ1==OK FQ2,FQ3 return TRUE does not matter how
                    // 2. FQ1!-OK FQ2==OK, and FQ3 return TRUE does not matter how
                    // 3. FQ1!=OK FQ2!=OK, FQ3==OK
                    // 4. FQ1!=OK FQ2!=OK, FQ3!=OK but FQ3 | FQ2 | FQ1 
                    /////////////////////////////////////////////////////////
                    if (ReceiveBTdata()) // packet CRC was OK == that mean FQ2 FQ3 can be tailored and noice canselation can be adjusted
                    {                    // on FQ3 packet will be attempt to fix errors RXreceiveFQ advanced to next frequency
                                         // after call RXreceiveFQ set to next listenning FQ  
                        //if ((RXreceiveFQ == 1) || (RXreceiveFQ == 2)) // pkt was ok on FQ1 (after receive switch points of listen frequency)
                        //{
                            //SetTimer0(0xff58); // 442mks+xxx=2247(1799??)/898mks == 2693/1340 = count 169/83 
SET_TIMER_AND_PROC:
                        if (!DataB0.Tmr3DoneMeasureFq1Fq2) // FQ1-FQ2 time measurement by timer3 was not done 
                        {
                            if (RXreceiveFQ == 1)
                               SetTimer0(TIME_FOR_PACKET0);
                            else
                               SetTimer0(TIME_FOR_PACKET);//Time4Packet); // count 220 = 3520 mks == why?
                        }
PROCESS_DATA:
                        ProcessBTdata();  // call will be processed only one (first) time 
                        if (RXreceiveFQ == 0) // pkt was ok on FQ3
                        {
AFTER_PROCESS:                            
                            if (ATCMD & MODE_CONNECT) // connection was established == earth get responce from luna
                            {
                                // need to set timeout if input data 
                                if (ATCMD & SOME_DATA_OUT_BUFFER) // data was collected to transmit
                                {
                                    if (BTqueueOutLen >= BT_TX_MAX_LEN) // buffer full 
                                        BTFlags.BTNeedsTX = 1;
                                    else 
                                        SetTimer0(TO_BTW_CHARS);//0xff00); // set timeout btw char
                                }
                            }
                            else // no connection yet == this is RX after transmit dial packet
                            {
                                if ((ATCMD & MODE_CALL_LUNA_COM)) // earth calls cubsat
                                   SetTimer0(DELAY_BTW_NEXT_DIAL); // 0x0000 == 4 sec till next attempt for earth to dial luna
                            }
                        }
                    }
                    else // on receive over FQ1 FQ2 FQ3 if paket is not OK
                    {
                       //putch('-');
                        if (RXreceiveFQ == 0) // pkt was not ok on FQ3
                        {
                            // and fix did not helped == missfortune ! == but may be something can be done?
                            goto AFTER_PROCESS;
                        }
                        else 
                        {
                            if (!DataB0.Tmr3DoneMeasureFq1Fq2) // adjust time btw packets
                                SetTimer0(TIME_FOR_PACKET);//Time4Packet); // count 220 = 3520 mks
                        }
                    }
                }
                else // RX interrupt in a moment of TX operation
                {
                }
                //DataB0.Tmr3Inturrupt = 0;// switch off interrupt from timer 3 if it was fired during processing
            }
#endif
            if (BTStatus & 0x20) // TX interrupt
            {
                if (BTType & 0x02) // TX mode
                {
                    // transmit timing with setup:
                    // <setup = 401.mks> <upload = 960/480 mks> <transmit = 442mks><IRQ 6mks> <dealy XXX>
                    // transmit timeing without setup:
                    // <upload = 960/480 mks> <transmit = 442mks> <IRQ 6mks> <dealy XXX>
                
                    if (TXSendOverFQ) // needs to send copy over FQ2 or FQ3
                    {
                        // TX initiated by TMR1 interrupt and Transmit function just prepear everything
                        // but flag usefull for initial calculation of time btw FQ1-FQ@
                        // SKIP_CALC_TX_TIME can use precalculated values 
                        // commenting SKIP_CALC_TX_TIME will be the best way to 
                        if (DataB0.Timer1Done3FQ)
                            goto NEXT_TRANSMIT;
                        // difference btw receive process (442mks + 6 + + 102/51 + 2653/1327 = 3207/1826 mks) and transmit upload (<upload = 960/480 mks> <transmit = 442mks> <IRQ 6mks>+xxx) xxx= 2247(1799??)/898 mks = 141(112??)/56 counts on 32MHz with 128 prescaler
                        //SetTimer0(0xff73); // delay to accomodate tranmsmit and receive process difference send waits when recive will be done
                        //SetTimer0(0xff73); // <= failure
                        //SetTimer0(0xff53);
                        //SetTimer0(0xff33);
                        //SetTimer0(0xff13);
                        //SetTimer0(0xfef0);
                        //SetTimer0(0xff93); // <== OK
                        //SetTimer0(0xff93);
                        //SetTimer0(0xffa3);
                        //SetTimer0(0xffc3);
                        //SetTimer0(0xffd3); // <== OK
                        if (TXSendOverFQ == 1)
                            SetTimer0(DELAY_BTW_SEND_PACKET);
                        else
                            goto NEXT_TRANSMIT;
                        //putch('x');
                    }
                    else // finished with FQ3 now needs go back to RX mode
                    {
                    
                        SwitchToRXdata();
                        //putch('=');
                        if (ATCMD & MODE_CONNECT) // connection was established == earth get responce from luna
                        {
SET_TO_BTW_CHARS:
                             if (ATCMD & SOME_DATA_OUT_BUFFER) // data was collected to transmit
                             {
                                 if (BTqueueOutLen >= BT_TX_MAX_LEN) // buffer full 
                                     BTFlags.BTNeedsTX = 1;
                                 else 
                                    SetTimer0(TO_BTW_CHARS);//0xff00); // set timeout btw char
                             }
                        }
                        else
                            SetTimer0(DELAY_BTW_NEXT_DIAL); // 0x0000 == 4 sec till next attempt for earth to dial luna
                    }
                }
                else // TX interrupt in a moment of RX operation
                {
                }
            }
        }
#if _OLD_VERSION
        else  if (I2C.Timer0Fired) // no interrup yet == can be a timeout
        {
            I2C.Timer0Fired = 0;
            if (BTType & 0x01) // RX
            {
PROCESS_TO:
                if (RXreceiveFQ == 0) 
                {
                    if (ATCMD & MODE_CONNECT) // connection was established possible timer is btw char
                    {
                        //goto SET_TO_BTW_CHARS;
                        if (ATCMD & SOME_DATA_OUT_BUFFER)
                            BTFlags.BTNeedsTX = 1;
                    }
                    else // no connection yet == this is RX after transmit dial packet
                        ATCMD &= (0xff ^MODE_DIAL); // this will repeat dial cubsat attempt
                }
                else // case when time btw FQ1 -> FQ2 did not catched == stay with timer over TMR0
                {
                    if (CheckSuddenRX())
                        goto MAIN_INT;

                    SwitchFQ(DoFqRXSwitch());// if it was RX over FQ2 than value RXreceiveFQ ==1
                                             // if it was RX over FQ3 than value RXreceiveFQ ==2
                    BTCE_high();
                    if (RXreceiveFQ == 1) // timeout over FQ2
                    {
                        RXreceiveFQ = 2;
                        //BTqueueInLen2 = 0;
                        // 442mks passed no packet on FQ2 then next packet can be during <upload = 960/480 mks> +<transmit = 442mks> = 1406/922 mks = 22 couts
                        SetTimer0(0xffad);//SetTimer0(0xff58); //442mks+xxx=2247(1799??)/898mks == 2693/1340 = count 169/83 
                        //putch('t');
                    }
                    else if (RXreceiveFQ == 2) // timeout over FQ3
                    {
                        // done with timeouts == may be packet possible to fix ? - it can be packet over FQ1 or two packets over FQ1 + FQ2
TO_ON_FQ3:
                        RXreceiveFQ = 0;
                        //BTqueueInLen3 = 0;
                        //putch('T');

                        if (BTokMsg == 0xff) // needs to fix packet FQ1 or FQ1+FQ2
                        {
                            // TBD fixes messages 
#ifdef EARTH_PROCESS_BAD_MESSAGES
                            // TBD fixes received messages on earth over second COM port to be compiled on PIC24
#endif
                            if (BTokMsg == 0xff) // message can not be fixed
                            {
                            
                            }
                        }
                        if ((ATCMD & MODE_CALL_LUNA_COM)) // earth calls cubsat
                        {
                            if (ATCMD & MODE_CONNECT) // was connection esatblished
                            {
                            }
                            else // no connection yet
                            {
                                SetTimer0(DELAY_BTW_NEXT_DIAL); // 0x0000 == 4 sec till next attempt for earth to dial luna 
                            }
                        }
                    }
                }
            }
            else if (BTType == 2) // timeout on TX in a progress == was send FQ1 or FQ2 packets
            {
                if (TXSendOverFQ) // FQ2 or FQ3
                {
NEXT_TRANSMIT:
                    TransmitBTdata();
                }
            }
        }
#else
        else  if (I2C.Timer0Fired) // no interrup yet == can be a timeout
        {
            I2C.Timer0Fired = 0;
            if (Timer1Id == DELAY_BTW_NEXT_DIAL)
                ATCMD &= (0xff ^MODE_DIAL); // this will repeat dial cubsat attempt
            else if (Timer1Id == TO_BTW_CHARS)
                BTFlags.BTNeedsTX = 1;
            else if (TIME_FOR_PACKET0)
                goto LABEL_TO_RX;
            else if (Timer1Id == TIME_FOR_PACKET)
            {
LABEL_TO_RX:
                if (CheckSuddenRX())
                    goto MAIN_INT;

                SwitchFQ(DoFqRXSwitch());// if it was RX over FQ2 than value RXreceiveFQ ==1
                BTCE_high();
                if (RXreceiveFQ == 0) 
                {
                    RXreceiveFQ = 1;
                    SetTimer0(TIME_FOR_PACKET);
                }
                if (RXreceiveFQ == 1) // timeout over FQ2
                {
                    RXreceiveFQ = 2;
                    SetTimer0(TIME_FOR_PACKET); 
                }
                else if (RXreceiveFQ == 2) // timeout over FQ3
                {
                    // done with timeouts == may be packet possible to fix ? - it can be packet over FQ1 or two packets over FQ1 + FQ2
TO_ON_FQ3:
                    RXreceiveFQ = 0;
                    if ((ATCMD & MODE_CALL_LUNA_COM)) // earth calls cubsat
                    {
                        if (ATCMD & MODE_CONNECT) // was connection esatblished
                        {
                        }
                        else // no connection yet
                        {
                            SetTimer0(DELAY_BTW_NEXT_DIAL); // 0x0000 == 4 sec till next attempt for earth to dial luna 
                        }
                    }
                    else
                        SetTimer0(TIME_FOR_PACKET0);
                }
            }
            else if (Timer1Id == DELAY_BTW_SEND_PACKET) // timeout on TX in a progress == was send FQ1 or FQ2 packets
            {
NEXT_TRANSMIT:
                TransmitBTdata();
            }
        }
#endif
#if _OLD_VERSION
        else if (DataB0.Tmr3Inturrupt) // time to switch frequency on RX operation
        {

            DataB0.Tmr3Inturrupt = 0;
            if (BTType == 1) // RX mode
            {
                if (CheckSuddenRX())
                    goto MAIN_INT;
                    
                SwitchFQ(FqRX); // if it was RX over FQ1 than value RXreceiveFQ ==0
                BTCE_high();
                if (DataB0.Timer3OutSyncRQ)
                {
                     // switching from round robin to plain FQ1
                     if (FqRXCount ==0)
                     {
                         DataB0.Timer3OutSyncRQ = 0;
                         DataB0.Tmr3RxFqSwitchLost = 1;
                         DataB0.Timer3SwitchRX = 0;
                     } 
                }

                if (RXreceiveFQ) // receiving over FQ2 or FQ3 and timeout on that FQ
                {
                    if (++RXreceiveFQ >=3) // FQ3 == timeout
                    {
                        goto TO_ON_FQ3;
                    }
                    
                }
                // case when it is listenning over FQ1 == needs to switch FQ1 over FQ2 to continue listening
                else
                {
#ifdef SHOW_RX_TX
   #ifdef SHOW_RX
                    if (FqRXCount ==0)
                       DEBUG_LED_ON;
                    else
                       DEBUG_LED_OFF;
   #endif
#endif
                }
            }
        }
#else   ////////////////////////////new version////////////////
        else if (DataB0.Tmr3Inturrupt) // time to switch frequency on RX operation
        {
            DataB0.Tmr3Inturrupt = 0;
            if (BTType == 1) // RX mode
            {
                if (CheckSuddenRX())
                    goto MAIN_INT;
                    
                SwitchFQ(FqRX); // if it was RX over FQ1 than value RXreceiveFQ ==0
                BTCE_high();
                if (DataB0.Timer3OutSyncRQ)
                {
                     DataB0.Tmr3DoneMeasureFq1Fq2 = 0;
                     // switching from round robin to plain FQ1
                     //if (FqRXCount ==0)
                     //{
                     //    DataB0.Timer3OutSyncRQ = 0;
                     //    DataB0.Tmr3RxFqSwitchLost = 1;
                     //    DataB0.Timer3SwitchRX = 0;
                     //} 
                }

                if (RXreceiveFQ) // receiving over FQ2 or FQ3 and timeout on that FQ
                {
                    if (++RXreceiveFQ >=3) // FQ3 == timeout
                    {
                        goto TO_ON_FQ3;
                    }
                    
                }
                // case when it is listenning over FQ1 == needs to switch FQ1 over FQ2 to continue listening
                else
                {
//#ifdef SHOW_RX_TX
//   #ifdef SHOW_RX
//                    if (FqRXCount ==0)
//                       ;//DEBUG_LED_ON;
//                    else
//                       DEBUG_LED_OFF;
//   #endif
//#endif
                }
            }
        }
#endif
        // check does it needs to transmit something?
        if (ATCMD & SOME_DATA_OUT_BUFFER)
        {
            if (BTFlags.BTNeedsTX) // needs to transmit buffer ether because timeout btw char or because buffer is full or data ready to trabnsmit
            {
                if (RXreceiveFQ == 0) // only if it is listening on FQ1
                {
                    if (BTType == 1) // RX/listen in progress
                    {
                        if (bittest(PORT_BT,Tx_CE)) // is some non processing interrupt in progress if not==
                        { 
                            BTCE_low();  // Chip Enable Activates RX/TX (now disable) == standby
                            if (CheckSuddenRX())
                                goto MAIN_INT;
                            else // ok possible to transmit
                            {
                                TransmitBTdata();
                                ATCMD &= (0xff ^SOME_DATA_OUT_BUFFER);
                                BTFlags.BTNeedsTX = 0;
                            }
                        }
                    }
                } 
            }
        }
        
        // done interrupt processing now: call/transmit
        if (ATCMD & MODE_CALL_LUNA_COM) // earth calls cubsat sequence:
        {
            // earth call cubesat that is a main mode == earth initiate transmission
            // 1. send msg on FQ1
            // 2. send msq on FQ2
            // 3. send msg on FQ3 and switched to RX mode
            // 4. listen on FQ1 with main timeout = (distance x 2)/C
            // 5. on timeout go back to point. 1
            // 6. on msg receive over FQ1 -> check CRC == OK then message marked as OK -> switch to FQ2
            //                                     CRC != OK then switch to FQ2
            // 7. wait ether for msg or timeout 
            // 8. msg recived/ timeout -> check CRC == OK -> message marked as OK -> switch to FQ3
            //                                  CRC != OK -> switch to FQ3
            //                                    timeout -> switch to FQ3
            // 9. wait for msg or timeout
            // 10. msg received/ timeout -> check CRC == OK -> message marked as OK -> switch to FQ1
            //                                    CRC != OK -> switch to FQ1 
            //                                      timeout -> switch to FQ1
            //                                        a) one of messages marked as OK -> process message
            //                                        b) attempt to fix message based on 3 wrong messages
            //                                        c) attempt to fix message based on 2 wrong message
            //                                        d) attempt to fix message based on 1 message
            if (ATCMD & MODE_CONNECT) // connection was established == earth get responce from luna
            {
#ifdef DEBUG_LED_CALL_LUNA
                if (Main.PingRQ)
                {
                     if (BTType & 0x01)// on rx only
                     if (RXreceiveFQ == 0) // only if it is listening on FQ1
                     {
                         BTqueueOut[0] = 'P'; BTqueueOut[1] = 'I';BTqueueOut[2] = 'N';BTqueueOut[3] = 'G';
                         Main.PingRQ = 0;
                         goto SEND_PKT_TX_MODE;
                     }
                }
#endif

            }
            else // no connection yet earth == wait for responce from luna
            {
                if (ATCMD & MODE_DIAL) // is it dialed or not
                {
                }
                else // was not dialed yet
                {
                    BTqueueOut[0] = 'l'; BTqueueOut[1] = 'u';BTqueueOut[2] = 'n';BTqueueOut[3] = 'a';

                    ATCMD |= MODE_DIAL;
SEND_PKT_TX_MODE:
                    SetupBT(SETUP_TX_MODE);
SEND_PKT_DIAL:
                    BTqueueOut[4] = Addr1;BTqueueOut[5] = Addr2;BTqueueOut[6] = Addr3;
                    // this is place where frequency for satellite can be adjusted because of temperature drift in satellite
                    // ground station monitors frequencies and found shift then send channels numbers
                    BTqueueOut[7] = Freq1;BTqueueOut[8] = Freq2;BTqueueOut[9] = Freq3;
                    BTpkt = PCKT_DIAL;
                    BTqueueOutLen = 10;
                    BTFlags.BTNeedsTX = 1;
                    ATCMD |= SOME_DATA_OUT_BUFFER;
                } 
            }
        }
        else if (ATCMD & MODE_CALL_EARTH) // cubsat calls earth
        {
            // cubesat starts to wait for a connection on command:
            //   atdtearth<cr>
            // 1. cubesat listen indefinetly to incoming message on FQ1
            // 2. message received -> check CRC == OK then message marked as OK -> switch to FQ2
            //                              CRC != OK then switch to FQ2
            // 3. wait ether for msg or timeout 
            // 4. msg recived/ timeout -> check CRC == OK -> message marked as OK -> switch to FQ3
            //                                      CRC != OK -> switch to FQ3
            //                                        timeout -> switch to FQ3
            // 5. wait for msg or timeout
            // 6. msg received/ timeout -> check CRC == OK -> message marked as OK -> switch to FQ1
            //                                      CRC != OK -> switch to FQ1 
            //                                        timeout -> switch to FQ1
            //                                        a) one of messages marked as OK -> process message
            //                                        b) attempt to fix message based on 3 wrong messages
            //                                        c) attempt to fix message based on 2 wrong message
            //                                        d) attempt to fix message based on 1 message
            // 7. on any msg OK or on fixed msg cubesat switches to MODE_CONNECT
            // 8. send responce to earth on FQ1 : difference in a body of message "earth" is a prime text in BTqueueOut
            // 9. send responce to earth on FQ2
            // 10. send responce to earth on FQ3
            // 11. responce send after Fq1+FQ2+FQ3 flag: RESPONCE_WAS_SENT 
            // 12. in MODE_CONNECT and RESPONCE_WAS_SENT received from EARTH tunnelings to a caller unit or to a comunication loop
            //     at the same time messages from any units tunneling to a communication 
            if (ATCMD & MODE_CONNECT) // connection was esatblished == luna get dial message from earth
            {
                if (ATCMD & RESPONCE_WAS_SENT) // responce to earth was send
                {
#ifdef DEBUG_LED_CALL_EARTH
                    if (Main.PingRQ)
                    {
                         //if (BTqueueOutLen == 0)
                         {
                             BTqueueOut[0] = 'p'; BTqueueOut[1] = 'i';BTqueueOut[2] = 'n';BTqueueOut[3] = 'g';
                             Main.PingRQ = 0;
                             goto SEND_PKT_TX_MODE;
                         }
                    }
#endif

                }
                else                           // responce to earth was not send from luna
                {
                    if (TXSendOverFQ==0)
                    {
                        BTqueueOut[0] = 'e';BTqueueOut[1] = 'a';BTqueueOut[2] = 'r';BTqueueOut[3] = 'z';
                        ATCMD |= RESPONCE_WAS_SENT;
                        goto SEND_PKT_DIAL;
                    }
                }
            }
            else // no connection yet == luna waits for a communication
            {
                if (ATCMD & INIT_BT_NOT_DONE)
                { 
                    SetupBT(SETUP_RX_MODE);
                    SetTimer0(TIME_FOR_PACKET0);
                    ATCMD &= (INIT_BT_NOT_DONE^0xff);
                }
            }
        }
    }
    return 1;
}


void Reset_device(void)
{
    
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef __PIC24H__
    // Configure Oscillator to operate the device at 40Mhz
    // Fosc= Fin*M/(N1*N2), Fcy=Fosc/2
    // Fosc= 8M*40/(2*2)=80Mhz for 8M input clock
    // or for 7.37MHz Fosc = 7.37*44/(2*2) = 81.07 Fcy=40.535
    // M=44 (PLL=42) == Fcy 40.535
    // M=22 (PLL=20) == Fcy 20.2675
    // total:
    // PLL=20=> M=22;    PLLPOST=0 => N2=2;   PLLPRE=2 => N1=4; Fosc=7.37*22/(2*4)=20.2675 MHz => Fcy=10.13375 MHz
    
    PLLFBD=42; // Fcy = 20.2675; // 42// M=44
           // for value = 50 it can be 92.125MHz and 46MIPS needs to make shure that FIN will be more then 4MHz and less 8MHz
           // OCTUN set more then 8MHz can be wrong
	CLKDIVbits.PLLPOST=0;		// N2=2 PLL VCO Output Divider Select bits (also denoted as �N2�, PLL postscaler)
                                // 00 = Output/2
                                // 01 = Output divided by 4 (default)
                                // 10 = Reserved
                                // 11 = Output divided by 8
                                // bit 5 Unimplemented: Read as
	CLKDIVbits.PLLPRE=0;		// N1=2 PLL Phase Detector Input Divider bits (also denoted as �N1�, PLL prescaler)
                                // 11111 = Input divided by 33
                                // 
                                // 00001 = Input divided by 3
                                // 00000 = Input divided by 2 (default)
	OSCTUN=0;//0x14;                // Tune FRC oscillator, if FRC is used
                                // bit 5-0 TUN<5:0>: FRC Oscillator Tuning bits(1)
                                // 111111 = Center frequency -0.375% (7.345 MHz)
                                // 100001 = Center frequency -11.625% (6.52 MHz)
                                // 100000 = Center frequency -12% (6.49 MHz)
                                // 011111 = Center frequency +11.625% (8.23 MHz)
                                // 011110 = Center frequency +11.25% (8.20 MHz)
                                // 000001 = Center frequency +0.375% (7.40 MHz)
                                // 000000 = Center frequency (7.37 MHz nominal)
                                // 010100 = 8MHz

	RCONbits.SWDTEN=0;          // Disable Watch Dog Timer ???

    // Clock switch to incorporate PLL
	__builtin_write_OSCCONH(0x01);		// Initiate Clock Switch to Primary
                                        // Oscillator with PLL (NOSC=0b011) 
	__builtin_write_OSCCONL(0x01);		// Start clock switching
                                        // bit 7 CLKLOCK: Clock Lock Enable bit 0 = Clock switching is enabled, system clock source can be modified by clock switching
                                        // bit 6 IOLOCK: Peripheral Pin Select Lock bit 0 = Peripherial pin select is not locked, write to peripheral pin select registers allowed
                                        // bit 5 LOCK: PLL Lock Status bit (read-only)
                                        // bit 4 Unimplemented: Read as �0�
                                        // bit 3 CF: Clock Fail Detect bit (read/clear by application)
                                        // bit 2 Unimplemented: Read as �0�
                                        // bit 1 LPOSCEN: Secondary (LP) Oscillator Enable bit 0 = Disable secondary oscillator
                                        // bit 0 OSWEN: Oscillator Switch Enable bit 1 = Request oscillator switch to selection specified by NOSC<2:0> bits
	while (OSCCONbits.COSC != 0b001);	// Wait for Clock switch to occur check for:
                                        // 011 = Primary oscillator (XT, HS, EC) with PLL

	while(OSCCONbits.LOCK!=1) {};       //  Wait for PLL to lock
                                        // bit 5 LOCK: PLL Lock Status bit (read-only)
                                        //    1 = Indicates that PLL is in lock, or PLL start-up timer is satisfied
                                        //    0 = Indicates that PLL is out of lock, start-up timer is in progress or PLL is disabled

    // CLKDIV:
    //     bit 15 ROI: Recover on Interrupt bit
    //         1 = Interrupts clears the DOZEN bit and the processor clock/peripheral clock ratio is set to 1:1
    //         0 = Interrupts have no effect on the DOZEN bit
    //     bit 14-12 DOZE<2:0>: Processor Clock Reduction Select bits
    //       111 = FCY/128
    //       110 = FCY/64
    //       101 = FCY/32
    //       100 = FCY/16
    //       011 = FCY/8 (default)
    //       010 = FCY/4
    //       001 = FCY/2
    //       000 = FCY/1
    //    bit 11 DOZEN: DOZE Mode Enable bit(1)
    //         1 = The DOZE<2:0> bits specify the ratio between the peripheral clocks and the processor clocks
    //         0 = Processor clock/peripheral clock ratio forced to 1:1
    CLKDIVbits.DOZE = 0b100 ; // speed of a processor is 40MOP => 2.5 MOP for 10MOPS = 0.625 MOPS
    CLKDIVbits.ROI = 1;

    // disable analog
	AD1CON1bits.ADON = 0; 
    // and switch analog pins to digital
    AD1PCFGL = 0xffff;
    //AD1PCFGH = 0xffff;
    // porta is not re-mappable and on 502 device it is RA0-RA4
    // SPI output in FLASH mem terminoligy:
    // SSCLOCK RA0(pin2), SSDATA_IN RA1(pin3), SSDATA_OUT RA2(pin9), SSCS RA3(pin10)
    //          0            0                        IN                  1
    TRISA = 0b00000100;  //0 = Output, 1 = Input 
    PORTA = 0b00001000;
    // this kaind funny, and for PIC24 and up = PRX pins can be reassigned to differrent preferias
    // additionaly needs to remember on which pin sits which PRX : // VSIAK SVERCHOK ZNAI SVOI SHESTOK
    __builtin_write_OSCCONL(OSCCON & 0xbf); // unlock port remapping
    // INT0 == pin 16 
    // INT1 == pin 21 == PR10 
    IN_FN_PPS_INT1 = IN_PIN_PPS_RP10; // RPINR0 
    // INT2 == pin 22 == PR11
    IN_FN_PPS_INT2 = IN_PIN_PPS_RP11;
    
    // PR5 - Serial RX  Pin 14
    IN_FN_PPS_U1RX = IN_PIN_PPS_RP5;
	// RR6 - Serial TX  Pin 15
    OUT_PIN_PPS_RP6 = OUT_FN_PPS_U1TX;
    // I2C:
    // SCL1 = I2C clock Pin 17 (this is NOT alernative I2c set as FPOR = 1 in configuration) 
    // SDA1 = I2C data Pin 18  this two pins permamet
    __builtin_write_OSCCONL(OSCCON | 0x40); //lock back port remapping
     
    //RBPU_ = 0;
    //bitclr(OPTION,RBPU_); //Each of the PORTB pins has a weak internal pull-up. A
                          //single control bit can turn on all the pull-ups. This is
                          //performed by clearing bit RBPU (OPTION_REG<7>).
                          //The weak pull-up is automatically turned off when the
                          //port pin is configured as an output. The pull-ups are
                          //disabled on a Power-on Reset.

    INT0_ENBL = 0; // disable external interrupt for GYRO 1
    INT1IE = 0;    // disable external interrupt for GYRO2
    enable_uart(); //Setup the hardware UART for 20MHz at 9600bps
    // next two bits has to be set after all intialization done
    //PEIE = 1;    // bit 6 PEIE: Peripheral Interrupt Enable bit
                 // 1 = Enables all unmasked peripheral interrupts
                 // 0 = Disables all peripheral interrupts
    //GIE = 1;     // bit 7 GIE: Global Interrupt Enable bit
                 // 1 = Enables all unmasked interrupts
                 // 0 = Disables all interrupts
#ifndef NO_I2C_PROC
    enable_I2C();
#endif
    TIMER0_INT_FLG = 0; // clean timer0 interrupt
    TIMER0_INT_ENBL = 0; // diasable timer0 interrupt
    TMR1IF = 0; // clean timer0 interrupt
    TMR1IE = 0; // diasable timer0 interrupt
    INT0_EDG = 1; // 1 = Interrupt on negative edge
    INT0_FLG = 0; // clean extrnal interrupt RB0 pin 6

    INT1IF = 0;
    INTEDG1 = 1;    

    INT2IF = 0;
    INTEDG2 = 1;    
     RtccInitClock();       //turn on clock source
     RtccWrOn();            //enable RTCC peripheral
     //RtccWriteTimeDate(&RtccTimeDate,TRUE);
     RtccReadTimeDate(&RtccTimeDateVal);
     mRtccOn();

#else  // ends PIC24 
////////////////////////////////////////////////////////////////////////////////////////////////////
//  for both diveses individual differences will bi inside ifdef
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _18F2321_18F25K20
    unsigned int iCount;
    unsigned int iCount2;
// this is will be better at the begining of a program
    // 18F2321
    //OSCCON = 0b01110000; //OSCILLATOR CONTROL REGISTER (ADDRESS 8Fh)
                         // bit 7 IDLEN: Idle Enable bit
                         //    1 = Device enters an Idle mode when a SLEEP instruction is executed
                         //    0 = Device enters Sleep mode when a SLEEP instruction is executed
                         // bit 6-4 IRCF<2:0>: Internal Oscillator Frequency Select bits
                         //    111 = 8 MHz (INTOSC drives clock directly)/16 MHz (HFINTOSC drives clock directly)
                         //    110 = 4 MHz / 8 MHz
                         //    101 = 2 MHz / 4 MHz
                         //    100 = 1 MHz(3) / 2 MHz
                         //    011 = 500 kHz / 1 MHz(3)
                         //    010 = 250 kHz / 500 kHz
                         //    001 = 125 kHz / 250 kHz
                         //    000 = 31 kHz (from either INTOSC/256 or INTRC directly)(2)
                         // bit 3 OSTS: Oscillator Start-up Time-out Status bit(1)
                         //    1 = Oscillator Start-up Timer (OST) time-out has expired; primary oscillator is running/
                         //        / Device is running from the clock defined by FOSC<2:0> of the CONFIG1 register
                         //    0 = Oscillator Start-up Timer (OST) time-out is running; primary oscillator is not ready
                         //        / Device is running from the internal oscillator (HFINTOSC or LFINTOSC)
                         // bit 2 IOFS: INTOSC Frequency Stable bit
                         //    1 = INTOSC frequency is stable
                         //    0 = INTOSC frequency is not stable
                         // bit 1-0 SCS<1:0>: System Clock Select bits
                         //    1x = Internal oscillator block
                         //    01 = Secondary (Timer1) oscillator
                         //    00 = Primary oscillato

   // 18F25K20
   OSCCON = 0b11110000; //OSCILLATOR CONTROL REGISTER (ADDRESS 8Fh)
                         //bit 7 IDLEN: Idle Enable bit
                         //   1         = Device enters Idle mode on SLEEP instruction
                         //   0         = Device enters Sleep mode on SLEEP instruction
                         //bit 6-4 IRCF<2:0>: Internal Oscillator Frequency Select bits
                         //    111      = 16 MHz (HFINTOSC drives clock directly)
                         //    110      = 8 MHz 
                         //    101      = 4 MHz 
                         //    100      = 2 MHz
                         //    011      = 1 MHz(3) - default after reset
                         //    010      = 500 kHz
                         //    001      = 250 kHz
                         //    000      = 31 kHz (from either HFINTOSC/512 or LFINTOSC directly)(2)
                         //bit 3 OSTS: Oscillator Start-up Time-out Status bit(1)
                         //       1     = Device is running from the clock defined by FOSC<2:0> of the CONFIG1 register
                         //       0     = Device is running from the internal oscillator (HFINTOSC or LFINTOSC)
                         //bit 2 IOFS: HFINTOSC Frequency Stable bit 
                         //        1    = HFINTOSC frequency is stable
                         //        0    = HFINTOSC frequency is not stable
                         //bit 1-0 SCS<1:0>: System Clock Select bits
                         //         1x  = Internal oscillator block
                         //         01  = Secondary (Timer1) oscillator
                         //         00  = Primary clock (determined by CONFIG1H[FOSC<3:0>]).

// OSCTUNE: OSCILLATOR TUNING REGISTER
// PLLEN: Frequency Multiplier PLL for HFINTOSC Enable bit(1)
//     1 = PLL enabled for HFINTOSC (8 MHz and 16 MHz only)
//     0 = PLL disabled
// that will multiply 16MHz * 4 = 64MHz

    PLLEN = 1;
#ifdef _NOT_SIMULATOR
    // for external HS 16MHZ x4 ==64MHz no need to check bit HFINTOSC frequency is stable
    //while((OSCCON&0b0000100) == 0); //Wait for frequency to stabilize
#endif

    // delay ??? for what reason ???

    if (PLLEN)
    {
        iCount2 = 0x0004;
        do
        {
            iCount = 0xffff;
            do
            {
            }
            while(--iCount);
        }
        while(--iCount2);
    }

	ADCON0 = 0b00000000;
    ADCON1 = 0b00001111;

//////////////////////////////////////////////
// Browm*                  MCLR/VPP/RE3 | 1     28| RB7/KBI3/PGD         *white
// Tx_CE                        RA0/AN0 | 2     27| RB6//KBI2/PGC        *gray
// Tx_CSN                       RA1/AN1 | 3     26| RB5/KBI1/PGM         *violet
// Tx_SCK           RA2/AN2/VREF-/CVREF | 4     25| RB4/KBI0/AN11        ---> Low == Serial RX_FULL (set High on input Com queue full)
// Tx_MOSI                RA3/AN3/VREF+ | 5     24| RB3/AN9/CCP2         <--- Low == TX_NOT_READY Next serial unit not ready to get data  
// Rx_MISO              RA4/T0CKI/C1OUT | 6     23| RB2/INT2/AN8          BT_RX
// dbg blnkLED  RA5/AN4/SS/HLVDIN/C2OUT | 7     22| RB1/INT1/AN10         BT_TX
//                                  VSS | 8     21| RB0/INT0/FLT0/AN12    Rx_IRQ
//     crystal            OSC1/CLKI/RA7 | 9     20| VDD
//     crystal            OSC2/CLKO/RA6 |10     19| VSS
// SSDATA_OUT          RC0/T1OSO/T13CKI |11     18| RC7/RX/DT        <--- Serial RX
// SSDATA_OUT2           RC1/T1OSI/CCP2 |12     17| RC6/TX/CK        ---> Serial TX
// SSDATA_OUT3                 RC2/CCP1 |13     16| RC5/SDO              SSCS
// SSDATA_IN                RC3/SCK/SCL |14     15| RC4/SDI/SDA          SSCLOCK      

// /CS  |1   8|  VCC       CS#  |1   8|  VCC       ^S   |1   8|  VCC
// DO   |2   7|  /HOLD     SO   |2   7|  HOLD#     Q    |2   7|  ~HOLD
// /WP  |3   6|  CLK       WP#  |3   6|  CLK       ~W   |3   6|  C
// GND  |4   5|  DI        GND  |4   5|  DI        VSS  |4   5|  D
///////////////////////// 
    //BT pin assignment

    // #define PORT_BT PORTAbits
    // #define Tx_CE      RA0	// RA0 pin 2 // Chip Enable Activates RX or TX mode -> Out
    // #define Tx_CSN     RA1	// RA1 pin 3 // SPI Chip Select ->Out
    // #define Tx_SCK     RA2    // RA2 pin 4  // SPI Clock -> Out
    // #define Tx_MOSI    RA3	// RA3 pin 5  // SPI Slave Data Input -> Out
    // #define Rx_MISO    RA4	// RA4 pin 6  // SPI Slave Data Output, with tri-state option -> In
    // #define Rx_IRQ     RB0    // RB0 pin 21 // Maskable interrupt pin. Active low -> in
    // #define BT_RX      RB2   // RB2 pin 23 BT in receive mode


    // RA6 & RA7 == IN Crystal osc
    // debug LED RA5 pin 7
    TRISA = 0b11010000;  //0 = Output, 1 = Input 
    PORTA = 0b00101110; // Q, Q, SSCS , Rx_MISO, Tx_MOSI, Tx_SCK , Tx_CSN , low=Tx_CE
    // RB0 - external INT Pin 21
    // RB4 - RX_FULL     pin 25 - out
    // RB3 - TX_NOT_READY pin 24 - in

    TRISB = 0b00001001;  //0 = Output, 1 = Input 
    PORTB = 0b00000000;  // nothing happened with amplifiers BT_TX,BT_RX=low

	
    
    // SPI output in FLASH mem terminoligy:
    
    //          0            0                        IN                  1
    // SSDATA_OUT  =  IN RC0 pin 11
    // SSDATA_OUT2 =  IN RC1 pin 12
    // SSDATA_OUT3 =  IN RC2 pin 13
    // SSDATA_IN   = out RC3 pin 14
    // SSCLOCK     = out RC4 pin 15
    // SSCS        = out RC5 pin 16
    // RC6 - Serial TX Pin 17
    // RC7 - Serial RX  Pin 18


    TRISC = 0b10000111;  //0 = Output, 1 = Input 
    PORTC = 0b01100000;
     
    //RBPU_ = 0;
    //bitclr(OPTION,RBPU_); //Each of the PORTB pins has a weak internal pull-up. A
                          //single control bit can turn on all the pull-ups. This is
                          //performed by clearing bit RBPU (OPTION_REG<7>).
                          //The weak pull-up is automatically turned off when the
                          //port pin is configured as an output. The pull-ups are
                          //disabled on a Power-on Reset.
                          

    // RE3 (pin1) MCLR == input 
    //TRISE = 0x00001000;

    PORTE = 0b11111111; 
    INT0_ENBL = 0; // disable external interrupt for GYRO 1
    INTEDG0 = 0;  // interrupt on 1->0 transition
    INT1IE = 0;    // disbalke external interrupt for GYRO2
    //TMR0ON = 0;
     // 0         bit 7 TMR0ON: Timer0 On/Off Control bit
     //               1 = Enables Timer0
     //               0 = Stops Timer0
     //  0        bit 6 T08BIT: Timer0 8-Bit/16-Bit Control bit
     //               1 = Timer0 is configured as an 8-bit timer/counter
     //               0 = Timer0 is configured as a 16-bit timer/counter
     //   0       bit 5 T0CS: Timer0 Clock Source Select bit
     //               1 = Transition on T0CKI pin
     //               0 = Internal instruction cycle clock (CLKO)
     //    0      bit 4 T0SE: Timer0 Source Edge Select bit
     //               1 = Increment on high-to-low transition on T0CKI pin
     //               0 = Increment on low-to-high transition on T0CKI pin
     //     0     bit 3 PSA: Timer0 Prescaler Assignment bit
     //               1 = TImer0 prescaler is NOT assigned. Timer0 clock input bypasses prescaler.
     //               0 = Timer0 prescaler is assigned. Timer0 clock input comes from prescaler output.
     //      110  bit 2-0 T0PS<2:0>: Timer0 Prescaler Select bits
     //           111 = 1:256 Prescale value
     //           110 = 1:128 Prescale value
     //           101 = 1:64 Prescale value
     //           100 = 1:32 Prescale value
     //           011 = 1:16 Prescale value
     //           010 = 1:8 Prescale value
     //           001 = 1:4 Prescale value
     //           000 = 1:2 Prescale value

#ifdef _18F25K20
     T0CON = 0b10000111;
#else
     T0CON = 0b10000110;
#endif

    //TMR1ON = 0;
     // 1         bit 7 RD16: 16-Bit Read/Write Mode Enable bit
     //               1 = Enables register read/write of TImer1 in one 16-bit operation
     //               0 = Enables register read/write of Timer1 in two 8-bit operations
     //  0        bit 6 T1RUN: Timer1 System Clock Status bit
     //               1 = Device clock is derived from Timer1 oscillator
     //               0 = Device clock is derived from another source
     //   00      bit 5-4 T1CKPS<1:0>: Timer1 Input Clock Prescale Select bits
     //              11 = 1:8 Prescale value
     //              10 = 1:4 Prescale value
     //              01 = 1:2 Prescale value
     //              00 = 1:1 Prescale value
     //     0     bit 3 T1OSCEN: Timer1 Oscillator Enable bit
     //               1 = Timer1 oscillator is enabled
     //               0 = Timer1 oscillator is shut off
     //             The oscillator inverter and feedback resistor are turned off to eliminate power drain.
     //      0    bit 2 T1SYNC: Timer1 External Clock Input Synchronization Select bit
     //              When TMR1CS = 1:
     //                1 = Do not synchronize external clock input
     //                0 = Synchronize external clock input
     //              When TMR1CS = 0:
     //                This bit is ignored. Timer1 uses the internal clock when TMR1CS = 0.
     //       0   bit 1 TMR1CS: Timer1 Clock Source Select bit
     //               1 = External clock from pin RC0/T1OSO/T13CKI (on the rising edge)
     //               0 = Internal clock (FOSC/4)
     //        1  bit 0 TMR1ON: Timer1 On bit
     //               1 = Enables Timer1
     //               0 = Stops Timer1
     T1CON = 0b10000000;
    //TMR3ON = 0;
     // 1            bit 7 RD16: 16-Bit Read/Write Mode Enable bit
     //                1 = Enables register read/write of Timer3 in one 16-bit operation
     //                0 = Enables register read/write of Timer3 in two 8-bit operations
     //  0  0        bit 6,3 T3CCP<2:1>: Timer3 and Timer1 to CCPx Enable bits
     //                1x = Timer3 is the capture/compare clock source for the CCP modules
     //                01 = Timer3 is the capture/compare clock source for CCP2; Timer1 is the capture/compare
     //                     clock source for CCP1
     //                00 = Timer1 is the capture/compare clock source for the CCP modules
     //   00         bit 5-4 T3CKPS<1:0>: Timer3 Input Clock Prescale Select bits
     //                11 = 1:8 Prescale value
     //                10 = 1:4 Prescale value
     //                01 = 1:2 Prescale value
     //                00 = 1:1 Prescale value
     //      0       bit 2 T3SYNC: Timer3 External Clock Input Synchronization Control bit
     //                     (Not usable if the device clock comes from Timer1/Timer3.)
     //                 When TMR3CS = 1:
     //                   1 = Do not synchronize external clock input
     //                   0 = Synchronize external clock input
     //                 When TMR3CS = 0:  
     //                   This bit is ignored. Timer3 uses the internal clock when TMR3CS = 0.
     //       0      bit 1 TMR3CS: Timer3 Clock Source Select bit
     //                1 = External clock input from Timer1 oscillator or T13CKI (on the rising edge after the first
     //                    falling edge)
     //                0 = Internal clock (FOSC/4)
     //        1     bit 0 TMR3ON: Timer3 On bit
     //                1 = Enables Timer3
     //                0 = Stops Timer3
     T3CON = 0b10000000;
     

#else // done _18F2321_18F25K20 // begins 16f884
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _16F884

// this is will be better at the begining of a program
    OSCCON = 0b01110000; //OSCILLATOR CONTROL REGISTER (ADDRESS 8Fh)
                         // bit 6-4 IRCF<2:0>: Internal RC Oscillator Frequency Select bits
                         //         000 = 31.25 kHz
                         //         001 = 125 kHz
                         //         010 = 250 kHz
                         //         011 = 500 kHz
                         //         100 = 1 MHz
                         //         101 = 2 MHz
                         //         110 = 4 MHz
                         //         111 = 8 MHz
                         // bit 3 OSTS: Oscillator Start-up Time-out Status bit(1)
                         //   1 = Device is running from the primary system clock
 

                        //   0 = Device is running from T1OSC or INTRC as a secondary system clock
                         //    Note 1: Bit resets to �0� with Two-Speed Start-up mode and LP, XT or HS selected as the
                         //    oscillator mode.
                         // bit 2 IOFS: INTOSC Frequency Stable bit
                         //   1 = Frequency is stable
                         //   0 = Frequency is not stable
                         // bit 1-0 SCS<1:0>: Oscillator Mode Select bits
                         //   00 = Oscillator mode defined by FOSC<2:0>
                         //   01 = T1OSC is used for system clock
                         //   10 = Internal RC is used for system clock
                         //   11 = Reserved

#ifdef _NOT_SIMULATOR
    while((OSCCON&0b00000100) == 0); //Wait for frequency to stabilize
#endif

    ANSEL =   0b00000000; //Turn pins to Digital instead of Analog
    CM2CON0 = 0b00000111; //Turn off comparator on RA port
    CM1CON0 = 0b00000111;
    // for each unit it is individual
    // RA0,1,2,3,4 this will be stepper motor control 1A,2A,1B,2B,ENBL
    //TRISA = 0b10100000;  //0 = Output, 1 = Input 
    //PORTA = 0b00000000;

    // serial FLASH pin assignment 
    // SSCLOCK RA7(pin16), SSDATA_IN RA6(pin15), SSDATA_OUT RA4, SSCS RA3
    //          0                0                   IN             1
    TRISA = 0b00110000;  //0 = Output, 1 = Input 
    PORTA = 0b00001000;  // SSCS set high

    // RB0 - external INT Pin 6
    TRISB = 0b00000001;  //0 = Output, 1 = Input 
    PORTB = 0b11111110;  

	// RC6 - Serial Out  Pin 25
	// RC7 - Serial In Pin 26
    // I2C:
    // RC3 - SCL = I2C clock Pin 18
    // RC4 - SDA = I2C data Pin 23
    // RB0 - external INT Pin 33
    TRISC = 0b10011000;  //0 = Output, 1 = Input 
    PORTC = 0b01000000;  
    //RBPU_ = 0;
    //bitclr(OPTION,RBPU_); //Each of the PORTB pins has a weak internal pull-up. A
                          //single control bit can turn on all the pull-ups. This is
                          //performed by clearing bit RBPU (OPTION_REG<7>).
                          //The weak pull-up is automatically turned off when the
                          //port pin is configured as an output. The pull-ups are
                          //disabled on a Power-on Reset.
                
    TRISD = 0b11111111; //0 = Output, 1 = Input 
    PORTD = 0b00000000;        
  
    // RE7 (pin1) MCLR == input
    TRISE = 0b11111111; //0 = Output, 1 = Input 
    TRISE = 0b10000000;
    INT0_ENBL = 0; // disable external interrupt for GYRO 1

#else // done _16F884
    ANSEL = 0b00000000; //Turn pins to Digital instead of Analog
    CMCON = 0b00000111; //Turn off comparator on RA port
    // for each unit it is individual
    // RA0,1,2,3,4 this will be stepper motor control 1A,2A,1B,2B,ENBL
    //TRISA = 0b10100000;  //0 = Output, 1 = Input 
    //PORTA = 0b00000000;

    // RA5 MCLR == input
    // serial FLASH pin assignment 
    // SSCLOCK RA7(pin16), SSDATA_IN RA6(pin15), SSDATA_OUT RA4(pin3), SSCS RA3(pin2)
    //          0                0                   IN             1
    TRISA = 0b00110000;  //0 = Output, 1 = Input 
    PORTA = 0b00001000;  // SSCS set high


	// RB5 - Serial Out  Pin 11
	// RB2 - Serial In Pin 8
    // I2C:
    // RB4 - SCL = I2C clock Pin 10
    // RB1 - SDA = I2C data Pin 7
    // RB0 - external INT Pin 6
    
                         // RB2 - serial input
                         // RB5 - serial out 
    TRISB = 0b00010111;  //0 = Output, 1 = Input 
    PORTB = 0b11111111;  
    //RBPU_ = 0;
    //bitclr(OPTION,RBPU_); //Each of the PORTB pins has a weak internal pull-up. A
                          //single control bit can turn on all the pull-ups. This is
                          //performed by clearing bit RBPU (OPTION_REG<7>).
                          //The weak pull-up is automatically turned off when the
                          //port pin is configured as an output. The pull-ups are
                          //disabled on a Power-on Reset.
    INT0_ENBL = 0; // disable external interrupt for GYRO 1
                          
#endif  // ifdef _16F884
#endif // #ifdef _18F2321_18F25K20
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////
// common part:
    //RBIF = 0;
    //RBIE = 1;
    
    enable_uart(); //Setup the hardware UART for 20MHz at 9600bps
    // next two bits has to be set after all intialization done
    //PEIE = 1;    // bit 6 PEIE: Peripheral Interrupt Enable bit
                 // 1 = Enables all unmasked peripheral interrupts
                 // 0 = Disables all peripheral interrupts
    //GIE = 1;     // bit 7 GIE: Global Interrupt Enable bit
                 // 1 = Enables all unmasked interrupts
                 // 0 = Disables all interrupts
#ifndef NO_I2C_PROC
    enable_I2C();
#endif
    // timer0 prescaler
#ifdef _18F2321_18F25K20
    //T0CON = 6; //prescaler 1 tick = 16mks => 1ms = 63 tic 2ms = 125 value 0xff00 mean 4ms value 0xf424 = 1s
#ifdef _18F25K20
     T0CON = 0b00000111;
#else
     T0CON = 0b00000110;
#endif
#endif    
    TIMER0_INT_FLG = 0; // clean timer0 interrupt
    TIMER0_INT_ENBL = 0; // diasable timer0 interrupt
    TMR1IF = 0; // clean timer0 interrupt
    TMR1IE = 0; // diasable timer0 interrupt
    INT0_EDG = 0; // high -> low == interrupt
    INT0_FLG = 0; // clean extrnal interrupt RB0 pin 6
#ifdef _18F2321_18F25K20
    INT1IF = 0;
    INTEDG1 = 0;    
    
#endif

    //INT0IE = 1; // enable external interrupt
#endif
     //CsLow();
}


void ShowMessage(void)
{
    putch(MY_UNIT);  // this message will circle over com and will be supressed by unit
    Puts("~");
    putch(MY_UNIT);
}

#include "commc8.h"


void BTbyteCRC(unsigned char bByte)
{
    SendBTbyte(bByte);  
    wCRCupdt(bByte);
}
unsigned char CheckPacket(unsigned char*MyData, unsigned char iLen)
{
    unsigned char i;
    unsigned char iCrc = 26;
    CRCcmp=0;
    CRC=0xffff;
    FSR_REGISTER = MyData;
    for (i = 0; i < iCrc; i++)
    {
        wCRCupdt(PTR_FSR);
        if (i == PACKET_LEN_OFFSET)
        {
            if (PTR_FSR<= BT_TX_MAX_LEN)
                iCrc = PTR_FSR + PACKET_LEN_OFFSET+1;// + sizeof(PacketStart);
            else
                goto RETURN_ERROR;
        }

        FSR_REGISTER++;
    }
    CRCcmp = ((((UWORD)PTR_FSR))<<8); FSR_REGISTER++;
    CRCcmp += ((UWORD)PTR_FSR);
    if (CRC == CRCcmp)
    {
#if 0
            if (RXreceiveFQ == 0)
            {
               if (BTFlags.BTFixed)
                  putch('3');
               else
                  putch('0');
            }
            else if (RXreceiveFQ == 1)
               putch('1');
            else if (RXreceiveFQ == 2)
               putch('2');
#endif
        return 0;
    }
RETURN_ERROR:
    return 0xff;

}
unsigned char BTFixlen(unsigned char*MyData, unsigned char iLen)
{
   unsigned char i;
   unsigned char j;
   
   unsigned char bByte;
   unsigned char bByte1;
   unsigned char iShift = 0;
   //unsigned char *ptr = MyData;
   
   FSR_REGISTER = MyData;
   if (iLen)
   {
       i=0;
       do
       { 
           if (PTR_FSR == 0xaa) // preambule
           {
               FSR_REGISTER++;
               if ((PTR_FSR == 0xF0) || (PTR_FSR == 0xF1)) // packet dial or packet data
               {
                   i++;
                   goto FIND_NONPRT;
               }
           }
           else if (PTR_FSR == 0xab) // possible shift 2
           {
                FSR_REGISTER++;
                if ((PTR_FSR &0xf8) == 0xc0)
                {
                    iShift = 2;
                    goto FIND_NONPRT;
                }
           }
           else if (PTR_FSR == 0xaf) // possible shift 4
           {
                FSR_REGISTER++;
                if ((PTR_FSR &0xe0) == 0)
                {
                    iShift = 4;
                    goto FIND_NONPRT;
                }
           }
           else if (INDF0 == 0xbc) // possible shift 2
           {
                FSR_REGISTER++;
                if ((PTR_FSR & 0x80) == 0)
                {
                    iShift = 2;
                    goto FIND_NONPRT;
                }
           }
           else if ((PTR_FSR == 0xF0) || (PTR_FSR == 0xF1)) // packet dial or packet data
           {
                FSR_REGISTER--;
                goto FIND_NONPRT;
           }
           i++;
       }
       while(i<5);
       return 0xff; 
FIND_NONPRT:
       i -=3; // that will be byte shift offset
       if (i & 0x80) // negative
       {
           FSR_REGISTER=&MyData[iLen];
           FSR_REGISTER--;
           j = i = -i;
           FSR1 = FSR_REGISTER - i;
           i = iLen - i;
           do
           {
              PTR_FSR = INDF1; FSR_REGISTER--; FSR1--;
           }
           while(--i);
           do
           {
               PTR_FSR = 0xaa;FSR_REGISTER--;
           }while(--j);
       }
       else if (i>0)
       {
           FSR1 = FSR_REGISTER; FSR1-=i;
           i = iLen - i -3;
           do
           {
              INDF1 = PTR_FSR; FSR_REGISTER++; FSR1++;
           }
           while(--i);
       }
       if (iShift)
       {
           FSR_REGISTER=&MyData[1];
           bByte = 0xaa;
           i = iLen-1;
           do
           {
               bByte1 = PTR_FSR;
               if (iShift == 2)
               {
#ifdef      _18F2321_18F25K20
                   #asm
                   RLNCF bByte1,1,1
                   RLNCF bByte1,1,1
                   #endasm
#else
                   pizdec
#endif             
                   bByte = bByte1 & 0x3;
                   bByte1 &= 0xfc;
               }
               else if (iShift == 4)
               {
#ifdef      _18F2321_18F25K20
                   #asm
                   SWAPF bByte1,1,1
                   #endasm
#else
                   pizdec
#endif             
                   bByte = bByte1 & 0xF;
                   bByte1 &= 0xf0;

               }
               else if (iShift == 6)
               {
#ifdef      _18F2321_18F25K20
                   #asm
                   SWAPF bByte1,1,1
                   RLNCF bByte1,1,1
                   RLNCF bByte1,1,1
                   #endasm
#else
                   pizdec
#endif             
                   bByte = bByte1 & 0x3F;
                   bByte1 &= 0xc0;
               }
               PTR_FSR  = bByte1;
               FSR_REGISTER--;
               PTR_FSR  |= bByte;
               FSR_REGISTER+=2;
           }
           while(--i);
       } 

/*
        if ((MyData[0] == 0xaa) && (MyData[1] == Addr1) && (MyData[2] == Addr2) && (MyData[3] == Addr3))
            return 0xff;
        // definetly first 4 bytes is not 0xaa+<addr> == it is possible to do len fix
        if ((MyData[26] == 0) && (MyData[27] == 0)) // at the end zero == strongly posible to do fix
        {
            for (i=27; i>=4; i--)
            {
               bByte = MyData[i-4];
               MyData[i] = bByte;
            }
            MyData[0] = 0xaa; MyData[1] = Addr1; MyData[2]= Addr2; MyData[3]=Addr3;
            for (i = 0; i < 26; i++)
               wCRCupdt(MyData[i]);
            CRCcmp = ((((UWORD)MyData[26]))<<8);
            CRCcmp += ((UWORD)MyData[27]);
            if (CRC == CRCcmp)
               return 0;
        }
*/
   }

   return 0; 
}
unsigned char BTFix3(void)
{
    unsigned char *ptr1 = BTqueueIn;
    unsigned char *ptr2 = BTqueueIn2;
    unsigned char *ptr3 = BTqueueIn3;
    unsigned char bByte1;
    unsigned char bByte2;
    unsigned char bByte3;
    unsigned char mask;
    unsigned char i;
    unsigned char iCrc = 26;
    CRCcmp=0;
    CRC=0xffff;   

    /*for (i = 0; i < 4; i++)
    {
        bByte1 = *ptr1;
        mask = (bByte1 ^ *ptr2);
        bByte1 &= mask ^ 0xff; 
        bByte1 |= mask & (*ptr3);
        *ptr1 = bByte1;
        wCRCupdt(bByte1);
        ptr1++;ptr2++;ptr3++;
    }
    wCRCupdt(*ptr1);
    ptr1++;ptr2++;ptr3++;*/
    for (i = 0; i < iCrc; i++)
    {
        bByte1 = *ptr1;
        if (i == 4) // offset of a FQ because this is a FQ1 then it easy to set
        {
            //if (ATCMD & MODE_CONNECT)
            //    bByte1 = OpponentFreq1;
            goto CONTINUE;
        }
        mask = (bByte1 ^ *ptr2);
        bByte1 &= mask ^ 0xff; 
        bByte1 |= mask & (*ptr3);
CONTINUE:
        *ptr1 = bByte1;

        wCRCupdt(bByte1);
        if (i == PACKET_LEN_OFFSET)
        {
            if (bByte1<= BT_TX_MAX_LEN)
                iCrc = bByte1 + sizeof(PacketStart);
        }

        ptr1++;ptr2++;ptr3++;
    }
    CRCcmp = ((((UWORD)*ptr1))<<8); ptr1++;
    CRCcmp += ((UWORD)*ptr1);
    if (CRC == CRCcmp)
    {
        BTFlags.BTFixed = 1;
        return 0;
    }
    return 0xff;
}
//=========================================================================================================
//   received data -> to output
//=========================================================================================================
void ProcessBTdata(void)
{
    //CRC = BTbyteCRC(CRC,0xaa);// preambul offset 0
    //CRC = BTbyteCRC(CRC,Addr1);  // addr1 offset 1
    //CRC = BTbyteCRC(CRC,Addr2);  // addr2 offset 2
    //CRC = BTbyteCRC(CRC,Addr3);  // addr3   // done for a case of missing first preambul+addr offset 3
    //CRC = BTbyteCRC(CRC,BTFQcurr);  // current frequency offset 4
    //CRC = BTbyteCRC(CRC,BTpkt);  // sequence/packet offset 5
    //CRC = BTbyteCRC(CRC,BTqueueOutLen);  // length   offset 6

    unsigned char *ptrMy =&BTqueueIn[0] ;
    unsigned char ilen;
    struct PacketStart *MyPacket;
    unsigned int BeginAddr;
    if (BTFlags.BT3fqProcessed)
        return;
    BTFlags.BT3fqProcessed = 1; // packet(s) is(are) fine == process done == flag to avoid process duplication packets

    if (BTokMsg ==1) //msg2
        ptrMy =&BTqueueIn2[0] ;
    else if (BTokMsg ==2) //msg3
        ptrMy =&BTqueueIn3[0] ;
    MyPacket = ptrMy;   

    ilen = MyPacket->BTpacketLen;//ptrMy[6];
#ifdef DEBUG_LED
    DEBUG_LED_ON;
#endif
    if (MyPacket->BTpacket == PCKT_DIAL)// (ptrMy[5] & PCKT_DIAL) // receved packet = dial call
    {
         //          6                          7                  8                  9 
        //BTqueueOut[0] = 'L'/*'l'*/;BTqueueOut[1] = 'u';BTqueueOut[2] = 'n';BTqueueOut[3] = 'a';
        //          10                    11                    12
        //BTqueueOut[4] = Addr1;BTqueueOut[5] = Addr2;BTqueueOut[6] = Addr3;
        //          13                    14                    15
        //BTqueueOut[7] = Freq1;BTqueueOut[8] = Freq2;BTqueueOut[9] = Freq3;
        // process to adjust frequency for different temperature ranges
        OpponentAddr1 = MyPacket->Adr1;OpponentAddr2 = MyPacket->Adr2;OpponentAddr3 = MyPacket->Adr3;
        OpponentFreq1 = MyPacket->Fq1;OpponentFreq2 = MyPacket->Fq2;OpponentFreq3 = MyPacket->Fq3;
        if (BTokMsg == 0)
        {
            Freq2 = Freq1 + (OpponentFreq2 - OpponentFreq1);
            Freq3 = Freq1 + (OpponentFreq3 - OpponentFreq1);
        }
        if (BTokMsg == 1)
        {
            Freq1 = Freq2 - (OpponentFreq2 - OpponentFreq1);
            Freq3 = Freq2 + (OpponentFreq3 - OpponentFreq2);
        }
        if (BTokMsg == 2)
        {
            Freq1 = Freq3 - (OpponentFreq3 - OpponentFreq1);
            Freq2 = Freq3 - (OpponentFreq3 - OpponentFreq2);
        }
        if (MyPacket->Type == 'e') 
        {        
            if (ATCMD & MODE_CALL_LUNA_COM) 
            {
                if (ATCMD & MODE_CONNECT) // connection was already established
                {
                }
                else
                    goto SEND_CONNECT;
            }
        }
        else if (MyPacket->Type == 'l')
        {
            if (ATCMD & MODE_CALL_EARTH)
            {
                if (ATCMD & MODE_CONNECT) // connection was already established
                {
                    // dial message from the earth - frquensies can be adjusted
                    ATCMD &= (RESPONCE_WAS_SENT ^0xff);
                }
                else
                {
SEND_CONNECT:
                    PutsToUnit("\n\rCONNECT\n\r");
                }
            }
        }
        else if (MyPacket->Type == 'W') // write data directly to FLASH/magnetoresistive memory/ferromagnetic memory
        {
        }
        else if (MyPacket->Type == 'R') // read request from FLASH/magnetoresistive memory/ferromagnetic memory
        {
        }
        else if (MyPacket->Type == 'P') // ping packet - now need to send responce right away
        {
            Main.PingRQ = 1;
        }
        else if (MyPacket->Type == 'p') // responce on ping packet
        {
            //Main.PingRQ = 1;
        }
        ATCMD |= MODE_CONNECT;
        
    }
    else // another packets == data
    {
        ptrMy+=sizeof(PacketStart);
#ifdef NON_STANDART_MODEM
        // write good packet into FLASH memory for processing
        CS_HIGH;  // that will interrupt FLASH operations read and write initiated from com

        PrevLen = NextLen;
        NextLen = ilen;
        TypePkt = 0;

        AdrBH = FlashEntryBH;
        wAddr = FlashEntry;
        if (FlashQueueSizeBH < FLASH_BUFFER_LEN_BH)
        {
            if (FlashQueueSize < FLASH_BUFFER_LEN)
            {
                Push2Flash(PrevLen);
                FlashQueueSize++;
                Push2Flash(NextLen);
                FlashQueueSize++;
                Push2Flash(TypePkt);
                FlashQueueSize++;
                do
                {
                    if (FlashQueueSizeBH < FLASH_BUFFER_LEN_BH)
                    {
                        if (FlashQueueSize < FLASH_BUFFER_LEN)
                        {
                            Push2Flash(*ptrMy); // inside: wAddr++
                            if (wAddr >= FLASH_BUFFER_LEN)
                                wAddr = 0;
                            FlashQueueSize++;
                            if (FlashQueueSize == 0)
                                FlashQueueSizeBH++;
                        }
                        //else  // loosing data !!!!
                        //{
                        //}
                    }
                    //else  // loosing data !!!!
                    //{
                    //}
                    ptrMy++;
                } while(--ilen);
             
                FlashEntry = wAddr;
                FlashEntryBH = AdrBH;
            }
        }
        CS_HIGH; // that will keep interrupted read/write from com (if any) -
                 //  on next byte from com interrupted w/r will send to flash enable write/interrupted addr
#else // NOT NON_STANDART_MODEM
        if (ATCMD & MODE_CALL_LUNA_COM)
        {
OUTPUT_DIRECTLY:
            /*
            if (BTokMsg == 0)
            {
               if (BTFlags.BTFixed)
                  putch('3');
               else
                  putch('0');
            }
            else if (BTokMsg == 1)
               putch('1');
            else if (BTokMsg == 2)
               putch('2');
            */
            do
            {
WAIT_SPACE_Q:
                if (AOutQu.iQueueSize >= (OUT_BUFFER_LEN-3)) // is enought space to output ??
                    goto WAIT_SPACE_Q;
                putch(*ptrMy);
#ifdef DEBUG_LED
                if (ATCMD & MODE_CALL_EARTH)
                {
                    if (*ptrMy == '?')
                    {
                        if (AInQu.iQueueSize == 0)
                        {
                            AInQu.Queue[AInQu.iEntry] = '?';
                            if (++AInQu.iEntry >= BUFFER_LEN)
                                AInQu.iEntry = 0;
                            AInQu.iQueueSize++;
                        }
                    }
                }
#endif
                ptrMy++;
            } while(--ilen);
        }
        else if (ATCMD & MODE_CALL_EARTH)
        {
            
            if (UnitFrom)
            {
                Main.SendWithEsc = 1;
                putch(UnitFrom);
                if (SendCMD)
                    putch(SendCMD);
                
                Main.SendWithEsc = 1;
                do
                {
                    putchWithESC(*ptrMy);
                    ptrMy++;
                } while(--ilen);
                Main.SendWithEsc = 0;
                putch(UnitFrom);
            }
            else
                goto OUTPUT_DIRECTLY;
        }
#endif // DONE with STANDART_MODEM
    }        
}
void BTCE_low(void)
{
    bitclr(PORT_BT,Tx_CE);	// Chip Enable Activates RX or TX mode (now disable)
}
void BTCE_high(void)
{
    bitset(PORT_BT,Tx_CE);	// Chip Enable Activates RX or TX mode (now TX mode) 
}
void AdjTimer3(void)
{
    if (DataB0.Timer3Ready2Sync)
    {
        // CRCcmp is just working variable
        CRCcmp = Tmr3LoadLow = Tmr3LoadLowCopy + Tmr3LoadLowCopy + MEDIAN_TIME;
        
        TMR3ON = 0;                       // stop timer3 (RX) for a moment
        //Tmr3LoadLow = ((MEDIAN_TIME - 0xffff)  + AdjustTimer3);
        //Tmr3LoadLow = Tmr3LoadLowCopy - Tmr3LoadLow;
        Tmr3LoadLow -= AdjustTimer3;
        TMR3ON = 1;                       // start timer (RX) back
        //if (Carry)
        //    Tmr3LoadLowCopy--;
        //else
        //    Tmr3LoadLowCopy++;
        DataB0.Timer3Ready2Sync = 0;
    }
}
//===========================================================================================
unsigned char ReceiveBTdata(void)
{
    unsigned char bret = 1;
   //unsigned char iCrc;
    unsigned char i;
    unsigned char bByte;
    unsigned char *ptrMy =&BTqueueIn[0] ;
    //CRC=0;

    if (DataB0.Timer3SwitchRX)
        BTCE_low();  // Chip Enable Activates RX or TX mode (now disable)
    if (RXreceiveFQ == 0) // received over FQ1
    {
        BTqueueInLen = 0;
        BTqueueInLen2 = 0;
        BTqueueInLen3 = 0;
        BTFlags.BT3fqProcessed = 0; // set flag that process was not done yet
        BTokMsg = 0xff;
        BTFlags.BTFixed = 0;
    }
    else if (RXreceiveFQ == 1) // received over FQ2
    {
        ptrMy =&BTqueueIn2[0] ;
    }
    else if (RXreceiveFQ == 2) // received over FQ3
    {
        ptrMy =&BTqueueIn3[0] ;
    }
    ptrMy[0] = 0;ptrMy[1] = 0;

    bitclr(PORT_BT,Tx_CSN);
    SendBTbyte(0x60); // 0110  0000 command R_RX_PL_WID
    bret = GetBTbyte();
    bitset(PORT_BT,Tx_CSN);
 
    bitclr(PORT_BT,Tx_CSN);
    SendBTcmd(0x61); // 0110  0001 command R_RX_PAYLOAD
    //iCrc = 26;
    for (i = 0; i< bret; i++)
    {
         bByte = GetBTbyte();
         if (i < 28)
         {
             ptrMy[i] = bByte;
         }
    }
    bitset(PORT_BT,Tx_CSN);

    bitclr(PORT_BT,Tx_CSN);
    SendBTbyte(0x27); // 0010 0111 command W_REGISTER =register is status = clean RX interrupt
    SendBTbyte(0x40); // clean RX interrupt
    bitset(PORT_BT,Tx_CSN);

    // TBD: output data as it is over com2 with speed at least 150000 bits/sec - ground station only
    i = BTFixlen(ptrMy, bret); // if it is posible to fix packet and packet was fixed i == 0

    if (!DataB0.Tmr3DoneMeasureFq1Fq2)  // FQ1-FQ2 measurements was not done yet
    {
        if (RXreceiveFQ == 0) 
        {
            if (i == 0)
                goto LOOKS_GOOD;
            // message looks bad on FQ1
            BTCE_high(); // Chip Enable Activates RX or TX mode (now RX mode) 
            return 0;
        }
        else if (RXreceiveFQ == 1) 
        {
            if (i == 0)
            {
                DataB0.Tmr3DoneMeasureFq1Fq2 = 1;
                goto LOOKS_GOOD;
            }
            // stop timer 3 == mesaurements will be done on next two good packets over FQ1 and FQ2
            TMR3ON = 0;
        }
    }
    else //if (DataB0.Tmr3DoneMeasureFq1Fq2)  // FQ1-FQ2 time measurement was done successfully
    {
    }
LOOKS_GOOD:
    // that switch will update BTStatus and switch frequency
    SwitchFQ(DoFqRXSwitch()); // if it was RX over FQ1 than value RXreceiveFQ ==0

    BTCE_high(); // Chip Enable Activates RX or TX mode (now RX mode) 
         
    // next packet ready to receive (on next frequency) - now prcocessing  
    if (bret > 28)
        bret = 28;
#ifdef _OLD_VERSION
if (BTokMsg == 0xff) // if paket was not recevet yet correctly
    {
        if (i == 0)  // if packet is possible to fix (and fixed)
        {
            if (CheckPacket(ptrMy, bret) == 0)    // CRC check ???
            {
                BTokMsg = RXreceiveFQ;
                if (DataB0.Tmr3DoneMeasureFq1Fq2)  // FQ1-FQ2 timeing was measured by timer3 (RX)
                {
ADJUST_TMR3:
                    AdjTimer3();

                    i = ptrMy[4];
                    if (i > RXreceiveFQ)
                        RXreceiveFQ = i;

                    if (DataB0.Tmr3RxFqSwitchLost)
                    {
                        DataB0.Tmr3RxFqSwitchLost = 0;
                        DataB0.Timer3SwitchRX = 1;
                        // round-robin already switched - just need to start from FQ1
                        TMR3ON = 0;
                        FqRXCount = 1;
                        i = Freq2;
                        TMR3ON = 1;
                        SwitchFQ(i);
                    }
                }
            }
        }
    }
    else
    {
        if (DataB0.Tmr3DoneMeasureFq1Fq2)
        {
            if (CheckPacket(ptrMy, bret) == 0)
                goto ADJUST_TMR3;
        }    
    }
#else
    if (BTokMsg == 0xff) // if paket was not recevet yet correctly (i.e. FQ1 not evaluated, or FQ1 was bad, or FQ1 FQ2 was bad)
    {
        if (i == 0)  // if packet possible to fix (and was fixed by shift)
        {
            if (CheckPacket(ptrMy, bret) == 0)    // now possible to do CRC check ???
            {
                BTokMsg = RXreceiveFQ;
                if (DataB0.Tmr3DoneMeasureFq1Fq2)  // FQ1-FQ2 timeing was measured by timer3 (RX)
                {
ADJUST_TMR3:
                    AdjTimer3();

                    // TBD that is not working but idea is: when packet received frequency number is in the packet 
                    //i = ptrMy[4];
                    //if (i > RXreceiveFQ)
                    //{
                    //    RXreceiveFQ = i;
                    //}

                    // lost sync of the packets (2 sec)
                    /*
                    if (DataB0.Tmr3RxFqSwitchLost)
                    {
                        DataB0.Tmr3RxFqSwitchLost = 0;
                        DataB0.Timer3SwitchRX = 1;
                        // round-robin already switched - just need to start from FQ1
                        TMR3ON = 0;
                        FqRXCount = 1;
                        i = Freq2;
                        TMR3ON = 1;
                        SwitchFQ(i);
                    }
                    */
                }
            }
        }
    }
    else
    {
        if (DataB0.Tmr3DoneMeasureFq1Fq2)
        {
            if (CheckPacket(ptrMy, bret) == 0)
                AdjTimer3();
        }    
    }
#endif
    // huck - but who cares? = len of a packet in offset of 28 

    ptrMy[LEN_OFFSET_INPUT_BUF] = bret;;
    if (++RXreceiveFQ >=3) // recevie over FQ3
    {
        RXreceiveFQ = 0;
        //BTqueueInLen3 = bret;
        if (BTokMsg == 0xff) // needs to fix packets
        {
            // 2. attempt to find matching size for all 1-2-3
            // if matching was found then do fix by majority rools 2 bits from 3 == 1 -> result = 1 and etc.
            // result stored in 1 then CRC recalulation on 1 
            // 3. attempt to find matchiong size of any 1-2 or 2-3 or 3-1
            // reported len will be 2 bytes less (CRC) then original recieved len (28)
            if ((BTqueueInLen == 28) && (BTqueueInLen2 == 28) && (BTqueueInLen3 == 28)) // all 3 matched size
            {
                BTokMsg = BTFix3();
                //BTokMsg = 0x80  | BTFix3();
                if (BTokMsg == 0)
                {
                    AdjTimer3();
                    /*
                    if (DataB0.Tmr3RxFqSwitchLost)
                    {
                        DataB0.Tmr3RxFqSwitchLost = 0;
                        DataB0.Timer3SwitchRX = 1;
                        // round-robin already switched - just need to start from FQ1
                        TMR3ON = 0;
                        FqRXCount = 1;
                        i = Freq2;
                        TMR3ON = 1;
                        SwitchFQ(i);
                    }*/
                }
                //BTokMsg = 0x80  | CheckPacket(BTqueueIn, BTqueueInLen);
            }
            else if ((BTqueueInLen == 28) && (BTqueueInLen2 == 28)) // all 2 matched size FQ1 & FQ2
            {
            }
            else if ((BTqueueInLen == 28) && (BTqueueInLen3 == 28)) // all 2 matched size FQ1 & FQ3
            {
            }
            else if ((BTqueueInLen2 == 28) && (BTqueueInLen3 == 28)) // all 3 matched size FQ2 & FQ3
            {
            }
        }
    }
    //else
    //{
        //if (RXreceiveFQ == 1) // recevie over FQ1
        //{
        //    BTqueueInLen = bret;
        //}
        //else // receive over FQ2
        //{
        //    BTqueueInLen2 = bret;
        //    // quick check that packet is OK (not a garbage)
        //    /*if (DataB0.Time3JustDone)
        //    {
        //        DataB0.Time3JustDone = 0;
        //        Time4Packet = (Tmr3LoadHigh<<9);
        //        Time4Packet |= ((unsigned char)Tmr3LoadLow)<<1;
        //        if (Tmr3LoadLow & 0x8000)
        //            Time4Packet |= 0x0100;
        //        if (Tmr3LoadLow & 0x80)
        //            Time4Packet |= 1;
        //    }*/
        //
        //}
    //}
DONE_RX:
    if (BTStatus & 0x40) // another unprocessed RX in a queue
    {
         Main.ExtInterrupt = 1;
         TMR3ON =0;
         SkipPtr++;
         TMR3ON =1;
    }
    return (BTokMsg != 0xff);
}
//===============================================================================================
void TransmitBTdata(void)
{
//#define ERROR_IN_PK_1 1
    // calculation of a transmit time:
    //  10mks after PTX CE high
    //  130mks PLL lock
    //  32 bytes + 1(preambul) + 3  (address) + 1 paclet control field = 37 bytes
    // 37 bytes * 8 = 296 bits = 300 bits
    // 300 bits / (1000000/s) = 296 mks
    // 6 mks IRQ time
    // total = 10 + 130 + 296 + 6 = 442mks 
    // on a processor 32MHz with 128 prescaler 4 counts=64mks
    // 446mks = 28 counts
    //
    // calculation of a time to upload 32 bytes:
    // 7682 cycles = 960 mks (or for PIC18F23K20 == twice faster) 480mks 
    unsigned char i;
    
    unsigned char bBy;
    CRC = 0xffff;
    if (BTqueueOutLen)
    {
        if (TXSendOverFQ == 0) // send over FQ1
        {
   
#ifdef SAVE_SPACE
            FSR_REGISTER = &BTqueueOut[0];
            BTqueueOutCopyLen = BTqueueOutLen;
            FSR1 = &BTqueueOutCopy[0];
            do
            {
                INDF1 = PTR_FSR;
            } while(--BTqueueOutLen);
#else            
            for (i = 0; i <BTqueueOutLen;i++)
            {
               bBy = BTqueueOut[i];
               BTqueueOutCopy[i] = bBy;
            }
            BTqueueOutCopyLen = BTqueueOutLen;
            BTqueueOutLen = 0;
#endif
            BTpktCopy = BTpkt;
        }
    }
    if (BTqueueOutCopyLen)
    {
        PORT_AMPL.BT_RX = 0;              // off RX amplifier
        BTCE_low();	// Chip Enable Activates RX or TX mode (now disable)
        BTType = 2; // type TX
        bitclr(PORT_BT,Tx_CSN);
        BTStatus= SendBTcmd(0xa0); // 1010  0000 command W_TX_PAYLOAD to register 00101 == RF_CH
        
        BTbyteCRC(0xaa);// preambul offset 0
        BTbyteCRC(0xaa);// preambul offset 1
        BTbyteCRC(0xaa);// preambul offset 2
        //BTbyteCRC(Addr1);  // addr1 offset 1
        //BTbyteCRC(Addr2);  // addr2 offset 2
        //BTbyteCRC(Addr3);  // addr3   // done for a case of missing first preambul+addr offset 3
        BTbyteCRC(BTpktCopy);  // sequence/packet offset 3
        BTbyteCRC(TXSendOverFQ);  // current frequency offset 4
        BTbyteCRC(BTqueueOutCopyLen);  // length   offset 5
        
        for (i = 0; i <BTqueueOutCopyLen;i++)
        {
#ifndef ERROR_IN_PK_1
            BTbyteCRC(BTqueueOutCopy[i]);
#else
            if (!(ATCMD & MODE_CONNECT))
                goto SEND_GOOD;
            if (i == 0)
            {
               if (TXSendOverFQ == 0)
               {
                    SendBTbyte(0);
                    wCRCupdt(BTqueueOutCopy[i]);
               }
               else goto SEND_GOOD;
            }
            else if (i == 1)
            {
               if (TXSendOverFQ == 1)
               {
                   SendBTbyte(0xff);
                   wCRCupdt(BTqueueOutCopy[i]);
               }
               else goto SEND_GOOD;
            }
            else if (i == 2)
            {
               if (TXSendOverFQ == 2)
               {
                   SendBTbyte(0xf0);
                   wCRCupdt(BTqueueOutCopy[i]);
               }
               else goto SEND_GOOD;
            }
            else
            {
SEND_GOOD:      BTbyteCRC(BTqueueOutCopy[i]);
            }
#endif
        }
        SendBTbyte(CRC>>8);  
        SendBTbyte(CRC&0x00ff);  
        SendBTbyte(0xff);  
        //   max len  header              CRC  end  Len
        i =    32    -6 -2    -1  -BTqueueOutCopyLen;
#ifdef SAVE_SPACE
        while (i > 0) 
        {
            SendBTbyte(0);  // padding zero till the end
            i--;
        }
#else
        do 
        { 
            SendBTbyte(0);  // padding zero till the end
        } while(--i);
#endif
        bitset(PORT_BT,Tx_CSN);
    
     
      // clean TX interrupt before send
      bitclr(PORT_BT,Tx_CSN);
      SendBTbyte(0x27); // 0010 0111 command W_REGISTER =register is status = clean TX interrupt
      SendBTbyte(0x20); // clean TX interrupt
      bitset(PORT_BT,Tx_CSN);

                 // data in Configuration Register
                 // 0101 0010  
                 //  1       MASK_RX_DR Mask interrupt caused by RX_DR
                 //           1: Interrupt not reflected on the IRQ pin
                 //           0: Reflect RX_DR as active low interrupt on the IRQ pin
                 //   0      MASK_TX_DS Mask interrupt caused by TX_DS
                 //           1: Interrupt not reflected on the IRQ pin
                 //           0: Reflect TX_DS as active low interrupt on the IRQ pin
                 //    1     MASK_MAX_RT Mask interrupt caused by MAX_RT
                 //           1: Interrupt not reflected on the IRQ pin
                 //           0: Reflect MAX_RT as active low interrupt on the IRQ pin
                 //      0   EN_CRC     Enable CRC. Forced high if one of the bits in
                 //           the EN_AA is high
                 //       0  CRCO CRC encoding scheme '0' - 1 byte '1' � 2 bytes
                 //        1 PWR_UP 1: POWER UP, 0:POWER DOWN
                 //         0PRIM_RX	RX/TX control 1: PRX, 0: PTX
	
        if (TXSendOverFQ ==0) // over FQ1 need to switch to TX mode == for FQ2 & FQ3 it is already done
        {
            bitclr(PORT_BT,Tx_CSN);
            BTStatus= SendBTcmd(0x20); // 001 0  0000 W_REGISTER to register 00000 == Configuration Register
            SendBTbyte(0x52);
            bitset(PORT_BT,Tx_CSN); // SPI Chip Select
        }

        INT0_FLG = 0;
        Main.ExtInterrupt = 0;
        
        

        if (DataB0.Timer1Done3FQ)
        {
            if (++TXSendOverFQ >= 3) // that is FQ3
                TXSendOverFQ = 0;
SWITCH_ANOTHER:
            i = FqTX;
            SwitchFQ(i); // in that call for TX over FQ1 value of TXSendOverFQ == 1
            TMR1ON = 0; // stop a timer for a little bit == to prevent interrupt at compare operation
            if (i != FqTX)
            {
                TMR1ON = 1;
                if (TXSendOverFQ == 1) // was FQ1
                    goto SWITCH_ANOTHER;
                goto TRANSMIT_NOW;
            }
            DataB0.Timer1DoTX = 1; // timer 1 will initiate transmit
            TMR1ON = 1;
        }
        else
        {

#ifdef SKIP_CALC_TX_TIME
   // in final version value to send data is already calculated 
#else 
   // that code for measurements only - it suppose to give a time btw FQ1 & FQ2
           SwitchFQ(FqTX); // for that call for TX  over FQ1 value TXSendOverFQ == 0
            if (++FqTXCount>=3)
            {
                FqTXCount = 0;
                FqTX = Freq1;
            }
            else
            {
                if (FqTXCount == 1)
                    FqTX = Freq2;
                else
                    FqTX = Freq3;
            }

            if (++TXSendOverFQ == 1) // send over FQ1
            {
                DataB0.Timer1Count = 0;
                DataB0.Timer1Meausre = 1;
                Tmr1High = 0;
                SetTimer1(0);  // timer 1 measure time btw fq1-fq2 transmit 
            }
            else // send over FQ2
            {
                if (DataB0.Timer1Meausre)
                {
                    TMR1ON = 0;              // stop timer measure it time btw send Fq1 -> Fq2
                    Tmr1LoadHigh = 0xffff - Tmr1High; // this will
                    Tmr1LoadLow = 0xffff - TIMER1;      // timer1 interupt reload values 
                    Tmr1TOHigh = Tmr1LoadHigh;
                    SetTimer1(Tmr1LoadLow);
                    DataB0.Timer1Done3FQ = 1; 
                    DataB0.Timer1Meausre = 0;
                    DataB0.Timer1Count = 1;

                }
            }
#endif
TRANSMIT_NOW:
            PORT_AMPL.BT_TX = 1;              // TX amplifier : TBD: is it enought time to start amplifier?
            BTCE_high(); // Chip Enable Activates RX or TX mode (now TX mode) 
        }
        INT0_ENBL = 1;
        TMR1ON = 1; // start temporary stoped timer (if it was stopped!)
        TMR0ON = 0; // during transmit no interrupt on timer 0
        I2C.Timer0Fired = 0;
    }
}
void SwitchToRXdata(void)
{
    PORT_AMPL.BT_TX = 0;              // off TX amplifier
    BTCE_low(); // Chip Enable Activates RX or TX mode (now disable)
    BTType = 1; // type RX

    SwitchFQ(FqRX); // switch back FQ1

    bitclr(PORT_BT,Tx_CSN); // SPI Chip Select // pipe 0
    SendBTbyte(0x27); // 0010  0111 command W_REGISTER to register 00111 == STATUS
    SendBTbyte(0x20); // clean TX interrupt
    bitset(PORT_BT,Tx_CSN);

                 // data in Configuration Register
                 // 0011 0011  
                 //  0       MASK_RX_DR Mask interrupt caused by RX_DR
                 //           1: Interrupt not reflected on the IRQ pin
                 //           0: Reflect RX_DR as active low interrupt on the IRQ pin
                 //   1      MASK_TX_DS Mask interrupt caused by TX_DS
                 //           1: Interrupt not reflected on the IRQ pin
                 //           0: Reflect TX_DS as active low interrupt on the IRQ pin
                 //    1     MASK_MAX_RT Mask interrupt caused by MAX_RT
                 //           1: Interrupt not reflected on the IRQ pin
                 //           0: Reflect MAX_RT as active low interrupt on the IRQ pin
                 //      0   EN_CRC     Enable CRC. Forced high if one of the bits in
                 //           the EN_AA is high
                 //       0  CRCO CRC encoding scheme '0' - 1 byte '1' � 2 bytes
                 //        1 PWR_UP 1: POWER UP, 0:POWER DOWN
                 //         1PRIM_RX	RX/TX control 1: PRX, 0: PTX
	


    bitclr(PORT_BT,Tx_CSN);
    BTStatus= SendBTcmd(0x20); // 001 0  0000 W_REGISTER to register 00000 == Configuration Register
    SendBTbyte(0x33);
    bitset(PORT_BT,Tx_CSN); // SPI Chip Select

    PORT_AMPL.BT_RX = 1;              // on RX amplifier
    //INT0_FLG = 0;
    Main.ExtInterrupt = 0;
    
    BTokMsg = 0xff;
    BTCE_high(); // Chip Enable Activates RX or TX mode (now RX mode) 
    //BTType &= 0xfd; // 0x02 // clean TX mode
    //BTType |= 0x01; // set RX mode
    INT0_ENBL = 1;
    //BTInturrupt = 0;

    if (BTStatus & 0x40) // was RX during TX attempt == then needs to read what it was
    {
        Main.ExtInterrupt = 1;
        //BTInturrupt = BTStatus;
    }
}
void SwitchFQ(unsigned char iFQ)
{
    if (DataB0.Timer3SwitchRX)
    {
        if (DataB0.Timer1SwitchTX)   // Mode  switchRX ==1 && switchTX ==1  case "G" (or 0x00000011)
        {
DO_SWITCHFQ:
            bitclr(PORT_BT,Tx_CSN);
            BTStatus= SendBTcmd(0x25); // 0010  0101 command W_REGISTER to register 00101 == RF_CH
            SendBTbyte(iFQ);  // set channel = FQ1
            BTFQcurr = iFQ;
            bitset(PORT_BT,Tx_CSN);
        }
        else                         // Mode  switchRX ==1 && switchTX ==0
        {
            if (BTType == 1)  // calls from RX places
                goto DO_SWITCHFQ;
            if (BTType == 2)  // calls from TX places
            {
                 if (DataB0.Timer1Done3FQ)
                 {
                     if (TXSendOverFQ == 1)
                     {
DO_SWITCH_TO_FQ1:
                         iFQ = Freq1;
                         goto DO_SWITCHFQ;
                     }
                 }
                 else
                 {
                     if (TXSendOverFQ == 0)
                         goto DO_SWITCH_TO_FQ1;
                 }
            }
            goto GET_STATUS;
        }
    }
    else
    {
        if (DataB0.Timer1SwitchTX)  // Mode  switchRX ==0 && switchTX ==1
        {
             if (BTType == 1)  // calls from RX places
             {
                  if (RXreceiveFQ == 0)
                      goto DO_SWITCH_TO_FQ1;
             }
             if (BTType == 2)  // calls from TX places
                 goto DO_SWITCHFQ;
            goto GET_STATUS;
        }
        else                        // Mode  switchRX ==0 && switchTX ==0
        {
GET_STATUS:
            bitclr(PORT_BT,Tx_CSN);
            BTStatus= SendBTcmd(0xff);
            bitset(PORT_BT,Tx_CSN);
        }
    }
}

void SetupBT(unsigned char SetupBtMode)
{
    // time for setup
    // 3211 cycles 401.mks
    unsigned char data;
    INT0_ENBL = 0; // disable interrupts
    BTCE_low(); // Chip Enable Activates RX or TX mode (now disable)
    if (!BTFlags.BTFirstInit)
    {
	    data = 0x79;//0x3d; why not???
                 // data in Configuration Register
                 // 0111 1001  
                 //  1       MASK_RX_DR Mask interrupt caused by RX_DR
                 //           1: Interrupt not reflected on the IRQ pin
                 //           0: Reflect RX_DR as active low interrupt on the IRQ pin
                 //   1      MASK_TX_DS Mask interrupt caused by TX_DS
                 //           1: Interrupt not reflected on the IRQ pin
                 //           0: Reflect TX_DS as active low interrupt on the IRQ pin
                 //    1     MASK_MAX_RT Mask interrupt caused by MAX_RT
                 //           1: Interrupt not reflected on the IRQ pin
                 //           0: Reflect MAX_RT as active low interrupt on the IRQ pin
                 //      0   EN_CRC     Enable CRC. Forced high if one of the bits in
                 //           the EN_AA is high
                 //       0  CRCO    CRC encoding scheme '0' - 1 byte '1' � 2 bytes
                 //        0 PWR_UP   1: POWER UP, 0:POWER DOWN
                 //         1PRIM_RX	RX/TX control 1: PRX, 0: PTX
        bitclr(PORT_BT,Tx_CSN); // SPI Chip Select
        BTStatus= SendBTcmd(0x20); //cmd = 0x20; = 001 0  0000 command W_REGISTER to register 00000 == Configuration Register
        // STATUS Status Register (In parallel to the SPI command word applied on the MOSI pin, the STATUS register
        //                         is shifted serially out on the MISO pin)
        // 0          - Only '0' allowed
        //  1         - RX_DR - Data Ready RX FIFO interrupt. Asserted when new data arrives RX FIFOb. Write 1 to clear bit.
        //   0        - TX_DS - Data Sent TX FIFO interrupt. Asserted when packet transmitted on TX. If AUTO_ACK is activated,
        //                      this bit is set high only when ACK is received. Write 1 to clear bit.
        //    0       - MAX_RT - Maximum number of TX retransmits interrupt Write 1 to clear bit. If MAX_RT is asserted it must
        //                       be cleared to enable further communication. 
        //     111    - RX_P_NO - Data pipe number for the payload available for reading from RX_FIFO
        //              000-101: Data Pipe Number
        //              110: Not Used
        //              111: RX FIFO Empty
        //        0   - TX_FULL TX FIFO full flag. 1: TX FIFO full.0: Available locations in TX FIFO.
        SendBTbyte(data);
        bitset(PORT_BT,Tx_CSN); // set high
        // deal with status:
        if (BTStatus & 0x10) // Maximum number of TX retransmits interrupt Write 1 to clear bit.
       {
           bitclr(PORT_BT,Tx_CSN); // SPI Chip Select // pipe 0
           SendBTbyte(0x27); // 0010  0111 command W_REGISTER to register 00111 == STATUS
           SendBTbyte(0x10);
           bitset(PORT_BT,Tx_CSN);
           // reads it again
           bitclr(PORT_BT,Tx_CSN); // SPI Chip Select
           BTStatus= SendBTcmd(0xff); //cmd = 0xff; = 1111 1111 command NOP to read STATUS register
           bitset(PORT_BT,Tx_CSN); // set high
       }
       if (BTStatus & 0x20) // TX_DS - Data Sent TX FIFO interrupt. Asserted when packet transmitted on TX. If AUTO_ACK is activated,
       {                    //         this bit is set high only when ACK is received. Write 1 to clear bit.
                            // at thet moment no transmit shuld be == clean interrupt
           bitclr(PORT_BT,Tx_CSN); // SPI Chip Select // pipe 0
           SendBTbyte(0x27); // 0010  0111 command W_REGISTER to register 00111 == STATUS
           SendBTbyte(0x20);
           bitset(PORT_BT,Tx_CSN);
           // reads it again
           bitclr(PORT_BT,Tx_CSN); // SPI Chip Select
           BTStatus= SendBTcmd(0xff); //cmd = 0xff; = 1111 1111 command NOP to read STATUS register
           bitset(PORT_BT,Tx_CSN); // set high
       }
       if (BTStatus & 0x40) // RX_DS - Data Sent TX FIFO interrupt. Asserted when packet transmitted on TX. If AUTO_ACK is activated,
       {                    //         this bit is set high only when ACK is received. Write 1 to clear bit.
                            // at thet moment no transmit shuld be == clean interrupt
           bitclr(PORT_BT,Tx_CSN); // SPI Chip Select // pipe 0
           SendBTbyte(0x27); // 0010  0111 command W_REGISTER to register 00111 == STATUS
           SendBTbyte(0x40);
           bitset(PORT_BT,Tx_CSN);
           // reads it again
           bitclr(PORT_BT,Tx_CSN); // SPI Chip Select
           BTStatus=SendBTcmd(0xe2); // 1110 0001 command FLUSH_RX
           bitset(PORT_BT,Tx_CSN); // set high
       }

       bitclr(PORT_BT,Tx_CSN); // SPI Chip Select // pipe 0
       SendBTbyte(0x21); // 0010  0001 command W_REGISTER to register 00001 == EN_AA
       SendBTbyte(00);  // 00 0 0 0 0 0 0 all ack on all pipes are disabled
       bitset(PORT_BT,Tx_CSN);

       /////////////////////////////////////////////////////////////////////////////////////////////////
       // for case very slow communication
       //  pipe preamble adr1 adr2 adr3 dat1 dat2 dat3 dat4 dat5 dat6 dat7 dat8       mean  len TX len RX
       //    1     AA   |  AA   AA   00|  AA   AA   AA   00   AA   AA   AA   00        = 0    8      4
       //    2     AA   |  AA   AA   FF|  AA   AA   AA   FF   AA   AA   AA   FF        = 1    8      4
       //    3     AA   |  AA   AA   AA|  AA   AA   AA  <byte>AA   AA   AA   <byte>    =<byte>8      4 
       //    4     AA   |  AA   AA   C0|  AA   AA   AA   C0  <byte>AAAAAAC0<byte>      =<byte>10     5
       //////////////////////////////////////////////////////////////////////////////////////////////////
       // sequence for transmitting is:
       // (a) pipe3 
       // (b) pipe1|pipe2|pipe1|pipe2|pipe1|pipe2|pipe1|pipe2
       // (c) pipe4
       // byte receved on pipe 3 must match byte receved on pipe4 and concatenated byte over pipe1+pipe2
       // error correction by best of 3
       // receving paket on pipe 1 mean receiving 0
       // receving paket on pipe 2 mean receving 1
       //////////////////////////////////////////////////////////////////////////////////////////////////
       bitclr(PORT_BT,Tx_CSN); // SPI Chip Select // pipe 0
       SendBTbyte(0x22); // 0010  0011 command W_REGISTER to register 00010 == EN_RXADDR
       SendBTbyte(0x01);  // 00 0 0 0 0 0 1 // pipe 0 enabled only
       bitset(PORT_BT,Tx_CSN);

    
       bitclr(PORT_BT,Tx_CSN);
       SendBTbyte(0x23); // 0010  0001 command W_REGISTER to register 00011 == SETUP_AW
       //SendBTbyte(0x01);  // 01 - 3 byte address 10 - 4 byte address 11 - 5 byte address
       SendBTbyte(0x03);  // 01 - 3 byte address 10 - 4 byte address 11 - 5 byte address
       bitset(PORT_BT,Tx_CSN);

       bitclr(PORT_BT,Tx_CSN);
       SendBTbyte(0x24); // 0010  0100 command W_REGISTER to register 00100 == SETUP_RETR
       SendBTbyte(0x00);  // 0- retransmit disbaled
       bitset(PORT_BT,Tx_CSN);

       bitclr(PORT_BT,Tx_CSN);
       SendBTbyte(0x25); // 0010  0101 command W_REGISTER to register 00101 == RF_CH
       //SendBTbyte(Freq1);  // set channel = FQ1
       SendBTbyte(BTFQcurr);
       bitset(PORT_BT,Tx_CSN);
       //BTFQcurr = Freq1;

       bitclr(PORT_BT,Tx_CSN);
       SendBTbyte(0x26); // 0010  0110 command W_REGISTER to register 00110 == RF_SETUP
       //SendBTbyte(0b00100110);  // 0 - continues carrier; 0; 1(0-force PLL)0 - 250 kbs;11 - 0dBm ; 0
                                // Only 000          - allowed, (001 and it is twice time of the TX packet) 
                                //         0         - Force PLL lock signal. Only used in test 
                                //          0        - Air Data Rate 1Mbps (�1� � 2Mbps)
                                //           11      - Set RF output power in TX mode '00' � -18dBm '01' � -12dBm '10' � -6dBm '11' � 0dBm
                                //             0     - Setup LNA gain The LNA gain makes it possible to reduce 
                                //                     the current consumption in RX mode with 0.8mA at the cost of
                                //                     1.5dB reduction in receiver sensitivity
       SendBTbyte(0b00000110);
       bitset(PORT_BT,Tx_CSN);
 
       bitclr(PORT_BT,Tx_CSN); // SPI Chip Select // address 
       SendBTbyte(0x30); // 0011  0000 command W_REGISTER to register 10000 == TX_ADDR_P0
       SendBTbyte(Addr1);   
       SendBTbyte(Addr2);   
       SendBTbyte(Addr3);   
       bitset(PORT_BT,Tx_CSN);

       bitclr(PORT_BT,Tx_CSN); // SPI Chip Select // address 
       SendBTbyte(0x2a); // 0010  1010 command W_REGISTER to register 01010 == RX_ADDR_P0
       SendBTbyte(Addr1);   
       SendBTbyte(Addr2);   
       SendBTbyte(Addr3);   
       SendBTbyte(0xaa);   
       SendBTbyte(0xaa);   
       bitset(PORT_BT,Tx_CSN);

       bitclr(PORT_BT,Tx_CSN); // SPI Chip Select // pipe 0
                       // 0000 0100 
                       //   11 0010 Number of bytes in RX payload in data pipe 0 (1 to 32 bytes).
                       //           0 Pipe not used 1 = 1 byte �32 = 32 bytes
                       //           for TX length defined by amount bytes clocked to RF24l01+
       SendBTbyte(0x31); // 0011  0001 command W_REGISTER to register 10001 == RX_PW_P0
       SendBTbyte(32-4);  // data = 32 byte payload will be preambul+addr1+addr2+addr3 
       bitset(PORT_BT,Tx_CSN);
/*
       bitclr(PORT_BT,Tx_CSN); // SPI Chip Select // pipe 1
       SendBTbyte(0x32); // 0011  0010 command W_REGISTER to register 10010 == RX_PW_P1
       SendBTbyte(32-3);   // data = 32 byte payload
       bitset(PORT_BT,Tx_CSN);

       bitclr(PORT_BT,Tx_CSN); // SPI Chip Select // pipe 2
       SendBTbyte(0x33); // 0011  0011 command W_REGISTER to register 10011 == RX_PW_P2
       SendBTbyte(32-3);   // data = 32 byte payload
       bitset(PORT_BT,Tx_CSN);

       bitclr(PORT_BT,Tx_CSN); // SPI Chip Select // pipe 3
       SendBTbyte(0x34); // 0011  0100 command W_REGISTER to register 10100 == RX_PW_P3
       SendBTbyte(32-3);   // data = 32 byte payload
       bitset(PORT_BT,Tx_CSN);

       bitclr(PORT_BT,Tx_CSN); // SPI Chip Select // pipe 4
       SendBTbyte(0x35); // 0011  0101 command W_REGISTER to register 10101 == RX_PW_P4
       SendBTbyte(32-3);   // data = 32 byte payload
       bitset(PORT_BT,Tx_CSN);
    
       bitclr(PORT_BT,Tx_CSN); // SPI Chip Select // pipe 5
       SendBTbyte(0x36); // 0011  0110 command W_REGISTER to register 10110 == RX_PW_P5
       SendBTbyte(32-3);   // data = 32 byte payload
       bitset(PORT_BT,Tx_CSN);
*/
       
      
       BTFlags.BTFirstInit = 1;
    }
    bitclr(PORT_BT,Tx_CSN);
    SendBTbyte(0xe1); // 1110 0001 command FLUSH_TX, Flush TX FIFO, used in TX mode
    bitset(PORT_BT,Tx_CSN);
    if (SetupBtMode == SETUP_TX_MODE)
        data = 0xb2;//10110010
    else
        data = 0x33;// = 0x3F; //PWR_UP = 1 ==============================================================================================
                 // data in Configuration Register
                 // 0011 0011  
                 //  0       MASK_RX_DR Mask interrupt caused by RX_DR
                 //           1: Interrupt not reflected on the IRQ pin
                 //           0: Reflect RX_DR as active low interrupt on the IRQ pin
                 //   1      MASK_TX_DS Mask interrupt caused by TX_DS
                 //           1: Interrupt not reflected on the IRQ pin
                 //           0: Reflect TX_DS as active low interrupt on the IRQ pin
                 //    1     MASK_MAX_RT Mask interrupt caused by MAX_RT
                 //           1: Interrupt not reflected on the IRQ pin
                 //           0: Reflect MAX_RT as active low interrupt on the IRQ pin
                 //      0   EN_CRC     Enable CRC. Forced high if one of the bits in
                 //           the EN_AA is high
                 //       0  CRCO CRC encoding scheme '0' - 1 byte '1' � 2 bytes
                 //        1 PWR_UP 1: POWER UP, 0:POWER DOWN
                 //         1PRIM_RX	RX/TX control 1: PRX, 0: PTX
	

    bitclr(PORT_BT,Tx_CSN);
    SendBTbyte(0x20); // 001 0  0000 W_REGISTER to register 00000 == Configuration Register
    SendBTbyte(data);
    bitset(PORT_BT,Tx_CSN); // SPI Chip Select
   

    PORT_AMPL.BT_RX = 1;              // on RX amplifier
    
    BTType = 1; // type RX
    BTokMsg = 0xff;
    BTCE_high(); // Chip Enable Activates RX or TX mode (now RX mode)
    //INT0_FLG = 0; // may be it will be some data allready in fifo buffers
    Main.ExtInterrupt = 0;
    INT0_ENBL = 1;
}
void wCRCupdt(int bByte)
{
    UWORD Temp ;
    unsigned char i;

    Temp = ((UWORD)bByte << 8);
    CRC ^= Temp;
    for (i = 8 ; i ; --i)
    {
        if (CRC & 0x8000)
        {
            //CRC = (CRC << 1) ^ 0x1021;
            CRC = (CRC << 1);
            CRC = CRC ^ 0x1021;
        }
        else
        {
            //CRC = (CRC << 1) ^ 0 ;
            CRC = (CRC << 1);
            CRC = CRC ^ 0 ;
        }
    }
}
unsigned char SendBTcmd(unsigned char cmd)
{
	int i= 8;
    unsigned char Data = 0;
    do 
    {
        Data <<=1;
        if (cmd & 0x80)
		    PORT_BT.Tx_MOSI = 1;
        else
            PORT_BT.Tx_MOSI = 0;
		PORT_BT.Tx_SCK = 1;
        if (bittest(PORT_BT_READ, Rx_MISO))
            Data |= 1;

        cmd <<= 1;
		PORT_BT.Tx_SCK = 0;
    } while(--i);
    return Data;
}

void SendBTbyte(unsigned char cmd)
{
	int i = 8;
    do 
    {
        if (cmd & 0x80)
		    PORT_BT.Tx_MOSI = 1;
        else
            PORT_BT.Tx_MOSI = 0;
		PORT_BT.Tx_SCK = 1;
        cmd <<= 1;
		PORT_BT.Tx_SCK = 0;
    } while(--i);
}

unsigned char GetBTbyte(void)
{
	int i = 8;
    unsigned int Data = 0;
    do 
    {
        Data <<= 1;
        PORT_BT.Tx_SCK = 1;
		if (bittest(PORT_BT_READ,Rx_MISO))
			Data |= 1;

		PORT_BT.Tx_SCK = 0;
    } while(--i);
    return Data;
}


