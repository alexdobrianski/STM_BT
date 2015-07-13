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
        else if (bByte == 'F') // manipulation with FLASH memory: read/write/erase/any flash command
        {
            Main.DoneWithCMD = 0; // long command
            DataB3.FlashCmd = 1;  //flash state machine START
            DataB3.FlashCmdLen = 1;
            
            
            //NOTE, <data> is a generic data, it could be the data to write or address of data to read.
            // send something to FLASH
            // F<length-of-packet><CMD><data>
            // send and receive responce from FLASH
            // F<length-of-packet><CMD><data>@<length-to-read>
            // in last case <length-of-packet> must include simbol '@'

            ///////////////////////////
            //Things to Note
            ///////////////////////////
            // 1. The Erase instruction must be preceded by a Write Enable (WREN) instruction.
            //     To Bulk erase instr:
            //          2F&#0#1&#0#6F&#0#1&c#72 
            //     To read 16 bytes:
            //          2F&#0#5&#0#3&#0#0&#0#0&#0#0@&#1#02
            //     To write data:
            //          2F&#0#1&#0#6F&#3#5&#0#2&#0#0&#0#0&#0#0Nasha Masha Luchshe Vashei potomy chto ona nasha 2 

            ///////////////////////////
            // Flash CMDs
            ///////////////////////////
            // \x02                   == flash write cmd
            // \x03                   == read from flash
            // \x04                   == Write Disable 0000 0100 04h
            // \x06                   == enable write to flash
            // \x20                   == erase 4Kbyte oage - this cmd does not exist on all flash mem
            // \xc7                   == erase all mem from flash
            // \xb9                   == (NOTE, not all flash has this cmd) Deep Power-down 1011 1001 B9h
            // \xab                   == (NOTE, not all flash has this cmd) Release from Deep Power-down
            
            ///////////////////////////
            // Custom
            ///////////////////////////
            // @                      == Chain read CMD after previous CMDs. 
            //                             NOTE, include Byte count when specifying CMD length 
            //                             Post '@' indicate length of bytes to be read.
            
            // F\x01\x06              == enable write (flash command 06) -> send 0x06
            // F\x01\0xc7             == erase all flash                 -> send =0xc7
            // F\x05\x03\x00\x12\x34@\x04 == read 4 bytes from a address 0x001234  -> send 0x03 0x00 0x12 0x34 <- read 4 bytes (must not to cross boundary)
            // F\x01\x06F\x0c\x02\x00\x11\x22\x00\x00\x00\x00\x00\x00\x00\x00 == write 8 bytes to address 0x001122
            // F\x01\x06F\x04\x20\x00\x04\x00 == erase sector (4K) starting from address 0x000400
        }
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
