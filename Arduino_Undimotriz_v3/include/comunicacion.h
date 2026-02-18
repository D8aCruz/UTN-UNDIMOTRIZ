#ifndef COMUNICACION_H
#define COMUNICACION_H

#include <Arduino.h>

// Variable global con el comando de relés recibido
extern volatile uint8_t relayCommand;

// Flags de comunicación
extern volatile bool recibirFlag;      // Setea serialEvent() cuando llegan datos
extern volatile bool transmitirFlag;   // Setea Timer1 cada 5ms

// Inicialización del módulo de comunicación
void inicializarComunicacion();

// Transmitir trama de salida con todos los datos (usa variables extern)
void transmitirDatos();

// Procesar el flag de recepción (llamada desde FSM)
void procesarRecepcionSerial();

// serialEvent() es llamada automáticamente por Arduino
// No necesita prototipo aquí

#endif