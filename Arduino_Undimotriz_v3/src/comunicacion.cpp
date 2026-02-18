#include "comunicacion.h"
#include "adc.h"
#include "velocidad.h"
#include "acelerometro.h"
#include "rele.h"

// Variable global que usará el módulo de relés
volatile uint8_t relayCommand = 0x00; // Inicializado con todos OFF

// Flags de comunicación (accesibles desde FSM)
volatile bool transmitirFlag = false;  // Setea ISR Timer1 cada 5ms
volatile bool recibirFlag = false;     // Setea serialEvent() cuando llegan datos

void inicializarComunicacion() {
    Serial.begin(115200);
    // ===== TIMER1 (16-bit) PARA TRANSMISIÓN CADA 5ms =====
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;
    
    // Configurar para 5ms con prescaler 64
    // 16MHz / 64 = 250000 Hz
    // Para 5ms: 250000 * 0.005 = 1250
    OCR1A = 1250;  // Exactamente 5ms ✓
    
    TCCR1B |= (1 << WGM12);   // CTC mode
    TCCR1B |= (1 << CS11) | (1 << CS10);  // Prescaler 64
    TIMSK1 |= (1 << OCIE1A);  // Habilitar interrupción

}

// ISR Timer1 - Cada 5ms
ISR(TIMER1_COMPA_vect) {
    transmitirFlag = true;  // Solo setea flag
}

// Transmitir trama de salida usando directamente las variables extern
void transmitirDatos() {
    uint8_t buffer[15];
    buffer[0] = 0xAA; // inicio de trama

    // Datos de Tensión, Corriente y Presión
    buffer[1]  = (adcTension >> 8) & 0xFF;
    buffer[2]  = adcTension & 0xFF;
    buffer[3]  = (adcCorriente >> 8) & 0xFF;
    buffer[4]  = adcCorriente & 0xFF;
    buffer[5]  = (adcPresion >> 8) & 0xFF;
    buffer[6]  = adcPresion & 0xFF;
    
    // Leer RPM una sola vez para evitar inconsistencias
    uint16_t rpm = getRPM();
    buffer[7]  = (rpm >> 8) & 0xFF;
    buffer[8]  = rpm & 0xFF;
    
    buffer[9]  = (accelX >> 8) & 0xFF;
    buffer[10] = accelX & 0xFF;
    buffer[11] = (accelY >> 8) & 0xFF;
    buffer[12] = accelY & 0xFF;
    // estado actual de relés (bits 0-3) + flag de sobretensión (bit 7)
    buffer[13] = getRelayState() & 0x0F;
    if (isSobretensionActiva()) {
        buffer[13] |= 0x80;
    }

    buffer[14] = 0x55; // fin de trama

    Serial.write(buffer, sizeof(buffer));
}

// ===== INTERRUPCIÓN SERIAL (llamada automáticamente por Arduino) =====
// Arduino invoca esta función automáticamente cuando hay datos en Serial
void serialEvent() {
    recibirFlag = true; // Setear flag de datos disponibles
}
// ===== PROCESAR FLAG DE RECEPCIÓN (llamada desde FSM) =====
// Byte inesperado, resetear contador
void procesarRecepcionSerial() {
    static uint8_t byteCount = 0;  // Contador de bytes recibidos
    static uint8_t bufferRecepcion[3] = {0}; // Buffer para trama entrante

    if (recibirFlag) {
        recibirFlag = false;  // Consumir flag

        while(Serial.available() > 0) {
            uint8_t byteLeido = Serial.read();
            if(byteCount == 0 && byteLeido == 0xAA) {
                // Inicio de trama detectado
                bufferRecepcion[byteCount++] = byteLeido;
            } else if (byteCount == 1){
                bufferRecepcion[byteCount++] = byteLeido;
            } else if (byteCount == 2) {
                bufferRecepcion[byteCount++] = byteLeido;
                // Fin de trama esperada
                if(byteLeido == 0x55) {
                    // Validar rango antes de asignar
                    if (bufferRecepcion[1] <= 0x0F) {  // Solo 4 relés (4 bits)
                        relayCommand = bufferRecepcion[1];
                    }
                }
                // Resetear contador para próxima trama
                byteCount = 0;
            } else {
                byteCount = 0;
            }
        }
    }
}


