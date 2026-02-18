#ifndef OFFSETCURRENT_H
#define OFFSETCURRENT_H

#include <Arduino.h>

// Obtener el offset de corriente según cantidad de relés activos
// Retorna el offset a restar de la lectura del ADC
uint16_t obtenerOffsetCorriente();

// Actualizar el offset según el estado actual de los relés
void actualizarOffsetCorriente();

#endif
