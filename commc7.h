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
        if (CallBkMain() == 0) // 0 = do continue; 1 = process queues
            continue;
        if (AInI2CQu.iQueueSize) // if something comes from I2C (slave received or some I2c device responded on read command)
        {
#ifdef USE_OLD_CMD_EQ
            if (RetransmitLen) // from I2C comes command =<LEN> - needs to retransmit everything to previously set device
            {                  //                          (set done by =5CC)
                if (!I2C.RetransComI2CSet)
                {
                    I2C.RetransComI2CSet = 1;
                    if (UnitFrom)
                    {
                        putch(UnitFrom);
                        if (SendCMD)
                            putch(SendCMD);
                    }
                }
REPEAT_OP1:                
                putchWithESC(getchI2C()); // if out queue does not has empty space putchWithESC will wait
                if (--RetransmitLen ==0)
                {
                    if (UnitFrom)
                        putch(UnitFrom);
                    Main.DoneWithCMD = 1; // long command done // this will unlock switching process from I2C to com
                    continue;
                }
                if (AInI2CQu.iQueueSize)
                    goto REPEAT_OP1;
                continue;
            }
#endif
#ifndef NO_I2C_PROC
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
#endif
        }
        if (AInQu.iQueueSize)      // in comm queue bytes
        {
            //if (RetransmitLen)
            //{
            //   // TBD
            //    continue;
            //}
            if (CallBkComm()) // 0 = do not process byte; 1 = process;
            {
#ifdef NEW_CMD_PROC
#else
                 // place where has to be checked realy message mode : if CallBkComm desided to process data then needs to monitor
                 // input message for not related to unit device
                 bWork = AInQu.Queue[AInQu.iExit]; // next char
                 if (Main.prepStream)
                 {
                     if (Main.prepSkip)
                         Main.prepSkip = 0;
                     else if (bWork == ESC_SYMB) // it is ESC char ??
                         Main.prepSkip = 1;
                     else if (bWork <= MAX_ADR)
                     {
                          if (bWork >= MIN_ADR) // msg to relay
                          {
#ifdef ALLOW_RELAY_TO_NEW
                              AllowOldMask4=AllowOldMask3;AllowOldMask3=AllowOldMask2;AllowOldMask2=AllowOldMask1;AllowOldMask1=AllowOldMask;AllowOldMask=AllowMask; // put mask into stack
                              AllowMask = 0;
                              if (bWork > MY_UNIT)
                              {
                                  iWork = bWork - (MY_UNIT+1);
                                  while(iWork)
                                  { 
                                      iWork--;
                                      AllowMask <<=1;AllowMask|=UNIT_MASK;        
                                  }
                              }
                              else if (bWork < MY_UNIT)
                              {
                                  iWork = (MY_UNIT-1);// - bWork;
                                  while(iWork)
                                  { 
                                      iWork--;
                                      AllowMask <<=1;AllowMask|=0x01;        
                                  }
                                  AllowMask |= UNIT_MASK_H;
                              }
                              else // unit matches
                              {
                                  Main.prepCmd = 1;
                                  Main.prepStream = 0;
                                  Main.prepZeroLen = 1;
                                  if (UnitFrom)              // if reply's unit was set
                                      AllowMask = UnitMask1; // then allow to send in CMD mode to exact unit
                                                             // otherwise unit can not initiate any transfer before endin processing CMD
                                  goto PROCESS_IN_CMD;
                              }
                              AllowMask &= AllowOldMask;
#else // NOT ALLOW_RELAY_TO_NEW
                              if (bWork == MY_UNIT) // unit matches
                              {
                                  Main.prepCmd = 1;
                                  Main.prepStream = 0;
                                  Main.prepZeroLen = 1;
                                  AllowMask = 0xff;
                                  goto PROCESS_IN_CMD;
                              }
                              AllowMask =  0;
#endif // end ALLOW_RELAY_TO_NEW
                              // now needs to stream everything exsisting from input comm queue to output comm queue
                              putch(bWork);
                              getch();
                              Main.prepZeroLen = 1;
                              RCIE = 0; // disable com1 interrupts
                              RetrUnit = bWork;
                              while(AInQu.iQueueSize)
                              {
                                  bWork = getch();
                                  putch(bWork);
                                  if (Monitor(bWork,RetrUnit)) // search for end of packet
                                  {
                                      RetrUnit = 0;
                                      break;
                                  }
                              }
                              RCIE = 1; // enable com1 interrupts
                              goto NO_PROCESS_IN_CMD; // give a chance to process char on a next loop
                          }
                     }
                 }
                 else // Main.prepCmd == monitor message processed inside ProcessCMD == search for end of packet
                 {
                     Monitor(bWork,MY_UNIT);
                 }
#endif // end NEW_CMD_PROC
PROCESS_IN_CMD:
                 ProcessCMD(getch());
NO_PROCESS_IN_CMD:;
            }
        }
        else  // nothing in both queue can sleep till interrupt
        {
            if (AInI2CQu.iQueueSize == 0)
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