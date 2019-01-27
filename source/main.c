#if defined(__XC)
    #include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
    #include <htc.h>        /* HiTech General Include File */
#endif

#include "hardware.h"
#include <stdint.h>        /* For uint8_t definition */
#include "system.h"
#include "uart.h"
#include "interrupts.h"
#include "adc.h"
#include "state.h"


#include <stdio.h>
#include <string.h>


void initSystem(void)
{
//    ConfigureOscillator();
    initGPIO();
    initTimer();
    initUART();
    initADC();
    
    stato.f_relayOn = 0;
    stato.f_transmitSensorReadings = 0;
}

/* TODO: set doubles to 32 bit */
int main(void)
{
    int i;
    char stringa[64];
    command_t command;
    uint16_t calibration = 512;
    double current = 0;
    double currentSum = 0;
    double amperePerMinute = 0;
    double powerConsumption = 0;
    int currentSampleNum = 0;
    int minuteCounter = 0;
    int powerOnCount = 0;
    
    const double powerConstant = 3.666;
    
    int calibrationMeanCount = 0;
    // initialize the device
    initSystem();
    
    RELAY = 0;
    stato.f_relayOn = 0;
    
    while (1)
    {
        command = nextCommand();
        
        switch (command) {
            case ON:
                RELAY = 1;
                stato.f_relayOn = 1;
                UARTputc('\n');
                /*minuteCounter = 0;
                current = 0;
                currentSum = 0;
                currentSampleNum = 0;
                f_readCurrentSensor = 0;
                f_1s = 0;*/
                break;
            case OFF:
                RELAY = 0;
                stato.f_relayOn = 0;
                calibrationMeanCount = 0;
                UARTputc('\n');
                break;
            case PRINT_READING:
                stato.f_transmitSensorReadings = 1;
                UARTputc('\n');
                break;

            case RESET:
                minuteCounter = 0;
                current = 0;
                currentSum = 0;
                currentSampleNum = 0;
                f_readCurrentSensor = 0;
                f_1s = 0;
                powerConsumption = 0;
                amperePerMinute = 0;
                UARTputc('\n');
                break;
                
            case READ_CURRENT:
                sprintf(stringa, "%.3f\n", current);
                UARTBlockingWrite((char*)stringa, strlen(stringa));
                break;
                
            case READ_POWER:
                sprintf(stringa, "%.3f\n", powerConsumption);
                UARTBlockingWrite((char*)stringa, strlen(stringa));
                break;
                
            case PRINT_STATE:
                sprintf(stringa, "%i\n", stato.f_relayOn);
                UARTBlockingWrite((char*)stringa, strlen(stringa));
                break;
                
            case ZERO_POWER:
                powerConsumption = 0;
                UARTputc('\n');
                break;
        }
                
        if (f_readCurrentSensor) {
            current = currentRead(&calibration);
            if (stato.f_relayOn) {
                currentSum += current;
                currentSampleNum++;

                if (current > 0.01 && powerOnCount++ > 5) {
                    NOTIFY = 1;
                }
                else if (current <= 0.01) {
                    powerOnCount = 0;
                    NOTIFY = 0;
                }
            }
            
            if (stato.f_transmitSensorReadings == 1) {
                sprintf(stringa, "state=%i, current=%.3f, ", stato.f_relayOn, current);
                UARTBlockingWrite((char*)stringa, strlen(stringa));
                sprintf(stringa, "adc=%i, cal=%i, ", readADC(), calibration);
                UARTBlockingWrite((char*)stringa, strlen(stringa));
                sprintf(stringa, "consumption=%.3f\n", powerConsumption);
                UARTBlockingWrite((char*)stringa, strlen(stringa));
            }
            f_readCurrentSensor = 0;
        }

        if (f_1s) {
            minuteCounter++;
            f_1s = 0;
        }
        
        if (minuteCounter >= 60) {
            minuteCounter = 0;
            
            amperePerMinute = currentSum/currentSampleNum;
            currentSum = currentSampleNum = 0;
            powerConsumption += amperePerMinute*powerConstant;
        }
        
        __delay_ms(1);
    }
}
