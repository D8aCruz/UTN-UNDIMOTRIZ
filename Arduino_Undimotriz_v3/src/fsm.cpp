#include "fsm.h"
#include "comunicacion.h"
#include "adc.h"
#include "velocidad.h"
#include "acelerometro.h"
#include "rele.h"
#include "offsetCurrent.h"

// ===== VARIABLES DE LA FSM =====
static EstadoSistema estadoActual = INIT;

// ===== DECLARACIÓN DE FUNCIONES DE ESTADO =====
static void estado_INIT();
static void estado_ADQUISICION();
static void estado_SINCRONIZACION();
static void estado_TRANSMISION();

// ===== TABLA DE PUNTEROS A FUNCIÓN =====
typedef void (*FuncionEstado)();

static const FuncionEstado tablaFSM[] = {
    estado_INIT,           // 0: INIT
    estado_ADQUISICION,    // 1: ADQUISICION
    estado_SINCRONIZACION, // 2: SINCRONIZACION
    estado_TRANSMISION     // 3: TRANSMISION
};

// ===== IMPLEMENTACIÓN DE ESTADOS =====

/**
 * Estado: INIT
 * Propósito: Inicializar todos los módulos del sistema (1 sola vez)
 * Transición: → ADQUISICION (inmediata)
 */
static void estado_INIT() {
    inicializarComunicacion();
    inicializarVelocidad(2);  // Pin 2 para sensor de velocidad
    inicializarAcelerometro();
    inicializarReles();
    actualizarOffsetCorriente();
    estadoActual = ADQUISICION;  // Transición inmediata
}

/**
 * Estado: ADQUISICION
 * Propósito: Capturar datos de todos los sensores
 * Duración estimada: ~1.5ms (ADC ~330µs + I2C ~1ms)
 * Transición: → SINCRONIZACION (inmediata)
 */
static void estado_ADQUISICION() {
    procesarMuestraADC();    // Leer tensión, corriente, presión
    leerAcelerometro();      // Leer aceleración X, Y
    
    estadoActual = SINCRONIZACION;  // Transición inmediata
}

/**
 * Estado: SINCRONIZACION
 * Propósito: Esperar evento del Timer1 (cada 5ms)
 * Transición: → TRANSMISION (cuando transmitirFlag = true)
 */
static void estado_SINCRONIZACION() {
    // Verificar si llegó el momento de transmitir
    if (transmitirFlag) {
        transmitirFlag = false;  // Consumir flag ANTES de transición
        estadoActual = TRANSMISION;
    }
    // Si no, permanece en este estado (espera pasiva)
}

/**
 * Estado: TRANSMISION
 * Propósito: Enviar trama de datos y verificar timeout de velocidad
 * Duración estimada: ~1ms (transmisión serial + verificación)
 * Transición: → ADQUISICION (inmediata)
 */
static void estado_TRANSMISION() {
    verificarTimeout();      // Verificar timeout del sensor de velocidad
    transmitirDatos();       // Enviar trama serial (15 bytes)
    
    estadoActual = ADQUISICION;  // Volver a capturar datos
}

// ===== ACCIÓN PRIORITARIA: ACTUALIZACIÓN DE CARGAS =====
/**
 * Función: actualizarCargas
 * Propósito: Procesar comandos de relés con máxima prioridad
 * Se ejecuta ANTES de la FSM en cada ciclo del loop
 * Latencia típica: <500µs desde recepción del comando
 */
static void actualizarCargas() {
    if (recibirFlag) {
        procesarRecepcionSerial();     // Decodificar comando de 3 bytes
        actualizarReles(relayCommand); // Aplicar inmediatamente a los pines
        actualizarOffsetCorriente();
    }
}

// ===== FUNCIONES PÚBLICAS =====

/**
 * Inicializar FSM (llamar desde setup())
 */
void inicializarFSM() {
    estadoActual = INIT;
}

/**
 * Ejecutar un ciclo de la FSM (llamar desde loop())
 * Orden de prioridad:
 *   1. Actualización de cargas (máxima prioridad)
 *   2. Ejecución del estado actual de la FSM
 */
void ejecutarFSM() {
    // PRIORIDAD 1: Actualizar cargas (si hay comando pendiente)
    actualizarCargas();
    
    // PRIORIDAD 2: Ejecutar estado actual de la FSM
    tablaFSM[estadoActual]();
}
