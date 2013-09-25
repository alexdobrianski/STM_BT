////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// begin COPY 7
///////////////////////////////////////////////////////////////////////   
// STREAM two types:
//    STREAM  ...=> processed inside a ProcessCMD in a area after "if (!getCMD)"
//    COMMANDS...=> processied inside a ProcessCMD in area "if (getCMD)" after #include "commc3.h"
//    STREAM or COMMANDS can be monitored in an area befor "if (!getCMD)"
//  COMMANDS...:= <OneCMD> || <LongCMD> on each command unit does action
//                                 action on ManyByte command depends
//                                 escape char is #:=> ##==#;#N==N(N=0-9)
// on <LongCMD> needs to clean DoneWithCMD = 0 and at the end of the command
// needs to set DoneWithCMD = 1 
// 
// stream from com:
// STREAM<Unit><COMMANDS><Unit>SSTREAM   or <unit><unit><COMMANDS><unit>
// STREAM                                 -> processed as !getCMD
//       <Unit>                           -> getCMD = 1;
//              COMMANDS                  -> processed as getCMD
//                       <Unit>           -> getCMD = 0;
//                             SSTREAM    -> processed as !getCMD
// stream from I2C: 
//    received data as slave device or receve desponce from different I2C device from as a master read request 
//    processed as a COMMANDS 
//    I2C_STREAM... == COMMANDS...
// 
//   
//  
//  receved data as responce on or just some data from inside unit
//    
// 
// b) retransmit to I2C device was set: UnitFrom=ToI2CAddr SendCMD=CMD RetransI2CCom = 1 RetransI2CComSet = 0
//    this can be done by command: =xcI where x=ToI2CAddr c=CMD  I=set retransmit
//    and 
// STREAM<Unit>=xcIstream<Unit>STREAM<unit>=xcIstram<unit>
//                 stream -> to I2C        stram-> to I2C
// 
// c)
// c) unit internaly reads something from I2C and 
//     I2Cstream-> process internaly

// stream from I2C:
// a) 
// <COMMAND>
//  CMD:= <OneByte> || <ManyBytes> on each command unit does action
//                                 action on ManyByte command is actualy on last byte

    while(1)
    {
#ifdef CHECK_NEXT
        if (Main.SuspendTX) // was suspend of a transmit because next unit was not ready to acsept data
        {
            if (!CHECK_NEXT)  // next unit is READY (low) to accsept data
            {
                if (AOutQu.iQueueSize)
                {
                    TXREG = AOutQu.Queue[AOutQu.iExit];
                    if (++AOutQu.iExit >= OUT_BUFFER_LEN)
                        AOutQu.iExit = 0;
                    AOutQu.iQueueSize--;
                }
                Main.SuspendTX = 0;
                if (Main.SuspendRetrUnit)
                {
                    RX_READY = 0;
                    Main.SuspendRetrUnit = 0;
                }    
            } 
        }
#endif
        if (CallBkMain() == 0) // 0 = do continue; 1 = process queues
            continue;
#ifndef NO_I2C_PROC
        if (AInI2CQu.iQueueSize) // if something comes from I2C (slave received or some I2c device responded on read command)
        {
            if (CallBkI2C())// 0 = do not process byte; 1 = process;
            {
                bitclr(bWork,0);
                if (Main.getCMD)
                    bitset(bWork,0);
                Main.getCMD = 1;
                ProcessCMD(getchI2C());
                Main.getCMD = 0;
                if (bittest(bWork,0))
                    Main.getCMD = 1;

            }
        }
#endif // #ifndef NO_I2C_PROC
        if (AInQu.iQueueSize)      // in comm queue bytes
        {
            if (CallBkComm()) // 0 = do not process byte; 1 = process;
            {
PROCESS_IN_CMD:
                 ProcessCMD(getch());
#ifdef RX_READY
               if (Main.PauseInQueueFull)
               {
                   if (AInQu.iQueueSize > (BUFFER_LEN-CRITICAL_BUF_SIZE))
                       ;//RX_READY = 1;
                   else
                   {
                       RX_READY = 0;
                       Main.PauseInQueueFull = 0;
                   }
               }
#endif

NO_PROCESS_IN_CMD:;
            }
        }
        else  // nothing in both queue can sleep till interrupt
        {
#ifdef NON_STANDART_MODEM
            if (FlashEntry == FlashExit)
            {
                if (Adr2BHEntry == Adr2BHExit)
                    Main.FlashRQ = 0;
                else
                {
      
NEEDS_FLASH_PROC:   Main.FlashRQ = 1;
                    if (!Main.getCMD) // only if it is not a command mode
                    {
                        if (RetrUnit==0) // can be write to com -> data can be sent
                        {
                            // now it is possible to process data from flash
                        }
                    } 
                }
            }
            else
                goto NEEDS_FLASH_PROC;
               
                
#endif
#ifndef NO_I2C_PROC
            if (AInI2CQu.iQueueSize == 0)
#endif
            {
                //if (I2C_B1.I2CMasterDone) // no communication over I2C
#ifdef __PIC24H__
                CLKDIVbits.DOZEN = 1; // switch clock from 40MOP=>1.25MOP
#else
#endif
            }
        }
   }
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// end COPY 7
///////////////////////////////////////////////////////////////////////