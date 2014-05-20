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
            if (BTExternal.iQueueSize)
            {
                if (DataB0.BTExternalWasStarted)
                {
                    if (OutPacketUnit)
                        goto NEXT_PORTION;
                }
                else
                {
                    DataB0.BTExternalWasStarted = 1;
                    // that will do loop (RX/TX) but will block Comm process
                    if (OutPacketUnit==0)
                    {
NEXT_PORTION:
                        while(AOutQu.iQueueSize < (OUT_BUFFER_LEN-2))
                        {
                            putch(getchExternal());
                            if (OutPacketUnit ==0)
                                break;
                        }
                        if (BTExternal.iQueueSize ==0)
                            DataB0.BTExternalWasStarted = 0;
                    }
                }
            }
            if (BTInternal.iQueueSize)
            {
                if (Main.getCMD == 0) // now not CMD mode
                {
                    if (Main.DoneWithCMD)// no long command in process
                    {
                        Main.getCMD = 1;
                        // ready to process all bytes from remote unit inside unit (i.e. FLASH and etc)
                        while(BTInternal.iQueueSize)
                        {
                            ProcessCMD(getchInternal());
                        }
                        Main.getCMD = 0;
                    }
                }
            }
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
