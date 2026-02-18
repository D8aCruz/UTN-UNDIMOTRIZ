#ifndef VELOCIDAD_H
#define VELOCIDAD_H

#include <Arduino.h>

// Inicialización del módulo de velocidad
void inicializarVelocidad(uint8_t pinSensor);

// Verificar timeout (pone RPM=0 si no hay pulsos)
void verificarTimeout();

// Obtener valor actual de RPM
uint16_t getRPM();

#endif