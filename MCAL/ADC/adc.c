/******************************************************************************
 *
 * Module: ADC
 *
 * File Name: adc.c
 *
 * Description: Source file for the TM4C123GH6PM ADC driver
 *
 * Author: Youssef Khaled
 *
 *******************************************************************************/

#include"adc.h"
#include "tm4c123gh6pm_registers.h"

#define ADC1ACTSS_REG       (*((volatile uint32 *)0x40039000))
#define ADC1EMUX_REG        (*((volatile uint32 *)0x40039014))
#define ADC1SSMUX3_REG      (*((volatile uint32 *)0x400390A0))
#define ADC1SSCTL3_REG      (*((volatile uint32 *)0x400390A4))
#define ADC1SSFSTAT3_REG    (*((volatile uint32 *)0x400390AC))
#define ADC1SSFIFO3_REG     (*((volatile uint32 *)0x40039048))

void ADC_Init(void)
{
    // Enable ADC clock
    SYSCTL_RCGCADC_REG |= 0x02;

    // Enable clock for GPIO Port E
    SYSCTL_RCGCGPIO_REG |= 0x10;
    while(!(SYSCTL_PRGPIO_REG & 0x10));

    // Enable alternative function on PE1 (ADC input pin)
    GPIO_PORTE_AFSEL_REG |= (1 << 1);

    // Disable digital I/O on PE1
    GPIO_PORTE_DEN_REG &= ~(1 << 1);

    // Enable analog function on PE1
    GPIO_PORTE_AMSEL_REG |= (1 << 1);

    // Configure PE1 as input pin
    GPIO_PORTE_DIR_REG &= ~(1 << 1);

    // Disable sample sequencer 3
    ADC1ACTSS_REG &= ~(1 << 3);

    // Configure trigger event for sequencer 3 (always sample)
    ADC1EMUX_REG |= (0xF << 12);

    // Configure input source for sequencer 3 (PE1/Ain2)
    ADC1SSMUX3_REG = 2;

    // Configure sample control bits for sequencer 3
    ADC1SSCTL3_REG |= (1 << 1);

    // Enable sample sequencer 3
    ADC1ACTSS_REG |= (1 << 3);
}

uint16 ADC_Read(void)
{
    // Wait until the FIFO is not empty
    while((ADC1SSFSTAT3_REG & (1 << 8))); // Check FIFO Empty flag

    // Read the ADC value from the FIFO
    uint16 adcValue = (uint16)(ADC1SSFIFO3_REG & 0xFFF); // Mask off lower 12 bits

    // Optionally, clear the FIFO
     ADC1SSFSTAT3_REG |= (1 << 8); // Set the corresponding bit to clear FIFO


    return adcValue;
}
