#ifndef RELE_H
#define RELE_H

#include <Arduino.h>

// Inicialización de relés
void inicializarReles();

// Actualizar relés según variable externa
void actualizarReles(uint8_t nuevoEstado);

// Obtener estado actual de relés en un byte
uint8_t getRelayState();

// Estado de protección por sobretensión
bool isSobretensionActiva();

// Protección por sobretensión: Activar todas las cargas
void proteccionXsobretension(uint16_t adcTension);

#endif