#include "pressureconverter.h"
#include <cstring>

PressureConverter::PressureConverter()
    : indiceBuffer(0), sumaBuffer(0), filtroHabilitado(true),
    presionActualPa(0.0), presionActualBar(0.0), alturaActualCm(0.0)
{
    // Inicializa buffer de media móvil
    std::memset(bufferAdc, 0, sizeof(bufferAdc));
}

PressureConverter::~PressureConverter()
{
}

double PressureConverter::convertirPresionBar(uint16_t adcValue)
{
    double Vout = (static_cast<double>(adcValue)/1023.0f) * VREF_ADC;
    double Vin  = (Vout+(G2*Voffset))/G1;

    // PASO 2: Voltaje → Corriente (4-20mA)
    // V = I * R → I = V / R
    double corriente_mA = (Vin / RESISTENCIA_SENSOR) * 1000.0f;

    // Asegura que la corriente esté dentro del rango válido

    presionActualBar = ((corriente_mA-Ioffset)/(MAX_CORRIENTE-MIN_CORRIENTE))*0.6;
    if(presionActualBar<MIN_PRESION_BAR) presionActualBar = MIN_PRESION_BAR;
    if(presionActualBar>MAX_PRESION_BAR) presionActualBar = MAX_PRESION_BAR;

    return presionActualBar;
}

double PressureConverter::convertirAlturaCm(double presionBar)
{
    presionActualPa = convertirPresionPa(presionBar);

    alturaActualCm = (presionActualPa/(DENSIDAD_AGUA*GRAVEDAD))*CM;
    // Evitar valores negativos
    if(alturaActualCm < 0) alturaActualCm = 0;
    return alturaActualCm;
}

double PressureConverter::convertirPresionPa(double presionBar)
{
    presionActualPa = presionBar * PASCAL;
    return presionActualPa;
}

double PressureConverter::obtenerPresionPa() const
{
    return presionActualPa;
}

double PressureConverter::obtenerPresionBar() const
{
    return presionActualBar;
}

double PressureConverter::obtenerAlturaCm() const
{
    return alturaActualCm;
}


