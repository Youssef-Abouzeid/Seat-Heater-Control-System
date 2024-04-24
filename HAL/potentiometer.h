/**********************************************************************************************
 *
 * HAL DRIVER: Potentiometer
 *
 * File Name: potentiometer.h
 *
 * Description: Header file for the Potentiometer driver
 *
 * Author: Youssef khaled
 *
 ***********************************************************************************************/
#ifndef POTENTIOMETER_H_
#define POTENTIOMETER_H_

#include "std_types.h"


// Define constants
#define REF_VOLTAGE 3.3        // Reference voltage (in volts)
#define ADC_RANGE 4095         // ADC range
#define TEMP_RANGE 45.0        // Temperature range (in Celsius)

// Function to convert ADC value to temperature
uint16 ADC_to_Temperature(void);



#endif /* POTENTIOMETER_H_ */
