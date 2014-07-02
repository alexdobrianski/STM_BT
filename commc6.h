////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// begin COPY 6
///////////////////////////////////////////////////////////////////////   
#ifndef NOT_USE_COM1
        AInQu.iEntry = 0;
        AInQu.iExit = 0;
		AInQu.iQueueSize = 0;

		AOutQu.iEntry = 0;
        AOutQu.iExit = 0;
		AOutQu.iQueueSize = 0;
#endif
#ifndef NO_I2C_PROC
        AInI2CQu.iEntry = 0;
        AInI2CQu.iExit = 0;
		AInI2CQu.iQueueSize = 0;

		AOutI2CQu.iEntry = 0;
        AOutI2CQu.iExit = 0;
		AOutI2CQu.iQueueSize = 0;
#endif
        //TimerB1=0;
#ifdef BT_TIMER1
#else
		TimerB1.SetSleep = 0;
        TimerB1.SetSyncTime = 0;
#endif

        //Main = 0;
        Main.getCMD = 0;
        Main.ESCNextByte = 0;
        Main.OutPacket = 0;
        Main.OutPacketESC = 0;
        Main.OutPacketLenNull = 0;

        Main.GetPkt = 0;
        Main.GetPktESC = 0;
        Main.GetPktLenNull = 0;
        Main.RelayPktLenNull = 0;
        Main.RelayGranted = 0;
        Main.RelayPktOverload = 0;
        Main.RelayPktESC = 0;
        Main.PrepI2C = 0;
        Main.DoneWithCMD = 1;

        //Main.OutPacket = 0;
        Main.OutPacketESC = 0;
        Main.LockToQueue = 0;

        Main.RetransmitTo = 0;
#ifdef NON_STANDART_MODEM
        Main.SendOverLink = 0;
        Main.SendOverLinkAndProc = 0;
        Main.SendOverLinkStarted = 0;
        Main.FlashRQ = 0;
        Main.PingRQ = 0;
        Main.PingRSPRQ = 0;
        Main.DoPing= 0;
        Main.ConstantPing=0;
#endif
        //Main.SendWithEsc = 0;
        //Main.CommLoopOK = 0;
        Main.LastWasUnitAddr = 0;
        PingAttempts = 0;

        SSPADD = UnitADR<<1;
        I2C.LockToI2C = 0;
        I2C.WaitQuToEmp = 0;
        I2C.SetI2CYesNo = 0;
        I2C.EchoWhenI2C = 1;

        I2C_B1.I2CBusBusy = 0;
        //BlockComm = 0;

        I2C.Timer0Fired = 0;
#ifdef I2C_INT_SUPPORT
		I2C_B1.NeedMaster = 0;
        I2C_B1.NeedRestart = 0;
        I2C_B1.NeedStop = 0;
        I2C_B1.NeedReceive = 0;
        I2C_B1.NeedSend = 0;
        //NeedReciveACK = 0;


#endif

        I2C_B1.I2CMasterDone = 1;
        RelayPkt = 0;
        OutPacketUnit = 0;
        AllowMask = 0xff;
        UnitMask1 = 0xff;
        UnitMask2 = 0;
        UnitFrom = '9'; // default unit = 9 TBD on gr stn it is ok but for cube/picosat it must be main comp
#ifdef SYNC_CLOCK_TIMER
#ifdef __PIC24H__
        FSR_REGISTER = &Ttilad;
        for (bWork = 0; bWork < 5*sizeof(Ttilad);bWork++)
        {
             PTR_FSR = 0;
        }
#else  // not __PIC24H__
#endif // __PIC24H__
#endif // SYNC_CLOCK_TIMER
#ifdef SSPORT
        DataB3.FlashWas1byteWrite = 0;
#endif
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// end COPY 6
