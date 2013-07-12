////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// begin COPY 4
///////////////////////////////////////////////////////////////////////   

        if (Main.SetFromAddr) //<unit>=Xc            //<unit>=Xci<unit> 
        {                     //       |         if ' ' than responce unit is not set
            if (bByte == ' ')
                bByte = 0;
#ifdef USE_OLD_CMD_EQ
            if (bByte == '*') // this will switch stream permanently
            {
                return;
            }
            if (bittest(bByte,7))
            {
                 RetransmitLen = bByte&0x7f;
                 Main.SetFromAddr = 0;
                 Main.DoneWithCMD = 0;
                 return;
            }
#endif
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
#ifdef USE_OLD_CMD_EQ
        else if (I2C.SetI2CYesNo) //<unit>=xcI<unit> I= com->I2C C = I2C->com ' '=nothing 
        {                         //         |
            I2C.SetI2CYesNo = 0;
            if (bByte == 'i') // it is just for convinience I2CReplyExpected can be set in any CMD
            {
                I2C.I2CReplyExpected = 1;
            }
            else if (bByte == 'I')
            {
                I2C.RetransComI2C = 1;
                I2C.RetransComI2CSet = 0;
                I2C.RetransI2CCom = 0;
            }
            else if (bByte == 'C') //<unit>=xcC = I2C->com ' '=nothing
            {
                I2C.RetransI2CCom = 1;
                I2C.RetransI2CComSet = 0;
                I2C.RetransComI2C = 0;
            }
            else // clean all set for retransmit
            {
                I2C.RetransI2CCom = 0;
                I2C.RetransI2CComSet = 0;
                I2C.RetransComI2C = 0;
            }
            Main.DoneWithCMD = 1; // long command =XCI done
        }
#endif
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
#ifdef USE_OLD_CMD_EQ
            I2C.RetransComI2C = 0;
            I2C.RetransComI2CSet = 0;
            I2C.RetransI2CCom = 0;            
#endif
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
#ifdef NON_STANDART_MODEM
        else if (bByte == '*')
        {
            if (ATCMD & MODE_CONNECT) // was connection esatblished?
            {
                 Main.SendOverLink = 1;
            }
        }
#endif
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
        else if (bByte == 'F') // manipulation with FLASH memory: read/write/erase/any flash command
        {
            Main.DoneWithCMD = 0; // long command
            DataB3.FlashCmd = 1;
            DataB3.FlashCmdLen = 1;
            // send something to FLASH
            // F<length-of-packet><CMD><data>
            // send and receive responce from FLASH
            // F<length-of-packet><CMD><data>@<length-to-read>
            // in last case <length-of-packet> must include simbol '@'
            // F\x01\x06              == write enable (flash command 06) -> send 0x06
            // F\x01\0xc7             == erase all flash                 -> send =0xc7
            // F\x05\x03\x00\x12\x34@\x04 == read 4 bytes from a address 0x001234  -> send 0x03 0x00 0x12 0x34 <- read 4 bytes (must not to cross boundary)
            // F\x01\x06F\x0c\x02\x00\x11\x22\x00\x00\x00\x00\x00\x00\x00\x00 == write 8 bytes to address 0x001122
            // F\x01\x06F\x04\x20\x00\x04\x00 == erase sector (4K) starting from address 0x000400
        }
#endif
/////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// end COPY 4
////////////////////////////////////////////////////////////////////////
