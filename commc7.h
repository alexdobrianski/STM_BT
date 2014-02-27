////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// begin COPY 7
///////////////////////////////////////////////////////////////////////   

    while(1)
    {
        putch_main();
        if (CallBkMain() == 0) // return 0 = do continue; 1 = process queues
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
            if (CallBkComm()) // return 0 = do not process byte; 1 = process;
            {
PROCESS_IN_CMD:
                 ProcessCMD(getch());
#ifdef RX_FULL
               if (RX_FULL)
               {
                   if (AInQu.iQueueSize > (BUFFER_LEN-CRITICAL_BUF_SIZE))
                       ;//RX_FULL = 1; // that is already set in interrupt routine
                   else
                   {                   // but if queue is out cleaning - allow to RX bytes.
                       RX_FULL = 0;
                   }
               }
#endif

NO_PROCESS_IN_CMD:;
            }
        }
        else  // nothing in comm input queue ==  can sleep till interrupt
        {
#ifdef NON_STANDART_MODEM
            if (FlashEntry == FlashExit)
            {
                if (FlashEntryBH == FlashExitBH)
                    Main.FlashRQ = 0;
                else
                {
      
NEEDS_FLASH_PROC:   Main.FlashRQ = 1;
                    if (!Main.getCMD) // only if it is not a command mode
                    {
                        if (RelayPkt==0) // can be write to com -> data can be sent
                        {
                            // now it is possible to process data from flash
                            Main.getCMD = 1;
                            CS_HIGH;
                            //ReadPrevLen;
                            //ReadNextLen;
                            //ReadTypePkt;

                            Main.getCMD = 0;
                            Main.FlashRQ = 0;
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
#ifdef _18F2321_18F25K20
               //sleep();
#endif
#endif
            }
        }
   }
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// end COPY 7
///////////////////////////////////////////////////////////////////////
