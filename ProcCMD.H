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

#include "commc3.h"

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
             {
                 return;
             }
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
             {
                 ATCMDStatus = 4;        // on next entry will be 5
             }
             else if (bByte == 's') // ats command
             { 
                 ATCMDStatus = 9;        // on next entry will be 10
             }
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
          else if (ATCMDStatus == 24) 
          {
          }
          // something wrong == done with AT
          ATCMDStatus = 0;
          Main.DoneWithCMD = 1; // long command done
          return;
    }
CONTINUE_NOT_AT:
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
    }
#include "commc4.h"


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
        else if (bByte == 'a')
        {
            ATCMDStatus = 1;
            Main.DoneWithCMD = 0; // long command
        }
        else if (bByte == 'A')  // additional command
        {
            ATCMDStatusAddon = 1;
            Main.DoneWithCMD = 0; // long command
        }
        else if (bByte == 'U')  // CMD upgrade PIC from storage <U><XXXXXX>=<xxxxxx>=<llll>// additional command
        {
            DataB0.UpgradeProgFlags =1;
            UpgradeProgStatus = 0;
            Main.DoneWithCMD = 0; // long command
        }
        else if (bByte == 'C')  // CRC flash <C><XXXXXX>=<xxxxxx>=<CRC1>=<CRC2>=<CRC3>=<CRC4> 
        {                       // if CRCX is OK respond "O000"
                                // if CRCX is not OK respond o<CRC1>=<CRC2>=<CRC3>=<CRC4>
                                // on each 1/4 of the 1/4 
                                // i.e C008000=00a000=1234=5678=9abc=def0
                                // CRC of 008000-008800 = 1234
                                // CRC    008800-009000 = 5678
                                //        009000-009800 = 9abc
                                //        009800-00a000 = def0
                                //   responce OOOO in case of all 4 CRC are OK
                                //   responce o0101=2345=5432=9876 on each not matched CRC
        }
        else if (bByte == 'c')  // CRC prog  <c><XXXXXX>=<xxxxxx>=<CRC1>=<CRC2>=<CRC3>=<CRC4>
        {
        }
SKIP_BYTE:;
    } // do not confuse: this is a else from getCMD == 1
}
