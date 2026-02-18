#ifndef FSM_H
#define FSM_H

#include <Arduino.h>

// ===== ENUMERACIÓN DE ESTADOS =====
enum EstadoSistema {
    INIT,              // Inicialización del sistema
    ADQUISICION,       // Captura de sensores
    SINCRONIZACION,    // Espera sincronizada de transmisión
    TRANSMISION        // Envío de datos
};

// ===== FUNCIONES PÚBLICAS DE LA FSM =====
void inicializarFSM();
void ejecutarFSM();

#endif
