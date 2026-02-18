/*
 * ESP32 Puente Serial - Sistema Integrado UNDIMOTRIZ v2.0
 * 
 * Arquitectura modular con máquina de estados:
 * - Serial: Comunicación con Arduino Nano (Serial2, GPIO 16/17)
 * - TCP: Servidor WiFi para aplicación Qt
 * - Procesamiento: Conversión ADC a unidades físicas
 * - LCD: Display I2C 20x4
 * - FSM: Coordinador de módulos sin bloqueos
 * 
 * Autor: Sistema UNDIMOTRIZ
 * Fecha: Diciembre 2025
 */

#include <Arduino.h>
#include "fsm.h"

void setup() {
  // Inicializar todos los módulos del sistema
  fsm_inicializar();
}

void loop() {
  // Ejecutar máquina de estados (un ciclo completo por iteración)
  fsm_ejecutar();
}


