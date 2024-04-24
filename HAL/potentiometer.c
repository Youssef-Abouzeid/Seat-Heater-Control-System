/**********************************************************************************************
 *
 * HAL DRIVER: Potentiometer
 *
 * File Name: potentiometer.c
 *
 * Description: source file for the Potentiometer driver
 *
 * Author: Youssef khaled
 *
 ***********************************************************************************************/
#include"potentiometer.h"
#include"adc.h"

// Function to convert ADC value to temperature
uint16 ADC_to_Temperature(void) {

    uint16 adc_value = ADC_Read();
    // Calculate voltage per ADC count
    float32 voltage_per_count = (float32)REF_VOLTAGE / ADC_RANGE;

    // Calculate temperature change per voltage
    float32 temp_change_per_voltage = (float32)TEMP_RANGE / REF_VOLTAGE;

    // Calculate temperature
    uint16 temperature =  (float32)(adc_value * voltage_per_count * temp_change_per_voltage);

    return temperature;
}
