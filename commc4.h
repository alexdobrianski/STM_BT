////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// begin COPY 4
///////////////////////////////////////////////////////////////////////   

        if (Main.SetFromAddr) //<unit>=Xci<unit> 
        {   
            if (bByte == ' ')
                bByte = 0;
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
            UnitFrom = bByte;
            Main.SetFromAddr = 0;
            Main.SetSendCMD = 1;
            return;
        }
        else if (Main.SetSendCMD) //<unit>=xCi<unit> 
        {
            if (bByte == ' ')
                bByte = 0;
            SendCMD = bByte;
            Main.SetSendCMD = 0;
            I2C.SetI2CYesNo = 1;
            return;
        }
        else if (I2C.SetI2CYesNo) //<unit>=xcI<unit> I= com->I2C C = I2C->com ' '=nothing 
        {
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
            else if (bByte == 'C')
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
        else if (bByte == '=') // <unit>=XCI<unit> from unit = X, CMD to send =C (space = no CMD) I = expect retransmit over I2C
        {                      //  =5CC == to unit=5 with CMD=C over Type=C (Com) (operation SET)
                               //  =5CI == to unit=5 with CMD=C over Type=I (I2C) (opeartion SET) equivalent of <5C<DATA>@ 
                               //  =* == to unit=5 with CMD=C over I2C == starting next byte all stream goes from com to I2C (retransmit)
                               //  =* == to unit=5 with CMD=C over Com == starting next byte all stream goes from I2C to com (retransmit)
                               //  =<NBIT+LEN> (LEN < 128) next LEN bytes will goes to previously set device
                               //  high bit has to be set
            Main.DoneWithCMD = 0; // long command
            Main.SetFromAddr = 1;
            I2C.RetransComI2C = 0;
            I2C.RetransComI2CSet = 0;
            I2C.RetransI2CCom = 0;
        }
        // processing CMD
        else if (bByte == '~') // reseved test message from itself
        {
            Main.CommLoopOK = 1;
        }
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
            // F\x01\x06              == write enable (flash command 06)
            // F\x05\x03\x00\x12\x34@\x04 == read 4 bytes from a address 0x001234
            // F\x01\x06F\x0c\x02\x00\x11\x22\x00\x00\x00\x00\x00\x00\x00\x00 == write 8 bytes to address 0x001122
            // F\x01\x06F\x04\x20\x00\x04\x00 == erase sector (4K) starting from address 0x000400
        }
#endif
/////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// end COPY 4
////////////////////////////////////////////////////////////////////////
