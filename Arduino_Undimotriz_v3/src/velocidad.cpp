#include "velocidad.h"

static volatile uint32_t ultimoPulsoMicros = 0;
static volatile uint32_t periodoMicros = 0;

static const uint32_t timeoutMicros = 500000; // 0.5s sin pulsos → RPM=0
static const uint8_t PULSOS_POR_REVOLUCION = 30; // 30 pulsos = 1 revolución
static uint8_t pinVelocidad;

// ISR: se ejecuta en cada pulso del sensor
void ISR_velocidad() {
    uint32_t ahora = micros();
    if (ultimoPulsoMicros != 0) {
        periodoMicros = ahora - ultimoPulsoMicros;
    }
    ultimoPulsoMicros = ahora;
}

void inicializarVelocidad(uint8_t pinSensor) {
    pinVelocidad = pinSensor;
    pinMode(pinVelocidad, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(pinVelocidad), ISR_velocidad, RISING);
}

void verificarTimeout() {
    if ((micros() - ultimoPulsoMicros) > timeoutMicros) {
        periodoMicros = 0; // fuerza RPM=0
    }
}

uint16_t getRPM() {
    // Lectura atómica de periodoMicros
    uint32_t periodo;
    noInterrupts();
    periodo = periodoMicros;
    interrupts();
    
    if (periodo == 0) return 0;

    // Calcular RPM con 30 pulsos por revolución
    // freqPulsos = frecuencia de pulsos (Hz)
    // freqRev = freqPulsos / 30 (revoluciones por segundo)
    // RPM = freqRev * 60
    float freqPulsos = 1000000.0 / periodo; // Hz de pulsos
    float freqRev = freqPulsos / PULSOS_POR_REVOLUCION; // Rev/s
    uint16_t rpm = (uint16_t)(freqRev * 60.0); // RPM

    return rpm;
}