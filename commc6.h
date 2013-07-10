////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// begin COPY 6
///////////////////////////////////////////////////////////////////////   

        AInQu.iEntry = 0;
        AInQu.iExit = 0;
		AInQu.iQueueSize = 0;

		AOutQu.iEntry = 0;
        AOutQu.iExit = 0;
		AOutQu.iQueueSize = 0;

        AInI2CQu.iEntry = 0;
        AInI2CQu.iExit = 0;
		AInI2CQu.iQueueSize = 0;

		AOutI2CQu.iEntry = 0;
        AOutI2CQu.iExit = 0;
		AOutI2CQu.iQueueSize = 0;
        //TimerB1=0;
#ifdef BT_TIMER1
#else
		TimerB1.SetSleep = 0;
        TimerB1.SetSyncTime = 0;
#endif

        //Main= 0;
        Main.getCMD = 0;
        Main.ESCNextByte = 0;
        Main.PrepI2C = 0;
        Main.DoneWithCMD = 1;
#ifdef NEW_CMD_PROC
        Main.CheckESC = 1;
#endif
        RetransmitLen = 0;
        Main.RetransmitTo = 0;
#ifdef NON_STANDART_MODEM
        Main.SendOverLink = 0;
#endif
        //Main.SendWithEsc = 0;
        //Main.CommLoopOK = 0;

        SSPADD = UnitADR<<1;
        I2C.LockToI2C = 0;
        I2C.WaitQuToEmp = 0;
        I2C.SetI2CYesNo = 0;
        I2C.EchoWhenI2C = 1;

        I2C_B1.I2CBusBusy = 0;
        //BlockComm = 0;

        I2C.Timer0Fired = 0;
        I2C.LastWasUnitAddr = 0;
#ifdef SPEED_SEND_DATA
        Speed.SpeedSend = 0;
        Speed.SpeedSendLocked = 0;
        Speed.SpeedSendUnit = 0;
        Speed.SpeedSendWithESC = 0;
        Speed.SpeedESCwas = 0;

#endif
#ifdef I2C_INT_SUPPORT
		I2C_B1.NeedMaster = 0;
        I2C_B1.NeedRestart = 0;
        I2C_B1.NeedStop = 0;
        I2C_B1.NeedReceive = 0;
        I2C_B1.NeedSend = 0;
        //NeedReciveACK = 0;


#endif
        I2C_B1.I2CMasterDone = 1;
        RetrUnit = 0;
        AllowMask = 0xff;
#ifdef ALLOW_RELAY_TO_NEW
        AllowOldMask1 = 0xff;AllowOldMask2=0xff;AllowOldMask3=0xff;AllowOldMask4=0xff; AllowOldMask =  0xff;
#endif
        UnitMask1 = 0xff;
        UnitMask2 = 0;
        Main.prepStream = 1;
        Main.prepCmd = 0;
        Main.prepSkip = 0;
        Main.prepZeroLen = 0;
        UnitFrom = 0;
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

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// end COPY 6
