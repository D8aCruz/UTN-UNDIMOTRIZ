#ifndef TCP_H
#define TCP_H

#include <Arduino.h>

// Configuración WiFi Access Point
#define AP_SSID "UNDIMOTRIZ"
#define AP_PASSWORD "12345678"
#define AP_CHANNEL 1
#define AP_MAX_CONNECTIONS 4

// Tamaño de la trama cruda enviada por el Nano
#define TRAMA_SIZE 15

// Bytes de inicio y fin del protocolo (los mismos que usa serial.cpp)
#define BYTE_INICIO 0xAA
#define BYTE_FIN    0x55

// Inicializa el servidor TCP (modo Access Point)
void iniciarTCP();

// Muestra estado de la red y clientes conectados (debug por serie)
void mostrarEstadoWiFi();

// Gestiona conexión/desconexión de clientes y recepción de comandos
void recibirComandoTCP();

// Reenvía directamente el buffer crudo recibido en serial.cpp al cliente TCP
void enviarTramaTCP();

#endif