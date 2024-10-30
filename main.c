/*
 * EE 690/615 Embedded Systems Lab No. 09
 * Group 02 :
 * 210020009 - Ganesh Panduranga Karamsetty
 * 210020036 - Rishabh Pomaje
 * Program to setup the I2C module on the muC and communicate with DAC (MCP-4725).
 * Using I2C module 3 <--> interfaced through Port D
*/
#define CLOCK_HZ    16000000

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

void CLK_enable( void );
void PORT_D_init( void );
void I2C3_setup( void );
void I2C_Tx( void );

int main(void)
{
    CLK_enable();                                               // Enable all the required Clocks
    PORT_D_init();
    I2C3_setup();
    while(1) {
            int i = 0 ;
            for (i = 0 ; i < 1000 ; i++){
                ;
            }

        I2C_Tx();
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

void I2C_Tx( void )
{

    I2C3_MDR_R = 0x0F;              // Send the first byte
    I2C3_MCS_R = 0x03;              // Start + Run

//    int i = 0 ;
//    for (i = 0 ; i < 1000 ; i++){
//        ;
//    }

    while(I2C3_MCS_R & 0x01);       // Wait until transmission completes
    if (I2C3_MCS_R & 0x02) {        // Check for errors
        I2C3_MCS_R = 0x04;          // Send Stop if error
        return;
    }
    I2C3_MDR_R = 0xFF;              // Send the next byte
    I2C3_MCS_R = 0x05;              // Run + Stop

//    for (i = 0 ; i < 1000 ; i++){
//            ;
//        }

    while(I2C3_MCS_R & 0x01);       // Wait until transmission completes
    if (I2C3_MCS_R & 0x02) {        // Check for errors
        I2C3_MCS_R = 0x04;          // Send Stop if error
    }
}

