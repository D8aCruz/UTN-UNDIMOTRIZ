#ifndef ADC_H
#define ADC_H

#include <Arduino.h>

// Variables globales con los valores promediados
extern volatile uint16_t adcTension;
extern volatile uint16_t adcCorriente;
extern volatile uint16_t adcPresion;

void procesarMuestraADC();

#endif