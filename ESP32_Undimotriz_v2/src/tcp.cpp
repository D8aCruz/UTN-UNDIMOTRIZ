#include "tcp.h"
#include "serial.h"       // buffer[], adcTension, adcCorriente, adcPresion, rpm, accelX, accelY, estadoReles
#include "procesamiento.h" // angulo, altura
#include <WiFi.h>

WiFiServer server(80);
WiFiClient cliente;

void iniciarTCP() {
  WiFi.softAP(AP_SSID, AP_PASSWORD, AP_CHANNEL, 0, AP_MAX_CONNECTIONS);

  IPAddress IP = WiFi.softAPIP();
  Serial.println("=== SERVIDOR WiFi INICIADO ===");
  Serial.print("Red WiFi: "); Serial.println(AP_SSID);
  Serial.print("Password: "); Serial.println(AP_PASSWORD);
  Serial.print("IP Servidor: "); Serial.println(IP);
  Serial.print("Puerto TCP: 80\n");
  Serial.println("Esperando conexiones...");
  Serial.println("===============================");

  server.begin();
}

void mostrarEstadoWiFi() {
  static unsigned long ultimoEstado = 0;

  if (millis() - ultimoEstado > 3000) { // cada 3 segundos
    int clientesConectados = WiFi.softAPgetStationNum();
    Serial.println("=== ESTADO WiFi ===");
    Serial.print("Clientes conectados a la red: ");
    Serial.println(clientesConectados);

    if (cliente && cliente.connected()) {
      Serial.println("Cliente TCP: CONECTADO");
    } else {
      Serial.println("Cliente TCP: DESCONECTADO");
    }

    // Debug: mostrar datos crudos que ya parsea serial.cpp
    Serial.println("=== MEDICIONES CRUDAS ===");
    Serial.print("Tension ADC: ");   Serial.println(adcTension);
    Serial.print("Corriente ADC: "); Serial.println(adcCorriente);
    Serial.print("Presion ADC: ");   Serial.println(adcPresion);
    Serial.print("RPM: ");           Serial.println(rpm);
    Serial.print("AccelX: ");        Serial.println(accelX);
    Serial.print("AccelY: ");        Serial.println(accelY);
    Serial.print("Angulo: ");        Serial.print(angulo); Serial.println(" grados");
    Serial.print("Altura: ");        Serial.print(altura); Serial.println(" cm");
    Serial.println("=========================");

    ultimoEstado = millis();
  }
}

void recibirComandoTCP() {
  // Conexión/desconexión
  if (!cliente || !cliente.connected()) {
    WiFiClient nuevoCliente = server.available();
    if (nuevoCliente) {
      cliente = nuevoCliente;
      Serial.println("=== CLIENTE CONECTADO ===");
      Serial.print("IP del cliente: "); Serial.println(cliente.remoteIP());
    }
  }

  if (cliente && !cliente.connected()) {
    Serial.println("=== CLIENTE DESCONECTADO ===");
    cliente.stop();
  }

  // Recepción de comandos desde Qt
  if (cliente && cliente.connected() && cliente.available() >= 3) {
    uint8_t comando[3];
    size_t bytesLeidos = cliente.readBytes(comando, 3);

    // Validar con los mismos bytes de inicio/fin que usa el protocolo
    if (bytesLeidos == 3 && comando[0] == BYTE_INICIO && comando[2] == BYTE_FIN) {
      uint8_t estadoRelesNuevo = comando[1];

      Serial.print("Comando recibido desde Qt, nuevo estado de relés: ");
      Serial.println(estadoRelesNuevo, HEX);

      // Actualizar variable y usar función del módulo serial
      estadoRelesSolicitado = estadoRelesNuevo;
      enviarComando();
    }
  }
}

void enviarTramaTCP() {
  if (cliente && cliente.connected()) {
    // Reenviar directamente el buffer crudo que llenó serial.cpp
    cliente.write(buffer, TRAMA_SIZE);

    // Debug opcional: mostrar la trama en hex
    Serial.println("=== TRAMA REENVIADA ===");
    for (int i = 0; i < TRAMA_SIZE; i++) {
      Serial.print(buffer[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
    Serial.println("=======================");
  }
}