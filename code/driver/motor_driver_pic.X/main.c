
/*
 * File:   main.c
 * Author: Ahmed "morga"
 * Created on July 27, 2025
 */
#include "header.h"

// =================== PIN DEFINITIONS ===================
// Motor 1
#define M1_DIR     LATBbits.LATB4   // Direction pin
#define M1_PWM_PIN TRISCbits.TRISC7  // PWM output pin
#define M1_RPM_PIN TRISBbits.TRISB0 // RPM input pin (Timer1 gate)

// Motor 2
#define M2_DIR     LATBbits.LATB5   // Direction pin
#define M2_PWM_PIN TRISCbits.TRISC6 // PWM output pin
#define M2_RPM_PIN TRISBbits.TRISB1 // RPM input pin (Timer3 gate)

// I2C pins
#define SDA_PIN     TRISCbits.TRISC4
#define SCL_PIN     TRISCbits.TRISC3
#define I2C_ADDRESS 0x13

// =================== MACROS ===================
#define DESIRED_RPM_M1 receved_esp32[0]
#define DESIRED_RPM_M2 receved_esp32[1]
#define PWM_FREQ_M1    receved_esp32[2]
#define PWM_FREQ_M2    receved_esp32[3]
#define RPM_TIMEOUT_C  25

// =================== GLOBAL VARIABLES ===================
// ESP32 buffers
uint8_t receved_esp32[4] = {0};  // Desired RPMs, PWM freq, current limit
uint8_t send_esp32[3]    = {0};  // Voltage, Amp L, Amp R
uint8_t i_receved = 3;
uint8_t i_send    = 3;

// Motor 1 variables
volatile uint32_t m1_speed = 0;
volatile uint8_t  m1_rpm[5] = {0};
volatile uint16_t m1_avg_rpm = 0;
volatile uint8_t  m1_i = 0;
volatile uint8_t  m1_duty = 0;
volatile uint8_t  m1_outDate = 0;
volatile uint8_t  m1_outDate3=0;
volatile uint8_t  m1_time_out = RPM_TIMEOUT_C;
volatile uint8_t  m1_T_OF = 0;

// Motor 2 variables
volatile uint32_t m2_speed = 0;
volatile uint8_t  m2_rpm[5] = {0};
volatile uint16_t m2_avg_rpm = 0;
volatile uint8_t  m2_i = 0;
volatile uint8_t  m2_duty = 0;
volatile uint8_t  m2_outDate = 1;
volatile uint8_t  m2_outDate3=0;
volatile uint8_t  m2_time_out = RPM_TIMEOUT_C;
volatile uint8_t  m2_T_OF = 0;

// Default PWM values
uint16_t F1 = 3000;
uint16_t F2 = 3000;
uint8_t D1 = 50;
uint8_t D2 = 50;

// =================== FUNCTION PROTOTYPES ===================
void Initialize(void);
void clock_init(void);
void Timer0_Init(void);
void I2C_Slave_Init(uint8_t address);
void perudic_func_ISR(void);
void I2C_ISR(void);
void m1_RPM_int(void);
void m2_RPM_int(void);
void m1_RPM_ISR(void);
void m2_RPM_ISR(void);
void m1_update_rpm(void);
void m2_update_rpm(void);
void con_m1_RPM(uint8_t RPM);
void con_m2_RPM(uint8_t RPM);
void m1_pwm_int(uint16_t pwm_frq);
void m2_pwm_int(uint16_t pwm_frq);
void m1_set_pwm_frq(uint32_t frq);
void m2_set_pwm_frq(uint32_t frq);
void m1_set_duty(uint16_t duty);
void m2_set_duty(uint16_t duty);

// =================== MAIN ===================
void main(void) 
{
    Initialize();
    while(1)
    {
        // Example: run motor 1 with desired PWM and RPM
        con_m1_RPM(DESIRED_RPM_M1);
        con_m2_RPM(DESIRED_RPM_M2);
    }
}

// =================== INTERRUPT ===================
void __interrupt() ISR(void)
{
    perudic_func_ISR();
    I2C_ISR();
    m1_RPM_ISR();
    m2_RPM_ISR();
}

void Initialize(void)
{
      //ANSELC = 0xE7;
      ANSELC=0;
      ANSELB=0;
      ANSELA=0;
     // Disable analog on RA3 (AN3)
    //ANSELAbits.ANSELA3 = 0;  
    //TRISAbits.TRISA3 = 0;  
      
    clock_init();
    Timer0_Init();
    I2C_Slave_Init(I2C_ADDRESS);
    pwm_int(1000);
    RPM_int();
    PEIE = 1;   // Peripherals Interrupts Enable Bit
    GIE = 1; 
    IPEN=0;
}



void perudic_func_ISR(void)
{
   if(PIR0bits.TMR0IF)      
    {   
        // LATAbits.LATA3 = !LATAbits.LATA3;  // toggle RA3
        PIR0bits.TMR0IF = 0; 
        con_m1_RPM(DESIRED_RPM_M1);
        M1_DIR=1;
    }

}



// =================== TIMER0 ===================


void Timer0_Init(void)
{
    T0CON0 = 0b10000000;       // T0EN = 1, 8-bit
    T0CON1 = 0b01111010;       // Fosc/4, prescaler 1:256, sync
    TMR0H  = 131;     // preload
    PIR0bits.TMR0IF = 0;       // clear interrupt flag
    PIE0bits.TMR0IE = 1;       // enable interrupt

}
void clock_init(void)
{
    OSCCON1=0x60;
    OSCFRQbits.HFFRQ=0b0111;
    OSCEN=0x40;


}


// =================== i2c ===================


void I2C_Slave_Init(uint8_t address)
{
   
  SSP1CON1 = 0b00110110;        // I2C slave, 7-bit, enable SSP
  SSP1CON2 = 0b00010001;
  SSP1CON3 = 0b00011111;
  
  SSP1STAT=SSP1STAT+1;
  SSP1STAT=SSP1STAT-1;
     
  //  SSP1STAT=0x80;
  //  SSP1CON1 = 0x06;        // I2C slave, 7-bit, enable SSP
   // SSP1CON2 = 0x01;
 
    SSP1CON1bits.SSPEN = 1;        // I2C slave, 7-bit, enable SSP

    
    
    SSP1ADD = (uint8_t) (I2C_ADDRESS<<1);     // shift for 7-bit address
    SSP1MSK = 0xFF;               // mask all bits

  
    SSP1DATPPS = 0x14;   //RC4->MSSP1:SDA1;    
    RC3PPS = 0x0D;   //RC3->MSSP1:SCL1;    
    RC4PPS = 0x0E;   //RC4->MSSP1:SDA1;    
    SSP1CLKPPS = 0x13;   //RC3->MSSP1:SCL1;    
    SDA_PIN=1;
    SCL_PIN=1;
    
    PIR3bits.SSP1IF = 0;       // enable MSSP interrupt
    PIE3bits.BCL1IE = 1;     
    PIE3bits.SSP1IE = 1;// enable bus collision interrupt
}

void I2C_ISR()
{
      
       // I2C MSSP interrupt
    if (PIR3bits.SSP1IF)
    {
       PIR3bits.SSP1IF = 0;
     //  tept++;
     // LATAbits.LATA3 =  !LATAbits.LATA3 ;
  

     if(!SSP1STATbits.D_nA)
     { 
         uint8_t te = SSP1BUF; // dummy read to clear buffer  
         if (SSP1STATbits.R_W)
         {
         i_receved=3;;
             
         }
         else
         {
         i_send=3;
         }
             
     }
       send_esp32[1]=m1_duty;
       send_esp32[2]=m1_avg_rpm;
     if (SSP1STATbits.R_W) // Master is reading from slave
        {
            if (!SSP1STATbits.BF)   // Buffer empty, ready to load
            {
             // SSP1BUF = m1_avg_rpm;
              SSP1BUF =  send_esp32[i_send] ;
              i_send--;
            }
        }
       
        else // Master writing
        {
         if (SSP1STATbits.BF)   // Buffer FULL, ready to BE READED
         {   
           
            
            receved_esp32[i_receved] = SSP1BUF;
            i_receved--;
          }
        }
  
         SSP1CON1bits.CKP=1;
        if (SSP1CON1bits.SSPOV) SSP1CON1bits.SSPOV = 0;
        if (SSP1CON1bits.WCOL)  SSP1CON1bits.WCOL  = 0;
      
    
    // I2C Bus Collision
    if (PIR3bits.BCL1IF)
    {
        PIR3bits.BCL1IF = 0;
        if (SSP1CON1bits.SSPOV) SSP1CON1bits.SSPOV = 0;
    }
    }
}



// =================== MOTOR 1 ===================

// Initialize PWM for motor 1
void m1_pwm_int(uint16_t pwm_frq)
{
    PWM3CON=0;                     // Disable PWM module before configuration
    T2PR =255;                      // Set Timer2 period register (default)
    PWM3DCH = 0x14;                 // Initial high byte of PWM duty
    PWM3DCL = 0x40;                 // Initial low byte of PWM duty
    m1_set_pwm_frq(3000);           // Set PWM frequency to 3 kHz
    m1_set_duty(0);                 // Set initial duty to 0 (motor stopped)
    TMR2IF=0;                        // Clear Timer2 interrupt flag
    TMR2IE=0;                        // Disable Timer2 interrupt
    TMR2=0;                           // Reset Timer2 counter
    T2CLKCON=1;                      // Select internal clock for Timer2
    T2CONbits.T2CKPS=0b101;          // Set Timer2 prescaler to 1:32
    T2CONbits.T2ON=1;                // Enable Timer2
    CCPTMRSbits.P3TSEL=0b01;         // Select Timer2 for PWM3
    PWM3CON = 0x80;                  // Enable PWM output
    RC7PPS = 0x07;                   // Map PWM3 output to RC7 pin
    TRISBbits.TRISB4=0;              // Set RB4 as output (PWM pin)
    TRISCbits.TRISC7=0;              // Set RC7 as output (PWM pin)
}

// Set PWM frequency for motor 1
void m1_set_pwm_frq(uint32_t frq)
{
    T2PR=(uint8_t)(((375000)/(uint32_t)frq)- 1); // Calculate Timer2 period from desired frequency
    m1_set_duty(m1_duty);                        // Update duty cycle according to new period
}


// Set PWM duty cycle for motor 1
void m1_set_duty(uint16_t duty_rw)
{ 
    duty_rw = (uint16_t)(((uint32_t)duty_rw *4*T2PR) / 100); // Scale duty 0-100% to 10-bit register
    if(duty_rw>1023) { duty_rw=1020; }                      // Limit to max 10-bit value
    PWM3DCH = (uint8_t)(duty_rw >>2);                       // Set high 8 bits
    PWM3DCL = (uint8_t)((PWM3DCL & 0x3F) | ((duty_rw & 0x03) << 6)); // Set low 2 bits
}

// Initialize RPM measurement for motor 1
void m1_RPM_int()
{
    T1GPPS = 0x08;                    // Map RB0 to Timer1 gate input
    TRISBbits.TRISB0=1;               // Set RB0 as input
    T1CLKbits.CS = 0b0001;            // Timer1 clock source FOSC/4
    T1CONbits.CKPS = 0b11;            // Timer1 prescaler 1:8
    T1CONbits.RD16 = 1;               // 16-bit mode
    T1CONbits.ON = 1;                  // Enable Timer1

    T1GCONbits.GE = 1;                // Enable Timer1 gate
    T1GCONbits.GPOL = 0;              // Gate active-high
    T1GCONbits.GTM = 1;               // Toggle mode
    T1GCONbits.GSPM = 0;              // Continuous mode (not single pulse)
    PIR5bits.TMR1GIF = 0;             // Clear gate interrupt flag

    PIR4bits.TMR1IF = 0;              // Clear overflow flag
    PIE4bits.TMR1IE = 1;              // Enable Timer1 overflow interrupt
}

// ISR for motor 1 RPM measurement
void m1_RPM_ISR()
{
    if (PIR5bits.TMR1GIF) // Gate event
    {
        T1GCONbits.GE = 0;      // Disable gate temporarily
        T1CONbits.ON=0;         // Stop timer
        if (m1_T_OF)            // If timer overflow occurred
        {
            m1_speed = (uint32_t)0x10000*m1_T_OF; // Combine overflow with counter
            m1_speed += TMR1;
        }
        else { m1_speed = TMR1; }                  

        m1_rpm[m1_i]= (uint8_t)(191100/m1_speed); // Convert timer counts to RPM
        m1_i++;
        if (m1_i==5) m1_i = 0;                     // Wrap index

        m1_T_OF = 0;             // Reset overflow
        TMR1 = 0;                // Reset timer
        m1_outDate=0;            // Mark RPM array as updated
        m1_outDate3=0;           
        m1_time_out=RPM_TIMEOUT_C; // Reset timeout

        PIR5bits.TMR1GIF = 0;    // Clear gate interrupt
        T1CONbits.ON=1; 
        T1GCONbits.GE = 1;       // Re-enable timer & gate
    }

    if (PIR4bits.TMR1IF) // Overflow event
    {
        m1_T_OF++;             // Increment overflow counter
        PIR4bits.TMR1IF = 0;   // Clear overflow flag
    }
}

// Update motor 1 RPM average
void m1_update_rpm()
{     
    if(!m1_outDate) // If not up-to-date
    {
        uint16_t sum = 0;
        for (uint8_t j = 0; j <= 4; j++) sum += m1_rpm[j]; // Sum last 5 measurements
        m1_avg_rpm = sum / 5;  // Calculate average RPM
        m1_outDate=1;       // Mark as updated
    }
    else
    {
        if(m1_time_out==0) // Timeout reached
        {
            for (uint8_t j = 0; j <= 4; j++) m1_rpm[j]=0; // Clear buffer
            m1_avg_rpm=0;
        }   
        else { m1_time_out--; } // Countdown
    }
}

// Control loop for motor 1
void con_m1_RPM(uint8_t de_rpm)
{
    m1_update_rpm(); // Update RPM

    if (de_rpm == 0) m1_set_duty(0); // Stop motor
    else
    {
        int16_t rpm_er = (int16_t)de_rpm - (int16_t)m1_avg_rpm;  // Calculate error
        int16_t new_duty = (int16_t)m1_duty + (rpm_er/12);    // Proportional correction

        if (new_duty < 0) m1_duty = 0;
        else if (new_duty > 99) m1_duty = 99;
        else m1_duty = (uint8_t)new_duty;

        m1_set_duty(m1_duty); // Apply PWM duty
        m1_outDate3=1;
    }
}

// =================== MOTOR 2 ===================

// PWM init for motor 2
void m2_pwm_int(uint16_t pwm_frq)
{
    PWM4CON=0;                    
    T4PR =255;                    
    PWM4DCH = 0x14;               
    PWM4DCL = 0x40; 
    m2_set_pwm_frq(3000);         
    m2_set_duty(0);               
    TMR4IF=0;                     
    TMR4IE=0;                     
    TMR4=0;                        
    T4CLKCON=1;                   
    CCPTMRSbits.P4TSEL=0b10;     
    T4CONbits.T4CKPS=0b101;      
    T4CONbits.T4ON=1;            
    PWM4CON = 0x80;              
    M2_PWM_PIN = 0x07;            
}

void m2_set_pwm_frq(uint32_t frq)
{
    T4PR=(uint8_t)(((375000)/(uint32_t)frq)- 1);  
    m2_set_duty(m2_duty);         
}

void m2_set_duty(uint16_t duty_rw)
{ 
    duty_rw = (uint16_t)(((uint32_t)duty_rw *4*T4PR) / 100); 
    if(duty_rw>1023) { duty_rw=1020; } 
    PWM4DCH = (uint8_t)(duty_rw >>2); 
    PWM4DCL = (uint8_t)((PWM4DCL & 0x3F) | ((duty_rw & 0x03) << 6)); 
}

void m2_RPM_int()
{
    T3GPPS = 0x08; 
    TRISBbits.TRISB0=1;          
    T3CLKbits.CS = 0b0001;       
    T3CONbits.CKPS = 0b11;       
    T3CONbits.RD16 = 1;          
    T3CONbits.ON = 1;            

    T3GCONbits.GE = 1;           
    T3GCONbits.GPOL = 0;         
    T3GCONbits.GTM = 1;          
    T3GCONbits.GSPM = 0;         
    PIR5bits.TMR3GIF = 0;        

    PIR4bits.TMR3IF = 0;         
    PIE4bits.TMR3IE = 1;         
}

void m2_RPM_ISR()
{
    if (PIR5bits.TMR3GIF)
    {
        T3GCONbits.GE = 0;     
        T3CONbits.ON=0;        
        if (m2_T_OF)
        {
            m2_speed = (uint32_t)0x10000*m2_T_OF; 
            m2_speed += TMR3;                     
        }
        else { m2_speed = TMR3; }

        m2_rpm[m2_i]= (uint8_t)(191100/m2_speed); 
        m2_i++;
        if (m2_i==5) m2_i = 0; 

        m2_T_OF = 0;          
        TMR3 = 0;             
        m2_outDate=0;         
        m2_outDate3=0;        
        m2_time_out=RPM_TIMEOUT_C;

        PIR5bits.TMR3GIF = 0; 
        T3CONbits.ON=1;       
        T3GCONbits.GE = 1;    
    }

    if (PIR4bits.TMR3IF) 
    {
        m2_T_OF++;           
        PIR4bits.TMR3IF = 0; 
    }
}

void m2_update_rpm()
{     
    if(!m2_outDate)
    {
        uint16_t sum = 0;
        for (uint8_t j = 0; j <= 4; j++) sum += m2_rpm[j];
        m2_avg_rpm = sum / 5;  
        m2_outDate=1;
    }
    else
    {
        if(m2_time_out==0)
        {
            for (uint8_t j = 0; j <= 4; j++) m2_rpm[j]=0;
            m2_avg_rpm=0;
        }   
        else { m2_time_out--; }
    }
}

void con_m2_RPM(uint8_t de_rpm)
{
    m2_update_rpm();

    if (de_rpm == 0) m2_set_duty(0); 
    else
    {
        int16_t rpm_er = (int16_t)de_rpm - (int16_t)m2_avg_rpm; 
        int16_t new_duty = (int16_t)m2_duty + (rpm_er/12); 

        if (new_duty < 0) m2_duty = 0;
        else if (new_duty > 99) m2_duty = 99;
        else m2_duty = (uint8_t)new_duty;

        m2_set_duty(m2_duty); 
        m2_outDate3=1;
    }
}
