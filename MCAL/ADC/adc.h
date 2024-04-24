/**********************************************************************************************
 *
 * Module: ADC
 *
 * File Name: adc.h
 *
 * Description: Header file for the TM4C123GH6PM ADC driver
 *
 * Author: Youssef khaled
 *
 ***********************************************************************************************/
#ifndef ADC_H_
#define ADC_H_

#include "std_types.h"

// Function prototypes
void ADC_Init(void);
uint16 ADC_Read(void);

#endif /* ADC_H_ */
