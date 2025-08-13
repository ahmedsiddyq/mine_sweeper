/*
 * File:   main.c
 * Author: morga
 *
 * Created on July 27, 2025, 1:02 AM
 */
 
#include "header/haders.h"
#include <stdint.h>

#define _XTAL_FREQ 48000000 // Adjust to your clock
    
// ==== motor con Pins  ====
#define Motor1_OFF     PORTCbits.RC2
#define Motor2_OFF     PORTBbits.RB6
#define M1_S1          PORTCbits.RC0
#define M2_S1          PORTBbits.RB4
#define killsw         PORTAbits.RA0
#define PIC_PWM        PORTAbits.RA1



// ====  sense Pins  ====
#define error1          PORTCbits.RC1
#define error2          PORTBbits.RB5
#define M1_CURRENT_SEN  PORTAbits.RA2
#define M2_CURRENT_SEN  PORTCbits.RC4
#define VOLTAGE_SEN     PORTAbits.RA4
#define A1              PORTCbits.RC5
#define A2              PORTCbits.RC3
#define B1              PORTAbits.RA5
#define B2              PORTCbits.RC6


// ====  data pins  ====
#define SDA             PORTCbits.RC7 
#define SCL             PORTBbits.RB7
#define LED             PORTBbits.RB6
#define i2c_adress      13



// Setup I/O Pins
void Initialize(void) {
    
     OSCEN=0b01000000;
     OSCCON1=0b00000000;
     OSCFRQ=0b00000111;
     
     
    //  I2C Setup pin 
    RC7PPS     = 0x08;    // SDA1 output
    RB7PPS     = 0x07;    // SCL1 output 
    SSP1DATPPS = 0b00010111;    // SDA1 input 
    SSP1CLKPPS = 0b00001111;    // SCL1 input
    
    RA1PPS     = 0x03;    // CCP1 output 

    // Set motor control pins as outputs
    TRISCbits.TRISC2 = 0;  // Motor1_OFF
    TRISBbits.TRISB6 = 0;  // Motor2_OFF
    TRISCbits.TRISC0 = 0;  // M1_S1
    TRISBbits.TRISB4 = 0;  // M2_S1
    TRISAbits.TRISA1 = 0;  // PIC_PWM
    
    // Set kill switch as input
    TRISAbits.TRISA0 = 1;  // killsw
    
    // Set sense pins as inputs
    TRISCbits.TRISC1 = 1;  // error1
    TRISBbits.TRISB5 = 1;  // error2
    TRISAbits.TRISA2 = 1;  // M1_CURRENT_SEN
    TRISCbits.TRISC4 = 1;  // M2_CURRENT_SEN
    TRISAbits.TRISA4 = 1;  // VOLTAGE_SEN
    TRISCbits.TRISC5 = 1;  // A1
    TRISCbits.TRISC3 = 1;  // A2
    TRISAbits.TRISA5 = 1;  // B1
    TRISCbits.TRISC6 = 1;  // B2
    
    // Set data pins
    TRISCbits.TRISC7 = 1;  // SDA (I2C - bidirectional)
    TRISBbits.TRISB7 = 0;  // SCL (I2C clock - output)
    TRISBbits.TRISB6 = 0;  // LED (output)
    
    // Initialize output states
    Motor1_OFF = 1;       // Start with motors off
    Motor2_OFF = 1;
    M1_S1 = 0;
    M2_S1 = 0;
    PIC_PWM = 0;
    LED = 0;              // Start with LED off
    
    RCONbits.IPEN = 1;  // Enable interrupt priority
    INTCONbits.GIEH = 1;  // Enable high priority interrupts
    INTCONbits.GIEL = 1;  // Enable low priority interrupts
    INTCONbits.PEIE = 1;  // Enable peripheral interrupts

    
//adc_init();
 timerint();
 I2C_Init();
}

//timer 1  make  375 event at seconed  to bisc function 
void timerint(){
   
    T0CON0=0b10000000;
    T0CON1=0b01111010;
    T0CON0bits.T016BIT = 0;   //  //to make 8-bit timer with comper  function
    T0CON1bits.T0ASYNC = 1;   // chose the Prescaler
    T0CON1bits.T0CKPS = 0b1010; // Prescaler 1:1024 to git 15625 hz
    TMR0L=0;
    TMR0H=131;// ser autorelod register comperd to low regiseter timer0
        
}
/*
void adc_init()
{
 ADCON1=0b11010011;//
 ADACT=0b00000010;
   
}
 */
void __interrupt(irq(TMR0), low_priority) TMR0_ISR(void)
{
  
    //send voltge 
    //send current
    //send semce
    //blink
    PIR0bits.TMR0IF = 0; //set timer intrrupt off

}
void I2C_Init() {
    
    /* PPS setting for using RB1 as SCL */
    SSP1CLKPPS = 0x09;
    RB1PPS = 0x0F;
    /* PPS setting for using RB2 as SDA */
    SSP1DATPPS = 0x0A;
    RB2PPS = 0x10;
    
    
    /* Set pins RB1 and RB2 as Digital */
    ANSELBbits.ANSELB1 = 0;
    ANSELBbits.ANSELB2 = 0;
    /* Set pull-up resistors for RB1 and RB2 */
    WPUBbits.WPUB1 = 1;
    WPUBbits.WPUB2 = 1;
    /* Set open-drain mode for RB1 and RB2 */
    ODCONBbits.ODCB1 = 1;
    ODCONBbits.ODCB2 = 1;

    /* I2C Host Mode: Clock = F_OSC / (4 * (SSP1ADD + 1)) */
    SSP1CON0=0b1;
    /* Set the baud rate divider to obtain the I2C clock at 100000 Hz*/
    SSP1ADD = 0x9F;
    
    

    SSP1STAT = 0x80;            // Slew rate control disabled (100kHz)
    SSP1CON1 = 0x36;            // I²C Slave mode, 7-bit address, enable SSP
    SSP1CON2 = 0x01;            // Clock stretch enabled
    SSP1ADD  = i2c_adress << 1;    // Load slave address
    SSP1IF = 0;                 // Clear interrupt flag
    SSP1IE = 1;                 // Enable MSSP interrupt
    PEIE = 1;                   // Enable peripheral interrupts
    GIE = 1;                    // Enable global interrupts
}

void void __interrupt(irq(), high_priority) TMR0_ISR(void){
    if (SSP1IE && SSP1IF) {
        if (!SSP1STATbits.D_nA && !SSP1STATbits.R_nW) {
            // Master Write -> receive data
            uint8_t I2C_Rdata = SSP1BUF;   // Dummy read to clear BF
            while (!BF);              // Wait until data is received
            I2C_Rdata = SSP1BUF;           // Actual data from master
            // Do something with data (e.g., store, toggle LED)
        }
        else if (!SSP1STATbits.D_nA && SSP1STATbits.R_nW) {
            // Master Read -> send data
            uint8_t outData = 0xAB;   // Example response
            SSP1BUF = outData;
        }
        SSP1IF = 0; // Clear MSSP interrupt
    }
    
     //send voltge 
     //send current
     //send semce
     //blink
}

void main(void) {
    Initialize();
    while(1)
    {
        
    
    }
}
