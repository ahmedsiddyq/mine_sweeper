/*
 * File:   main.c
 * Author: Ahmed
 * Target: PIC18F25Q10
 * Clock : 48 MHz internal
 */
#include <stdint.h>
#include <xc.h>

// =================== CONFIGURATION BITS ===================
#pragma config FEXTOSC = OFF
#pragma config RSTOSC = HFINTOSC_64MHZ
#pragma config CLKOUTEN = OFF
#pragma config CSWEN = ON
#pragma config FCMEN = ON
#pragma config MCLRE = EXTMCLR
#pragma config PWRTE = OFF
#pragma config LPBOREN = OFF
#pragma config BOREN = SBORDIS
#pragma config BORV = VBOR_190
#pragma config ZCD = OFF
#pragma config PPS1WAY = ON
#pragma config STVREN = ON
#pragma config XINST = OFF
#pragma config WDTCPS = WDTCPS_31
#pragma config WDTE = OFF
#pragma config WDTCWS = WDTCWS_7
#pragma config WDTCCS = SC
#pragma config WRT0 = OFF
#pragma config WRT1 = OFF
#pragma config WRT2 = OFF
#pragma config WRT3 = OFF
#pragma config WRTC = OFF
#pragma config WRTB = OFF
#pragma config WRTD = OFF
#pragma config SCANE = ON
#pragma config LVP = ON
#pragma config CP = OFF
#pragma config CPD = OFF
#pragma config EBTR0 = OFF
#pragma config EBTR1 = OFF
#pragma config EBTR2 = OFF
#pragma config EBTR3 = OFF
#pragma config EBTRB = OFF

#define _XTAL_FREQ 48000000UL    // 48 MHz internal oscillator
