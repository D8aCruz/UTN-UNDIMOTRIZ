#ifndef PROCESAMIENTO_H
#define PROCESAMIENTO_H

#include <Arduino.h>

// Variables globales con datos procesados
extern float tension;
extern float corriente;
extern float potencia;
extern float presion;    // Presión en bar
extern float altura;     // Altura en cm
extern float rpmProcesado;
extern float angulo;


// Procesar datos crudos del serial y convertirlos a unidades físicas
void procesarDatos();
float cuentasToPresionBar(uint16_t adcCrudo);
float convertirPresionPa(float presionBar);
float presionToAlturaCm(float presionBar);

// Debug: imprimir detalles de la cadena de presión
void imprimirDebugPresion();

#endif
