//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// ProcCMD.H//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
void ProcessCMD(unsigned char bByte)
{
    #ifdef __PIC24H__
    char * ptrlFrsCopy; // attention C30 compiled do not understand pointers anything but char* !!!! needs to make a cast!!!!
    char * ptrlCorrection;
    #else
    unsigned char bFrsCopyL;
    unsigned char bFrsCopyH;
    unsigned char bFrsCopyHH;
    #endif
    unsigned char bCarry;
    unsigned char bWork;

    
    unsigned char bWorkL;
    unsigned char bWorkH;
    unsigned char bWS;
    
//////////////////////////////////////////////////////////////////////
// done area with no difference as a command or stream distingvishing
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
    if (!Main.getCMD)
    {
/////////////////////////////////////////////////////////////////////
// this is a STREAM area of processing
// yes, yes, it is ugly == but processor is PIC, and stack for calls can be limited
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#include "commc3.h"
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
// begin COPY 3
/////////////////////////////////////////////////////////////////////
    //if (!Main.getCMD) // outside of the include was if == unit in "stream" relay mode
    //{
        // getCMD == 0
        // in stream was ESC char and now needs to echo that char to loop
        if (Main.ESCNextByte)
            Main.ESCNextByte = 0;
        else
        {  
            // if this is addressed to this unit then process it and switch "stream" -> "command" mode
            if (bByte == MY_UNIT)
            {
                Main.getCMD = 1; //next will be: <CMD>
                Main.SetFromAddr = 0;
                Main.SetSendCMD = 0;
                I2C.ESCI2CChar = 0;
                Main.LastWasUnitAddr = 1;
                Main.LockToQueue =0; //that will allow to switch back ability to retansmit
                return;
            }
            else if (bByte == ESC_SYMB)   // ESC char - needs to echo next simbol to loop
                Main.ESCNextByte = 1;
        }
        // relay char to the loop, bcs now it is "stream" mode      
        putch(bByte); //ok
SKIP_ECHO_BYTE: ;
    }
    else    // now unit in command mode == processing all data
    {

        if (Main.RetransmitTo) // command =X* was entered - all packet till end was retransmitted to different unit
        {
            if (Main.ESCNextByte)
            {
                Main.ESCNextByte = 0;
                goto RETRANSMIT;
            }
            else
            {
                if (bByte == ESC_SYMB)
                {
                    Main.LastWasUnitAddr = 0;
RETRANSMIT:                    
                    putch(bByte);
                    return;
                }
                else if (bByte == MY_UNIT)
                {
                    if (Main.LastWasUnitAddr)  // pakets can not travel with 0 length - it is definetly was a lost packet and
                        goto RETRANSMIT;

                     Main.RetransmitTo = 0;
                     bByte = UnitFrom;
                     goto RETRANSMIT;
                }
                goto RETRANSMIT;
            }
        }
 
        // getCMD == 1 
        // stream addressing this unit
        if (Main.ESCNextByte)
            Main.ESCNextByte = 0;
        else
        {
            if (bByte == ESC_SYMB)
            {
                Main.LastWasUnitAddr = 0;
#ifndef NO_I2C_PROC

                if (!Main.PrepI2C)
                    Main.ESCNextByte = 1;
                else
                    I2C.ESCI2CChar = 1;
#else
                Main.ESCNextByte = 1;
#endif
                return;
            }
            else if (bByte == MY_UNIT)
            {
                if (Main.LastWasUnitAddr)  // pakets can not travel with 0 length - it is definetly was a lost packet and
                    return;           // needs to continue CMD mode  

                Main.getCMD = 0; // CMD stream done 
#ifndef NO_I2C_PROC
                if (Main.PrepI2C) // execute I2C if CMD stream done 
                {
                    bByte = '@';
                    goto END_I2C_MSG_WAIT;
                }
#endif
                return;
            }
            Main.LastWasUnitAddr = 0;
        }
#ifndef NO_I2C_PROC
//////////////////////////////////////////////////////////////////////////////////
//  I2C command processing:
//     "<"<I2CAddr><DATA>@ or "<"<I2C addr><data><unit> 
//     "<"<I2Caddr><data>">"L@   or "<"<I2Caddr><data>">"L<unit> 
//         where L is a length data to read
//     ">"<I2C addr>L@  or ">"<I2C addr>L<unit> 
//         where L is a length bytes to read
//////////////////////////////////////////////////////////////////////////////////
        if (Main.PrepI2C) // stream addressing I2C 
        {
I2C_PROCESS:
            if (I2C.WaitQuToEmp)      // out queue was to be emptied before any next operation with another I2C
            {
                if (I2Caddr == 0xff)  
                {
                    I2Caddr = bByte;  // first after '<' is address 
                    return;
                }
WAIT_QU_EMP:
                if (AOutI2CQu.iQueueSize) // wait untill ouput I2C queue will be empty to start communication to I2C
                    goto WAIT_QU_EMP;
                //if (TXIE)
                //    goto WAIT_QU_EMP;
                I2C.WaitQuToEmp = 0;
                //BlockComm = 1;
            }
            if (I2C.NextI2CRead)
            {
                LenI2CRead = bByte;
                Main.PrepI2C = 0;
                goto END_I2C_MSG_WAIT;
            }
            if (I2C.ESCI2CChar)
            {
                I2C.ESCI2CChar = 0;
                goto PUT_CHAR;
            }
            if (bByte == '@') // this is end of the message
            {
                Main.PrepI2C = 0;
                goto END_I2C_MSG_WAIT;
            }
            if (bByte == '>') // this is end of the message and start read from same I2C device
            {
                I2C.NextI2CRead = 1;
                return;
            }
PUT_CHAR:

            putchI2C(bByte);
            if (AOutI2CQu.iQueueSize < 14) // packet can be long
                return;
END_I2C_MSG_WAIT:              // TBD this loop has to have limitation - bus can be dead

#ifdef I2C_INT_SUPPORT ////////////////////////////////////////////////////////
            InitI2cMaster();
WAIT_I2C_DONE:
            if (!I2C_B1.I2CMasterDone)  // needs to wait
                goto WAIT_I2C_DONE;
            if (bByte == '@')
                Main.PrepI2C = 0;
#else // not I2C_INT_SUPPORT /////////////////////////////////////////////////
            if (AOutI2CQu.iQueueSize) // this is a case when something in a queue and needs to send it
            {
                //if (I2C_B1.I2CBusBusy) // needs to wait when I2C will be not busy
                //    goto END_I2C_MSG_WAIT;
WAIT_I2C_START:
                if (InitI2cMaster()) // TBD I2C line busy by somebody else what to do?
                    goto WAIT_I2C_START;
                sendI2C();
                //if (sendI2C())  // if return not Zero == error in send everything else has to be skipped
                //    goto DONE_DONE_I2C;
                // out qu must be cleaned (in sendI2C)
                //AOutI2CQu.iQueueSize = 0;
                //AOutI2CQu.iEntry = 0;
                //AOutI2CQu.iExit = 0;
         
                if (bByte == '@')
                {
DONE_I2C:                
                    //I2Caddr = 0xff;
                    Main.PrepI2C = 0;
                    //BlockComm = 0;
                }
                ReleseI2cMaster();
            }
            if (I2C.NextI2CRead)
            {
                I2C_B1.I2Cread = 1;
I2C_WAIT_READ:              // TBD this loop has to have limitation - bus can be dead
                if (InitI2cMaster()) // if in restart somebody uses line then collision
                    goto DONE_DONE_I2C;
                sendI2C();
                //if (sendI2C()) // send address only - TBD needs to check how was ACK on address
                //    goto DONE_DONE_I2C;
                receiveI2C();
DONE_DONE_I2C:
                I2C.NextI2CRead = 0;
                goto DONE_I2C;
            }

#endif // not I2C_INT_SUPPORT/////////////////////////////////////////////////////////////
            Main.DoneWithCMD = 1; // long command ends
            return;
        }  // end if a adressing I2C stream
#endif
//////////////////////////////////////////////////////////////////////////////
// FLASH command processing
// set by external comman like F
//        F<length-of-packet><CMD><data>
//            send and receive responce from FLASH
//        F<length-of-packet><CMD><data>@<length-to-read>
//            in last case <length-of-packet> must include simbol '@'
//////////////////////////////////////////////////////////////////////////////
#ifdef SSPORT
        if (DataB3.FlashCmdShort)
        {
            if (DataB3.FlashCmdLen) // store length of a flash command
            {
                DataB3.FlashCmdLen = 0;
                CountWrite = bByte;
                DataB3.FlashRead = 0;
                FlashCmdPos = 1;
                OldFlashCmd = 0x06;
                CurFlashCmd = 0x02;
                DataB3.FlashCmdShort = 0;
                DataB3.FlashCmd = 1;
            }
        }
        else if (DataB3.FlashCmd)
        {
            if (DataB3.FlashCmdLen) // store length of a flash command
            {
                DataB3.FlashCmdLen = 0;
                CountWrite = bByte;
                DataB3.FlashRead = 0;
                FlashCmdPos = 0;
                //CS_LOW;
            }
            else
            {
                if (DataB3.FlashRead)
                {
                    goto SEND_AGAIN;
CONTINUE_READ: 
                    Main.SendWithEsc = 1;
                    do 
                    {
                        putchWithESC(GetSSByte()); // read byte from FLASh will goes to Com
                                                   // if size bigger then 13 bytes it can be delay (putchWithESC waits out queue avalable space)
                        putch_main(); // that will initiate send 
                    } while(--bByte);
                    DataB3.FlashRead = 0;
                    Main.SendWithEsc = 0;
                    if (UnitFrom)
                        putch(UnitFrom);
                    goto DONE_WITH_FLASH;
                }
                else if (CountWrite == 1) // this will be last byte to write or it can be symb=@ request to read
                {
                    if (OldFlashCmd == 0)  // no prev command "write enable" == read command
                    {
                        if (bByte == '@') // without CS_HIGH will be next read
                        {
                            DataB3.FlashRead = 1;
                            {
                                if (UnitFrom)
                                {
                                    putch(UnitFrom);
                                    if (SendCMD)
                                        putch(SendCMD);
                                }
                            }
                            return;
                        }
                    }
                }
                if (FlashCmdPos == 0)
                {
                   if (CountWrite== 1)
                   {
                       if (OldFlashCmd!=0) // two 1 byte commands in a row
                       {
                            CS_LOW;
                            SendSSByte(OldFlashCmd);
                            CS_HIGH;
                            nop();nop();
WRITE_FLASH_CMD:
                            CS_LOW;
                            SendSSByte(bByte);
                            CS_HIGH;
                            OldFlashCmd = 0;
                            DataB3.FlashCmd = 0;
                            Main.DoneWithCMD = 1; // long command flash manipulation done
                            CountWrite = 0;
                            return;
                       }
                       if (bByte != 0x06) // only command 06 allow to be in stack
                           goto WRITE_FLASH_CMD;
                       OldFlashCmd = bByte;
                       DataB3.FlashWas1byteWrite = 1;
                   }
                   else
                   {
                       CurFlashCmd = bByte;
                       if (!DataB3.FlashWas1byteWrite)
                       {
                           OldFlashCmd = 0;
                       }
                       DataB3.FlashWas1byteWrite = 0;
                   }
                } 
                else if (FlashCmdPos == 1)
                    AdrFlash1 = bByte;
                else if (FlashCmdPos == 2)
                    AdrFlash2 = bByte;
                else if (FlashCmdPos == 3)
                    AdrFlash3 = bByte;
                else
                {
SEND_AGAIN:         // that is equivalent of a function -- just on some PIC it is not enought stack
                    if (btest(SSPORT,SSCS)) // another FLASH write can pause FLASH write intiated by serial 
                    {
                        if (OldFlashCmd != 0)
                        {
                            CS_LOW;
                            SendSSByte(OldFlashCmd);
                            CS_HIGH;
                            nop();nop();
                        }
                        CS_LOW;
                        SendSSByte(CurFlashCmd);
                        SendSSByte(AdrFlash1);
                        SendSSByte(AdrFlash2);
                        SendSSByte(AdrFlash3);
                    }
                    if (!DataB3.FlashRead)
                        SendSSByte(bByte);
                    if (++AdrFlash3 ==0)
                    {
                        if (++AdrFlash2 ==0)
                            ++AdrFlash1;
                        CS_HIGH;          // that is extention == FLASH can not read/write over same page
                        goto SEND_AGAIN;
                    }
                    if (DataB3.FlashRead)
                        goto CONTINUE_READ;
                }
                FlashCmdPos++;
               
                //SendSSByte(bByte);
                //SendSSByteFAST(bByte); //for testing only
                if (--CountWrite)
                    return;
DONE_WITH_FLASH:
                OldFlashCmd = 0;
                CurFlashCmd = 0;
                DataB3.FlashCmd = 0;
                CS_HIGH;
                Main.DoneWithCMD = 1; // long command flash manipulation done 
            }
            return;
        }
#endif // SSPORT
/////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// end COPY 3
////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// additional code proceessed as Cmd :
/////////////////////////////////////////////////////////////////////
    if (ATCMDStatusAddon)
    {
        ATCMDStatusAddon++; 
        if (ATCMDStatusAddon == 2) // AT
        {
            if (bByte != 'T')
            {
DONE_ADDON_CMD:
                ATCMDStatusAddon = 0;
                Main.DoneWithCMD = 1; // long command done
            }
            return;
        }
        if (bByte == 0x0a)
            goto DONE_ADDON_CMD;
        if (ATCMDStatusAddon == 3) // ATD
        {
            if (bByte == 'D')      // ATD - repeat send DIAL from Earth to Luna
            {
                 if (ATCMD & MODE_CONNECT)
                 {
                     Main.PingRQ = 1;
                 }
                 goto DONE_ADDON_CMD;
            }
            if (bByte == 'P')      // ATP - repeat 10 times send PING from Earth to Luna
            {
                Main.DoPing =1;
                PingAttempts = 20;
                Main.ConstantPing = 0;
            }
            if (bByte == 'C')      // ATC - constant PING from Earth to Luna
            {
                Main.DoPing =1;
                PingAttempts = 2;
                Main.ConstantPing = 1;
            }
            if (bByte == 'X')      // ATX - stop pinging
            {
                Main.DoPing =0;
                PingAttempts = 2;
                Main.ConstantPing = 0;
            }
        }
    }
    if (ATCMDStatus)
    {
         ATCMDStatus++;
         if (ATCMDStatus == 2) // At
         {
             if (bByte == 't') // at command starts == othervise commane done
                 return;
             else
             {
                  ATCMDStatus = 0;
                  Main.DoneWithCMD = 1; // long command done
                  goto CONTINUE_NOT_AT;
             }
         }
         else if (ATCMDStatus == 3) // ATd // ATs // ATh
         {
             if (bByte == 'd') // atd command
                 ATCMDStatus = 4;        // on next entry will be 5
             else if (bByte == 's') // ats command
                 ATCMDStatus = 9;        // on next entry will be 10
             else if (bByte == 'h') // ath command
             { 
                 ATCMDStatus = 3;        // on next entry will be 4
                 InitModem();
             }
             else 
                ATCMDStatus = 3;         // on next entry will be 4
             return;
         }
         else if (ATCMDStatus == 4) // some AT command == skip till 0x0a
         {
              if (bByte == 0x0a) // CR == AT command done
              {
                  ATCMDStatus = 0;
                  Main.DoneWithCMD = 1; // long command done
              }
              else
                  ATCMDStatus = 3; // on next entry will be 4
              return;
         }
         if (bByte == 0x0a) // CR == AT command done
         {
             ATCMDStatus = 0;
             Main.DoneWithCMD = 1; // long command done
             //if (ATCMD)
             //{
             //    ATCMD |= INIT_BT_NOT_DONE;  // execute mode
             //    BTFlags.BTFirstInit = 0;
             //}
             return;
         }
         if (ATCMDStatus == 5) // atdt
         {
             if (bByte != 't')
                ATCMDStatus = 3;
             return;
         }
         else if (ATCMDStatus == 6) // atdt
         {
              // ATDTl - connect to CubeSat and use Comm for tranfering data
              // when connection established it relpy with "CONNECT" and com will be in relay state (no unit wrapping)
              ////////////////////////////////////////////////////////////////////////////
              // ATDTE - set module in listen mode on FQ1 and when connection esatblished it reply with "CONNECT" to a unit
              ////////////////////////////////////////////////////////////////////////////
             ATCMD = 0;
             if (bByte == 'e') // atdtEARTH
                 ATCMD = MODE_CALL_EARTH;//1;
             else if (bByte == 'l') // atdtluna
                 ATCMD = MODE_CALL_LUNA_COM;//4;
             ATCMD |= INIT_BT_NOT_DONE;  // execute mode
             BTFlags.BTFirstInit = 0;

             ATCMDStatus = 6; // on next entry will be 7
             //SetupBT();
             INT0_ENBL = 1;
             return;
         }
         else if (ATCMDStatus == 7) // skip till 0x0a
         {
              ATCMDStatus = 6; // on next entry will be 7
              return;
         }
         else if (ATCMDStatus == 10) // ats command: atsx= atsy= atsy=
         {
              ATCMD = 0;
              // from ATSX=LunBMF1F2F3 // set 
              // i.e. ATSX=Lun00020J3k
              //  Addr =   Lun
              //              0         = 0 = 9600 1 = 19200 3 = 38400 5 = 57600
              //               0       - mode = 0 == 250kb check summ on chip
              //                                1 == 250kb no check summ on chip check sum in software
              //                                2 == 250kb 3 channels FQ1->FQ2->FQ3 internal check summ on individual msg
              //                                3 == 128kb
              //                02     - channel 1 frequency == 02
              //                  0J   = channel 2 frequency = 0x0A = 10
              //                    3k = channel 3 frequency = 0x3B = 59
              ////////////////////////////////////////////////////////////////////////////
              // ATSZ=Z set module in listen state on FQ1 when first message receved it reply with "CONNECT" to a UNIT
              ////////////////////////////////////////////////////////////////////////////
              // ATSY=<pkt><len><data> set into a FLASH memory <pkt> with a <data>
              ////////////////////////////////////////////////////////////////////////////

              if (bByte == 'z') // ATSZ=Z set module in listen state on FQ1 when first message receved it reply with "CONNECT" to a UNIT
              {
                 ATCMD = MODE_CALL_LUNA_COM;//4;
                 ATCMDStatus = 6; // on next entry will be 7

              }
              if (bByte == 'x') // from ATSX=LunBMF1F2F3 // set 
                 ATCMDStatus = 10;      // on next entry will be 11
              if (bByte == 'y') // ATSY=<pkt><len><data> set into a FLASH memory <pkt> with a <data>
                 ATCMDStatus = 23;  // on next entry will be 24
              return;
         }
         else if (ATCMDStatus == 11) // ATSX=LunBMF1F2F3
              return;                //     A
         else if (ATCMDStatus == 12) // ATSX=LunBMF1F2F3
         {                           //      A
                 Addr1 = bByte;
                 if (Addr1 == '.')
                     Addr1 = 0xaa;    
                 eeprom_write(0x34, Addr1);
                 return;
          }
          else if (ATCMDStatus == 13) // ATSX=LunBMF1F2F3
          {                           //        A
                 Addr2 = bByte;
                 if (Addr2 == '.')
                     Addr2 = 0xaa;    
                 eeprom_write(0x35, Addr2);
                 return;
          }
          else if (ATCMDStatus == 14) // ATSX=LunBMF1F2F3
          {                           //         A
                 Addr3 = bByte;
                 if (Addr3 == '.')
                     Addr3 = 0xaa;    
                 eeprom_write(0x36, Addr3);
                 return;
          }
          else if (ATCMDStatus == 15) // ATSX=LunBMF1F2F3
          {                           //         A
                Config01 = bByte;
//             ' '(space)  1 frequency mode on RX and 1 frequency to TX
//              a          3 frequency mode on RX and 1 frequency to TX
//              b          1 frequency mode on RX and 3 frequency to TX
//              c          3 frequency mode on RX and 3 frequency to TX
//              e          3 frequency mode on RX and 1 frequency to TX, RX allow to switch from 1->3->1 frequency
//              g          3 frquency mode on RX and 3 frequency to TX, RX allow to switch from 1->3->1 frequency
                eeprom_write(0x30, Config01);
                return;
          }
          else if (ATCMDStatus == 16) // ATSX=LunBMF1F2F3
          {                           //          A
                //Config01 |= bByte<<2;
                return;
          }
          else if (ATCMDStatus == 17) // ATSX=LunBMF1F2F3
          {                           //           A
                Freq1 = bByte<<4;
                 return;
          }
          else if (ATCMDStatus == 18) // ATSX=LunBMF1F2F3
          {                           //            A
                Freq1 |= bByte & 0x0f;
                //BTFQcurr = Freq1;
                FqTX = Freq1;
                FqTXCount = 0;
                eeprom_write(0x31, Freq1); // slow operation == 4 simbols at one write
                return;
          }
          else if (ATCMDStatus == 19) // ATSX=LunBMF1F2F3
          {                           //             A
                Freq2 = bByte<<4;
                return;
          }
          else if (ATCMDStatus == 20) // ATSX=LunBMF1F2F3
          {                           //              A
                Freq2 |= bByte & 0x0f;
                eeprom_write(0x32, Freq2); // slow operation == 4 simbols at one write
                return;
          }
          else if (ATCMDStatus == 21) // ATSX=LunBMF1F2F3
          {                           //               A
                Freq3 = bByte<<4;
                return;
          }
          else if (ATCMDStatus == 22) // ATSX=LunBMF1F2F3
          {                           //                A
                Freq3 |= bByte & 0x0f;
                eeprom_write(0x33, Freq3); // slow operation == 4 simbols at one write
                ATCMDStatus = 3; // on next entry will be 4
                //SetupBT();
                //INT0_ENBL = 1;
                return;
          }
          //else if (ATCMDStatus == 24) 
          //{
          //}
          // something wrong == done with AT
          ATCMDStatus = 0;
          Main.DoneWithCMD = 1; // long command done
          return;
    }
CONTINUE_NOT_AT:
    if ((DataB0.ExcgFlashcmd) || (DataB0.ExcgRcvCmd))              // SEND to another connected unit data from FLASH memory
    {                                     // S<XXXXXX>=<llll>
        ExchCmdStatus++;                  // address=XXXXXX length=llll
        if (ExchCmdStatus == 1)
            ExcgArd1 = bByte;
        else if (ExchCmdStatus == 2)
            ExcgArd2 = bByte;
        else if (ExchCmdStatus == 3)
            ExcgArd3 = bByte;
        else if (ExchCmdStatus == 4)
            ;
        else if (ExchCmdStatus == 5)
            ExcgLen = ((UWORD)bByte)<<8;
        else if (ExchCmdStatus == 6)
        {
            ExcgLen += bByte;
            Main.DoneWithCMD = 1;
            if (DataB0.ExcgFlashcmd)
            {
                DataB0.ExcgFlashcmd = 0;
                ExchSendStatus =1;
            }
            else
            {
                DataB0.ExcgRcvCmd = 0;
            }
        }
    }
    if (DataB0.UpgradeProgFlags)
    {
        // CMD upgrade PIC from storage
        //<U><XXXXXX>=<xxxxxx>=<llll>
        // load from XXXXXX (FLASH) to xxxxxx (prog memory) llll bytes and restart processor
        UpgradeProgStatus++;
        if (UpgradeProgStatus == 1) //<U><XXxxxx>=<xxxxxx>=<llll>
            AdrBH= bByte;
        else if (UpgradeProgStatus == 2)//<U><xxXXxx>=<xxxxxx>=<llll>
            wAddr = ((UWORD)bByte)<<8;
        else if (UpgradeProgStatus == 3) //<U><xxxxXX>=<xxxxxx>=<llll>
            wAddr += bByte;
        else if (UpgradeProgStatus == 4) //<U><......>=<......>.<....> 
            ;        
        else if (UpgradeProgStatus == 5) //<U><xxxxxx>=<XXxxxx>=<llll> 
            ProgAdrBH = bByte;
        else if (UpgradeProgStatus == 6) //<U><xxxxxx>=<xxXXxx>=<llll> 
            wProgAddr =  ((UWORD)bByte)<<8;
        else if (UpgradeProgStatus == 7) //<U><xxxxxx>=<xxxxXX>=<llll> 
            wProgAddr += bByte;

        else if (UpgradeProgStatus == 8) //<U><......>.<......>=<....> 
            ;
        else if (UpgradeProgStatus == 9) //<U><xxxxxx>=<xxxxxx>=<LLll> 
            wProgMaxAddr = ((UWORD)bByte)<<8;        
        else if (UpgradeProgStatus == 10) //<U><xxxxxx>=<xxxxxx>=<llLL> 
        {
            wProgMaxAddr += bByte;
            Main.DoneWithCMD = 1; // long command done
            DataB0.UpgradeProgFlags = 0;
            // reset after upgrade
            PrgUnit(ProgAdrBH, wProgAddr, wProgMaxAddr);
        }
        return;
    }
    if (DataB0.CRCCalcFals)
    {
        // CRC flash <C><XXXXXX>=<xxxxxx>=<CRC1>=<CRC2>=<CRC3>=<CRC4>
        CRCFlashCalcStatus++;
        if (CRCFlashCalcStatus ==1)      // <C><XXxxxx>=<xxxxxx>=<CRC1>=<CRC2>=<CRC3>=<CRC4>
            AdrBH= bByte;
        else if (CRCFlashCalcStatus == 2)// <C><xxXXxx>=<xxxxxx>=<CRC1>=<CRC2>=<CRC3>=<CRC4>
            wAddr = ((UWORD)bByte)<<8;
        else if (CRCFlashCalcStatus == 3)// <C><xxxxXX>=<xxxxxx>=<CRC1>=<CRC2>=<CRC3>=<CRC4>
            wAddr += bByte;
        else if (CRCFlashCalcStatus == 4)// <C><xxxxxx>=<xxxxxx>=<CRC1>=<CRC2>=<CRC3>=<CRC4> 
            ;        
        else if (CRCFlashCalcStatus == 5)// <C><xxxxxx>=<XXxxxxx>=<CRC1>=<CRC2>=<CRC3>=<CRC4>
            AdrBH2= bByte;
        else if (CRCFlashCalcStatus == 6)// <C><xxxxxx>=<XXxxxx>=<CRC1>=<CRC2>=<CRC3>=<CRC4>
            wAddr2 = ((UWORD)bByte)<<8;
        else if (CRCFlashCalcStatus == 7)// <C><xxxxxx>=<xxXXxx>=<CRC1>=<CRC2>=<CRC3>=<CRC4>
            wAddr2 += bByte;
        else if (CRCFlashCalcStatus == 8)// <C><xxxxxx>=<xxxxXX>=<CRC1>=<CRC2>=<CRC3>=<CRC4> 
            ;    
        else if (CRCFlashCalcStatus == 9)// <C><xxxxxx>=<XXxxxx>=<CRc1>=<crc2>=<crc3>=<crc4>
            CRC1Cmp = ((UWORD)bByte)<<8;
        else if (CRCFlashCalcStatus == 10)// <C><xxxxxx>=<XXxxxx>=<CRc1>=<crc2>=<crc3>=<crc4>
            CRC1Cmp += bByte;
        else if (CRCFlashCalcStatus == 11)// <C><xxxxxx>=<XXxxxx>=<CRc1>=<crc2>=<crc3>=<crc4>
//            ;    
//        else if (CRCFlashCalcStatus == 12)// <C><xxxxxx>=<XXxxxx>=<CRc1>=<crc2>=<crc3>=<crc4>
///            CRC2Cmp = ((UWORD)bByte)<<8;
//        else if (CRCFlashCalcStatus == 13)// <C><xxxxxx>=<XXxxxx>=<CRc1>=<crc2>=<crc3>=<crc4>
//            CRC2Cmp += bByte;
//        else if (CRCFlashCalcStatus == 14)// <C><xxxxxx>=<XXxxxx>=<CRc1>=<crc2>=<crc3>=<crc4> 
//            ;    9
//        else if (CRCFlashCalcStatus == 15)// <C><xxxxxx>=<XXxxxx>=<CRc1>=<crc2>=<crc3>=<crc4>
//            CRC3Cmp = ((UWORD)bByte)<<8;        
//        else if (CRCFlashCalcStatus == 16)// <C><xxxxxx>=<XXxxxx>=<CRc1>=<crc2>=<crc3>=<crc4>
//            CRC3Cmp += bByte;
//        else if (CRCFlashCalcStatus == 17)// <C><xxxxxx>=<XXxxxx>=<CRc1>=<crc2>=<crc3>=<crc4> 
//            ;    
//        else if (CRCFlashCalcStatus == 18)// <C><xxxxxx>=<XXxxxx>=<CRc1>=<crc2>=<crc3>=<crc4>
//            CRC4Cmp = ((UWORD)bByte)<<8;
//        else if (CRCFlashCalcStatus == 19)// <C><xxxxxx>=<XXxxxx>=<CRc1>=<crc2>=<crc3>=<crc4>
        {
//            CRC4Cmp += bByte;

            DataB0.CRCCalcFals = 0;
            CRCFlashCalcStatus = 0;
            // do calculation of CRC - (a)calculates dif AdrBH2|wAddr2 - AdrBH|wAddr
            wDiff = Sub3(AdrBH2, wAddr2, AdrBH, wAddr);
            DiffBH = btempres;
            // Dif /4 == >>2
            wDiff>>=2;
            wtemp = ((UWORD)DiffBH)<<14;
            DiffBH >>=2;
            wDiff |= wtemp;
            // add 
            wAddr2 = Add3(AdrBH, wAddr, DiffBH, wDiff);
            AdrBH2 = btempres;
            CRC1 = CRCCacl();

 //           AdrBH = AdrBH2;
 //           wAddr = wAddr2;
 //           wAddr2 = Add3(AdrBH, wAddr, DiffBH, wDiff);
 //           AdrBH2 = btempres;
 //           CRC2 = CRCCacl();

 //           AdrBH = AdrBH2;
 //           wAddr = wAddr2;
 //           wAddr2 = Add3(AdrBH, wAddr, DiffBH, wDiff);
 //           AdrBH2 = btempres;
 //           CRC3 = CRCCacl();

 //           AdrBH = AdrBH2;
 //           wAddr = wAddr2;
 //           wAddr2 = Add3(AdrBH, wAddr, DiffBH, wDiff);
 //           AdrBH2 = btempres;
 //           CRC4 = CRCCacl();
            if (CRC1 == CRC1Cmp)
                putch('O');
 //           else
 //           {
 //               putch('o');
 //           }
 //           if (CRC2 == CRC2Cmp)
 //               putch('O');
 //           else
 //           {
 //               putch('o');
 //           }
 //           if (CRC3 == CRC3Cmp)
 //               putch('O');
 //           else
 //           {
 //               putch('o');
 //           }
 //           if (CRC4 == CRC4Cmp)
 //               putch('O');
 //           else
 //           {
 //               putch('o');
 //           }
        }
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//#include "commc4.h"
////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// begin COPY 4
///////////////////////////////////////////////////////////////////////   

// standart commands:
//<unit>=Xci<unit> - set responce unit(X) and command (c)
//<unit>=X*<unit> and all data transfers to X till end of the packet
//<unit>=X?<unit>   - timer sync message
//<unit>~<unit>   - message from itself from other side of the loop    
        if (Main.SetFromAddr) //<unit>=Xc            //<unit>=Xci<unit> 
        {                     //       |         if ' ' than responce unit is not set
            if (bByte == ' ')
                bByte = 0;
            UnitFrom = bByte;
            Main.SetFromAddr = 0;
            Main.SetSendCMD = 1;

            return;
        }
        else if (Main.SetSendCMD) //<unit>=xC                 ///<unit>=xCi<unit> 
        {                         //        |        if ' ' than SendCMD is not set
            if (bByte == ' ')
                bByte = 0;
            SendCMD = bByte;
            Main.SetSendCMD = 0;
            I2C.SetI2CYesNo = 1;
            Main.DoneWithCMD = 1; // long command "=XC" done

            if (bByte == '*')  // "=X*" and all data transfers to X till end of the packet
            {
                 //if (UnitFrom) // assuming that unit was specified 
                 {
                     putch(UnitFrom);putch(UnitFrom); // twice to avoid lost bytes
                     Main.RetransmitTo = 1;
                     return;
                 }
            }
#ifdef SYNC_CLOCK_TIMER
            if (bByte == '?')
            {
#ifdef __PIC24H__
                Tmr4Count =TMR4;  // it is possible to count delays from interrupt to recorded time
                Tmr4CountH = TMR5HLD;
                TAfter.Timer = (((unsigned long)Tmr4CountH)<<16) | ((unsigned long)Tmr4Count);
                TAfter.Second = Tmr4CountOld;
                RtccReadTimeDate(&TAfter.Rtcc);
                putch(UnitFrom);putch(UnitFrom);
                FSR_REGISTER = &Tdelta;
                Main.SendWithEsc = 1;
                for (bWork = 0; bWork < 4*sizeof(Ttilad);bWork++)
                {
                    putchWithESC(PTR_FSR);FSR_REGISTER++;
                }
                putch(UnitFrom); // close packet
                Main.SendWithEsc = 0;
#else  // not __PIC24H__
#endif // __PIC24H__
                return;
            }
#endif // SYNC_CLOCK_TIMER

#ifdef RESPONCE_ON_EQ
			if (UnitFrom) // basically that is ACK
            {
            	putch(UnitFrom);
                if (SendCMD)
                	putch(SendCMD);
                putch('~');
                putch(UnitFrom);
            }
#endif
            return;
        }
        else if (bByte == '=') // new version   "=XC" where X - unit to responce and C - one byte command to responce 
                               // if command is "=X*" than all packet till end has to be send over com to device X with closing packet byte (X at the end)
                               
                               // old verion
                               // <unit>=XCI<unit> from unit = X, CMD to send =C (space = no CMD) I = expect retransmit over I2C
        {                      //  '=5CC' == to unit=5 with CMD=C over Type=C (Com) (operation SET)
                               //  '=5CI' == to unit=5 with CMD=C over Type=I (I2C) (opeartion SET) equivalent of <5C<DATA>@ 
                               //  '=*'   == to unit=5 with CMD=C over I2C == starting next byte all stream goes from com to I2C (retransmit)
                               //  '=*'   == to unit=5 with CMD=C over Com == starting next byte all stream goes from I2C to com (retransmit)
                               //  '=<NBIT+LEN>' (LEN < 128) next LEN bytes will goes to previously set device
                               //  high bit has to be set
            Main.DoneWithCMD = 0; // long command
            Main.SetFromAddr = 1;
        }
        // processing CMD
        else if (bByte == '~') // reseved test message from itself
        {
            Main.CommLoopOK = 1;
#ifdef SYNC_CLOCK_TIMER
#ifdef __PIC24H__
            memcpy(&Tdelta,&Ttilad,sizeof(Tdelta));
#else
#ifdef      _18F2321_18F25K20
#endif 
#endif // __PIC24H__
#endif // SYNC_CLOCK_TIMER
        }
#ifndef NO_I2C_PROC
        else if (bByte == '<') // "<"<I2CAddr><DATA>@ or "<"<I2C addr><data><unit> 
        {                      // "<"<I2Caddr><data>">"L@   or "<"<I2Caddr><data>">"L<unit> 
                               // where L is a length data to read
            Main.DoneWithCMD = 0; // long command
            I2Caddr = 0xff;
            Main.PrepI2C = 1;
            I2C_B1.I2Cread = 0;
            I2C.WaitQuToEmp = 1;
            I2C.NextI2CRead = 0;
       }
        else if (bByte == '>') // ><I2C addr>L@  or ><I2C addr>L<unit> where L is a length bytes to read
        {                      
            Main.DoneWithCMD = 0; // long command
            I2Caddr = 0xff;
            Main.PrepI2C = 1;
            I2C.WaitQuToEmp =  1;
            I2C.NextI2CRead = 1;
        }
#endif // NO_I2C_PROC
#ifdef SSPORT
        else if (bByte == 'f') // speed up cmd to write into FLASh memory
        {
            // instead of :
            // F\x01\x06F\x0c\x02\x00\x11\x22\x00\x00\x00\x00\x00\x00\x00\x00 == write 8 bytes to address 0x001122
            //              f\x0b\x00\x11\x22\x00\x00\x00\x00\x00\x00\x00\x00 == write 8 bytes to address 0x001122
            Main.DoneWithCMD = 0; // long command
            DataB3.FlashCmdShort = 1;
            DataB3.FlashCmdLen = 1;
        }
#endif
/////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// end COPY 4
////////////////////////////////////////////////////////////////////////


// additional code:
//
#ifdef NON_STANDART_MODEM
        else if (bByte == '$') // send everything to connected BT till end of the packet and ask to retransmit data to link
        {
            if (ATCMD & MODE_CONNECT) // was connection esatblished?
            {
                 Main.SendOverLink = 1;
                 Main.SendOverLinkAndProc = 1;
                 Main.SendOverlinkWasESC = 0;
            }
            // TBD: on no connection established - needs to do something with uplink data - it is going to nowhere
        }
        else if (bByte == '*') // send everything to connected BT till end of the packet and process inside BT 
        {
            if (ATCMD & MODE_CONNECT) // was connection esatblished?
            {
                 Main.SendOverLink = 1;
                 Main.SendOverlinkWasESC = 0;
            }
            // TBD: on no connection established - needs to do something with uplink data - it is going to nowhere
        }
#endif
        else if (bByte == 'a')  // begining of the command at...
        {
            ATCMDStatus = 1;
            Main.DoneWithCMD = 0; // long command
        }
        else if (bByte == 'A')  // additional command AT....
        {
            ATCMDStatusAddon = 1;
            Main.DoneWithCMD = 0; // long command
        }
        else if (bByte == 'U')  // CMD upgrade PIC from storage U<XXXXXX>=<xxxxxx>=<llll>// additional command
        {
            DataB0.UpgradeProgFlags =1;
            UpgradeProgStatus = 0;
            Main.DoneWithCMD = 0; // long command
        }
        else if (bByte == 'C')  // CRC flash <C><XXXXXX>=<xxxxxx>=<CRC1> 
        {                       // if CRC1 is OK respond "O"
                                // if CRCX is not OK respond c<XXXXXXXX>=<xxxxxx>=<CRC1>=<CRC2>
                                // on each 1/2 of the 1/2 
                                // i.e C008000=009000=1234
                                // CRC of 008000-008800 = 1234
                                // CRC    008800-009000 = 9abc
                                //   responce O in case of CRC are OK
                                //   responce o0101=2345=5432=9876 on each not matched CRC
            DataB0.CRCCalcFals = 1;
            CRCFlashCalcStatus = 0;
            Main.DoneWithCMD = 0; // long command
        }
        else if (bByte == 'c')  // CRC prog  <c><XXXXXX>=<xxxxxx>=<CRC1>=<CRC2>=<CRC3>=<CRC4>
        {
        }
        else if (bByte == 'S')             // SEND to another connected unit data from FLASH memory
        {                                  // S<XXXXXX>=<llll> 
            DataB0.ExcgFlashcmd = 1;       // address=XXXXXX length=llll
            ExchCmdStatus = 0;
            Main.DoneWithCMD = 0; // long command
        }
        else if (bByte == 's')             // prepear to recive from another connection unit data from FLASH memory
        {                                  // s<XXXXXX>=<llll> 
            DataB0.ExcgRcvCmd = 1;       // address=XXXXXX length=llll
            ExchCmdStatus = 0;
            Main.DoneWithCMD = 0; // long command
        }
        
SKIP_BYTE:;
    } // do not confuse: this is a else from getCMD == 1
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// end of ProcCMD.H //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
