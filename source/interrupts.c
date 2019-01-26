/******************************************************************************/
/*Files to Include                                                            */
/******************************************************************************/

#if defined(__XC)
    #include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
    #include <htc.h>        /* HiTech General Include File */
#endif

#include <stdint.h>         /* For uint8_t definition */
#include "hardware.h"
#include "system.h"        /* For true/false definition */
#include "uart.h"
#include "state.h"

/******************************************************************************/
/* Interrupt Routines                                                         */
/******************************************************************************/

/* Baseline devices don't have interrupts. Note that some PIC16's 
 * are baseline devices.  Unfortunately the baseline detection macro is 
 * _PIC12 */
#ifndef _PIC12

char f_readCurrentSensor = 0;
char f_1s = 0;
int count_1s = 0;

static uint32_t milliseconds = 0;

void interrupt isr(void)
{
//    static int count_1s = 0;
    char read;
    
    if (PIR0bits.TMR0IF) {
        if (count_1s % 200 == 0) {
            f_readCurrentSensor = 1;
        }
        if (count_1s++ >= 1000) {
            count_1s = 0;
            f_1s = 1;
        }
        PIR0bits.TMR0IF = 0;
    }
    
    if (PIR3bits.TX1IF) {
        if (to_transmit > 0) {
            UARTputc(uart_write_buffer[uart_tx_index++]);
            to_transmit--;
        }
        
        if (to_transmit <= 0) {
            PIE3bits.TX1IE = 0;
            to_transmit = 0;
        }
    }
    
    if (PIR3bits.RC1IF) {
        
        if (RC1STAbits.OERR) {
            RC1STAbits.CREN = 0;
        }
        read = RC1REG;
        
        stato.f_transmitSensorReadings = 0;
        
        if (read == '\r') {
            //UARTputc('\n');
            UARTReceivedChar('\n');
        }
        else {
            UARTReceivedChar(read);
        }
        //UARTputc(read);
        
        RC1STAbits.CREN = 1;
    }
}

#endif


