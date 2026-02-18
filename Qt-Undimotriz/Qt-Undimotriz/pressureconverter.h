#ifndef PRESSURECONVERTER_H
#define PRESSURECONVERTER_H

#include <cstdint>

// ===== Constantes físicas y de sensor =====

/**
 * @class PressureConverter
 * @brief Maneja la conversión de ADC a unidades físicas para el sensor de presión.
 *
 * Realiza las siguientes conversiones:
 * ADC (0-1023) → Voltaje → Corriente (4-20mA) → Presión (bar) → Altura (cm)
 *
 * Incluye filtro de media móvil de 30 muestras para reducir ruido.
 */
class PressureConverter
{
public:
    PressureConverter();
    ~PressureConverter();

    /**
     * @brief Convierte valor ADC de presión a altura en centímetros
     * @param adcValue Valor ADC (0-1023)
     * @return Altura en centímetros
     */
    double convertirPresionBar(uint16_t adcValue);
    double convertirPresionPa(double presionBar);
    double convertirAlturaCm(double presionBar);
    /**
     * @brief Obtiene la presión actual en Pascales
     * @return Presión en Pa
     */
    double obtenerPresionPa() const;

    /**
     * @brief Obtiene la presión actual en bar
     * @return Presión en bar
     */
    double obtenerPresionBar() const;

    /**
     * @brief Obtiene la altura actual en centímetros
     * @return Altura en cm
     */
    double obtenerAlturaCm() const;

private:
    // Constantes de conversión (4-20mA sensor)
    static constexpr double RESISTENCIA_SENSOR = 220.0; // Ohmios
    static constexpr double MIN_PRESION_BAR = 0.0;      // bar
    static constexpr double MAX_PRESION_BAR = 0.6;      // bar
    static constexpr double MIN_CORRIENTE = 4.0;        // mA
    static constexpr double Ioffset = 3.8;              // mA
    static constexpr double MAX_CORRIENTE = 20.0;       // mA
    static constexpr double DENSIDAD_AGUA = 997.0;      // kg/m³
    static constexpr double GRAVEDAD = 9.80665;         // m/s²
    static constexpr double CM = 100;
    static constexpr double PASCAL = 100000;
    const float VREF_ADC = 5.0f;    // idealmente calibrable
    const float Voffset = 1.25f;    // Voltios generada por el LM317
    const float G1 = 2.697f;        // Ganancia del sensor G1 = (1 + Rf/Ri), tal que Rf=5.6k, Ri=1k
    const float G2 = 1.697f; // Ganancia del sensor G2 = Rf/Ri
    // Filtro de media móvil
    static constexpr int BUFFER_SIZE = 30;
    uint16_t bufferAdc[BUFFER_SIZE];
    int indiceBuffer;
    uint32_t sumaBuffer;
    bool filtroHabilitado;

    // Valores actuales
    double presionActualPa;
    double presionActualBar;
    double alturaActualCm;
};

#endif // PRESSURECONVERTER_H
