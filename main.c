/*
* EE 690/615 Embedded Systems Lab No. 09
* Group 02 :
* 210020009 - Ganesh Panduranga Karamsetty
* 210020036 - Rishabh Pomaje
* Program to setup the I2C module on the muC and communicate with DAC (MCP-4725).
* Using I2C module 3 <--> interfaced through Port D
*/
#define CLOCK_HZ    16000000
#define DAC_ADDR    0x60
#define WAVERES     500
#define PI          3.14159265358979
#define maxRES      4096
#define Vref        3.3

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "tm4c123gh6pm.h"

void CLK_enable( void );
void PORT_D_init( void );
void I2C3_setup( void );
void I2C_Tx( void );
void sineWave( void ) ;
void sawToothWave( float ) ;

uint16_t sineTable[WAVERES] ;
uint16_t sawTable[maxRES] ;

int main(void)
{
    CLK_enable();                                               // Enable all the required Clocks
    PORT_D_init();
    I2C3_setup();
    while(1) {
//        sineWave();
        sawToothWave( 2.5 ) ;
    }
}

void CLK_enable( void )
{
    // Setting up all the clocks
    SYSCTL_RCGCI2C_R |= 0x8 ;                             // Enabling the clock to UART module 7
    SYSCTL_RCGCGPIO_R |= 0x00008 ;
}

void PORT_D_init( void )
{
    // Configure this port as a I2C master
    GPIO_PORTD_LOCK_R = 0x4C4F434B ;                            // Unlock commit register
    GPIO_PORTD_CR_R |= 0xFF ;                                    // Make PORT-E configurable
    GPIO_PORTD_DEN_R  = 0x03 ;                                   // Set PORT-E pins as digital pins
    GPIO_PORTD_AFSEL_R = 0x03 ;
    GPIO_PORTD_PCTL_R = 0x33 ;                                  // Selecting the peripheral for the driving AFSEL
    GPIO_PORTD_ODR_R = 0x02 ;
    GPIO_PORTD_PUR_R = 0x03 ;
}

void I2C3_setup( void )
{
    // Time period register value calculation
    int SCL_HP = 6 ;
    int SCL_LP = 4 ;
    int SCL_CLK = 100000 ;
    int TPR = (1.0 * CLOCK_HZ / (2 * (SCL_HP + SCL_LP) * SCL_CLK)) - 1 ;

    I2C3_MCR_R = 0x10 ;                                         // Master Control Register
    I2C3_MTPR_R = TPR ;
    I2C3_MSA_R = 0xC0 ;
}

void sineWave( void )
{
    int sample_idx ;
    uint16_t hexVal ;

    for (sample_idx = 0; sample_idx < WAVERES; sample_idx++){
        sineTable[sample_idx] = 2048 + 2048  * (sin(2 * PI * 10 * sample_idx / WAVERES)) ;
    }

    while(1){
        // If switch 02 is pressed then exit and go to the other function
        // else continue with the same waveform
        for (sample_idx = 0; sample_idx < WAVERES; sample_idx++){
//            hexVal = 2048 + 2048  * (sin(2 * PI * 10 * sample_idx / WAVERES)) ; // Value to send
            hexVal = sineTable[sample_idx] ;
            I2C3_MDR_R = (hexVal >> 8) ;              // Send the first byte
            I2C3_MCS_R = 0x03;              // Start + Run

            while(I2C3_MCS_R & 0x01);       // Wait until transmission completes
//            if (I2C3_MCS_R & 0x02) {        // Check for errors
//            I2C3_MCS_R = 0x04;          // Send Stop if error
//            return;
//            }
            I2C3_MDR_R = (hexVal & 0xFF) ;              // Send the next byte
            I2C3_MCS_R = 0x05;              // Run + Stop

            while(I2C3_MCS_R & 0x01);       // Wait until transmission completes
            if (I2C3_MCS_R & 0x02) {        // Check for errors
            I2C3_MCS_R = 0x04;          // Send Stop if error
            }
        }
    }
}

void sawToothWave( float Vmax )
{

    int num_samples = (1.0 * maxRES * (Vmax / Vref)) ;

    uint16_t hexVal = 0 ;
    int i ;
    while(1){
        // If switch 02 is pressed then exit and go to the other function
        // else continue with the same waveform
        for (i = 0; i < num_samples; i++){
//            hexVal = 2048 + 2048  * (sin(2 * PI * 10 * sample_idx / WAVERES)) ; // Value to send
            hexVal += 0x001 ;
            I2C3_MDR_R = (hexVal >> 8) ;              // Send the first byte
            I2C3_MCS_R = 0x03;              // Start + Run

            while(I2C3_MCS_R & 0x01);       // Wait until transmission completes
            if (I2C3_MCS_R & 0x02) {        // Check for errors
            I2C3_MCS_R = 0x04;          // Send Stop if error
            return;
            }
            I2C3_MDR_R = (hexVal & 0xFF) ;              // Send the next byte
            I2C3_MCS_R = 0x05;              // Run + Stop

            while(I2C3_MCS_R & 0x01);       // Wait until transmission completes
            if (I2C3_MCS_R & 0x02) {        // Check for errors
            I2C3_MCS_R = 0x04;          // Send Stop if error
            }
        }
        hexVal = 0 ;
    }
}
