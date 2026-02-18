#ifndef ACELEROMETRO_H
#define ACELEROMETRO_H

#include <Arduino.h>

// Variables globales con las lecturas crudas (ya compensadas con offsets)
extern volatile int16_t accelX;
extern volatile int16_t accelY;

// Inicialización del acelerómetro
void inicializarAcelerometro();

// Lectura de aceleraciones X e Y
void leerAcelerometro();

#endif