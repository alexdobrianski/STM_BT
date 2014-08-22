UWORD Add3(unsigned char bFirst, UWORD wFirst, unsigned char bSecond, UWORD wSecond)
{ 
    wtemp = wFirst + wSecond;
    if (wtemp >= wFirst)
        btempres = bFirst + bSecond;
    else
        btempres = bFirst + bSecond+1;
    return wtemp;
}

UWORD Sub3(unsigned char bFirst, UWORD wFirst, unsigned char bSecond, UWORD wSecond)
{ 
    if (wFirst >= wSecond)
        btempres = bFirst - bSecond;
    else
        btempres = bFirst - bSecond - 1;
     return (wFirst - wSecond);
}

void wCRCupdt(int bByte)
{
    UWORD Temp ;
    unsigned char iBits;

    Temp = ((UWORD)bByte << 8);
    CRC ^= Temp;
    for (iBits = 8 ; iBits ; --iBits)
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

UWORD CRCCacl(void)
{
    // AdrBH, wAddr start adddress -> AdrBH2, wAddr2-1  end address 
    CRC = 0xFFFF;
    AdrFlash1 = AdrBH;
    AdrFlash2 = wAddr >> 8;
    AdrFlash3 = wAddr & 0xff;
    CS_HIGH;
NEXT_BYTE:    
    if (btest(SSPORT,SSCS)) // another FLASH write can pause FLASH write intiated by serial 
    {
        CS_LOW;
        SendSSByte(0x03); // read
        SendSSByte(AdrFlash1);
        SendSSByte(AdrFlash2);
        SendSSByte(AdrFlash3);
    }
    wCRCupdt(GetSSByte());
    wtemp = ((UWORD)AdrFlash2)<<8 | (UWORD)AdrFlash3; 
    if (++AdrFlash3 ==0)
    {
        if (++AdrFlash2 ==0)
            ++AdrFlash1;
        CS_HIGH;          // that is extention == FLASH can not read/write over same page
    }
    if (AdrFlash1 <= AdrBH2)
    {
        if (wtemp < wAddr2)
            goto NEXT_BYTE;
    }
    return CRC;
}

void ProcessExch(void)
{
    unsigned char bByte;
    if (BTqueueOutLen == 0)   // only when nothing in BT output queue 
    {
        if (ATCMD & MODE_CONNECT)
        {
            if (!Main.SendOverLink)   // if it is no packet from COM
            {
                ptrMy = &BTqueueOut[0];
                if (ExchSendStatus == 1) // 1 - send packet with data from flash memory to another unit  
                {
                    if (btest(SSPORT,SSCS))
                    {
                        
                        CS_LOW;
                        SendSSByte(0x03);
                        SendSSByte(ExcgArd1);
                        SendSSByte(ExcgArd2);
                        SendSSByte(ExcgArd3);
                        //bByte=GetSSByte();
                        //CS_HIGH;
// #define FLASH_PCKT_SIZE 16 in Var002.h
                        BTpkt = PCKT_DATA;
                        *ptrMy++ = '*';
                        //              f\x0b\x00\x11\x22\x00\x00\x00\x00\x00\x00\x00\x00 == write 8 bytes to address 0x001122
                        *ptrMy++ = 'f';
                        *ptrMy++ = FLASH_PCKT_SIZE+3;
                        *ptrMy++ = ExcgArd1;*ptrMy++ = ExcgArd2;*ptrMy++ = ExcgArd3;
                        for (i = 0; i < FLASH_PCKT_SIZE; i++)
                        {
                            bByte = GetSSByte(); 
                            *ptrMy++ = bByte;
                            ExcgArd3++;
                            if (ExcgArd3 == 0)
                            {              
                                ExcgArd2++;
                                if (ExcgArd2 == 0)
                                {
                                    ExcgArd1++;
                                }
                            }
                        }
                        CS_HIGH;
                        BTqueueOutLen = FLASH_PCKT_SIZE+6;
                        ATCMD |= SOME_DATA_OUT_BUFFER;
                        if (ExcgLen<= FLASH_PCKT_SIZE)
                        {
                            ExcgLen = 0;
                            ExchSendStatus = 0; // done send
                        }
                        else
                        {
                            ExcgLen-= FLASH_PCKT_SIZE;
                            ExchSendStatus = 2; // wait for a next transmit
                        }
                    }
                }
                else if (ExchSendStatus == 2) // 2 - wait for interrupt from BT == send was done and then switch for back ExchSendStatus=1 == to send next packet
                {                             // because output message in BT TX queue - it is savely to set ExchSendStatus=1 on first occasion
                    ExchSendStatus = 1;
                }
                else if (ExchSendStatus == 10) // 3. send command "*S000000=4100" to different unit
                {
GET_ALL_PKT_FROM_FLASH:
                    BTpkt = PCKT_DATA;
                    *ptrMy++ = '*'; *ptrMy++ = 'S';
                    *ptrMy++ = ExcgArd1; *ptrMy++ = ExcgArd2; *ptrMy++ = ExcgArd3; *ptrMy++ = '=';
                    *ptrMy++ = ExcgLen >> 8; *ptrMy++ = ExcgLen & 0xff;
                    BTqueueOutLen = 8;
                    ExchSendStatus = 11;
                    ATCMD |= SOME_DATA_OUT_BUFFER;
                }
                else if (ExchSendStatus == 12) // 3. all pakets done no needs to go over BITS array to resend lost data
                {

                    ExcgArd1 = ExcgArd1Init;  ExcgArd2 = ExcgArd2Init; ExcgArd3 = ExcgArd3Init;
                    ExcgLen = ExcgLenInit;
                    ExchByte = 0;
                    j = 0xff;
                    CRC1Cmp = 0;
                    for (ExchByte = 0; ExchByte <FLASH_ARRAY_BITS; ExchByte++)
                    {
                        if (FLASH_SEND[ExchByte] != 0xff)
                        {
                            ExchBits = 0x80;
                            bByte = FLASH_SEND[ExchByte];
                            for (i=0; i< 8; i++)
                            {
                                if ((bByte & ExchBits) == 0) // skipped packet ??
                                {
                                    if (j == 0xff)
                                    {
                                        bByte1 = ExcgArd1; bByte2 = ExcgArd2; bByte3 = ExcgArd3;
                                        j = i;
                                    }
                                    CRC1Cmp += FLASH_PCKT_SIZE;
                                }
                                else // was the good packet
                                {
                                    if (j != 0xff) // done bad packet(s) detected
                                        break;
                                }
                                ExcgArd3+=FLASH_PCKT_SIZE;
                                if (ExcgArd3 < FLASH_PCKT_SIZE)
                                {
                                    ExcgArd2++;
                                    if (ExcgArd2==0)
                                        ExcgArd1++;
                                }
                                ExcgLen -= FLASH_PCKT_SIZE;

                            }
                        }
                        else
                        {
                            if (j != 0xff) // done with sequential lost 8 packets
                                break;
#ifdef _NOT_16BYTE_PKT
                            for (i=0; i< 8; i++)
                            {
                                ExcgArd3+=FLASH_PCKT_SIZE;
                                if (ExcgArd3 < FLASH_PCKT_SIZE)
                                {
                                    ExcgArd2++;
                                    if (ExcgArd2==0)
                                        ExcgArd1++;
                                }
                            }
#else
                            ExcgArd2++;
                            if (ExcgArd2==0)
                                ExcgArd1++;
                            ExcgLen -=0x100;

#endif
                            ExchByte++;
                        }
                    }
                    if (CRC1Cmp != 0)
                    {
                        ExcgArd1 = bByte1;  ExcgArd2 = bByte2; ExcgArd3 = bByte3;
                        ExcgLen = CRC1Cmp;
                        goto GET_ALL_PKT_FROM_FLASH;
                    }
                    else // done really 
                    {
                        ExchSendStatus = 0;
                    }
                }
            }    
        }
    }
}