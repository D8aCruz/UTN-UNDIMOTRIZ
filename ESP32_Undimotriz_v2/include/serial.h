#ifndef SERIAL_H
#define SERIAL_H

#include <Arduino.h>

// Buffer compartido de la trama recibida
extern uint8_t buffer[15];

// Variables globales con los datos recibidos
extern volatile uint16_t adcTension;
extern volatile uint16_t adcCorriente;
extern volatile uint16_t adcPresion;
extern volatile uint16_t rpm;
extern volatile int16_t accelX;
extern volatile int16_t accelY;
extern volatile uint8_t estadoRelesRecibido;    // Estado actual desde Arduino
extern volatile uint8_t estadoRelesSolicitado;  // Estado solicitado por TCP/Qt


// Inicialización del puerto serie
void inicializarSerial();

// Recepción de trama de 15 bytes (retorna true si se recibió trama válida)
bool recibirTrama(unsigned long timeout = 100);

// Transmisión de comando de relés (3 bytes)
void enviarComando();

#endif