/*
 * File:   main.c
 * Author: Ahmed "morga"
 * Created on July 27, 2025
 */

#include "header/haders.h"
#include <xc.h>
#include <stdint.h>

#define _XTAL_FREQ 48000000UL    // 48 MHz internal oscillator
#define TMR0_PRELOAD 131          // Timer0 preload for 375 Hz

// =================== PIN DEFINITIONS ===================
// Motor control
#define M1_OFF      LATCbits.LATC2
#define M2_OFF      LATBbits.LATB5
#define M1_S1       LATCbits.LATC0
#define M2_S1       LATBbits.LATB4
#define KILL_SW     PORTAbits.RA0
#define PIC_PWM     LATAbits.LATA1

// Sense pins
#define ERR1        PORTCbits.RC1
#define ERR2        PORTBbits.RB3
#define M1_CUR_SEN  PORTAbits.RA2
#define M2_CUR_SEN  PORTCbits.RC4
#define VOLT_SEN    PORTAbits.RA4
#define A1          PORTCbits.RC5
#define A2          PORTCbits.RC3
#define B1          PORTAbits.RA5
#define B2          PORTCbits.RC6

// I2C pins
#define SDA_PIN     TRISCbits.TRISC7
#define SCL_PIN     TRISBbits.TRISB7

// LED indicator
#define LED         LATAbits.LATA3

// I2C settings
#define I2C_ADDRESS 0x0D   // 7-bit address
volatile uint8_t i2c_byte = 0;

// =================== FUNCTION PROTOTYPES ===================
void Initialize(void);
void Timer0_Init(void);
void I2C_Slave_Init(uint8_t address);

// =================== MAIN ===================
void main(void)
{
    Initialize();

    while (1)
    {
        
        LED = 0;
        __delay_ms(500);
        LED = 0;
        __delay_ms(500);
        
    }
}

// =================== INITIALIZATION ===================
void Initialize(void)
{
    // Clock: 48 MHz HFINTOSC
    OSCEN   = 0b01000000;  // Enable HFINTOSC
    OSCCON1 = 0b00000000;  // HFFRQ from OSCFRQ
    OSCFRQ  = 0b00000111;  // 48 MHz

    // Motor control outputs
    TRISCbits.TRISC2 = 0;
    TRISBbits.TRISB5 = 0;
    TRISCbits.TRISC0 = 0;
    TRISBbits.TRISB4 = 0;
    TRISAbits.TRISA1 = 0;

    // Kill switch input
    TRISAbits.TRISA0 = 1;

    // Sense inputs
    TRISCbits.TRISC1 = 1;
    TRISBbits.TRISB3 = 1;
    TRISAbits.TRISA2 = 1;
    TRISCbits.TRISC4 = 1;
    TRISAbits.TRISA4 = 1;
    TRISCbits.TRISC5 = 1;
    TRISCbits.TRISC3 = 1;
    TRISAbits.TRISA5 = 1;
    TRISCbits.TRISC6 = 1;

    // I2C pins as inputs (open-drain)
    SDA_PIN = 1;
    SCL_PIN = 1;

    // LED output
    TRISBbits.TRISB6 = 0;

    // Initial states
    M1_OFF = 1;
    M2_OFF = 1;
    M1_S1 = 0;
    M2_S1 = 0;
    PIC_PWM = 0;
    LED = 0;

    // Enable global & peripheral interrupts
    INTCONbits.GIE  = 1;
    INTCONbits.PEIE = 1;

    // Initialize peripherals
    Timer0_Init();
    I2C_Slave_Init(I2C_ADDRESS);
}

// =================== TIMER0 ===================
void Timer0_Init(void)
{
    // Timer0: 8-bit, prescaler 1:256, 375 Hz
    T0CON0 = 0b10000000;       // T0EN = 1, 8-bit
    T0CON1 = 0b01000110;       // Fosc/4, prescaler 1:256, sync
    TMR0L  = TMR0_PRELOAD;     // preload
    PIR0bits.TMR0IF = 0;       // clear interrupt flag
    PIE0bits.TMR0IE = 1;       // enable interrupt
}

// =================== I2C SLAVE ===================
void I2C_Slave_Init(uint8_t address)
{
    SSP1CON1 = 0b00110110;        // I2C slave, 7-bit, enable SSP
    SSP1CON2 = 0x00;
    SSP1CON3 = 0b00000000;

    SSP1ADD = (address << 1);     // shift for 7-bit address
    SSP1MSK = 0xFE;               // mask all bits

    PIR3bits.SSP1IF = 0;
    PIE3bits.SSP1IE = 1;          // enable MSSP interrupt
    PIE3bits.BCL1IE = 1;          // enable bus collision interrupt
}

// =================== ISR ===================
void __interrupt() ISR(void)
{
    // Timer0 interrupt
    if (PIR0bits.TMR0IF)
    {
        PIR0bits.TMR0IF = 0;
        TMR0L = TMR0_PRELOAD;  // reload for exact 375 Hz

        // Place 375 Hz tasks here
    }

    // I2C MSSP interrupt
    if (PIR3bits.SSP1IF)
    {
        if (SSP1STATbits.R_W) // Master reading
        {
            if (!SSP1STATbits.BF)
            {
                SSP1BUF = 'a'; // example response
            }
        }
        else // Master writing
        {
            i2c_byte = SSP1BUF;
        }

        // Clear overflow/collision flags & release clock
        SSP1CON1bits.CKP = 1;
        if (SSP1CON1bits.SSPOV) SSP1CON1bits.SSPOV = 0;
        if (SSP1CON1bits.WCOL)  SSP1CON1bits.WCOL  = 0;
        PIR3bits.SSP1IF = 0;
    }

    // I2C Bus Collision
    if (PIR3bits.BCL1IF)
    {
        PIR3bits.BCL1IF = 0;
    }
}
