#include "rele.h"
#include "offsetCurrent.h"

// Definición de tensión máxima en ADC para activar protección
#define MAX_ADC_TENSION 700  // Vmax = 80V, tal que 700 cuentas -> Vin = 75V < Vmax

// Definir pines de relés (D3-D4-D5-D6 relés)
const int relayPins[4] = {3, 4, 5, 6};
volatile uint8_t estadoReles;
static volatile bool sobretensionActiva = false;

void inicializarReles() {
    for (int i = 0; i < 4; i++) {
        pinMode(relayPins[i], OUTPUT);
        digitalWrite(relayPins[i], HIGH); // Con lógica invertida: HIGH = OFF
    }
    estadoReles = 0x00; // 0x00 = todos los bits en 0 = todos OFF (lógica invertida)
}

void actualizarReles(uint8_t nuevoEstado) {
    // Actualizar todos los relés según el nuevo estado
    for (int i = 0; i < 4; i++) {
        bool nuevoBit = (nuevoEstado >> i) & 0x01;
        digitalWrite(relayPins[i], nuevoBit ? LOW : HIGH);
    }
    estadoReles = nuevoEstado;
}

uint8_t getRelayState() {

    return estadoReles;
}

bool isSobretensionActiva() {
    return sobretensionActiva;
}

void proteccionXsobretension(uint16_t adcTension) {
    if(adcTension >= MAX_ADC_TENSION) {
        // Activar protección: todos los relés ON (lógica invertida)
        actualizarReles((uint16_t)0x0F); // 0x00 = todos los bits en 0 = todos ON (lógica invertida)
        actualizarOffsetCorriente();
        sobretensionActiva = true;
    } else {
        sobretensionActiva = false;
    }
}