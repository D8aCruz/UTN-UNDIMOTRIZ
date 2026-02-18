#include "adc.h"
#include "offsetCurrent.h"
#include "rele.h"

// Pines analógicos
const uint8_t pinTension   = A3;
const uint8_t pinCorriente = A7;
const uint8_t pinPresion   = A6;

// Variables globales con los valores a transmitir
volatile uint16_t adcTension   = 0;
volatile uint16_t adcCorriente = 0;
volatile uint16_t adcPresion   = 0;

// Tamaño de ventana para media móvil (ajustable)
static const uint8_t TAMANIO_VENTANA = 20;

struct FiltroMediaMovil {
	uint16_t buffer[TAMANIO_VENTANA];
	uint32_t suma;
	uint8_t indice;
	uint8_t conteo;
};

static FiltroMediaMovil filtroTension = { {0}, 0, 0, 0 };
static FiltroMediaMovil filtroCorriente = { {0}, 0, 0, 0 };
static FiltroMediaMovil filtroPresion = { {0}, 0, 0, 0 };

static uint16_t aplicarMediaMovil(FiltroMediaMovil &filtro, uint16_t muestra) {
	filtro.suma = filtro.suma - filtro.buffer[filtro.indice] + muestra;
	filtro.buffer[filtro.indice] = muestra;
	if (filtro.conteo < TAMANIO_VENTANA) {
		filtro.conteo++;
	}
	filtro.indice++;
	filtro.indice %= TAMANIO_VENTANA;
	return (uint16_t)(filtro.suma / filtro.conteo);
}

// ===== PROCESAR UNA MUESTRA (llamada desde FSM) =====
void procesarMuestraADC() {
	uint16_t lecturaTension = analogRead(pinTension);
	uint16_t lecturaCorriente = analogRead(pinCorriente);
	uint16_t lecturaPresion = analogRead(pinPresion);

	adcTension = aplicarMediaMovil(filtroTension, lecturaTension);
	proteccionXsobretension(adcTension);	
	adcCorriente = aplicarMediaMovil(filtroCorriente, lecturaCorriente);
	if(adcCorriente>obtenerOffsetCorriente()){
		adcCorriente -= obtenerOffsetCorriente();
	} else {
		adcCorriente = 0;
	}
	adcPresion = aplicarMediaMovil(filtroPresion, lecturaPresion);
}
