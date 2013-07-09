///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// begin of COPY 0
///////////////////////////////////////////////////////////////////////
#ifdef _18F2321
#define _18F2321_18F25K20
#endif

#ifdef _18F25K20
#define _18F2321_18F25K20
#endif

#ifdef _16F884
#define _16F884_16F724
#endif

#ifdef _16F724
#define _16F884_16F724
#endif


// preprocessing different MASKS for different units
#if (MY_UNIT == '1')
    #define UNIT_MASK   0b00000010
    #define UNIT_MASK_H 0b11111110
#endif 

#if (MY_UNIT == '2')
    #define UNIT_MASK   0b00000100
    #define UNIT_MASK_H 0b11111100
#endif 

#if (MY_UNIT == '3')
    #define UNIT_MASK   0b00001000
    #define UNIT_MASK_H 0b11111000
#endif 

#if (MY_UNIT == '4')
    #define UNIT_MASK   0b00010000
    #define UNIT_MASK_H 0b11110000
#endif 

#if (MY_UNIT == '5')
    #define UNIT_MASK   0b00100000
    #define UNIT_MASK_H 0b11100000
#endif 

#if (MY_UNIT == '6')
    #define UNIT_MASK   0b01000000
    #define UNIT_MASK_H 0b11000000
#endif 

#if (MY_UNIT == '7')
    #define UNIT_MASK   0b10000000
    #define UNIT_MASK_H 0b10000000
#endif 

#if (MY_UNIT == '8')
    #define UNIT_MASK   0b10000000
    #define UNIT_MASK_H 0b00000000
#endif 


//#define pAddInc\
//            FSR_REGISTER ++;\
//            PTR_FSR+=CARRY_BYTE;\
//            if (Carry)\
//                CARRY_BYTE = 1;\
//            else\
//                CARRY_BYTE = 0;
#define pSubInc\
            FSR_REGISTER ++;\
            PTR_FSR-=CARRY_BYTE;\
            CARRY_BYTE = !Carry;

#define pSub___(x,y,z)\
            PTR_FSR-=x;\
            if (!Carry)\
                CARRY_BYTE++;\
            PTR_FSR-=y;\
            if (!Carry)\
                CARRY_BYTE++;\
            PTR_FSR-=z;\
            if (!Carry)\
                CARRY_BYTE++;\
            pSubInc;


#define pSub__(x,y)\
            PTR_FSR-=x;\
            if (!Carry)\
                CARRY_BYTE++;\
            PTR_FSR-=y;\
            if (!Carry)\
                CARRY_BYTE++;\
            pSubInc;

#define pSub_(x)\
            PTR_FSR-=x;\
            if (!Carry)\
                CARRY_BYTE++;\
            pSubInc;

#define pSubS___(x,y,z)\
            if (bittest(x,7))\
            {\
                W =0xff;\
                PTR_FSR-=W;\
                if (!Carry)\
                    CARRY_BYTE++;\
            }\
            if (bittest(y,7))\
            {\
                W =0xff;\
                PTR_FSR-=W;\
                if (!Carry)\
                    CARRY_BYTE++;\
            }\
            if (bittest(z,7))\
            {\
                W =0xff;\
                PTR_FSR-=W;\
                if (!Carry)\
                    CARRY_BYTE++;\
            }\
            pSubInc;


#define pSubS__(x,y)\
            if (bittest(x,7))\
            {\
                W =0xff;\
                PTR_FSR-=W;\
                if (!Carry)\
                    CARRY_BYTE++;\
            }\
            if (bittest(y,7))\
            {\
                W =0xff;\
                PTR_FSR-=W;\
                if (!Carry)\
                    CARRY_BYTE++;\
            }\
            pSubInc;

#define pSubS_(x)\
            if (bittest(x,7)) \
            {\
                W =0xff;\
                PTR_FSR-=W;\
                if (!Carry)\
                    CARRY_BYTE++;\
            }\
            pSubInc;

#define pSubSEnd___(x,y,z)\
            if (bittest(x,7))\
            {\
                PTR_FSR++;\
            }\
            if (bittest(y,7))\
            {\
                PTR_FSR++;\
            }\
            if (bittest(z,7))\
            {\
                PTR_FSR++;\
            }


#define pSubSEnd__(x,y)\
            if (bittest(x,7))\
            {\
                PTR_FSR++;\
            }\
            if (bittest(y,7))\
            {\
                PTR_FSR++;\
            }

#define pSubSEnd_(x)\
            if (bittest(x,7))\
            {\
                PTR_FSR++;\
            }





#define pAddInc\
            FSR_REGISTER ++;\
            PTR_FSR+=CARRY_BYTE;\
            CARRY_BYTE = Carry;

#define pAdd___(x,y,z)\
            PTR_FSR+=x;\
            if (Carry)\
                CARRY_BYTE=++;\
            PTR_FSR+=y;\
            if (Carry)\
                CARRY_BYTE++;\
            PTR_FSR+=z;\
            if (Carry)\
                CARRY_BYTE++;\
            pAddInc;

#define pAdd__(x,y)\
            PTR_FSR+=x;\
            if (Carry)\
                CARRY_BYTE++;\
            PTR_FSR+=y;\
            if (Carry)\
                CARRY_BYTE++;\
            pAddInc;

#define pAdd_(x)\
            PTR_FSR+=x;\
            if (Carry)\
                CARRY_BYTE++;\
            pAddInc;

#define pAddS__(x,y)\
            if (bittest(x,7))\
            {\
                W =0xff;\
                PTR_FSR+=W;\
                if (Carry)\
                    CARRY_BYTE++;\
            }\
            if (bittest(y,7))\
            {\
                W =0xff;\
                PTR_FSR+=W;\
                if (Carry)\
                    CARRY_BYTE++;\
            }\
            pAddInc;

#define pAddS_(x)\
            if (bittest(x,7)) \
            {\
                W =0xff;\
                PTR_FSR+=W;\
                if (Carry)\
                    CARRY_BYTE++;\
            }\
            pAddInc;

#define pAddSEnd__(x,y)\
            if (bittest(x,7))\
            {\
                PTR_FSR--;\
            }\
            if (bittest(y,7))\
            {\
                PTR_FSR--;\
            }

#define pAddSEnd_(x)\
            if (bittest(x,7))\
            {\
                PTR_FSR--;\
            }

#define SSPBUF_RX SSPBUF
#define SSPBUF_TX SSPBUF


#define VOLATILE
// on 88,884,2321 I2C_BRG=SSPADD
#define I2C_BRG SSPADD


// to be compatible with VS2010
#ifdef WIN32
    #ifdef _16F88
    #endif
    #ifdef _16F884
    #endif

    #ifdef _18F2321
    //#include "18F2321.H"
    #endif
#endif
// on small processors it is 1 byte
#define WORD unsigned char
#define UWORD unsigned char
// different processors different includes and different RAM BANKS
//////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifdef _16F88
#include "int16CXX.H"
#define RAM_BANK_0 0
#define RAM_BANK_1 1
#define RAM_BANK_2 2
#define RAM_BANK_3 3
#define RAM_BANK_4 2
#define RAM_BANK_5 3
#define TIMER0_CONTROL_REG OPTION_REG
#define TIMER0_BYTE TMR0
#define TIMER0_INT_ENBL TMR0IE
#define TIMER0_INT_FLG  TMR0IF
#define FSR_REGISTER FSR
#define PTR_FSR INDF
#define INT0_EDG INTEDG
#define INT0_FLG INT0IF
#define INT0_ENBL INT0IE
#define I2CPORT PORTB
#define I2CTRIS TRISB
#define I2C_SDA 1
#define I2C_SCL 4
#define TIMER0_INT_ENBL TMR0IE
#define TIMER0_INT_FLG  TMR0IF
#undef _Q_PROCESS
#ifdef I2C_INT_SUPPORT
#warning "No I2C wirmware support for PIC16F88 device"
#undef I2C_INT_SUPPORT
#endif
#define _TRMT TRMT
#endif
//////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifdef _16F884
#include "int16CXX.H"
#define RAM_BANK_0 0
#define RAM_BANK_1 1
#define RAM_BANK_2 2
#define RAM_BANK_3 3
#define RAM_BANK_4 2
#define RAM_BANK_5 3

#define TIMER0_CONTROL_REG OPTION_REG
#define TIMER0_BYTE TMR0
#define TIMER0_INT_ENBL T0IE
#define TIMER0_INT_FLG  T0IF
#define FSR_REGISTER FSR
#define PTR_FSR INDF
#define INT0_EDG INTEDG
#define INT0_FLG INTF
#define INT0_ENBL INTE
#define I2CPORT PORTB
#define I2CTRIS TRISB
#define I2C_SDA 1
#define I2C_SCL 4
#ifdef I2C_INT_SUPPORT
//#warning "No I2C wirmware support for PIC16F884 device"
//#undef I2C_INT_SUPPORT
#endif
#undef _Q_PROCESS
#define _TRMT TRMT
#endif

//////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifdef _16F724
#include "int16CXX.H"
#define RAM_BANK_0 0
#define RAM_BANK_1 1
#define RAM_BANK_2 2
#define RAM_BANK_3 3
#define RAM_BANK_4 2
#define RAM_BANK_5 3

#define TIMER0_CONTROL_REG OPTION_REG
#define TIMER0_BYTE TMR0
#define TIMER0_INT_ENBL T0IE
#define TIMER0_INT_FLG  T0IF
#define FSR_REGISTER FSR
#define PTR_FSR INDF
#define INT0_EDG INTEDG
#define INT0_FLG INTF
#define INT0_ENBL INTE
#define I2CPORT PORTB
#define I2CTRIS TRISB
#define I2C_SDA 1
#define I2C_SCL 4
#ifdef I2C_INT_SUPPORT
//#warning "No I2C wirmware support for PIC16F884 device"
//#undef I2C_INT_SUPPORT
#endif
#undef _Q_PROCESS
#define _TRMT TRMT
#endif


//////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifdef _18F25K20
#undef _Q_PROCESS
#ifdef __18CXX
//char PORTE    @ 0xF84;
#include "p18f2321.h"
#define RCIF PIR1bits.RCIF
#define RCIE PIE1bits.RCIE
#define CREN RCSTAbits.CREN
#define TXIE PIE1bits.TXIE
#define TXIF PIR1bits.TXIF
#define TRMT TXSTAbits.TRMT
#define BCLIF PIR2bits.BCLIF
#define SEN SSPCON2bits.SEN
#define PEN SSPCON2bits.PEN
#define SSPIF PIR1bits.SSPIF
#define SSPOV SSPCON1bits.SSPOV
#define BF SSPSTATbits.BF
#define ACKEN SSPCON2bits.ACKEN
#define ACKDT SSPCON2bits.ACKDT
#define RCEN SSPCON2bits.RCEN
#define RSEN SSPCON2bits.RSEN
#define P SSPSTATbits.P
#define S SSPSTATbits.S
#define DA_ SSPSTATbits.D_A
#define BCLIE PIE2bits.BCLIE 
#define TMR0IF INTCONbits.TMR0IF
#define T0SE T0CONbits.T0SE
#define TMR0ON T0CONbits.TMR0ON
#define TMR0IE INTCONbits.TMR0IE
#define TMR1IF PIR1bits.TMR1IF
#define INT0IF INTCONbits.INT0IF
#define INT0IE INTCONbits.INT0IE
#define GIE INTCONbits.GIE 
#define POR_ RCONbits.POR
#define PEIE INTCONbits.PEIE
#define RBIF INTCONbits.RBIF
#define TMR1IE PIE1bits.TMR1IE
#define IDLEN OSCCONbits.IDLEN 
#define TXEN TXSTAbits.TXEN
#define SMP SSPSTATbits.SMP
#define CKE SSPSTATbits.CKE
#define WCOL SSPCON1bits.WCOL
#define SSPEN SSPCON1bits.SSPEN
#define Carry STATUSbits.C
#define T08BIT T0CONbits.T08BIT
#define PLLEN OSCTUNEbits.PLLEN
#define INTEDG0 INTCON2bits.INTEDG0
#define RD EECON1bits.RD
#define WREN EECON1bits.WREN
#define WR EECON1bits.WR
#define TX9 TXSTAbits.TX9
#define RX9 RCSTAbits.RX9
#define SPEN RCSTAbits.SPEN
#define BRGH TXSTAbits.BRGH
#define SYNC TXSTAbits.SYNC
#define ADDEN RCSTAbits.ADDEN
#define SSPIE PIE1bits.SSPIE
#define nop()    {_asm nop _endasm}
//#define ClrWdt() {_asm clrwdt _endasm}
#define sleep()  {_asm sleep _endasm}
#define seset()  {_asm reset _endasm}
#define FSR_REGISTER ptr_FSR
unsigned char *ptr_FSR;
#define PTR_FSR (*ptr_FSR)
#else
//#include "18f25fk20.h"
#include "int18XXX.H"
char PORTE    @ 0xF84;
#define FSR_REGISTER FSR0
#define PTR_FSR INDF0
#endif
#define RAM_BANK_0 0
#define RAM_BANK_1 1
#define RAM_BANK_2 2
#define RAM_BANK_3 3
#define RAM_BANK_4 4
#define RAM_BANK_5 5
#define TIMER0_CONTROL_REG T0CON
#define TIMER0_BYTE TMR0L
#define TIMER0_INT_ENBL TMR0IE
#define TIMER0_INT_FLG  TMR0IF

#define INT0_EDG INTEDG0
#define INT0_FLG INT0IF
#define INT0_ENBL INT0IE
#define I2CPORT PORTC
#define I2CTRIS TRISC
#define I2C_SDA 4
#define I2C_SCL 3
#define _TRMT TRMT
#endif
//////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifdef _18F2321
#undef _Q_PROCESS
#ifdef __18CXX
//char PORTE    @ 0xF84;
#include "p18f2321.h"
#define RCIF PIR1bits.RCIF
#define RCIE PIE1bits.RCIE
#define CREN RCSTAbits.CREN
#define TXIE PIE1bits.TXIE
#define TXIF PIR1bits.TXIF
#define TRMT TXSTAbits.TRMT
#define BCLIF PIR2bits.BCLIF
#define SEN SSPCON2bits.SEN
#define PEN SSPCON2bits.PEN
#define SSPIF PIR1bits.SSPIF
#define SSPOV SSPCON1bits.SSPOV
#define BF SSPSTATbits.BF
#define ACKEN SSPCON2bits.ACKEN
#define ACKDT SSPCON2bits.ACKDT
#define RCEN SSPCON2bits.RCEN
#define RSEN SSPCON2bits.RSEN
#define P SSPSTATbits.P
#define S SSPSTATbits.S
#define DA_ SSPSTATbits.D_A
#define BCLIE PIE2bits.BCLIE 
#define TMR0IF INTCONbits.TMR0IF
#define T0SE T0CONbits.T0SE
#define TMR0ON T0CONbits.TMR0ON
#define TMR0IE INTCONbits.TMR0IE
#define TMR1IF PIR1bits.TMR1IF
#define INT0IF INTCONbits.INT0IF
#define INT0IE INTCONbits.INT0IE
#define GIE INTCONbits.GIE 
#define POR_ RCONbits.POR
#define PEIE INTCONbits.PEIE
#define RBIF INTCONbits.RBIF
#define TMR1IE PIE1bits.TMR1IE
#define IDLEN OSCCONbits.IDLEN 
#define TXEN TXSTAbits.TXEN
#define SMP SSPSTATbits.SMP
#define CKE SSPSTATbits.CKE
#define WCOL SSPCON1bits.WCOL
#define SSPEN SSPCON1bits.SSPEN
#define Carry STATUSbits.C
#define T08BIT T0CONbits.T08BIT
#define PLLEN OSCTUNEbits.PLLEN
#define INTEDG0 INTCON2bits.INTEDG0
#define RD EECON1bits.RD
#define WREN EECON1bits.WREN
#define WR EECON1bits.WR
#define TX9 TXSTAbits.TX9
#define RX9 RCSTAbits.RX9
#define SPEN RCSTAbits.SPEN
#define BRGH TXSTAbits.BRGH
#define SYNC TXSTAbits.SYNC
#define ADDEN RCSTAbits.ADDEN
#define SSPIE PIE1bits.SSPIE
#define nop()    {_asm nop _endasm}
//#define ClrWdt() {_asm clrwdt _endasm}
#define sleep()  {_asm sleep _endasm}
#define seset()  {_asm reset _endasm}
#define FSR_REGISTER ptr_FSR
unsigned char *ptr_FSR;
#define PTR_FSR (*ptr_FSR)
#else
//#include "18f2321.h"
#include "int18XXX.H"
char PORTE    @ 0xF84;
#define FSR_REGISTER FSR0
#define PTR_FSR INDF0
#endif
#define RAM_BANK_0 0
#define RAM_BANK_1 1
#define RAM_BANK_2 0
#define RAM_BANK_3 1
#define RAM_BANK_4 0
#define RAM_BANK_5 1
#define TIMER0_CONTROL_REG T0CON
#define TIMER0_BYTE TMR0L
#define TIMER0_INT_ENBL TMR0IE
#define TIMER0_INT_FLG  TMR0IF

#define INT0_EDG INTEDG0
#define INT0_FLG INT0IF
#define INT0_ENBL INT0IE
#define I2CPORT PORTC
#define I2CTRIS TRISC
#define I2C_SDA 4
#define I2C_SCL 3
#define _TRMT TRMT
#undef UWORD  
#define UWORD unsigned long
#endif
//////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifdef __PIC24H__
#undef WORD
#ifdef HI_TECH_C
   #include "htc.h"
   #include "pic24HJ64GP502.h"
#else // end of HI_TECH
   #include "p24hxxxx.h"
   #ifdef __PIC24HJ64GP502__
      #include "p24HJ64GP502.h"
   #endif
   #ifdef __PIC24HJ64GP504__
      #include "p24HJ64GP504.h"
   #endif
   #include "rtcc.h"

   #include "pps.h"
   #include "string.h"
#endif
// bugs in C30 compiler!! this can create unpredictable code
#define VOLATILE volatile 
//#define VOLATILE
#define SSPIF IFS1bits.MI2C1IF
#define BF I2C1STATbits.RBF
//#define BF bittest(I2C1STAT,2)
#define ACKEN I2C1CONbits.ACKEN
#define DA_ I2C1STATbits.D_A
#define SSPBUF_RX I2C1RCV
#define SSPBUF_TX I2C1TRN
#define P I2C1STATbits.P
#define ACKDT I2C1CONbits.ACKDT
#define RCEN I2C1CONbits.RCEN
#define ACKSTAT I2C1STATbits.ACKSTAT
#define SSPADD I2C1BRG
#define I2C_BRG I2C1BRG
#define RSEN I2C1CONbits.RSEN
#define PEN I2C1CONbits.PEN
#define S I2C1STATbits.S
#define BCLIF I2C1STATbits.BCL
#define SSPOV I2C1STATbits.I2COV
#define RCIE IEC0bits.U1RXIE
#define RCIF IFS0bits.U1RXIF
#define RCSTA U1STA
#define RCREG U1RXREG
#define _TRMT U1STAbits.TRMT
#define TXREG U1TXREG
#define TXIE IEC0bits.U1TXIE

#ifdef USE_COM2
#define RCIECOM2 IEC1bits.U2RXIE
#define RCIFCOM2 IFS1bits.U2RXIF
#define RCSTACOM2 U2STA
#define RCREGCOM2 U2RXREG
#define TRMTCOM2 U2STAbits.TRMT
#define TXREGCOM2 U2TXREG
#define TXIECOM2 IEC1bits.U2TXIE
#define TXENCOM2 U2STAbits.UTXEN
#define TXIFCOM2 IFS1bits.U2TXIF
#endif

#define TIMER0_INT_FLG IFS0bits.T1IF
#define TIMER0_INT_ENBL IEC0bits.T1IE
#define TMR0ON T1CONbits.TON
#define TMR1IF IFS0bits.T2IF
#define TMR1IE IEC0bits.T2IE
#define TMR1ON T2CONbits.TON
#define INT0_FLG IFS0bits.INT0IF
#define INT0_ENBL IEC0bits.INT0IE
#define INT1IF IFS1bits.INT1IF
#define INT1IE IEC1bits.INT1IE
#define INT2IF IFS1bits.INT2IF
#define INT2IE IEC1bits.INT2IE
#define POR_ RCONbits.POR
#define FSR_REGISTER ptr_FSR
unsigned char *ptr_FSR;
#define PTR_FSR (*ptr_FSR)
#define TXEN U1STAbits.UTXEN
#define TXIF IFS0bits.U1TXIF
#define WCOL I2C1STATbits.IWCOL
#define SEN I2C1CONbits.SEN
#define TIMER0_BYTE TMR1
#define INT0_EDG INTCON2bits.INT0EP
#define INTEDG1 INTCON2bits.INT1EP
#define INTEDG2 INTCON2bits.INT2EP
#define Carry SRbits.C
#ifdef HI_TECH_C
#define nop() asm("nop");
#define __builtin_write_OSCCONH(arg) OSCCONH=arg;
#define __builtin_write_OSCCONL(arg) OSCCONL=arg;
#else
#define nop Nop
#endif
    #ifndef WORD
    #define WORD unsigned int
    #endif
#endif
//////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////


#ifdef SSPORT
#ifdef FLASH_POWER_DOWN
void CsLow(void);
void CsHigh(void);
#define CS_LOW  CsLow(); // set low Chip Select
#define CS_HIGH CsHigh(); // set high Chip Select
#else
#define CS_LOW  bclr(SSPORT,SSCS); // set low Chip Select
#define CS_HIGH bset(SSPORT,SSCS); // set high Chip Select
#endif
#endif 


#define _NOT_SIMULATOR 1
#define _OPTIMIZED_ 1



#ifdef _18F2321_18F25K20
//#undef _OPTIMIZED_
//#define DELAY_I2C 14
//#define DELAY_1_I2C W = DELAY_I2C; while(--W);
#define DELAY_START_I2C nop();nop();nop();
#define DELAY_1_I2C nop();nop();nop();

// on 18F232l in Fosc=32MHz Fcy= 8MIPS BRG=0x05 for 800kHz I2C operation
// on 18F232l in Fosc=32MHz Fcy= 8MIPS BRG=0x09 for 800kHz I2C operation
//#define FAST_DELAY 0x5
//#define SLOW_DELAY 0x9

// on 18F232l in Fosc=32MHz Fcy= 8MIPS BRG=0x1f for 250kHz I2C operation
#define FAST_DELAY 0x1f
#define SLOW_DELAY 0x1f

// on 18F232l in Fosc=32MHz Fcy = 8MIPS BRG=0x12 for 400kHz I2C operation
//#define FAST_DELAY 0x13
//#define SLOW_DELAY 0x13


      //nop();nop();nop();nop();nop();nop();nop();nop();nop();
#else
   #ifdef __PIC24H__
#define FAST_DELAY 98
#define SLOW_DELAY 98 
   #else
// on 16LF88 in Fosc=8MHz Fcy= 2MIPS 3 nop() = 666kHz
#define DELAY_START_I2C nop();nop();nop();
#define DELAY_1_I2C nop();nop();nop();

// on 16LF884 in Fosc=8MHz Fcy= 2MIPS BRG=0x06 for 285kHz I2C operation
// on 16LF884 in Fosc=8MHz Fcy= 2MIPS BRG=0x0a for 181kHz I2C operation
#define FAST_DELAY 0x6
#define SLOW_DELAY 0xa 
   #endif
#endif

////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _16F88
///////////////////////////////////////////////////////////////////////////////////////////
// this line for debug and may be normal operations
// on pin 15 (RA6) no oscilator output  
#pragma config PWRTE=off, WDTE=off, FOSC=4, BODEN=off
//#pragma config PWRTE=on, WDTE=off, FOSC=4, BODEN=off
//#define DEBUG_OSC_PIN15 1

// this line for pin 15 (RA6) oscilator output 2MHz:
//#pragma config PWRTE=off, WDTE=off, FOSC=5, BODEN=off
//#pragma config |= 0x2f20
// for debuging by pickit 3
#pragma config |= 0x2720
// 0010 1111 0010 0000
//   1                = 1 = Code protection off
//    0               = 0 = CCP1 function on RB3
//      1             = 1 = In-Circuit Debugger disabled, RB6 and RB7 are general purpose I/O pins
//       11           = 11 = Write protection off
//         1          = 1 = Code protection off;
//           0        = 1 = RB3/PGM pin has PGM function, Low-Voltage Programming enabled
//            0       = 0 = BOR disabled
//             1      = 0 = RA5/MCLR/VPP pin function is digital I/O, MCLR internally tied to VDD
//                0   = 0 = PWRT enabled
//                 0  = 0 = WDT disabled
//              1   00= FOSC=4 =100 = INTRC oscillator; port I/O function on both RA6/OSC2/CLKO pin and RA7/OSC1/CLKI pin
// bit 13 CP: Flash Program Memory Code Protection bits
//            1 = Code protection off; 0 = 0000h to 0FFFh code-protected (all protected)
// bit 12 CCPMX: CCP1 Pin Selection bit
//            1 = CCP1 function on RB0; 0 = CCP1 function on RB3
// bit 11 DEBUG: In-Circuit Debugger Mode bit
//            1 = In-Circuit Debugger disabled, RB6 and RB7 are general purpose I/O pins
//            0 = In-Circuit Debugger enabled, RB6 and RB7 are dedicated to the debugger
// bit 10-9 WRT<1:0>: Flash Program Memory Write Enable bits
//           11 = Write protection off
//           10 = 0000h to 00FFh write-protected, 0100h to 0FFFh may be modified by EECON control
//           01 = 0000h to 07FFh write-protected, 0800h to 0FFFh may be modified by EECON control
//           00 = 0000h to 0FFFh write-protected
// bit 8 CPD: Data EE Memory Code Protection bit
//            1 = Code protection off; 0 = Data EE memory code-protected
// bit 7 LVP: Low-Voltage Programming Enable bit
//            1 = RB3/PGM pin has PGM function, Low-Voltage Programming enabled
//            0 = RB3 is digital I/O, HV on MCLR must be used for programming
// bit 6 BOREN: Brown-out Reset Enable bit
//            1 = BOR enabled ; 0 = BOR disabled
// bit 5 MCLRE: RA5/MCLR/VPP Pin Function Select bit
//            1 = RA5/MCLR/VPP pin function is MCLR ; 0 = RA5/MCLR/VPP pin function is digital I/O, MCLR internally tied to VDD
// bit 3 PWRTEN: Power-up Timer Enable bit
//            1 = PWRT disabled; 0 = PWRT enabled
// bit 2 WDTEN: Watchdog Timer Enable bit
//            1 = WDT enabled ; 0 = WDT disabled
// bit 4, 1-0 FOSC<2:0>: Oscillator Selection bits
//         111 = EXTRC oscillator; CLKO function on RA6/OSC2/CLKO
//         110 = EXTRC oscillator; port I/O function on RA6/OSC2/CLKO
//         101 = INTRC oscillator; CLKO function on RA6/OSC2/CLKO pin and port I/O function on RA7/OSC1/CLKI pin
//         100 = INTRC oscillator; port I/O function on both RA6/OSC2/CLKO pin and RA7/OSC1/CLKI pin
//         011 = ECIO; port I/O function on RA6/OSC2/CLKO
//         010 = HS oscillator
//         001 = XT oscillator
//         000 = LP oscillator
#pragma config reg2 = 0
#endif
//////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifdef _16F884
/////////////////////////////////////////////////////////////////////////////////////////
// this line for debug and may be normal operations
// on pin 15 (RA6) no oscilator output  
//#pragma config PWRTE=on, WDTE=off, FOSC=4, BODEN=off
//#define DEBUG_OSC_PIN15 1

// this line for pin 15 (RA6) oscilator output 2MHz
#pragma config PWRTE=off, WDTE=off
//#pragma config |= 0x30f4
// for debuging by pickit 3
#pragma config |= 0x10f4
// 0001 0000 1110 0000
// 00                  == not in use
//   1                 = 1 = In-Circuit Debugger disabled, RB6 and RB7 are general purpose I/O pins
//    1                == LVP (1) = RB3/PGM pin has PGM function, Low-Voltage Programming enabled, (0) disabled
//      0              == FCMEN Fail-Safe Clock Monitor (1) Enabled (0) disabled
//       0             == IESO Internal - external Swithcover bit (1) eabled (0) disabled
//        00           == BOREN - brown-out reset selection
//                       11 - BOR enable
//                       10 - BOR enabled at operation and disabled at sleep
//                       01 - BOR controlled by SBOREN bit in PCON register
//                       00 - BOR disabled
//           1         == CPD (data code protection (1) disabled (0) enabled
//            1        == CP Data Code protection (1) program memory code protection disabled (0) enabled
//             1       == MCLRE - (1) RE3 is MCLR is on (0) RE3 - digital input
//              1      == PWRTE - (1) power up timer disabled (0) powerup timer enabled
//                0    == WTDE watchdog timer (1) enabled (0) disabled
//                 100 =100 = INTRC oscillator; port I/O function on both RA6/OSC2/CLKO pin and RA7/OSC1/CLKI pin
// bit 13 DEBUG: In-Circuit Debugger Mode bit
//            1 = In-Circuit Debugger disabled, RB6 and RB7 are general purpose I/O pins
//            0 = In-Circuit Debugger enabled, RB6 and RB7 are dedicated to the debugger
// bit 12 LVP: Low-Voltage Programming Enable bit
//            1 = RB3/PGM pin has PGM function, Low-Voltage Programming enabled
//            0 = RB3 is digital I/O, HV on MCLR must be used for programming
// bit 11 FCMEN
// bit 10 IESO
// bit 9-8 BOREN: Brown-out Reset Enable bits
// bit 7   CPD Data EE Memory Code Protection bit
//            1 = Code protection off; 0 = Data EE memory code-protected
// bit 6 CP: Flash Program Memory Code Protection bits
//            1 = Code protection off; 0 = 0000h to 0FFFh code-protected (all protected)
// bit 5 MCLRE: RA5/MCLR/VPP Pin Function Select bit
//            1 = RE3/MCLR/VPP pin function is MCLR ; 0 = RE3/MCLR/VPP pin function is digital I/O, MCLR internally tied to VDD
// bit 4 PWRTEN: Power-up Timer Enable bit
//            1 = PWRT disabled; 0 = PWRT enabled
// bit 3 WDTEN: Watchdog Timer Enable bit
//            1 = WDT enabled ; 0 = WDT disabled
// bit 2-0: Oscillator Selection bits
//         111 = RC oscillator; CLKO function on RA6/OSC2/CLKO, RC on RA7
//         110 = RCIO oscillator; port I/O function on RA6/OSC2/CLKO, RC on RA7
//         101 = INTRC oscillator; CLKOUT function on RA6/OSC2/CLKO pin and port I/O function on RA7/OSC1/CLKI pin
//         100 = INTRC oscillator; port I/O function on both RA6/OSC2/CLKO pin and RA7/OSC1/CLKI pin
//         011 = EC: port I/O function on RA6/OSC2/CLKO, CLKIN on RA7
//         010 = HS oscillator; Highspeed cristal/resonator on RA6 and RA7
//         001 = XT oscillator; Crystal/resonator on RA6 and RA7
//         000 = LP oscillator; low power crystal on RA6 and RA7
#pragma config reg2 = 0x0700
// CONFIG2
//  bit 10-9  WRT Flash program memory self write enable bit
//  bit 8     BOR4V  (1) brownout reset 4.0v (1_ brownout reset 2.1V
// 0000 0111 0000 0000
#endif

//////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifdef _16F724
/////////////////////////////////////////////////////////////////////////////////////////
// this line for debug and may be normal operations
// on pin 15 (RA6) no oscilator output  
//#pragma config PWRTE=on, WDTE=off, FOSC=4, BODEN=off
//#define DEBUG_OSC_PIN15 1

// this line for pin 15 (RA6) oscilator output 2MHz
#pragma config PWRTE=off, WDTE=off
//#pragma config |= 0x30f4
// for debuging by pickit 3
#pragma config |= 0x10f4
// 0001 0000 1110 0000
// 00                  == not in use
//   1                 = 1 = In-Circuit Debugger disabled, RB6 and RB7 are general purpose I/O pins
//    1                == LVP (1) = RB3/PGM pin has PGM function, Low-Voltage Programming enabled, (0) disabled
//      0              == FCMEN Fail-Safe Clock Monitor (1) Enabled (0) disabled
//       0             == IESO Internal - external Swithcover bit (1) eabled (0) disabled
//        00           == BOREN - brown-out reset selection
//                       11 - BOR enable
//                       10 - BOR enabled at operation and disabled at sleep
//                       01 - BOR controlled by SBOREN bit in PCON register
//                       00 - BOR disabled
//           1         == CPD (data code protection (1) disabled (0) enabled
//            1        == CP Data Code protection (1) program memory code protection disabled (0) enabled
//             1       == MCLRE - (1) RE3 is MCLR is on (0) RE3 - digital input
//              1      == PWRTE - (1) power up timer disabled (0) powerup timer enabled
//                0    == WTDE watchdog timer (1) enabled (0) disabled
//                 100 =100 = INTRC oscillator; port I/O function on both RA6/OSC2/CLKO pin and RA7/OSC1/CLKI pin
// bit 13 DEBUG: In-Circuit Debugger Mode bit
//            1 = In-Circuit Debugger disabled, RB6 and RB7 are general purpose I/O pins
//            0 = In-Circuit Debugger enabled, RB6 and RB7 are dedicated to the debugger
// bit 12 LVP: Low-Voltage Programming Enable bit
//            1 = RB3/PGM pin has PGM function, Low-Voltage Programming enabled
//            0 = RB3 is digital I/O, HV on MCLR must be used for programming
// bit 11 FCMEN
// bit 10 IESO
// bit 9-8 BOREN: Brown-out Reset Enable bits
// bit 7   CPD Data EE Memory Code Protection bit
//            1 = Code protection off; 0 = Data EE memory code-protected
// bit 6 CP: Flash Program Memory Code Protection bits
//            1 = Code protection off; 0 = 0000h to 0FFFh code-protected (all protected)
// bit 5 MCLRE: RA5/MCLR/VPP Pin Function Select bit
//            1 = RE3/MCLR/VPP pin function is MCLR ; 0 = RE3/MCLR/VPP pin function is digital I/O, MCLR internally tied to VDD
// bit 4 PWRTEN: Power-up Timer Enable bit
//            1 = PWRT disabled; 0 = PWRT enabled
// bit 3 WDTEN: Watchdog Timer Enable bit
//            1 = WDT enabled ; 0 = WDT disabled
// bit 2-0: Oscillator Selection bits
//         111 = RC oscillator; CLKO function on RA6/OSC2/CLKO, RC on RA7
//         110 = RCIO oscillator; port I/O function on RA6/OSC2/CLKO, RC on RA7
//         101 = INTRC oscillator; CLKOUT function on RA6/OSC2/CLKO pin and port I/O function on RA7/OSC1/CLKI pin
//         100 = INTRC oscillator; port I/O function on both RA6/OSC2/CLKO pin and RA7/OSC1/CLKI pin
//         011 = EC: port I/O function on RA6/OSC2/CLKO, CLKIN on RA7
//         010 = HS oscillator; Highspeed cristal/resonator on RA6 and RA7
//         001 = XT oscillator; Crystal/resonator on RA6 and RA7
//         000 = LP oscillator; low power crystal on RA6 and RA7
#pragma config reg2 = 0x0700
// CONFIG2
//  bit 10-9  WRT Flash program memory self write enable bit
//  bit 8     BOR4V  (1) brownout reset 4.0v (1_ brownout reset 2.1V
// 0000 0111 0000 0000
#endif

//////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifdef _18F2321
#ifdef __18CXX
#include "p18f2321.h"
#define rambank varlocate
#endif
///////////////////////////////////////////////////////////////////////////////////////////
// this line for debug and may be normal operations
// on pin 15 (RA6) no oscilator output  
//#pragma config PWRTE=on, WDTE=off, FOSC=4, BODEN=off
//#define DEBUG_OSC_PIN15 1

// this line for pin 15 (RA6) oscilator output 2MHz
//#pragma config PWRTE=off, WDTE=off, FOSC=5, BODEN=off
//#pragma config |= 0x2f20
// for debuging by pickit 3
//#pragma config |= 0x27
// CONFIG1H: CONFIGURATION REGISTER 1 HIGH (BYTE ADDRESS 300001h)
// bit 7 IESO: Internal/External Oscillator Switchover bit
// 1         = Oscillator Switchover mode enabled
// 0         = Oscillator Switchover mode disabled
// bit 6 FCMEN: Fail-Safe Clock Monitor Enable bit
//  1        = Fail-Safe Clock Monitor enabled
//  0        = Fail-Safe Clock Monitor disabled
// bit 5-4 Unimplemented: Read as ‘0’
//   00
// bit 3-0 FOSC<3:0>: Oscillator Selection bits
//      11xx = External RC oscillator, CLKO function on RA6
//      101x = External RC oscillator, CLKO function on RA6
//      1001 = Internal oscillator block, CLKO function on RA6, port function on RA7
//      1000 = Internal oscillator block, port function on RA6 and RA7
//      0111 = External RC oscillator, port function on RA6
//      0110 = HS oscillator, PLL enabled (Clock Frequency = 4 x FOSC1)
//      0101 = EC oscillator, port function on RA6
//      0100 = EC oscillator, CLKO function on RA6
//      0011 = External RC oscillator, CLKO function on RA6
//      0010 = HS oscillator
//      0001 = XT oscillator
//      0000 = LP oscillator
// 0000 1000
#ifdef __18CXX
#pragma config OSC=INTIO2
#else
#pragma config[1] = 0x08
#endif
// CONFIG2L: CONFIGURATION REGISTER 2 LOW (BYTE ADDRESS 300002h)
//  bit 7-5 Unimplemented: Read as ‘0’
// 000
//  bit 4-3 BORV<1:0>: Brown-out Reset Voltage bits(1)
//    1 1   = Minimum setting 
//    0 0   = Maximum setting
// bit 2-1 BOREN<1:0>: Brown-out Reset Enable bits(2)
//       11 = Brown-out Reset enabled in hardware only (SBOREN is disabled)
//       10 = Brown-out Reset enabled in hardware only and disabled in Sleep mode(SBOREN is disabled)
//       01 = Brown-out Reset enabled and controlled by software (SBOREN is enabled)
//       00 = Brown-out Reset disabled in hardware and software
// bit 0 PWRTEN: Power-up Timer Enable bit(2)
//         1 = PWRT disabled 0 = PWRT enabled
//
// 0001 1000
#ifdef __18CXX
#pragma config BOR=OFF
#else
#pragma config[2] = 0x18
#endif
// CONFIG2H: CONFIGURATION REGISTER 2 HIGH (BYTE ADDRESS 300003h)
// bit 7-5 Unimplemented: Read as ‘0’
// bit 4-1 WDTPS<3:0>: Watchdog Timer Postscale Select bits
//    1111 = 1:32,768  1110 = 1:16,384   1101 = 1:8,192 1100 = 1:4,096
//    1011 = 1:2,048   1010 = 1:1,024    1001 = 1:512   1000 = 1:256
//    0111 = 1:128     0110 = 1:64       0101 = 1:32    0100 = 1:16
//    0011 = 1:8       0010 = 1:4        0001 = 1:2     0000 = 1:1
// bit 0 WDTEN: Watchdog Timer Enable bit 1 = WDT enabled 0 = WDT disabled (control is placed on the SWDTEN bit)
// 0000 0000
#ifdef __18CXX
#pragma config WDT=OFF
#else
#pragma config[3] = 0x00
#endif
//  CONFIG3H: CONFIGURATION REGISTER 3 HIGH (BYTE ADDRESS 300005h)
// bit 7 MCLRE: MCLR Pin Enable bit
// 1        = MCLR pin enabled; RE3 input pin disabled 
// 0        = RE3 input pin enabled; MCLR disabled
// bit 6-3 Unimplemented: Read as ‘0’
//  000 0
// bit 2 LPT1OSC: Low-Power Timer1 Oscillator Enable bit 
//       1   = Timer1 configured for low-power operation
//       0   = Timer1 configured for higher power operation
// bit 1 PBADEN: PORTB A/D Enable bit (Affects ADCON1 Reset state. ADCON1 controls PORTB<4:0> pin configuration.)
//        1  = PORTB<4:0> pins are configured as analog input channels on Reset
//        0  = PORTB<4:0> pins are configured as digital I/O on Reset
// bit 0 CCP2MX: CCP2 MUX bit 
//         1 = CCP2 input/output is multiplexed with RC1 
//         0 = CCP2 input/output is multiplexed with RB3
// 1000 0001


#ifdef __18CXX
   //#pragma config MCLRE=ON, CCP2MX=ON
   #pragma config MCLRE=ON
#else
   #pragma config[5] = 0x81
#endif
// CONFIG4L: CONFIGURATION REGISTER 4 LOW (BYTE ADDRESS 300006h)
// bit 7 DEBUG: Background Debugger Enable bit
//  1        = Background debugger disabled, RB6 and RB7 configured as general purpose I/O pins
//  0        = Background debugger enabled, RB6 and RB7 are dedicated to in-circuit debug
// bit 6 XINST: Extended Instruction Set Enable bit
//   1       = Instruction set extension and Indexed Addressing mode enabled
//   0       = Instruction set extension and Indexed Addressing mode disabled (Legacy mode)
// bit 5-4 BBSIZ<1:0>: Boot Block Size Select bits
//         PIC18F4221/4321 Devices:
//    1x     = 1024 Words
//    01     = 512 Words
//    00     = 256 Words
//         PIC18F2221/2321 Devices:
//    1x     = 512 Words
//    x1     = 512 Words
//    00     = 256 Words
// bit 3 Reserved: Maintain as ‘0’
// bit 2 LVP: Single-Supply ICSP™ Enable bit 
//       1   = Single-Supply ICSP enabled 
//       0   = Single-Supply ICSP disabled
// bit 1 Unimplemented: Read as ‘0’
//        0
// bit 0 STVREN: Stack Full/Underflow Reset Enable bit 
//         1 = Stack full/underflow will cause Reset
//         0 = Stack full/underflow will not cause Reset
// 0100 0100
#ifdef __DEBUG
    #ifdef __18CXX
        #pragma config XINST=ON, LVP=ON
    #else
        #pragma config[6] = 0x00
    #endif
#else
    #ifdef __18CXX
        #pragma config XINST=ON, LVP=ON
    #else
        #pragma config[6] = 0x80
    #endif
#endif
// CONFIG5L: CONFIGURATION REGISTER 5 LOW (BYTE ADDRESS 300008h)
// bit 7-2 Unimplemented: Read as ‘0’
// bit 1 CP1: Code Protection bit 
//        1  = Block 1 not code-protected 
//        0  = Block 1 code-protected(1)
// bit 0 CP0: Code Protection bit 
//         1 = Block 0 not code-protected 
//         0 = Block 0 code-protected(1)
// 0000 0011
#ifdef __18CXX
#pragma config CP0=OFF, CP1=OFF
#else
#pragma config[8] = 0x03
#endif
// CONFIG5H: CONFIGURATION REGISTER 5 HIGH (BYTE ADDRESS 300009h)
// bit 7 CPD: Data EEPROM Code Protection bit 
// 1          = Data EEPROM not code-protected 
// 0          = Data EEPROM code-protected
// bit 6 CPB: Boot Block Code Protection bit 
//  1         = Boot block not code-protected 
//  0         = Boot block code-protected(1)
// bit 5-0 Unimplemented: Read as ‘0’
// 1100 0000
#ifdef __18CXX
#pragma config CPD=OFF, CPB=OFF
#else
#pragma config[9] = 0xc0
#endif
// CONFIG6L: CONFIGURATION REGISTER 6 LOW (BYTE ADDRESS 30000Ah)
// bit 7-2 Unimplemented: Read as ‘0’
// bit 1 WRT1: Write Protection bit 
//         1  = Block 1 not write-protected 
//         0  = Block 1 write-protected
// bit 0 WRT0: Write Protection bit 
//          1 = Block 0 not write-protected 
//          0 = Block 0 write-protected
// 00000 0011
#ifdef __18CXX
#pragma config WRT1=OFF, WRT0=OFF
#else
#pragma config[0xa] = 0x03
#endif
// CONFIG6H: CONFIGURATION REGISTER 6 HIGH (BYTE ADDRESS 30000Bh)
// bit 7 WRTD: Data EEPROM Write Protection bit 
// 1        = Data EEPROM not write-protected 
// 0        = Data EEPROM write-protected
// bit 6 WRTB: Boot Block Write Protection bit 
//  1       = Boot block not write-protected 
//  0       = Boot block write-protected(2)
// bit 5 WRTC: Configuration Register Write Protection bit 
//   1      = Configuration registers (300000-3000FFh) not write-protected 
//   0      = Configuration registers (300000-3000FFh) write-protected
// bit 4-0 Unimplemented: Read as ‘0’
//    0 0000
// 1110 0000
#ifdef __18CXX
#pragma config WRTD=OFF, WRTB=OFF, WRTC=OFF
#else
#pragma config[0xb] = 0xe3
#endif
//
// CONFIG7L: CONFIGURATION REGISTER 7 LOW (BYTE ADDRESS 30000Ch)
// bit 7-2 Unimplemented: Read as ‘0’
// bit 1 EBTR1: Table Read Protection bit 
//        1  = Block 1 not protected from table reads executed in other blocks 
//        0  = Block 1 protected from table reads executed in other blocks
// bit 0 EBTR0: Table Read Protection bit 
//         1 = Block 0 not protected from table reads executed in other blocks 
//         0 = Block 0 protected from table reads executed in other blocks
// 0000 0011
#ifdef __18CXX
#pragma config EBTR1=OFF, EBTR0=OFF
#else
#pragma config[0xc] = 0x03
#endif
// CONFIG7H: CONFIGURATION REGISTER 7 HIGH (BYTE ADDRESS 30000Dh)
// bit 7 Unimplemented: Read as ‘0’
// bit 6 EBTRB: Boot Block Table Read Protection bit 
//  1      = Boot block not protected from table reads executed in other blocks 
//  0      = Boot block protected from table reads executed in other blocks(1)
// bit 5-0 Unimplemented: Read as ‘0’
// 0100 0000
#ifdef __18CXX
#pragma config EBTRB=OFF
#else
#pragma config[0xd] = 0x40
#endif
#endif
//////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifdef _18F25K20
#ifdef __18CXX
#include "p18f25k20.h"
#define rambank varlocate
#endif
///////////////////////////////////////////////////////////////////////////////////////////
// this line for debug and may be normal operations
// on pin 15 (RA6) no oscilator output  
//#pragma config PWRTE=on, WDTE=off, FOSC=4, BODEN=off
//#define DEBUG_OSC_PIN15 1

// this line for pin 15 (RA6) oscilator output 2MHz
//#pragma config PWRTE=off, WDTE=off, FOSC=5, BODEN=off
//#pragma config |= 0x2f20
// for debuging by pickit 3
//#pragma config |= 0x27
// CONFIG1H: CONFIGURATION REGISTER 1 HIGH (BYTE ADDRESS 300001h)
// bit 7 IESO: Internal/External Oscillator Switchover bit
//   1         = Oscillator Switchover mode enabled
//   0         = Oscillator Switchover mode disabled
// bit 6 FCMEN: Fail-Safe Clock Monitor Enable bit
//    1        = Fail-Safe Clock Monitor enabled
//    0        = Fail-Safe Clock Monitor disabled
// bit 5-4 Unimplemented: Read as ‘0’
// bit 3-0 FOSC<3:0>: Oscillator Selection bits
//        11xx = External RC oscillator, CLKOUT function on RA6
//        101x = External RC oscillator, CLKOUT function on RA6
//        1001 = Internal oscillator block, CLKOUT function on RA6, port function on RA7
//        1000 = Internal oscillator block, port function on RA6 and RA7
//        0111 = External RC oscillator, port function on RA6
//        0110 = HS oscillator, PLL enabled (Clock Frequency = 4 x FOSC1)
//        0101 = EC oscillator, port function on RA6
//        0100 = EC oscillator, CLKOUT function on RA6
//        0011 = External RC oscillator, CLKOUT function on RA6
//        0010 = HS oscillator
//        0001 = XT oscillator
//        0000 = LP oscillator
//   0000 0110
#ifdef __18CXX
#pragma config OSC=INTIO2
#else
#pragma config[1] = 0x08
#endif
// CONFIG2L: CONFIGURATION REGISTER 2 LOW (BYTE ADDRESS 300002h)
// bit 7-5 Unimplemented: Read as ‘0’
//    000
// bit 4-3 BORV<1:0>: Brown-out Reset Voltage bits(1)
//       11    = VBOR set to 1.8V nominal
//       10    = VBOR set to 2.2V nominal
//       01    = VBOR set to 2.7V nominal
//       00    = VBOR set to 3.0V nominal
// bit 2-1 BOREN<1:0>: Brown-out Reset Enable bits(2)
//          11 = Brown-out Reset enabled in hardware only (SBOREN is disabled)
//          10  = Brown-out Reset enabled in hardware only and disabled in Sleep mode 
//                   (SBOREN is disabled) 
//          01  = Brown-out Reset enabled and controlled by software (SBOREN is enabled) 
//          00  = Brown-out Reset disabled in hardware and software
// bit 0 PWRTEN: Power-up Timer Enable bit(2)
//            1 = PWRT disabled 
//            0 = PWRT enabled 
//    0001 1000
#ifdef __18CXX
#pragma config BOR=OFF
#else
#pragma config[2] = 0x10
#endif
// CONFIG2H: CONFIGURATION REGISTER 2 HIGH (BYTE ADDRESS 300003h)
// bit 7-5      Unimplemented: Read as ‘0’
// 000
// bit 4-1 WDTPS<3:0>: Watchdog Timer Postscale Select bits
//    1111 = 1:32,768    1110 = 1:16,384    1101 = 1:8,192    1100 = 1:4,096
//    1011 = 1:2,048     1010 = 1:1,024     1001 = 1:512      1000 = 1:256
//    0111 = 1:128       0110 = 1:64        0101 = 1:32       0100 = 1:16
//    0011 = 1:8         0010 = 1:4         0001 = 1:2
//    0000 = 1:1
// bit 0 WDTEN: Watchdog Timer Enable bit
//         1 = WDT is always enabled. SWDTEN bit has no effect
//         0 = WDT is controlled by SWDTEN bit of the WDTCON register
// 0000 0000
#ifdef __18CXX
#pragma config WDT=OFF
#else
#pragma config[3] = 0x00
#endif
//  CONFIG3H: CONFIGURATION REGISTER 3 HIGH (BYTE ADDRESS 300005h)
// bit 7 MCLRE: MCLR Pin Enable bit 
// 1 = MCLR pin enabled; RE3 input pin disabled
// 0 = RE3 input pin enabled; MCLR disabled
// bit 6-4 Unimplemented: Read as ‘0’
//  000
// bit 3 HFOFST: HFINTOSC Fast Start-up
//      1    = HFINTOSC starts clocking the CPU without waiting for the oscillator to stabilize.
//      0    = The system clock is held off until the HFINTOSC is stable.
// bit 2 LPT1OSC: Low-Power Timer1 Oscillator Enable bit
//       1   = Timer1 configured for low-power operation
//       0   = Timer1 configured for higher power operation
// bit 1 PBADEN: PORTB A/D Enable bit (Affects ANSELH Reset state. ANSELH controls PORTB<4:0> pin configuration.)
//        1  = PORTB<4:0> pins are configured as analog input channels on Reset
//        0  = PORTB<4:0> pins are configured as digital I/O on Reset
// bit 0 CCP2MX: CCP2 MUX bit
//         1 = CCP2 input/output is multiplexed with RC1 
//         0 = CCP2 input/output is multiplexed with RB3 
// 1000 1000


#ifdef __18CXX
   //#pragma config MCLRE=ON, CCP2MX=ON
   #pragma config MCLRE=ON
#else
   #pragma config[5] = 0x88
#endif
// CONFIG4L: CONFIGURATION REGISTER 4 LOW (BYTE ADDRESS 300006h)
// bit 7 DEBUG: Background Debugger Enable bit
// 1         = Background debugger disabled, RB6 and RB7 configured as general purpose I/O pins 
// 0         = Background debugger enabled, RB6 and RB7 are dedicated to In-Circuit Debug
// bit 6 XINST: Extended Instruction Set Enable bit
//  1        = Instruction set extension and Indexed Addressing mode enabled 
//  0        = Instruction set extension and Indexed Addressing mode disabled (Legacy mode)
// bit 5-3 Unimplemented: Read as ‘0’ 
//   00 0
// bit 2 LVP: Single-Supply ICSP Enable bit
//       1   = Single-Supply ICSP enabled 
//       0   = Single-Supply ICSP disabled 
// bit 1 Unimplemented: Read as ‘0’ 
//        0
// bit 0 STVREN: Stack Full/Underflow Reset Enable bit
//         1 = Stack full/underflow will cause Reset 
//         0 = Stack full/underflow will not cause Reset
// 0100 0100
#ifdef __DEBUG
    #ifdef __18CXX
        #pragma config XINST=ON, LVP=ON
    #else
        #pragma config[6] = 0x00
    #endif
#else
    #ifdef __18CXX
        #pragma config XINST=ON, LVP=ON
    #else
        #pragma config[6] = 0x80
    #endif
#endif
// CONFIG5L: CONFIGURATION REGISTER 5 LOW (BYTE ADDRESS 300008h)
// bit 7-4 Unimplemented: Read as ‘0’
// 0000
// bit 3 CP3: Code Protection bit(1)
//      1    = Block 3 not code-protected 
//      0    = Block 3 code-protected
// bit 2 CP2: Code Protection bit(1)
//       1   = Block 2 not code-protected 
//       0   = Block 2 code-protected 
// bit 1 CP1: Code Protection bit
//        1  = Block 1 not code-protected 
//        0  = Block 1 code-protected 
// bit 0 CP0: Code Protection bit
//         1 = Block 0 not code-protected 
//         0 = Block 0 code-protected
// 0000 1111
#ifdef __18CXX
#pragma config CP0=OFF, CP1=OFF
#else
#pragma config[8] = 0x0f
#endif
// CONFIG5H: CONFIGURATION REGISTER 5 HIGH (BYTE ADDRESS 300009h)
// bit 7 CPD: Data EEPROM Code Protection bit
// 1         = Data EEPROM not code-protected
// 0         = Data EEPROM code-protected
// bit 6 CPB: Boot Block Code Protection bit
//  1        = Boot Block not code-protected
//  0        = Boot Block code-protected
// bit 5-0 Unimplemented: Read as ‘0’
//   00 0000
// 1100 0000
#ifdef __18CXX
#pragma config CPD=OFF, CPB=OFF
#else
#pragma config[9] = 0xc0
#endif
// CONFIG6L: CONFIGURATION REGISTER 6 LOW (BYTE ADDRESS 30000Ah)
// bit 7-4 Unimplemented: Read as ‘0’
// 0000
// bit 3 WRT3: Write Protection bit(1)
//      1    = Block 3 not write-protected 
//      0    = Block 3 write-protected
// bit 2 WRT2: Write Protection bit(1)
//       1   = Block 2 not write-protected
//       0   = Block 2 write-protected
// bit 1 WRT1: Write Protection bit
//        1  = Block 1 not write-protected
//        0  = Block 1 write-protected
// bit 0 WRT0: Write Protection bit
//         1 = Block 0 not write-protected
//         0 = Block 0 write-protected
// 00000 1111
#ifdef __18CXX
#pragma config WRT1=OFF, WRT0=OFF
#else
#pragma config[0xa] = 0x0f
#endif
// CONFIG6H: CONFIGURATION REGISTER 6 HIGH (BYTE ADDRESS 30000Bh)
// bit 7 WRTD: Data EEPROM Write Protection bit
// 1         = Data EEPROM not write-protected
// 0         = Data EEPROM write-protected
// bit 6 WRTB: Boot Block Write Protection bit
//  1        = Boot Block not write-protected
//  0        = Boot Block write-protected
// bit 5 WRTC: Configuration Register Write Protection bit(1)
//   1       = Configuration registers not write-protected
//   0       = Configuration registers write-protected
// bit 4-0 Unimplemented: Read as ‘0’
//    0 0000

// 1110 0000
#ifdef __18CXX
#pragma config WRTD=OFF, WRTB=OFF, WRTC=OFF
#else
#pragma config[0xb] = 0xe0
#endif
//
// CONFIG7L: CONFIGURATION REGISTER 7 LOW (BYTE ADDRESS 30000Ch)
// bit 7-4 Unimplemented: Read as ‘0’
// 0000
// bit 3 EBTR3: Table Read Protection bit(1)
//      1    = Block 3 not protected from table reads executed in other blocks
//      0    = Block 3 protected from table reads executed in other blocks
// bit 2 EBTR2: Table Read Protection bit(1)
//       1   = Block 2 not protected from table reads executed in other blocks 
//       0   = Block 2 protected from table reads executed in other blocks 
// bit 1 EBTR1: Table Read Protection bit
//        1  = Block 1 not protected from table reads executed in other blocks 
//        0  = Block 1 protected from table reads executed in other blocks 
// bit 0 EBTR0: Table Read Protection bit
//         1 = Block 0 not protected from table reads executed in other blocks 
//         0 = Block 0 protected from table reads executed in other blocks
// 0000 1111
#ifdef __18CXX
#pragma config EBTR1=OFF, EBTR0=OFF
#else
#pragma config[0xc] = 0x0f
#endif
// CONFIG7H: CONFIGURATION REGISTER 7 HIGH (BYTE ADDRESS 30000Dh)
// bit 7 Unimplemented: Read as ‘0’
// 0
// bit 6 EBTRB: Boot Block Table Read Protection bit
//  1 = Boot Block not protected from table reads executed in other blocks
//  0 = Boot Block protected from table reads executed in other blocks
// bit 5-0 Unimplemented: Read as ‘0
//   00 0000
// bit 5-0 Unimplemented: Read as ‘0’
// 0100 0000
#ifdef __18CXX
#pragma config EBTRB=OFF
#else
#pragma config[0xd] = 0x40
#endif
#endif
//////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifdef __PIC24HJ64GP502__
   // configuration
   _FBS( RBS_NO_RAM & BSS_NO_BOOT_CODE & BWRP_WRPROTECT_OFF);
   // Boot Segment RAM Code Protection Size
   //   11        = No Boot RAM defined
   //   10        = Boot RAM is 128 bytes
   //   01        = Boot RAM is 256 bytes
   //   00        = Boot RAM is 1024 bytes
   // Boot Segment Program Flash Code Protection Size
   //       X11   = No Boot program Flash segment 
   //            Boot space is 1K Instruction Words (except interrupt vectors)
   //       110   = Standard security; boot program Flash segment ends at 0x0007FE
   //       010   = High security; boot program Flash segment ends at 0x0007FE 
   //            Boot space is 4K Instruction Words (except interrupt vectors)
   //       101   = Standard security; boot program Flash segment, ends at 0x001FFE
   //       001   = High security; boot program Flash segment ends at 0x001FFE
   //            Boot space is 8K Instruction Words (except interrupt vectors)
   //       100   = Standard security; boot program Flash segment ends at 0x003FFE
   //       000   = High security; boot program Flash segment ends at 0x003FFE
   // Boot Segment Program Flash Write Protection 
   //          1  = Boot segment can be written 
   //          0  = Boot segment is write-protected
   _FSS(RSS_NO_RAM & SSS_NO_FLASH & SWRP_WRPROTECT_OFF); 
   // Secure Segment RAM Code Protection
   //   11        = No Secure RAM defined
   //   10        = Secure RAM is 256 Bytes less BS RAM
   //   01        = Secure RAM is 2048 Bytes less BS RAM
   //   00        = Secure RAM is 4096 Bytes less BS RAM
   // Secure Segment Program Flash Code Protection Size (Secure segment is not implemented on 32K devices)
   //       X11   = No Secure program flash segment 
   //            Secure space is 4K IW less BS
   //       110   = Standard security; secure program flash segment startsat End of BS, ends at 0x001FFE
   //       010   = High security; secure program flash segment starts at End of BS, ends at 0x001FFE
   //            Secure space is 8K IW less BS
   //       101   = Standard security; secure program flash segment starts at End of BS, ends at 0x003FFE
   //       001   = High security; secure program flash segment starts at End of BS, ends at 0x003FFE
   //            Secure space is 16K IW less BS
   //       100   = Standard security; secure program flash segment starts at End of BS, ends at 007FFEh
   //       000   = High security; secure program flash segment starts at End of BS, ends at 0x007FFE
   // Secure Segment Program Flash Write-Protect bit
   //          1  = Secure Segment can bet written
   //          0  = Secure Segment is write-protected
   _FGS(GSS_OFF & GCP_OFF & GWRP_OFF);
   // General Segment Code-Protect bit
   //        11   = User program memory is not code-protected
   //        10   = Standard security
   //        0x   = High security
   // General Segment Write-Protect bit
   //          1  = User program memory is not write-protected
   //          0  = User program memory is write-protected
   _FOSCSEL(FNOSC_FRCPLL);
// & IESO_ON);
   // Two-speed Oscillator Start-up Enable bit
   //   1         = Start-up device with FRC, then automatically switch to the user-selected oscillator source when ready
   //   0         = Start-up device with user-selected oscillator source			
   // Initial Oscillator Source Selection bits
   //        111  = Internal Fast RC (FRC) oscillator with postscaler
   //        110  = Internal Fast RC (FRC) oscillator with divide-by-16
   //        101  = LPRC oscillator
   //        100  = Secondary (LP) oscillator
   //        011  = Primary (XT, HS, EC) oscillator with PLL
   //        010  = Primary (XT, HS, EC) oscillator
   //        001  = Internal Fast RC (FRC) oscillator with PLL
   //        000  = FRC oscillator
   _FOSC(FCKSM_CSECME & IOL1WAY_OFF & OSCIOFNC_ON & POSCMD_NONE);
   // Clock Switching Mode bits
   //   1x        = Clock switching is disabled, Fail-Safe Clock Monitor is disabled
   //   01        = Clock switching is enabled, Fail-Safe Clock Monitor is disabled
   //   00        = Clock switching is enabled, Fail-Safe Clock Monitor is enabled
   // Peripheral pin select configuration
   //     1       = Allow only one reconfiguration
   //     0       = Allow multiple reconfigurations
   // OSC2 Pin Function bit (except in XT and HS modes)
   //        1    = OSC2 is clock output
   //        0    = OSC2 is general purpose digital I/O pin
   // Primary Oscillator Mode Select bits
   //         11  = Primary oscillator disabled
   //         10  = HS Crystal Oscillator mode
   //         01  = XT Crystal Oscillator mode
   //         00  = EC (External Clock) mode
   _FWDT(FWDTEN_OFF);
   // Watchdog Timer Enable bit
   //   1         = Watchdog Timer always enabled (LPRC oscillator cannot be disabled. Clearing the SWDTEN bit in the RCON register has no effect.)
   //   0         = Watchdog Timer enabled/disabled by user software (LPRC can be disabled by clearing the SWDTEN bit in the RCON register)
   // Watchdog Timer Window Enable bit
   //    1        = Watchdog Timer in Non-Window mode
   //    0        = Watchdog Timer in Window mode
   // Watchdog Timer Prescaler bit
   //      1       = 1:128
   //      0       = 1:32
   // Watchdog Timer Postscaler bits
   //       1111   = 1:32,768
   //       1110   = 1:16,384
   //       0001   = 1:2
   //       0000   = 1:1
   _FPOR(ALTI2C_OFF & FPWRT_PWR1);
   // Alternate I2C™ pins
   //      1       = I2C mapped to SDA1/SCL1 pins
   //      0       = I2C mapped to ASDA1/ASCL1 pins
   // Power-on Reset Timer Value Select bits
   //       111    = PWRT = 128 ms
   //       110    = PWRT = 64 ms
   //       101    = PWRT = 32 ms
   //       100    = PWRT = 16 ms
   //       011    = PWRT = 8 ms
   //       010    = PWRT = 4 ms
   //       001    = PWRT = 2 ms
   //       000    = PWRT = Disabled
   _FICD(ICS_PGD1 & JTAGEN_OFF)
   // JTAG Enable bit
   //     1        = JTAG enabled
   //     0        = JTAG disabled
   // ICD Communication Channel Select bits
   //         11   = Communicate on PGEC1 and PGED1
   //         10   = Communicate on PGEC2 and PGED2
   //         01   = Communicate on PGEC3 and PGED3
   //         00   = Reserved, do not use

   _FUID0('G');
   _FUID1('Y');
   _FUID2('R');
   _FUID3('O');
   
#endif
//////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifdef __PIC24HJ64GP504__
   // configuration
   _FBS( RBS_NO_RAM & BSS_NO_BOOT_CODE & BWRP_WRPROTECT_OFF);
   // Boot Segment RAM Code Protection Size
   //   11        = No Boot RAM defined
   //   10        = Boot RAM is 128 bytes
   //   01        = Boot RAM is 256 bytes
   //   00        = Boot RAM is 1024 bytes
   // Boot Segment Program Flash Code Protection Size
   //       X11   = No Boot program Flash segment 
   //            Boot space is 1K Instruction Words (except interrupt vectors)
   //       110   = Standard security; boot program Flash segment ends at 0x0007FE
   //       010   = High security; boot program Flash segment ends at 0x0007FE 
   //            Boot space is 4K Instruction Words (except interrupt vectors)
   //       101   = Standard security; boot program Flash segment, ends at 0x001FFE
   //       001   = High security; boot program Flash segment ends at 0x001FFE
   //            Boot space is 8K Instruction Words (except interrupt vectors)
   //       100   = Standard security; boot program Flash segment ends at 0x003FFE
   //       000   = High security; boot program Flash segment ends at 0x003FFE
   // Boot Segment Program Flash Write Protection 
   //          1  = Boot segment can be written 
   //          0  = Boot segment is write-protected
   _FSS(RSS_NO_RAM & SSS_NO_FLASH & SWRP_WRPROTECT_OFF); 
   // Secure Segment RAM Code Protection
   //   11        = No Secure RAM defined
   //   10        = Secure RAM is 256 Bytes less BS RAM
   //   01        = Secure RAM is 2048 Bytes less BS RAM
   //   00        = Secure RAM is 4096 Bytes less BS RAM
   // Secure Segment Program Flash Code Protection Size (Secure segment is not implemented on 32K devices)
   //       X11   = No Secure program flash segment 
   //            Secure space is 4K IW less BS
   //       110   = Standard security; secure program flash segment startsat End of BS, ends at 0x001FFE
   //       010   = High security; secure program flash segment starts at End of BS, ends at 0x001FFE
   //            Secure space is 8K IW less BS
   //       101   = Standard security; secure program flash segment starts at End of BS, ends at 0x003FFE
   //       001   = High security; secure program flash segment starts at End of BS, ends at 0x003FFE
   //            Secure space is 16K IW less BS
   //       100   = Standard security; secure program flash segment starts at End of BS, ends at 007FFEh
   //       000   = High security; secure program flash segment starts at End of BS, ends at 0x007FFE
   // Secure Segment Program Flash Write-Protect bit
   //          1  = Secure Segment can bet written
   //          0  = Secure Segment is write-protected
   _FGS(GSS_OFF & GCP_OFF & GWRP_OFF);
   // General Segment Code-Protect bit
   //        11   = User program memory is not code-protected
   //        10   = Standard security
   //        0x   = High security
   // General Segment Write-Protect bit
   //          1  = User program memory is not write-protected
   //          0  = User program memory is write-protected
   _FOSCSEL(FNOSC_FRCPLL);
// & IESO_ON);
   // Two-speed Oscillator Start-up Enable bit
   //   1         = Start-up device with FRC, then automatically switch to the user-selected oscillator source when ready
   //   0         = Start-up device with user-selected oscillator source			
   // Initial Oscillator Source Selection bits
   //        111  = Internal Fast RC (FRC) oscillator with postscaler
   //        110  = Internal Fast RC (FRC) oscillator with divide-by-16
   //        101  = LPRC oscillator
   //        100  = Secondary (LP) oscillator
   //        011  = Primary (XT, HS, EC) oscillator with PLL
   //        010  = Primary (XT, HS, EC) oscillator
   //        001  = Internal Fast RC (FRC) oscillator with PLL
   //        000  = FRC oscillator
   _FOSC(FCKSM_CSECME & IOL1WAY_OFF & OSCIOFNC_ON & POSCMD_NONE);
   // Clock Switching Mode bits
   //   1x        = Clock switching is disabled, Fail-Safe Clock Monitor is disabled
   //   01        = Clock switching is enabled, Fail-Safe Clock Monitor is disabled
   //   00        = Clock switching is enabled, Fail-Safe Clock Monitor is enabled
   // Peripheral pin select configuration
   //     1       = Allow only one reconfiguration
   //     0       = Allow multiple reconfigurations
   // OSC2 Pin Function bit (except in XT and HS modes)
   //        1    = OSC2 is clock output
   //        0    = OSC2 is general purpose digital I/O pin
   // Primary Oscillator Mode Select bits
   //         11  = Primary oscillator disabled
   //         10  = HS Crystal Oscillator mode
   //         01  = XT Crystal Oscillator mode
   //         00  = EC (External Clock) mode
   _FWDT(FWDTEN_OFF);
   // Watchdog Timer Enable bit
   //   1         = Watchdog Timer always enabled (LPRC oscillator cannot be disabled. Clearing the SWDTEN bit in the RCON register has no effect.)
   //   0         = Watchdog Timer enabled/disabled by user software (LPRC can be disabled by clearing the SWDTEN bit in the RCON register)
   // Watchdog Timer Window Enable bit
   //    1        = Watchdog Timer in Non-Window mode
   //    0        = Watchdog Timer in Window mode
   // Watchdog Timer Prescaler bit
   //      1       = 1:128
   //      0       = 1:32
   // Watchdog Timer Postscaler bits
   //       1111   = 1:32,768
   //       1110   = 1:16,384
   //       0001   = 1:2
   //       0000   = 1:1
   _FPOR(ALTI2C_OFF & FPWRT_PWR1);
   // Alternate I2C™ pins
   //      1       = I2C mapped to SDA1/SCL1 pins
   //      0       = I2C mapped to ASDA1/ASCL1 pins
   // Power-on Reset Timer Value Select bits
   //       111    = PWRT = 128 ms
   //       110    = PWRT = 64 ms
   //       101    = PWRT = 32 ms
   //       100    = PWRT = 16 ms
   //       011    = PWRT = 8 ms
   //       010    = PWRT = 4 ms
   //       001    = PWRT = 2 ms
   //       000    = PWRT = Disabled
   _FICD(ICS_PGD1 & JTAGEN_OFF)
   // JTAG Enable bit
   //     1        = JTAG enabled
   //     0        = JTAG disabled
   // ICD Communication Channel Select bits
   //         11   = Communicate on PGEC1 and PGED1
   //         10   = Communicate on PGEC2 and PGED2
   //         01   = Communicate on PGEC3 and PGED3
   //         00   = Reserved, do not use

   _FUID0('M');
   _FUID1('A');
   _FUID2('I');
   _FUID3('N');
   
#endif
//////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////

#define Clock_8MHz
      
#define bitset(i,j) i |= 1U << j
#define bitclr(i,j) i &= ~(1U << j)
#define bittest(i,j) (i & (1U<<j))
#define bset(i,j) i.j=1
#define bclr(i,j) i.j=0
#define btest(i,j) (i.j)

//#ifdef __PIC24H__
//#define bitset(i,j) i |= (1U << j)
//#define bitclr(i,j) i &= ~(1U << j)
//#define bittest(i,j) (i & (1U<<j))

//#define bset(i,j) i.j=1
//#define bclr(i,j) i.j=0
//#define btest(i,j) (i.j)


//#endif

#pragma rambank RAM_BANK_2
///////////////////////////////////////BANK 2//////////////////////
#ifdef BT_TIMER1
unsigned long Timer1Cnt;
#else
unsigned int TIMER1_ADJUST;
unsigned char TIMER1_ADJ0;

unsigned char TMR130;
unsigned char TMR1SEC;
unsigned char TMR1MIN;
unsigned char TMR1HOUR;
unsigned char TMR1DAY;
unsigned char TMR1YEAR;

//#ifdef __18CXX
struct _TimerB1{
unsigned SetSyncTime:1;
unsigned SetSleep:1;
} TimerB1;
//#else
//bit SetSyncTime;
//bit SetSleep;
//#endif

unsigned char setTMR130;
unsigned char setTMR1SEC;
unsigned char setTMR1MIN;
unsigned char setTMR1HOUR;
unsigned char setTMR1DAY;
#endif

#pragma rambank RAM_BANK_0
/////////////////////////////////////BANK 0///////////////////////
#ifndef BUFFER_LEN
 #ifdef _18F2321_18F25K20
   #define BUFFER_LEN 18
   #define OUT_BUFFER_LEN 18
 #else
   #ifdef __PIC24H__
     #define BUFFER_LEN 20
     #define OUT_BUFFER_LEN 20
   #else
     #define BUFFER_LEN 14
     #define OUT_BUFFER_LEN 14
   #endif
 #endif
#endif

// 0 byte == iInQuSize
// 1 byte == iPtr1InQu
// 2 byte == iPtr2InQu
// 3-BUFFER_LEN queue
struct AQueue
{
    WORD iEntry;
    WORD iQueueSize;
    WORD iExit;
    unsigned char Queue[BUFFER_LEN];
};
struct BQueue
{
    WORD iEntry;
    WORD iQueueSize;
    WORD iExit;
    unsigned char Queue[OUT_BUFFER_LEN];
};
VOLATILE struct AQueue AInQu;
VOLATILE struct BQueue AOutQu;
#ifdef USE_COM2
VOLATILE struct AQueue AInQuCom2;
VOLATILE struct BQueue AOutQuCom2;
#endif

#define MAX_ADR '9'
#define MIN_ADR '1'
//#define ESC_SYMB '#'
#define ESC_SYMB 0x23

unsigned char UnitADR;
unsigned char UnitFrom;
unsigned char SendCMD;
unsigned char RetransmitLen;
struct _MainB2{
unsigned RetransmitTo:1;
unsigned getCMD:1;
unsigned getHbit:1;
unsigned ESCNextByte:1;
unsigned PrepI2C:1;
unsigned SetFromAddr:1;
unsigned SetSendCMD:1;
unsigned SendWithEsc:1;
#ifdef USE_COM2
unsigned SendCom2WithEsc:1;
#endif
unsigned CommLoopOK:1;

unsigned DoneWithCMD:1;
unsigned ComNotI2C:1;

VOLATILE unsigned prepStream:1;
VOLATILE unsigned prepCmd:1;
VOLATILE unsigned prepSkip:1;
VOLATILE unsigned prepZeroLen:1;
#ifdef EXT_INT
VOLATILE unsigned ExtInterrupt:1;
VOLATILE unsigned ExtInterrupt1:1;
VOLATILE unsigned ExtInterrupt2:1;
VOLATILE unsigned InDoneNoSleep:1;
VOLATILE unsigned ExtFirst:1;
#endif
#ifdef NEW_CMD_PROC
VOLATILE unsigned CheckESC:1;
#endif
} Main;
VOLATILE unsigned char RetrUnit;
VOLATILE unsigned char AllowMask;
VOLATILE unsigned char UnitMask1;
VOLATILE unsigned char UnitMask2;
#ifdef ALLOW_RELAY_TO_NEW
VOLATILE unsigned char AllowOldMask;
VOLATILE unsigned char AllowOldMask1;
VOLATILE unsigned char AllowOldMask2;
VOLATILE unsigned char AllowOldMask3;
VOLATILE unsigned char AllowOldMask4;
#endif

//bit BlockComm;
// this is in BANK0
struct _I2CB3_B4{
unsigned LockToI2C:1;
unsigned ESCI2CChar:1;
unsigned WaitQuToEmp:1;
unsigned NextI2CRead:1;
unsigned SetI2CYesNo:1;
unsigned EchoWhenI2C:1;

//} I2C_B3;
//struct _I2CB4{
#ifdef USE_OLD_CMD_EQ
unsigned RetransI2CCom:1;
unsigned RetransI2CComSet:1;
unsigned RetransComI2C:1;
unsigned RetransComI2CSet:1;
#endif
VOLATILE unsigned I2CGettingPKG:1;
unsigned I2CReplyExpected:1;
unsigned RetransI2ComCSet:1;
unsigned Timer0Fired:1;
unsigned LastWasUnitAddr:1;
unsigned SendComOneByte:1;

unsigned AddresWasSend:1;
} I2C;

#pragma rambank RAM_BANK_1
// this is in BANK1
struct _I2CB4_B5{
unsigned I2Cread:1;
VOLATILE unsigned I2CBusBusy:1;
#ifdef I2C_INT_SUPPORT
unsigned NeedMaster:1;
unsigned NeedRestart:1;
unsigned NeedStop:1;
unsigned NeedReceive:1;
unsigned NeedSend:1;
unsigned SendACK_NAK:1;
#endif
VOLATILE unsigned I2CMasterDone:1;
} I2C_B1;

unsigned char I2Caddr;
VOLATILE struct AQueue AInI2CQu;
VOLATILE struct BQueue AOutI2CQu;

#ifdef SPEED_SEND_DATA
#pragma rambank SPEED_SEND_DATA
struct _SpeedB6{
unsigned SpeedSend:1;
unsigned SpeedSendLocked:1;
unsigned SpeedSendUnit:1;
unsigned SpeedSendWithESC:1;
unsigned SpeedESCwas:1;
} Speed;
unsigned char *ptrSpeed;
unsigned char LenSpeed;
#endif

#pragma rambank RAM_BANK_0
unsigned char LenI2CRead;
unsigned char I2CReplyCMD;
#ifdef __PIC24H__
rtccTimeDate RtccTimeDate;
rtccTimeDate RtccTimeDateVal;
#endif

//bit ReTransToI2C;
unsigned char eeprom_read(unsigned char addr);
void eeprom_write(unsigned char addr, unsigned char value);



#define SPBRG_4800_40MIPS 2064

#define SPBRG_9600 51
#define SPBRG_9600_40MIPS 1040

#define SPBRG_19200 25

#define SPBRG_19200_8MHZ 25
#define SPBRG_19200_16MHZ 51
#define SPBRG_19200_32MHZ 103
#define SPBRG_19200_64MHZ 207
#define SPBRG_19200_40MIPS 519

#define SPBRG_38400_8MHZ 13
#define SPBRG_38400_16MHZ 25
#define SPBRG_38400_32MHZ 51
#define SPBRG_38400_64MHZ 103
#define SPBRG_38400_40MIPS 259

#define SPBRG_38400 12

#define SPBRG_57600 8
#define SPBRG_57600_16MHZ 16
#define SPBRG_57600_32MHZ 34
#define SPBRG_57600_64MHZ 68
#define SPBRG_57600_40MIPS 172

#define SPBRG_115200_16MHZ 8
#define SPBRG_115200_32MHZ 16
#define SPBRG_115200_64MHZ 34
#define SPBRG_115200_40MIPS 85



//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//      END COPY 0
/////////////////////////////////////////////////////////////////
