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
// protocol diagram for mode TX3 & RX3
// for 2321 time TTT == RRR == 0x2ADD = 10973 op = 0.001371625 sec
//          time 123456789AB = (TO= 93*128 = 11904 op = 0.001488 sec ) + (Packet prep = 18445) = 0x768D = 30349 = 0.003793625 sec
// i.e. time line :
// int =         0x2ACB == TTT time
// set timeout = 0x2BD4 == TO   
// before TX     0x5AB5 Transmit preperation !!!!!! == 1BAA = 7082 op
// TX set = 0x765F
// for receiver:
// time "123456789AB" - Time X = time "7654321"
// X = 0x3500= X > 0x2ACB
#define MEDIAN_TIME 0x4000
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
#define SKIP_CALC_TX_TIME 1


//// [1 GYRO]->[2 MEM]-> [3 POW] -> [4 STM] -> [5 BT] -| 
//  A                                                |
//  --------------------------------------------------
#define MY_UNIT '5' 

#define BT_TIMER1 1
#define BT_TIMER3 1


//#define __DEBUG
//#define SHOW_RX_TX
//#define SHOW_RX

/////////////////////////////////////////////////////////////////////////////////////////////////////
// define blinking LED on pin 14 (RC3)
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define DEBUG_LED
#ifdef DEBUG_LED
#define DEBUG_LED_OFF bitclr(PORTC,3)
#define DEBUG_LED_ON bitset(PORTC,3)
///////////////////////////////////////////////////////////////
//   for a blinking LED behive like CUBESAT/CRAFT
//   it is waiting for connection, wait for pkt, and when pkt is Ok it send back to earth reply packet, and blinks
///////////////////////////////////////////////////////////////
//#define DEBUG_LED_CALL_EARTH
///////////////////////////////////////////////////////////////
//   for a blinking LED behive like Ground Station, it is constantly sends pktm if received pkt, then it blinks
///////////////////////////////////////////////////////////////
#define DEBUG_LED_CALL_LUNA
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
#define I2C_ONLY_MASTER 1

// master support done via interrupts and firmware - commenting next line and I2C will be a software work
#define I2C_INT_SUPPORT 1

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

#define SSPORT PORTA
#define SSCLOCK 2
#define SSDATA_IN 3
#define SSDATA_OUT 4
#define SSCS       5

// this is for Cubesat version - 3 FLASH memory processing
#define SSPORT2  PORTC
#define SSDATA_OUT2 0
#define SSDATA_OUT3 1
#endif
/////////////////////////////////////////////////////////////////////////////////
//   BT definitions
/////////////////////////////////////////////////////////////////////////////////
#define PORT_BT PORTA
#define Tx_CE      0	// RA0 pin 2 // Chip Enable Activates RX or TX mode
#define Tx_CSN     1	// RA1 pin 3 // SPI Chip Select
#define Tx_SCK     2    // RA2 pin 4  // SPI Clock
#define Tx_MOSI    3	// RA3 pin 5  // SPI Slave Data Input
#define Rx_MISO    4	// RA4 pin 6  // SPI Slave Data Output, with tri-state option
#define Rx_IRQ     0    // RB0 pin 21 // Maskable interrupt pin. Active low
#define PORT_AMPL PORTB
#define BT_TX      1   // RB1 pin 22 BT in transmit mode
#define BT_RX      2   // RB2 pin 23 BT in receive mode

#define SETUP_RX_MODE 0
#define SETUP_TX_MODE 1

///////////////////////////////////////////////////////////////////////////////////
//   serial port semaphores
//     RX_FULL     signals to prev unit to send data
//     TX_NOT_READY   check next in loop for ready to receive data
////////////////////////////////////////////////////////////////////////////////////
#define RX_FULL PORTC.5
#define TX_NOT_READY PORTC.4



// redifine output buffer size
#define BUFFER_LEN 40
#define OUT_BUFFER_LEN 40


//#include "commc0.h"
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
#pragma config[1] = 0x06
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
#ifdef TX_NOT_READY
unsigned SuspendTX:1;
unsigned SuspendRetrUnit:1;
unsigned InputQueueIsFull:1;
unsigned PauseOutQueueFull:1;
#endif
unsigned RetransmitTo:1;
#ifdef NON_STANDART_MODEM
unsigned SendOverLink:1;
unsigned SendOverLinkAndProc:1;
unsigned FlashRQ:1;
#endif
VOLATILE unsigned SomePacket:1;
VOLATILE unsigned OutPacket:1;
VOLATILE unsigned OutPacketESC:1;
VOLATILE unsigned OutPacketZeroLen:1;

VOLATILE unsigned PacketProcessing:1;
VOLATILE unsigned CMDToProcessGetESC:1;
VOLATILE unsigned CMDProcessLastWasUnitAddr:1;
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

VOLATILE unsigned prepStream:1;
VOLATILE unsigned prepCmd:1;
VOLATILE unsigned InComRetransmit:1;
VOLATILE unsigned InComZeroLenMsg:1;
#ifdef EXT_INT
VOLATILE unsigned ExtInterrupt:1;
VOLATILE unsigned ExtInterrupt1:1;
VOLATILE unsigned ExtInterrupt2:1;
VOLATILE unsigned InDoneNoSleep:1;
VOLATILE unsigned ExtFirst:1;
#endif
} Main;

VOLATILE unsigned char OutPacketUnit;
VOLATILE unsigned char RelayToNextUnit;
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

unsigned SendComOneByte:1;

unsigned AddresWasSend:1;
} I2C;

#pragma rambank RAM_BANK_1
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
unsigned char LenI2CRead;
unsigned char I2CReplyCMD;
#ifdef __PIC24H__
rtccTimeDate RtccTimeDate;
rtccTimeDate RtccTimeDateVal;
#endif

//bit ReTransToI2C;
unsigned char eeprom_read(unsigned char addr);
void eeprom_write(unsigned char addr, unsigned char value);



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



//#define DELAY_BTW_NEXT_DIAL 0x0001
//#define DELAY_BTW_NEXT_DIAL 0xff01
#define DELAY_BTW_NEXT_DIAL 0xe001
//#define DELAY_BTW_NEXT_DIAL 0x1001
//#define TIME_FOR_PACKET 0xff24
//#define DELAY_BTW_SEND_PACKET 0xffd3
#define TO_BTW_CHARS 0xff00

#define TIME_FOR_PACKET 0xfef4
#define DELAY_BTW_SEND_PACKET 0xffa3




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
//#include "commc1.h"
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// begin of COPY 1
///////////////////////////////////////////////////////////////////////
//#pragma rambank RAM_BANK_2
//unsigned char CMD[4];
//bit CmdFetch;
//bit ExecCMD;
//bit ReqSendMsg;
//#pragma rambank RAM_BANK_0

//unsigned char CommLinesOld;
#ifdef __PIC24H__
#ifdef HI_TECH_C
#define IF_SSPIF void interrupt _MI2C1Interrupt(void) @ MI2C1_VCTR
#define IF_RCIF void interrupt _U1RXInterrupt(void) @ U1RX_VCTR
#define IF_TXIE void interrupt _U1TXInterrupt(void) @ U1TX_VCTR
#define IF_TIMER0_INT_FLG void interrupt _T1Interrupt(void) @ T1_VCTR
#define IF_TMR1IF void interrupt _T2Interrupt(void) @ T2_VCTR
#ifdef BT_TIMER3
#define IF_TMR3IF void interrupt _T3Interrupt(void) @ T4_VCTR
#endif
#define IF_INT0_FLG void interrupt _INT0Interrupt(void) @ INT0_VCTR
#define IF_INT1IF void interrupt _INT1Interrupt(void) @ INT1_VCTR
#define IF_INT2IF void interrupt _INT2Interrupt(void) @ INT2_VCTR
#else // hitech ends
//#define _ISR __attribute__((interrupt))
//#define _ISRFAST __attribute__((interrupt, shadow))
#define _ISR __attribute__((interrupt))
#define INTERRUPT void __attribute__((interrupt, auto_psv))
#define IF_SSPIF void __attribute__((interrupt, no_auto_psv)) _MI2C1Interrupt(void)
#define IF_RCIF void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void)
#define IF_TXIE void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt(void)
#define IF_TIMER0_INT_FLG void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void)
#define IF_TMR1IF void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void)
#ifdef BT_TIMER3
#define IF_TMR3IF void __attribute__((interrupt, no_auto_psv)) _T4Interrupt(void)
#endif
#define IF_INT0_FLG void __attribute__((interrupt, no_auto_psv)) _INT0Interrupt(void)
#define IF_INT1IF void __attribute__((interrupt, no_auto_psv)) _INT1Interrupt(void)
#define IF_INT2IF void __attribute__((interrupt, no_auto_psv)) _INT2Interrupt(void)
#define IF_RCERRORCOM1 void __attribute__((interrupt, no_auto_psv)) _U1ErrInterrupt(void)

#ifdef USE_COM2
#define IF_RCIFCOM2 void __attribute__((interrupt, no_auto_psv)) _U2RXInterrupt(void)
#define IF_TXIECOM2 void __attribute__((interrupt, no_auto_psv)) _U2TXInterrupt(void)
#define IF_RCERRORCOM2 void __attribute__((interrupt, no_auto_psv)) _U2ErrInterrupt(void)
#endif

#define IF_SLAVEI2C void __attribute__((interrupt, no_auto_psv)) _SI2C1Interrupt(void)

#define IF_TMR5IF void __attribute__((interrupt, no_auto_psv)) _T5Interrupt(void)
#define IF_TMR4IF void __attribute__((interrupt, no_auto_psv)) _T4Interrupt(void)
#define IF_RTCC void __attribute__((interrupt, no_auto_psv)) _RTCCInterrupt(void)

#endif // C30 ends
#else // end of C30 support
#define IF_SSPIF if (SSPIF)
#define IF_BCLIF if (BCLIF)
#define IF_RCIF  if (RCIF)
#define IF_TXIE  if (TXIE)
#define IF_TIMER0_INT_FLG if (TIMER0_INT_FLG)
#define IF_TMR1IF if (TMR1IF)
#ifdef BT_TIMER3
#define IF_TMR3IF if (TMR3IF)
#endif
#define IF_INT0_FLG if (INT0_FLG)
#define IF_INT1IF if (INT1IF)
#ifdef __18CXX
#pragma interrupt int_server
#define INTERRUPT void
#pragma code high_vector=0x08
void interrupt_at_high_vector(void)
{
    _asm
         goto int_server
    _endasm
}

#else // CC5 CC8

#define INTERRUPT interrupt
#pragma codeLevel 1
#pragma optimize 1
 #ifdef _18F2321_18F25K20 
 #else 
#include "inline.h"
 #endif
#pragma origin 4
#pragma rambank RAM_BANK_0
/////////////////////////////////////////////BANK 0//////////////////////
#ifdef _18F2321_18F25K20  // comaptability mode == branch on 0ffset 8 and no priority
#pragma origin 8
#endif

#endif // __18CXX || CC5 || CC8

#endif // end of NOT __C30
#ifndef __PIC24H__
INTERRUPT int_server( void)
{
//    unsigned char CommLines;
//    unsigned char DeltaTime;
//    unsigned char TransferBits;
#ifdef __18CXX
#else
    int_save_registers ;   // W, STATUS (and PCLATH)
   
 #ifdef _18F2321_18F25K20
   unsigned long sv_FSR = FSR_REGISTER;
 #else
   unsigned char sv_FSR = FSR_REGISTER;  // save FSR if required
 #endif
#endif

   unsigned char work1;
   unsigned char work2;
// redefinitions
#define GETCH_BYTE work2
#endif
#ifndef NO_I2C_PROC
   //////////////////////////////////////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////////////////////////////////////
   IF_SSPIF    //if (SSPIF)    // I2C interrupt
   {
#ifdef __PIC24H__
       unsigned int work2;
       register unsigned int W;
#endif
       SSPIF = 0; // clean interrupt
       if (BF)     // receved salve mode byte - buffer is full
       {
#ifdef I2C_INT_SUPPORT
          
          if (I2C_B1.NeedReceive) // needs to receive data
          {
                if (I2C_B1.SendACK_NAK)
                   goto RECV_NEXTBYTE;

               //  work2 = SSPBUF_RX;
               //ACKDT = 0;
               ACKEN = 1;  // send ACK/NACK it will be another SSPIF set at the end of ACK 
                           // but delay in storage will deal with it 
               I2C_B1.SendACK_NAK = 1;
               goto STORE_BYTES;
          }
//          if (I2C_B1.NeedSend) // master need to send + buffer is full - we in wronng place in wrong time
//              goto CHECK_ANOTHER;
#endif
           if (DA_) // 1 = this is byte of data  
           {
               //work2 = SSPBUF;
STORE_BYTES:
               if (AInI2CQu.iQueueSize < BUFFER_LEN)
               {
                   AInI2CQu.Queue[AInI2CQu.iEntry] = SSPBUF_RX;//work2;
                   if (++AInI2CQu.iEntry >= BUFFER_LEN)
                       AInI2CQu.iEntry = 0;
                   AInI2CQu.iQueueSize++;
               }
               else
                   W = SSPBUF_RX; // this byte is skipped to process = no space in a queue
#ifdef I2C_INT_SUPPORT
 //              if (I2C_B1.NeedReceive) // master asked to receive data
 //              {
               if (SSPIF)
               {
                   SSPIF = 0;
                   goto RECV_NEXTBYTE;
               }

               goto MAIN_EXIT;

//WAIT_FOR_SSPIF:
//                   if (!SSPIF)               // must be another interrupt flag set after ACKEN = 1;
//{
//work1 =22;
//                       goto WAIT_FOR_SSPIF;
//}
//                   SSPIF = 0;
//                   //if (!ACKEN)               // must be cleared
//                   //    SSPIF = 0;
//
//                   if (--LenI2CRead)
//                   {
//                       if (LenI2CRead == 1)  // for a last receiving byte prepear NAK
//                           ACKDT = 1;
//                       RCEN = 1; // Receive Enable for a next byte
//                       goto ExitIntr;
//                   }
//                   else
//                   {
//                       work1 =456;
//                       goto ENFORCE_STOP;
//                   } 
//               }
#endif
               //DA_ = 0;
               //bitset(PORTA,2);
           }
           else    // this is a address
           {
               W = SSPBUF_RX;
               I2C.I2CGettingPKG =1;
           }
           //BF = 0;      
           goto MAIN_EXIT;
       }
#ifdef I2C_INT_SUPPORT
       else // for send it is an empty buffer // for receive it is first byte
       {
           if (I2C_B1.NeedReceive) // master needs to receive data == adress was send
           {
RECV_NEXTBYTE:
               //bitset(ddebug,3);
               //if (S)
               //    bitset(ddebug,5); 
               if (P)
               {
               //    //bitset(ddebug,6); 
                   goto ENFORCE_STOP;
               }

               I2C_B1.SendACK_NAK = 0;  // ACK or NAK was not send yet
               if (ACKDT)
                   goto ENFORCE_STOP;

               //I2C_BRG = FAST_DELAY;//0x5;
               RCEN = 1; // Receive Enable
               if (--LenI2CRead == 0)
               {
                   //bitclr(ddebug,1);
                   ACKDT = 1;
               }
               goto MAIN_EXIT;
           }
           if (I2C_B1.NeedSend)
           {
               if (ACKSTAT) // ack was not receved == error!!!!
               {
                   //ddebug++;                   
                   AOutI2CQu.iQueueSize = 0;
                   LenI2CRead = 0;
#ifdef _Q_PROCESS
                   if (AllGyro.GyroDataIs)
                   {
                        //bitset(ddebug,7); 
                        AllGyro.GyroDataIs  = 0;
                   }
#endif
                   //if (I2Caddr == 0x68)
#ifdef USE_INT
                       Main.ExtInterrupt = 1;
#endif
                   //else
                   //    Main.ExtInterrupt1 = 1;

                   goto ENFORCE_STOP;
               }
               if (AOutI2CQu.iQueueSize)
               {
                   SSPBUF_TX = AOutI2CQu.Queue[AOutI2CQu.iExit];
                   if (++AOutI2CQu.iExit >= OUT_BUFFER_LEN)
                       AOutI2CQu.iExit = 0;
                   AOutI2CQu.iQueueSize--;
               }
               else // done I2C transmit
               {
                   I2C_B1.NeedSend = 0;
                   if (LenI2CRead) // will be receive from I2C after write == restart condition
                   {
                       I2C_BRG = SLOW_DELAY;//0x9; // back to speed 
                       RSEN = 1;
                       I2C_B1.NeedRestart = 1;
                       I2C_B1.I2Cread = 1;
                       I2C.I2CGettingPKG =1;
//                       goto CHECK_SSPIF_AGAIN;
                   }
                   else   // done with transmit and that it
                   {

ENFORCE_STOP:
                       I2C_BRG = SLOW_DELAY;//0x9;
                       PEN = 1;
//WAIT_FOR_SSPIF4:
//                   if (!SSPIF)               // must be another interrupt flag set after ACKEN = 1;
//                       goto WAIT_FOR_SSPIF4;

                       I2C_B1.NeedStop = 1;
                       I2C_B1.NeedRestart = 0;
                       I2C_B1.NeedReceive = 0;
                       I2C_B1.NeedSend = 0;
                       I2C_B1.NeedMaster = 0;
//CHECK_SSPIF_AGAIN:
//WAIT_FOR_SSPIF3:
//                   if (!SSPIF)               // must be another interrupt flag set after ACKEN = 1;
//                       goto WAIT_FOR_SSPIF3;
//
//
//                       if (SSPIF)               // if PEN done then SSPIF set
//                       {
//                           SSPIF = 0;
//                           goto CHECK_ANOTHER;
//                       }
                   }
               }
               goto MAIN_EXIT;
           }
       }
#endif
//CHECK_ANOTHER:
       if (S)  // this is a Start condition for I2C
       {
           I2C_B1.I2CBusBusy = 1;
           //S = 0;
           //bitset(PORTA,2);
#ifdef I2C_INT_SUPPORT
           if (BCLIF)
           {
               BCLIF = 0;
               goto MAIN_EXIT;
           }
           if (I2C_B1.NeedRestart)
           {
               I2C_B1.NeedRestart = 0;
               I2C_B1.NeedReceive = 1;
               
//               if (LenI2CRead == 1)
//                   ACKDT = 1;
//               else
               goto SENTI2C_ADDR;
           }
           if (I2C_B1.NeedMaster)
           {
               //I2C.I2CGettingPKG =1;  // blocking - anyway it will be some packege
               I2C_B1.NeedMaster = 0;
               if (AOutI2CQu.iQueueSize)  // in output Que there are something then it will be req Send first
                   I2C_B1.NeedSend = 1;
               else
                   I2C_B1.NeedReceive = 1;
SENTI2C_ADDR:
               I2C_BRG = FAST_DELAY;//0x5;         // twise faster
               work2 = I2Caddr<<1;   // first will be address it is shifted by 1 bit left
               if (I2C_B1.I2Cread)
                   bitset(work2,0);  // thi can be wriye
               SSPBUF_TX = work2;       // send adress byte (tranmit/receive operation - does not matter)
               ACKDT = 0;            // ACK set for all transmission ; on last byte it will be NAK
               goto MAIN_EXIT;
           }
           //work1 =30;

           goto MAIN_EXIT;
#endif
       }
       if (P)  // I2C bus is free
       {
#ifdef I2C_INT_SUPPORT
           if (I2C_B1.NeedStop) // needs to close MASTER mode and go back to SLAVE 
           {
               I2C_B1.NeedStop = 0;
#ifndef __PIC24H__
               BCLIE = 0; // disable collision interrupt
#endif
#ifndef I2C_ONLY_MASTER
               SSPCON1 =0b00011110;
              
               //0        -WCOL: Write Collision Detect bit
               //          1 = An attempt to write the SSPBUF register failed because the SSP module is busy
               //              (must be cleared in software)
               //          0 = No collision
               // 0       -SSPOV: Receive Overflow Indicator bit
               //          1 = A byte is received while the SSPBUF register is still holding the previous byte. SSPOV is
               //              a “don’t care” in Transmit mode. SSPOV must be cleared in software in either mode.
               //          0 = No overflow
               //  1      -SSPEN: Synchronous Serial Port Enable bit
               //          1 = Enables the serial port and configures the SDA and SCL pins as serial port pins
               //          0 = Disables serial port and configures these pins as I/O port pins
               //   1     -CKP: Clock Polarity Select bit.SCK release control
               //          1 = Enable clock
               //          0 = Holds clock low (clock stretch). (Used to ensure data setup time.)
               //    1110-SSPM<3:0>: Synchronous Serial Port Mode Select bits
               //          0110 = I2C Slave mode, 7-bit address
               //          0111 = I2C Slave mode, 10-bit address
               //          1011 = I2C Firmware Controlled Master mode (Slave Idle)
               //          1110 = I2C Slave mode, 7-bit address with Start and Stop bit interrupts enabled
               //          1111 = I2C Slave mode, 10-bit address with Start and Stop bit interrupts enabled
               //          1000 = I2C MASTER mode
               SSPCON2 = 0b00000000;
               SSPADD = UnitADR<<1;  // ready to get something to device
#endif
               // out qu must be cleaned
               AOutI2CQu.iQueueSize = 0;
               AOutI2CQu.iEntry = 0;
               AOutI2CQu.iExit = 0;
               I2C.NextI2CRead = 0;
               I2C_B1.I2CMasterDone = 1;
               //SSPEN = 1;
           }
#else
           I2C_B1.I2CMasterDone = 1;
#endif
           I2C_B1.I2CBusBusy = 0;
           //P = 0;
           I2C.I2CGettingPKG = 0;
           goto MAIN_EXIT;
       }
#ifdef I2C_INT_SUPPORT
       if (SSPOV) // overflow == done - done
       {
           SSPOV = 0;
           W = SSPBUF_RX;
           //work1 =2;
           goto ENFORCE_STOP;
       }
#endif


#ifdef __PIC24H__
MAIN_EXIT:;
#else
       //work1 =33;
       goto MAIN_EXIT;

#endif
    }
#ifndef __PIC24H__
 #ifdef I2C_INT_SUPPORT
   /////////////////////////////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////////////////////////////
   IF_BCLIF //if (BCLIF)
   {
       BCLIF = 0;
       //work1 =1;
       goto ENFORCE_STOP;
   }
 #endif
#endif

#ifdef __PIC24H__
   //////////////////////////////////////////////////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////////////////////////////////////////////////
   IF_SLAVEI2C
   {
       unsigned int work2;
       register unsigned int W;
       SSPIF = 0; // clean interrupt
       if (BF)     // receved salve mode byte - buffer is full
       {
           if (DA_) // 1 = this is byte of data  
           {
               //work2 = SSPBUF;
STORE_BYTES:
               if (AInI2CQu.iQueueSize < BUFFER_LEN)
               {
                   AInI2CQu.Queue[AInI2CQu.iEntry] = SSPBUF_RX;//work2;
                   if (++AInI2CQu.iEntry >= BUFFER_LEN)
                       AInI2CQu.iEntry = 0;
                   AInI2CQu.iQueueSize++;
               }
               else
                   W = SSPBUF_RX; // this byte is skipped to process = no space in a queue
               //DA_ = 0;
               //bitset(PORTA,2);
           }
           else    // this is a address
           {
               W = SSPBUF_RX;
               I2C.I2CGettingPKG =1;
           }
           //BF = 0;
           I2C1CONbits.SCLREL=1;
           //goto MAIN_EXIT;
       }
//CHECK_ANOTHER:
       if (S)  // this is a Start condition for I2C
           I2C_B1.I2CBusBusy = 1;
       if (P)  // I2C bus is free
       {
           I2C_B1.I2CMasterDone = 1;
           I2C_B1.I2CBusBusy = 0;
           I2C.I2CGettingPKG = 0;
       }
MAIN_EXIT:;

   }
#endif
#endif //#ifndef NO_I2C_PROC
#ifdef __PIC24H__
#ifdef USE_COM2
   ///////////////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////////////
   IF_RCIFCOM2 //if (RCIF)
   {

       unsigned int work1;
       unsigned int work2;
       RCIFCOM2 = 0;  // on pic24 needs to clean interrupt manualy
       if (U2STAbits.URXDA)
       {
           while(U2STAbits.URXDA)
           {
               work1 = RCSTACOM2;
               work2 = RCREGCOM2;
               if (AInQuCom2.iQueueSize < BUFFER_LEN)
               {
                   AInQuCom2.Queue[AInQuCom2.iEntry] = work2;
                   if (++AInQuCom2.iEntry >= BUFFER_LEN)
                       AInQuCom2.iEntry = 0;
                   AInQuCom2.iQueueSize++;
               }
           }
        }
   }
   /////////////////////////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////////////////////////
// UART2 errors
   IF_RCERRORCOM2
   {
       unsigned int work1;
       unsigned int work2;

       work1 = RCSTACOM2;
       work2 = RCREGCOM2;
               
       if (bittest(work1,3)) //PERR)
       {
            bitclr(RCSTACOM2,3);
       }
       if (bittest(work1,2)) //FERR)
       {
            bitclr(RCSTACOM2,2);
       }
       if (bittest(work1,1)) //OERR)
       {
            bitclr(RCSTACOM2,1);
       }
   }
   ///////////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////////
   IF_TXIECOM2 //if (TXIE) // expecting interrupts
   {
       unsigned int work1;
       TXIFCOM2 = 0; // on PIC24 needs to clean interrupt manualy
       if (!U2STAbits.UTXBF) // is any space in HW output queue ? (if bit == 0 then it is posible to push another byte)
       {
           if (AOutQuCom2.iQueueSize)
           {
               {
                   TXREGCOM2 = AOutQuCom2.Queue[AOutQuCom2.iExit];
                   if (++AOutQuCom2.iExit >= OUT_BUFFER_LEN)
                       AOutQuCom2.iExit = 0;
                   AOutQuCom2.iQueueSize--;
               }
           }
           else
           {
               if (TRMTCOM2)    // if nothing ina queue and transmit done - then disable interrupt for transmit
               {             // otherwise it will be endless
                    // for speed up output - first bytes already send + at the end needs to send UnitAddr
                   TXIECOM2 = 0;
               }
               else // transmit buffer has something in it (also for pic24 in a bufer there is a data)
               {
                      TXIECOM2 = 0;         // avoid reentry of interrupt
               }
           }
       }
   }
#endif // USE_COM2
#endif //__PIC24H__
   ///////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////
   IF_RCIF //if (RCIF)
   {
#ifdef __PIC24H__
       unsigned int work1;
       unsigned int work2;
#define GETCH_BYTE work2

#endif

       //RCIF = 0;  // cleaned by reading RCREG
#ifdef __PIC24H__
       RCIF = 0;  // on pic24 needs to clean interrupt manualy
       if (U1STAbits.URXDA) // this bit indicat that bytes avalable in FIFO
       {
           while(U1STAbits.URXDA)
           {
#else
       if (RCIE)    // on another pics : ??????
       {
           while(RCIF) // interrupt indicate data avalable in FIFO
           {
#endif

               work1 = RCSTA;
               GETCH_BYTE = RCREG;

#ifdef SYNC_CLOCK_TIMER
               if (!Main.getCMD) // CMD not receved et.
               {
                   if (GETCH_BYTE == MY_UNIT) // record time of received message
                   {
#ifdef __PIC24H__
                       Tmr4Count =TMR4;  // it is possible to count delays from interrupt to recorded time
                       Tmr4CountH = TMR5HLD;
                       Ttilad.Timer = (((unsigned long)Tmr4CountH)<<16) | ((unsigned long)Tmr4Count);
                       Ttilad.Second = Tmr4CountOld;
                       RtccReadTimeDate(&Ttilad.Rtcc);
#else
                       // for BT devicer
#endif
                   }
               }     
#endif // SYNC_CLOCK_TIMER

#ifdef __PIC24H__
               goto NO_RC_ERROR;
#else
               if (bittest(work1,2)) //FERR)
                   goto RC_ERROR;
               if (bittest(work1,1)) //OERR)
                   goto RC_ERROR;
               goto NO_RC_ERROR;
RC_ERROR:
               CREN = 0;
               CREN = 1; 

#endif
RC_24_ERROR:
               Main.getCMD = 0;
               //Main.PrepI2C = 0;
               continue; // can be another bytes
NO_RC_ERROR:
               // optimized version

               if (Main.InComRetransmit)     // prev was escape in STREAM mode or in packet retransmit mode = need retransmit byte as it is
               {
                   Main.InComRetransmit = 0;
                   Main.InComZeroLenMsg = 0;
                   goto RELAY_SYMB;          // ===> jump to retransmit byte
               }
               else if (RelayToNextUnit != 0) // relay packet to next unit, at the same time unit can process adressed stream == 
               {                              // packet to another unit will be retransmit directly without entering queue
                   if (GETCH_BYTE == ESC_SYMB) // inside relay packet ESC char - needs to retransmit as it is and also garanteed to retransmit next byte 
                   {
                       Main.InComZeroLenMsg = 0;
                       Main.InComRetransmit = 1;  //====> retransmit, no jump == retransmit entry
RELAY_SYMB:
                       if (Main.LockToQueue)
                           goto INSERT_TO_COM_Q;

                       if (Main.OutPacket) // serial output busy only way is for any data is via input queue
                       {
                           Main.LockToQueue =1;
                           goto INSERT_TO_COM_Q;
                       } 
                       // exact copy from putchar == it is big!!! but it can not be called recursivly!!
                       ///////////////////////////////////////////////////////////////////////////////////////
                       // direct output to com1
                       ///////////////////////////////////////////////////////////////////////////////////////
                       if (AOutQu.iQueueSize == 0)  // if this is a com and queue is empty then needs to directly send byte(s) 
                       {                            // on 16LH88,16F884,18F2321 = two bytes on pic24 = 4 bytes
                           // at that point Uart interrupt is disabled
                           if (_TRMT)            // indicator that tramsmit shift register is empty (on pic24 it is also mean that buffer is empty too)
                           {
                               TXEN = 1;
                               I2C.SendComOneByte = 0;
                               TXREG = GETCH_BYTE; // this will clean TXIF on 88,884 and 2321
                           }
                           else // case when something has allready send directly
                           {
#ifdef __PIC24H__
                               TXIF = 0; // for pic24 needs to clean uart interrupt in software
                               if (!U1STAbits.UTXBF) // on pic24 this bit is empy when at least there is one space in Tx buffer
                                   TXREG = GETCH_BYTE;   // full up TX buffer to full capacity, also cleans TXIF
                               else
                                   goto SEND_BYTE_TO_QU; // placing simbol into queue will also enable uart interrupt
#else
                               if (!I2C.SendComOneByte)      // one byte was send already 
                               {
                                   TXREG = GETCH_BYTE;           // this will clean TXIF 
                                   I2C.SendComOneByte = 1;
                               }
                               else                     // two bytes was send on 88,884,2321 and up to 4 was send on pic24
                               {
                                   goto SEND_BYTE_TO_QU; // placing simbol into queue will also enable uart interrupt
                               }
#endif
                           }
                       }
                       else
                       {
                           if (AOutQu.iQueueSize < OUT_BUFFER_LEN)
                           {
SEND_BYTE_TO_QU:
                               AOutQu.Queue[AOutQu.iEntry] = GETCH_BYTE; // add bytes to a queue
                               if (++AOutQu.iEntry >= OUT_BUFFER_LEN)
                                   AOutQu.iEntry = 0;
                               AOutQu.iQueueSize++; // this is unar operation == it does not interfere with interrupt service decrement
                               //if (!Main.PrepI2C)      // and allow transmit interrupt
                               TXIE = 1;  // placed simbol will be pushed out of the queue by interrupt
#ifdef RX_FULL
                               if (AOutQu.iQueueSize > (OUT_BUFFER_LEN-CRITICAL_BUF_SIZE))
                               {
                                   Main.PauseOutQueueFull = 1;
                                   RX_FULL = 1;
                               }
#endif
                           } 
                       }
#ifdef TX_NOT_READY
                        // if it is only one unit on PC then it is possible to set bit 
                       // needs inform previous unit to suspend send bytes
                       // that will be done on TX interrupt
                       if (TX_NOT_READY)  // next unit is not ready to accsept the data 
                       {
                           Main.SuspendRetrUnit = 1;
                           RX_FULL = 1;
                       }
#endif

                       goto END_INPUT_COM;
                       /////////////////////////////////////////////////////////////////////////////////////////////
                       //   end of direct output to com1
                       /////////////////////////////////////////////////////////////////////////////////////////////
                   }
                   else if (GETCH_BYTE == RelayToNextUnit) // is it relay message done ??
                   {
                       if (Main.InComZeroLenMsg) // packets with 0 length does not exsists!!!
                           goto RELAY_SYMB; // ===> retransmit
                       RelayToNextUnit = 0;
                       Main.SomePacket = 0;
                       goto RELAY_SYMB; // ===> retransmit
                   }
                   else if (GETCH_BYTE == MY_UNIT)
                   {
                       RelayToNextUnit = 0;
                       goto SET_MY_UNIT;
                   }
                   else if (GETCH_BYTE <= MAX_ADR)
                   {            
                       if (GETCH_BYTE >= MIN_ADR) // msg to relay
                           goto TO_ANOTHER_UNIT;
                   }
                   
                   Main.InComZeroLenMsg = 0;
                   goto RELAY_SYMB; // ===> retransmit
               }
               else if (Main.PacketProcessing)  // case - (not Main.InComRetransmit) && (RelayToNextUnit == 0) && (Main.PacketProcessing)
               {
                   if (Main.CMDToProcessGetESC)
                   {
                      Main.CMDToProcessGetESC = 0;   // =======> process next after escaped byte  => insert into queue
                   }
                   else if (GETCH_BYTE == ESC_SYMB)
                   {
                        Main.CMDToProcessGetESC = 1; // ======> process escape byte => insert into queue
                   }
                   else if (GETCH_BYTE == MY_UNIT)   // ======> received last byte in the message 
                   {
                      Main.PacketProcessing = 0;         // =======> process last byte => insert in queue = but packet done
                   }
                   else if (GETCH_BYTE <= MAX_ADR) // if inside packet present another one == send it to loop
                   {            
                       if (GETCH_BYTE >= MIN_ADR) // packet to relay
                          // retransmit to another unit has a priority - current status freazed
                           goto TO_ANOTHER_UNIT;
                   }         
                   ;  // =======> process message => insert in queue
               }
               else // now == (not Main.InComRetransmit) && (RelayToNextUnit == 0) && (not Main.PacketProcessing) == no any packet yet
               { 
                   // what is that????? if it is not a FLASH request - well assume this case
                   if (!Main.FlashRQ)
                   {
                       if (GETCH_BYTE == ESC_SYMB) // 1. is it ESC ? == next simbol will be relayed also
                       {
                           Main.InComZeroLenMsg = 0;
                           Main.InComRetransmit = 1;    //====> retransmit next also
                           goto RELAY_SYMB; // ===> jump to retransmit
                       }
                       else if (GETCH_BYTE == MY_UNIT) // 2. is this our packet?? (CMD to be proccesed)
                       {
SET_MY_UNIT:
                           Main.PacketProcessing = 1;
                           Main.CMDToProcessGetESC = 0;
                           //Main.CMDProcessLastWasUnitAddr = 1;
                           Main.getCMD =1; // first byte of a packet was eated
                           Main.LastWasUnitAddr = 1;
                           Main.ESCNextByte = 0;
                           goto END_INPUT_COM;
                       }
                       else // 3. is it a packet adressed to another unit (to be retransmitted without processing)
                       {
                           if (GETCH_BYTE <= MAX_ADR) 
                           {            
                               if (GETCH_BYTE >= MIN_ADR) // packet to relay to another untis
                               {
TO_ANOTHER_UNIT:                   Main.SomePacket = 1;     // set everything to relay the packet to another unit
                                   RelayToNextUnit = GETCH_BYTE;
                                   Main.InComZeroLenMsg = 1;
                                   Main.InComRetransmit = 0;
                               }
                           }
                           // last case == anything else will be retransmitted directly to the next unit 
                           // probably this is a garbage - needs to clean it
                           goto END_INPUT_COM;
                           //goto RELAY_SYMB; // ===> jump to retransmit
                       }
                   }
               }    
//////////////////////////////////////////////////////////////////
// end of optimized version
//////////////////////////////////////////////////////////////////
INSERT_TO_COM_Q:
               if (AInQu.iQueueSize < BUFFER_LEN)
               {
                   AInQu.Queue[AInQu.iEntry] = GETCH_BYTE;
                   if (++AInQu.iEntry >= BUFFER_LEN)
                       AInQu.iEntry = 0;
#pragma updateBank 0
                   AInQu.iQueueSize++;
#pragma updateBank 1
               }
               //else
               //    W = RCREG; // this byte is skipped to process
#ifdef RX_FULL
               // if no space in input serial queue then ask previous unit to hold next byte
               // hardware XON/XOFF needs to be enabled for com port on PC
              
               if (AInQu.iQueueSize > (BUFFER_LEN-CRITICAL_BUF_SIZE))
               {
                   Main.InputQueueIsFull = 1;
                   RX_FULL = 1;
               }
               else
                   RX_FULL = 0;
#endif
           }
        }
END_INPUT_COM:;
               //bitclr(PORTA,2);
   }
#ifdef __PIC24H__
    /////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////
// UART errors
   IF_RCERRORCOM1
   {
       unsigned int work1;
       unsigned int work2;

       work1 = RCSTA;
       work2 = RCREG;
       //RCIF = 0;  // on pic24 needs to clean interrupt manualy
       if (bittest(work1,3)) //PERR)
       {
           bitclr(RCSTA,3);
       }
       if (bittest(work1,2)) //FERR)
       {
           bitclr(RCSTA,2);
       }
       if (bittest(work1,1)) //OERR)
       {
            bitclr(RCSTA,1);
       }
   }
#endif

   ////////////////////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////
   IF_TXIE //if (TXIE) // expecting interrupts
   {
#ifdef __PIC24H__
       unsigned int work1;
       TXIF = 0;             // for pic24 needs to clean interrupt flag inside software
       if (!U1STAbits.UTXBF) // is any space in HW output queue ? (if bit == 0 then it is posible to push another byte)
#else
       if (TXIF)        
#endif
       {
#ifdef TX_NOT_READY
           if (TX_NOT_READY)  // next unit is not ready to acsept data 
           {
               // suspend transmit
                Main.SuspendTX = 1;
                goto CLOSE_SEND;
           }
#endif
           if (AOutQu.iQueueSize)
           {
               
               // load to TXREG will clean TXIF
               TXREG = AOutQu.Queue[AOutQu.iExit];
               if (++AOutQu.iExit >= OUT_BUFFER_LEN)
                   AOutQu.iExit = 0;
               AOutQu.iQueueSize--;
               if (Main.PauseOutQueueFull)
               {
                   if (AOutQu.iQueueSize > (OUT_BUFFER_LEN-CRITICAL_BUF_SIZE))
                       ;
                   else
                       Main.PauseOutQueueFull = 0;
               }   
           }
           else
           {
SPEED_SEND:
CLOSE_SEND:
               // if nothing ina queue and transmit done - then disable interrupt for transmit
               // otherwise it will be endless
               // for speed up output - first bytes already send + at the end needs to send UnitAddr
               TXIE = 0;
               // transmit buffer has something in it (also for pic24 in a bufer there is a data)
               // avoid reentry of interrupt
           }
       }
CONTINUE_WITH_ISR:;
   }
    /////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////
    IF_TIMER0_INT_FLG //if (TIMER0_INT_FLG) // can be transfer byte using TMR0
    {
        TIMER0_INT_FLG = 0; // clean timer0 interrupt
        //bitset(TIMER0_CONTROL_REG,5);
#ifndef __PIC24H__
        T0SE = 1;
   #ifdef _18F2321_18F25K20
       TMR0ON = 0;
   #endif
        I2C.Timer0Fired = 1;
        TIMER0_INT_ENBL = 0; // diasable timer0 interrupt
#else  // for __PIC24H__
   #ifdef TEMP_MEASURE
#ifdef _16F88
DELAY_1MKS:
#endif
       if (TEMP_Index > 0) // case when needs to send sequence with a time to TEMP sensor
       {
BEGIN_TERM_OP:           
           switch(TEMP_QUEUE[16 - TEMP_Index])
           {
           case 0xe0:  // init termomenter
               Check();
               if (TEMP_Status == 0) 
               {
                   TEMP_I_O = 0; TEMP_MEASURE = 0; TIMER0_BYTE = (TEMP_MASTER_RST-15); // 480 mks (mesaure==513)
               }
               else
               {
#define VISUAL_TEMP 1
#ifdef VISUAL_TEMP
                   TEMP_MEASURE = 1;
#endif
                   TIMER0_BYTE = (TEMP_MASTER_RST_WAIT-5); TEMP_I_O = 1;  TEMP_Index--;  // 30 mks (measure==64)
               } 
               TEMP_Status++; // 0->1 1 ->2
               break;
           case 0xf0:  // wait from termometer to respond
               Check();
               if (TEMP_MEASURE) // DS1822 responded by lowing bus
               {
                   // no respond from TEMP sensor == skip everything
TMR0_DONE_1:
                   TEMP_Status = 0;
                   TEMP_Index = 0;
                   goto TMR0_DONE;
               }
               else 
               {
                   // temperature ready wait another 480-30 mks
                   TEMP_Index--; TIMER0_BYTE = TEMP_DS1822_WAIT;
                   TEMP_Status =0;
               }
               break;
           case 0xc0: // wait for a temp sensor to finish temparature conversion
               Check();
               if (TEMP_TYPE&0x01) // was timeout
               {
                   if (TEMP_TYPE&0x02) // was request to read 1 bit yet
                   {
                        if (TEMP_TYPE&0x04) // was request
                        {
                             if (TEMP_TYPE&0x08)
                             {
                                 Check();
                                 if (TEMP_TYPE & 0x80) // temp finish temp converion == it is posible to read now
                                 {
                                     
                                     TEMP_Index--;
                                     TEMP_TYPE = 0x00;
                                     goto BEGIN_TERM_OP;
                                 }
                                 else // temp responded with 0 == need to continue pull sensor 
                                 {
                                     TEMP_TYPE = 0x01;
                                     TEMP_I_O = 1;
                                     TIMER0_BYTE = 0;
                                 }
                             }
                             else
                             {
                                 Check();
READ_SAMPLING_2:
                                 TEMP_TYPE |= 0x08;
                                 if (TEMP_MEASURE) // 1 
                                 {
                                     TEMP_TYPE |= 0x80;    
                                     TIMER0_BYTE =TEMP_MASTER_READ_WAIT_1;
                                 }
                                 else
                                     TIMER0_BYTE =TEMP_MASTER_READ_WAIT_0;
                             }
                        }
                        else // TEMP_MASTER_WRITE/READ_DELAY expired
                        {

                             TEMP_TYPE |= 0x04;
#ifdef VISUAL_TEMP
                             //TEMP_MEASURE = 1;
#endif
                             TEMP_I_O = 1;
DELAY_WRITE_DONE1MKS_2:
                             TIMER0_BYTE =(TEMP_MASTER_READ_SAMPLE_WAIT);
#ifdef _16F88
                              while(!TIMER0_INT_FLG)
                              {
                                  //Check();
                              }
                              TIMER0_INT_FLG = 0;
                              goto READ_SAMPLING_2;
#endif

                        }
                   }
                   else // was not send request to read yet
                   {
                       TEMP_TYPE |= 0x02;
                       TEMP_I_O = 0;
                       TEMP_MEASURE = 0;
#ifdef _16F88
                       
                       MASTER_MKS_DELAY;//nop();nop();nop();nop();//nop();nop();nop();nop();
#ifdef VISUAL_TEMP
                       TEMP_MEASURE = 1;
#endif
                       TEMP_I_O = 1;
                       //TEMP_I_O = 1;
                       TEMP_TYPE |= 0x04;
                       goto DELAY_WRITE_DONE1MKS_2;
#else
                       goto START_WRITE_BIT;
#endif 
                  }
               }
               else
               {
                   TEMP_TYPE = 0x01;
                   TIMER0_BYTE = 1;
                   TEMP_I_O = 1;
               }
               break;
           case 0:
               //Check();
               TEMP_I_O = 1;
               TEMP_Status = 0;
               TEMP_Index = 16;
               break;
               //goto TMR0_DONE_1;
           default:
               //Check();
               if (TEMP_Status ==0)
               {
                   
                   TEMP_byte = TEMP_QUEUE[16 - TEMP_Index];
                   if (TEMP_byte & 0x80) // operation write
                   {
                       
                       TEMP_Status = ((unsigned int)(TEMP_byte&0x0f))<<3;
START_WRITE_BYTE:       
                       TEMP_Index--;
                       TEMP_TYPE = 0x01;
START_BOTH:
                       TEMP_byte = TEMP_QUEUE[16 - TEMP_Index];
START_WRITE_BIT:
                       Check();
                       TEMP_I_O = 0;
                       TEMP_MEASURE = 0;
#ifdef _16F88
                       MASTER_MKS_DELAY;
                       if (TEMP_byte&0x01) // write 1
                       {
                           TEMP_TYPE |= 0x02;
#ifdef VISUAL_TEMP
                           TEMP_MEASURE = 1; // 1 mks
#endif
                           TEMP_I_O = 1;  // 1mks
                           goto DEALY_WRITE0_SEND;
                       }
                       else
                       {
                           TIMER0_BYTE =TEMP_MASTER_WRITE_0;
                           TEMP_TYPE |= 0x02;
                       }
                       //goto DELAY_WRITE_DONE1MKS_1;
#else
                       TIMER0_BYTE = TEMP_MASTER_WRITE_DELAY;
#endif
                   }
                   else // operation read
                   {
                       TEMP_ReadIndex = 0;
                       TEMP_Status = ((unsigned int)(TEMP_byte&0x0f))<<3;
START_READ_BYTE:
                       //TEMP_Index--;
                       TEMP_TYPE = 0x00;
#ifdef _16F88
                       TEMP_byte = TEMP_QUEUE[16 - TEMP_Index];
START_READ_BIT:
                       Check();
                       TEMP_I_O = 0;
                       TEMP_MEASURE = 0;

                       goto DELAY_READ_DONE1MKS_1;
#else
                       goto START_BOTH;
#endif
                   }
               }
               else // continue read/write
               {
                   if (TEMP_TYPE & 0x01) // write opearation
                   {
                       if (TEMP_TYPE & 0x02) // 0 or 1 was send
                       {

                           if (TEMP_TYPE & 0x04) // dealy after write slot was send 
                           {
DONE_WRITE_BYTE:
                               TEMP_byte >>=1;
                               if ((--TEMP_Status) & 0x07) // continue with bit
                               {
                                  Check();
                                  TEMP_TYPE = 0x01;
                                  goto START_WRITE_BIT;
                               }
                               else // byte done
                               {
                                   Check();
                                   if (TEMP_Status) // continye with another byte
                                       goto START_WRITE_BYTE;
                                   else // write done
                                   {
                                       TEMP_Index--;
                                       TEMP_TYPE = 0x00;
                                       goto BEGIN_TERM_OP;
                                   }
                               }
           
                           }
                           else // delay after write slot was not send yet (1 mks)
                           {
DEALY_WRITE0_SEND:
#ifdef VISUAL_TEMP
                               TEMP_MEASURE = 1;
#endif
                               TEMP_I_O = 1;
                               TEMP_TYPE |= 0x04;
                               if (TEMP_byte&0x01)
                                   TIMER0_BYTE = TEMP_MASTER_WRITE_1_AFTER; // 60 mks
                               else
#ifdef _16F88
                                   goto DONE_WRITE_BYTE;
#else
                                   TIMER0_BYTE = TEMP_MASTER_WRITE_0_AFTER; // 1 mks
#endif
                           }
                       }
                       else  // TEMP_MASTER_WRITE_DELAY expired : send 0 or 1
                       {
#ifdef _16F88                   
DELAY_WRITE_DONE1MKS_1:
#endif

                           TEMP_TYPE |= 0x02;
                           if (TEMP_byte&0x01) // write 1
                           {
#ifdef _16F88
                               goto DEALY_WRITE0_SEND;
#else
                               TIMER0_BYTE =TEMP_MASTER_WRITE_1; // 1 mks
#endif

                           }
                           else // write 0
                           {
                               //TEMP_I_O = 1;
                               TIMER0_BYTE =TEMP_MASTER_WRITE_0; // 60 mks (real 107)
                           }
                       } 
                   }
                   else // read opeartion
                   {
                       if (TEMP_TYPE & 0x02) // 0 or 1 will be read
                       {
                            if (TEMP_TYPE & 0x04) // delay after sampling was send
                            {
                                if ((--TEMP_Status) & 0x07) // this was a bit from byte
                                {
                                    Check();
                                    TEMP_TYPE = 0;
                                    goto START_READ_BIT;
                                }
                                else
                                {
                                    Check();
                                    TEMP_READ_QUEUE[TEMP_ReadIndex++]= TEMP_byte;
                                    if (TEMP_Status) // not all bytes was reseved yet
                                        goto START_READ_BYTE;
                                    else // done with read
                                    {
                                        Check();
                                        TEMP_Index--;
                                        TEMP_TYPE = 0x00;
                                        TEMP_WORD = (((unsigned long)TEMP_READ_QUEUE[1])<<8) + ((unsigned long)TEMP_READ_QUEUE[0]);
                                        if (TEMP_WORD != TEMP_SENSOR[0])
                                        {
                                            TEMP_SENSOR[0] = (((unsigned long )TEMP_READ_QUEUE[1])<<8) + ((unsigned long)TEMP_READ_QUEUE[0]);
                                            // TBD sprintf(&chTEMP_SENSOR[0],"%02d",TEMP_SENSOR[0]>>4);
                                        }
                                        Check();
#ifdef _16F88
                                        TIMER0_INT_FLG = 0;
                                        TIMER0_INT_ENBL = 0; // diasable timer0 interrupt
                                        
#else
                                        goto BEGIN_TERM_OP;
#endif
                                    }
                                }
                            }
                            else // sampling and delay after sampling
                            {
                                //Check();
READ_SAMPLING:
                                TEMP_byte>>=1;
                                TEMP_TYPE |= 0x04;
                                if (TEMP_MEASURE) // 1 
                                {
                                    TEMP_byte |= 0x80;    
                                    TIMER0_BYTE =TEMP_MASTER_READ_WAIT_1;
                                }
                                else
                                    TIMER0_BYTE =TEMP_MASTER_READ_WAIT_0;
                                
                            }
                        
                       }
                       else // TEMP_MASTER_WRITE/READ_DELAY expired
                       {
                           //Check();
DELAY_READ_DONE1MKS_1:
                          MASTER_MKS_DELAY;
#ifdef VISUAL_TEMP
                           TEMP_MEASURE = 1;
#endif
                           TEMP_I_O = 1;
                           TEMP_TYPE |= 0x02;
                           TIMER0_BYTE =TEMP_MASTER_READ_SAMPLE_WAIT;
#ifdef _16F88
                           while(!TIMER0_INT_FLG)
                           {
                               //Check();
                           }
                           TIMER0_INT_FLG = 0;
                           goto READ_SAMPLING;
#endif
                       }
                   }
               }
               break;
           }
       }
       else // case when it wait some event to signal
       {
TMR0_DONE:
            T0SE = 1;
            DataB0.Timer0Fired = 1;
            TIMER0_INT_ENBL = 0; // diasable timer0 interrupt
       }
       Check();
   #endif
       {
TMR0_DONE:
          TMR0ON = 0;
          I2C.Timer0Fired = 1;
          TIMER0_INT_ENBL = 0; // diasable timer0 interrupt
       }
#endif
    }
//    else 
//NextCheckBit:

    /////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////
    IF_TMR1IF //if (TMR1IF)  // update clock
    {
        TMR1IF = 0;
        //TmrAllConter++; 
#ifdef BT_TIMER1
        if (DataB0.Timer1Meausre)
        {
            Tmr1High++; // timer count and interrupts continue
        }
        else if (DataB0.Timer1Count)
        {
            if ((++Tmr1TOHigh) == 0)
            {
                if (DataB0.Timer1DoTX) // was a request to TX data on that frquency
                {
                    PORT_AMPL.BT_TX = 1;
                    bitset(PORT_BT,Tx_CE);
                    //DataB0.Timer1Count = 0; // switch off req round robin
                    //DataB0.Timer1Meausre = 1; // and set timer measure
                    //TMR1 = 0;
                    //Tmr1High = 0;
                    //goto SWITCH_FQ;
                    DataB0.Timer1DoTX = 0;
                }
                //else
                //{
                    //TIMER1 = Tmr1LoadLow;  
                    TMR1H = (unsigned char)(Tmr1LoadLow>>8);
                    TMR1L = (unsigned char)(Tmr1LoadLow&0xff);
                    //DataB0.Timer1Inturrupt = 1; // and relaod timer
                    Tmr1TOHigh = Tmr1LoadHigh;
//#define SHOW_RX
                    if (++FqTXCount>=3)
                    {
                       FqTXCount = 0;
                       FqTX = Freq1;
#ifdef SHOW_RX_TX
   #ifdef SHOW_RX
   #else
                       DEBUG_LED_ON;
   #endif
#endif

                    }
                    else
                    {
//#ifdef SHOW_RX_TX
//   #ifdef SHOW_RX
//   #else
                      DEBUG_LED_OFF;
//   #endif
//#endif

                        if (FqTXCount == 1)
                            FqTX = Freq2;
                        else
                            FqTX = Freq3;
                    }
                //}
            }
        }
        
#else // BT timer1
        if (++TMR130 == TIMER1_ADJ0)
        {
#ifdef __PIC24H__
            TMR2 += TIMER1_ADJUST;//46272; // needs to be adjusted !!!
            if (TMR2 < TIMER1_ADJUST)//46272)
                goto TMR2_COUNT_DONE;
#else

RE_READ_TMR1:
            work2 = TMR1H;
            work1 = TMR1L;
//          point of time
            if (work2 != TMR1H)    // 4 tick
                goto RE_READ_TMR1;// 
            if (work1 > 149)        // 4 tick
            {
                work1 += 0x6b;        
                work2 += 0x7b;        
                nop();
                nop();              // 8 tick
            }
            else
            {
                work1 += 0x6b;    
                work2 += 0x8f;    // 8 tick
                
            }
            TMR1L = 0;            // 1 tick
            TMR1H = work2;        // 2 tick
            TMR1L = work1;        // 2 tick
            // error will be in 1/30 TMR30 counter
            // needs to make sure from 0-29 is 1130 ticks plus on each TMR130 value
            //   on TMR130 == 30 it is adjust to next sec proper set
#endif
        }
        else if (TMR130 >TIMER1_ADJ0)
        {
            //RtccReadTimeDate(&RtccTimeDateVal);
TMR2_COUNT_DONE:
            TMR130 = 0;
            if (++TMR1SEC > 59)
            {
                TMR1SEC=0;
                if (++TMR1MIN > 59)
                {
                    TMR1MIN = 0;
                    //RTTCCounts = 0;
                    if (++TMR1HOUR > 23)
                    {
                         TMR1HOUR = 0;
                         if (TMR1YEAR & 0x3) // regilar year
                         {
                             if (++TMR1DAY > 365)
                             {
                                 TMR1DAY = 0;
                                 TMR1YEAR++;
                             }
                         }
                         else // leap year
                         {
                             if (++TMR1DAY > 366)
                             {
                                 TMR1DAY = 0;
                                 TMR1YEAR++;
                             } 
                         }
#ifdef USE_LCD
                         sprintf(&LCDDD[0],"%02d",TMR1DAY);
#endif
                    }
#ifdef USE_LCD
                    sprintf(&LCDHH[0],"%02d",TMR1HOUR);
#endif
                }
#ifdef USE_LCD
                sprintf(&LCDMM[0],"%02d",TMR1MIN);
#endif
            }
#ifdef USE_LCD
            sprintf(&LCDSS[0],"%02d",TMR1SEC);
#endif
        }
#endif
    }

#ifdef RTCC_INT
   ///////////////////////////////////////////////////////////////////////////////////////////////
   //////////////////////////////////////////////////Real time clock & counter
   ///////////////////////////////////////////////////////////////////////////////////////////////
   IF_RTCC
   {
       Tmr4Count =TMR4;
       Tmr4CountH = TMR5HLD;

       T4CONbits.TON = 0;
       TMR5HLD = 0;
       TMR4 = 0;
       T4CONbits.TON = 1;
       
       Tmr4CountOld3= Tmr4CountOld2;
       Tmr4CountOld2= Tmr4CountOld;
       Tmr4CountOld= (((unsigned long)Tmr4CountH)<<16) | ((unsigned long)Tmr4Count);

       //RTTCCounts++;
       IFS3bits.RTCIF = 0;        
   }
#endif   

#ifdef BT_TIMER3
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    IF_TMR3IF // RX timer for BT
    {
        TMR3IF = 0;
        
        if (DataB0.Tmr3DoMeausreFq1_Fq2)
        {
            Tmr3High++; // timer count till next interrupt from BT RX on FQ2
        }
        else if (DataB0.Tmr3Run)
        {
            if ((++Tmr3TOHigh) == 0)
            {
                //TIMER3 = Tmr3LoadLow;  
                TMR3H = (unsigned char)(Tmr3LoadLow>>8);
                TMR3L = (unsigned char)(Tmr3LoadLow&0xff);
                Tmr3TOHigh = Tmr3LoadHigh;
                Tmr3LoadLow = Tmr3LoadLowCopy;
                if (SkipPtr)
                {
                   SkipPtr--;
                }
                else
                {
                    DataB0.Tmr3Inturrupt = 1;
                    if (++FqRXCount>=3)
                    {
                        FqRXCount = 0;
                        FqRX = Freq1;
                        if (OutSyncCounter)
                        {
                             // this will produce request to switch off Round-Robin to one FQ listening
                            if (DataB0.AlowSwitchFq1ToFq3)
                               if (--OutSyncCounter == 0)
                               {
                                  DataB0.Timer3OutSyncRQ = 1;
                               }
                        }
#ifdef DEBUG_LED
                       DEBUG_LED_OFF;
   #ifdef DEBUG_LED_CALL_LUNA
                        if (ATCMD & MODE_CONNECT)
                        {
                            if (ESCCount == 0)
                            {
                                if (!BTFlags.BTNeedsTX)
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
                        }
   #endif
#endif

                    }
                    else
                    {
                        if (FqRXCount == 1)
                           FqRX = Freq2;
                        else
                           FqRX = Freq3;
                    }
                }
            }
        }
        
    }
#endif

#ifdef EXT_INT
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    IF_INT0_FLG //if (INT0_FLG)
    {
        INT0_FLG = 0;
        if (INT0_ENBL)
        {
            Main.ExtInterrupt = 1;
            //Main.ExtFirst =1;
            
#ifdef BT_TX
            // communication BT - on interrupt needs goto standby state
            // may be for RX it is owerkill but for TX it is definetly == in TX it should not stay longer
            // TBD: also may be need to switch off transmitter or receiver
            //BTCE_low();  // Chip Enable Activates RX or TX mode (now disable)
            
            if (BTType & 0x01) // it was RX operation
            {
                if (DataB0.Timer3SwitchRX)
                    bitclr(PORT_BT,Tx_CE);	// Chip Enable (Activates RX or TX mode) == now standby

                if (DataB0.Tmr3DoneMeasureFq1Fq2) // receive set
                {
                    SkipPtr++; // set of next frquency will be in CallBackMain
                    AdjustTimer3 = TIMER3;
                    DataB0.Timer3Ready2Sync = 1;
                }
                else // needs to monitor FQ1 and FQ2 receive time
                {
                    if (RXreceiveFQ == 0) // it is receive over Fq1 == need to start timer3 to record time btw Fq1 and FQ2
                    {
                        DataB0.Tmr3DoMeausreFq1_Fq2 = 1;
                        TMR3H = 0;
                        TMR3L = 0;
                        TMR3IF = 0;
                        TMR3IE = 1;
                        Tmr3High  = 0;
                        T3CON = 0b10000001;
                    }
                    else if (RXreceiveFQ == 1) // it was receive over Fq2
                    {
                        if (DataB0.Tmr3DoMeausreFq1_Fq2) // timer for a measure was started ??
                        {
                            TMR3ON = 0;                            // stop timer3 for a moment 
                            Tmr3LoadLowCopy =0xFFFF - TIMER3;      // timer3 interupt reload values 
                            Tmr3LoadLowCopy += 52;                 // ofset from begining of a interrupt routine
                            Tmr3LoadLow = Tmr3LoadLowCopy - MEDIAN_TIME;
                            TMR3H = (Tmr3LoadLow>>8);
                            TMR3L = (unsigned char)(Tmr3LoadLow&0xFF);
                            //TMR3L = 0;//xff;
                            TMR3ON = 1; // continue run
                            Tmr3TOHigh = Tmr3LoadHigh = 0xffff - Tmr3High;
                            DataB0.Tmr3DoMeausreFq1_Fq2 = 0;           // switch in timer3 interrupt routine from "measure time FQ1-FQ2"
                            DataB0.Tmr3Run = 1;               // to "run timer3 on BT RX"
                            DataB0.Tmr3Inturrupt = 0;         // when "measured time FQ1-FQ2" passed it will be timer3 interrupt
                            //SkipPtr =1;
                            DataB0.Tmr3RxFqSwitchLost = 0;
                        }
                    }
                }
                if (!DataB0.Tmr3RxFqSwitchLost)
                    OutSyncCounter = 125; // 2.5 sec no packets == switch for out of sync
            }
            else // TX operation
                bitclr(PORT_BT,Tx_CE);	// Chip Enable (Activates RX or TX mode) == now standby
#endif
        }
    }
 #ifdef _18F2321_18F25K20 
 //#define USE_INT1 1
 #endif
 #ifdef __PIC24H__
 #define USE_INT1 1
 #endif
 #ifdef USE_INT1
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    IF_INT1IF //if (INT1IF)
    {
        INT1IF = 0;
        if (INT1IE)
        {
            Main.ExtInterrupt1 = 1;
            //Main.ExtFirst =0;
        }
    }
  #ifdef __PIC24H__
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    IF_INT2IF //if (INT1IF)
    {
        INT2IF = 0;
        if (INT2IE)
        {
            Main.ExtInterrupt2 = 1;
            //Main.ExtFirst =0;
        }
    }
  #endif // only for PIC24
 #endif // USE_INT1
#endif  // EXT_INT
//    else 
//Next2CheckBit:
//    if (RBIF)
//    {
////#pragma updateBank 0
//    }
#ifndef __PIC24H__
ExitIntr:
/*
    if (TimerB1.SetSleep)
    {
        TimerB1.SetSleep = 0;
        GIE = 0;
    }
*/
MAIN_EXIT:

 #ifdef __18CXX
 #else
    
    FSR_REGISTER = sv_FSR;       // restore FSR if saved
    int_restore_registers // W, STATUS (and PCLATH)
 #endif
}
#endif // not __PIC24H__
//#define ONEBIT_TMR0_LEN 0x10
// temp vars will be on bank 1 together with I2C queue
#pragma rambank RAM_BANK_1
////////////////////////////////////BANK 1////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//      END COPY1
/////////////////////////////////////////////////////////////////



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
//#include "commc6.h"
////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// begin COPY 6
///////////////////////////////////////////////////////////////////////   

        AInQu.iEntry = 0;
        AInQu.iExit = 0;
		AInQu.iQueueSize = 0;

		AOutQu.iEntry = 0;
        AOutQu.iExit = 0;
		AOutQu.iQueueSize = 0;
#ifndef NO_I2C_PROC
        AInI2CQu.iEntry = 0;
        AInI2CQu.iExit = 0;
		AInI2CQu.iQueueSize = 0;

		AOutI2CQu.iEntry = 0;
        AOutI2CQu.iExit = 0;
		AOutI2CQu.iQueueSize = 0;
#endif
        //TimerB1=0;
#ifdef BT_TIMER1
#else
		TimerB1.SetSleep = 0;
        TimerB1.SetSyncTime = 0;
#endif

        //Main = 0;
        Main.getCMD = 0;
        Main.ESCNextByte = 0;
        Main.PacketProcessing = 0;
        Main.CMDToProcessGetESC = 0;

        Main.PrepI2C = 0;
        Main.DoneWithCMD = 1;

        Main.prepStream = 1;
        Main.prepCmd = 0;
        Main.InComRetransmit = 0;
        Main.InComZeroLenMsg = 0;

        Main.SomePacket = 0;
        Main.OutPacket = 0;
        Main.OutPacketESC = 0;
        Main.OutPacketZeroLen = 0;
        Main.LockToQueue = 0;

        Main.RetransmitTo = 0;
#ifdef NON_STANDART_MODEM
        Main.SendOverLink = 0;
        Main.SendOverLinkAndProc = 0;
        Main.FlashRQ = 0;
#endif
        //Main.SendWithEsc = 0;
        //Main.CommLoopOK = 0;
        Main.LastWasUnitAddr = 0;

        SSPADD = UnitADR<<1;
        I2C.LockToI2C = 0;
        I2C.WaitQuToEmp = 0;
        I2C.SetI2CYesNo = 0;
        I2C.EchoWhenI2C = 1;

        I2C_B1.I2CBusBusy = 0;
        //BlockComm = 0;

        I2C.Timer0Fired = 0;
#ifdef I2C_INT_SUPPORT
		I2C_B1.NeedMaster = 0;
        I2C_B1.NeedRestart = 0;
        I2C_B1.NeedStop = 0;
        I2C_B1.NeedReceive = 0;
        I2C_B1.NeedSend = 0;
        //NeedReciveACK = 0;


#endif
#ifdef TX_NOT_READY
        Main.SuspendTX = 0;
        Main.SuspendRetrUnit = 0;
        Main.InputQueueIsFull = 0;
        Main.PauseOutQueueFull = 0;
#endif
        I2C_B1.I2CMasterDone = 1;
        RelayToNextUnit = 0;
        AllowMask = 0xff;
        UnitMask1 = 0xff;
        UnitMask2 = 0;
        UnitFrom = 0;
#ifdef SYNC_CLOCK_TIMER
#ifdef __PIC24H__
        FSR_REGISTER = &Ttilad;
        for (bWork = 0; bWork < 5*sizeof(Ttilad);bWork++)
        {
             PTR_FSR = 0;
        }
#else  // not __PIC24H__
#endif // __PIC24H__
#endif // SYNC_CLOCK_TIMER
        DataB3.FlashWas1byteWrite = 0;

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// end COPY 6


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
    //bitset(PORTA,4);
    //bitset(PORTA,3);
    //bitset(SSPCON,4);  // set clock high;
//#include "commc7.h"
////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// begin COPY 7
///////////////////////////////////////////////////////////////////////   
// 3 cases 
//    NO PACKET - some bytes not adressed to anybody - all NO PACKET bytes supressed by interrupt processing routine
//    <u>packet<u> - packet adressed to anybody, but not to current unit
//    <U>packet<U> - packet adressed to current unit, must be processed inside unit
//     u= from 0 to 9
//  escape char is #
// on <LongCMD> needs to clean DoneWithCMD = 0 and at the end of the command
// 
// stream from com:
//       <Unit>                           -> getCMD = 1;
//              COMMANDS                  -> processed as getCMD
//                       <Unit>           -> getCMD = 0;
// stream from I2C: 
//    received data as slave device or receve responce from different I2C device (master read request) 
//    processed as a COMMANDS 
//    I2C_STREAM... == COMMANDS...
// 
//   

    while(1)
    {
#ifdef TX_NOT_READY
        if (Main.SuspendTX) // was suspend of a transmit because next unit was not ready to acsept data
        {
            if (!TX_NOT_READY)  // next unit is READY (low) to accsept data
            {
                if (AOutQu.iQueueSize)
                {
                    TXREG = AOutQu.Queue[AOutQu.iExit];
                    if (++AOutQu.iExit >= OUT_BUFFER_LEN)
                        AOutQu.iExit = 0;
                    AOutQu.iQueueSize--;
                }
                Main.SuspendTX = 0;
                if (Main.SuspendRetrUnit)
                {
                    RX_FULL = 0;
                    Main.SuspendRetrUnit = 0;
                }    
            } 
        }
#endif
        if (CallBkMain() == 0) // return 0 = do continue; 1 = process queues
            continue;
#ifndef NO_I2C_PROC
        if (AInI2CQu.iQueueSize) // if something comes from I2C (slave received or some I2c device responded on read command)
        {
            if (CallBkI2C())// 0 = do not process byte; 1 = process;
            {
                bitclr(bWork,0);
                if (Main.getCMD)
                    bitset(bWork,0);
                Main.getCMD = 1;
                ProcessCMD(getchI2C());
                Main.getCMD = 0;
                if (bittest(bWork,0))
                    Main.getCMD = 1;

            }
        }
#endif // #ifndef NO_I2C_PROC
        if (AInQu.iQueueSize)      // in comm queue bytes
        {
            if (CallBkComm()) // return 0 = do not process byte; 1 = process;
            {
PROCESS_IN_CMD:
                 ProcessCMD(getch());
#ifdef RX_FULL
               if (Main.InputQueueIsFull)
               {
                   if (AInQu.iQueueSize > (BUFFER_LEN-CRITICAL_BUF_SIZE))
                       ;//RX_FULL = 1; // that is already set in interrupt routine
                   else
                   {                   // but if queue is out cleaning - allow to RX bytes.
                       RX_FULL = 0;
                       Main.InputQueueIsFull = 0;
                   }
               }
#endif

NO_PROCESS_IN_CMD:;
            }
        }
        else  // nothing in comm input queue ==  can sleep till interrupt
        {
#ifdef NON_STANDART_MODEM
            if (FlashEntry == FlashExit)
            {
                if (FlashEntryBH == FlashExitBH)
                    Main.FlashRQ = 0;
                else
                {
      
NEEDS_FLASH_PROC:   Main.FlashRQ = 1;
                    if (!Main.getCMD) // only if it is not a command mode
                    {
                        if (RelayToNextUnit==0) // can be write to com -> data can be sent
                        {
                            // now it is possible to process data from flash
                            Main.getCMD = 1;
                            CS_HIGH;
                            //ReadPrevLen;
                            //ReadNextLen;
                            //ReadTypePkt;

                            Main.getCMD = 0;
                            Main.FlashRQ = 0;
                        }
                    } 
                }
            }
            else
                goto NEEDS_FLASH_PROC;
               
                
#endif
#ifndef NO_I2C_PROC
            if (AInI2CQu.iQueueSize == 0)
#endif
            {
                //if (I2C_B1.I2CMasterDone) // no communication over I2C
#ifdef __PIC24H__
                CLKDIVbits.DOZEN = 1; // switch clock from 40MOP=>1.25MOP
#else
#endif
            }
        }
   }
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// end COPY 7
///////////////////////////////////////////////////////////////////////


} // at the end will be Sleep which then continue to main







// for pic18f2321
#define SPBRG_SPEED SPBRG_57600_64MHZ
//#define SPBRG_SPEED SPBRG_38400_32MHZ
//#define SPBRG_SPEED SPBRG_19200_32MHZ
// for pic24hj64gp502
//#define SPBRG_SPEED SPBRG_57600_40MIPS

//#include "commc2.h"
/////////////////////////////////////////////////////////////////
//      Begin COPY 2
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
#ifdef SSPORT
void SendSSByte(unsigned char bByte);
unsigned char GetSSByte(void);
void SendSSByteFAST(unsigned char bByte); // for a values <= 3
#endif


void enable_uart(void);
#ifndef NO_I2C_PROC
void enable_I2C(void);
#endif
void EnableTMR1(void);
void putch(unsigned char simbol)
{
    while(Main.SomePacket) // wait to output to be clean == no packet in serial output
    ;
    // monitor output packets and set Main.OutPacket accordinly
    if (Main.OutPacketESC)
        Main.OutPacketESC =0;
    else if (simbol == ESC_SYMB)
        Main.OutPacketESC = 1;
    else 
    {
        if (Main.OutPacket)
        {
            if (simbol == OutPacketUnit)
            {
                if (!Main.OutPacketZeroLen)
                    Main.OutPacket = 0;
            }
            else
                Main.OutPacketZeroLen = 0;
        }
        else
        {
            if (simbol <= MAX_ADR)
            {
                if (simbol  >= MIN_ADR)
                {
                    Main.OutPacket = 1;
                    Main.OutPacketZeroLen = 1;
                    OutPacketUnit = simbol; 
                }
            }
        }
    }
    if (AOutQu.iQueueSize == 0)  // if this is a com and queue is empty then needs to directly send byte(s) 
    {                            // on 16LH88,16F884,18F2321 = two bytes on pic24 = 4 bytes
#ifdef TX_NOT_READY
CHECK_NEXT_UNIT:        
        if (TX_NOT_READY)  // next unit is not ready to acsept data 
        {
             // TBD: probably requare TimeOut???
             goto CHECK_NEXT_UNIT; 
        }
#endif
        // at that point Uart interrupt is disabled
#ifdef __PIC24H__
        if (!U1STAbits.UTXBF) // on pic24 this bit is empy when at least there is one space in Tx buffer
        {
            TXIF = 0; // for pic24 needs to clean uart interrupt in software
            TXEN = 1; // just in case ENABLE transmission
            TXREG = simbol;   // full up TX buffer to full capacity, also cleans TXIF
        }
        else
        {
            goto SEND_BYTE_TO_QU; // placing simbol into queue will also enable uart interrupt
        }
#else
        if (_TRMT)            // indicator that tramsmit shift register is empty (on pic24 it is also mean that buffer is empty too)
        {
            TXEN = 1;
            I2C.SendComOneByte = 0;
            TXREG = simbol; // this will clean TXIF on 88,884 and 2321
        }
        else // case when something has allready send directly
        {

            if (!I2C.SendComOneByte)      // one byte was send already 
            {
                TXREG = simbol;           // this will clean TXIF 
                I2C.SendComOneByte = 1;
            }
            else                     // two bytes was send on 88,884,2321 and up to 4 was send on pic24
                goto SEND_BYTE_TO_QU; // placing simbol into queue will also enable uart interrupt
        }
#endif
    }
    else
    {
        if (AOutQu.iQueueSize < OUT_BUFFER_LEN)
        {
SEND_BYTE_TO_QU:
            AOutQu.Queue[AOutQu.iEntry] = simbol; // add bytes to a queue
            if (++AOutQu.iEntry >= OUT_BUFFER_LEN)
                AOutQu.iEntry = 0;
            AOutQu.iQueueSize++; // this is unar operation == it does not interfere with interrupt service decrement
            //if (!Main.PrepI2C)      // and allow transmit interrupt
            TXIE = 1;  // placed simol will be pushed out of the queue by interrupt
        } 
    }
}

#ifdef USE_COM2
#ifdef __PIC24H__
void putchCom2(unsigned char simbol)
{
    if (AOutQuCom2.iQueueSize == 0)  // if this is a com and queue is empty then needs to directly send byte(s) 
    {                            // on 16LH88,16F884,18F2321 = two bytes on pic24 = 4 bytes
        // at that point Uart interrupt is disabled
        if (!U2STAbits.UTXBF) // on pic24 this bit is empy when at least there is one space in Tx buffer
        {
            TXENCOM2 = 1; // just in case ENABLE transmission
            //TXIF = 0; // for pic24 needs to clean uart interrupt in software
            TXREGCOM2 = simbol;   // full up TX buffer to full capacity, also cleans TXIF
        }
        else
        {
            goto SEND_BYTE_TO_QU; // placing simbol into queue will also enable uart interrupt
        }
    }
    else
    {
        if (AOutQuCom2.iQueueSize < OUT_BUFFER_LEN)
        {
SEND_BYTE_TO_QU:
            AOutQuCom2.Queue[AOutQuCom2.iEntry] = simbol; // add bytes to a queue
            if (++AOutQuCom2.iEntry >= OUT_BUFFER_LEN)
                AOutQuCom2.iEntry = 0;
            AOutQuCom2.iQueueSize++; // this is unar operation == it does not interfere with interrupt service decrement
            //if (!Main.PrepI2C)      // and allow transmit interrupt
            TXIECOM2 = 1;  // placed simbol will be pushed out of the queue by interrupt
        } 
    }
}

void putchCom2WithESC(unsigned char simbol)
{
    if (Main.SendCom2WithEsc)
    {
WAIT_SPACE_Q:
        if (AOutQuCom2.iQueueSize >= (OUT_BUFFER_LEN-3)) // is enought space to output ??
            goto WAIT_SPACE_Q;
        if (simbol == ESC_SYMB)
           goto PUT_ESC;

        if (simbol >= MIN_ADR)
        {
            if (simbol <= MAX_ADR)
            {
PUT_ESC:
                putchCom2(ESC_SYMB);
            }
        }
    }
    putchCom2(simbol);
}
void PutsCom2(const char * s)
{
	while(*s)
	{
        if (Main.SendCom2WithEsc)
        {
            if (*s >= MIN_ADR)
                if (*s <= MAX_ADR)
                    putchCom2(ESC_SYMB);
        } 
		putchCom2(*s);
		s++;
	}
}
unsigned char getchCom2(void)
{
    unsigned char bRet = AInQuCom2.Queue[AInQuCom2.iExit];
    if (++AInQuCom2.iExit >= BUFFER_LEN)
        AInQuCom2.iExit = 0;
    AInQuCom2.iQueueSize --;
    return bRet;
}


#endif // __PIC24H_
#endif // USE_COM2
#ifndef NO_I2C_PROC
void putchI2C(unsigned char simbol)
{
    AOutI2CQu.Queue[AOutI2CQu.iEntry] = simbol; // add bytes to a queue
    if (++AOutI2CQu.iEntry >= OUT_BUFFER_LEN)
        AOutI2CQu.iEntry = 0;
    AOutI2CQu.iQueueSize++;
}
#endif
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
void Puts(const char * s)
{
	while(*s)
	{
        if (Main.SendWithEsc)
        {
            if (*s >= MIN_ADR)
                if (*s <= MAX_ADR)
                    putch(ESC_SYMB);
        } 
		putch(*s);
		s++;
	}
}
#pragma rambank RAM_BANK_0
////////////////////////////////////////////BANK 0//////////////////////////

unsigned char getch(void)
{
    unsigned char bRet = AInQu.Queue[AInQu.iExit];
    if (++AInQu.iExit >= BUFFER_LEN)
        AInQu.iExit = 0;
#pragma updateBank 0
    AInQu.iQueueSize --;
    return bRet;
}
#pragma updateBank 1

/*
unsigned char getch(void)
{
    unsigned char bRet = InQu[iPtr2InQu];
    if (++iPtr2InQu >= BUFFER_LEN)
        iPtr2InQu = 0;
    iInQuSize --;
    return bRet;
}*/
#ifndef NO_I2C_PROC
unsigned char getchI2C(void)
{
    unsigned char bRet = AInI2CQu.Queue[AInI2CQu.iExit];
    if (++AInI2CQu.iExit >= BUFFER_LEN)
        AInI2CQu.iExit = 0;
    AInI2CQu.iQueueSize --;
    return bRet;
}
void InsertI2C(unsigned char bWork)
{
    if (AInI2CQu.iQueueSize < BUFFER_LEN)
    {
        AInI2CQu.Queue[AInI2CQu.iEntry] = bWork;
        if (++AInI2CQu.iEntry >= BUFFER_LEN)
            AInI2CQu.iEntry = 0;
        AInI2CQu.iQueueSize++;
    }
//    else
//    {
//bWork = 0;
//    } 
}
#endif // #ifndef NO_I2C_PROC
#pragma rambank RAM_BANK_1
//////////////////////////////////////////////BANK 1///////////////////////////


unsigned char eeprom_read(unsigned char addr);
void eeprom_write(unsigned char addr, unsigned char value);
 #ifndef NO_I2C_PROC
/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////
unsigned char InitI2cMaster(void)
{
#ifdef I2C_INT_SUPPORT
WAIT_STOP:
//      if (!P)
//      {
//         if (!S)
//             goto FIRST_I2C;
//         goto WAIT_STOP;
//      }
FIRST_I2C:
      //bitset(I2CTRIS,I2C_SDA);  // SDA=1
      //bitset(I2CTRIS,I2C_SCL);  // SCL=1
      I2C_B1.I2CMasterDone = 0;
      
#ifndef I2C_ONLY_MASTER
   SSPCON2 =0b00000000;
   SSPCON1 &=0b11111000; // for PIC18F2321 only set master 1000
#endif
      //SSPCON1 =0b00011000;
      // I2C_BRG == SSPADD on 88, 884, 2321
      I2C_BRG = SLOW_DELAY;//0x9;//12; // 400 kHz formula:
                     // 400000 = 32000000/(4*(SSPADD +1))
                     // 4*(SSPADD + 1) = 32000000/400000 = 80
                     // SSPADD + 1 = 80/4 = 20
                     // SSPADD = 20-1 = 19 = 0x13
                     // or for 100kHz
                     // SSPADD + 1 = 32000000/100000/4 = 80
                     // SSPADD = 79 = 0x4F
      //SMP = 0; // 1 for < 400kHz 0 == 400kHz
#ifndef __PIC24H__
      CKE = 0;
#endif
      WCOL = 0;
      SSPOV = 0;
      BCLIF = 0;
      I2C_B1.NeedMaster = 1;

      //SSPEN = 1; // enable I2C
      //BCLIE = 1; // enable collision interrupt
      SEN = 1;   // Start condition Enable      
      return 1;
#else

I2C_IWAIT_READ:
     if (I2C.NextI2CRead) // this will be restart condition
         goto FROM_RESTART;
     if (I2C_B1.I2CBusBusy) // needs to wait when I2C will be not busy
         goto I2C_IWAIT_READ;
      I2C_B1.I2CMasterDone = 0;
FROM_RESTART:
//#pragma updateBank 0
     bitclr(I2CTRIS,I2C_SDA);  // SDA=0 SCL=1
     DELAY_START_I2C;
     bitclr(I2CTRIS,I2C_SCL);  // SDA=0 SCL=0
     DELAY_1_I2C;      // TBD - different delay for different units for arbitration
     bitset(I2CTRIS,I2C_SDA);  // SDA up for a little bit to check that bus not busy
//#pragma updateBank 1
     DELAY_1_I2C;
     if (bittest(I2CPORT,I2C_SDA))  // that we succesfully garbed I2C bus
     {
         bitclr(I2CTRIS,I2C_SDA);  // bus is ours
         return 0;
     }
     bitset(I2CTRIS,I2C_SCL);  // release I2C bus immiduatly : SDA=1 SCL=1
     return 1;
#endif
}
/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////
#ifdef I2C_INT_SUPPORT
#else
unsigned char sendI2C()
{
    unsigned char bWork;
    unsigned char bCount;
    bWork = I2Caddr<<1;   // first will be address it is shifted by 1 bit left
    if (I2C_B1.I2Cread)
        bitset(bWork,0);  // thi can be wriye
    FSR_REGISTER = &AOutI2CQu.Queue[AOutI2CQu.iExit]; // the goes bytes of data
    goto SEND_I_ADDR;
    
    while(AOutI2CQu.iQueueSize)
    {
        AOutI2CQu.iQueueSize--;
        bWork = PTR_FSR;
        FSR_REGISTER++;

        //bWork = AOutI2CQu.Queue[AOutI2CQu.iExit]; // the goes bytes of data
            
        //if (++AOutI2CQu.iExit >= OUT_BUFFER_LEN)
        //    AOutI2CQu.iExit = 0;
//#pragma updateBank 0
        //AOutI2CQu.iQueueSize--;
//#pragma updateBank 1

SEND_I_ADDR:
///////////////////////////////////////////////////////////////////////////
//                                            optimozation part of a send
//////////////////////////////////////////////////////////////////////////
#ifdef _OPTIMIZED_
//#pragma updateBank 0
        bCount = 9;
        Carry = 0;
        goto SEND_I_FIRST_BIT;

        // on entry SDA = 0 SCL = 0
        while (--bCount)
        {
            //DELAY_1_I2C; // this dealy is not nessesary - loop already does delay
            bitset(I2CTRIS,I2C_SCL);     // SDA = X SCL = 1 set SCL high
            DELAY_1_I2C;
            bitclr(I2CTRIS,I2C_SCL);     // SDA = X SCL = 0  set SCL low
SEND_I_FIRST_BIT:
#ifdef      _18F2321_18F25K20
            #asm
              RLCF bWork,1,1
            #endasm
#else
            RLF(bWork,1);
#endif
            if (Carry)
                bitset(I2CTRIS,I2C_SDA); //SDA = 1 SCL = 0 set SDA High
            else
                bitclr(I2CTRIS,I2C_SDA); //SDA = 0 SCL = 0 set SDA Low
        }
        // carry rotate full circle and returned back == 0
        // on exit SDA = 0 SCL = 0

        // bWork now is the same as at the begining
        // wait for ACK
        //
        //bitclr(I2CTRIS,I2C_SDA);         //SDA = 0 SCL = 0 set SDA low - this was done on last loop
        //DELAY_1_I2C;
        // check:
        //bitset(I2CTRIS,I2C_SDA);        //  now SDA = 1 (looks like high) with SCL = 0 prepear to read input value from receiver

        bitset(I2CTRIS,I2C_SCL);        // SDA = 0 SCL = 1 set SCL high to read ACK (or NACK)
        bitset(I2CTRIS,I2C_SDA);        //  now SDA = 1 (looks like high) with SCL = 1 prepear to read input value from receiver
        
#pragma updateBank 1
        
        if (bittest(I2CPORT,I2C_SDA))   //     if bit set then it is NAK - TBD then transfered byte was not acsepted
            Carry = 1;
        bitclr(I2CTRIS,I2C_SDA);        //SDA = 0 SCL = 1 set SDA low for next step 
        DELAY_1_I2C;
        bitclr(I2CTRIS,I2C_SCL);     // SDA = 0 SCL = 0 and ready to go with next byte
#ifdef _NOT_SIMULATOR
        if (Carry)           // if NAK was then done with communication
        {
            bWork = 1;
            //break;
            goto CLEAN_I2C_OUT_QUEUE;//return;  
        }
#endif            

#else
        ///////////////////////////////////////////////////////////////////////////
        //                              non optimization part of a send
        ///////////////////////////////////////////////////////////////////////////
        bCount = 9;  // stupid but will be smaller code
#pragma updateBank 0
        // on entry SDA = 0 SCL = 0
        while (--bCount)
        {
            // on each itereation SDA = X SCL = 0
            if (bittest(bWork,7))
                bitset(I2CTRIS,I2C_SDA); //SDA = 1 SCL = 0 set SDA High
            else
                bitclr(I2CTRIS,I2C_SDA); //SDA = 0 SCL = 0 set SDA Low
            //DELAY_1_I2C; // this dealy is not nessesary - loop already does delay
            bitset(I2CTRIS,I2C_SCL);     // SDA = X SCL = 1 set SCL high
            DELAY_1_I2C;
            bitclr(I2CTRIS,I2C_SCL);     // SDA = X SCL = 0  set SCL low
            bWork <<=1;
            
        }

        // on exit SDA = X SCL = 0

        // bWork now is 0
        // wait for ACK
        //
        bitclr(I2CTRIS,I2C_SDA);         //SDA = 0 SCL = 0 set SDA low
        DELAY_1_I2C;
        // check:
        bitset(I2CTRIS,I2C_SCL);        // SDA = 0 SCL = 1 set SCL high to read ACK (or NACK)
CLOCK_STRETCH:
        if (!bittest(I2CPORT,I2C_SCL))   //  SCL must be release by recepient
        goto CLOCK_STRETCH;

        bitset(I2CTRIS,I2C_SDA);        //  now SDA = 1 (looks like high) with SCL = 0 prepear to read input value from receiver

#pragma updateBank 1
        if (bittest(I2CPORT,I2C_SDA))   //     if bit set then it is NAK - TBD then needs to repeat byte
            ++bWork;//bWork = 1;
        bitclr(I2CTRIS,I2C_SDA);        //SDA = 0 SCL = 1 set SDA low for next step 
        //DELAY_1_I2C;
        bitclr(I2CTRIS,I2C_SCL);     // SDA = 0 SCL = 0 and ready to go with next byte
#ifdef _NOT_SIMULATOR
        if (bittest(bWork,0))           // if NAK was then done with communication
        {
            break;
            //if (!I2C_B1.I2Cread)
                //bitset(PORTA,4);
            //goto CLEAN_I2C_OUT_QUEUE;//return;  
        }
        //else
        //{
        //    //if (!I2C_B1.I2Cread)
        //        //bitclr(PORTA,4);
        //}
#endif            

#endif  // NOT OPTIMIZED VERSION
    }
CLEAN_I2C_OUT_QUEUE:
    AOutI2CQu.iQueueSize = 0;
    AOutI2CQu.iEntry = 0;
    AOutI2CQu.iExit = 0;
    return bWork;
}
/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////
void receiveI2C()
{
    unsigned char bWork;
    unsigned char bCount;
    // on entry SDA = 0 SCL = 0

///////////////////////////////////////////////////////////////////////////////
//                                          optimization part of receive
//////////////////////////////////////////////////////////////////////////////
#ifdef _OPTIMIZED_
    //GIE =0;
    bitset(I2CTRIS,I2C_SDA);     // SDA = 1 (input) SCL = 0
    DELAY_1_I2C;
    while(LenI2CRead)
    {
        bitset(I2CTRIS,I2C_SCL);     // SDA = 1(input) SCL = 1 
        bWork = 0;
RELESE_CHECK:
        if (!bittest(I2CPORT,I2C_SCL))  // check when SCL will be relesed TBD - can be endless loop
            goto RELESE_CHECK;
                                 //SDA = 1(INPUT) SCL = 1 now ready to read
        bCount = 9;  // stupid but will be smaller code
//#pragma updateBank 0
        //DELAY_1_I2C;            
        //goto READ_I_BEGIN;
//#pragma updateBank 0
        
        while (--bCount)
        {
            bitset(I2CTRIS,I2C_SCL);     // SDA = ? SCL = 1 
//#pragma updateBank 1
READ_I_BEGIN:
            //DELAY_1_I2C;            
            Carry = 0;
            if (bittest(I2CPORT,I2C_SDA))
                Carry = 1;
#ifdef      _18F2321_18F25K20
            #asm
              RLCF bWork,1,1
            #endasm
#else
            RLF(bWork,1);
#endif
            bitclr(I2CTRIS,I2C_SCL);     // SDA = ? SCL = 0
            //DELAY_1_I2C;
        }
        // send ACK (or NAK on last read byte
                                 // SDA = ? SCL = 0
        if (LenI2CRead == 1)
            bitset(I2CTRIS,I2C_SDA);     // SDA = 1 SCL = 0 NAK
        else
            bitclr(I2CTRIS,I2C_SDA);     // SDA = 0 SCL = 0  ACK
//DELAY_1_I2C;
        bitset(I2CTRIS,I2C_SCL);        //SDA = ACK or NAK SCL = 1 -> send
        DELAY_1_I2C;
        bitclr(I2CTRIS,I2C_SCL);        // SDA = ACK or NAK SCL = 0
                                        // delay will be all following code:
        LenI2CRead--;
#ifdef _NOT_SIMULATOR
#else
        bWork = '*';
#endif
        InsertI2C(bWork);
        bitset(I2CTRIS,I2C_SDA);     // SDA = 1 (input) SCL = 0
    } // to next loop it is SDA = 0 SCL = 0
    bitclr(I2CTRIS,I2C_SDA);     // SDA = 0 SCL = 0
#else         // START NON OPTIMIZED VERSION
    //////////////////////////////////////////////////////////////////////////////////
    //                                   non optimization part of receive
    //////////////////////////////////////////////////////////////////////////////////
    //GIE =0;
    bitset(I2CTRIS,I2C_SDA);     // SDA = 1 (input) SCL = 0
    DELAY_1_I2C;
    while(LenI2CRead)
    {
        bitset(I2CTRIS,I2C_SCL);     // SDA = 1(input) SCL = 1 
        bWork = 0;
RELESE_CHECK:
        if (!bittest(I2CPORT,I2C_SCL))  // check when SCL will be relesed TBD - can be endless loop
            goto RELESE_CHECK;
        
        //bitset(I2CTRIS,I2C_SDA); //SDA = 1(INPUT) SCL = 1 now ready to read
        

        bCount = 9;  // stupid but will be smaller code
        while (--bCount)
        {
            DELAY_1_I2C;            
            bWork <<=1;
            if (bittest(I2CPORT,I2C_SDA))
                bitset(bWork,0);
            
            bitclr(I2CTRIS,I2C_SCL);     // SDA = ? SCL = 0
            DELAY_1_I2C;
            if (bCount != 1)
                bitset(I2CTRIS,I2C_SCL);     // SDA = ? SCL = 1 
        }
        // send ACK (or NAK on last read byte
                                 // SDA = ? SCL = 0
     
        if (LenI2CRead == 1)
            bitset(I2CTRIS,I2C_SDA);     // SDA = 1 SCL = 0 NAK
        else
            bitclr(I2CTRIS,I2C_SDA);     // SDA = 0 SCL = 0  ACK
        DELAY_1_I2C;
        bitset(I2CTRIS,I2C_SCL);        //SDA = ACK or NAK SCL = 1 -> send
        DELAY_1_I2C;
        bitclr(I2CTRIS,I2C_SCL);        // SDA = ACK or NAK SCL = 0
                                // delay will be all following code:
        LenI2CRead--;
#ifdef _NOT_SIMULATOR
#else
        bWork = '*';
#endif
        InsertI2C(bWork);
        bitset(I2CTRIS,I2C_SDA);     // SDA = 1 (input) SCL = 0
    } // to next loop it is SDA = 0 SCL = 0
    bitclr(I2CTRIS,I2C_SDA);     // SDA = 0 SCL = 0

#endif   // END NOT OPTIMIZED VERSION

    //GIE =1;
}
void ReleseI2cMaster(void)
{
     if (I2C.NextI2CRead) // this will be restart condition
     {
         // at this call   SDA = 0 and SCL = 0
         DELAY_1_I2C;
         bitset(I2CTRIS,I2C_SDA); // SDA = 1 SCL = 0
         DELAY_1_I2C;
         bitset(I2CTRIS,I2C_SCL); // SDA = 1 SCL = 1;
         //DELAY_1_I2C;  // delay propagated
         return;
     }
     //SSPCON = 0b00011110;
     // at this call   SDA = 0 and SCL = 0
     DELAY_1_I2C;
     bitset(I2CTRIS,I2C_SCL); // SDA = 0 SCL = 1;
     DELAY_1_I2C;
     bitset(I2CTRIS,I2C_SDA); // SDA = 1 SCL = 1
     //SSPEN = 1;
}
#endif //I2C_INT_SUPPORT
#endif //  #ifndef NO_I2C_PROC
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// end COPY 2
//////////////////////////////////////////////////////////////////////



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
                if (!Main.PrepI2C)
                    Main.ESCNextByte = 1;
                else
                    I2C.ESCI2CChar = 1;
             
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
                    if (!btest(SSPORT,SSCS)) // another FLASH write can pause FLASH write intiated by serial 
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
                if (DataB3.FlashWas1byteWrite)
                {
                    if (OldFlashCmd != 0x06) // not write enable command
                    {
                        CS_LOW;
                        SendSSByte(OldFlashCmd);
                    }
                }
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
#ifndef NO_I2C_PROC
             else if (bByte == 'L') // atdtLuna
                 ATCMD = MODE_CALL_LUNA_I2C;//2;
#endif
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

//#include "commc4.h"
////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// begin COPY 4
///////////////////////////////////////////////////////////////////////   

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
     T0CON = 0b10000110;
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
    // or may be not this call comes after INT == that mean inside timer3 it will be not advansed
    // counter SkipPtr prevents from this
    // second place it is a timer0 == this is before timer3 startd to work
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
#ifndef NO_I2C_PROC
             || (ATCMD & MODE_CALL_LUNA_I2C)
#endif
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
#ifndef NO_I2C_PROC
    if (Main.ComNotI2C) // prev was COM processing
    {
        //Main.ComNotI2C = 1;
        bReturn = 1;                    // do process data   
    }
    else               // prev was I2C processing
    {
        if (Main.DoneWithCMD)
        {
            Main.ComNotI2C = 1;
            bReturn = 1;               // do process data
        }
    }    
    return bReturn;
#else
    return 1;                          // do process data
#endif
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

/* ////////////////////////////////////////////////////////////////////////         debug code  
            if (BTStatus & 0x40)
            {
                if (FqRXCount)
                {
                    if (FqRXCount == 1)
                       putch('1');
                    else
                       putch('2');
 
                }
                else
                    putch('0');
            }
            if (BTStatus & 0x20)
            {
                if (FqTXCount)
                {
                    if (FqTXCount == 1)
                       putch('b');
                    else
                       putch('c');
 
                }
                else
                    putch('a');
            }
            if (BTStatus & 0x20)
            {
                if (TXSendOverFQ)
                {
                    if (TXSendOverFQ == 1)
                       putch('b');
                    else
                       putch('c');
 
                }
                else
                    putch('a');
            }
    //////////////////////////////////////////////////////////////////end debug code
*/
            if (BTStatus & 0x40) // RX interrupt
            {
                if (BTType & 0x01) // RX
                {
                    //putch('r');
                    // receve timing OK message dial
                    // <receive = 446mks><6mks IRQ> <1051 mks process on each FQ><ok msg 102mks>
                    // <receive = 446mks><6mks IRQ> <1051 mks process on each FQ><not msg 102mks>
                    // <receive = 446mks><6mks IRQ> <1051 mks process on each FQ><ok msg 102mks>|<ok msg data 1602mks>
                    // max process = 446mks + 6 + + 102 + 2653 = 3207 mks
                    // on FQ3 with max correction :
                    // <receive = 446mks><6mks IRQ> <1051 mks process on each FQ><correction 3447mks>ok msg 102mks>|<ok msg data 1602mks>
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
                            //SetTimer0(0xff58); // 446mks+xxx(2247mks) = 2693 = count 169
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
                                    if ((ATCMD & MODE_CALL_LUNA_COM) 
#ifndef NO_I2C_PROC
                                         || (ATCMD & MODE_CALL_LUNA_I2C)
#endif
                                       ) // earth calls cubsat
                                       SetTimer0(DELAY_BTW_NEXT_DIAL); // 0x0000 == 4 sec till next attempt for earth to dial luna
                                }
                            }
                            //putch(bWork);
                        }
                        else if (RXreceiveFQ == 2) // after receive FQ2 cases: 1) FQ1==OK FQ2 == does not matter 2) FQ1!=OK FQ2==OK
                        {
                            //putch('1');
                            goto SET_TIMER_AND_PROC;
                            //SetTimer0(0xff58); // 446mks+xxx(2247mks) = 2693 = count 169
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
            if (BTStatus & 0x20) // TX interrupt
            {
                if (BTType & 0x02) // TX mode
                {
                    // transmit timing with setup:
                    // <setup = 401.mks> <upload = 960 mks> <transmit = 446mks> <dealy XXX>
                    // transmit timeing without setup:
                    // <upload = 960 mks> <transmit = 446mks> <dealy XXX>
                
                    if (TXSendOverFQ) // needs to send copy over FQ2 or FQ3
                    {
                        if (DataB0.Timer1Done3FQ)
                            goto NEXT_TRANSMIT;
                        // difference btw receive process (<1051 mks>+<OK msg 102mks>+<3207>) and transmit upload (960mks+xxx) xxx= 2247 mks = 141 counts on 32MHz with 128 prescaler
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
                        // 446mks passed no packet on FQ2 then next packet can be during <upload = 960 mks> +<transmit = 446mks> = 1406 mks = 22 couts
                        SetTimer0(0xff58); 
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
                        if ((ATCMD & MODE_CALL_LUNA_COM) 
#ifndef NO_I2C_PROC
                            || (ATCMD & MODE_CALL_LUNA_I2C)
#endif
                           ) // earth calls cubsat
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
	CLKDIVbits.PLLPOST=0;		// N2=2 PLL VCO Output Divider Select bits (also denoted as ‘N2’, PLL postscaler)
                                // 00 = Output/2
                                // 01 = Output divided by 4 (default)
                                // 10 = Reserved
                                // 11 = Output divided by 8
                                // bit 5 Unimplemented: Read as
	CLKDIVbits.PLLPRE=0;		// N1=2 PLL Phase Detector Input Divider bits (also denoted as ‘N1’, PLL prescaler)
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
                                        // bit 4 Unimplemented: Read as ‘0’
                                        // bit 3 CF: Clock Fail Detect bit (read/clear by application)
                                        // bit 2 Unimplemented: Read as ‘0’
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
   OSCCON = 0b01110000; //OSCILLATOR CONTROL REGISTER (ADDRESS 8Fh)
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
// Tx_SCK +SSCLOCK  RA2/AN2/VREF-/CVREF | 4     25| RB4/KBI0/AN11
// Tx_MOSI+SSDATA_IN      RA3/AN3/VREF+ | 5     24| RB3/AN9/CCP2
// Rx_MISO+SSDATA_OUT   RA4/T0CKI/C1OUT | 6     23| RB2/INT2/AN8          BT_RX
//   SSCS       RA5/AN4/SS/HLVDIN/C2OUT | 7     22| RB1/INT1/AN10         BT_TX
//                                  VSS | 8     21| RB0/INT0/FLT0/AN12    Rx_IRQ
//     crystal            OSC1/CLKI/RA7 | 9     20| VDD
//     crystal            OSC2/CLKO/RA6 |10     19| VSS
// SSDATA_OUT2         RC0/T1OSO/T13CKI |11     18| RC7/RX/DT        <--- Serial RX
// SSDATA_OUT3           RC1/T1OSI/CCP2 |12     17| RC6/TX/CK        ---> Serial TX
//                             RC2/CCP1 |13     16| RC5/SDO          ---> Low == Serial RX_FULL (set High on Com queue full)
// dbg blinking LED         RC3/SCK/SCL |14     15| RC4/SDI/SDA      <--- Low == TX_NOT_READY Next serial unit not ready to get data  


    //BT pin assignment

    // #define PORT_BT PORTAbits
    // #define Tx_CE      RA0	// RA0 pin 2 // Chip Enable Activates RX or TX mode -> Out
    // #define Tx_CSN     RA1	// RA1 pin 3 // SPI Chip Select ->Out
    // #define Tx_SCK     RA2    // RA2 pin 4  // SPI Clock -> Out
    // #define Tx_MOSI    RA3	// RA3 pin 5  // SPI Slave Data Input -> Out
    // #define Rx_MISO    RA4	// RA4 pin 6  // SPI Slave Data Output, with tri-state option -> In
    // #define Rx_IRQ     RB0    // RB0 pin 21 // Maskable interrupt pin. Active low -> in
    // #define BT_RX      RB2   // RB2 pin 23 BT in receive mode


    // SPI output in FLASH mem terminoligy:
    // SSCLOCK RA2(pin4), SSDATA_IN RA3(pin5), SSDATA_OUT RA4(pin6), SSCS RA5(pin7)
    //          0            0                        IN                  1
    // RA6 & RA7 == IN Crystal osc
    TRISA = 0b11010000;  //0 = Output, 1 = Input 
    PORTA = 0b00101110; // high=, Tx_CSN, Tx_SCK/SSCLOCK, Tx_MOSI/SSDATA_IN, , SSCS, low=Tx_CE
    // RB0 - external INT Pin 21
    TRISB = 0b00000001;  //0 = Output, 1 = Input 
    PORTB = 0b00000000;  // nothing happened with amplifiers BT_TX,BT_RX=low

    // RC7 - Serial RX  Pin 18
	// RC6 - Serial TX Pin 17
    // RC5 - RX_FULL     pin 16 - out
    // RC4 - TX_NOT_READY pin 15 - in
    // debug LED RC3 pin 14
    // SSDATA_OUT2 = IN RC0 pin 11
    // SSDATA_OUT3 = IN RC1 pin 12
    TRISC = 0b10010011;  //0 = Output, 1 = Input 
    PORTC = 0b01000000;
     
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
     T0CON = 0b00000110;

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
                         //    Note 1: Bit resets to ‘0’ with Two-Speed Start-up mode and LP, XT or HS selected as the
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
    T0CON = 6; //prescaler 1 tick = 16mks => 1ms = 63 tic 2ms = 125 value 0xff00 mean 4ms value 0xf424 = 1s
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
}


void ShowMessage(void)
{
    if (!TX_NOT_READY)  // output unit ID only if next unit is receiving data
    {
        // if message initiated by unit needs to check then it is possible to do:
        while(!Main.prepStream) // this will wait untill no relay message in a progress
        {
        }
        // in a case of a CMD replay it is safly to skip that check - unit allow to send message in CMD mode
        putch(MY_UNIT);  // this message will circle over com and will be supressed by unit
        Puts("~");
        putch(MY_UNIT);
    }
}

//#include "commc8.h"
/////////////////////////////////////////////////////////////////
//      Begin COPY 8
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

#ifndef __PIC24H__
#ifndef _16F724
// EECON1
//        bit 7 EEPGD: Flash Program or Data EEPROM Memory Select bit
//            1 = Access Flash program memory
//            0 = Access data EEPROM memory
//        bit 6 CFGS: Flash Program/Data EEPROM or Configuration Select bit
//            1 = Access Configuration registers
//            0 = Access Flash program or data EEPROM memory
//        bit 5 Unimplemented: Read as '0'
//        bit 4 FREE: Flash Row Erase Enable bit
//            1 = Erase the program memory row addressed by TBLPTR on the next WR command (cleared
//               by completion of erase operation)
//           0 = Perform write only
//        bit 3 WRERR: Flash Program/Data EEPROM Error Flag bit
//            1 = A write operation is prematurely terminated (any Reset during self-timed programming in
//                normal operation, or an improper write attempt)
//            0 = The write operation completed
//          Note: When a WRERR occurs, the EEPGD and CFGS bits are not cleared.
//          This allows tracing of the error condition.
//        bit 2 WREN: Flash Program/Data EEPROM Write Enable bit
//            1 = Allows write cycles to Flash program/data EEPROM
//            0 = Inhibits write cycles to Flash program/data EEPROM
//        bit 1 WR: Write Control bit
//            1 = Initiates a data EEPROM erase/write cycle or a program memory erase cycle or write cycle
//                (The operation is self-timed and the bit is cleared by hardware once write is complete.
//                The WR bit can only be set (not cleared) in software.)
//            0 = Write cycle to the EEPROM is complete
//        bit 0 RD: Read Control bit
//            1 = Initiates an EEPROM read
//                (Read takes one cycle. RD is cleared in hardware. The RD bit can only be set (not cleared)
//                 in software. RD bit cannot be set when EEPGD = 1 or CFGS = 1.)
//            0 = Does not initiate an EEPROM read
unsigned char eeprom_read(unsigned char addr)
{
    EEADR = addr;
    EECON1 &= 0x3F;
    RD = 1;
    return EEDATA;
}
void eeprom_write(unsigned char addr, unsigned char value)
{
    EEADR = addr;
    EEDATA = value;
    EECON1 &= 0x3F;
    Carry = 0;
    if(GIE)
        Carry = 1;
    GIE = 0;
    WREN = 1;
    EECON2 = 0x55;
    EECON2 = 0xAA;
    WR = 1;
    WREN = 0;
    if(Carry)
        GIE = 1;
    while(WR)
    {
    }
}
#endif
#endif

void enable_uart(void)//bit want_ints)
{
 
#ifdef __PIC24H__
    TXIE = 0;  // this is macro redifinitions to be compatible with 16LF88 16LF884 18F2321
    RCIE = 0; // disable interrupt on recieve byte
    U1MODE = 0b000100010001000;
             //0                // bit 15 UARTEN: UARTx Enable bit(1)
                                //         1 = UARTx is enabled; all UARTx pins are controlled by UARTx as defined by UEN<1:0>
                                //         0 = UARTx is disabled; all UARTx pins are controlled by port latches; UARTx power consumption minimal
                                // bit 14 Unimplemented: Read as ‘0’
             // 0               // bit 13 USIDL: Stop in Idle Mode bit
                                //         1 = Discontinue module operation when device enters Idle mode
                                //         0 = Continue module operation in Idle mode
             //  0              // bit 12 IREN: IrDA® Encoder and Decoder Enable bit(2)
                                //         1 = IrDA encoder and decoder enabled
                                //         0 = IrDA encoder and decoder disabled
             //   1             // bit 11 RTSMD: Mode Selection for UxRTS Pin bit
                                //         1 = UxRTS pin in Simplex mode
                                //         0 = UxRTS pin in Flow Control mode
             //    0            // bit 10 Unimplemented: Read as ‘0’
                                // bit 9-8 UEN<1:0>: UARTx Enable bits
             //     00          //        11 = UxTX, UxRX and BCLK pins are enabled and used; UxCTS pin controlled by port latches
                                //        10 = UxTX, UxRX, UxCTS and UxRTS pins are enabled and used
                                //        01 = UxTX, UxRX and UxRTS pins are enabled and used; UxCTS pin controlled by port latches
                                //        00 = UxTX and UxRX pins are enabled and used; UxCTS and UxRTS/BCLK pins controlled by port latches
             //       1         // bit 7 WAKE: Wake-up on Start bit Detect During Sleep Mode Enable bit
                                //         1 = UARTx continues to sample the UxRX pin; interrupt generated on falling edge; bit cleared in hardware on following rising edge
                                //         0 = No wake-up enabled
             //        0        // bit 6 LPBACK: UARTx Loopback Mode Select bit
                                //         1 = Enable Loopback mode
                                //         0 = Loopback mode is disabled
             //         0       // bit 5 ABAUD: Auto-Baud Enable bit
                                //         1 = Enable baud rate measurement on the next character – requires reception of a Sync field (55h)
                                //             before other data; cleared in hardware upon completion
                                //         0 = Baud rate measurement disabled or completed
             //          0      // bit 4 URXINV: Receive Polarity Inversion bit
                                //        1 = UxRX Idle state is ‘0’
                                //        0 = UxRX Idle state is ‘1’
             //           1     // bit 3 BRGH: High Baud Rate Enable bit
                                //        1 = BRG generates 4 clocks per bit period (4x baud clock, High-Speed mode)
                                //        0 = BRG generates 16 clocks per bit period (16x baud clock, Standard mode)
             //            00   // bit 2-1 PDSEL<1:0>: Parity and Data Selection bits
                                //       11 = 9-bit data, no parity
                                //       10 = 8-bit data, odd parity
                                //       01 = 8-bit data, even parity
                                //       00 = 8-bit data, no parity
             //              0  // bit 0 STSEL: Stop Bit Selection bit
                                //        1 = Two Stop bits
                                //        0 = One Stop bit
    U1BRG = SPBRG_SPEED;
    U1STAbits.UTXBRK = 0;
    U1STAbits.UTXISEL1 = 0;     // 11 = Reserved
    U1STAbits.UTXISEL0 = 0;     // 10 = Interrupt generated when a character is transferred to the Transmit Shift register and the transmit buffer becomes empty
                                // 01 = Interrupt generated when the last transmission is over (last character shifted out of Transmit Shift register) and all the transmit operations are completed
                                // 00 = Interrupt generated when any character is transferred to the Transmit Shift Register (this implies at least one location is empty in the transmit buffer)

    U1STAbits.URXISEL1 = 0;
    U1STAbits.URXISEL0 = 0;     // 11 = Interrupt is set on UxRSR transfer making the receive buffer full (i.e., has 4 data characters)
                                // 10 = Interrupt is set on UxRSR transfer making the receive buffer 3/4 full (i.e., has 3 data characters)
                                // 0x = Interrupt is set when any character is received and transferred from the UxRSR to the receive buffer. Receive buffer has one or more characters
    TXEN = 0;  // this is macro redifinitions to be compatible with 16LF88 16LF884 18F2321 // disable transmit
    RCIF = 0;  // clean bit of receive interrupt
    RCIE = 1;  // enable interrupt on recieve byte
    U1MODEbits.UARTEN = 1; // enable uart
#ifdef USE_COM2
    // second UART
    TXIECOM2 = 0;  // this is macro redifinitions to be compatible with 16LF88 16LF884 18F2321
    RCIECOM2 = 0; // disable interrupt on recieve byte
#ifdef INVERS_COM2
    U2MODE = 0b000100010011000;
#else
    U2MODE = 0b000100010001000;
#endif                             
             //0                // bit 15 UARTEN: UARTx Enable bit(1)
                                //         1 = UARTx is enabled; all UARTx pins are controlled by UARTx as defined by UEN<1:0>
                                //         0 = UARTx is disabled; all UARTx pins are controlled by port latches; UARTx power consumption minimal
                                // bit 14 Unimplemented: Read as '0'
             // 0               // bit 13 USIDL: Stop in Idle Mode bit
                                //         1 = Discontinue module operation when device enters Idle mode
                                //         0 = Continue module operation in Idle mode
             //  0              // bit 12 IREN: IrDA Encoder and Decoder Enable bit(2)
                                //         1 = IrDA encoder and decoder enabled
                                //         0 = IrDA encoder and decoder disabled
             //   1             // bit 11 RTSMD: Mode Selection for UxRTS Pin bit
                                //         1 = UxRTS pin in Simplex mode
                                //         0 = UxRTS pin in Flow Control mode
             //    0            // bit 10 Unimplemented: Read as '0'
                                // bit 9-8 UEN<1:0>: UARTx Enable bits
             //     00          //        11 = UxTX, UxRX and BCLK pins are enabled and used; UxCTS pin controlled by port latches
                                //        10 = UxTX, UxRX, UxCTS and UxRTS pins are enabled and used
                                //        01 = UxTX, UxRX and UxRTS pins are enabled and used; UxCTS pin controlled by port latches
                                //        00 = UxTX and UxRX pins are enabled and used; UxCTS and UxRTS/BCLK pins controlled by port latches
             //       1         // bit 7 WAKE: Wake-up on Start bit Detect During Sleep Mode Enable bit
                                //         1 = UARTx continues to sample the UxRX pin; interrupt generated on falling edge; bit cleared in hardware on following rising edge
                                //         0 = No wake-up enabled
             //        0        // bit 6 LPBACK: UARTx Loopback Mode Select bit
                                //         1 = Enable Loopback mode
                                //         0 = Loopback mode is disabled
             //         0       // bit 5 ABAUD: Auto-Baud Enable bit
                                //         1 = Enable baud rate measurement on the next character requires reception of a Sync field (55h)
                                //             before other data; cleared in hardware upon completion
                                //         0 = Baud rate measurement disabled or completed
             //          1      // bit 4 URXINV: Receive Polarity Inversion bit
                                //        1 = UxRX Idle state is '0'
                                //        0 = UxRX Idle state is '1'
             //           1     // bit 3 BRGH: High Baud Rate Enable bit
                                //        1 = BRG generates 4 clocks per bit period (4x baud clock, High-Speed mode)
                                //        0 = BRG generates 16 clocks per bit period (16x baud clock, Standard mode)
             //            00   // bit 2-1 PDSEL<1:0>: Parity and Data Selection bits
                                //       11 = 9-bit data, no parity
                                //       10 = 8-bit data, odd parity
                                //       01 = 8-bit data, even parity
                                //       00 = 8-bit data, no parity
             //              0  // bit 0 STSEL: Stop Bit Selection bit
                                //        1 = Two Stop bits
                                //        0 = One Stop bit
    U2BRG = SPBRG_SPEEDCOM2;
    //   boud rate formula: BRGH = 0
    // BoudRate = Fcy/ (16 * (UxBRG + 1))
    //                      BRGH = 1
    // BoudRate = Fcy/ (4 * (UxBRG + 1))
    // (4 * (UxBRG + 1)) = Fcy/BoudRate
    // (UxBRG + 1) = Fcy/(4*BoudRate)
    // UxBRG = Fcy/(4*BoudRate) - 1;
    // 
    // for example on Fcy = 10.13375 MHz = 10133750 and U2BRG = 263 BoudRate = 9596
    // #define SPBRG_9600_10MIPS 262
    // #define SPBRG_57600_10MIPS 43
//    

    U2STAbits.UTXBRK = 0;
    U2STAbits.UTXISEL1 = 0;     // 11 = Reserved
    U2STAbits.UTXISEL0 = 0;     // 10 = Interrupt generated when a character is transferred to the Transmit Shift register and the transmit buffer becomes empty
                                // 01 = Interrupt generated when the last transmission is over (last character shifted out of Transmit Shift register) and all the transmit operations are completed
                                // 00 = Interrupt generated when any character is transferred to the Transmit Shift Register (this implies at least one location is empty in the transmit buffer)

    U2STAbits.URXISEL1 = 0;
    U2STAbits.URXISEL0 = 0;     // 11 = Interrupt is set on UxRSR transfer making the receive buffer full (i.e., has 4 data characters)
                                // 10 = Interrupt is set on UxRSR transfer making the receive buffer 3/4 full (i.e., has 3 data characters)
                                // 0x = Interrupt is set when any character is received and transferred from the UxRSR to the receive buffer. Receive buffer has one or more characters

                                // bit 15,13 UTXISEL<1:0>: Transmission Interrupt Mode Selection bits
                                //    11 = Reserved; do not use
                                //    10 = Interrupt when a character is transferred to the Transmit Shift Register, and as a result, the
                                //         transmit buffer becomes empty
                                //    01 = Interrupt when the last character is shifted out of the Transmit Shift Register; all transmit
                                //         operations are completed
                                //    00 = Interrupt when a character is transferred to the Transmit Shift Register (this implies there is
                                //         at least one character open in the transmit buffer)
                                // bit 14 UTXINV: Transmit Polarity Inversion bit
                                //         If IREN = 0:
                                //     1 = UxTX Idle state is '0'
                                //     0 = UxTX Idle state is '1'
                                //         If IREN = 1:
                                //     1 = IrDA encoded UxTX Idle state is '1'
                                //     0 = IrDA encoded UxTX Idle state is '0'
                                // bit 12 Unimplemented: Read as '0'
                                // bit 11 UTXBRK: Transmit Break bit
                                //     1 = Send Sync Break on next transmission Start bit, followed by twelve '0' bits, followed by Stop bit;
                                //         cleared by hardware upon completion
                                //     0 = Sync Break transmission disabled or completed
                                // bit 10 UTXEN: Transmit Enable bit(1)
                                //     1 = Transmit enabled, UxTX pin controlled by UARTx
                                //     0 = Transmit disabled, any pending transmission is aborted and buffer is reset. UxTX pin controlled
                                //          by port
                                // bit 9 UTXBF: Transmit Buffer Full Status bit (read-only)
                                //     1 = Transmit buffer is full
                                //     0 = Transmit buffer is not full, at least one more character can be written
                                // bit 8 TRMT: Transmit Shift Register Empty bit (read-only)
                                //     1 = Transmit Shift Register is empty and transmit buffer is empty (the last transmission has completed)
                                //     0 = Transmit Shift Register is not empty, a transmission is in progress or queued
                                // bit 7-6 URXISEL<1:0>: Receive Interrupt Mode Selection bits
                                //    11 = Interrupt is set on UxRSR transfer making the receive buffer full (i.e., has 4 data characters)
                                //    10 = Interrupt is set on UxRSR transfer making the receive buffer 3/4 full (i.e., has 3 data characters)
                                //    0x = Interrupt is set when any character is received and transferred from the UxRSR to the receive
                                //         buffer. Receive buffer has one or more characters
                                // bit 5 ADDEN: Address Character Detect bit (bit 8 of received data = 1)
                                //     1 = Address Detect mode enabled. If 9-bit mode is not selected, this does not take effect
                                //     0 = Address Detect mode disabled
                                // bit 4 RIDLE: Receiver Idle bit (read-only)
                                //     1 = Receiver is Idle
                                //     0 = Receiver is active
                                // bit 3 PERR: Parity Error Status bit (read-only)
                                //     1 = Parity error has been detected for the current character (character at the top of the receive FIFO)
                                //     0 = Parity error has not been detected
                                // bit 2 FERR: Framing Error Status bit (read-only)
                                //     1 = Framing error has been detected for the current character (character at the top of the receive
                                //         FIFO)
                                //     0 = Framing error has not been detected
                                // bit 1 OERR: Receive Buffer Overrun Error Status bit (read/clear only)
                                //     1 = Receive buffer has overflowed
                                //     0 = Receive buffer has not overflowed. Clearing a previously set OERR bit (1 ? 0 transition) resets
                                //         the receiver buffer and the UxRSR to the empty state
                                // bit 0 URXDA: Receive Buffer Data Available bit (read-only)
                                //     1 = Receive buffer has data, at least one more character can be read
                                //     0 = Receive buffer is empty


#ifdef INVERS_COM2
    U2STAbits.UTXINV = 1;       // inversion of TX data == idle = 0
#else
    U2STAbits.UTXINV = 0;       // TX data == idle = 1
#endif
    TXENCOM2 = 0;  // this is macro redifinitions to be compatible with 16LF88 16LF884 18F2321 // disable transmit
    RCIFCOM2 = 0;  // clean bit of receive interrupt
    RCIECOM2 = 1;  // enable interrupt on recieve byte
    U2MODEbits.UARTEN = 1; // enable uart
#endif // second uart

#else // 88,884,2321

    TX9 = 0;
    RX9 = 0;
//#ifdef _18F2321_18F25K20
//    SPBRGH = 0;
//    BRG16 = 0;
//#endif

    BRGH = 1; //Normal speed UART port 0x98 
              // 00000x00 BRGH: High Baud Rate Select bit
              //Asynchronous mode:
              // 1 = High speed 0 = Low speed
    SPBRG = SPBRG_SPEED;//51;// 9600
    //SPBRG = 25;// 19200
    //SPBRG = 12;// 38400
    //SPBRG = 8; // 57600

    SYNC = 0; // port 0x98 = 000x0000 x=0 asynch mode 1-synchr
    SPEN = 1; // port 0x18 x0000000 SPEN: Serial Port Enable bit
              // 1 = Serial port enabled (configures RB2/SDO/RX/DT 
              // and RB5/SS/TX/CK pins as serial port pins)

    //TXIF = 0;
    TXIE = 0;
    TXEN = 0;
    //TXEN = 1; //Enable transmission port 0x8c
              // AUSART Transmit Interrupt Enable bit
              // 1 = Enabled 0 = Disabled
    ADDEN = 0;
    //RCIF = 0;  // clean bit of receive interrupt
    RCIE = 1; // enable interrupt on recieve byte
    CREN = 1; // port 0x18 000x0000 CREN: Continuous Receive Enable bit
              // Asynchronous mode: 1 = Enables continuous receive
    
    //WREN = 1;
#endif
}    
#ifndef NO_I2C_PROC
void enable_I2C(void)
{
#ifdef __PIC24H__
   I2C1CON = 0b1000000000000000; 
                                 // bit 15 I2CEN: I2Cx Enable bit
           //  1                 //      1 = Enables the I2Cx module and configures the SDAx and SCLx pins as serial port pins
                                 //      0 = Disables the I2Cx module; all I2C pins are controlled by port functions
           //   0                // bit 14 Unimplemented: Read as ‘0’
           //    0               // bit 13 I2CSIDL: Stop in Idle Mode bit
                                 //      1 = Discontinue module operation when device enters Idle mode
                                 //      0 = Continue module operation in Idle mode
           //     0              // bit 12 SCLREL: SCLx Release Control bit (when operating as I2C slave)
                                 //      1 = Release SCLx clock 
                                 //      0 = Hold SCLx clock low (clock stretch)
                                 //          If STREN = 1:
                                 //          Bit is R/W (i.e., software may write ‘0’ to initiate stretch and write ‘1’ to release clock). Hardware clear
                                 //          at beginning of slave transmission and at end of slave reception.
                                 //          If STREN = 0:
                                 //          Bit is R/S (i.e., software may only write ‘1’ to release clock). Hardware clear at beginning of slave transmission.
           //      0             // bit 11 IPMIEN: Intelligent Platform Management Interface (IPMI) Enable bit
                                 //      1 = IPMI Support mode is enabled; all addresses Acknowledged
                                 //      0 = IPMI Support mode disabled
           //       0            // bit 10 A10M: 10-Bit Slave Address bit
                                 // 1 = I2CxADD register is a 10-bit slave address 0 = I2CxADD register is a 7-bit slave address
           //        0           // bit 9 DISSLW: Disable Slew Rate Control bit
                                 //      1 = Slew rate control disabled 0 = Slew rate control enabled
           //         0          // bit 8 SMEN: SMBus Input Levels bit
                                 // 1 = Enable I/O pin thresholds compliant with SMBus specification 0 = Disable SMBus input thresholds
           //          0         // bit 7 GCEN: General Call Enable bit (when operating as I2C slave)
                                 //      1 = Enable interrupt when a general call address is received in the I2CxRSR register (module is enabled for reception)
                                 //      0 = General call address disabled
           //           0        // bit 6 STREN: SCLx Clock Stretch Enable bit (I2C Slave mode only; used in conjunction with SCLREL bit)
                                 //      1 = Enable software or receive clock stretching 0 = Disable software or receive clock stretching
           //            0       // bit 5 ACKDT: Acknowledge Data bit (I2C Master mode; receive operation only) 
                                 // Value that will be transmitted when the software initiates an Acknowledge sequence
                                 //      1 = Send NACK during Acknowledge 0 = Send ACK during Acknowledge
           //             0      // bit 4 ACKEN: Acknowledge Sequence Enable bit (I2C Master mode receive operation)
                                 //      1 = Initiate Acknowledge sequence on SDAx and SCLx pins and transmit ACKDT data bit (hardware clear at end of master Acknowledge sequence)
                                 //      0 = Acknowledge sequence not in progress
           //              0     // bit 3 RCEN: Receive Enable bit (I2C Master mode)
                                 //      1 = Enables Receive mode for I2C (hardware clear at end of eighth bit of master receive data byte)
                                 //      0 = Receive sequence not in progress
           //               0    // bit 2 PEN: Stop Condition Enable bit (I2C Master mode)
                                 //      1 = Initiate Stop condition on SDAx and SCLx pins (hardware clear at end of master Stop sequence)
                                 //      0 = Stop condition not in progress
           //                0   // bit 1 RSEN: Repeated Start Condition Enable bit (I2C Master mode)
                                 //      1 = Initiate Repeated Start condition on SDAx and SCLx pins (hardware clear at end of master Repeated Start sequence)
                                 //      0 = Repeated Start condition not in progress
           //                 0  // bit 0 SEN: Start Condition Enable bit (I2C Master mode)
                                 //      1 = Initiate Start condition on SDAx and SCLx pins (hardware clear at end of master Start sequence)
                                 //      0 = Start condition not in progress
      IFS1bits.MI2C1IF = 0; // clean interrupt
      IEC1bits.MI2C1IE = 1; // enable interupt on master
#else // end of PIC24

#ifdef _18F2321_18F25K20
 #ifndef I2C_ONLY_MASTER
    SSPCON1 =0b00111110; // TBD in _18F2321_18F25K20 master I2C implemented 
 #else
    SSPCON1 =0b00011000;
    SSPCON2 =0b00000000;
 #endif
#else // 88, 884
    #warning "pic18f884 does have I2C master support in firmaware"
    SSPCON = 0b00111110;
#endif
             //0        -WCOL: Write Collision Detect bit
             //          1 = An attempt to write the SSPBUF register failed because the SSP module is busy
             //              (must be cleared in software)
             //          0 = No collision
             // 0       -SSPOV: Receive Overflow Indicator bit
             //          1 = A byte is received while the SSPBUF register is still holding the previous byte. SSPOV is
             //              a “don’t care” in Transmit mode. SSPOV must be cleared in software in either mode.
             //          0 = No overflow
             //  1      -SSPEN: Synchronous Serial Port Enable bit
             //          1 = Enables the serial port and configures the SDA and SCL pins as serial port pins
             //          0 = Disables serial port and configures these pins as I/O port pins
             //   1     -CKP: Clock Polarity Select bit.SCK release control
             //          1 = Enable clock
             //          0 = Holds clock low (clock stretch). (Used to ensure data setup time.)
             //    1110-SSPM<3:0>: Synchronous Serial Port Mode Select bits
             //          0110 = I2C Slave mode, 7-bit address
             //          0111 = I2C Slave mode, 10-bit address
             //          1011 = I2C Firmware Controlled Master mode (Slave Idle)
             //          1110 = I2C Slave mode, 7-bit address with Start and Stop bit interrupts enabled
             //          1111 = I2C Slave mode, 10-bit address with Start and Stop bit interrupts enabled
             //          1000 = I2C MASTER mode
    SMP = 0;
    CKE = 0;

    SSPEN = 1; // enable I2C
    //P = 0;     // no stop
    //S = 0;     // no start
    //BF = 0;    // nothing in a buffer (may be better read SSPBUFF?)
    SSPIF = 0;  // clean inderrupt
    SSPIE = 1;  // enable interrupt
    //SSPOV = 0;  // clean owerflow
#endif
}
#endif
void EnableTMR1(void)
{
#ifdef __PIC24H__

#else
    TMR1L =0;
    TMR1H = 0;
    T1CON = 0b1001001;
    //                 T1CON: TIMER1 CONTROL REGISTER (ADDRESS 10h)
    //        1        TMR1ON: Timer1 On bit 1 = Enables Timer1 0 = Stops Timer1
    //                         1 = External clock from pin RB6/AN5(1)/PGC/T1OSO/T1CKI (on the rising edge)
    //                         0 = Internal clock (FOSC/4)
    //         00      T1CKPS<1:0>: Timer1 Input Clock Prescale Select bits
    //                        11 =1:8 Prescale value with FOSC/4 == 16cmd per count
    //                        10 =1:4 Prescale value with FOSC/4 == 8 cmd per count
    //                        01 =1:2 Prescale value with FOSC/4 == 4 cmd per count
    //                        00 =1:1 Prescale value with FOSC/4 == 2 cmd per count
    //           1     T1OSCEN - 1 ocsilator enabled 0 disabled
    //            0    T1SYNC  - ignored with TMR1CS == 0
    //             0   TMR1CS: Timer1 Clock Source Select bit internal FOSC/4 
    //              1  TMR1ON - timer enabled
    TMR1IF = 0;
    TMR1IE = 0;
    //TMR1ON = 1;  // TMR1ON: Timer1 On bit
                 // 1 = Enables Timer1
                 // 0 = Stops Timer1
#endif
}
//#define SSPORT PORTA
//#define SSCLOCK 7
//#define SSDATA_IN 6
//#define SSDATA_OUT 5
//#define SSCS       4
#ifdef SSPORT
// it is working with port in bank 0
#pragma rambank RAM_BANK_0
void SendSSByte(unsigned char bByte)
{
    WORD bWork;
    //bitclr(SSPORT,SSCS); // set low Chip Select
    bWork = 8;
#pragma updateBank 0

    do
    {
#ifdef __PIC24H__
        // nobody uses portA ??? make sure!!!
        // otherwise it must be 3 commands instead of one:
         bclr(SSPORT,SSCLOCK);
         nop();
         bclr(SSPORT,SSDATA_IN);

        // SSCLOCK RA0(pin2), SSDATA_IN RA1(pin3), SSDATA_OUT RA2(pin9), SSCS RA3(pin10)
        //PORTA = 0b00000000;
#else
        bclr(SSPORT,SSCLOCK);
        bclr(SSPORT,SSDATA_IN);
#endif
		if (bittest(bByte,7))
            bset(SSPORT,SSDATA_IN);
#ifdef _OPTIMIZED_
  #ifdef __PIC24H__
         bByte<<=1;
  #else
   #ifdef      _18F2321_18F25K20
            #asm
              RLCF bByte,1,1
            #endasm
   #else
            RLF(bByte,1);
   #endif
  #endif
#else // not optimized version
        bByte<<=1;
#endif
        bset(SSPORT,SSCLOCK);
    }
    while (--bWork); // 7*8 = 56 or 8*8 = 64 commands
    bclr(SSPORT,SSCLOCK);
    //nop();
    //bclr(SSPORT,SSDATA_IN);
    //bset(SSPORT,SSCS); // set high Chip Select
}
#pragma updateBank 1
void SendSSByteFAST(unsigned char bByte)
{
#ifdef __PIC24H__
    // nobody uses portA ??? make sure!!!
    // otherwise it must be changed on bit 1:
    // SSCLOCK RA0(pin2), SSDATA_IN RA1(pin3), SSDATA_OUT RA2(pin9), SSCS RA3(pin10)
    bclr(SSPORT,SSDATA_IN);
    nop();
    bset(SSPORT,SSCLOCK);  // bit 7
    nop();
    bclr(SSPORT,SSCLOCK);
    nop();
    bset(SSPORT,SSCLOCK);  // bit 6
    nop();
    bclr(SSPORT,SSCLOCK);
    nop();
    bset(SSPORT,SSCLOCK);  // bit 5
    nop();
    bclr(SSPORT,SSCLOCK);
    nop();
    bset(SSPORT,SSCLOCK);  // bit 4
    nop();
    bclr(SSPORT,SSCLOCK);
    nop();
    bset(SSPORT,SSCLOCK);  // bit 3
    nop();
    bclr(SSPORT,SSCLOCK);
    nop();
    bset(SSPORT,SSCLOCK);  // bit 2
    nop();
    bclr(SSPORT,SSCLOCK);
    //if (bittest(bByte,1))
    if (bByte&2)
    {
        bset(SSPORT,SSDATA_IN);
        nop();
    }
    bset(SSPORT,SSCLOCK);  // bit 1
    nop();
    PORTA = 0b00000000;
    //bclr(SSPORT,SSCLOCK);
    //nop();
    //bclr(SSPORT,SSDATA_IN); // essential

    //if (bittest(bByte,0))
    if (bByte&1)
    {
        bset(SSPORT,SSDATA_IN);
        nop();
    }
    bset(SSPORT,SSCLOCK);  // bit 0
    nop();
    bclr(SSPORT,SSCLOCK);  // 42 commands
#else
    bclr(SSPORT,SSDATA_IN);
    bset(SSPORT,SSCLOCK);  // bit 7
    bclr(SSPORT,SSCLOCK);
    bset(SSPORT,SSCLOCK);  // bit 6
    bclr(SSPORT,SSCLOCK);
    bset(SSPORT,SSCLOCK);  // bit 5
    bclr(SSPORT,SSCLOCK);
    bset(SSPORT,SSCLOCK);  // bit 4
    bclr(SSPORT,SSCLOCK);
    bset(SSPORT,SSCLOCK);  // bit 3
    bclr(SSPORT,SSCLOCK);
    bset(SSPORT,SSCLOCK);  // bit 2
    bclr(SSPORT,SSCLOCK);
    //if (bittest(bByte,1))
    if (bByte&2)
        bset(SSPORT,SSDATA_IN);
    bset(SSPORT,SSCLOCK);  // bit 1
    bclr(SSPORT,SSCLOCK);
    bclr(SSPORT,SSDATA_IN);
    //if (bittest(bByte,0))
    if (bByte&1)
        bset(SSPORT,SSDATA_IN);
    bset(SSPORT,SSCLOCK);  // bit 0
    bclr(SSPORT,SSCLOCK);  // 23 commands
#endif
}
#ifndef SSPORT_READ
#define SSPORT_READ  SSPORT
#define SSDATA_OUT_READ SSDATA_OUT
#endif

unsigned char GetSSByte(void)
{
    int bWork;
    unsigned int bWork2;
    //bitclr(SSPORT,SSCS); // set low Chip Select
    bWork = 8;
#pragma updateBank 0
    bWork2 = 0;
    do
    {
        bWork2 <<=1;
        bset(SSPORT,SSCLOCK);
        //nop();
        //bitclr(bWork2,0); // bWork2 is unsigned == zero in low bit garanteed check assembler code to confirm
//#undef SSDATA_OUT2
#ifdef SSDATA_OUT2

        if (btest(SSPORT_READ,SSDATA_OUT))
        {
            if (btest(SSPORT2,SSDATA_OUT2))
                goto FLASH_MAJORITY;
            else if (btest(SSPORT2,SSDATA_OUT3))
                goto FLASH_MAJORITY;
        }
        else if (btest(SSPORT2,SSDATA_OUT2))
                 if (btest(SSPORT2,SSDATA_OUT3))
                 {
FLASH_MAJORITY:
                     bitset(bWork2,0);
                 }
#else
        if (btest(SSPORT_READ,SSDATA_OUT_READ))
            bitset(bWork2,0);
#endif
        bclr(SSPORT,SSCLOCK);
    }
    while (--bWork);
    return bWork2;
    //bset(SSPORT,SSCS); // set high Chip Select
}
#pragma updateBank 1
#ifdef FLASH_POWER_DOWN
void CsLow(void)
{
    
    bclr(SSPORT,SSCS); // set low Chip Select
    SendSSByte(0xab);  // power save mode off
    bset(SSPORT,SSCS); // set high Chip Select
    //                    3mks needs to wait
}
void CsHigh(void)
{
     bset(SSPORT,SSCS); // set high Chip Select
     nop();nop();
     bclr(SSPORT,SSCS); // set low Chip Select
     SendSSByte(0xb9);  // set power save mode
     bset(SSPORT,SSCS); // set high Chip Select
}
#endif
#endif
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// end COPY 8
///////////////////////////////////////////////////////////////////////


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
                iCrc = PTR_FSR + sizeof(PacketStart);
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
        ATCMD |= MODE_CONNECT;
        
    }
    else // another packets == data
    {
        ptrMy+=sizeof(PacketStart);
#ifdef NON_STANDART_MODEM
        // write good packet into FLASH memory for processing
        CS_HIGH;  // that will interrupt operations read and write comming from com

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
    {
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
         i = BTFixlen(ptrMy, bret); // if it is posible to fix packet i == 0

         if (!DataB0.Tmr3DoneMeasureFq1Fq2)
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
                 // stop timer 3 == needs to get two good packets over FQ1 and FQ2
                 TMR3ON = 0;
             }
         }
         else //if (DataB0.Tmr3DoneMeasureFq1Fq2)
         {
             //if (DataB0.Timer3OldSkipSwitch) // 1 == it is not a time to switch frequency yet == wrong packet
             //{
             //    if (i) // pkt is not OK
             //    {
             //        if (RXreceiveFQ == 0)
             //        {
             //            BTCE_high(); // Chip Enable Activates RX or TX mode (now RX mode) 
             //            goto DONE_RX;
             //        }
             //    }
             //}
         }
LOOKS_GOOD:
         // that switch will update BTStatus
         SwitchFQ(DoFqRXSwitch()); // if it was RX over FQ1 than value RXreceiveFQ ==0
         //SwitchFQ(FqRX); // next frequency

         BTCE_high(); // Chip Enable Activates RX or TX mode (now RX mode) 
         

         if (bret > 28)
             bret = 28;
         
         if (BTokMsg == 0xff) // if paket was not recevet yet correctly
         {
              if (i == 0)
              {
                  if (CheckPacket(ptrMy, bret) == 0)
                  {
                       BTokMsg = RXreceiveFQ;
                       if (DataB0.Tmr3DoneMeasureFq1Fq2)
                       //if ((AdjustTimer3 > 0x3700) || (AdjustTimer3 < 0x3300))
                       {
/*                          if (ptrMy[6] == '+')
                          {
                             Tmr3LoadLowCopy++;
                          }
                          if (ptrMy[6] == '-')
                          {
                             Tmr3LoadLowCopy--;
                          }
*/
ADJUST_TMR3:
                          //putch('?');
                          AdjTimer3();
                          //TMR3ON = 0;
                          ////Tmr3LoadLow = ((MEDIAN_TIME - 0xffff)  + AdjustTimer3);
                          ////Tmr3LoadLow = Tmr3LoadLowCopy - Tmr3LoadLow;
                          //Tmr3LoadLow = Tmr3LoadLowCopy + Tmr3LoadLowCopy + MEDIAN_TIME;
                          //Tmr3LoadLow -= AdjustTimer3;
                          //TMR3ON = 1;
                          i = ptrMy[4];
                          if (i > RXreceiveFQ)
                              RXreceiveFQ = i;
                          if (DataB0.Tmr3RxFqSwitchLost)
                          {
                              //FqRXCount = RXreceiveFQ;
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

    }
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
                //BTokMsg = 0x80  | CheckPacket(BTqueueIn, BTqueueInLen);
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
    // 300 bits / (1000000/s) = 300 mks
    // 6 mks IRQ time
    // total = 10 + 130 + 300 + 6 = 446mks 
    // on a processor 32MHz with 128 prescaler 4 counts=64mks
    // 446mks = 28 counts
    //
    // calculation of a time to upload 32 bytes:
    // 7682 cycles = 960 mks 
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
        i =    32    -sizeof(PacketStart) -2    -1  -BTqueueOutCopyLen;
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
                 //       0  CRCO CRC encoding scheme '0' - 1 byte '1' û 2 bytes
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

#ifndef SKIP_CALC_TX_TIME 
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
                 //       0  CRCO CRC encoding scheme '0' - 1 byte '1' û 2 bytes
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
        if (DataB0.Timer1SwitchTX)   // Mode  switchRX ==1 && switchTX ==1
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
//    if (BTType == 1)
//    {
//        if (DataB0.Timer3SwitchRX)
//        {
//            // freq switch done without switching off RX amplifier
//DO_SWITCHFQ:
//            bitclr(PORT_BT,Tx_CSN);
//            BTStatus= SendBTcmd(0x25); // 0010  0101 command W_REGISTER to register 00101 == RF_CH
//            SendBTbyte(iFQ);  // set channel = FQ1
//            BTFQcurr = iFQ;
//            bitset(PORT_BT,Tx_CSN);
//        }
//        else
//        {
//GET_STATUS:
//            bitclr(PORT_BT,Tx_CSN);
//            BTStatus= SendBTcmd(0xff);
//            bitset(PORT_BT,Tx_CSN);
//        }
//    }
//    else if (BTType == 2) // TX
//    {
//        if (DataB0.Timer1SwitchTX)
//            goto DO_SWITCHFQ;
//        //if (TXSendOverFQ == 1) // on TX
//        //{
//            iFQ = Freq1;
//            goto DO_SWITCHFQ;
//        //}
//        goto GET_STATUS;
//    }
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
                 //       0  CRCO    CRC encoding scheme '0' - 1 byte '1' û 2 bytes
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
       SendBTbyte(0b00100110);  // 0 - continues carrier; 0; 1(0-force PLL)0 - 250 kbs;11 - 0dBm ; 0
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
                       //           0 Pipe not used 1 = 1 byte à32 = 32 bytes
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
                 //       0  CRCO CRC encoding scheme '0' - 1 byte '1' û 2 bytes
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
	WORD i= 8;
    unsigned char Data = 0;
    do 
    {
        Data <<=1;
        if (cmd & 0x80)
		    PORT_BT.Tx_MOSI = 1;
        else
            PORT_BT.Tx_MOSI = 0;
		PORT_BT.Tx_SCK = 1;
        if (bittest(PORT_BT, Rx_MISO))
            Data |= 1;

        cmd <<= 1;
		PORT_BT.Tx_SCK = 0;
    } while(--i);
    return Data;
}

void SendBTbyte(unsigned char cmd)
{
	WORD i;
	i = 8;
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
    unsigned char Data = 0;
	WORD i = 8;
    do 
    {
        Data <<= 1;
        PORT_BT.Tx_SCK = 1;
		if (bittest(PORT_BT,Rx_MISO))
			Data |= 1;

		PORT_BT.Tx_SCK = 0;

    } while(--i);
    return Data;
}


