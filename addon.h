
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
unsigned char AdwancePkt(void)
{
    ExcgArd3+=FLASH_PCKT_SIZE;
    if (ExcgArd3 < FLASH_PCKT_SIZE)
    {
        ExcgArd2++;
        if (ExcgArd2 == 0)
            ExcgArd1++;
    }
    ExchBits>>=1;
    if (ExchBits == 0)
    {
        ExchBits=0x80;
        ExchByte++;
    }
    ExcgLen -= FLASH_PCKT_SIZE;
    if ((ExcgLen ==0) || (ExcgLen > 0xfff0))
        return 1;
    return 0;
}
#define EXCG_SEND_PACKET_1          1
#define EXCG_WAIT_INT_FROM_BT_2     2
#define EXCG_PING_10               10
#define EXCG_WAIT_PING_DONE_11     11
#define EXCG_SEND_S_CMD_12         12
#define EXCG_WAIT_TO_OR_END_14     14
#define EXCG_ALL_PKTS_DONE_15      15
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
                if (ExchSendStatus == EXCG_SEND_PACKET_1) // 1 - send packet with data from flash memory to another unit  
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
                        }
                        CS_HIGH;

                        BTqueueOutLen = FLASH_PCKT_SIZE+6;
                        ATCMD |= SOME_DATA_OUT_BUFFER;
                        if (AdwancePkt())
                            ExchSendStatus = 0; // done send
                        else
                            ExchSendStatus = EXCG_WAIT_INT_FROM_BT_2; // wait for a next transmit
                    }
                }
                else if (ExchSendStatus == EXCG_WAIT_INT_FROM_BT_2) // 2 - wait for interrupt from BT == send was done and then switch for back ExchSendStatus=1 == to send next packet
                {                             // because output message in BT TX queue - it is safely to set ExchSendStatus=1 on first occasion
                    ExchSendStatus = EXCG_SEND_PACKET_1;
                }
                else if (ExchSendStatus == EXCG_PING_10) // 3. needs to ping to get distance BTV transmitters and reset all TX - RX timers
                {
                    OldUnitFrom = UnitFrom;
                    UnitFrom = 0; // to process block distance data from send over comm
                    Main.DoPing =1;
                    PingAttempts = 2;
                    Main.ConstantPing = 0;
                    ExchSendStatus = EXCG_WAIT_PING_DONE_11; // wait for a ping done
                    ExcgTime = Timer1HCount;
                }
                else if (ExchSendStatus == EXCG_WAIT_PING_DONE_11) // 3. wait for a ping done
                {
                    if (!Main.DoPing)
                    {
                        ExchSendStatus = EXCG_SEND_S_CMD_12; // send "*S000000=4100" to different unit
                        UnitFrom = OldUnitFrom;
                        ExcgTime = DistTimeOut;
                    }

                }
                else if (ExchSendStatus == EXCG_SEND_S_CMD_12)// 3. send command "*S000000=4100" to different unit
                {

                    BTpkt = PCKT_DATA;
                    *ptrMy++ = '*'; *ptrMy++ = 'S';
                    *ptrMy++ = ExcgArd1; *ptrMy++ = ExcgArd2; *ptrMy++ = ExcgArd3; *ptrMy++ = '=';
                    *ptrMy++ = ExcgLen >> 8; *ptrMy++ = ExcgLen & 0xff;
                    BTqueueOutLen = 8;
                    ExchSendStatus = EXCG_WAIT_TO_OR_END_14;
                    ATCMD |= SOME_DATA_OUT_BUFFER;
                    // now need to calculate time for all packets to be receved (responce of the last packet)
                    // Timer1HCount is ticks each TX time - 3 tick - one message send - all time is == ExcgLen / 16 * 3 ticks + dowble distance btw transmitters
                    ExcgTime0 = Timer1HCount;
                    ExcgTime += ExcgTime0;
                    CRC1Cmp = ExcgLen>>3;
                    ExcgTime += CRC1Cmp;
                    CRC1Cmp>>=1;
                    ExcgTime += CRC1Cmp;
                    if (ExcgTime > ExcgTime0)  // normal case
                        DataB0.ExcgTOCmpType =1;
                    else
                        DataB0.ExcgTOCmpType =0;
                }
                else if (ExchSendStatus == EXCG_WAIT_TO_OR_END_14)  //waiting for TO or the end of data 
                {                                                    // TO chaked here and last packet set in FLASH write
                    if (DataB0.ExcgTOCmpType)
                    {
                        if (Timer1HCount > ExcgTime) // TO
                            goto WAS_TO;
                        if (Timer1HCount <= ExcgTime0) // TO
                            goto WAS_TO;
                    }
                    else
                    {
                        if (Timer1HCount > ExcgTime)
                            if (Timer1HCount <= ExcgTime0) // TO
                                goto WAS_TO;
                    }
                }
                else if (ExchSendStatus == EXCG_ALL_PKTS_DONE_15) // 3. all pakets done no needs to go over BITS array to resend lost data
                {
WAS_TO:
                    ExcgArd1 = ExcgArd1Init;  ExcgArd2 = ExcgArd2Init; ExcgArd3 = ExcgArd3Init;
                    ExcgLen = ExcgLenInit;
                    ExchByte = 0;
                    j = 0xff;
                    CRC1Cmp = 0;
                    ExchBits = 0x80;
                    for (ExchByte = 0; ExchByte <FLASH_ARRAY_BITS; )
                    {
                        if (FLASH_SEND[ExchByte] != 0xff)
                        {
                            bByte = FLASH_SEND[ExchByte];
                            for (i=0; i< 8; i++)
                            {
                                if ((bByte & ExchBits) == 0) // skipped packet ??
                                {
                                    if (j == 0xff)
                                    {
                                        bByte1 = ExcgArd1; bByte2 = ExcgArd2; bByte3 = ExcgArd3;
                                        j = ExchByte;
                                        bByteOut = ExchBits; 
                                    }
                                    CRC1Cmp += FLASH_PCKT_SIZE;
                                }
                                else // was the good packet
                                {
                                    if (j != 0xff) // done bad packet(s) detected
                                        goto ALL_CHECK_SEND;
                                }
                                if (AdwancePkt())
                                    goto ALL_CHECK_SEND;
                            }
                        }
                        else
                        {
                            if (j != 0xff) // done with sequential lost 8 packets
                                goto ALL_CHECK_SEND;
                            for (i=0; i< 8; i++)
                            {
                                if (AdwancePkt())
                                    goto ALL_CHECK_SEND;
                            }
                        }
                    }
ALL_CHECK_SEND:
                    if (CRC1Cmp != 0)
                    {
                        ExcgArd1 = bByte1;  ExcgArd2 = bByte2; ExcgArd3 = bByte3;
                        ExcgLen = CRC1Cmp;
                        ExchByte = j;
                        ExchBits = bByteOut;
                        ExchSendStatus = EXCG_PING_10; // resend losted packets
                    }
                    else // done really 
                    {
                        ExchSendStatus = 0;
                    }
                }
                if (ATCMD & MODE_CALL_LUNA_COM) // earth calls cubsat sequence:
                {
                    if (Main.PingRQ || Main.PingRSPRQ)
                    {
                        //if (BTqueueOutLen == 0)   // only when nothing in BT output queue
                        //if (!Main.SendOverLink)   // if it is no packet from COM 
                        BTqueueOut[0] = 'P'; BTqueueOut[1] = 'I';BTqueueOut[2] = 'N';BTqueueOut[3] = 'G';
                        Main.PingRQ = 0;
                        Main.PingRSPRQ = 0;
                        goto SEND_PKT_TX_MODE;
                    }
                }
                else if (ATCMD & MODE_CALL_EARTH) // cubsat calls earth
                {
                    if (ATCMD & RESPONCE_WAS_SENT) // responce to earth was send
                    {
#ifdef DEBUG_LED_CALL_EARTH
                        if (Main.PingRQ || Main.PingRSPRQ)
                        //{
                        //     if (BTqueueOutLen == 0)   // only when nothing in BT output queue
                        //     {
                        //         BTqueueOut[0] = 'p'; BTqueueOut[1] = 'i';BTqueueOut[2] = 'n';BTqueueOut[3] = 'g';
                        //         //BTqueueOut[0] = 'e';BTqueueOut[1] = 'a';BTqueueOut[2] = 'r';BTqueueOut[3] = 'z';
                        //         Main.PingRQ = 0;
                        //         Main.PingRSPRQ = 0;
                        //         goto SEND_PKT_TX_MODE;
                        //         
                        //     }
                        //}
#else
                        if (Main.PingRSPRQ)
#endif
                        {
                            //if (BTqueueOutLen == 0)   // only when nothing in BT output queue
                            //if (!Main.SendOverLink)   // if it is no packet from COM 
                            BTqueueOut[0] = 'p'; //BTqueueOut[1] = 'i';BTqueueOut[2] = 'n';BTqueueOut[3] = 'g';
                            Main.PingRSPRQ = 0;
                            ptrTemp = (unsigned char *)&DistMeasure;
                            //FSR1 = &BTqueueOut[1];
                            ptrMy++; // &BTqueueOut[1]
                            for (i = 0; i < sizeof(DistMeasure); i++)
                            {
                                //INDF1 = PTR_FSR;
                                bByte = *ptrTemp;
                                *ptrMy = bByte;
                                ptrTemp++;//FSR_REGISTER++;
                                ptrMy++;//FSR1++;
                            }
                            BTpkt = PCKT_DIAL;
                            BTqueueOutLen = 1+sizeof(DistMeasure);
                            ATCMD |= SOME_DATA_OUT_BUFFER;
                        }
                    }
                    else                           // responce to earth was not send from luna
                    {
                        //if (BTqueueOutLen == 0)   // only when nothing in BT output queue
                        //if (!Main.SendOverLink)   // if it is no packet from COM 
                        BTqueueOut[0] = 'e';BTqueueOut[1] = 'a';BTqueueOut[2] = 'r';BTqueueOut[3] = 'z';
                        ATCMD |= RESPONCE_WAS_SENT;
                        goto SEND_PKT_DIAL;
                    }
                }
            }    
        }
        else // not if (ATCMD & MODE_CONNECT)
        {
            if (!Main.SendOverLink)   // if it is no packet from COM
            {
                if (ATCMD & MODE_CALL_LUNA_COM) // earth calls cubsat sequence:
                {
                    // earth call cubesat that is a main mode == earth initiate transmission
                    // 1. send msg on FQ1
                    // 2. send msq on FQ2
                    // 3. send msg on FQ3 and switched to RX mode
                    // 4. listen on FQ1 with main timeout = (distance x 2)/C
                    // 5. on timeout go back to point. 1
                    // 6. on msg receive over FQ1 -> check CRC == OK then message marked as OK -> switch to FQ2
                    // 7. wait ether for msg or timeout 
                    // 8. msg recived/ timeout -> check CRC == OK -> message marked as OK -> switch to FQ3
                    //                                    timeout -> switch to FQ3
                    // 9. wait for msg or timeout
                    // 10. msg received/ timeout -> check CRC == OK -> message marked as OK -> switch to FQ1
                    //                                      timeout -> switch to FQ1
                    //                                        a) one of messages marked as OK -> process message
                    //                                        b) attempt to fix message based on 3 wrong messages
                    //                                        c) attempt to fix message based on 2 wrong message

                    // no connection yet earth == wait for responce from luna
                    if (ATCMD & MODE_DIAL) // is it dialed or not
                    {
                    }
                    else // was not dialed yet
                    {
                        //if (BTqueueOutLen == 0)   // only when nothing in BT output queue
                        if (!Main.SendOverLink)   // if it is no packet from COM 
                        {
                            BTqueueOut[0] = 'l'; BTqueueOut[1] = 'u';BTqueueOut[2] = 'n';BTqueueOut[3] = 'a';

                            ATCMD |= MODE_DIAL;
SEND_PKT_TX_MODE:
                            if (ATCMD & INIT_BT_NOT_DONE)
                            { 
                                 SetupBT(SETUP_RX_MODE);
                                 ATCMD &= (INIT_BT_NOT_DONE^0xff);
                                 //DataB0.RXLoopBlocked =1;
                                 DataB0.Tmr3Run = 0;
                                 FqRXCount = 0;
                                 FqRX = Freq1;
                                 FqRXCount = 0;
                                 SwitchToRXdata();
                                 DataB0.RXMessageWasOK = 0;
                                 DataB0.RXPktIsBad = 0;
                                 DataB0.RXLoopBlocked = 0;
                                 DataB0.RXLoopAllowed = 1;
                                 //SetTimer0(TIME_FOR_PACKET0);
                                 DataB0.Timer1LoadLowOpponent = 0;
                            }
SEND_PKT_DIAL:
                            BTqueueOut[4] = Addr1;BTqueueOut[5] = Addr2;BTqueueOut[6] = Addr3;
                            // this is place where frequency for satellite can be adjusted because of temperature drift in satellite
                            // ground station monitors frequencies and found shift then send channels numbers
                            BTqueueOut[7] = Freq1;BTqueueOut[8] = Freq2;BTqueueOut[9] = Freq3;
                            if (DataB0.Timer1Count)
                            {
                                BTqueueOut[10] = (unsigned char)(Tmr1LoadLow&0xFF);
                                BTqueueOut[11] = (Tmr1LoadLow>>8);
                            }
                            else
                            {
                                BTqueueOut[10] = 0x00;
                                BTqueueOut[11] = 0x00;
                            }
                            BTpkt = PCKT_DIAL;
                            BTqueueOutLen = 12;
                            ATCMD |= SOME_DATA_OUT_BUFFER;
                        } 
                    }
                }
                else if (ATCMD & MODE_CALL_EARTH) // cubsat calls earth
                {
                    // no connection yet == luna waits for a communication
                    if (ATCMD & INIT_BT_NOT_DONE)
                    { 
                        SetupBT(SETUP_RX_MODE);
                    
                        ATCMD &= (INIT_BT_NOT_DONE^0xff);
                        DataB0.Tmr3Run = 0;
                        FqRXCount = 0;
                        FqRX = Freq1;
                        SwitchToRXdata();
                        //SetTimer0(TIME_FOR_PACKET0);
                        //DataB0.RXLoopBlocked =0;
                        DataB0.Tmr3DoneMeasureFq1Fq2 = 0;
                        DataB0.RXMessageWasOK = 0;
                        DataB0.RXPktIsBad = 0;
                        DataB0.RXLoopBlocked = 0;
                        DataB0.RXLoopAllowed = 1;
                        DataB0.Timer1LoadLowOpponent = 0;
                    }
                }
            }
        }
    }
}
