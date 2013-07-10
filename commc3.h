/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
// begin COPY 3
/////////////////////////////////////////////////////////////////////
    //if (!Main.getCMD) // outside of the include was if == unit in "stream" relay mode
    //{
#ifdef NEW_CMD_PROC
#else
        // getCMD == 0
        // in stream was ESC char and now needs to echo that char to loop
        if (Main.ESCNextByte)
            Main.ESCNextByte = 0;
        else
        {  
            // if this is addressed to this unit then process it and switch "stream" -> "command" mode
            if (bByte == UnitADR)
            {
                Main.getCMD = 1; //next will be: <CMD>
                Main.SetFromAddr = 0;
                Main.SetSendCMD = 0;
                I2C.ESCI2CChar = 0;
                I2C.LastWasUnitAddr = 1;
                return;
            }
            else if (bByte == ESC_SYMB)   // ESC char - needs to echo next simbol to loop
                Main.ESCNextByte = 1;
        }
        // relay char to the loop, bcs now it is "stream" mode      
        putch(bByte); //ok
#endif
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
                    I2C.LastWasUnitAddr = 0;
RETRANSMIT:                    
                    putch(bByte);
                    return;
                }
                else if (bByte == UnitADR)
                {
                    if (I2C.LastWasUnitAddr)  // pakets can not travel with 0 length - it is definetly was a lost packet and
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
                I2C.LastWasUnitAddr = 0;
                if (!Main.PrepI2C)
                    Main.ESCNextByte = 1;
                else
                    I2C.ESCI2CChar = 1;
             
                return;
            }
            else if (bByte == UnitADR)
            {
                if (I2C.LastWasUnitAddr)  // pakets can not travel with 0 length - it is definetly was a lost packet and
                    return;           // needs to continue CMD mode  

                Main.getCMD = 0; // CMD stream done 
#ifndef NO_I2C_PROC
                if (Main.PrepI2C) // execute I2C if CMD stream done 
                {
                    bByte = '@';
                    goto END_I2C_MSG_WAIT;
                }
#endif
            }
            I2C.LastWasUnitAddr = 0;
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
#ifdef USE_OLD_CMD_EQ
            if (I2C.NextI2CRead) // something expected ?
            {
                if (I2C.RetransI2CCom)
                {
                    InsertI2C('=');
                    InsertI2C(LenI2CRead | 0x80);
                }
            }
#endif
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
        if (DataB3.FlashCmd)
        {
            if (DataB3.FlashCmdLen) // store length of a flash command
            {
                DataB3.FlashCmdLen = 0;
                CountWrite = bByte;
                DataB3.FlashRead = 0;
                CS_LOW;
            }
            else
            {
                if (DataB3.FlashRead)
                {
                    DataB3.FlashRead = 0;
                    //if (!Main.ComNotI2C)
                    //{
                    //    //do 
                    //    //{
                    //    //    InsertI2C(GetSSByte()); // read byte from FLASh will goes to I2C < 10 bytes
                    //    //} while(--bByte);
                    //    //InsertI2C('@');
                    //}
                    //else
                    {
                        Main.SendWithEsc = 1;
                        do 
                        {
                            putchWithESC(GetSSByte()); // read byte from FLASh will goes to Com
                                                       // if size bigger then 13 bytes it can be delay (putchWithESC waits out queue avalable space)
                        } while(--bByte);
                        Main.SendWithEsc = 0;
                        if (UnitFrom)
                            putch(UnitFrom);
                    }
                    goto DONE_WITH_FLASH;
                }
                else if (CountWrite == 1) // this will be last byte to write or it can be symb=@ request to read
                {
                    if (bByte == '@') // without CS_HIGH will be next read
                    {
                        DataB3.FlashRead = 1;
                        //if (!Main.ComNotI2C) // CMD comes from I2C - reply from read should goes back to I2C
                        //{
                        //    //InsertI2C('<');
                        //    //InsertI2C(UnitFrom);
                        //    //if (SendCMD)
                        //    //    InsertI2C(SendCMD);
                        //}
                        //else     // CMD comes from Com == relay (read) must go back to comm
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
                SendSSByte(bByte);
                //SendSSByteFAST(bByte); //for testing only
                if (--CountWrite)
                    return;
DONE_WITH_FLASH:
                DataB3.FlashCmd = 0;
                CS_HIGH;
                if (!Main.ComNotI2C) // CMD comes from I2C - reply from read should goes back to I2C
                {
                     // initiate send using I2C
                     //InitI2cMaster();
                }
                else
                {
                    //if (UnitFrom)
                    //    putch(UnitFrom);
                }
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
