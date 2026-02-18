#ifndef FSM_H
#define FSM_H

#include <Arduino.h>

// ============================================================================
// ESTADOS DE LA MÁQUINA
// ============================================================================

enum EstadoSistema {
  FSM_LEER_ARDUINO,           // Lee trama desde Arduino por Serial2
  FSM_CONVERTIR_MEDICIONES,   // Convierte datos crudos a unidades físicas
  FSM_TRANSMITIR_QT,          // Envía buffer a aplicación Qt por TCP
  FSM_ATENDER_COMANDOS,       // Recibe y procesa comandos desde Qt
  FSM_REFRESCAR_PANTALLA,     // Actualiza display LCD
  FSM_DIAGNOSTICO_RED         // Muestra estado WiFi y debug
};

// ============================================================================
// FUNCIONES PÚBLICAS
// ============================================================================

// Inicializa todos los módulos del sistema
void fsm_inicializar();

// Ejecuta un ciclo de la máquina de estados
void fsm_ejecutar();

// Obtiene el estado actual (para debug)
EstadoSistema fsm_obtenerEstado();

#endif
