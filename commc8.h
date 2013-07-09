/////////////////////////////////////////////////////////////////
//      Begin COPY 8
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

#ifndef __PIC24H__
#ifndef _16F724
// EECON1
//        bit 7 EEPGD: Flash Program or Data EEPROM Memory Select bit
//            1 = Access Flash program memory
//            0 = Access data EEPROM memory
//        bit 6 CFGS: Flash Program/Data EEPROM or Configuration Select bit
//            1 = Access Configuration registers
//            0 = Access Flash program or data EEPROM memory
//        bit 5 Unimplemented: Read as '0'
//        bit 4 FREE: Flash Row Erase Enable bit
//            1 = Erase the program memory row addressed by TBLPTR on the next WR command (cleared
//               by completion of erase operation)
//           0 = Perform write only
//        bit 3 WRERR: Flash Program/Data EEPROM Error Flag bit
//            1 = A write operation is prematurely terminated (any Reset during self-timed programming in
//                normal operation, or an improper write attempt)
//            0 = The write operation completed
//          Note: When a WRERR occurs, the EEPGD and CFGS bits are not cleared.
//          This allows tracing of the error condition.
//        bit 2 WREN: Flash Program/Data EEPROM Write Enable bit
//            1 = Allows write cycles to Flash program/data EEPROM
//            0 = Inhibits write cycles to Flash program/data EEPROM
//        bit 1 WR: Write Control bit
//            1 = Initiates a data EEPROM erase/write cycle or a program memory erase cycle or write cycle
//                (The operation is self-timed and the bit is cleared by hardware once write is complete.
//                The WR bit can only be set (not cleared) in software.)
//            0 = Write cycle to the EEPROM is complete
//        bit 0 RD: Read Control bit
//            1 = Initiates an EEPROM read
//                (Read takes one cycle. RD is cleared in hardware. The RD bit can only be set (not cleared)
//                 in software. RD bit cannot be set when EEPGD = 1 or CFGS = 1.)
//            0 = Does not initiate an EEPROM read
unsigned char eeprom_read(unsigned char addr)
{
    EEADR = addr;
    EECON1 &= 0x3F;
    RD = 1;
    return EEDATA;
}
void eeprom_write(unsigned char addr, unsigned char value)
{
    EEADR = addr;
    EEDATA = value;
    EECON1 &= 0x3F;
    Carry = 0;
    if(GIE)
        Carry = 1;
    GIE = 0;
    WREN = 1;
    EECON2 = 0x55;
    EECON2 = 0xAA;
    WR = 1;
    WREN = 0;
    if(Carry)
        GIE = 1;
    while(WR)
    {
    }
}
#endif
#endif

void enable_uart(void)//bit want_ints)
{
 
#ifdef __PIC24H__
    TXIE = 0;  // this is macro redifinitions to be compatible with 16LF88 16LF884 18F2321
    RCIE = 0; // disable interrupt on recieve byte
    U1MODE = 0b000100010001000;
             //0                // bit 15 UARTEN: UARTx Enable bit(1)
                                //         1 = UARTx is enabled; all UARTx pins are controlled by UARTx as defined by UEN<1:0>
                                //         0 = UARTx is disabled; all UARTx pins are controlled by port latches; UARTx power consumption minimal
                                // bit 14 Unimplemented: Read as ‘0’
             // 0               // bit 13 USIDL: Stop in Idle Mode bit
                                //         1 = Discontinue module operation when device enters Idle mode
                                //         0 = Continue module operation in Idle mode
             //  0              // bit 12 IREN: IrDA® Encoder and Decoder Enable bit(2)
                                //         1 = IrDA encoder and decoder enabled
                                //         0 = IrDA encoder and decoder disabled
             //   1             // bit 11 RTSMD: Mode Selection for UxRTS Pin bit
                                //         1 = UxRTS pin in Simplex mode
                                //         0 = UxRTS pin in Flow Control mode
             //    0            // bit 10 Unimplemented: Read as ‘0’
                                // bit 9-8 UEN<1:0>: UARTx Enable bits
             //     00          //        11 = UxTX, UxRX and BCLK pins are enabled and used; UxCTS pin controlled by port latches
                                //        10 = UxTX, UxRX, UxCTS and UxRTS pins are enabled and used
                                //        01 = UxTX, UxRX and UxRTS pins are enabled and used; UxCTS pin controlled by port latches
                                //        00 = UxTX and UxRX pins are enabled and used; UxCTS and UxRTS/BCLK pins controlled by port latches
             //       1         // bit 7 WAKE: Wake-up on Start bit Detect During Sleep Mode Enable bit
                                //         1 = UARTx continues to sample the UxRX pin; interrupt generated on falling edge; bit cleared in hardware on following rising edge
                                //         0 = No wake-up enabled
             //        0        // bit 6 LPBACK: UARTx Loopback Mode Select bit
                                //         1 = Enable Loopback mode
                                //         0 = Loopback mode is disabled
             //         0       // bit 5 ABAUD: Auto-Baud Enable bit
                                //         1 = Enable baud rate measurement on the next character – requires reception of a Sync field (55h)
                                //             before other data; cleared in hardware upon completion
                                //         0 = Baud rate measurement disabled or completed
             //          0      // bit 4 URXINV: Receive Polarity Inversion bit
                                //        1 = UxRX Idle state is ‘0’
                                //        0 = UxRX Idle state is ‘1’
             //           1     // bit 3 BRGH: High Baud Rate Enable bit
                                //        1 = BRG generates 4 clocks per bit period (4x baud clock, High-Speed mode)
                                //        0 = BRG generates 16 clocks per bit period (16x baud clock, Standard mode)
             //            00   // bit 2-1 PDSEL<1:0>: Parity and Data Selection bits
                                //       11 = 9-bit data, no parity
                                //       10 = 8-bit data, odd parity
                                //       01 = 8-bit data, even parity
                                //       00 = 8-bit data, no parity
             //              0  // bit 0 STSEL: Stop Bit Selection bit
                                //        1 = Two Stop bits
                                //        0 = One Stop bit
    U1BRG = SPBRG_SPEED;
    U1STAbits.UTXBRK = 0;
    U1STAbits.UTXISEL1 = 0;     // 11 = Reserved
    U1STAbits.UTXISEL0 = 0;     // 10 = Interrupt generated when a character is transferred to the Transmit Shift register and the transmit buffer becomes empty
                                // 01 = Interrupt generated when the last transmission is over (last character shifted out of Transmit Shift register) and all the transmit operations are completed
                                // 00 = Interrupt generated when any character is transferred to the Transmit Shift Register (this implies at least one location is empty in the transmit buffer)

    U1STAbits.URXISEL1 = 0;
    U1STAbits.URXISEL0 = 0;     // 11 = Interrupt is set on UxRSR transfer making the receive buffer full (i.e., has 4 data characters)
                                // 10 = Interrupt is set on UxRSR transfer making the receive buffer 3/4 full (i.e., has 3 data characters)
                                // 0x = Interrupt is set when any character is received and transferred from the UxRSR to the receive buffer. Receive buffer has one or more characters
    TXEN = 0;  // this is macro redifinitions to be compatible with 16LF88 16LF884 18F2321 // disable transmit
    RCIF = 0;  // clean bit of receive interrupt
    RCIE = 1;  // enable interrupt on recieve byte
    U1MODEbits.UARTEN = 1; // enable uart
#ifdef USE_COM2
    // second UART
    TXIECOM2 = 0;  // this is macro redifinitions to be compatible with 16LF88 16LF884 18F2321
    RCIECOM2 = 0; // disable interrupt on recieve byte
#ifdef INVERS_COM2
    U2MODE = 0b000100010011000;
#else
    U2MODE = 0b000100010001000;
#endif                             
             //0                // bit 15 UARTEN: UARTx Enable bit(1)
                                //         1 = UARTx is enabled; all UARTx pins are controlled by UARTx as defined by UEN<1:0>
                                //         0 = UARTx is disabled; all UARTx pins are controlled by port latches; UARTx power consumption minimal
                                // bit 14 Unimplemented: Read as '0'
             // 0               // bit 13 USIDL: Stop in Idle Mode bit
                                //         1 = Discontinue module operation when device enters Idle mode
                                //         0 = Continue module operation in Idle mode
             //  0              // bit 12 IREN: IrDA Encoder and Decoder Enable bit(2)
                                //         1 = IrDA encoder and decoder enabled
                                //         0 = IrDA encoder and decoder disabled
             //   1             // bit 11 RTSMD: Mode Selection for UxRTS Pin bit
                                //         1 = UxRTS pin in Simplex mode
                                //         0 = UxRTS pin in Flow Control mode
             //    0            // bit 10 Unimplemented: Read as '0'
                                // bit 9-8 UEN<1:0>: UARTx Enable bits
             //     00          //        11 = UxTX, UxRX and BCLK pins are enabled and used; UxCTS pin controlled by port latches
                                //        10 = UxTX, UxRX, UxCTS and UxRTS pins are enabled and used
                                //        01 = UxTX, UxRX and UxRTS pins are enabled and used; UxCTS pin controlled by port latches
                                //        00 = UxTX and UxRX pins are enabled and used; UxCTS and UxRTS/BCLK pins controlled by port latches
             //       1         // bit 7 WAKE: Wake-up on Start bit Detect During Sleep Mode Enable bit
                                //         1 = UARTx continues to sample the UxRX pin; interrupt generated on falling edge; bit cleared in hardware on following rising edge
                                //         0 = No wake-up enabled
             //        0        // bit 6 LPBACK: UARTx Loopback Mode Select bit
                                //         1 = Enable Loopback mode
                                //         0 = Loopback mode is disabled
             //         0       // bit 5 ABAUD: Auto-Baud Enable bit
                                //         1 = Enable baud rate measurement on the next character requires reception of a Sync field (55h)
                                //             before other data; cleared in hardware upon completion
                                //         0 = Baud rate measurement disabled or completed
             //          1      // bit 4 URXINV: Receive Polarity Inversion bit
                                //        1 = UxRX Idle state is '0'
                                //        0 = UxRX Idle state is '1'
             //           1     // bit 3 BRGH: High Baud Rate Enable bit
                                //        1 = BRG generates 4 clocks per bit period (4x baud clock, High-Speed mode)
                                //        0 = BRG generates 16 clocks per bit period (16x baud clock, Standard mode)
             //            00   // bit 2-1 PDSEL<1:0>: Parity and Data Selection bits
                                //       11 = 9-bit data, no parity
                                //       10 = 8-bit data, odd parity
                                //       01 = 8-bit data, even parity
                                //       00 = 8-bit data, no parity
             //              0  // bit 0 STSEL: Stop Bit Selection bit
                                //        1 = Two Stop bits
                                //        0 = One Stop bit
    U2BRG = SPBRG_SPEEDCOM2;
    //   boud rate formula: BRGH = 0
    // BoudRate = Fcy/ (16 * (UxBRG + 1))
    //                      BRGH = 1
    // BoudRate = Fcy/ (4 * (UxBRG + 1))
    // (4 * (UxBRG + 1)) = Fcy/BoudRate
    // (UxBRG + 1) = Fcy/(4*BoudRate)
    // UxBRG = Fcy/(4*BoudRate) - 1;
    // 
    // for example on Fcy = 10.13375 MHz = 10133750 and U2BRG = 263 BoudRate = 9596
    // #define SPBRG_9600_10MIPS 262
    // #define SPBRG_57600_10MIPS 43
//    

    U2STAbits.UTXBRK = 0;
    U2STAbits.UTXISEL1 = 0;     // 11 = Reserved
    U2STAbits.UTXISEL0 = 0;     // 10 = Interrupt generated when a character is transferred to the Transmit Shift register and the transmit buffer becomes empty
                                // 01 = Interrupt generated when the last transmission is over (last character shifted out of Transmit Shift register) and all the transmit operations are completed
                                // 00 = Interrupt generated when any character is transferred to the Transmit Shift Register (this implies at least one location is empty in the transmit buffer)

    U2STAbits.URXISEL1 = 0;
    U2STAbits.URXISEL0 = 0;     // 11 = Interrupt is set on UxRSR transfer making the receive buffer full (i.e., has 4 data characters)
                                // 10 = Interrupt is set on UxRSR transfer making the receive buffer 3/4 full (i.e., has 3 data characters)
                                // 0x = Interrupt is set when any character is received and transferred from the UxRSR to the receive buffer. Receive buffer has one or more characters

                                // bit 15,13 UTXISEL<1:0>: Transmission Interrupt Mode Selection bits
                                //    11 = Reserved; do not use
                                //    10 = Interrupt when a character is transferred to the Transmit Shift Register, and as a result, the
                                //         transmit buffer becomes empty
                                //    01 = Interrupt when the last character is shifted out of the Transmit Shift Register; all transmit
                                //         operations are completed
                                //    00 = Interrupt when a character is transferred to the Transmit Shift Register (this implies there is
                                //         at least one character open in the transmit buffer)
                                // bit 14 UTXINV: Transmit Polarity Inversion bit
                                //         If IREN = 0:
                                //     1 = UxTX Idle state is '0'
                                //     0 = UxTX Idle state is '1'
                                //         If IREN = 1:
                                //     1 = IrDA encoded UxTX Idle state is '1'
                                //     0 = IrDA encoded UxTX Idle state is '0'
                                // bit 12 Unimplemented: Read as '0'
                                // bit 11 UTXBRK: Transmit Break bit
                                //     1 = Send Sync Break on next transmission Start bit, followed by twelve '0' bits, followed by Stop bit;
                                //         cleared by hardware upon completion
                                //     0 = Sync Break transmission disabled or completed
                                // bit 10 UTXEN: Transmit Enable bit(1)
                                //     1 = Transmit enabled, UxTX pin controlled by UARTx
                                //     0 = Transmit disabled, any pending transmission is aborted and buffer is reset. UxTX pin controlled
                                //          by port
                                // bit 9 UTXBF: Transmit Buffer Full Status bit (read-only)
                                //     1 = Transmit buffer is full
                                //     0 = Transmit buffer is not full, at least one more character can be written
                                // bit 8 TRMT: Transmit Shift Register Empty bit (read-only)
                                //     1 = Transmit Shift Register is empty and transmit buffer is empty (the last transmission has completed)
                                //     0 = Transmit Shift Register is not empty, a transmission is in progress or queued
                                // bit 7-6 URXISEL<1:0>: Receive Interrupt Mode Selection bits
                                //    11 = Interrupt is set on UxRSR transfer making the receive buffer full (i.e., has 4 data characters)
                                //    10 = Interrupt is set on UxRSR transfer making the receive buffer 3/4 full (i.e., has 3 data characters)
                                //    0x = Interrupt is set when any character is received and transferred from the UxRSR to the receive
                                //         buffer. Receive buffer has one or more characters
                                // bit 5 ADDEN: Address Character Detect bit (bit 8 of received data = 1)
                                //     1 = Address Detect mode enabled. If 9-bit mode is not selected, this does not take effect
                                //     0 = Address Detect mode disabled
                                // bit 4 RIDLE: Receiver Idle bit (read-only)
                                //     1 = Receiver is Idle
                                //     0 = Receiver is active
                                // bit 3 PERR: Parity Error Status bit (read-only)
                                //     1 = Parity error has been detected for the current character (character at the top of the receive FIFO)
                                //     0 = Parity error has not been detected
                                // bit 2 FERR: Framing Error Status bit (read-only)
                                //     1 = Framing error has been detected for the current character (character at the top of the receive
                                //         FIFO)
                                //     0 = Framing error has not been detected
                                // bit 1 OERR: Receive Buffer Overrun Error Status bit (read/clear only)
                                //     1 = Receive buffer has overflowed
                                //     0 = Receive buffer has not overflowed. Clearing a previously set OERR bit (1 ? 0 transition) resets
                                //         the receiver buffer and the UxRSR to the empty state
                                // bit 0 URXDA: Receive Buffer Data Available bit (read-only)
                                //     1 = Receive buffer has data, at least one more character can be read
                                //     0 = Receive buffer is empty


#ifdef INVERS_COM2
    U2STAbits.UTXINV = 1;       // inversion of TX data == idle = 0
#else
    U2STAbits.UTXINV = 0;       // TX data == idle = 1
#endif
    TXENCOM2 = 0;  // this is macro redifinitions to be compatible with 16LF88 16LF884 18F2321 // disable transmit
    RCIFCOM2 = 0;  // clean bit of receive interrupt
    RCIECOM2 = 1;  // enable interrupt on recieve byte
    U2MODEbits.UARTEN = 1; // enable uart
#endif // second uart

#else // 88,884,2321

    TX9 = 0;
    RX9 = 0;
//#ifdef _18F2321_18F25K20
//    SPBRGH = 0;
//    BRG16 = 0;
//#endif

    BRGH = 1; //Normal speed UART port 0x98 
              // 00000x00 BRGH: High Baud Rate Select bit
              //Asynchronous mode:
              // 1 = High speed 0 = Low speed
    SPBRG = SPBRG_SPEED;//51;// 9600
    //SPBRG = 25;// 19200
    //SPBRG = 12;// 38400
    //SPBRG = 8; // 57600

    SYNC = 0; // port 0x98 = 000x0000 x=0 asynch mode 1-synchr
    SPEN = 1; // port 0x18 x0000000 SPEN: Serial Port Enable bit
              // 1 = Serial port enabled (configures RB2/SDO/RX/DT 
              // and RB5/SS/TX/CK pins as serial port pins)

    //TXIF = 0;
    TXIE = 0;
    TXEN = 0;
    //TXEN = 1; //Enable transmission port 0x8c
              // AUSART Transmit Interrupt Enable bit
              // 1 = Enabled 0 = Disabled
    ADDEN = 0;
    //RCIF = 0;  // clean bit of receive interrupt
    RCIE = 1; // enable interrupt on recieve byte
    CREN = 1; // port 0x18 000x0000 CREN: Continuous Receive Enable bit
              // Asynchronous mode: 1 = Enables continuous receive
    
    //WREN = 1;
#endif
}    
void enable_I2C(void)
{
#ifdef __PIC24H__
   I2C1CON = 0b1000000000000000; 
                                 // bit 15 I2CEN: I2Cx Enable bit
           //  1                 //      1 = Enables the I2Cx module and configures the SDAx and SCLx pins as serial port pins
                                 //      0 = Disables the I2Cx module; all I2C pins are controlled by port functions
           //   0                // bit 14 Unimplemented: Read as ‘0’
           //    0               // bit 13 I2CSIDL: Stop in Idle Mode bit
                                 //      1 = Discontinue module operation when device enters Idle mode
                                 //      0 = Continue module operation in Idle mode
           //     0              // bit 12 SCLREL: SCLx Release Control bit (when operating as I2C slave)
                                 //      1 = Release SCLx clock 
                                 //      0 = Hold SCLx clock low (clock stretch)
                                 //          If STREN = 1:
                                 //          Bit is R/W (i.e., software may write ‘0’ to initiate stretch and write ‘1’ to release clock). Hardware clear
                                 //          at beginning of slave transmission and at end of slave reception.
                                 //          If STREN = 0:
                                 //          Bit is R/S (i.e., software may only write ‘1’ to release clock). Hardware clear at beginning of slave transmission.
           //      0             // bit 11 IPMIEN: Intelligent Platform Management Interface (IPMI) Enable bit
                                 //      1 = IPMI Support mode is enabled; all addresses Acknowledged
                                 //      0 = IPMI Support mode disabled
           //       0            // bit 10 A10M: 10-Bit Slave Address bit
                                 // 1 = I2CxADD register is a 10-bit slave address 0 = I2CxADD register is a 7-bit slave address
           //        0           // bit 9 DISSLW: Disable Slew Rate Control bit
                                 //      1 = Slew rate control disabled 0 = Slew rate control enabled
           //         0          // bit 8 SMEN: SMBus Input Levels bit
                                 // 1 = Enable I/O pin thresholds compliant with SMBus specification 0 = Disable SMBus input thresholds
           //          0         // bit 7 GCEN: General Call Enable bit (when operating as I2C slave)
                                 //      1 = Enable interrupt when a general call address is received in the I2CxRSR register (module is enabled for reception)
                                 //      0 = General call address disabled
           //           0        // bit 6 STREN: SCLx Clock Stretch Enable bit (I2C Slave mode only; used in conjunction with SCLREL bit)
                                 //      1 = Enable software or receive clock stretching 0 = Disable software or receive clock stretching
           //            0       // bit 5 ACKDT: Acknowledge Data bit (I2C Master mode; receive operation only) 
                                 // Value that will be transmitted when the software initiates an Acknowledge sequence
                                 //      1 = Send NACK during Acknowledge 0 = Send ACK during Acknowledge
           //             0      // bit 4 ACKEN: Acknowledge Sequence Enable bit (I2C Master mode receive operation)
                                 //      1 = Initiate Acknowledge sequence on SDAx and SCLx pins and transmit ACKDT data bit (hardware clear at end of master Acknowledge sequence)
                                 //      0 = Acknowledge sequence not in progress
           //              0     // bit 3 RCEN: Receive Enable bit (I2C Master mode)
                                 //      1 = Enables Receive mode for I2C (hardware clear at end of eighth bit of master receive data byte)
                                 //      0 = Receive sequence not in progress
           //               0    // bit 2 PEN: Stop Condition Enable bit (I2C Master mode)
                                 //      1 = Initiate Stop condition on SDAx and SCLx pins (hardware clear at end of master Stop sequence)
                                 //      0 = Stop condition not in progress
           //                0   // bit 1 RSEN: Repeated Start Condition Enable bit (I2C Master mode)
                                 //      1 = Initiate Repeated Start condition on SDAx and SCLx pins (hardware clear at end of master Repeated Start sequence)
                                 //      0 = Repeated Start condition not in progress
           //                 0  // bit 0 SEN: Start Condition Enable bit (I2C Master mode)
                                 //      1 = Initiate Start condition on SDAx and SCLx pins (hardware clear at end of master Start sequence)
                                 //      0 = Start condition not in progress
      IFS1bits.MI2C1IF = 0; // clean interrupt
      IEC1bits.MI2C1IE = 1; // enable interupt on master
#else // end of PIC24

#ifdef _18F2321_18F25K20
 #ifndef I2C_ONLY_MASTER
    SSPCON1 =0b00111110; // TBD in _18F2321_18F25K20 master I2C implemented 
 #else
    SSPCON1 =0b00011000;
    SSPCON2 =0b00000000;
 #endif
#else // 88, 884
    #warning "pic18f884 does have I2C master support in firmaware"
    SSPCON = 0b00111110;
#endif
             //0        -WCOL: Write Collision Detect bit
             //          1 = An attempt to write the SSPBUF register failed because the SSP module is busy
             //              (must be cleared in software)
             //          0 = No collision
             // 0       -SSPOV: Receive Overflow Indicator bit
             //          1 = A byte is received while the SSPBUF register is still holding the previous byte. SSPOV is
             //              a “don’t care” in Transmit mode. SSPOV must be cleared in software in either mode.
             //          0 = No overflow
             //  1      -SSPEN: Synchronous Serial Port Enable bit
             //          1 = Enables the serial port and configures the SDA and SCL pins as serial port pins
             //          0 = Disables serial port and configures these pins as I/O port pins
             //   1     -CKP: Clock Polarity Select bit.SCK release control
             //          1 = Enable clock
             //          0 = Holds clock low (clock stretch). (Used to ensure data setup time.)
             //    1110-SSPM<3:0>: Synchronous Serial Port Mode Select bits
             //          0110 = I2C Slave mode, 7-bit address
             //          0111 = I2C Slave mode, 10-bit address
             //          1011 = I2C Firmware Controlled Master mode (Slave Idle)
             //          1110 = I2C Slave mode, 7-bit address with Start and Stop bit interrupts enabled
             //          1111 = I2C Slave mode, 10-bit address with Start and Stop bit interrupts enabled
             //          1000 = I2C MASTER mode
    SMP = 0;
    CKE = 0;

    SSPEN = 1; // enable I2C
    //P = 0;     // no stop
    //S = 0;     // no start
    //BF = 0;    // nothing in a buffer (may be better read SSPBUFF?)
    SSPIF = 0;  // clean inderrupt
    SSPIE = 1;  // enable interrupt
    //SSPOV = 0;  // clean owerflow
#endif
}
void EnableTMR1(void)
{
#ifdef __PIC24H__

#else
    TMR1L =0;
    TMR1H = 0;
    T1CON = 0b1001001;
    //                 T1CON: TIMER1 CONTROL REGISTER (ADDRESS 10h)
    //        1        TMR1ON: Timer1 On bit 1 = Enables Timer1 0 = Stops Timer1
    //                         1 = External clock from pin RB6/AN5(1)/PGC/T1OSO/T1CKI (on the rising edge)
    //                         0 = Internal clock (FOSC/4)
    //         00      T1CKPS<1:0>: Timer1 Input Clock Prescale Select bits
    //                        11 =1:8 Prescale value with FOSC/4 == 16cmd per count
    //                        10 =1:4 Prescale value with FOSC/4 == 8 cmd per count
    //                        01 =1:2 Prescale value with FOSC/4 == 4 cmd per count
    //                        00 =1:1 Prescale value with FOSC/4 == 2 cmd per count
    //           1     T1OSCEN - 1 ocsilator enabled 0 disabled
    //            0    T1SYNC  - ignored with TMR1CS == 0
    //             0   TMR1CS: Timer1 Clock Source Select bit internal FOSC/4 
    //              1  TMR1ON - timer enabled
    TMR1IF = 0;
    TMR1IE = 0;
    //TMR1ON = 1;  // TMR1ON: Timer1 On bit
                 // 1 = Enables Timer1
                 // 0 = Stops Timer1
#endif
}
//#define SSPORT PORTA
//#define SSCLOCK 7
//#define SSDATA_IN 6
//#define SSDATA_OUT 5
//#define SSCS       4
#ifdef SSPORT
// it is working with port in bank 0
#pragma rambank RAM_BANK_0
void SendSSByte(unsigned char bByte)
{
    WORD bWork;
    //bitclr(SSPORT,SSCS); // set low Chip Select
    bWork = 8;
#pragma updateBank 0

    do
    {
#ifdef __PIC24H__
        // nobody uses portA ??? make sure!!!
        // otherwise it must be 3 commands instead of one:
         bclr(SSPORT,SSCLOCK);
         nop();
         bclr(SSPORT,SSDATA_IN);

        // SSCLOCK RA0(pin2), SSDATA_IN RA1(pin3), SSDATA_OUT RA2(pin9), SSCS RA3(pin10)
        //PORTA = 0b00000000;
#else
        bclr(SSPORT,SSCLOCK);
        bclr(SSPORT,SSDATA_IN);
#endif
		if (bittest(bByte,7))
            bset(SSPORT,SSDATA_IN);
#ifdef _OPTIMIZED_
  #ifdef __PIC24H__
         bByte<<=1;
  #else
   #ifdef      _18F2321_18F25K20
            #asm
              RLCF bByte,1,1
            #endasm
   #else
            RLF(bByte,1);
   #endif
  #endif
#else // not optimized version
        bByte<<=1;
#endif
        bset(SSPORT,SSCLOCK);
    }
    while (--bWork); // 7*8 = 56 or 8*8 = 64 commands
    bclr(SSPORT,SSCLOCK);
    //nop();
    //bclr(SSPORT,SSDATA_IN);
    //bset(SSPORT,SSCS); // set high Chip Select
}
#pragma updateBank 1
void SendSSByteFAST(unsigned char bByte)
{
#ifdef __PIC24H__
    // nobody uses portA ??? make sure!!!
    // otherwise it must be changed on bit 1:
    // SSCLOCK RA0(pin2), SSDATA_IN RA1(pin3), SSDATA_OUT RA2(pin9), SSCS RA3(pin10)
    bclr(SSPORT,SSDATA_IN);
    nop();
    bset(SSPORT,SSCLOCK);  // bit 7
    nop();
    bclr(SSPORT,SSCLOCK);
    nop();
    bset(SSPORT,SSCLOCK);  // bit 6
    nop();
    bclr(SSPORT,SSCLOCK);
    nop();
    bset(SSPORT,SSCLOCK);  // bit 5
    nop();
    bclr(SSPORT,SSCLOCK);
    nop();
    bset(SSPORT,SSCLOCK);  // bit 4
    nop();
    bclr(SSPORT,SSCLOCK);
    nop();
    bset(SSPORT,SSCLOCK);  // bit 3
    nop();
    bclr(SSPORT,SSCLOCK);
    nop();
    bset(SSPORT,SSCLOCK);  // bit 2
    nop();
    bclr(SSPORT,SSCLOCK);
    //if (bittest(bByte,1))
    if (bByte&2)
    {
        bset(SSPORT,SSDATA_IN);
        nop();
    }
    bset(SSPORT,SSCLOCK);  // bit 1
    nop();
    PORTA = 0b00000000;
    //bclr(SSPORT,SSCLOCK);
    //nop();
    //bclr(SSPORT,SSDATA_IN); // essential

    //if (bittest(bByte,0))
    if (bByte&1)
    {
        bset(SSPORT,SSDATA_IN);
        nop();
    }
    bset(SSPORT,SSCLOCK);  // bit 0
    nop();
    bclr(SSPORT,SSCLOCK);  // 42 commands
#else
    bclr(SSPORT,SSDATA_IN);
    bset(SSPORT,SSCLOCK);  // bit 7
    bclr(SSPORT,SSCLOCK);
    bset(SSPORT,SSCLOCK);  // bit 6
    bclr(SSPORT,SSCLOCK);
    bset(SSPORT,SSCLOCK);  // bit 5
    bclr(SSPORT,SSCLOCK);
    bset(SSPORT,SSCLOCK);  // bit 4
    bclr(SSPORT,SSCLOCK);
    bset(SSPORT,SSCLOCK);  // bit 3
    bclr(SSPORT,SSCLOCK);
    bset(SSPORT,SSCLOCK);  // bit 2
    bclr(SSPORT,SSCLOCK);
    //if (bittest(bByte,1))
    if (bByte&2)
        bset(SSPORT,SSDATA_IN);
    bset(SSPORT,SSCLOCK);  // bit 1
    bclr(SSPORT,SSCLOCK);
    bclr(SSPORT,SSDATA_IN);
    //if (bittest(bByte,0))
    if (bByte&1)
        bset(SSPORT,SSDATA_IN);
    bset(SSPORT,SSCLOCK);  // bit 0
    bclr(SSPORT,SSCLOCK);  // 23 commands
#endif
}
#ifndef SSPORT_READ
#define SSPORT_READ  SSPORT
#define SSDATA_OUT_READ SSDATA_OUT
#endif

unsigned char GetSSByte(void)
{
    int bWork;
    unsigned int bWork2;
    //bitclr(SSPORT,SSCS); // set low Chip Select
    bWork = 8;
#pragma updateBank 0
    bWork2 = 0;
    do
    {
        bWork2 <<=1;
        bset(SSPORT,SSCLOCK);
        //nop();
        //bitclr(bWork2,0); // bWork2 is unsigned == zero in low bit garanteed check assembler code to confirm
//#undef SSDATA_OUT2
#ifdef SSDATA_OUT2
        if (btest(SSPORT_READ2,SSDATA_OUT))
        {
            if (btest(SSPORT_READ2,SSDATA_OUT2))
                goto FLASH_MAJORITY;
            else if (btest(SSPORT_READ2,SSDATA_OUT3))
                goto FLASH_MAJORITY;
        }
        else if (btest(SSPORT_READ2,SSDATA_OUT2))
                 if (btest(SSPORT_READ2,SSDATA_OUT3))
                 {
FLASH_MAJORITY:
                     bitset(bWork2,0);
                 }
#else
        if (btest(SSPORT_READ,SSDATA_OUT_READ))
            bitset(bWork2,0);
#endif
        bclr(SSPORT,SSCLOCK);
    }
    while (--bWork);
    return bWork2;
    //bset(SSPORT,SSCS); // set high Chip Select
}
#pragma updateBank 1
#ifdef FLASH_POWER_DOWN
void CsLow(void)
{
    
    bclr(SSPORT,SSCS); // set low Chip Select
    SendSSByte(0xab);  // power save mode off
    bset(SSPORT,SSCS); // set high Chip Select
    //                    3mks needs to wait
}
void CsHigh(void)
{
     bset(SSPORT,SSCS); // set high Chip Select
     nop();nop();
     bclr(SSPORT,SSCS); // set low Chip Select
     SendSSByte(0xb9);  // set power save mode
     bset(SSPORT,SSCS); // set high Chip Select
}
#endif
#endif
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// end COPY 8
///////////////////////////////////////////////////////////////////////
