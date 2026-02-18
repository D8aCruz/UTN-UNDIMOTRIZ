#include "procesamiento.h"
#include "serial.h"   // Para acceder a adcTension, adcCorriente, adcPresion, accelX, accelY, rpm

// Variables de salida
float tension   = 0.0;
float corriente = 0.0;
float potencia  = 0.0;
float presion   = 0.0;  // Presión en bar
float altura    = 0.0;  // Altura en cm
float rpmProcesado = 0.0;
float angulo    = 0.0;

// Factores de conversión generales
static const float FACTOR_TENSION   = 0.1077f;
static const float FACTOR_CORRIENTE = 0.00096f;


// Constantes para conversión de presión (sensor 4-20mA, 0-0.6 bar)
static const float MIN_PRESION      = 0.0f;      // bar
static const float MAX_PRESION      = 0.6f;      // bar
static const float MIN_CORRIENTE    = 4.0f;      // mA
static const float Ioffset          = 3.8f;      // mA
static const float MAX_CORRIENTE    = 20.0f;     // mA
static const float DENSIDAD_H2O     = 997.0f;    // kg/m³
static const float GRAVEDAD         = 9.80665f;  // m/s²
static constexpr float CM           = 100.0f;
static constexpr float PASCAL       = 100000.0f;

constexpr float VREF_ADC    = 5.0f;     // Voltaje de referencia del ADC
constexpr float ADC_MAX     = 1023.0f;  // Resolución del ADC (10 bits)

constexpr float Resistencia = 220.0f;   // Resistencia de 220 ohmios para convertir corriente a tensión (V = I * R)

constexpr float Voffset     = 1.25f;      // Referencia LM317, tensión de salida a 0 mA (4 mA - 1.25V) 
constexpr float G           = 2.6969697f; // G = 1 + (R2/R1) = 1 + (5.6kΩ / 3.3kΩ)
constexpr float K           = 2.1212121f; // K = (G-1) * Voffset = 2.1212121V


float cuentasToPresionBar(uint16_t adcCrudo) {
  // 1) ADC cuentas --> Vout
  float Vout = (adcCrudo / ADC_MAX) * VREF_ADC;
  // 2) Vout → Vin (considerando offset y ganancia del sensor)
  float Vshunt = (Vout+K)/G;
  // Tensión → Corriente (mA)
  float corriente_mA = (Vshunt / Resistencia) * 1000.0f;

  // Corriente → Presión (bar)
  float presionBar = ((corriente_mA - (Ioffset)) / (MAX_CORRIENTE - MIN_CORRIENTE)) * (MAX_PRESION - MIN_PRESION) + MIN_PRESION;
  if (presionBar < MIN_PRESION) presionBar = MIN_PRESION;
  if (presionBar > MAX_PRESION) presionBar = MAX_PRESION;
  return presionBar;
}
float convertirPresionPa(float presionBar) {
  // Presión → Altura (cm)
  float presionPascal = presionBar * PASCAL;  // bar → Pascal
  return presionPascal;
}

float presionToAlturaCm(float presionBar) {
  float presionPascal = convertirPresionPa(presionBar);

  float alturaCm = (presionPascal / (DENSIDAD_H2O * GRAVEDAD)) * CM;  // m → cm
  // Evitar valores negativos
  if(alturaCm < 0) alturaCm = 0;
  
  return alturaCm;
}

void procesarDatos() {
  // Conversión directa de cuentas a unidades físicas
  tension   = adcTension   * FACTOR_TENSION;
  corriente = adcCorriente * FACTOR_CORRIENTE;
  potencia  = tension * corriente;  // Potencia en W
  
  // Conversión de cuentas ADC → presion en Bar 
  presion = cuentasToPresionBar(adcPresion);
  altura = presionToAlturaCm(presion);

  // rpm ya está en revoluciones por minuto, se copia directamente si < 600, sino se considera ruido y se pone el valor anterior
  rpmProcesado = (rpm > 600)? rpmProcesado: rpm; 

  // Cálculo del ángulo a partir del acelerómetro
  angulo = atan2((float)accelY, (float)accelX) * 180.0 / PI;
}

void imprimirDebugPresion() {
  float Vout = (adcPresion / ADC_MAX) * VREF_ADC;
  float Vin = (Vout + K) / G;
  float Vshunt = Vin;
  float corriente_mA = (Vshunt / Resistencia) * 1000.0f;
  float presionBar = cuentasToPresionBar(adcPresion);
  float alturaCm = presionToAlturaCm(presionBar);

  Serial.println("=== DEBUG PRESION ===");
  Serial.print("ADC (cuentas): "); Serial.println(adcPresion);
  Serial.print("Vout (V): "); Serial.println(Vout, 4);
  Serial.print("Vin (V): "); Serial.println(Vin, 4);
  Serial.print("Vshunt (V): "); Serial.println(Vshunt, 4);
  Serial.print("Corriente (mA): "); Serial.println(corriente_mA, 4);
  Serial.print("Presion (bar): "); Serial.println(presionBar, 4);
  Serial.print("Altura (cm): "); Serial.println(alturaCm, 2);
  Serial.println("=====================");
}


