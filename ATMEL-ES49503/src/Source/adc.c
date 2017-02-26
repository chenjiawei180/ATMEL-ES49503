/*
 * adc.c
 *
 * Created: 2017/2/23 9:16:38
 *  Author: chenjiawei
 */ 

#include "adc.h"

struct adc_module adc_instance;

void Configure_Adc(void)
{
	struct port_config pin_conf;
	port_get_config_defaults(&pin_conf);
	
	pin_conf.direction  = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(PIN_PB06, &pin_conf);
	port_pin_set_output_level(PIN_PB06, false);
	
	struct adc_config config_adc;
	adc_get_config_defaults(&config_adc);
	config_adc.reference = ADC_REFERENCE_AREFA;
	config_adc.positive_input = ADC_POSITIVE_INPUT_PIN0;
	config_adc.clock_prescaler = ADC_CLOCK_PRESCALER_DIV256;
	adc_init(&adc_instance, ADC0, &config_adc);
	adc_enable(&adc_instance);
}

void Adc_Read_AdcValue(uint16_t *result)
{
	adc_start_conversion(&adc_instance);
	do {
		/* Wait for conversion to be done and read out result */
	} while (adc_read(&adc_instance, result) == STATUS_BUSY);
}