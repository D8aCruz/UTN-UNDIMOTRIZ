#include "fsm.h"
#include "serial.h"
#include "tcp.h"
#include "procesamiento.h"
#include "lcd.h"

// ============================================================================
// VARIABLES PRIVADAS
// ============================================================================

static EstadoSistema estadoActual = FSM_LEER_ARDUINO;
static bool hayDatosNuevos = false;  // Flag para optimizar procesamiento

// ============================================================================
// DECLARACIÓN DE FUNCIONES PRIVADAS
// ============================================================================

static void fsm_leerArduino();
static void fsm_convertirMediciones();
static void fsm_transmitirQt();
static void fsm_atenderComandos();
static void fsm_refrescarPantalla();
static void fsm_diagnosticoRed();

// ============================================================================
// TABLA DE TRANSICIONES (Punteros a función)
// ============================================================================

typedef void (*FuncionEstado)();

static const FuncionEstado tablaEstados[] = {
  fsm_leerArduino,          // FSM_LEER_ARDUINO
  fsm_convertirMediciones,  // FSM_CONVERTIR_MEDICIONES
  fsm_transmitirQt,         // FSM_TRANSMITIR_QT
  fsm_atenderComandos,      // FSM_ATENDER_COMANDOS
  fsm_refrescarPantalla,    // FSM_REFRESCAR_PANTALLA
  fsm_diagnosticoRed        // FSM_DIAGNOSTICO_RED
};

// ============================================================================
// FUNCIONES PÚBLICAS
// ============================================================================

/**
 * Inicializa todos los módulos del sistema
 */
void fsm_inicializar() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("\n========================================");
  Serial.println("  ESP32 - SISTEMA UNDIMOTRIZ v2.0");
  Serial.println("  Maquina de Estados Integrada");
  Serial.println("========================================\n");
  
  Serial.println("[INIT] Inicializando Serial2 (Arduino)...");
  inicializarSerial();
  
  Serial.println("[INIT] Inicializando TCP/WiFi (Qt)...");
  iniciarTCP();

  
  Serial.println("[INIT] Inicializando LCD I2C...");
  inicializarLCD();
  
  Serial.println("\n[OK] Sistema inicializado correctamente!");
  Serial.println("[INFO] Entrando al ciclo principal...\n");
}

/**
 * Ejecuta un ciclo completo de la máquina de estados
 */
void fsm_ejecutar() {
  // Ejecutar función del estado actual mediante tabla
  tablaEstados[estadoActual]();
}

/**
 * Retorna el estado actual (útil para debug)
 */
EstadoSistema fsm_obtenerEstado() {
  return estadoActual;
}

// ============================================================================
// IMPLEMENTACIÓN DE ESTADOS (Funciones privadas)
// ============================================================================

/**
 * ESTADO 1: LEER_ARDUINO
 * Lee trama de 15 bytes desde Arduino por Serial2
 * - No bloqueante (timeout 10ms)
 * - Activa flag si hay datos nuevos
 */
static void fsm_leerArduino() {
  // Intentar leer trama con timeout corto
  if (recibirTrama(30)) {
    hayDatosNuevos = true;
  }
  
  // Transición: Siempre avanzar
  estadoActual = FSM_CONVERTIR_MEDICIONES;
}

/**
 * ESTADO 2: CONVERTIR_MEDICIONES
 * Convierte valores crudos del ADC a unidades físicas
 * - Solo procesa si hay datos nuevos (optimización)
 * - Calcula: tensión, corriente, presión, rpm, ángulo
 */
static void fsm_convertirMediciones() {
  if (hayDatosNuevos) {
    procesarDatos();
    hayDatosNuevos = false;  // Consumir flag
  }
  
  // Transición: Siempre avanzar
  estadoActual = FSM_TRANSMITIR_QT;
}

/**
 * ESTADO 3: TRANSMITIR_QT
 * Envía buffer crudo completo al cliente Qt por TCP
 * - Envía los 15 bytes tal cual (sin reconstruir)
 * - No bloqueante (verifica si hay cliente conectado)
 */
static void fsm_transmitirQt() {
  enviarTramaTCP();
  
  // Transición: Siempre avanzar
  estadoActual = FSM_ATENDER_COMANDOS;
}

/**
 * ESTADO 4: ATENDER_COMANDOS
 * Recibe comandos desde Qt y los reenvía al Arduino
 * - Valida formato: [0xAA][comando][0x55]
 * - Actualiza estadoRelesSolicitado
 * - Llama a enviarComando() para transmitir a Arduino
 */
static void fsm_atenderComandos() {
  recibirComandoTCP();
  
  // Transición: Siempre avanzar
  estadoActual = FSM_REFRESCAR_PANTALLA;
}

/**
 * ESTADO 5: REFRESCAR_PANTALLA
 * Actualiza display LCD 20x4
 * - Control interno de frecuencia (millis())
 * - Muestra valores procesados o crudos
 */
static void fsm_refrescarPantalla() {
  actualizarLCD();
  
  // Transición: Siempre avanzar
  estadoActual = FSM_DIAGNOSTICO_RED;
}

/**
 * ESTADO 6: DIAGNOSTICO_RED
 * Muestra estado de red WiFi y clientes conectados
 * - Control interno de frecuencia (cada 3 segundos)
 * - Incluye debug de mediciones crudas
 */
static void fsm_diagnosticoRed() {
  mostrarEstadoWiFi();
  
  // Transición: Volver al inicio del ciclo
  estadoActual = FSM_LEER_ARDUINO;
}
