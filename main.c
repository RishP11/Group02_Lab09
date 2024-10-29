/*
 * EE 690/615 Embedded Systems Lab No. 07
 * Group 02 :
 * 210020009 - Ganesh Panduranga Karamsetty
 * 210020036 - Rishabh Pomaje
 * Program to setup the UART module on the muC and communicate with another muC.
*/
#define CLOCK_HZ    16000000

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

void CLK_enable( void );
void PORT_F_init( void );
void PORT_E_init( void );
void UART7_setup( void );
void UART_Tx( unsigned char );
unsigned char UART_Rx( void );

int main(void)
{
    CLK_enable();                                               // Enable all the required Clocks
    PORT_F_init();                                              // Setup Port F to interface with LEDs and Switches
    PORT_E_init();                                              // Setup Port E to interface with the UART
    UART7_setup();                                              // Setup UART Module 07
    while(1){
        unsigned char rxData = UART_Rx() ;                      // Check if there is any Rx message in FIFO
        if ((UART7_RSR_R & (0x02)) != 0){                       // ...|3-|2-|1-|0-|
            GPIO_PORTF_DATA_R = 0x02 ;                          // There is an error so turn ON red LED.
            UART7_ECR_R = 0xFF ;                                // Clear the Receive status register
        }
        else{
            // GPIO_PORTF_DATA_R = |_|_|_|SW1|G|B|R|SW2|
            if (rxData == 0xAA){
                GPIO_PORTF_DATA_R = 0x08 ;                      // Received Data is 0xAA so turn on green LED.
            }
            else if (rxData == 0xF0){
                GPIO_PORTF_DATA_R = 0x04 ;                      // Received Data is 0xF0 so turn on blue LED.
            }
        }
        unsigned char status_byte = GPIO_PORTF_DATA_R;          // Send message (if any)
        if (((~status_byte) & 0x10) != 0){
            UART_Tx(0xF0);                                      // Transmit 0xF0 if SW1 is pressed
        }
        if (((~status_byte) & 0x01) != 0){
            UART_Tx(0xAA);                                      // Transmit 0xAA if SW2 is pressed
        }
    }
}

void CLK_enable( void )
{
    // Setting up the UART clocks
    SYSCTL_RCGCUART_R |= (1 << 7) ;                             // Enabling the clock to UART module 7
    SYSCTL_RCGCGPIO_R |= 0x00000010 ;                           // Enable clock to GPIO_E
    SYSCTL_RCGCGPIO_R |= 0x00000020 ;                           // Enable clock to GPIO_F
}

void UART_Tx( unsigned char data )
{
    while((UART7_FR_R & (1 << 3)) != 0){                        // |7-TXFE|6-RXFF|5-TXFF|4-RXFE|3-BUSY|2-...|1-...|0-CTS|
         ;                                                      // Check for BUSY bit and wait for Tx-FIFO to become free
    }
    UART7_DR_R = data ;                                         // Place the Tx-message in the Data Register
}

unsigned char UART_Rx( void )
{
    if((UART7_FR_R & 0x40) != 0){                               // |7-TXFE|6-RXFF|5-TXFF|4-RXFE|3-BUSY|2-...|1-...|0-CTS|
        unsigned char rxData = UART7_DR_R ;
        return rxData ;                                         // Read the Rx-message from the Data Register since Rx-FIFO is full
    }
    else{
        return 0x00 ;                                           // Otherwise return Null Packet.
    }
}

void UART7_setup( void )
{
    UART7_CTL_R = 0x00 ;                                        // Disabling the UART

    // Calculations for Baud Rate Divisor
    int UARTSysClk = CLOCK_HZ ;                                 // Using system clock for UART module
    int clk_div = 16 ;                                          // Clock divider depending on the communication rate
    int baud_rate = 9600 ;                                      // Baud rate for communication

    float BRD = (1.0 * UARTSysClk) / (clk_div * baud_rate) ;    // Baud rate divisor (BRD)
    int BRDI = BRD ;                                            // Integer part of BRD
    BRD = BRD - BRDI ;                                          // Extracting the fractional part of BRD
    int BRDF = 64 * BRD + 0.5 ;                                 // Fractional part of the BRD to write to the register

    // Configuring the UART
    UART7_IBRD_R = BRDI ;                                       // Integer part of the BRD :: 16-bits
    UART7_FBRD_R = BRDF ;                                       // Fractional part of the BRD :: 6 bits
    UART7_LCRH_R |= (1 << 6) | (1 << 5) | (1 << 1) ;            // |7-SPS|6,5-WLEN|4-FEN|3-STP2|2-EPS|1-PEN|0-BRK|
    UART7_CC_R = 0x00 ;                                         // Clock source of the register
    UART7_ECR_R = 0xFF ;                                        // Clear any existing errors in RSR
    UART7_CTL_R |= (1 << 9) | (1 << 8) | (1 << 0) ;             // |9-RXE|8-TXE|7-LBE|6-...|5-HSE|4-EOT|3-SMART|2-SIRLP|1-SIREN|0-UARTEN|
}

void PORT_E_init( void )
{
    GPIO_PORTE_LOCK_R = 0x4C4F434B ;                            // Unlock commit register
    GPIO_PORTE_CR_R = 0xF1 ;                                    // Make PORT-E configurable
    GPIO_PORTE_DEN_R = 0x03 ;                                   // Set PORT-E pins as digital pins
    GPIO_PORTE_DIR_R = 0x02 ;                                   // Set PORT-E pin directions
    GPIO_PORTE_PUR_R = 0x02 ;                                   // Pull-Up-Resistor Register
    GPIO_PORTE_AFSEL_R = 0x03 ;                                 // Alternate function select for the PE0 and PE1
    GPIO_PORTE_PCTL_R = 0x11 ;                                  // Selecting the peripheral for the driving AFSEL
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
