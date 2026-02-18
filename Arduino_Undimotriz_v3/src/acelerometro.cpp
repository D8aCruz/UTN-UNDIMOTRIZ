#include "acelerometro.h"
#include <Wire.h>
#include <MPU6050.h>

// Instancia del acelerómetro
MPU6050 accelgyro;

volatile int16_t accelX = 0;
volatile int16_t accelY = 0;

void inicializarAcelerometro() {
    Wire.begin();
    accelgyro.initialize();
    // Inicialización silenciosa, sin mensajes por serial
}

void leerAcelerometro() {
    int16_t ax, ay, az;
    accelgyro.getAcceleration(&ax, &ay, &az);

    accelX = ax;
    accelY = ay;
}