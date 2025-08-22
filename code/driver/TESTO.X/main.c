/*
 * File:   main.c
 * Author: Ahmed
 * Target: PIC18F25Q10
 * Clock : 48 MHz internal
 */

#include <stdint.h>


// CONFIG1L
#pragma config FEXTOSC = OFF    // External Oscillator mode Selection bits (Oscillator not enabled)
#pragma config RSTOSC = HFINTOSC_64MHZ// Power-up default value for COSC bits (HFINTOSC with HFFRQ = 64 MHz and CDIV = 1:1)

// CONFIG1H
#pragma config CLKOUTEN = OFF   // Clock Out Enable bit (CLKOUT function is disabled)
#pragma config CSWEN = ON       // Clock Switch Enable bit (Writing to NOSC and NDIV is allowed)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor enabled)

// CONFIG2L
#pragma config MCLRE = EXTMCLR  // Master Clear Enable bit (MCLR pin (RE3) is MCLR)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (Power up timer disabled)
#pragma config LPBOREN = OFF    // Low-power BOR enable bit (Low power BOR is disabled)
#pragma config BOREN = SBORDIS  // Brown-out Reset Enable bits (Brown-out Reset enabled , SBOREN bit is ignored)

// CONFIG2H
#pragma config BORV = VBOR_190  // Brown Out Reset Voltage selection bits (Brown-out Reset Voltage (VBOR) set to 1.90V)
#pragma config ZCD = OFF        // ZCD Disable bit (ZCD disabled. ZCD can be enabled by setting the ZCDSEN bit of ZCDCON)
#pragma config PPS1WAY = ON     // PPSLOCK bit One-Way Set Enable bit (PPSLOCK bit can be cleared and set only once; PPS registers remain locked after one clear/set cycle)
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Extended Instruction Set and Indexed Addressing Mode disabled)

// CONFIG3L
#pragma config WDTCPS = WDTCPS_31// WDT Period Select bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = OFF       // WDT operating mode (WDT Disabled)

// CONFIG3H
#pragma config WDTCWS = WDTCWS_7// WDT Window Select bits (window always open (100%); software control; keyed access not required)
#pragma config WDTCCS = SC      // WDT input clock selector (Software Control)

// CONFIG4L
#pragma config WRT0 = OFF       // Write Protection Block 0 (Block 0 (000800-001FFFh) not write-protected)
#pragma config WRT1 = OFF       // Write Protection Block 1 (Block 1 (002000-003FFFh) not write-protected)
#pragma config WRT2 = OFF       // Write Protection Block 2 (Block 2 (004000-005FFFh) not write-protected)
#pragma config WRT3 = OFF       // Write Protection Block 3 (Block 3 (006000-007FFFh) not write-protected)

// CONFIG4H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-30000Bh) not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block (000000-0007FFh) not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)
#pragma config SCANE = ON       // Scanner Enable bit (Scanner module is available for use, SCANMD bit can control the module)
#pragma config LVP = ON         // Low Voltage Programming Enable bit (Low voltage programming enabled. MCLR/VPP pin function is MCLR. MCLRE configuration bit is ignored)

// CONFIG5L
#pragma config CP = OFF         // UserNVM Program Memory Code Protection bit (UserNVM code protection disabled)
#pragma config CPD = OFF        // DataNVM Memory Code Protection bit (DataNVM code protection disabled)

// CONFIG5H

// CONFIG6L
#pragma config EBTR0 = OFF      // Table Read Protection Block 0 (Block 0 (000800-001FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection Block 1 (Block 1 (002000-003FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection Block 2 (Block 2 (004000-005FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR3 = OFF      // Table Read Protection Block 3 (Block 3 (006000-007FFFh) not protected from table reads executed in other blocks)

// CONFIG6H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot Block (000000-0007FFh) not protected from table reads executed in other blocks)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#define _XTAL_FREQ 48000000UL   // For __delay_ms()



#define TMR0_PRELOAD 131          // Timer0 preload for 375 Hz
#define SDA_PIN     TRISCbits.TRISC4
#define SCL_PIN     TRISCbits.TRISC3



#define I2C_ADDRESS 0x13   // 7-bit address
volatile uint8_t i2c_byte = 0;



void Initialize(void);
void clock_init(void);
void Timer0_Init(void);
void I2C_Slave_Init(uint8_t address);



void set_duty(uint16_t duty_rw)

{ 
 //   duty_rw = (uint16_t)duty_rw * 10;

duty_rw = (uint16_t)(((uint32_t)duty_rw * 4 * 256) / 100);
PWM3DCH = duty_rw >> 2;
    PWM3DCL = (PWM3DCL & 0x3F) | ((duty_rw & 0x03) << 6);
    

    // PWM3DCH = (duty_rw & 0x03FC)>>2;
    // PWM3DCL = (duty_rw & 0x0003)<<6;
    
/*
    uint16_t duty=dutys;
    duty =( dutys*10);
  //  duty = (duty/100);
    if (duty>1023)
    {duty=1000;}
    PWM3DCH = (uint8_t)(duty >> 2); 
    PWM3DCL = (PWM3DCL & 0x3F) | ((duty & 0x03) << 6);
    */
}

void set_pwm_frq(uint16_t frq)
{
 // T2PR =(uint8_t)((93750 / frq) - 1);  // Set PWM period
    T2PR=255;
    set_duty(1);    
}
void pwm_int(uint16_t pwm_frq)
{
TRISAbits.TRISA3 = 1;
PWM3CON=0;
T2PR =255;
PWM3DCH = 0x14;
PWM3DCL = 0x40; 
set_pwm_frq(pwm_frq);
TMR2IF=0;
TMR2IE=0;
T2CLKCON=1;
T2CONbits.T2CKPS=0b101;
T2CONbits.T2ON=1;
TRISAbits.TRISA3 = 0; 
PWM3CON = 0x80;   
RA3PPS = 0x07; 
      
}



uint16_t x=1;
void main(void) 
{
    
    Initialize();
   GIE=0; 
    while(1)
    {

      set_duty(x);
      x++;
     __delay_ms(100);
     if(x>99)
     {x=1;}
     if(x<1)
     {x=99;}
    }
}
void Initialize(void)
{
      //ANSELC = 0xE7;
      ANSELC=0;
      ANSELB=0;
      ANSELA=0;
     // Disable analog on RA3 (AN3)
      
    ANSELAbits.ANSELA3 = 0;  
    // Set RA3 as output
    // Start with LED OFF
    TRISAbits.TRISA3 = 0;  
    // LATAbits.LATA3 = 0;
    SDA_PIN=1;
    SCL_PIN=1;
    

    clock_init();
    Timer0_Init();
    I2C_Slave_Init(I2C_ADDRESS);
    pwm_int(1000);

    PEIE = 1;   // Peripherals Interrupts Enable Bit
    GIE = 1; 
    IPEN=0;
}
void I2C_Slave_Init(uint8_t address)
{
   
    SSP1CON1 = 0b00111110;        // I2C slave, 7-bit, enable SSP
    SSP1CON2 = 0x00;
    SSP1CON3 = 0b00000000;
    
    SSP1ADD = (uint8_t) (I2C_ADDRESS<<1);     // shift for 7-bit address
    SSP1MSK = 0xFF;               // mask all bits

  
    SSP1DATPPS = 0x14;   //RC4->MSSP1:SDA1;    
    RC3PPS = 0x0D;   //RC3->MSSP1:SCL1;    
    RC4PPS = 0x0E;   //RC4->MSSP1:SDA1;    
    SSP1CLKPPS = 0x13;   //RC3->MSSP1:SCL1;    
    
    
    PIR3bits.SSP1IF = 0;       // enable MSSP interrupt
    PIE3bits.BCL1IE = 1;     
    PIE3bits.SSP1IE = 1;// enable bus collision interrupt
}

// =================== TIMER0 ===================
void Timer0_Init(void)
{
    T0CON0 = 0b10000000;       // T0EN = 1, 8-bit
    T0CON1 = 0b01111010;       // Fosc/4, prescaler 1:256, sync
    TMR0H  = TMR0_PRELOAD;     // preload
    PIR0bits.TMR0IF = 0;       // clear interrupt flag
    PIE0bits.TMR0IE = 1;       // enable interrupt

}
void clock_init(void)
{
    OSCCON1=0x60;
    OSCFRQbits.HFFRQ=0b0111;
    OSCEN=0x40;


}


void __interrupt() ISR(void)
{
    if(PIR0bits.TMR0IF)
        
    {      // LATAbits.LATA3 = !LATAbits.LATA3;  // toggle RA3
      
    }

    // I2C MSSP interrupt
    if (PIR3bits.SSP1IF)
    { 
       // LATAbits.LATA3 =  !LATAbits.LATA3 ;

         if (SSP1STATbits.R_W) // Master is reading from slave
        {
            if (!SSP1STATbits.BF)   // Buffer empty, ready to load
            {
                SSP1BUF = 'a';
          
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