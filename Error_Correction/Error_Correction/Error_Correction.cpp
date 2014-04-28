// Error_Correction.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string.h>
#include <stdio.h>
#include <Windows.h>
typedef WORD UWORD; 

//////////////////////////////////////////////////////////////
unsigned int i;         // int -> char
unsigned int iTotalLen; // int -> char
unsigned int iCrc;      // int -> char
//unsigned char *ptr1;
//unsigned char *ptr2;
//unsigned char *ptr3;
unsigned char CRCM8TX;
unsigned char CRCM8TX2;
unsigned char CRCM8TX3;
UWORD CRCcmp;
UWORD CRC;
UWORD CRC16TX;
unsigned char BTqueueOutCopy[512];
int BTqueueInLen;
unsigned char BTqueueIn[512];
int BTqueueInLen2;
unsigned char BTqueueIn2[512];
int BTqueueInLen3;
unsigned char BTqueueIn3[512];
unsigned char OutputMsg[512];
unsigned char *ptrOut;
unsigned char *ptrTemp;
unsigned char mask;

struct _BTFLAGS
{
    unsigned BT3fqProcessed:1;
    unsigned BTFirstInit:1;
    unsigned RxInProc:1;
    unsigned CRCM8F:1;
    unsigned Check01:1;
    unsigned Check02:1;
} BTFlags;

unsigned char WREG;

void SendBTbyteM1(unsigned char bByte)
{
    BTqueueIn[BTqueueInLen++]=bByte;
}
void SendBTbyteM2(unsigned char bByte)
{
    BTqueueIn2[BTqueueInLen2++]=bByte;
}
void SendBTbyteM3(unsigned char bByte)
{
    BTqueueIn3[BTqueueInLen3++]=bByte;
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
    SendBTbyteM1(CRCM8TX);
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
    SendBTbyteM2(CRCM8TX);
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
    SendBTbyteM3(CRCM8TX);
    CRCM8TX|=1;
}

unsigned char *FSR_REGISTER;
#define PTR_FSR (*FSR_REGISTER)
int IntRXCount;
unsigned char *FSR1;
#define INDF1 (*FSR1)
unsigned char *FSR2;
#define INDF2 (*FSR2)

unsigned char CheckPacket(unsigned char*MyData, unsigned int iLen)  // used IntRXCount as a number of the RF Ch
{  
    iTotalLen = iCrc = iLen;//26;
#ifdef WIN32
    iCrc = iTotalLen-2; 
#endif
    CRC=0xffff;
    FSR_REGISTER = MyData;
    FSR1 = OutputMsg;
    BTFlags.CRCM8F = 0;
    CRCM8TX = 0xff;
    //                               0  1  2  3  4  5 6--
    // new ctrl packet format is  : AA AA AA F0 CH LN CTRL CRC16 CRCM8
    // new data packet format is  : AA AA AA F1 CH LN DATA CRC16 CRCM8
    //   RF Ch1 from len everything is ^ 0xAA
    //   RF Ch2 from len everything is ^ 0x55
    // restoration of the packet done by 
    for (i = 0; i < iTotalLen; i++)
    {
#ifndef WIN32
        if (i != 4) // packet 0;1;2 added last
        {
            if (i>=5)
#endif
            {
                if (IntRXCount == 0)
                {
#ifdef      _18F2321_18F25K20
                    if (!BTFlags.CRCM8F)
                        CRCM8TX *= 251;    
                    else
                        CRCM8TX *= 223;
#else
                    if (!BTFlags.CRCM8F)
                    {
                        CRCM8TX = (CRCM8TX * 251)%256;
                        BTFlags.CRCM8F = 1;
                    }
                    else
                    {
                        CRCM8TX = (CRCM8TX * 223)%256;
                        BTFlags.CRCM8F = 0;
                    }
#endif
                }
                else if (IntRXCount == 1) // 239 139
                {
 #ifdef      _18F2321_18F25K20
                    if (!BTFlags.CRCM8F)
                        CRCM8TX *= 239;    
                    else
                        CRCM8TX *= 139;
#else
                    if (!BTFlags.CRCM8F)
                    {
                        CRCM8TX = (CRCM8TX * 239)%256;
                        BTFlags.CRCM8F = 1;
                    }
                    else
                    {
                        CRCM8TX = (CRCM8TX * 139)%256;
                        BTFlags.CRCM8F = 0;
                    }
#endif
                }
                else// if (IntRXCount == 2) // 227 151
                {
 #ifdef      _18F2321_18F25K20
                    if (!BTFlags.CRCM8F)
                        CRCM8TX *= 227;    
                    else
                        CRCM8TX *= 151;
#else
                    if (!BTFlags.CRCM8F)
                    {
                        CRCM8TX = (CRCM8TX * 227)%256;
                        BTFlags.CRCM8F = 1;
                    }
                    else
                    {
                        CRCM8TX = (CRCM8TX * 151)%256;
                        BTFlags.CRCM8F = 0;
                    }
#endif
                }
 #ifdef      _18F2321_18F25K20
                #asm
                BTG BTFlags,3,1
                #endasm
#endif
                INDF1 = PTR_FSR ^CRCM8TX;
                CRCM8TX = PTR_FSR|1;
            }   
            if (i<iCrc)
                wCRCupdt(INDF1);
#ifndef WIN32
            if (i == PACKET_LEN_OFFSET)
            {
                if (INDF1<= BT_TX_MAX_LEN)
                {
                    iTotalLen = INDF1 + PACKET_LEN_OFFSET+1+2;// + sizeof(PacketStart);
                    iCrc = INDF1 + PACKET_LEN_OFFSET+1;
                }    
                else
                    goto RETURN_ERROR;
            }
        }
#endif
        FSR_REGISTER++;
        FSR1++;
    }

    FSR1--;
    FSR1--;
    CRCcmp = ((((UWORD)INDF1))<<8); FSR1++;
    CRCcmp += ((UWORD)INDF1);
    
    if (CRC == CRCcmp)
        return 0;
RETURN_ERROR:
    return 0xff;
}

unsigned char BTFix3(void)
{
    
    unsigned char bByte1;
    unsigned char bByte2;
    unsigned char bByte3;
    unsigned char bByteOut;
    iCrc = BTqueueInLen;
    iTotalLen = BTqueueInLen;
#ifdef WIN32
    iCrc = iTotalLen-2; 
#endif
    FSR_REGISTER = BTqueueIn;
    FSR1 = BTqueueIn2;
    FSR2 = BTqueueIn3;
    ptrOut = OutputMsg;
    CRCcmp=0;
    CRC=0xffff; 
    CRCM8TX3 = CRCM8TX2 = CRCM8TX = 0xff;
    BTFlags.CRCM8F = 0;
    if (iTotalLen < BTqueueInLen2)
        iTotalLen = BTqueueInLen2;
    if (iTotalLen < BTqueueInLen3)
        iTotalLen = BTqueueInLen3;

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
    for (i = 0; i < iTotalLen; i++)
    {
#ifdef      _18F2321_18F25K20
        if (!BTFlags.CRCM8F)
        {
            CRCM8TX *= 251;
            CRCM8TX2 *= 239;
            CRCM8TX3 *= 227;
        }
        else
        {
            CRCM8TX *= 223;
            CRCM8TX2 *= 139;
            CRCM8TX3 *= 151;
        }
        #asm
        BTG BTFlags,3,1
        #endasm
#else
        if (!BTFlags.CRCM8F)
        {
            CRCM8TX = (CRCM8TX * 251)%256;
            CRCM8TX2 = (CRCM8TX2 * 239)%256;
            CRCM8TX3 = (CRCM8TX3 * 227)%256;
            BTFlags.CRCM8F = 1;
        }
        else
        {
            CRCM8TX = (CRCM8TX * 223)%256;
            CRCM8TX2 = (CRCM8TX2 * 139)%256;
            CRCM8TX3 = (CRCM8TX3 * 151)%256;
            BTFlags.CRCM8F = 0;
        }
#endif
#ifndef WIN32
        if (i != 4)
        {
#endif
            bByteOut = bByte1 = PTR_FSR ^ CRCM8TX;
            //CRCM8TX = PTR_FSR|1;
            bByte2 = INDF1 ^ CRCM8TX2;
            //CRCM8TX2 = INDF1|1;
            mask = bByte1 ^ bByte2;
            bByteOut &= mask ^ 0xff;
            bByte3 = INDF2 ^ CRCM8TX3;
            mask &= bByte3;
            //CRCM8TX3 = INDF2|1;
            bByteOut |= mask;
            if (i < iCrc) 
                wCRCupdt(bByteOut);
            // crazy - only 3 FSR on the processor
            ptrTemp = FSR_REGISTER;
            FSR_REGISTER = ptrOut;
            PTR_FSR = bByteOut;
            FSR_REGISTER = ptrTemp;
            CRCM8TX =  (CRCM8TX ^ bByteOut)|1;
            CRCM8TX2 =  (CRCM8TX2 ^ bByteOut)|1;
            CRCM8TX3 =  (CRCM8TX3 ^ bByteOut)|1;
#ifndef WIN32
        }
        else
        {
            *ptrOut = 0;
        }
        if (i == PACKET_LEN_OFFSET)
        {
            if (bByteOut<= BT_TX_MAX_LEN)
            {
                iLenTotal = bByteOut + PACKET_LEN_OFFSET+1+3;// + sizeof(PacketStart);
                iCrc = bByteOut + PACKET_LEN_OFFSET+1;
            }     
            else
                goto RETURN_ERROR;
        }
#endif
        FSR_REGISTER++;
        FSR1++;
        FSR2++;
        ptrOut++;
    }
    FSR_REGISTER = ptrOut;
    FSR_REGISTER--;
    FSR_REGISTER--;
   
    //wCRCupdt(0);
    CRCcmp = ((((UWORD)PTR_FSR))<<8); FSR_REGISTER++;
    CRCcmp += ((UWORD)PTR_FSR);
    if (CRC == CRCcmp)
        return 0;
RETURN_ERROR:
    return 0xff;
}

unsigned char BTFix2(void)
{
    int  Vote;
    unsigned char CmpCount;
    unsigned char bByte1;
    unsigned char bByte2;
    unsigned char bByteOut;

    unsigned char FisrtR;
    unsigned char SecondR;
    unsigned char FisrtR2;
    unsigned char SecondR2;

    unsigned char NextByte1;
    unsigned char NextByte2;

    unsigned char CRCM8TXNext;
    unsigned char CRCM8TX2Next;
    Vote = 128;
    iTotalLen = BTqueueInLen;
    BTFlags.Check01 = 0;
    if ((BTqueueInLen > 0) && (BTqueueInLen2 > 0))
    {
#ifdef WIN32
        FSR_REGISTER = &BTqueueIn[0];
        FSR1 = &BTqueueIn2[0];
#else
        FSR_REGISTER = &BTqueueIn[4];
        FSR1 = &BTqueueIn2[5];
#endif
        if (BTqueueInLen > BTqueueInLen2)
            iTotalLen = BTqueueInLen2;
        FisrtR = 251;
        SecondR = 223;
        FisrtR2 = 239;
        SecondR2 = 139;
    }
    else if ((BTqueueInLen2 > 0) && (BTqueueInLen3 > 0))
    {
#ifdef WIN32
        FSR_REGISTER = &BTqueueIn2[0];
        FSR1 = &BTqueueIn3[0];
#else
        FSR_REGISTER = &BTqueueIn2[4];
        FSR1 = &BTqueueIn3[5];
#endif
        if (BTqueueInLen2 > BTqueueInLen3)
            iTotalLen = BTqueueInLen3;
        else
            iTotalLen = BTqueueInLen2;
        FisrtR = 239;
        SecondR = 139;
        FisrtR2 = 227;
        SecondR2 = 151;
    }
    else if ((BTqueueInLen > 0) && (BTqueueInLen3 > 0))
    {
#ifdef WIN32
        FSR_REGISTER = &BTqueueIn[0];
        FSR1 = &BTqueueIn3[0];
#else
        FSR_REGISTER = &BTqueueIn[4];
        FSR1 = &BTqueueIn3[5];
#endif
        if (BTqueueInLen > BTqueueInLen3)
            iTotalLen = BTqueueInLen3;
        else
            iTotalLen = BTqueueInLen;
        FisrtR = 251;
        SecondR = 223;
        FisrtR2 = 227;
        SecondR2 = 151;
    }
    else
        goto RETURN_ERROR;
    iCrc = iTotalLen;
#ifdef WIN32
    iCrc = iTotalLen-2; 
#endif

#ifdef WIN32
    FSR2 = &OutputMsg[0];
#else
    FSR2 = &OutputMsg[5];
#endif
    CRCcmp=0;
#ifdef WIN32
    CRC=0xffff;   
#else
    CRC=0x50d4;   
    //wCRCupdt(0xaa);
    //wCRCupdt(0xaa);
    //wCRCupdt(0xaa);
    wCRCupdt(PTR_FSR);
    FSR_REGISTER++;
#endif

    CRCM8TX2 = CRCM8TX = 0xff;
    BTFlags.CRCM8F = 0;
    
#ifdef WIN32
    for (i = 0; i < iTotalLen; i++)
#else
    for (i = 5; i < iTotalLen; i++)
#endif
    {
        if (!BTFlags.CRCM8F)
        {
            CRCM8TX *= FisrtR;
            CRCM8TX2 *= FisrtR2;
        }
        else
        {
            CRCM8TX *= SecondR;
            CRCM8TX2 *= SecondR2;
        }

#ifdef      _18F2321_18F25K20
        #asm
        BTG BTFlags,3,1
        #endasm
#else
         BTFlags.CRCM8F = !BTFlags.CRCM8F;
#endif    
        bByteOut = bByte1 = PTR_FSR ^ CRCM8TX;
        bByte2 = INDF1 ^ CRCM8TX2;
        CmpCount=0;
        if (bByte1 != bByte2)
        {
            if (i < (iTotalLen-1))
            {
                if (Vote>=128)
                    goto CHECK_SECOND_MSG;
CHECK_SECOND_MSG:
                // check case first is wrong; second may be OK 
                CmpCount++;
                CRCM8TXNext = (CRCM8TX ^ bByte2)|1;
                CRCM8TX2Next = INDF1|1;
                if (!BTFlags.CRCM8F)
                {
                    CRCM8TXNext *= FisrtR;
                    CRCM8TX2Next *= FisrtR2;
                }
                else
                {
                    CRCM8TXNext *= SecondR;
                    CRCM8TX2Next *= SecondR2;
                }
                FSR_REGISTER++;
                FSR1++;
                NextByte1 = PTR_FSR ^ CRCM8TXNext;
                NextByte2 = INDF1 ^ CRCM8TX2Next;
                FSR_REGISTER--;
                FSR1--;
                if (NextByte1 == NextByte2)
                {
                    Vote++;
                    bByteOut = bByte2;
                    CRCM8TX =  (CRCM8TX ^ bByte2)|1;
                    CRCM8TX2 = INDF1|1;
                }
                else
                {
                    if (CmpCount >=2)
                        goto EXIT_WITH_CHECK;
TEST_FIRST_MSG:
                    CmpCount++;
#if 0
                    // covered cases (a) two consequetive error bytes in second message
                    //               (b) first byte correct
                    //               (c) first byte correct and second wrong in second message
                    //               (d) first byte wrong and second wrong in second message
                    //  probability for (d) is p = 1/(28*27)*p(1-byte-error-in paket) = 0.001322*p(error)
                    // for ground station better to store packet and fixed it later from DB 
                    CRCM8TX = PTR_FSR|1;
                    CRCM8TX2 = (CRCM8TX2 ^ bByte1)|1;
#else
                    CRCM8TXNext = PTR_FSR|1;
                    CRCM8TX2Next = (CRCM8TX2 ^ bByte1)|1;
                    if (!BTFlags.CRCM8F)
                    {
                        CRCM8TXNext *= FisrtR;
                        CRCM8TX2Next *= FisrtR2;
                    }
                    else
                    {
                        CRCM8TXNext *= SecondR;
                        CRCM8TX2Next *= SecondR2;
                    }
                    FSR_REGISTER++;
                    FSR1++;
                    NextByte1 = PTR_FSR ^ CRCM8TXNext;
                    NextByte2 = INDF1 ^ CRCM8TX2Next;
                    FSR_REGISTER--;
                    FSR1--;
                    if (NextByte1 == NextByte2)
                    {
                        Vote--;
                        CRCM8TX = PTR_FSR|1;
                        CRCM8TX2 = (CRCM8TX2 ^ bByte1)|1;
                        //bByte2= bByte1;
                    }
                    else // case that two sequensed bytes are broken in two messages
                    {
                        if (CmpCount >=2)
                        {
EXIT_WITH_CHECK:
                            CRCM8TX = PTR_FSR|1;
                            CRCM8TX2 = INDF1|1;
                        }
                        else
                            goto CHECK_SECOND_MSG;
                    }
#endif
                }
            }
            else
            {
                if (Vote>128)
                    bByteOut = bByte2;
            }
        }
        else // if (bByte1 == bByte2)
        {
            CRCM8TX =  (CRCM8TX ^ bByteOut)|1;
            CRCM8TX2 =  (CRCM8TX2 ^ bByteOut)|1;
        }
        INDF2 = bByteOut;
        if (i < iCrc) 
            wCRCupdt(bByteOut);
#ifndef WIN32
        if (i == PACKET_LEN_OFFSET)
        {
            if (bByteOut<= BT_TX_MAX_LEN)
            {
                iTotalLen = bByteOut + PACKET_LEN_OFFSET+1+3;// + sizeof(PacketStart);
                iCrc = bByteOut + PACKET_LEN_OFFSET+1;
            }     
            else
                goto RETURN_ERROR;
        }
#endif
        FSR_REGISTER++;
        FSR1++;
        FSR2++;
    }
    FSR2--;
    FSR2--;
   
    //wCRCupdt(0);
    CRCcmp = ((((UWORD)INDF2))<<8); FSR2++;
    CRCcmp += ((UWORD)INDF2);
    if (CRC == CRCcmp)
        return 0;
RETURN_ERROR:
    return 0xff;
}

int main(int argc,char *argv[])
{
    //HRESULT hres;
    //IWbemLocator *pLoc = 0;
    //IWbemServices *pSvc = 0;
    int Rand256[] = { 2,      3,      5,      7,     11,     13,     17,     19,     23,     29,
     31,     37,     41,     43,     47,     53,     59,     61,     67,     71,
     73,     79,     83,     89,     97,    101,    103,    107,    109,    113,
    127,    131,    137,    139,    149,    151,    157,    163,    167,    173,
    179,    181,    191,    193,    197,    199,    211,    223,    227,    229,
    233,    239,    241,    251,};
    int Rep[256];
    BTqueueInLen=0;
    BTqueueInLen2=0;
    BTqueueInLen3=0;
    /*
    int ii=0;
    int jj=0;
    int len = 0;
    for (int i= 0; i < sizeof(Rand256)/sizeof(int);i++)
    {
        for (int j = 0; j < sizeof(Rand256)/sizeof(int);j++)
        {
           
            int iVal = 3;
            for (int k=1; k< 255;k++)
            {
                if (k&1)
                    iVal = iVal* Rand256[i];
                else
                    iVal = iVal* Rand256[j];
                iVal %=256;
                if (iVal ==1) // loop
                {
                    if (len < k)
                    {
                        ii = i;
                        jj = j;
                        len = k;
                        printf("\n a=%d b=%d max=%d",Rand256[i],Rand256[j],len);
                       
                    }
                    break;
                }
            }
        }
    }
    */
    /*
    int ii=0;
    int jj=0;
    int ii1=0;
    int jj1=0;
    int len = 0;
    for (int i= 0; i < sizeof(Rand256)/sizeof(int);i++)
    {
        for (int j = 0; j < sizeof(Rand256)/sizeof(int);j++)
        {
            for (int i1= 0; i1 < sizeof(Rand256)/sizeof(int);i1++)
            {
                for (int j1 = 0; j1 < sizeof(Rand256)/sizeof(int);j1++)
                {  
                    int iVal = 1;
                    for (int k=1; k< 255;k++)
                    {
                        if (k%5 ==0)
                            iVal = iVal* Rand256[i];
                        else if (k%5 ==1)
                            iVal = iVal* Rand256[j];
                        else if (k%5 ==2)
                            iVal = iVal* Rand256[i1];
                        else
                            iVal = iVal* Rand256[j1];
                        iVal %=256;
                       if (iVal ==1) // loop
                       {
                           if (len < k)
                           {
                               ii = i;
                               jj = j;
                               ii1 = i1;
                               jj1 = j1;
                               len = k;
                               printf("\n a=%d b=%d c=%d d=%d max=%d",Rand256[i],Rand256[j],Rand256[i1],Rand256[j1],len);
                           }
                           break;
                       }
                    }
                }
            }
        }
    }*/
   
    int ii=0;
    int jj=0;
    int ii1=0;
    int len = 0;
    for (int i= 0; i < sizeof(Rand256)/sizeof(int);i++)
    {
        for (int j = 0; j < sizeof(Rand256)/sizeof(int);j++)
        {
            //for (int i1= 0; i1 < sizeof(Rand256)/sizeof(int);i1++)
            {
                {  
                    int iVal = 1;
                    int iMax= 0;
                    Rep[0]=iVal;
                    int iFound = 0;
                    for (int k=1; k< 255;k++)
                    {
                        iFound = 0;
                        if (k&1)
                            iVal = iVal* Rand256[j];
                        else //if (k%4 ==1)
                            iVal = iVal* Rand256[i];
                        //else
                        //    iVal = iVal* Rand256[i1];
                        iVal %=256;
                        Rep[k]=iVal;
                        if (k>0)
                        {
                            if (iVal == 0)
                            {
                                iMax = k;
                                break;
                            }
                            else
                            {
                                for (int l=0; l<k;l++)
                                {
                                    if (Rep[l] == iVal) // loop
                                    {
                                        iFound = 1;
                                        iMax = k;
                                        break;
                                    }
                                }
                                if (iFound ==0)
                                {
                                    if (iMax < k)
                                        iMax = k;
                                }
                                else
                                {
                                    break;
                                }
                            }
                        }
                    }
                   
                    if (len < iMax)
                    {
                        printf("\n Rep[l=%d]=%d iVal=%d",iMax, Rep[iMax], iVal);
                        ii = i;
                        jj = j;
                        //ii1 = i1;
                        len = iMax;
                        printf("\na=%d b=%d max=%d\n",Rand256[i],Rand256[j],len);
                        for (int m=0; m<=len;m++)
                        {
                            printf("%d=%d ",m,Rep[m]);
                        }
                    }
                }
            }
        }
    }

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

 
    int i;
    for (i= 0; i < 256;i++)
    {
        BTqueueOutCopy[i]=i;
        BTqueueOutCopy[i+256] = i/2;
    }
    int BTqueueOutCopyLen = sizeof(BTqueueOutCopy)-3;
    CRC = 0xffff;
    for (int FqTXCount=0; FqTXCount<3;FqTXCount++)
    {
        CRCM8TX = 0xff;
        BTFlags.CRCM8F = 0;

        /////////////////////////////////////////////////////////////////////
        if (FqTXCount ==0)
        {
            //BTbyteCRCM1(BTqueueOutCopyLen);  // length   offset 5
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
            //BTbyteCRCM2(BTqueueOutCopyLen);  // length   offset 5
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
            //BTbyteCRCM3(BTqueueOutCopyLen);  // length   offset 5
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
    }
    IntRXCount = 0;
    if (CheckPacket(BTqueueIn, sizeof(BTqueueIn)-1)<0xff)
        printf("\n BTqueueIn ok");
    IntRXCount =1;
    if (CheckPacket(BTqueueIn2, sizeof(BTqueueIn2)-1)<0xff)
        printf("\n BTqueueIn2 ok");
    IntRXCount=2;
    if (CheckPacket(BTqueueIn3, sizeof(BTqueueIn3)-1)<0xff)
        printf("\n BTqueueIn3 ok");
   
#if 0
    for (i = 0; i <sizeof(BTqueueIn)-1;i+=3)
        BTqueueIn[i]=0xaa;
    for (i = 1; i <sizeof(BTqueueIn)-1;i+=3)
        BTqueueIn2[i]=0xff;
    for (i = 2; i <sizeof(BTqueueIn)-1;i+=3)
        BTqueueIn3[i]=0xff;
#endif

#if 1
    for (i = 0; i <sizeof(BTqueueIn)-1;i+=4)
        BTqueueIn[i]=0xaa;
    for (i = 2; i <sizeof(BTqueueIn)-1;i+=4)
        BTqueueIn2[i]=0xee;
#endif


    IntRXCount = 0;
    if (CheckPacket(BTqueueIn, sizeof(BTqueueIn)-1)<0xff)
        printf("\n BTqueueIn ok");
    else
        printf("\n BTqueueIn bnroken");
    IntRXCount = 1;
    if (CheckPacket(BTqueueIn2, sizeof(BTqueueIn2)-1)<0xff)
        printf("\n BTqueueIn2 ok");
    else
        printf("\n BTqueueIn2 bnroken");

    if (BTFix3() < 3)
        printf("\n BTFix3 fixed");
    else
        printf("\n BTFix3 not fixed");
    
    if(BTFix2() <3)
        printf("\n BTFix2 fixed 0-1");
    else
        printf("\n BTFix2 not fixed 0-1");

    BTqueueInLen = 0;
    if(BTFix2() < 3)
        printf("\n BTFix2 fixed 2-3");
    else
        printf("\n BTFix2 not fixed 2-3");

    BTqueueInLen = BTqueueInLen2;
    BTqueueInLen2 = 0;
    if(BTFix2() <3)
        printf("\n BTFix2 fixed 0-3");
    else
        printf("\n BTFix2 not fixed 0-3");
    //memcpy(BTqueueIn,message,sizeof(message));
    //memcpy(BTqueueIn2,message,sizeof(message));
    //memcpy(BTqueueIn3,message,sizeof(message));
    // encrypt
    /*  
    int FisrtR=251;
    int SecondR=223;
    int iVal=1;
    int iCrc8 = 0;
    
    for (int i = 1;i<sizeof(message);i++)
    {
        if (i&1)
            iVal=(message[i-1]*FisrtR)%256;
        else
            iVal=(message[i-1]*SecondR)%256;
        message[i] = message[i]^iVal;
    }
    iCrc8 = (message[sizeof(message)-1]*FisrtR)%256;
   */
    for (int i = 1; i < sizeof(BTqueueOutCopy);i++)
    {
       // first error restoration
#if 0
        if ((i&1) == 0)
            BTqueueIn[i]=0;
#endif
        // second error restoration
#if 0
            if ((i&1) == 1)
            BTqueueIn2[i]=0;
#endif
            // third error restoration
#if 0
            BTqueueIn[i]=1;
#endif
            // fourth error restoration
#if 0
            BTqueueIn2[i]=0xff;
#endif
 
 
    }
    /*
    int iOld=BTqueueIn[0];
    for (int i = 1; i < sizeof(BTqueueIn);i++)
    {
        if (i&1)
            iVal=(iOld*FisrtR)%256;
        else
            iVal=(iOld*SecondR)%256;
        if (BTqueueIn2[i] != (iVal ^ BTqueueIn[i]))
        {
            // byte not matched - two cases error in (a) message or (b) in BTqueueIn2
            if (i < (sizeof(BTqueueIn)-1))
            {
                // check that error in message
                int iCurrent = iVal ^ BTqueueIn2[i];
                int iValNext;
                if ((i+1)&1)
                   iValNext=(iCurrent*FisrtR)%256;
                else
                   iValNext=(iCurrent*SecondR)%256;
                if (BTqueueIn[i+1] == (BTqueueIn2[i+1]^iValNext))
                {
                    iOld = iCurrent;
                    BTqueueIn[i] = BTqueueIn2[i];
                }
                else // the error in BTqueueIn2
                {
                    iOld = BTqueueIn[i];
                    BTqueueIn[i] = BTqueueIn[i]^iVal;
                }
            }
            else
            {
                iOld = BTqueueIn[i];
                BTqueueIn[i] = BTqueueIn[i]^iVal;
            }
        }
        else
        {
            iOld = BTqueueIn[i];
            BTqueueIn[i] = BTqueueIn[i]^iVal;
        }
    }
    
    if (iCrc8 != (iOld*FisrtR)%256)
    {
        memcpy(BTqueueIn,BTqueueIn2,sizeof(BTqueueIn));
    }*/
 
 
 
    // decrypt
    /*
    iOld=message[0];
    for (int i = 1;i<sizeof(message);i++)
    {
       
        if (i&1)
            iVal=(iOld*FisrtR)%256;
        else
            iVal=(iOld*SecondR)%256;
        iOld = message[i];
        message[i] = message[i]^iVal;
    }*/
    if (memcmp(BTqueueIn,BTqueueOutCopy,sizeof(BTqueueOutCopy)-3)==0)
        printf("message matched");
 
    return(0);
 }