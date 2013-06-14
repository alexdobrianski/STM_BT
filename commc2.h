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
void enable_I2C(void);
void EnableTMR1(void);
void putch(unsigned char simbol)
{
    if (AOutQu.iQueueSize == 0)  // if this is a com and queue is empty then needs to directly send byte(s) 
    {                            // on 16LH88,16F884,18F2321 = two bytes on pic24 = 4 bytes
        // at that point Uart interrupt is disabled
#ifdef __PIC24H__
        if (!U1STAbits.UTXBF) // on pic24 this bit is empy when at least there is one space in Tx buffer
        {
            TXEN = 1; // just in case ENABLE transmission
            //TXIF = 0; // for pic24 needs to clean uart interrupt in software
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

void putchI2C(unsigned char simbol)
{
    AOutI2CQu.Queue[AOutI2CQu.iEntry] = simbol; // add bytes to a queue
    if (++AOutI2CQu.iEntry >= OUT_BUFFER_LEN)
        AOutI2CQu.iEntry = 0;
    AOutI2CQu.iQueueSize++;
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
#pragma rambank RAM_BANK_1
//////////////////////////////////////////////BANK 1///////////////////////////


unsigned char eeprom_read(unsigned char addr);
void eeprom_write(unsigned char addr, unsigned char value);
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
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// end COPY 2
//////////////////////////////////////////////////////////////////////
