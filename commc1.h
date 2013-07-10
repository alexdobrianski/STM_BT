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
               work2 = RCREG;
#ifdef SYNC_CLOCK_TIMER

               if (!Main.getCMD) // CMD not receved et.
               {
                   if (work2 == MY_UNIT) // record time of received message
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
#ifdef USE_OLD_CMD_EQ
               I2C.RetransComI2C = 0;
#endif
               //Main.PrepI2C = 0;
               continue; // can be another bytes
NO_RC_ERROR:
#ifdef NEW_CMD_PROC
               // optimized version
               
               if (Main.prepSkip)     // was skip = retransmit byte - top priority
               {
                   Main.prepSkip = 0;
                   Main.prepZeroLen = 0;
                   goto RELAY_SYMB; // ===> retransmit
               }
               else if (RetrUnit) // relay data to next unit during processing streaming == stream to another unit retransmit directly without entering queue
               {
                   if (work2 == ESC_SYMB) // do relay but account that is was ESC char 
                   {
                       Main.prepZeroLen = 0;
                       Main.prepSkip = 1;//====> retransmit
RELAY_SYMB:
                       // exact copy from putchar == it is big!!! but it can be called recursivly!!
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
                               TXREG = work2; // this will clean TXIF on 88,884 and 2321
                           }
                           else // case when something has allready send directly
                           {
#ifdef __PIC24H__
                               TXIF = 0; // for pic24 needs to clean uart interrupt in software
                               if (!U1STAbits.UTXBF) // on pic24 this bit is empy when at least there is one space in Tx buffer
                                   TXREG = work2;   // full up TX buffer to full capacity, also cleans TXIF
                               else
                                   goto SEND_BYTE_TO_QU; // placing simbol into queue will also enable uart interrupt
#else
                               if (!I2C.SendComOneByte)      // one byte was send already 
                               {
                                   TXREG = work2;           // this will clean TXIF 
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
                               AOutQu.Queue[AOutQu.iEntry] = work2; // add bytes to a queue
                               if (++AOutQu.iEntry >= OUT_BUFFER_LEN)
                                   AOutQu.iEntry = 0;
                               AOutQu.iQueueSize++; // this is unar operation == it does not interfere with interrupt service decrement
                               //if (!Main.PrepI2C)      // and allow transmit interrupt
                               TXIE = 1;  // placed simbol will be pushed out of the queue by interrupt
                           } 
                       }
                       goto END_INPUT_COM;
                       /////////////////////////////////////////////////////////////////////////////////////////////
                       //   end of direct output to com1
                       /////////////////////////////////////////////////////////////////////////////////////////////
                   }
                   else if (work2 == RetrUnit) // relay done
                   {
                       if (Main.prepZeroLen) // packets with 0 length does not exsists
                           goto RELAY_SYMB; // ===> retransmit
                       RetrUnit = 0;
                       goto RELAY_SYMB; // ===> retransmit
                   }
                   else if (work2 == MY_UNIT)
                   {
                       RetrUnit = 0;
                       goto SET_MY_UNIT;
                   }
                   else if (work2 <= MAX_ADR)
                   {            
                       if (work2 >= MIN_ADR) // msg to relay
                           goto TO_ANOTHER_UNIT;
                   }
                   
                   Main.prepZeroLen = 0;
                   goto RELAY_SYMB; // ===> retransmit
               }
               else if (Main.getCMD)
               {
                   if (Main.CheckESC)
                   {
                      Main.CheckESC = 0;  // =======> process message => insert in queue
                   }
                   else if (work2 == ESC_SYMB)
                   {
                        Main.CheckESC = 1;
                   }
                   else if (work2 == MY_UNIT)
                      ; // =======> process message => insert in queue
                   else if (work2 <= MAX_ADR)
                   {            
                       if (work2 >= MIN_ADR) // msg to relay
                          // retransmit to another unit has a priority - current status freazed
                           goto TO_ANOTHER_UNIT;
                   }         
                   ;  // =======> process message => insert in queue
               }
               else // not a command mode == stream mode 
               if (work2 == ESC_SYMB) // do relay
               {
                   Main.prepZeroLen = 0;
                   Main.prepSkip = 1;//====> retransmit
                   goto RELAY_SYMB; // ===> retransmit
               }
               else if (work2 == MY_UNIT) // message addresed to a unit
               {
SET_MY_UNIT:
                   Main.getCMD =1; // byte eated
                   I2C.LastWasUnitAddr = 1;
                   Main.CheckESC = 0;
                   Main.ESCNextByte = 0;
                   goto END_INPUT_COM;
               }
               else
               {
                   if (work2 <= MAX_ADR)
                   {            
                       if (work2 >= MIN_ADR) // msg to relay to another untis
                       {
TO_ANOTHER_UNIT:          
                           RetrUnit = work2;
                           Main.prepZeroLen = 1;
                           Main.prepSkip = 0;
                       }
                   }
                   goto RELAY_SYMB; // ===> retransmit
               }
//////////////////////////////////////////////////////////////////
// end of optimized version
//////////////////////////////////////////////////////////////////
#else
               if (RetrUnit) // relay data to next unit during processing streaming == stream to another unit retransmit directly without entering queue
               {
                   if (Main.prepStream)
                   {
                      
                       if (Main.prepSkip)     // was skip = clean and relay
                       {
                           Main.prepSkip = 0;
                           Main.prepZeroLen = 0;
                           goto RELAY_SYMB; // ===> retransmit
                       }
                       else if (work2 == ESC_SYMB) // do relay
                       {
                           Main.prepZeroLen = 0;
                           Main.prepSkip = 1;//====> retransmit
RELAY_SYMB:
                           // exact copy from putchar == it is big!!! but it can be called recursivly!!
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
                                   TXREG = work2; // this will clean TXIF on 88,884 and 2321
                               }
                               else // case when something has allready send directly
                               {
#ifdef __PIC24H__
                                    TXIF = 0; // for pic24 needs to clean uart interrupt in software
                                    if (!U1STAbits.UTXBF) // on pic24 this bit is empy when at least there is one space in Tx buffer
                                        TXREG = work2;   // full up TX buffer to full capacity, also cleans TXIF
                                    else
                                        goto SEND_BYTE_TO_QU; // placing simbol into queue will also enable uart interrupt
#else
                                    if (!I2C.SendComOneByte)      // one byte was send already 
                                    {
                                        TXREG = work2;           // this will clean TXIF 
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
                                   AOutQu.Queue[AOutQu.iEntry] = work2; // add bytes to a queue
                                   if (++AOutQu.iEntry >= OUT_BUFFER_LEN)
                                       AOutQu.iEntry = 0;
                                   AOutQu.iQueueSize++; // this is unar operation == it does not interfere with interrupt service decrement
                                   //if (!Main.PrepI2C)      // and allow transmit interrupt
                                   TXIE = 1;  // placed simbol will be pushed out of the queue by interrupt
                               } 
                           }
                           goto END_INPUT_COM;
                           /////////////////////////////////////////////////////////////////////////////////////////////
                           //   end of direct output to com1
                           /////////////////////////////////////////////////////////////////////////////////////////////
                       }
                       else if (work2 == RetrUnit) // relay done
                       {
                           if (Main.prepZeroLen) // packets with 0 length does not exsists
                               goto RELAY_SYMB; // ===> retransmit
                           RetrUnit = 0;
                           Main.prepStream = 1;
#ifdef ALLOW_RELAY_TO_NEW
                           AllowMask = AllowOldMask;AllowOldMask= AllowOldMask1;AllowOldMask1= AllowOldMask2;AllowOldMask2= AllowOldMask3;AllowOldMask3= AllowOldMask4; // restore allow mask
                           AllowOldMask4 = 0xff;
#else
                           AllowMask = 0xff; // it is possible to send msg to any unit over COM
#endif
                           goto RELAY_SYMB; // ===> retransmit
                       }
#ifdef ALLOW_RELAY_TO_NEW
#endif
                       Main.prepZeroLen = 0;
                       goto RELAY_SYMB; // ===> retransmit
                   }
                   // ===> process simbol
               }
#endif
INSERT_TO_COM_Q:
               if (AInQu.iQueueSize < BUFFER_LEN)
               {
                   AInQu.Queue[AInQu.iEntry] = work2;
                   if (++AInQu.iEntry >= BUFFER_LEN)
                       AInQu.iEntry = 0;
#pragma updateBank 0
                   AInQu.iQueueSize++;
#pragma updateBank 1
               }
               //else
               //    W = RCREG; // this byte is skipped to process
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
#ifdef SPEED_SEND_DATA
           if (Speed.SpeedSendLocked)
               goto SPEED_SEND;
#endif
           if (AOutQu.iQueueSize)
           {
               //if (!BlockComm) // com and I2C shared same output queue - in case of I2C nothing goes to com
               {
                   // load to TXREG will clean TXIF
                   TXREG = AOutQu.Queue[AOutQu.iExit];
                   if (++AOutQu.iExit >= OUT_BUFFER_LEN)
                       AOutQu.iExit = 0;
                   AOutQu.iQueueSize--;
               }
           }
           else
           {
SPEED_SEND:
#ifdef SPEED_SEND_DATA
               if (Speed.SpeedSend)
               {
                   Speed.SpeedSendLocked = 1;
                   if (Speed.SpeedSendUnit)
                   {
                       TXREG = UnitFrom;
DONE_WITH_SPEED:
                       Speed.SpeedSend =0;
                       Speed.SpeedSendLocked = 0;
                       goto CONTINUE_WITH_ISR;        
                   }
                   if (Speed.SpeedSendWithESC)
                   {
                       if (Speed.SpeedESCwas)
                       {
                           Speed.SpeedESCwas = 0;
                           goto SPEED_TX;
                       }
                       work1 = ptrSpeed[LenSpeed];
                       if (work1 == ESC_SYMB)
                       {
                           // escape it
ESCAPE_IT:                 TXREG = ESC_SYMB;
                           Speed.SpeedESCwas = 1;
                           goto CONTINUE_WITH_ISR;        
                       }
                       if (work1 < MIN_ADR)
                           goto SPEED_TX;
                       if (work1 > MAX_ADR)
                           goto SPEED_TX;
                       goto ESCAPE_IT;
                   }
SPEED_TX:
                   TXREG = ptrSpeed[LenSpeed];
                   if ((--LenSpeed) == 0)
                   {
                        if (UnitFrom)
                           Speed.SpeedSendUnit = 1;
						else                        
                           goto DONE_WITH_SPEED;
                   }
                   goto CONTINUE_WITH_ISR; 
              }
#endif

               if (_TRMT)    // if nothing ina queue and transmit done - then disable interrupt for transmit
               {             // otherwise it will be endless
                    // for speed up output - first bytes already send + at the end needs to send UnitAddr
                   TXIE = 0;
               }
               else // transmit buffer has something in it (also for pic24 in a bufer there is a data)
               {
#ifdef SPEED_SEND_DATA
                  if (!Speed.SpeedSend) // nothing in a output queue and speed send is done then needs to disable interrupt to
#endif
                      TXIE = 0;         // avoid reentry of interrupt
                  //I2C.SendComOneByte = 0;
               }
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
#ifdef SHOW_RX_TX
   #ifdef SHOW_RX
   #else
                      DEBUG_LED_OFF;
   #endif
#endif

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
    IF_TMR3IF // RX timer
    {
        TMR3IF = 0;
        

        if (DataB0.Timer3Meausre)
        {
            Tmr3High++; // timer count and interrupts continue
        }
        else if (DataB0.Timer3Count)
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
                    DataB0.Timer3Inturrupt = 1;
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
            
#ifdef BT_TIMER3
            if (BTType & 0x01) // it was RX operation
            {
                if (DataB0.Timer3SwitchRX)
                    bitclr(PORT_BT,Tx_CE);	// Chip Enable Activates RX or TX mode (now standby)

                if (DataB0.Timer3DoneMeasure) // receive set
                {
                    SkipPtr++; // set of next frquency will be in CallBackMain
                    AdjustTimer3 = TIMER3;
                    DataB0.Timer3Ready2Sync = 1;
                }
                else // needs to monitor FQ1 and FQ2 receive time
                {
                    if (RXreceiveFQ == 0) // it is receive over Fq1 == need to start timer to record time btw Fq1 and FQ2
                    {
                        DataB0.Timer3Meausre = 1;
                        TMR3H = 0;
                        TMR3L = 0;
                        TMR3IF = 0;
                        TMR3IE = 1;
                        Tmr3High  = 0;
                        T3CON = 0b10000001;
                    }
                    else if (RXreceiveFQ == 1) // it was receive over Fq2
                    {
                        if (DataB0.Timer3Meausre) // timer for a measure was started ??
                        {
                            TMR3ON = 0;
                            Tmr3LoadLowCopy =0xFFFF - TIMER3;      // timer1 interupt reload values 
                            Tmr3LoadLowCopy += 52;
                            Tmr3LoadLow = Tmr3LoadLowCopy - MEDIAN_TIME;
                            TMR3H = (Tmr3LoadLow>>8);
                            TMR3L = (unsigned char)(Tmr3LoadLow&0xFF);
                            //TMR3L = 0;//xff;
                            TMR3ON = 1; // continue run
                            Tmr3TOHigh = Tmr3LoadHigh = 0xffff - Tmr3High;
                            DataB0.Timer3Meausre = 0;
                            DataB0.Timer3Count = 1;
                            DataB0.Timer3Inturrupt = 0;
                            //SkipPtr =1;
                            DataB0.Timer3OutSync = 0;
                        }
                    }
                }
                if (!DataB0.Timer3OutSync)
                    OutSyncCounter = 125; // 2.5 sec no packets == switch for out of sync
            }
            else
                bitclr(PORT_BT,Tx_CE);	// Chip Enable Activates RX or TX mode (now standby)
#endif

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
#ifdef NEW_CMD_PROC
#else
unsigned char Monitor(unsigned char bWork, unsigned char CheckUnit)
{
    if (Main.prepSkip)
    {
        Main.prepZeroLen = 0;
        Main.prepSkip = 0;
    }
    else if (bWork == ESC_SYMB) // it is ESC char ??
    {
        Main.prepSkip = 1;
        Main.prepZeroLen = 0;
    }
    else if (bWork == CheckUnit) // 
    {
        if (Main.prepZeroLen) // packets with zero length does not exsists
            return 0;   
        Main.prepCmd = 0;
        Main.prepStream = 1;
#ifdef ALLOW_RELAY_TO_NEW
        AllowMask = AllowOldMask;AllowOldMask= AllowOldMask1;AllowOldMask1= AllowOldMask2;AllowOldMask2= AllowOldMask3;AllowOldMask3= AllowOldMask4; // restore allow mask
        AllowOldMask4 = 0xff;
#else
        AllowMask = 0xff;
#endif
        return 1;
    }
    else
        Main.prepZeroLen = 0;
    return 0;
}
#endif
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//      END COPY1
/////////////////////////////////////////////////////////////////
