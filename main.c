/*
 * EE 690/615 Embedded Systems Lab No. 09
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
void PORT_D_init( void );
void PORT_E_init( void );
void PORT_F_init( void );
void I2C3_setup( void );
void I2C2_setup( void );
void I2C_Tx( unsigned char );
unsigned char I2C_Rx( void );

int main(void)
{
    CLK_enable();                                               // Enable all the required Clocks
    PORT_D_init();
    PORT_E_init();
    PORT_F_init();
    I2C3_setup();
    I2C2_setup();

    while(1){
        if ((I2C2_SCSR_R & 0x04) != 0) {  // 0x04 = DATA-RIS (Data Received)
            int rxData = I2C2_SDR_R;
            if (rxData == 0xAA) {
                GPIO_PORTF_DATA_R = 0x02; // Turn on LED
            }
        }
        I2C_Tx(0xAA) ;
        int txStatus = I2C3_MCS_R ;
        while ((txStatus & 0x01) == 0){
            ;
        }
    }
}

void CLK_enable( void )
{
    // Setting up all the clocks
    SYSCTL_RCGCI2C_R |= 0x4 ;                             // Enabling the clock to UART module 7
    SYSCTL_RCGCI2C_R |= 0x8 ;                             // Enabling the clock to UART module 7
    SYSCTL_RCGCGPIO_R |= 0x00008 ;
    SYSCTL_RCGCGPIO_R |= 0x00010 ;
    SYSCTL_RCGCGPIO_R |= 0x00020 ;
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
    I2C3_MSA_R = 0x76 ;
}


void PORT_E_init( void )
{
    // Acts as I2C slave
    GPIO_PORTE_LOCK_R = 0x4C4F434B ;                            // Unlock commit register
    GPIO_PORTE_CR_R |= 0xFF ;                                    // Make PORT-E configurable
    GPIO_PORTE_DEN_R = 0x30 ;                                   // Set PORT-E pins as digital pins
    GPIO_PORTE_AFSEL_R = 0x30 ;                                 // Alternate function select for the PE0 and PE1
    GPIO_PORTE_PCTL_R = 0x330000 ;                                  // Selecting the peripheral for the driving AFSEL
    GPIO_PORTE_ODR_R = 0x20 ;
}

void I2C2_setup( void )
{
    I2C2_MCR_R = 0x20  ;                                         // Master Control Register
    I2C2_SOAR_R = 0x3B ;
    I2C2_SCSR_R = 0x01 ;
}


void PORT_F_init( void )
{
    GPIO_PORTF_LOCK_R = 0x4C4F434B ;                            // Unlock commit register
    GPIO_PORTF_CR_R |= 0xFF ;                                    // Make PORT-F configurable
    GPIO_PORTF_DEN_R = 0x1F ;                                   // Set PORT-F pins as digital pins
    GPIO_PORTF_DIR_R = 0x0E ;                                   // Set PORT-F pin directions
    GPIO_PORTF_PUR_R = 0x11 ;                                   // Pull-Up-Resistor Register
    GPIO_PORTF_DATA_R = 0x00 ;                                  // Clearing previous data
}

void I2C_Tx( unsigned char data )
{
    I2C3_MDR_R = data ;
    I2C3_MCS_R = 0x07;  // START and STOP
    while (I2C3_MCS_R & 0x01);  // Wait until BUSY bit clears

}

unsigned char I2C_Rx( void )
{
    unsigned char rx_Data = I2C2_SDR_R ;
    return rx_Data ;
}
