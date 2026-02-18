#include "offsetCurrent.h"
#include "rele.h"

// Offsets de corriente según cantidad de relés activos
// Ajustar estos valores según calibración real del sistema
static const uint16_t OFFSETS_CORRIENTE[5] = {
  31,   // 0 relés activos - sin carga
  38,   // 1 relé activo
  45,   // 2 relés activos
  51,   // 3 relés activos
  56    // 4 relés activos
};

// Offset actual seleccionado
static uint16_t offsetActual = 0;

// Contar cuántos relés están activos (bit=0 → ON con lógica invertida)
static uint8_t contarRelesActivos(uint8_t estadoReles) {
  uint8_t count = 0;
  for (uint8_t i = 0; i < 4; i++) {
    // Lógica invertida: bit=0 → ON
    if (((estadoReles >> i) & 0x01) == 1) {
      count++;
    }
  }
  return count;
}

void actualizarOffsetCorriente() {
    // Leer estado actual de los relés
    uint8_t estado = getRelayState();
    
    // Contar cuántos están activos
    uint8_t relesActivos = contarRelesActivos(estado);
    
    // Seleccionar offset correspondiente (0 a 4)
    offsetActual = OFFSETS_CORRIENTE[relesActivos];
}

uint16_t obtenerOffsetCorriente() {
  return offsetActual;
}
