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
///
// mesurement of TTT can be done on uncommenting MEASURE_EXACT_TX_TIME, need to account that it is transfer of 32 bytes in DEBUG_LED mode
// protocol diagram for mode TX3 & RX3
// for 2321 time TTT == RRR == 0x2ADD = 10973 op = 0.001371625 sec
//          time 123456789AB = (TX = 10973 op = 0.001371625)+(TO= 93*128 = 11904 op = 0.001488)+(Packet prep = 7472) = total = 30349 = 0.003793625 sec
// for 25K20time TTT == RRR ==        = 7051 op = 0.000459 sec
//          time 123456789AB = (TX = 7344 op = 0.0004406875 sec)+(TO= 47*256 = 12032 op = 0.000752 sec ) + (Packet prep = 7724) = total = 27100 = 0.00169375sec
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
//#define MEDIAN_TIME 0x4000
//#define MEDIAN_TIME 0x2000
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

#include "Var001.h"


#include "commc0.h"


#include "Var002.h"
#if 1
void DebugLock(UWORD StopCount);
#endif
void ClockOutByte(void);

void SetupBT(unsigned char SetupBtMode);
unsigned char SwitchFQ(unsigned char iFQ);
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
unsigned char getchExternal(void);
unsigned char getchInternal(void);
/*
unsigned char getchBTComIn(void);
*/
void putchInternal(unsigned char simbol);
void putchExternal(unsigned char simbol);
void PrgUnit(unsigned char bTop, UWORD Addr, UWORD uwTillAddr);
void InitModem(void)
{
    ATCMDStatus = 0;
    ATCMDStatusAddon = 0;
    ATCMD = 0;
    FqTX = Freq1;
    FqTXCount = 0;
    //BTFQcurr = Freq1;
        
    //RXreceiveFQ = 0;
    //TXSendOverFQ = 0;
    FqRXCount = 0;
    FqRX = Freq1;
    FqTXCount = 0;
    FqTX = Freq1;
    BTType = 0;
    BTokMsg = 0xff;

    DataB0.Tmr3DoneMeasureFq1Fq2 = 0;
    //DataB0.Time3JustDone = 0;
    DataB0.Tmr3DoMeausreFq1_Fq2 = 0;
    DataB0.Tmr3Run = 0;
    DataB0.Timer3OutSyncRQ = 0;
    // 1 = round robin on FQ1->Fq2->Fq3
    // 0 = working on FQ1 only
    DataB0.Timer3Ready2Sync = 0;
    DataB0.TransmitESC = 0;
    //DataB0.Timer3SkipFQ2 = 0;
    //DataB0.Timer3SkipFQ3 = 0;
    DataB0.RXLoopAllowed = 1;
    DataB0.RXLoopBlocked = 0;
    DataB0.RXMessageWasOK = 0;
    DataB0.Timer1LoadLowOpponent = 0;
    DataB0.BTExternalWasStarted = 0;
    BTExternal.iEntry = 0;
	BTExternal.iExit = 0;
    BTExternal.iQueueSize = 0;

    Timer1Id = 0;
}
#ifdef NON_STANDART_MODEM
// adr 0z000000 (0) == Adr2B = 0x00  (var Ard2BH = 0x00)
// adr 0x001000 (4K)== Adr2B = 0x10  (var Ard2BH = 0x00)
// adr 0x002000 (8k)== Adr2B = 0x20  (var Ard2BH = 0x00)
// adr 0x010000 (65536)Adr2B = 0x00  (var Adr2BH = 0x01)


void Erace4K(unsigned char Adr2B);
#endif
#ifdef DEBUG_SIM
unsigned int Sendbtcmd;
#endif

void main()
{
    unsigned char bWork;
    unsigned char *ptrMemset;
    Reset_device();
#ifdef DEBUG_SIM
     Sendbtcmd = 0;
#endif

    // needs to check what is it:
    //if (TO) // Power up or MCLR
    {
        // Unit == 1 (one) is ADC and unit == 2 (two) is DAC 
        // Unit == 3 Gyro
        UnitADR = MY_UNIT;//'5'; // BT communication module
        //UnitADR = '8';
        //UnitADR = '4';
#include "commc6.h"

        // TBD: what are default values????
        // needs to grab it from last compilation???
        Config01 = eeprom_read(0x30);
        Freq1 = eeprom_read(0x31);
        Freq2 = eeprom_read(0x32);
        Freq3 = eeprom_read(0x33);
        Addr1 = eeprom_read(0x34);
        Addr2 = eeprom_read(0x35);
        Addr3 = eeprom_read(0x36);
        DistTimeOut  = ((UWORD)(eeprom_read(0x37))<<8);
        DistTimeOut |=  (UWORD) eeprom_read(0x38);
        InitModem();

        DataB3.FlashCmd = 0;
        DataB3.FlashCmdLen = 0;
        DataB3.FlashRead = 0;
        DataB3.FlashCmdNeedWait = 0;
        OldFlashCmd = 0;

        
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
    BTFlags.RxInProc = 0;
    SkipRXTmr3 = 0;
    ESCCount = 0;
#ifdef SKIP_CALC_TX_TIME
    TMR1ON = 0;              // stop timer measure it time btw send Fq1 -> Fq2
    DataB0.Timer1Meausre = 0;
    DataB0.Timer1Count = 1;

    Tmr1LoadHigh = 0xffff;
    Tmr1LoadLow = 0x8975;      // timer1 interupt reload values 
    Tmr1TOHigh = Tmr1LoadHigh;
    SetTimer1(Tmr1LoadLow);
#else
    DataB0.Timer1Count = 0;
#endif
    

#ifdef NON_STANDART_MODEM
    //Adr2BH = 0;
    //Erace4K(0);
    // TBD: search for the begining of a queue

    // needs to search for packets and process it if they was stored
#endif

    T2Byte0=0;

    T2Count1=0;
    T2Byte1=0;

    T2Count2=0;
    T2Byte2=0;

    T2Count3=0;
    T2Byte3=0;
    TMR2Count = 0;
    Timer1HCount = 0;
    Timer3HCount = 0;
    DataB0.EnableFlashWrite = 0;
    DataB0.UpgradeProgFlags = 0;
    UpgradeProgStatus = 0;
/*
    DataB0.CRCCalcFals = 0;
    CRCFlashCalcStatus = 0;
*/
    DataB3.FlashCmdShort = 0;
    DataB0.ExcgFlashcmd = 0;
    DataB0.ExcgRcvCmd = 0;
    ExchCmdStatus = 0;
    ExchSendStatus = 0;

    BTInternal.iEntry = 0;
    BTInternal.iExit = 0;
    BTInternal.iQueueSize = 0;

    BTExternal.iEntry = 0;
    BTExternal.iExit = 0;
    BTExternal.iQueueSize = 0;
/*
    BTComIn.iEntry = 0;
    BTComIn.iExit = 0;
    BTComIn.iQueueSize = 0;
*/
#ifdef DEBUG_LED
    DEBUG_LED_OFF;
    DebugLedCount = 0;
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
    ptrMemset = &DistMeasure;
    for (bWork = 0; bWork <sizeof(DistMeasure);bWork++)
    {
        *ptrMemset++=0;
    }
    iAdjRX = 0;
    SetTimer3(0);
    DataB0.ExcgFlashcmd = 0;
    DataB0.ExcgRcvCmd = 0;
    DataB0.ExcgSendinProgress = 0;

    ShowMessage();
    INT0_ENBL = 1;
    /*ptrMemset = &BTqueueOut[0];
    for (bWork = 0; bWork <32;bWork++)
    {
        *ptrMemset++=bWork;
    }*/
    DataB0.EnableFlashWrite = 1;
    DataB0.EnableFlashWrite = 0;
    ProgAdrBH = 0;
    wProgAddr = 0;

    AdrBH= 0;
    wAddr = 0x8000;
    wProgMaxAddr = 0x2CA0;
    PrgUnit(ProgAdrBH, wProgAddr, wProgMaxAddr);

#ifdef DEFAULT_CALL_EARTH
    ATCMD = MODE_CALL_EARTH;     
#endif

#ifdef DEFAULT_CALL_LUNA
    Main.DoPing = 1;
    Main.ConstantPing = 1;
    PingAttempts = 2;
    ATCMD = MODE_CALL_LUNA_COM;
#endif
    ATCMD |= INIT_BT_NOT_DONE;
    PingDelay = PING_DELAY;

    //bitset(PORTA,4);
    //bitset(PORTA,3);
    //bitset(SSPCON,4);  // set clock high;
#include "commc7.h"


} // at the end will be Sleep which then continue to main







// for pic18f2321
#ifdef DEBUG_SIM
#define SPBRG_SPEED 2
#else
#define SPBRG_SPEED SPBRG_57600_64MHZ
#endif
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
#if 0
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
#endif // #if 0
#endif // NON_STANDART_MODEM


void putchExternal(unsigned char simbol)
{
    if (Main.SendWithEsc)
    {
        if (simbol == ESC_SYMB)
            goto PUT_ESC;

        if (simbol >= MIN_ADR)
        {
            if (simbol <= MAX_ADR)
            {
PUT_ESC:
                BTExternal.Queue[BTExternal.iEntry] = ESC_SYMB; // add bytes to a queue
                if (++BTExternal.iEntry >= BT_BUF)
                    BTExternal.iEntry = 0;
                BTExternal.iQueueSize++;
            }
        }
    }
    BTExternal.Queue[BTExternal.iEntry] = simbol; // add bytes to a queue
    if (++BTExternal.iEntry >= BT_BUF)
        BTExternal.iEntry = 0;
    BTExternal.iQueueSize++; // this is unar operation == it does not interfere with interrupt service decrement
}

void putchInternal(unsigned char simbol)
{
    BTInternal.Queue[BTInternal.iEntry] = simbol; // add bytes to a queue
    if (++BTInternal.iEntry >= BT_BUF)
        BTInternal.iEntry = 0;
    BTInternal.iQueueSize++; // this is unar operation == it does not interfere with interrupt service decrement
}
unsigned char getchExternal(void)
{
    unsigned char bRet = BTExternal.Queue[BTExternal.iExit];
    if (++BTExternal.iExit >= BT_BUF)
        BTExternal.iExit = 0;

    BTExternal.iQueueSize --;
    return bRet;
}

unsigned char getchInternal(void)
{
    unsigned char bRet = BTInternal.Queue[BTInternal.iExit];
    if (++BTInternal.iExit >= BT_BUF)
        BTInternal.iExit = 0;

    BTInternal.iQueueSize --;
    return bRet;
}
/*
unsigned char getchBTComIn(void)
{
    unsigned char bRet = BTComIn.Queue[BTComIn.iExit];
    if (++BTComIn.iExit >= BT_BUF)
        BTComIn.iExit = 0;

    BTComIn.iQueueSize --;
    return bRet;
}
*/


void Reset_device(void);
void ShowMessage(void);
#pragma rambank RAM_BANK_4
#include "addon.h"
#pragma rambank RAM_BANK_1
#include "ProcCMD.h"
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
     TMR3CS = 0;//TIMER3 = iTime;
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
void SetTimer2(void)
{
     //T2Byte0 =1;
     TMR2IF = 0; // clean timer1 interrupt
     TMR2IE = 1;  // enable timer1 interrupt

     // 0            bit 7 Unimplemented: Read as 0
     //  0000        bit 6-3 T2OUTPS<3:0>: Timer2 Output Postscale Select bits
     //              0000 = 1:1 Postscale
     //              0001 = 1:2 Postscale
     //              1111 = 1:16 Postscale
     //      1       bit 2 TMR2ON: Timer2 On bit
     //              1 = Timer2 is on
     //              0 = Timer2 is off
     //       11     bit 1-0 T2CKPS<1:0>: Timer2 Clock Prescale Select bits
     //              00 = Prescaler is 1
     //              01 = Prescaler is 4
     //              1x = Prescaler is 16
     TMR2 = 0;
     TMR2Count = 0;
     T2CON = 0b00000111;
     
}
unsigned char CheckSuddenRX(void)
{
    if (Main.ExtInterrupt)
        return 1;
    if (BTType == 1) // RX is
    {
        BTCE_low();  // Chip Enable Activates RX (now disable) == standby
        //bitclr(PORT_BT,Tx_CSN); // SPI Chip Select
        //BTStatus=SendBTcmd(0xff); //cmd = 0xff; = 1111 1111 command NOP to read STATUS register
        //bitset(PORT_BT,Tx_CSN); // set high
        //if (BTStatus & 0x40) // it is unprocessed interrupt on RX
        if (Main.ExtInterrupt)
        {
            Main.ExtInterrupt = 1;
            return 1;
        }
    }
    
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
    if (DataB0.BTExternalWasStarted)  // do not process anything from comm if BT received data are forwarded to loop
        return 0;
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
    if (ATCMD & MODE_CONNECT) // was connection esatblished
    {
        //   Main.SendOverLink == 0 => return from call back and process com queue
        if (!Main.SendOverLink)   // if command * was send then data has to be transferred to up/down link
            return 1;             // untill end of the packet
        if (Main.SendOverLinkStarted)   // COm started to fillup BT queue
            goto FILLUP_BT_BUFFER;
        if (BTqueueOutLen)        // if BTqueue is not empty
            return 0;             // skip to process any bytes from COM
FILLUP_BT_BUFFER:
        if (BTqueueOutLen >= BT_TX_MAX_LEN) // buffer full
        {
SET_FLAG:
            ATCMD |= SOME_DATA_OUT_BUFFER; // that will force transmit on next FQ1
            return 0;             // skip to process any bytes from COM
        }
        bByte = AInQu.Queue[AInQu.iExit]; // just pickup byte
        if (Main.SendOverlinkWasESC) // that is done only to account ESC
            Main.SendOverlinkWasESC = 0;
        else if (bByte == ESC_SYMB)       // that is done only to account ESC
            Main.SendOverlinkWasESC = 1;  // each ESC must be transmitted - to be processed on another end
        else if (bByte == MY_UNIT)
        {
            Main.SendOverLink = 0;
            Main.SendOverLinkStarted = 0;
            ATCMD |= SOME_DATA_OUT_BUFFER; // that will force transmit on next FQ1
            return 1;             // that will process end of message inside main CMD loop
        }
        getch();   // clean queue
        if (BTqueueOutLen ==0)
        {
            // one extra byte (a) '*' or (b) 'N' 
            if (Main.SendOverLinkAndProc)
                BTqueueOut[BTqueueOutLen] = bByte;
            else
                BTqueueOut[BTqueueOutLen] = '*';
            ++BTqueueOutLen;
            Main.SendOverLinkStarted = 1;
        }
        BTqueueOut[BTqueueOutLen] = bByte;
        if (++BTqueueOutLen >= BT_TX_MAX_LEN) // buffer full
            goto SET_FLAG;
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
unsigned char DoFqRXSwitch(void)
{
    unsigned char bByte;
    
    //TMR3ON = 0; // stop TMR3 for a little bit
    // or may be not this call comes after INT == that mean inside timer3 it will be not advanced
    // counter SkipRXTmr3 prevents from this
    // second place it is a timer0 == this is before timer3 started to work
    //
    ///////////////////////////////////////////////////////////////////////
    //  FqRXCount & FqRX 
    //  0           Freq1
    //  1           Freq2
    //  2           Freq3
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
unsigned char DoFqTXSwitch(void)
{

    //unsigned char bByte;
    
    //TMR3ON = 0; // stop TMR3 for a little bit
    // or may be not this call comes after INT == that mean inside timer3 it will be not advanced
    // counter SkipRXTmr3 prevents from this
    // second place it is a timer0 == this is before timer3 started to work
    //
    ///////////////////////////////////////////////////////////////////////
    //  FqTXCount & FqTX 
    //  0           Freq1
    //  1           Freq2
    //  2           Freq3
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
    //bByte = FqTX;
    // TBD may be needs to adjust timer 3 ???
    //TMR3ON = 1;
    return FqTX;//bByte;
}
unsigned char SwitchFQ(unsigned char iFQ)
{
    unsigned char bWork;
              bitclr(PORT_BT,Tx_CSN);
              bWork= SendBTcmd(0x25); // 0010  0101 command W_REGISTER to register 00101 == RF_CH
              SendBTbyte(iFQ);  // set channel = FQ1
              //BTFQcurr = iFQ;
              bitset(PORT_BT,Tx_CSN);
    return bWork;
}

///////////////////////////////////////////////////////////////////////////////////////////
//      
//          PRE PROCESSING CALL BACK
//
///////////////////////////////////////////////////////////////////////////////////////////
unsigned char CallBkMain(void) // 0 = do continue; 1 = process queues
{
    unsigned char bWork;
    ProcessExch();
    if (INT0_ENBL)
    {   
MAIN_INT_ENTRY:          
        if (Main.ExtInterrupt) // received interrupt == it can be TX or RX from BT
        {
            // clean prev TX, sudden RX, and MAX_RT interrupt before send
            bitclr(PORT_BT,Tx_CSN);
            BTStatus=SendBTcmd(0x27); // 0010 0111 command W_REGISTER =register is status = clean TX interrupt
            SendBTbyte(0x70); // clean TX RX MAX_RT interrupt
            bitset(PORT_BT,Tx_CSN);

            if (BTType == 1) // RX mode
            {
                if (DataB0.RXPktIsBad)  //that error happens only on read operation now!!!
                {
#if 0
                    if (T2Byte1 ==0)
                    {        
                        if (T2Byte0 == 20)
                        {
                            T2Count1=TMR2Count;
                            T2Byte1=TMR2;
                        }
                        else
                            T2Byte0++;
                    }
                    else if (T2Byte2 ==0)
                    {        
                        T2Count2=TMR2Count;
                        T2Byte2=TMR2;
                    } 
                    else if (T2Byte3 ==0)
                    {        
                        T2Count3=TMR2Count;
                        T2Byte3=TMR2;
                    }
#endif
                    Main.ExtInterrupt = 0;
                    DataB0.RXPktIsBad = 0;
                    // need to read bad message to clean it
                    bitclr(PORT_BT,Tx_CSN);
                    SendBTbyte(0xe2); // 1110  0010 command flash RX FIFO
                    bitset(PORT_BT,Tx_CSN);

                    //bitclr(PORT_BT,Tx_CSN);
                    //SendBTbyte(0x27); // 0010 0111 command W_REGISTER =register is status = clean RX interrupt
                    //SendBTbyte(0x40); // clean RX interrupt
                    //bitset(PORT_BT,Tx_CSN);

                    if (DataB0.RXPkt2IsBad)  // that is exact case when on fq2 listenning was an error 
                    {
                        // (a) connect must be calceled
                        ATCMD &= (MODE_CONNECT^0xff);
#ifdef DEBUG_LED_CALL_EARTH
                        ATCMD &= (RESPONCE_WAS_SENT ^0xff);
#endif
                        // (b) packet to transfer must be calceled
                        ATCMD &= (SOME_DATA_OUT_BUFFER ^0xff);
                        BTqueueOutLen = 0;
                        // (c) switch to listen on FQ1 
                        BTCE_low();  // Chip Enable (RX or TX mode) now disable== standby
                        FqRXCount = 0;
                        FqRX = Freq1;
                        BTStatus = SwitchFQ(FqRX);
                        if (BTStatus & 0x40)
                        {
                            Main.ExtInterrupt = 1;
                        }
                        DataB0.RXPkt2IsBad = 0;
                    }    

                    BTCE_high();          // continue listeniong on FQX
                    return 1;
                }
            }

            // what it was ?
            //BTCE_low();  // Chip Enable Activates RX or TX mode (now disable)

            //bitclr(PORT_BT,Tx_CSN); // SPI Chip Select
            //BTStatus=SendBTcmd(0xff); //cmd = 0xff; = 1111 1111 command NOP to read STATUS register
            //bitset(PORT_BT,Tx_CSN); // set high
MAIN_INT:
            Main.ExtInterrupt = 0;

            //if (BTStatus & 0x40) // RX interrupt
            if (BTType == 1) // RX mode
            {

                //putch('r');
                // receve timing OK message dial
                // <receive = 442mks><6mks IRQ> <1051/525 mks process on each FQ><ok msg 102/51mks>
                // <receive = 442mks><6mks IRQ> <1051/525 mks process on each FQ><not msg 102/51mks>
                // <receive = 442mks><6mks IRQ> <1051/525 mks process on each FQ><ok msg 102/51mks>|<ok msg data 1602/801mks>
                // max process = 442mks + 6 + + 102/51 + 2653/1327 = 3207/1826 mks
                // on FQ3 with max correction :
                // <receive = 442mks><6mks IRQ> <1051/525 mks process on each FQ><correction 3447/1723mks>ok msg 102/51mks>|<ok msg data 1602/801mks>
                //TMR0ON = 0;
        
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
                    ProcessBTdata(); // call will be processed only one (first) time 
                }
                if (BTStatus & 0x40)
                {
                    // TBD: check the case:
                    if (!Main.ExtInterrupt)
                        Main.ExtInterrupt = 1;
                    //DataB0.RXPktIsBad = 1;
                }    
            }

            //if (BTStatus & 0x20) // TX interrupt comes before TMR1 interrupt. Switch FqTXCount done inside TMR1 interrupt 
            if (BTType == 2) // TX mode
            {
                if (BTStatus & 0x40) // was RX interrupt
                {
                    bitclr(PORT_BT,Tx_CSN);
                    SendBTbyte(0xe2); // 1110  0010 command flash RX FIFO
                    bitset(PORT_BT,Tx_CSN);
                }
                if (BTStatus & 0x20)
                {
#if 0
                            DebugLock(6);
#endif

                    // transmit timing with setup:
                    // <setup = 401.mks> <upload = 960/480 mks> <transmit = 442mks><IRQ 6mks> <dealy XXX>
                    // transmit timeing without setup:
                    // <upload = 960/480 mks> <transmit = 442mks> <IRQ 6mks> <dealy XXX>
                    // FqTXCount == next TX frequency
                    if (FqTXCount) // needs to send copy over FQ2 or FQ3
                    {
                        // TX initiated by TMR1 interrupt and Transmit function just prepear everything
                        // flag used for initial calculation of time btw FQ1-FQ@
                        if (DataB0.Timer1Count) 
                            goto NEXT_TRANSMIT;
                        // difference btw receive process (442mks + 6 + + 102/51 + 2653/1327 = 3207/1826 mks) and transmit upload (<upload = 960/480 mks> <transmit = 442mks> <IRQ 6mks>+xxx) xxx= 2247(1799??)/898 mks = 141(112??)/56 counts on 32MHz with 128 prescaler
                        //SetTimer0(0xff73); // delay to accomodate tranmsmit and receive process difference send waits when recive will be done
                        SetTimer0(DELAY_BTW_SEND_PACKET);  // that control delay to accomodate RX processing time
                    }
                    else // finished with FQ3 now needs go back to RX mode
                    {
                        if (!(ATCMD & SOME_DATA_OUT_BUFFER)) // if nothing in BT output queue then switch to RX 
                        {
                            if (ATCMD & MODE_CALL_EARTH)
                            {
                                if (!(ATCMD & RESPONCE_WAS_SENT))
                                    ATCMD &= (RESPONCE_WAS_SENT ^0xff);
                            }
                            DataB0.RXLoopBlocked = 0;
                            DataB0.RXLoopAllowed = 1;
#if 0
                            DebugLock(2);
#endif

                            SwitchToRXdata();
                            if (!(ATCMD & MODE_CONNECT)) // connection was not established == earth get responce from luna
                            {
                                SetTimer0(DELAY_BTW_NEXT_DIAL); // 0x0000 == 4 sec till next attempt for earth to dial luna
                            }
                        }
                        // othrwise it will be next TX - bytes are ready in BT output queue     
                        else
                        {
                            BTCE_low();
                        }
                    }
                }
            }
        }

        else  if (I2C.Timer0Fired) // no interrup yet == can be a timeout
        {
            I2C.Timer0Fired = 0;
            if (Timer1Id == DELAY_BTW_NEXT_DIAL)
            {
                if (!(ATCMD & MODE_CONNECT))
                { 
                    ATCMD &= (0xff ^MODE_DIAL); // this will repeat dial cubsat attempt
                } 
                // RX now on FQ1 listening indefinatly
            }
            else if (Timer1Id == DELAY_BTW_SEND_PACKET) // timeout on TX in a progress == was send FQ1 or FQ2 packets
            {
                //nop();nop();nop();//nop();//nop();//nop();//nop();//nop();
                //nop();//nop();//nop();//nop();//nop();//nop();//nop();//nop();
                // amlount of NOP calculated such way:
                // the best stable last digit on the Tmr1LoadLow
                // and last bit of Tmr1LoadLow is 1;
NEXT_TRANSMIT:
                TransmitBTdata();
            }
        }
        else if (DataB0.Tmr3Inturrupt) // time to switch frequency on RX operation
        {
            // in case DataB0.Timer3OutSyncRQ==1 it will be no TMR3 (RX) interrupts 
            // and FqRXCount will stay zero
            // in another case (normal sync FQ1-FQ2 measured) needs to advance FQ to a next listenning 
            DataB0.Tmr3Inturrupt = 0;
#if 0
            DebugLock(12);
#endif

            if (BTType == 1) // RX mode
            {
                // for check it will stop listenning ->BTCE_low()
                if (CheckSuddenRX())  // just by luck or delay it was RX of the packet
                {   // if it was TMR3 int then FQ was switched to next - now needs to restore previous FQ value
                    if (FqRXCount==0)
                    {
                        FqRXCount = 2;
                        FqRX = Freq3;
                    }
                    else
                    {
                        if (--FqRXCount == 1)
                            FqRX = Freq2;
                        else
                            FqRX = Freq1;
                    }
                    IntRXCount = FqRXCount;
                    goto MAIN_INT_ENTRY;//MAIN_INT;
                }
                // BTCE_low() now!!
                BTStatus = SwitchFQ(DoFqRXSwitch()); // FqRXCount points on next FQ after switch
                if (BTStatus & 0x40)
                {
                    // TBD - needs to check - does such case exsist?? normally it should not
                    Main.ExtInterrupt = 1;
                    IntRXCount = FqRXCount;
                    goto MAIN_INT_ENTRY;//MAIN_INT;
                }
                if (DataB0.Tmr3DoneMeasureFq1Fq2)
                {
                    if (FqRXCount == 0)
                    {
                        if (DataB0.RXLoopAllowed) // if it was request to TX then need to switch off round-robin
                        {
                            BTCE_high();          // continue listeniong on FQ1
                            DataB0.RXLoopBlocked = 0;
                        }
                        else                          // do not initate listenning
                            DataB0.RXLoopBlocked = 1; // listenning was blocked
                    }
                    else
                        BTCE_high();          // continue listeniong on FQ2-FQ3
                }
                else
                    BTCE_high();          // continue listeniong on FQ1
            }
            //else
            //    DoFqRXSwitch();
               
        }

        
        // check does it needs to transmit something?
        if (ATCMD & SOME_DATA_OUT_BUFFER)
        {
            if (DataB0.Timer1Count) // first TX on  FQ1 & FQ@ was done == timer 1 is set
            {
                if (FqTXCount == 0) // only when next TX will be on Fq1/
                {
                    // does RX get FQ1 and FQ2 packets ? if YES then TMR3 switches RX and loop can be blocked
                    if (DataB0.Tmr3DoneMeasureFq1Fq2)
                    {   ////////////////////////////////////////////////////////////////////////////////////////////
                        // case 1 == TX FQ1-FQ2 done and RX FQ1-FQ2 was successfull
                        if (DataB0.Timer3OutSyncRQ)
                            goto TIMING_CHECK;
                        if (!BTFlags.RxInProc)
                            goto TIMING_CHECK; 
                        
                        if (DataB0.RXLoopBlocked) // only when round-robin RX blocked
                        {
TIMING_CHECK:
                            Time1Left = TIMER1;
                            // value 0xffff-8000 =0xE0bf - that is max value when TX will be possible
                            // (TO= 93*128 = 11904 op = 0.001488)+(Packet prep = 7472=0.000467)
                            // 1 char = 0.0002sec TO= 93*128 = 11904 == 3.75 char
                            // allow to get 3 char 0.0002*3 *16,000,000= 9600 cycles
                            // 0xffff - (8000+10000) = 0xB9AF
                            // all time btw FQ1 FQ2 is 26807 = 0.0016754375 = 8.3 char all 3 freq = 25 char
                            if (Time1Left< MAX_TX_POSSIBLE)
                            {
                                if (BTqueueOutLen >= BT_TX_MAX_LEN)
                                    goto INIT_TX;
                                if (Time1Left > MIN_TX_POSSIBLE)
                                {
INIT_TX:
                                    //if (T2Byte0 ==0)
                                    //{
                                    //    //SetTimer2();
                                    //    T2Byte0 = 1;
                                    //}
                                    if (Main.ExtInterrupt)
                                        goto MAIN_INT_ENTRY;
                                    BTCE_low();
                                    TransmitBTdata();
                                    ATCMD &= (0xff ^SOME_DATA_OUT_BUFFER);
                                }
                            }
                        }
                        else
                            DataB0.RXLoopAllowed = 0;
                    }
                    else // for RX was not measured time btw FQ1 and FQ2 yet
                    {   /////////////////////////////////////////////////////////////////////////////////////////////
                        // case 2 == TX FQ1-FQ2 done and RX FQ1-FQ2 was NOT done yet
                        if (FqRXCount == 0) // only when next TX will be on Fq1
                        {
                            // for check it will stop listenning ->BTCE_low()
                            if (CheckSuddenRX())  // just by luck or delay it was RX of the packet
                            {
                                // interrupt for some reason was missed
                                DataB0.Tmr3DoMeausreFq1_Fq2 = 1;
                                SetTimer3(0);
                                DataB0.Tmr3Run = 0;
                                goto MAIN_INT;
                            }
                            goto TIMING_CHECK;
                        }
                    }
                }
                else // (FqTXCount != 0) // next TX will be on Fq2/Fq3 == it is impossible to start TX does not matter how
                {
                    // does RX get FQ1 and FQ2 packets ? if YES then TMR3 switches RX and loop must be blocked
                    if (DataB0.Tmr3DoneMeasureFq1Fq2)
                    {
                        DataB0.RXLoopAllowed = 0;
                    }
                }    
            }
            else // first time TX over FQ1-FQ2 was not set yet == needs to send  
            {
                if (FqTXCount == 0) // only when next TX will be on Fq1/
                {
                    if (DataB0.Tmr3DoneMeasureFq1Fq2)
                    {   //////////////////////////////////////////////////////////////////////////////////////////////
                        // case 3 == TX was not done but RX FQ1-FQ2 was successfull
                        // that will yeld RX and TX postponed
                        //if (!BTFlags.RxInProc)
                        //    goto TIMING_CHECK;

                        if (DataB0.RXLoopBlocked) // only when round-robin RX blocked
                            goto INIT_TX;
                        else
                            DataB0.RXLoopAllowed = 0;
                    }
                    else
                    {   //////////////////////////////////////////////////////////////////////////////////////////////
                        // case 4 == TX was not done and no RX FQ1-FQ2
                        if (BTType == 1)  // is ir RX now??
                        {
                            if (FqRXCount == 0) // is it RX over FQ1 ??  
                            {
                                //TMR0ON = 0;
                	            goto INIT_TX;
                            }
                        }
                    }    
                }
                else
                {
                    // does RX get FQ1 and FQ2 packets ? if YES then TMR3 switches RX and loop must be blocked
                    if (DataB0.Tmr3DoneMeasureFq1Fq2)
                    {
                        DataB0.RXLoopAllowed = 0;
                    }
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
	CLKDIVbits.PLLPOST=0;		// N2=2 PLL VCO Output Divider Select bits (also denoted as N2, PLL postscaler)
                                // 00 = Output/2
                                // 01 = Output divided by 4 (default)
                                // 10 = Reserved
                                // 11 = Output divided by 8
                                // bit 5 Unimplemented: Read as
	CLKDIVbits.PLLPRE=0;		// N1=2 PLL Phase Detector Input Divider bits (also denoted as N1, PLL prescaler)
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
                                        // bit 4 Unimplemented: Read as 0
                                        // bit 3 CF: Clock Fail Detect bit (read/clear by application)
                                        // bit 2 Unimplemented: Read as 0
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
    //bitclr(OPTION,RBPU_); //Each of the PORTB pins has a weak internal pull-up. A-
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
#ifdef DEBUG_SIM
   OSCCON = 0b11110000; //OSCILLATOR CONTROL REGISTER (ADDRESS 8Fh)
#else
   OSCCON = 0b11111000; //OSCILLATOR CONTROL REGISTER (ADDRESS 8Fh)
#endif
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
    PORTB = 0b00000001;  // nothing happened with amplifiers BT_TX,BT_RX=low

	
    
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
                         //    Note 1: Bit resets to 0 with Two-Speed Start-up mode and LP, XT or HS selected as the
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



    // 251 223
    // a=251 b=223 max=128
    // 0=1 1=223 2=165 3=187 4=89 5=135 6=93 7=3 8=241 9=239 10=85 11=11 12=201 13=23 14=141 15=211 
    // 16=225 17=255 18=5 19=91 20=57 21=167 22=189 23=163 24=209 25=15 26=181 27=171 28=169 29=55 30=237 31=115 
    // 32=193 33=31 34=101 35=251 36=25 37=199 38=29 39=67 40=177 41=47 42=21 43=75 44=137 45=87 46=77 47=19 
    // 48=161 49=63 50=197 51=155 52=249 53=231 54=125 55=227 56=145 57=79 58=117 59=235 60=105 61=119 62=173 63=179 
    // 64=129 65=95 66=37 67=59 68=217 69=7 70=221 71=131 72=113 73=111 74=213 75=139 76=73 77=151 78=13 
    // 79=83 80=97 81=127 82=133 83=219 84=185 85=39 86=61 87=35 88=81 89=143 90=53 91=43 92=41 
    // 93=183 94=109 95=243 96=65 97=159 98=229 99=123 100=153 101=71 102=157 103=195 104=49 105=175 
    // 106=149 107=203 108=9 109=215 110=205 111=147 112=33 113=191 114=69 115=27 116=121 117=103 118=253 
    // 119=99 120=17 121=207 122=245 123=107 124=233 125=247 126=45 127=51 128=1 

    // 239 139
    // a=239 b=139 max=128
    // 0=1 1=139 2=197 3=247 4=153 5=19 6=189 7=159 8=113 9=91 10=245 11=7 12=137 13=99 14=109 15=47 
    // 16=225 17=43 18=37 19=23 20=121 21=179 22=29 23=191 24=81 25=251 26=85 27=39 28=105 29=3 30=205 31=79 
    // 32=193 33=203 34=133 35=55 36=89 37=83 38=125 39=223 40=49 41=155 42=181 43=71 44=73 45=163 46=45 47=111 
    // 48=161 49=107 50=229 51=87 52=57 53=243 54=221 55=255 56=17 57=59 58=21 59=103 60=41 61=67 62=141 63=143 
    // 64=129 65=11 66=69 67=119 68=25 69=147 70=61 71=31 72=241 73=219 74=117 75=135 76=9 77=227 78=237 
    // 79=175 80=97 81=171 82=165 83=151 84=249 85=51 86=157 87=63 88=209 89=123 90=213 91=167 92=233 
    // 93=131 94=77 95=207 96=65 97=75 98=5 99=183 100=217 101=211 102=253 103=95 104=177 105=27 
    // 106=53 107=199 108=201 109=35 110=173 111=239 112=33 113=235 114=101 115=215 116=185 117=115 118=93 
    // 119=127 120=145 121=187 122=149 123=231 124=169 125=195 126=13 127=15 128=1 

    // 227 151
    // a=227 b=151 max=128
    // 0=1 1=151 2=229 3=19 4=217 5=255 6=29 7=27 8=241 9=39 10=149 11=227 12=73 13=15 14=77 15=107 
    // 16=225 17=183 18=69 19=179 20=185 21=31 22=125 23=187 24=209 25=71 26=245 27=131 28=41 29=47 30=173 31=11 
    // 32=193 33=215 34=165 35=83 36=153 37=63 38=221 39=91 40=177 41=103 42=85 43=35 44=9 45=79 46=13 47=171 
    // 48=161 49=247 50=5 51=243 52=121 53=95 54=61 55=251 56=145 57=135 58=181 59=195 60=233 61=111 62=109 63=75 
    // 64=129 65=23 66=101 67=147 68=89 69=127 70=157 71=155 72=113 73=167 74=21 75=99 76=201 77=143 78=205 
    // 79=235 80=97 81=55 82=197 83=51 84=57 85=159 86=253 87=59 88=81 89=199 90=117 91=3 92=169 
    // 93=175 94=45 95=139 96=65 97=87 98=37 99=211 100=25 101=191 102=93 103=219 104=49 105=231 
    // 106=213 107=163 108=137 109=207 110=141 111=43 112=33 113=119 114=133 115=115 116=249 117=223 118=189 
    // 119=123 120=17 121=7 122=53 123=67 124=105 125=239 126=237 127=203 128=1 

void BTbyteCRC(unsigned char bByte)
{
    wCRCupdt(bByte);
    SendBTbyte(bByte);  
}

    // 251 223
void BTbyteCRCm1(unsigned char bByte)
{
#ifdef      _18F2321_18F25K20
    if (!BTFlags.CRCM8F)
        CRCM8TX *= 251;    
    else
        CRCM8TX *= 223;
    #asm
    BTG BTFlags,3,1
    #endasm
#else
    WREG=CRCM8TX;
    if (!BTFlags.CRCM8F)
    {
        CRCM8TX = (WREG * 251)%256;
        BTFlags.CRCM8F = 1;
    }
    else
    {
        CRCM8TX = (WREG * 223)%256;
        BTFlags.CRCM8F = 0;
    }
#endif
    CRCM8TX ^= bByte;
    SendBTbyte(CRCM8TX);
    CRCM8TX|=1;
}
void BTbyteCRCM1(unsigned char bByte)
{
    wCRCupdt(bByte);
    BTbyteCRCm1(bByte);
}

// 239 139
void BTbyteCRCM2(unsigned char bByte)
{
    //wCRCupdt(bByte);
#ifdef      _18F2321_18F25K20
    if (!BTFlags.CRCM8F)
        CRCM8TX *= 239;    
    else
        CRCM8TX *= 139;
    #asm
    BTG BTFlags,3,1
    #endasm
#else
    WREG=CRCM8TX;
    if (!BTFlags.CRCM8F)
    {
        CRCM8TX = (WREG * 239)%256;
        BTFlags.CRCM8F = 1;
    }
    else
    {
        CRCM8TX = (WREG * 139)%256;
        BTFlags.CRCM8F = 0;
    }
#endif
    CRCM8TX ^= bByte;
    SendBTbyte(CRCM8TX);
    CRCM8TX|=1;
}
// 227 151
void BTbyteCRCM3(unsigned char bByte)
{
#ifdef      _18F2321_18F25K20
    if (!BTFlags.CRCM8F)
        CRCM8TX *= 227;    
    else
        CRCM8TX *= 151;
    #asm
    BTG BTFlags,3,1
    #endasm
#else
    WREG=CRCM8TX;
    if (!BTFlags.CRCM8F)
    {
        CRCM8TX = (WREG * 227)%256;
        BTFlags.CRCM8F = 1;
    }
    else
    {
        CRCM8TX = (WREG * 151)%256;
        BTFlags.CRCM8F = 0;
    }
#endif
    CRCM8TX ^= bByte;
    SendBTbyte(CRCM8TX);
    CRCM8TX|=1;
}
#include "FixBtPkt.h"

//=========================================================================================================
//   received data -> to output
//=========================================================================================================
void ProcessBTdata(void)
{
//             0  1  2  3  4  5  6 
// RF ctrl pkt AA AA AA F0 CH LN CTRL CRC16 CRCM8
// RF data pkt AA AA AA F1 CH LN DATA CRC16 CRCM8


    unsigned char *ptrMy =&OutputMsg[0] ;
    unsigned char ilen;
    struct PacketStart *MyPacket;
    unsigned int BeginAddr;

    if (BTFlags.BT3fqProcessed)
        return;

    BTFlags.BT3fqProcessed = 1; // packet(s) is(are) fine == process done == flag to avoid process duplication packets
    MyPacket = ptrMy;   

    ilen = MyPacket->BTpacketLen;//ptrMy[6];
#ifdef DEBUG_LED
    //if (DebugLedCount == 0)
    {
        DEBUG_LED_ON;
        DebugLedCount = DEBUG_LED_COUNT;
    }
#endif
    if (MyPacket->BTpacket == PCKT_DIAL)// (ptrMy[5] & PCKT_DIAL) // receved packet = dial call
    {
        //             0  1  2  3  4  5  6 
        // RF ctrl pkt AA AA AA F0 CH LN CTRL CRC16 CRCM8
        //    CTRL=  le r1 r2 r3 A1 A2 A3 F1 F2 F3 <TMR>
        if ((MyPacket->Type == 'e') || (MyPacket->Type == 'l'))
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
            if (MyPacket->Tmr1LLowOpponent !=0)
            {
                Tmr3LoadLowCopy = MyPacket->Tmr1LLowOpponent;
                DataB0.Timer1LoadLowOpponent = 1;
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
                        if (ATCMD & RESPONCE_WAS_SENT)
                            ;
                        else
                            ATCMD &= (RESPONCE_WAS_SENT ^0xff);
                    }
                    else
                    {
SEND_CONNECT:
                        PutsToUnit("\r\nCONNECT\r\n");
                    }
#ifdef DEBUG_LED_CALL_EARTH
                    ATCMD &= (RESPONCE_WAS_SENT ^0xff);
#endif
               }
            }
        }
        else if (MyPacket->Type == 'P') // ping packet - now need to send responce right away
        {
            if (MyPacket->Tmr1LLowOpponent !=0)
            {
                Tmr3LoadLowCopy = MyPacket->Tmr1LLowOpponent;
                DataB0.Timer1LoadLowOpponent = 1;
            }
            Main.PingRSPRQ = 1;
        }
        else if (MyPacket->Type == 'p') // responce on ping packet
        {
            if ((ptrMy[23] != 0) && (ptrMy[24] != 0))
            {
                Tmr3LoadLowCopy = (((UWORD)ptrMy[24])<<8);
                Tmr3LoadLowCopy |= ptrMy[23]; 
                DataB0.Timer1LoadLowOpponent = 1;
            }
            //Main.PingRQ = 1;
            ATCMD |= MODE_CONNECT;
           // output to com - BT input message has to go via output queue
           if (UnitFrom)
           {
                Main.SendWithEsc = 0;
                putchExternal(UnitFrom);
                Main.SendWithEsc = 1;
                //if (SendCMD)
                putchExternal('D'); // commnand 'D'istance
                ptrMy = &OutputMsg[7] ;
                ilen--;
                while(ilen>0)
                {
                    putchExternal(*ptrMy);
                    ptrMy++;
                    ilen--;
                }
                // now needs to send correspendend values from TX unit to be able calculate distance on DB server
                ptrMy = &DistMeasure;
                ilen = sizeof(DistMeasure);
                while(ilen >0)
                {
                    putchExternal(*ptrMy);
                    ptrMy++;
                    ilen--;
                }
                Main.SendWithEsc = 0;
                putchExternal(UnitFrom);
           }
        }
        ATCMD |= MODE_CONNECT;
        
    }
    else // another packets == data
    {
        //             0  1  2  3  4  5  6 
        // RF data pkt AA AA AA F1 CH LN DATA CRC16 CRCM8
        ptrMy+=6;
#ifdef NON_STANDART_MODEM
        // write good packet into FLASH memory for processing
        if (*ptrMy == '*') // pkt needs to be processed inside UNIT
        {
            ptrMy++;
            ilen--;
            while(ilen>0)
            {
                putchInternal(*ptrMy);
                ptrMy++;
                ilen--;
            }
        }
        else // pkt needs to be processed outside == in remode loop for user *ptrMy
        {
            Main.SendWithEsc = 0;
            putchExternal(*ptrMy);
            Main.SendWithEsc = 1;
            ptrMy++;
            ilen--;
            while(ilen>0)
            {
                putchExternal(*ptrMy);
                ptrMy++;
                ilen--;
            }
            Main.SendWithEsc = 0;
            putchExternal(OutputMsg[6]);
        }    

#else // NOT NON_STANDART_MODEM
        if (ATCMD & MODE_CALL_LUNA_COM)
        {
OUTPUT_DIRECTLY:
            /*
            if (BTokMsg == 0)
            {
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
#ifdef DEBUG_SIM
void BTCE_highTX(void)
{
    bitset(PORT_BT,Tx_CE);	// Chip Enable Activates RX or TX mode (now TX mode) 
}
#endif 
void BTCE_high(void)
{
    bitset(PORT_BT,Tx_CE);	// Chip Enable Activates RX or TX mode (now TX mode) 
}
#if 1

void DebugLock(UWORD StopCount)
{
                            if (++T2Byte3 == StopCount)
                            {
                                T2Byte1 = Timer1HCount;
                                T2Byte2 = TIMER1;
                            }
                            else if (T2Byte3 > StopCount)
                            {
                                T2Byte3 = StopCount+1;
                            }
}
#endif

// global value IntRXCount
void AdjDistance(void)
{
    DistMeasure.RXbTmr1H = DistMeasure.RXaTmr1H;
    DistMeasure.RXbTmr1 = DistMeasure.RXaTmr1;
    DistMeasure.RXaTmr1H = PossibleRXTmr1H;
    if (IntRXCount==1)
       DistMeasure.RXaTmr1H^= 0xf000;
    else if (IntRXCount==2)
       DistMeasure.RXaTmr1H^= 0xff00;
    if (res &0x80) // negative
        DistMeasure.RXaTmr1 = -res;
    else
        DistMeasure.RXaTmr1 = res;
    if (res)
        DistMeasure.RXaTmr1 *=128;
    if (res &0x80) // negative
    {
        if (iShift == 2)
            DistMeasure.RXaTmr1 -= 32;
        else if (iShift == 4)
            DistMeasure.RXaTmr1 -= 64;
        else if (iShift == 6)
            DistMeasure.RXaTmr1 -= 96;

        DistMeasure.RXaTmr1 = PossibleRXTmr1-DistMeasure.RXaTmr1;
        //DistMeasure.RXaTmr1 +=128;
    }
    else
    {
        if (iShift == 2)
            DistMeasure.RXaTmr1 += 32;
        else if (iShift == 4)
            DistMeasure.RXaTmr1 += 64;
        else if (iShift == 6)
            DistMeasure.RXaTmr1 += 96;
        
        DistMeasure.RXaTmr1 += PossibleRXTmr1;
        DistMeasure.RXaTmr1 -=96;    
    }
}
void AdjTimer3(void)
{
    if (DataB0.Timer3Ready2Sync)
    {
#if 0
        // CRCcmp is just working variable
        //CRCcmp = 0xffff - AdjustTimer3;
        //CRCcmp -= MEDIAN_TIME;
        CRCcmp = AdjustTimer3 + MEDIAN_TIME;
        CRCcmp = Tmr3LoadLowCopy - CRCcmp;
        CRCcmp -=4;
        if (CRCcmp > 10)
        {
            TMR3ON = 0;                       // stop timer3 (RX) for a moment
            //Tmr3LoadLow = Tmr3LoadLowCopy + CRCcmp;
            Tmr3LoadLow = CRCcmp; 
            TMR3ON = 1;                       // start timer (RX) back
        }
        DataB0.Timer3Ready2Sync = 0;
#else
        //AdjustTimer3 += 16;
        CRCcmp = 0xffff - AdjustTimer3;
        CRCcmp -= MEDIAN_TIME;
        if ((CRCcmp >= 0x200) && (CRCcmp <= 0xfc00))
        {
            Tmr3LoadLow = Tmr3LoadLowCopy + CRCcmp;
            if ((iAdjRX > 0) && (iAdjRX <32)) 
                AdjRX -= CRCcmp;
        }
        if (!DataB0.Timer1LoadLowOpponent)
        {
            if (iAdjRX == 0)
            {
                AdjRX = CRCcmp;
            }
            if (iAdjRX >= 64)
            {
                iAdjRX = 0;
                AdjRX = CRCcmp;
            }
            if (iAdjRX >= 32)
            {
            
                AdjRX = CRCcmp - AdjRX;
                if (AdjRX > 0x8000)
                {
                    AdjRX = 0xffff-AdjRX;
                    AdjRX /= (unsigned char) iAdjRX;
                    AdjRX = 0xffff-AdjRX;
                }
                else
                    AdjRX = AdjRX / (unsigned char) iAdjRX;
                DataB0.Timer3Ready2Sync = 0;
                iAdjRX = 0;
                Tmr3LoadLowCopy += AdjRX; 
                AdjRX = CRCcmp;
            }
        }
        //if (++AdjRX == 0x800)
        //{
        //   AdjRX = 0;
        //}
        DataB0.Timer3Ready2Sync = 0;
#endif
    }
}
//===========================================================================================
unsigned char ReceiveBTdata(void)
{
    unsigned char bret = 1;
   //unsigned char iCrc;
    unsigned char iFix;
    unsigned char bByte;
    unsigned char *ptrMy ;
    //CRC=0;
    //BTCE_low();  // Chip Enable (Activates RX or TX mode) now disable== standby

    // clean RX interrupt (in time of listelling)
    //bitclr(PORT_BT,Tx_CSN);
    //SendBTbyte(0x27); // 0010 0111 command W_REGISTER =register is status = clean RX interrupt
    //SendBTbyte(0x40); // clean RX interrupt
    //bitset(PORT_BT,Tx_CSN);

    //IntRXCount = FqRXCount;
    DataB0.RXMessageWasOK = 0;

    // read len of the RX packet
    bitclr(PORT_BT,Tx_CSN);
    BTStatus= SendBTcmd(0x60); // 0110  0000 command R_RX_PL_WID
    bret = GetBTbyte();
    bitset(PORT_BT,Tx_CSN);
    if (bret > 32) // error - length can not be bigger 32 bytes (special case from spec)
    {
        bitclr(PORT_BT,Tx_CSN);
        SendBTbyte(0xe2); // 1110  0010 command flash RX FIFO
        bitset(PORT_BT,Tx_CSN);
        // that will continue RX on the same frequency
        return 0;
    }
    if (IntRXCount == 0) // received over FQ1
    {
        ptrMy =&BTqueueIn[0];
INIT_FQ_RX:
        BTqueueInLen3 = 0;
        BTqueueInLen2 = 0;
        BTqueueInLen = 0;
        BTFlags.BT3fqProcessed = 0; // set flag that process was not done yet
        BTokMsg = 0xff;
        BTFlags.RxInProc = 1;
    }
    else if (IntRXCount == 1) // received over FQ2
    {
        ptrMy =&BTqueueIn2[0] ;
        if (!BTFlags.RxInProc)
           goto INIT_FQ_RX;
    }
    else if (IntRXCount == 2) // received over FQ3
    {
        ptrMy =&BTqueueIn3[0] ;
        if (!BTFlags.RxInProc)
           goto INIT_FQ_RX;
    }
    ptrMy[0] = 0;ptrMy[1] = 0;
 
    bitclr(PORT_BT,Tx_CSN);
    SendBTcmd(0x61); // 0110  0001 command R_RX_PAYLOAD
    for (i = 0; i< bret; i++)
    {
         bByte = GetBTbyte();
         if (i < LEN_OFFSET_INPUT_BUF)
         {
             ptrMy[i] = bByte;
         }
    }
    bitset(PORT_BT,Tx_CSN);

    
    // TBD: output data as it is over com2 with speed at least 150000 bits/sec - ground station only
    // return len of the packet
    iFix = BTFixlen(ptrMy, bret); // if it is posible to fix packet and packet was fixed i == 0


#if 0
    if (iFix)
    {
        DebugLock(4);
    }
#endif

#if 0
    if (iFix)
    {
                if (IntRXCount == 0)
                {
                   T2Count1++;
                   if (++T2Byte1 == 2)
                      T2Byte1 = 2;
                }
                else if (IntRXCount == 1)
                {
                   T2Count2++;
                   //if (T2Byte2 == 2)
                   //   T2Byte2 = 2;
                }      
                else
                {
                   T2Count3++;
                   if (++T2Byte3 == 2)
                      T2Byte3 = 2;
                }  
    }
#endif

    if (!DataB0.Tmr3DoneMeasureFq1Fq2)  // FQ1-FQ2 measurements was not done yet
    {

        if (IntRXCount == 0) 
        {
            if (iFix)
                goto LOOKS_GOOD;
            // message looks bad on FQ1 == continue to listen on F1
            BTCE_high(); // Chip Enable Activates RX or TX mode (now RX mode) 
            return 0;
        }
        else if (IntRXCount == 1) 
        {
            if (iFix)
            {
                DataB0.Tmr3DoneMeasureFq1Fq2 = 1; // that set TMR3 to properly switch RX 
                goto LOOKS_GOOD;                  // from that moment RX switched by TMR3
            }
            // stop TMR3 == mesaurements will be done on next two good packets over FQ1 and FQ2
            BTCE_low();  // Chip Enable (RX or TX mode) now disable== standby
            FqRXCount = 0;
            FqRX = Freq1;
            SwitchFQ(FqRX);
            TMR3ON = 0;  
            BTCE_high(); // Enable Activates RX on FQ1 
                         // INT over FQ1 will initiate TMR3 to meeasure time. 
            return 0;
        }
    }
    // in case when RX FQ1-FQ2 measure done
    // and if (i) (packet already can not be fixed)
    // that packet comes in moment when it can be good - needs to switch on next FQ
LOOKS_GOOD:  


    // that switch will update BTStatus and switch frequency

    // cases when it is possible to switch frequency:
    //                                                   imposible:::
    //  FqRXCount         0    1   1    2   2    0 |     0 1 2
    //  FqRXRealCount      1   1    2   2    0   0 |     2 0 1
    if (DataB0.Tmr3DoneMeasureFq1Fq2)
    {
        // out of sycn case: stay on FqRxCount = 0 indefinetly intill Ok packet
        if (DataB0.Timer3OutSyncRQ)
        {
            BTCE_high();
            goto SKIP_SWITCH_2;
        }
        // SYNC_DEBUG 4 if next if will be commented == will be no sync of FQ with FqRXRealCount on RX operation
        if (IntRXCount == 0 && FqRXRealCount == 2)
            goto SKIP_SWITCH;
        else if (IntRXCount == 1 && FqRXRealCount== 0)
            goto SKIP_SWITCH;
        else if (IntRXCount == 2 && FqRXRealCount== 1)
            goto SKIP_SWITCH;
    }
    BTCE_low();  // Chip Enable (RX or TX mode) now disable== standby
    //BTStatus = 
    SwitchFQ(DoFqRXSwitch()); // if it was RX over FQ1 than value RXreceiveFQ ==0
    //if (BTStatus & 0x40)
    //{
    //    Main.ExtInterrupt = 1;
    //}

SKIP_SWITCH:
    // now needs to deside - to continue to listen or need to do TX?
    // flag DataB0.RXLoopBlocked = 1 will set for such case 
    if (FqRXCount == 0)
    {
        if (DataB0.RXLoopAllowed) // if it was request to TX then need to switch off round-robin
        {
            BTCE_high();          // continue listeniong on FQ1
            DataB0.RXLoopBlocked = 0;
        }
        else                          // do not initate listenning
            DataB0.RXLoopBlocked = 1; // listenning was blocked
    }
    else
        BTCE_high(); // Chip Enable Activates RX or TX mode (now RX mode) 
  
SKIP_SWITCH_2:
         
    // next packet ready to receive (on next frequency) - now prcocessing  
    if (bret > LEN_OFFSET_INPUT_BUF)
        bret = LEN_OFFSET_INPUT_BUF;

    if (BTokMsg == 0xff) // if paket was not recevet yet correctly (i.e. FQ1 not evaluated, or FQ1 was bad, or FQ1 FQ2 was bad)
    {
        if (iFix)  // if packet possible to fix (and/or it was fixed by shift)
        {
            if (CheckPacket(ptrMy, bret) == 0)  //used IntRXCount  // now possible to do CRC check ???
            {
                if (PossibleRXTmr1 != 0)
                    AdjDistance();
                if (IntRXCount == 0)      // set OK messaghe only on FQ1
                {
                    
                    DataB0.RXMessageWasOK = 1;
                    // case: was out of synch but now get packet on FQ1
                    if (DataB0.Timer3OutSyncRQ)
                    {
                        // packet was Ok now need to restore RX timer3
                        AdjustTimer3 = Tmr3LoadLow -MEDIAN_TIME;
                        AdjustTimer3 += TIMER3;
                        TMR3H = (AdjustTimer3>>8);
                        TMR3L = (unsigned char)(AdjustTimer3&0xFF);
                        BTCE_low();  // Chip Enable (RX or TX mode) now disable== standby
                        SwitchFQ(DoFqRXSwitch()); // if it was RX over FQ1 than value RXreceiveFQ ==0
                        BTCE_high(); // Chip Enable Activates RX or TX mode (now RX mode)
                        FqRXRealCount = 1; 
                        DataB0.Timer3OutSyncRQ =0;
                        DataB0.Timer3Ready2Sync = 0;
                    }    
                }

                BTokMsg = IntRXCount;
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
                }
                OutSyncCounter = 250; // 2.5 sec no packets == switch for out of sync
            }
        }
    }
    else
    {
        if (DataB0.Tmr3DoneMeasureFq1Fq2)
        {
            if (CheckPacket(ptrMy, bret) == 0) // used IntRXCount
            {
#if 0
    if (iFix)
    {
                if (IntRXCount == 0)
                   T2Count1++;
                else if (IntRXCount == 1)
                   T2Count2++;
                else
                {
                   T2Count3++;
                }
    }
#endif

                AdjTimer3();
                OutSyncCounter = 250; // 2.5 sec no packets == switch for out of sync
            }
        }    
    }
    
    // huck - but who cares? = len of a packet in offset of 28 
    //if (i)
    //    bret = 0;
    ptrMy[LEN_OFFSET_INPUT_BUF] = iFix;//bret;



    if (IntRXCount == 2) // recevie over FQ3
    {

        //RXreceiveFQ = 0;
        //BTqueueInLen3 = bret;
        if (BTokMsg == 0xff) // needs to fix packets
        {
            // 2. attempt to find matching size for all 1-2-3
            // if matching was found then do fix by majority rools 2 bits from 3 == 1 -> result = 1 and etc.
            // result stored in 1 then CRC recalulation on 1 
            // 3. attempt to find matchiong size of any 1-2 or 2-3 or 3-1
            // reported len will be 2 bytes less (CRC) then original recieved len (28)
            if ((BTqueueInLen > 8) && (BTqueueInLen2 > 8) && (BTqueueInLen3 >8)) // all 3 matched size
            {
                BTokMsg = BTFix3();
                //BTokMsg = 0x80  | BTFix3();

                //BTokMsg = 0x80  | CheckPacket(BTqueueIn, BTqueueInLen);
            }
            else if ((BTqueueInLen >8 ) && (BTqueueInLen2 > 8)) // all 2 matched size FQ1 & FQ2
            {
                BTokMsg = BTFix2();
            }
            else if ((BTqueueInLen > 8) && (BTqueueInLen3 > 8)) // all 2 matched size FQ1 & FQ3
            {
                BTokMsg = BTFix2();
            }
            else if ((BTqueueInLen2 > 8) && (BTqueueInLen3 > 8)) // all 3 matched size FQ2 & FQ3
            {
                BTokMsg = BTFix2();
            }
            if (BTokMsg != 0xff)
            {
                AdjTimer3();
                OutSyncCounter = 250; // 2.5 sec no packets == switch for out of sync
            }

        }
    }
DONE_RX:
    //if (BTStatus & 0x40) // another unprocessed RX in a queue
    //{
    //     Main.ExtInterrupt = 1;
    //     TMR3ON =0;
    //     SkipRXTmr3++;
    //     TMR3ON =1;
    //}
    if (IntRXCount == 2)
    {
        BTFlags.RxInProc = 0;
    }
    //if (DataB0.Tmr3Inturrupt)
    //{
    //    DataB0.Tmr3Inturrupt = 1;
    //}
    return (BTokMsg != 0xff);
}
//===============================================================================================
void TransmitBTdata(void)
{
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
    //unsigned char i;
    
    unsigned char bBy;
    PORT_AMPL.BT_RX = 0;              // off RX amplifier
    nop();
    PORT_AMPL.BT_TX = 1;              // TX amplifier : TBD: is it enought time to start amplifier?
    
    if (BTqueueOutLen)
    {
        if (FqTXCount == 0) // send over FQ1
        {
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
                 //       0  CRCO CRC encoding scheme '0' - 1 byte '1' ϋ 2 bytes
                 //        1 PWR_UP 1: POWER UP, 0:POWER DOWN
                 //         0PRIM_RX	RX/TX control 1: PRX, 0: PTX

            bitclr(PORT_BT,Tx_CSN);
            BTStatus= SendBTcmd(0x20); // 001 0  0000 W_REGISTER to register 00000 == Configuration Register
            SendBTbyte(0x12);
            bitset(PORT_BT,Tx_CSN); // SPI Chip Select
            if (BTStatus & 0x60)
            {
                bitclr(PORT_BT,Tx_CSN);
                SendBTbyte(0x27); // 0010 0111 command W_REGISTER =register is status = clean TX interrupt
                SendBTbyte(0x70); // clean TX RX MAX_RT interrupt
                bitset(PORT_BT,Tx_CSN);
            }
            if (BTStatus & 0x40) // explanation: it is TX now any sudden RX must be cleaned
            {
                bitclr(PORT_BT,Tx_CSN);
                SendBTbyte(0xe2); // 1110  0010 command flash RX FIFO
                bitset(PORT_BT,Tx_CSN);
            }
   
            for (i = 0; i <BTqueueOutLen;i++)
            {
               bBy = BTqueueOut[i];
               BTqueueOutCopy[i] = bBy;
            }
            BTqueueOutCopyLen = BTqueueOutLen;
            BTqueueOutLen = 0;	

            BTpktCopy = BTpkt;
            CRC = 0xffff;
        }
    }
    if (BTqueueOutCopyLen)
    {
        CRCM8TX = 0xFF;
        BTFlags.CRCM8F = 0;
        
        BTCE_low();	// Chip Enable Activates RX or TX mode (now disable)
        BTType = 2; // type TX
        bitclr(PORT_BT,Tx_CSN);
        BTStatus= SendBTcmd(0xa0); // 1010  0000 command W_TX_PAYLOAD to register 00101 == RF_CH
        
        BTbyteCRC(0xaa);// preambul offset 0
        BTbyteCRC(0xaa);// preambul offset 1
        BTbyteCRC(0xaa);// preambul offset 2
        BTbyteCRC(BTpktCopy);  // sequence/packet offset 3
        SendBTbyte(FqTXCount);//BTbyteCRC(FqTXCount);  // current frequency offset 4

// RF ctrl pkt AA AA AA F0 CH LN CTRL CRC16 CRCM8
// RF data pkt AA AA AA F1 CH LN DATA CRC16 CRCM8

// SYNC_DEBUG 9 - set errors in 3 send packets
//#define ERROR_IN_PK_1 1
  
/////////////////////////////////////////////////////////////////////
        if (FqTXCount ==0)
        {
            BTbyteCRCM1(BTqueueOutCopyLen);  // length   offset 5
            for (i = 0; i <BTqueueOutCopyLen;i++)
            {
#ifndef ERROR_IN_PK_1
                BTbyteCRCM1(BTqueueOutCopy[i]);
#else
                if ((BTqueueOutCopy[0] != 'P') && (BTqueueOutCopy[0] != 'p'))
                    goto SEND_GOOD1;
                if ((i&0x03) == 0)
                {
                    SendBTbyte(0);
                    wCRCupdt(BTqueueOutCopy[i]);
                    if (!BTFlags.CRCM8F)
                        CRCM8TX *= 251;    
                    else
                        CRCM8TX *= 223;
                    #asm
                    BTG BTFlags,3,1
                    #endasm
                    CRCM8TX ^= BTqueueOutCopy[i];
                    CRCM8TX|=1;
                }
                else
                {
SEND_GOOD1:          BTbyteCRCM1(BTqueueOutCopy[i]);
                }
#endif
            }
            CRC16TX = CRC;
            WREG=(CRC16TX>>8);
            BTbyteCRCm1(WREG);
            WREG = (CRC16TX&0x00ff);
            BTbyteCRCm1(WREG);  
            //SendBTbyteM1(CRCM8TX^0xAA);
        }
        else if (FqTXCount ==1)
        {
            BTbyteCRCM2(BTqueueOutCopyLen);  // length   offset 5
            for (i = 0; i <BTqueueOutCopyLen;i++)
            {
#ifndef ERROR_IN_PK_1
                BTbyteCRCM2(BTqueueOutCopy[i]);
#else
                if ((BTqueueOutCopy[0] != 'P') && (BTqueueOutCopy[0] != 'p'))
                    goto SEND_GOOD2;
                if ((i&0x03) == 1)
                {
                    SendBTbyte(0xff);
                    if (!BTFlags.CRCM8F)
                        CRCM8TX *= 239;    
                    else
                        CRCM8TX *= 139;
                    #asm
                    BTG BTFlags,3,1
                    #endasm
                    CRCM8TX ^= BTqueueOutCopy[i];
                    CRCM8TX|=1;
                }
                else
                {
SEND_GOOD2:          BTbyteCRCM2(BTqueueOutCopy[i]);
                }
#endif
            } 
            // was already calculated on TX FQ1
            WREG=(CRC16TX>>8);
            BTbyteCRCM2(WREG);
            WREG = (CRC16TX&0x00ff);
            BTbyteCRCM2(WREG);
            //SendBTbyteM2(CRCM8TX^0xAA);
        }
        else // if (FqTXCount ==2)
        {
            BTbyteCRCM3(BTqueueOutCopyLen);  // length   offset 5
            for (i = 0; i <BTqueueOutCopyLen;i++)
            {
#ifndef ERROR_IN_PK_1
                BTbyteCRCM3(BTqueueOutCopy[i]);
#else
                if ((BTqueueOutCopy[0] != 'P') && (BTqueueOutCopy[0] != 'p'))
                    goto SEND_GOOD3;
                if ((i&0x03) == 2)
                {
                    SendBTbyte(0xf0);
                    if (!BTFlags.CRCM8F)
                        CRCM8TX *= 227;    
                    else
                        CRCM8TX *= 151;
                    #asm
                    BTG BTFlags,3,1
                    #endasm
                    CRCM8TX ^= BTqueueOutCopy[i];
                    CRCM8TX|=1;
                }
                else
                {
SEND_GOOD3:          BTbyteCRCM3(BTqueueOutCopy[i]);
                }
#endif
            }
            WREG=(CRC16TX>>8);
            BTbyteCRCM3(WREG);
            WREG = (CRC16TX&0x00ff);
            BTbyteCRCM3(WREG);  
            //SendBTbyteM3(CRCM8TX^0xAA);
        }
        // NO CRC - from now CRC is exect for all 3 packages
        /////////////////////////////////////////////////////////////////////

        //wCRCupdt(FqTXCount);
          
        //   max len  header              CRC   Len
        i =    32    -6 -2     -BTqueueOutCopyLen;
        do 
        { 
            SendBTbyte(0);  // padding zero till the end
        } while(--i);

        bitset(PORT_BT,Tx_CSN);

        SwitchFQ(FqTX); // set current FQX to be transmitted (advance for FqTXCount will be in TMR1)

      //if (BTStatus & 0x40)
      //{
      //    bitclr(PORT_BT,Tx_CSN);
      //    SendBTbyte(0xe2); // 1110  0010 command flash RX FIFO
      //    bitset(PORT_BT,Tx_CSN);
      //}

        if (DataB0.Timer1Count)
        {
            DataB0.Timer1DoTX = 1; // TMR1 will initiate transmit
            goto TRANSMIT_ON_TMR1_INT;
        }
        else
        {

   // that code for measurements only - it suppose to give a time btw FQ1 & FQ2
            if (FqTXCount == 0) // send over FQ1
            {
                DataB0.Timer1Count = 0;
                DataB0.Timer1Meausre = 1;
                Tmr1High = 0;
                SetTimer1(0);  // timer 1 measure time btw fq1-fq2 transmit 
                BTCE_high(); // Chip Enable Activates RX or TX mode (now TX mode) 

            }
            else // send over FQ2
            {
                if (DataB0.Timer1Meausre)
                {
                    Tmr1LoadHigh = 0xffff - Tmr1High; // this will
                    Tmr1TOHigh = Tmr1LoadHigh;
                    DataB0.Timer1Meausre = 0;
                    DataB0.Timer1Count = 1;

                    //TMR1ON = 0;              // stop timer measure it time btw send Fq1 -> Fq2
                    Tmr1LoadLow = 0xffff - TIMER1;      // timer1 interupt reload values 
                    // offset timer value to be exect tick
                    // 3 commands for Tmr1LoadLow = 0xffff - TIMER1
                    // 9 commands for Tmr1LoadLow+LEN_OFFSET
                    // -0x26 ticks after ISR entry
                    // total 0x1a
#define LEN_OFFSET  0x1a
                    SetTimer1(Tmr1LoadLow+LEN_OFFSET);
                    BTCE_high(); // Chip Enable Activates RX or TX mode (now TX mode) 
                    Tmr1LoadLowCopy = Tmr1LoadLow - 12;
                    Tmr1LoadLowCopy = Tmr1LoadLowCopy | 1;
                    // SYNC_DEBUG 1 set the same value and in TransmitBTdata to have perfect sync
                    //Tmr1LoadLowCopy = 0x977b;
                    //TMR1ON = 1; // start temporary stoped timer (if it was stopped!)
                    DistMeasure.TXPeriod = Tmr1LoadLowCopy; 
                }
            }
#ifdef DEBUG_SIM
            BTCE_highTX();
#else
//            BTCE_high(); // Chip Enable Activates RX or TX mode (now TX mode) 
#endif

            //SwitchFQ(FqTX);  // set FQX to TX -  
            DoFqTXSwitch(); // now FqTXCount is advanced to next TX frequency 

        }
TRANSMIT_ON_TMR1_INT:
        INT0_ENBL = 1;
    }
}

void SwitchToRXdata(void)
{
    BTCE_low(); // Chip Enable Activates RX or TX mode (now disable)
    
    PORT_AMPL.BT_TX = 0;              // off TX amplifier
    nop();
    PORT_AMPL.BT_RX = 1;              // on RX amplifier

    BTStatus = SwitchFQ(FqRX); // switch back FQX (whatever it is); BTStatus updated
    //if (BTStatus & 0x60)  // ?? interrupt TX on last FQ3
    //{
    //    bitclr(PORT_BT,Tx_CSN);
    //    SendBTbyte(0xe2); // 1110  0010 command flash RX FIFO
    //    bitset(PORT_BT,Tx_CSN);

    //    bitclr(PORT_BT,Tx_CSN); // SPI Chip Select // pipe 0
    //    SendBTbyte(0x27); // 0010  0111 command W_REGISTER to register 00111 == STATUS
    //    SendBTbyte(0x60); // clean TX interrupt
    //    bitset(PORT_BT,Tx_CSN);
    //}
    
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
                 //       0  CRCO CRC encoding scheme '0' - 1 byte '1' ϋ 2 bytes
                 //        1 PWR_UP 1: POWER UP, 0:POWER DOWN
                 //         1PRIM_RX	RX/TX control 1: PRX, 0: PTX
	

    bitclr(PORT_BT,Tx_CSN);
    BTStatus= SendBTcmd(0x20); // 001 0  0000 W_REGISTER to register 00000 == Configuration Register
    SendBTbyte(0x13);
    bitset(PORT_BT,Tx_CSN); // SPI Chip Select

    INT0_FLG = 0;
    INT0_ENBL = 1;
    BTType = 1; // type RX
    BTCE_high(); // Chip Enable Activates RX or TX mode (now RX mode) 
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
	    data = 0x71;//0x3d; why not???
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
                 //       0  CRCO    CRC encoding scheme '0' - 1 byte '1' ϋ 2 bytes
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
        //if (BTStatus & 0x10) // Maximum number of TX retransmits interrupt Write 1 to clear bit.
       {

           //bitclr(PORT_BT,Tx_CSN); // SPI Chip Select // pipe 0
           //SendBTbyte(0x27); // 0010  0111 command W_REGISTER to register 00111 == STATUS
           //SendBTbyte(0x70);        // clear all interrupts
           //bitset(PORT_BT,Tx_CSN);
           // reads it again
           //bitclr(PORT_BT,Tx_CSN); // SPI Chip Select
           //BTStatus= SendBTcmd(0xff); //cmd = 0xff; = 1111 1111 command NOP to read STATUS register
           //bitset(PORT_BT,Tx_CSN); // set high
       }
       //if (BTStatus & 0x20) // TX_DS - Data Sent TX FIFO interrupt. Asserted when packet transmitted on TX. If AUTO_ACK is activated,
       {                    //         this bit is set high only when ACK is received. Write 1 to clear bit.
                            // at thet moment no transmit shuld be == clean interrupt
           bitclr(PORT_BT,Tx_CSN); // SPI Chip Select
           BTStatus= SendBTcmd(0xa0); // 1010 0000 flash TX FIFO
           bitset(PORT_BT,Tx_CSN); // set high

           //bitclr(PORT_BT,Tx_CSN); // SPI Chip Select // pipe 0
           //SendBTbyte(0x27); // 0010  0111 command W_REGISTER to register 00111 == STATUS
           //SendBTbyte(0x20);
           //bitset(PORT_BT,Tx_CSN);
           // reads it again
       }
       //if (BTStatus & 0x40) // RX_DS - Data Sent TX FIFO interrupt. Asserted when packet transmitted on TX. If AUTO_ACK is activated,
       {                    //         this bit is set high only when ACK is received. Write 1 to clear bit.
                            // at thet moment no transmit shuld be == clean interrupt
           bitclr(PORT_BT,Tx_CSN); // SPI Chip Select
           BTStatus=SendBTcmd(0xe2); // 1110 0001 command FLUSH_RX
           bitset(PORT_BT,Tx_CSN); // set high
 
           bitclr(PORT_BT,Tx_CSN); // SPI Chip Select // pipe 0
           SendBTbyte(0x27); // 0010  0111 command W_REGISTER to register 00111 == STATUS
           SendBTbyte(0x70);  // flash all interrupts
           bitset(PORT_BT,Tx_CSN);
           // reads it again
       }

       bitclr(PORT_BT,Tx_CSN); // SPI Chip Select // pipe 0
       SendBTbyte(0x21); // 0010  0001 command W_REGISTER to register 00001 == EN_AA
       SendBTbyte(00);  // 00 0 0 0 0 0 0 all ack on all pipes are disabled
       bitset(PORT_BT,Tx_CSN);

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
       SendBTbyte(FqRX);//BTFQcurr);
       bitset(PORT_BT,Tx_CSN);
       //BTFQcurr = Freq1;

       bitclr(PORT_BT,Tx_CSN);
       SendBTbyte(0x26); // 0010  0110 command W_REGISTER to register 00110 == RF_SETUP
                         // CONT_WAVE 0          R/W Enables continuous carrier transmit when high.
                         // Reserved   0         R/W Only '0' allowed
                         // RF_DR_LOW   0        R/W Set RF Data Rate to 250kbps. See RF_DR_HIGH for encoding.
                         // PLL_LOCK     0       R/W Force PLL lock signal. Only used in test
                         // RF_DR_HIGH     0     R/W Select between the high speed data rates. This bit 
                         //                          is dont care if RF_DR_LOW is set.
                         //                          Encoding:00  1Mbps 01  2Mbps 10  250kbps 11  Reserved
                         // RF_PWR          11   R/W Set RF output power in TX mode '00'  -18dBm '01'  -12dBm '10'  -6dBm '11'  0dBm
                         // Obsolete          0  Dont care
                                //
       SendBTbyte(0b00000110);
       bitset(PORT_BT,Tx_CSN);
 
       bitclr(PORT_BT,Tx_CSN); // SPI Chip Select // address 
       SendBTbyte(0x30); // 0011  0000 command W_REGISTER to register 10000 == TX_ADDR_P0
       SendBTbyte(Addr1);   
       SendBTbyte(Addr2);   
       SendBTbyte(Addr3);
       SendBTbyte(0xaa);   
       SendBTbyte(0xaa);   
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
                       //           0 Pipe not used 1 = 1 byte ΰ32 = 32 bytes
                       //           for TX length defined by amount bytes clocked to RF24l01+
       SendBTbyte(0x31); // 0011  0001 command W_REGISTER to register 10001 == RX_PW_P0
       //SendBTbyte(32-4);  // data = 32 byte payload will be preambul+addr1+addr2+addr3
       SendBTbyte(32);  // data = 32 byte payload will be preambul+addr1+addr2+addr3 
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
        data = 0x13;//00110011
    else
        data = 0x13;// = 0x3F; //PWR_UP = 1 ==============================================================================================
                 // data in Configuration Register
                 // 0001 0011  
                 //  0       MASK_RX_DR Mask interrupt caused by RX_DR
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
                 //       0  CRCO CRC encoding scheme '0' - 1 byte '1' ϋ 2 bytes
                 //        1 PWR_UP 1: POWER UP, 0:POWER DOWN
                 //         1PRIM_RX	RX/TX control 1: PRX, 0: PTX
    bitclr(PORT_BT,Tx_CSN);
    SendBTbyte(0x20); // 001 0  0000 W_REGISTER to register 00000 == Configuration Register
    SendBTbyte(data);
    bitset(PORT_BT,Tx_CSN); // SPI Chip Select

    PORT_AMPL.BT_TX = 0;              // on RX amplifier
    nop();
    PORT_AMPL.BT_RX = 1;              // on RX amplifier
    
    BTType = 1; // type RX
    BTokMsg = 0xff;
    BTCE_high(); // Chip Enable Activates RX or TX mode (now RX mode)
    //INT0_FLG = 0; // may be it will be some data allready in fifo buffers
    Main.ExtInterrupt = 0;
    INT0_ENBL = 1;
}
unsigned char SendCMD;
unsigned char SendBTcmd(unsigned char cmd)
{
#ifdef DEBUG_SIM
    unsigned char Data=1;
    putch0(cmd);
    switch(Sendbtcmd)
    {
    case 0:Data = 0x7e;break;
    case 1:Data = 0x6e;break;
    case 2:Data = 0x4e;break;
    case 3:Data = 0x0e;break;
    case 9:Data = 0x20;break; // FQ1 TX
    case 12:Data = 0x20;break; // FQ2 TX
    case 15:Data = 0x20;break; // FQ3 TX done
    case 0x16:Data = 0x20;break; // FQ1 TX done
    case 0x19:Data = 0x20;break; // FQ1 TX done
    case 0x1C:Data = 0x20;break; // FQ1 TX done
    case 0x1F:Data = 0x40;break; // FQ1 RX from sudent RX
    case 0x3F:Data = 0x40;break; // FQ1 RX from sudent RX
    }
    Sendbtcmd++;
    return (Data);
#else
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
#endif
}
void ClockOutByte(void)
{
    int i = 8;
    do 
    {
        PORT_BT.Tx_SCK = 1;
        nop();
		PORT_BT.Tx_SCK = 0;
    } while(--i);
}
void SendBTbyte(unsigned char cmd)
{
#ifdef DEBUG_SIM
    putch0(cmd);
    SendCMD = cmd; 
#else
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
#endif
}

unsigned char GetBTbyte(void)
{
#ifdef DEBUG_SIM
    unsigned int Data = 0;
    putch0(Data);
    switch(Sendbtcmd)
    {
    case 0x20:Data = 28;break;  // RX FQ1 Length
    case 0x22:Data = 0xaa;break; // continue preambule
    case 0x23:Data = 0xaa;break;
    case 0x24:Data = 0xaa;break;
    case 0x25:Data = 0xf0;break; // dial pkt
    case 0x26:Data = 0x00;break; // RX FQ count
    case 0x27:Data = 0x0a;break; // len of the data
    case 0x28:Data = 'e';break; 
    case 0x29:Data = 'a';break; 
    case 0x2a:Data = 'r';break; 
    case 0x2b:Data = 'z';break; 
    case 0x2c:Data = 0xaa;break;
    case 0x2d:Data = 0xaa;break;
    case 0x2e:Data = 0xaa;break;
    case 0x2f:Data = 0x0a;break; // FQ1
    case 0x30:Data = 0x1b;break; // FQ1
    case 0x31:Data = 0x46;break; // FQ1
    case 0x32:Data = 0xbc;break; // FQ1
    case 0x33:Data = 0x03;break; // FQ1
    case 0x34:Data = 0xff;break; // FQ1

    case 0x40:Data = 28;break;  // RX FQ1 Length
    case 0x42:Data = 0xaa;break; // continue preambule
    case 0x43:Data = 0xaa;break;
    case 0x44:Data = 0xaa;break;
    case 0x45:Data = 0xf0;break; // dial pkt
    case 0x46:Data = 0x01;break; // RX FQ count
    case 0x47:Data = 0x0a;break; // len of the data
    case 0x48:Data = 'e';break; 
    case 0x49:Data = 'a';break; 
    case 0x4a:Data = 'r';break; 
    case 0x4b:Data = 'z';break; 
    case 0x4c:Data = 0xaa;break;
    case 0x4d:Data = 0xaa;break;
    case 0x4e:Data = 0xaa;break;
    case 0x4f:Data = 0x0a;break; // FQ1
    case 0x50:Data = 0x1b;break; // FQ1
    case 0x51:Data = 0x46;break; // FQ1
    case 0x52:Data = 0xbf;break; // FQ1
    case 0x53:Data = 0x76;break; // FQ1
    case 0x54:Data = 0xff;break; // FQ1
    }
    Sendbtcmd++;

#else
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
#endif
    return Data;
}

#ifdef _18F2321_18F25K20  // comaptability mode == branch on 0ffset 8 and no priority
#pragma insertConst
#if 0
#pragma origin 0x4000
UWORD dummy(UWORD Adress)
{
     UWORD RdFlash;
    TBLPTRU = 0;
    TBLPTRH = Adress>>8;
    TBLPTRL = Adress&0xff;
READ_WORD:
     #asm
     TBLRD*+
     #endasm// ; read into TABLAT and increment
     RdFlash = ((UWORD)(TABLAT))<<8;
     #asm
     TBLRD*+ 
     #endasm//; read into TABLAT and increment
     RdFlash += TABLAT;
     return RdFlash;
}
UWORD dummy2(UWORD Adress)
{
     UWORD RdFlash;
    TBLPTRU = 0;
    TBLPTRH = Adress>>8;
    TBLPTRL = Adress&0xff;
READ_WORD:
     #asm
     TBLRD*+
     #endasm// ; read into TABLAT and increment
     RdFlash = ((UWORD)(TABLAT))<<8;
     #asm
     TBLRD*+ 
     #endasm//; read into TABLAT and increment
     RdFlash += TABLAT;
     return RdFlash;
}
#endif
#pragma origin 0x7b00
#endif

#define PROC_WRITE_LEN 32
#define PROC_ERACE_LEN 64
#define PROC_ERASE_MASK 0x20
void setAddr(unsigned char bTop, UWORD Adress)
{
    TBLPTRU = bTop;
    TBLPTRH = Adress>>8;
    TBLPTRL = Adress&0xff;
}
unsigned char ReadFlash(void)
{
    //UWORD RdFlash;
    //if (DataB0.EnableFlashWrite)
    {
READ_WORD:
        #asm
        TBLRD*+
        #endasm// ; read into TABLAT and increment
        return (TABLAT);
        //#asm
        //TBLRD*+ 
        //#endasm//; read into TABLAT and increment
        //RdFlash += TABLAT;
     //return RdFlash;
    }
    //return 0;
}
void EraceFlash(unsigned char bTop, UWORD Adress)
{
     if (DataB0.EnableFlashWrite)
     {
         setAddr(bTop,Adress);
ERASE_BLOCK:
         //BSF EECON1, EEPGD ; point to Flash program memory
         EEPGD = 1;
         //BCF EECON1, CFGS ; access Flash program memory
         CFGS = 0;
         //BSF EECON1, WREN ; enable write to memory
         WREN = 1;
         //BSF EECON1, FREE ; enable block Erase operation
         FREE = 1;
         //BCF INTCON, GIE ; disable interrupts
         if (DataB0.GIE)
             GIE = 0;
         //Required MOVLW 55h
         EECON2 = 0x55;
         //Sequence MOVWF EECON2 ; write 55h
         //MOVLW 0AAh
         //MOVWF EECON2 ; write 0AAh
         EECON2 = 0xaa;
         //BSF EECON1, WR ; start erase (CPU stall)
         WR = 1;
         //BSF INTCON, GIE ; re-enable interrupts
         if (DataB0.GIE)
             GIE = 1;
     }
}

void WriteFlash(unsigned char bTop, UWORD Adress, unsigned char *MemPtr)
{
     if (DataB0.EnableFlashWrite)
     {
         setAddr(bTop,Adress);
         for (i=0; i<PROC_WRITE_LEN;i++)
         {
             WREG = *MemPtr;
             TABLAT = WREG; 
             #asm
             TBLWT*+
             #endasm
             MemPtr++;
         }
         #asm
         TBLRD*-
         #endasm

         //setAddr(Adress);
         EEPGD = 1;
         CFGS = 0;
         WREN = 1;
         if (DataB0.GIE)
             GIE = 0;
         EECON2 = 0x55;
         EECON2 = 0xaa;
         WR = 1;
         if (DataB0.GIE)
             GIE = 1;
         WREN = 0;
    }
}
void DSendSSByte(unsigned char bByte)
{
    if (DataB0.EnableFlashWrite)
    {
        i = 8;
        do
        {
            bclr(SSPORT,SSCLOCK);
            bclr(SSPORT,SSDATA_IN);
		    if (bittest(bByte,7))
                bset(SSPORT,SSDATA_IN);
            bByte<<=1;
            bset(SSPORT,SSCLOCK);
        }
        while (--i); // 7*8 = 56 or 8*8 = 64 commands
        bclr(SSPORT,SSCLOCK);
    }
}

unsigned char DGetSSByte(void)
{
    unsigned int j;
    //bitclr(SSPORT,SSCS); // set low Chip Select
    i = 8;
    j = 0;
    if (DataB0.EnableFlashWrite)
    {
        do
        {
            j <<=1;
            bset(SSPORT,SSCLOCK);
        
            //bitclr(bWork2,0); // bWork2 is unsigned == zero in low bit garanteed check assembler code to confirm
#ifdef SSDATA_OUT2

            if (btest(SSPORT_READ,SSDATA_OUT))
            {
                if (btest(SSPORT2_READ,SSDATA_OUT2))
                    goto FLASH_MAJORITY;
                else if (btest(SSPORT2_READ,SSDATA_OUT3))
                    goto FLASH_MAJORITY;
            }
            else if (btest(SSPORT2_READ,SSDATA_OUT2))
                     if (btest(SSPORT2_READ,SSDATA_OUT3))
                     {
FLASH_MAJORITY:
                         bitset(j,0);
                     }
#else
            if (btest(SSPORT_READ,SSDATA_OUT_READ))
                bitset(j,0);
#endif
            bclr(SSPORT,SSCLOCK);
        }
        while (--i);
    }
    return j;
}

void DCheckStatus(void)
{
    unsigned char StatusByte;
    if (DataB0.EnableFlashWrite)
    {
        StatusByte = 0x80;
        CS_LOW;
        DSendSSByte(0x05);
        while( StatusByte & 0x80)
        {
            StatusByte = DGetSSByte();
        }
        CS_HIGH;
    }
}

unsigned char DGetFromFlash( void)
{
    if (DataB0.EnableFlashWrite)
    {
        if (btest(SSPORT,SSCS)) // is it HIGH ???
        {
            DCheckStatus(); 
            CS_LOW;
            DSendSSByte(0x03);
            DSendSSByte(AdrBH);
            DSendSSByte(wAddr>>8);
            DSendSSByte(wAddr&0xff);
        }
        i = DGetSSByte();
        wAddr++;
        if ((wAddr &0xff) == 0) // page read done set next page
        {
            CS_HIGH;
            if (wAddr  == 0) // over 64K
                AdrBH++;
        }
    }
    return i;    
}

void PrgUnit(unsigned char bTop, UWORD Addr, UWORD uwTillAddr)
{
    unsigned char bFlash;
    UWORD iCount;
    unsigned char iFlashRead;
    if (DataB0.EnableFlashWrite)
    {
        GIE = 0;
        DataB0.GIE = GIE;
        CS_HIGH; // set high == next DGetFromFlash will set read address
        for (iCount = Addr; iCount < uwTillAddr; iCount+=PROC_WRITE_LEN)
        {
            if ((iCount&PROC_ERASE_MASK) == 0)
                EraceFlash(bTop,iCount);
            for (iFlashRead = 0; iFlashRead < PROC_WRITE_LEN; iFlashRead++)
            {
                DGetFromFlash(); // each read advance pointer in flash memory
                BTqueueIn[iFlashRead] = i; // it is in i var!!!
            } 
            WriteFlash(bTop,iCount, BTqueueIn);
        }
        #asm
        RESET
        #endasm
    }
}
