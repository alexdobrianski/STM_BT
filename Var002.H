//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////end of var002.h//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
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
unsigned CRCCalcFals:1; 
} DataB0;

#ifdef DEBUG_LED
int PingDelay;
unsigned char DebugLedCount;
#endif

unsigned char FqTXCount;
unsigned char FqTX;
//unsigned char TXSendOverFQ;
#ifdef _18F2321_18F25K20
#ifndef _WIN32
UWORD TIMER0 @ 0xFD6;
UWORD TIMER1 @ 0xFCE;
UWORD TIMER3 @ 0xFB2;
#endif
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

///////////////////////////////////////BANK 4//////////////////////////
#pragma rambank RAM_BANK_4

unsigned char AdrBH;
UWORD wAddr;
unsigned char AdrBH2;
UWORD wAddr2;
unsigned char DiffBH;
UWORD wDiff;
UWORD wtemp;
unsigned char btempres;

UWORD CRC1;
UWORD CRC2;
UWORD CRC3;
UWORD CRC4;

UWORD CRC1Cmp;
UWORD CRC2Cmp;
UWORD CRC3Cmp;
UWORD CRC4Cmp;

#pragma rambank RAM_BANK_3
/////////////////////////////////////////////BANK 3//////////////////////
struct _DataB3{
unsigned FlashCmd:1;
unsigned FlashCmdLen:1;
unsigned FlashRead:1;
unsigned FlashWas1byteWrite:1;
unsigned FlashCmdShort:1;
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

unsigned char ProgAdrBH;
UWORD wProgAddr;
UWORD wProgMaxAddr;

// structure of a packet acsepted from BT
unsigned char NextLen;
unsigned char TypePkt;

#pragma rambank RAM_BANK_3
///////////////////////////////////////BANK 5//////////////////////////
#define BT_BUF 120
//unsigned char ByteInbank5;
struct _BtComIn
{
    WORD iEntry;
    WORD iQueueSize;
    WORD iExit;
    unsigned char Queue[BT_BUF];
} BTComIn;

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
unsigned char CRCFlashCalcStatus;

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

//void wCRCupdt(int bByte);
unsigned char SendBTcmd(unsigned char cmd);
void SendBTbyte(unsigned char cmd);
unsigned char GetBTbyte(void);

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////end of var002.h//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
