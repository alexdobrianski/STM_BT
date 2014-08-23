
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
		INDF1 = PTR_FSR;
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
    {
        OutputMsgLen = iTotalLen;
        return 0;
    }
RETURN_ERROR:
    OutputMsgLen = 0;
    return 0xff;
}

// 
unsigned char BTFixlen(unsigned char*MyData, unsigned char iLen)
{
   iShift = 0;
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
                    iShift = 6;
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
           else 
           {
               FSR_REGISTER++;
           }
           i++;
       }
       while(i<11);
       return 0; 
FIND_NONPRT:
#if 0
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
#endif


       i -=3; // that will be byte shift offset
       res = i;
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
           FSR1 = MyData;
           FSR_REGISTER = FSR1+i;
           j = i;
           i = iLen - i -1;
           do
           {
              INDF1 = PTR_FSR; FSR_REGISTER++; FSR1++;
           }
           while(--i);
           do
           {
               INDF1 = 0;FSR1++;
           }while(--j);
       }
       if (iShift)
       {
           //j++;
           FSR_REGISTER=&MyData[1];
           bByte1 = 0xaa;
           i = iLen-res - 1;
           do
           {
               bByte2 = PTR_FSR;
               if (iShift == 2)
               {
#ifdef      _18F2321_18F25K20
                   #asm
                   RLNCF bByte2,1,1
                   RLNCF bByte2,1,1
                   #endasm
#else
                   bByte2 = (bByte2 << 2) || (bByte2 >> 6);
#endif             
                   bByte1 = bByte2 & 0x03;
                   bByte2 &= 0xfc;
               }
               else if (iShift == 4)
               {
#ifdef      _18F2321_18F25K20
                   #asm
                   SWAPF bByte2,1,1
                   #endasm
#else
                   bByte2 = (bByte2 << 4) || (bByte2 >> 4);
#endif             
                   bByte1 = bByte2 & 0xF;
                   bByte2 &= 0xf0;

               }
               else if (iShift == 6)
               {
#ifdef      _18F2321_18F25K20
                   #asm
                   SWAPF bByte2,1,1
                   RLNCF bByte2,1,1
                   RLNCF bByte2,1,1
                   #endasm
#else
                   bByte2 = (bByte2 << 6) || (bByte2 >> 2);
#endif             
                   bByte1 = bByte2 & 0x3F;
                   bByte2 &= 0xc0;
               }
               PTR_FSR  = bByte2;
               FSR_REGISTER--;
               PTR_FSR  |= bByte1;
               FSR_REGISTER++;
               FSR_REGISTER++;
           }
           while(--i);
       } 
       if ((MyData[0] == 0xaa) || (MyData[1] == 0xaa) || (MyData[2] == 0xaa))
       {
           FSR_REGISTER = MyData;
           PTR_FSR = 0xaa;
           FSR_REGISTER++;
           PTR_FSR = 0xaa;
           FSR_REGISTER++;
           PTR_FSR = 0xaa;
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
   return iLen - res; 
}

unsigned char BTFix3(void)
{
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
#ifndef WIN32
        if (i<4)
        {
            bByteOut = bByte1 = PTR_FSR;
            mask = (bByte1 ^ INDF1);
            bByteOut &= mask ^ 0xff; 
            bByteOut |= mask & INDF2;
            wCRCupdt(bByteOut);
            ptrTemp = FSR_REGISTER;
            FSR_REGISTER = ptrOut;
            PTR_FSR = bByteOut;
            FSR_REGISTER = ptrTemp;
        }
        else if (i > 4)
        {
#endif
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
            ptrTemp = FSR_REGISTER;
            *ptrOut = 0;
            FSR_REGISTER = ptrTemp;
        }
        if (i == PACKET_LEN_OFFSET)
        {
            if (bByteOut<= BT_TX_MAX_LEN)
            {
                iTotalLen = bByteOut + PACKET_LEN_OFFSET+1+2;// + sizeof(PacketStart);
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
    {
        OutputMsgLen = iTotalLen;
        return 0;
    }
RETURN_ERROR:
    OutputMsgLen = 0;
    return 0xff;
}

unsigned char BTFix2(void)
{
    Vote = 128;
    iTotalLen = BTqueueInLen;
    BTFlags.Check01 = 0;
    if ((BTqueueInLen > 0) && (BTqueueInLen2 > 0))
    {
#ifdef WIN32
        FSR_REGISTER = &BTqueueIn[0];
        FSR1 = &BTqueueIn2[0];
#else
        FSR_REGISTER = &BTqueueIn[3];
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
        FSR_REGISTER = &BTqueueIn2[3];
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
        FSR_REGISTER = &BTqueueIn[3];
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

    FSR2 = &OutputMsg[0];
    CRCcmp=0;
#ifdef WIN32
    CRC=0xffff;   
#else
    CRC=0x50d4;  
    INDF2=0xAA;FSR2++;
    INDF2=0xAA;FSR2++;
    INDF2=0xAA;FSR2++;
    INDF2=PTR_FSR;FSR2++;
    //wCRCupdt(0xaa);
    //wCRCupdt(0xaa);
    //wCRCupdt(0xaa);
    wCRCupdt(PTR_FSR);
    FSR_REGISTER++;
    INDF2=0;FSR2++;
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
                goto CHECK_FIRST_MSG;
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
CHECK_FIRST_MSG:
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
                iTotalLen = bByteOut + PACKET_LEN_OFFSET+1+2;// + sizeof(PacketStart);
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
    {
        OutputMsgLen = iTotalLen ;
        return 0;
    }
RETURN_ERROR:
    OutputMsgLen = 0;
    return 0xff;
}
