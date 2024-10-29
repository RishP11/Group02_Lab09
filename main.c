/*
 * EE 690/615 Embedded Systems Lab No. 07
 * Group 02 :
 * 210020009 - Ganesh Panduranga Karamsetty
 * 210020036 - Rishabh Pomaje
 * Program to setup the I2C module on the muC and communicate with DAC (MCP-4725).
*/
#define CLOCK_HZ    16000000

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

void CLK_enable( void );
void PORT_A_init( void );
void PORT_B_init( void );
void PORT_F_init( void );
void I2C_setup( void );
void I2C_Tx( uint8 );
uint8 I2C_Rx( void );

int main(void)
{
    CLK_enable();                                               // Enable all the required Clocks
    PORT_F_init();                                              // Setup Port F to interface with LEDs and Switches
    PORT_E_init();                                              // Setup Port E to interface with the UART
    UART7_setup();                                              // Setup UART Module 07
    while(1){
        ;
    }
}

void CLK_enable( void )
{
    // Setting up all the clocks
    SYSCTL_RCGCI2C_R |= (1 << 0) | (1 << 1) ;                             // Enabling the clock to UART module 7
    SYSCTL_RCGCGPIO_R |= (1 << 5) | (1 << 1) | (1 << 0) ;                           // Enable clock to GPIO_E
}

void PORT_A_init( void )
{
    // Configure this port as a I2C slave
    GPIO_PORTA_LOCK_R = 0x4C4F434B ;                            // Unlock commit register
    GPIO_PORTA_CR_R = 0xF1 ;                                    // Make PORT-E configurable
    GPIO_PORTA_DEN_R |= (1 << 6) | (1 << 7)  ;                                   // Set PORT-E pins as digital pins
    GPIO_PORTA_DIR_R = 0x00 ;                                   // Set PORT-E pin directions
//    GPIO_PORTA_PUR_R = 0x02 ;                                   // Pull-Up-Resistor Register
    GPIO_PORTA_AFSEL_R |= (1 << 6) | (1 << 7) ;                                 // Alternate function select for the PE0 and PE1
    GPIO_PORTA_PCTL_R = 0x33000000 ;                                  // Selecting the peripheral for the driving AFSEL
}

void PORT_B_init( void )
{
    GPIO_PORTB_LOCK_R = 0x4C4F434B ;                            // Unlock commit register
    GPIO_PORTB_CR_R = 0xF1 ;                                    // Make PORT-E configurable
    GPIO_PORTB_DEN_R |= (1 << 2) | (1 << 3) ;                                   // Set PORT-E pins as digital pins
    GPIO_PORTB_DIR_R |= (1 << 2) | (1 << 3) ;                                   // Set PORT-E pin directions
    GPIO_PORTB_PUR_R |= (1 << 2) | (1 << 3) ;                                   // Pull-Up-Resistor Register
    GPIO_PORTB_AFSEL_R |= (1 << 2) | (1 << 3) ;                                 // Alternate function select for the PE0 and PE1
    GPIO_PORTB_PCTL_R = 0x00003300 ;                                  // Selecting the peripheral for the driving AFSEL
    GPIO_PORTB_ODR_R |= (1 << 3) ;
}

void PORT_F_init( void )
{
    GPIO_PORTF_LOCK_R = 0x4C4F434B ;                            // Unlock commit register
    GPIO_PORTF_CR_R = 0xF1 ;                                    // Make PORT-F configurable
    GPIO_PORTF_DEN_R = 0x1F ;                                   // Set PORT-F pins as digital pins
    GPIO_PORTF_DIR_R = 0x0E ;                                   // Set PORT-F pin directions
    GPIO_PORTF_PUR_R = 0x11 ;                                   // Pull-Up-Resistor Register
    GPIO_PORTF_DATA_R = 0x00 ;                                  // Clearing previous data
}

void I2C_setup( void )
{
    // Time period register value calculation
    int SCL_HP = 6 ;
    int SCL_LP = 4 ;
    int SCL_CLK = 100000 ;
    int TPR = (1.0 * CLOCK_HZ / (2 * (SCL_HP + SCL_LP) * SCL_CLK)) - 1 ;

    I2C0_MCR_R = 0x10 ;                                         // Master Control Register
    I2C0_MTPR_R = TPR ;
    I2C0_MSA_R = 0x076 ;
    I2C0_MCS_R |= (1 << 1) | (1 << 2) ;//Control register
}

void I2C_Tx( uint8 data )
{
    I2C0_MDR_R = data ;
    I2C0_MCS_R |= (1 << 0) ;
}

