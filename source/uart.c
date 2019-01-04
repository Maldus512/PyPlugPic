/******************************************************************************/
/*Files to Include                                                            */
/******************************************************************************/

#if defined(__XC)
    #include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
    #include <htc.h>        /* HiTech General Include File */
#endif

#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */
#include <string.h>

#include "system.h"
#include "uart.h"


char uart_write_buffer[UART_BUF];
char uart_read_buffer[UART_BUF];
int uart_tx_index = 0;
int uart_rx_index = 0;
int to_transmit = 0;


command_t commandFIFO[16];
int command_index_head = 0;
int command_index_tail = 0;


void initUART() {
    
    // Set the EUSART1 module to the options selected in the user interface.

    // ABDOVF no_overflow; SCKP Non-Inverted; BRG16 16bit_generator; WUE disabled; ABDEN disabled;
    BAUD1CON = 0x08;

    // SPEN enabled; RX9 8-bit; CREN enabled; ADDEN disabled; SREN disabled;
//    RC1STA = 0x90;

    // TX9 8-bit; TX9D 0; SENDB sync_break_complete; TXEN disabled; SYNC asynchronous; BRGH hi_speed; CSRC slave;
//    TX1STA = 0x24;

    // Baud Rate = 9600; SP1BRGL 64;
    SP1BRGL = 0x40;

    // Baud Rate = 9600; SP1BRGH 3;
    SP1BRGH = 0x03;
    
    TX1STAbits.TXEN = 1;
    TX1STAbits.SYNC = 0;
    TX1STAbits.TX9 = 0;
    TX1STAbits.BRGH = 1;
    RC1STAbits.SPEN = 1;
    RC1STAbits.CREN = 1;
    
    
    INTCONbits.PEIE = 1;
    PIE3bits.RC1IE = 1;
    
}


void UARTputc(char txData)
{
    while(0 == PIR3bits.TX1IF)
    {
    }
    TX1REG = txData;    // Write the data byte to the USART.
}

void UARTBlockingWrite(char *data, int len)
{
    while(len--) {
        while(0 == PIR3bits.TX1IF)
        {
        }
        TX1REG = *(data++);    // Write the data byte to the USART.
    }
}

/*
void UARTWrite(char *data, int len) {
    
    if (len > UART_BUF){
        len = UART_BUF;
    }
    
    if (uart_tx_index + to_transmit + len < UART_BUF) {
        memcpy(&uart_write_buffer[uart_tx_index+to_transmit], data, len);
    }
    else {
        memcpy((unsigned char*)&uart_write_buffer[uart_tx_index], (unsigned char*)data, UART_BUF-(uart_tx_index+to_transmit));
        memcpy((unsigned char*)&uart_write_buffer[0],(unsigned char*) data, len-(UART_BUF-(uart_tx_index+to_transmit)));
    }
    if (0 == PIR3bits.TX1IF) {
        UARTputc(uart_write_buffer[uart_tx_index++]);
        to_transmit = len-1;
    }
    else {
        to_transmit += len;
    }

    PIE3bits.TX1IE = 1;
}*/



void UARTReceivedChar(char data) {
    char* cmd;
    command_t newCommand = NO_CMD;
    uart_read_buffer[uart_rx_index] = data;
    
    uart_rx_index = (uart_rx_index+1) % UART_BUF;
    
    /* If I'm overwriting unread data push the index further away */
    if (uart_rx_index == UART_BUF) {
        uart_rx_index = 0;
    }

    if (data == '\n') {
        uart_read_buffer[uart_rx_index] = '\0';
        uart_rx_index = 0;
        cmd = strstr(uart_read_buffer, "AT");

        if (memcmp(cmd, "ATON", 4) == 0) {
            newCommand = ON;
        }
        else if (memcmp(cmd, "ATOFF", 5) == 0) {
            newCommand = OFF;
        }
        else if (memcmp(cmd, "ATPRINT", 7) ==0) {
            newCommand = PRINT_READING;
        }
        else if (memcmp(cmd, "ATCAL", 5) == 0) {
            newCommand = CALIBRATE;
        }
        else if (memcmp(cmd, "ATRESET", 7) == 0) {
            newCommand = RESET;
        }
        else if (memcmp(cmd, "ATREAD", 6) == 0) {
            newCommand = READ_CURRENT;
        }
        else if (memcmp(cmd, "ATPOWER", 7) == 0) {
            newCommand = READ_POWER;
        }
        else if (memcmp(cmd, "ATZERO", 6) == 0) {
            newCommand = ZERO_POWER;
        }
        else {
            return;
        }
        
        commandFIFO[command_index_tail] = newCommand;
        command_index_tail = (command_index_tail+1)%16;
        
        if (command_index_tail == command_index_head) {
            command_index_head = (command_index_head+1)%16;
        }
    }
}

/*
int UARTRead(char *data, int len) {
    int read_bytes, tmp;
    char *newline;
    if (uart_rx_index  == uart_read_index) {
        return 0;
    }
    else if (uart_rx_index > uart_read_index) {
        read_bytes = len < uart_rx_index - uart_read_index ? len : uart_rx_index - uart_read_index;
        memcpy(data, &uart_read_buffer[uart_read_index], read_bytes);
    }
    else {
        read_bytes = len < UART_BUF - uart_read_index ? len : UART_BUF - uart_read_index;
        memcpy(data, &uart_read_buffer[uart_read_index], read_bytes);
        
        if (read_bytes < len) {
            tmp = len - read_bytes < uart_rx_index ? len-read_bytes : uart_rx_index;
            memcpy(&data[read_bytes], uart_read_buffer, tmp);
            read_bytes+=tmp;
        }
    }
    uart_read_index = (uart_read_index+read_bytes) % UART_BUF;
    
    return read_bytes;
}*/


command_t nextCommand() {
    command_t cmd = commandFIFO[command_index_head];
    
    if (command_index_head != command_index_tail) {
        
        command_index_head = (command_index_head+1) %16;
        return cmd;
    }
    else {
        return NO_CMD;
    }
}