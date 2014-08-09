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
    if (ATCMD & MODE_CONNECT)
    {
        if (BTqueueOutLen == 0)   // only when nothing in BT output queue
        {
            if (!Main.SendOverLink)   // if it is no packet from COM 
            {
                if (ExchSendStatus)
                {
                    if (btest(SSPORT,SSCS))
                    {
                        CS_LOW;
                        SendSSByte(0x03);
                        SendSSByte(ExcgArd1);
                        SendSSByte(ExcgArd2);
                        SendSSByte(ExcgArd3);
                        GetSSByte();
                        CS_HIGH;

                        BTpkt = PCKT_DATA;
                        BTqueueOutLen = 12;
                        ATCMD |= SOME_DATA_OUT_BUFFER;
                        BTqueueOut[0] = 'l'; BTqueueOut[1] = 'u';BTqueueOut[2] = 'n';BTqueueOut[3] = 'a';
                    }
                }
            }    
        }
    }
}