// DebugProtocol.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h> 
#define NON_STANDART_MODEM 1

#define _18F25K20 1

//typedef unsigned short int UWORD; 

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
    int LostPkt[4096];
    void SetLostPkt(int iArrayLen, int * iArray)
    {
        int i;
        for (i = 0; i <sizeof(LostPkt)/sizeof(int); i++)
        {
            LostPkt[i] = 1;
        }
        for (i = 0; i <iArrayLen; i++)
        {
            LostPkt[i] = iArray[i];
        }
    }
    void SetOneLostPkt(int iPkt)
    {
        LostPkt[iPkt] = 0;
    }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "..\..\Var001.H"





#include "..\..\commc0.h"



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "..\..\Var002.H"


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
    case 3:FlashAdr3 = bByte;FlashPointerByteN = 0;FlashAdr = FlashAdr1 * 256*256 + FlashAdr2*256+FlashAdr3;
        if (FlashCMD == 0x20)
        {
            for (int ip=0; ip < 4096; ip++)
            {
                FlashMemory[FlashAdr+ip] = 0xff;
            }
        }
        else if (FlashCMD == 0xd8)
        {
            for (int ip=0; ip < 1024*64; ip++)
            {
                FlashMemory[FlashAdr+ip] = 0xff;
            }
        }
        break;
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
int CheckEndOfPacketAndSend(unsigned char bByte)
{
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
    {
        ATCMD |= SOME_DATA_OUT_BUFFER; // that will force transmit on next FQ1
        return 0;             // skip to process any bytes from COM
    }        
}
#include "..\..\addon.h"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "..\..\ProcCMD.H"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
};

int iStat;
unsigned char ConstByte;
unsigned char ReadComInHex(unsigned char bInByte, BOOL &CharReady)
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
    unsigned char BitSave;
    if (BTEarth->AOutQu.iQueueSize)
    {
        bWorkByte = BTEarth->AOutQu.Queue[BTEarth->AOutQu.iExit];
        if (++BTEarth->AOutQu.iExit >= OUT_BUFFER_LEN)
            BTEarth->AOutQu.iExit = 0;
        BTEarth->AOutQu.iQueueSize--;
        fwrite(&bWorkByte,1,1,FileComOutEarth);
    }
    BTEarth->Timer1HCount+=3;
    if (BTEarth->ATCMD & SOME_DATA_OUT_BUFFER) // BTEarth send something to the BTLuna
    {
        bRet = TRUE;
        // earth to luna TX
        BOOL LostPacket = FALSE;
        if (EarthLunaPktN < (sizeof(BTEarth->LostPkt)/sizeof(int)) )
        {
            LostPacket = (BTEarth->LostPkt[EarthLunaPktN] == 0); // zero == packet lost 1 = packet transmitted
            if (LostPacket)
                bRet = TRUE;
            else
                EarthLunaPktN = EarthLunaPktN;

        }
        EarthLunaPktN++;
        
        if (BTEarth->BTqueueOut[0] == '*') // process by Luna BT unit
        {
            BTLuna->Main.PktAsCMD = 1;
            BTLuna->BTInternal.iQueueSize =1 ;
            for (int ii = 1; ii <BTEarth->BTqueueOutLen;ii++)
            {
                if (!LostPacket)
                {
                    if (BTLuna->Main.SendOverLink) // fillup BT buffer till end of data
                    {
                        BTLuna->CheckEndOfPacketAndSend(BTEarth->BTqueueOut[ii]);
                    }
                    else
                        BTLuna->ProcessCMD(BTEarth->BTqueueOut[ii]);
                }
                else
                {
                }
            }
            BTLuna->Main.PktAsCMD = 0;
            BTLuna->BTInternal.iQueueSize =0 ;
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
    if (BTEarth->Main.DoPing)
    {
        bRet = TRUE;
        if (--BTEarth->PingAttempts == 0)
            BTEarth->Main.DoPing = 0;
    }
    BTLuna->ProcessExch();
    if (BTLuna->AOutQu.iQueueSize)
    {
        bWorkByte = BTLuna->AOutQu.Queue[BTLuna->AOutQu.iExit];
        if (++BTLuna->AOutQu.iExit >= OUT_BUFFER_LEN)
            BTLuna->AOutQu.iExit = 0;
        BTLuna->AOutQu.iQueueSize--;
        fwrite(&bWorkByte,1,1,FileComOutLuna);
    }
    BTLuna->Timer1HCount+=3;
    if (BTLuna->ATCMD & SOME_DATA_OUT_BUFFER) // BTEarth send something to the BTLuna
    {
        bRet = TRUE;
        // luna to earth communication
        // 
        BOOL LostPacket = FALSE;
        if (EarthLunaPktN <(sizeof(BTLuna->LostPkt)/sizeof(int)) )
        {
            LostPacket = (BTLuna->LostPkt[EarthLunaPktN] == 0); // zero == packet lost 1 = packet transmitted
            if (LostPacket)
                bRet = TRUE;
        }
        LunaEarthPktN++;
        if (BTLuna->BTqueueOut[0] == '*') // process by Luna BT unit
        {
            BTEarth-> Main.PktAsCMD = 1;
            BitSave = BTEarth->BTInternal.iQueueSize;
            BTEarth->BTInternal.iQueueSize =1 ;
            for (int ii = 1; ii <BTLuna->BTqueueOutLen;ii++)
            {
                if (!LostPacket)
                {
                    if (BTEarth->Main.SendOverLink) // fillup BT buffer till end of data
                    {
                        BTEarth->CheckEndOfPacketAndSend(BTLuna->BTqueueOut[ii]);
                    }
                    else
                        BTEarth->ProcessCMD(BTLuna->BTqueueOut[ii]);
                }
                else
                {
                }
            }
            BTEarth->Main.PktAsCMD = 0;
            BTEarth->BTInternal.iQueueSize = BitSave ;
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
    if (BTLuna->Main.DoPing)
    {
        bRet = TRUE;
        if (--BTLuna->PingAttempts == 0)
            BTLuna->Main.DoPing = 0;
    }
    BTEarth->ProcessExch();

    // just speed up for simulation -> all packets ready to transfer will be in 
    BTLuna->ProcessExch();
    BTEarth->ProcessExch();
    return (bRet);
}
//#define TEST_ALL_OK
//#define TEST_2PKT_LOST
#define TEST_TO
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
    BTLuna.CS_HIGH;
    BTLuna.ATCMD |= MODE_CONNECT;
    BTLuna.DataB0.ExcgFlashcmd = 0;
    BTLuna.DataB0.ExcgRcvCmd = 0;
    BTLuna.DataB0.ExcgSendinProgress = 0;
    BTLuna.BTInternal.iQueueSize = 0 ;
    
    BTEarth.FlashMemoryLen = 1*1024*1024;
    BTEarth.FlashMemory = (unsigned char*) GlobalAlloc( GMEM_FIXED, BTLuna.FlashMemoryLen);// 1 MB
    memset(BTEarth.FlashMemory,0xff,BTEarth.FlashMemoryLen);
    memset(BTEarth.EEPROM, 0xff, sizeof(BTEarth.EEPROM));
    BTEarth.CS_HIGH;
    BTEarth.ATCMD |= MODE_CONNECT;
    BTEarth.DataB0.ExcgFlashcmd = 0;
    BTEarth.DataB0.ExcgRcvCmd = 0;
    BTEarth.DataB0.ExcgSendinProgress = 0;
    BTEarth.BTInternal.iQueueSize = 0 ;

#ifdef TEST_ALL_OK
    {
        int LostPkt[] = {
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
        };
        BTLuna.SetLostPkt(sizeof(LostPkt)/sizeof(int), &LostPkt[0]);
        BTEarth.SetLostPkt(sizeof(LostPkt)/sizeof(int), &LostPkt[0]);
    }
#endif
#ifdef TEST_2PKT_LOST
    {
        int LostPkt[] = {
            1,1,1,1,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,0
        };
        BTLuna.SetLostPkt(sizeof(LostPkt)/sizeof(int), &LostPkt[0]);
        BTEarth.SetLostPkt(sizeof(LostPkt)/sizeof(int), &LostPkt[0]);
    }
#endif
#ifdef TEST_TO
    {
        int LostPkt[] = {
            1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
        };
        BTLuna.SetLostPkt(sizeof(LostPkt)/sizeof(int), &LostPkt[0]);
        BTEarth.SetLostPkt(sizeof(LostPkt)/sizeof(int), &LostPkt[0]);
        BTLuna.SetOneLostPkt(0x400);
        BTEarth.SetOneLostPkt(0x400);
    }
#endif



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
                    OutPutByte = ReadComInHex(bByff[0], CharReady);
                    if (CharReady)
                    {
                        if (BTEarth.Main.SendOverLink) // fillup BT buffer till end of data
                        {
                            BTEarth.CheckEndOfPacketAndSend(OutPutByte);
                        }
                        else
                            BTEarth.ProcessCMD(OutPutByte);
                        if (BTEarth.Main.SendOverLink) // fillup BT buffer till end of data
                        {
                        }
                        else
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

