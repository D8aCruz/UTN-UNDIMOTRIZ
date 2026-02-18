#include "serial.h"
#include "tcp.h"

// Definición del buffer compartido
uint8_t buffer[15];

// Definición de variables globales
volatile uint16_t adcTension   = 0;
volatile uint16_t adcCorriente = 0;
volatile uint16_t adcPresion   = 0;
volatile uint16_t rpm          = 0;
volatile int16_t accelX        = 0;
volatile int16_t accelY        = 0;
volatile uint8_t estadoRelesRecibido   = 0;
volatile uint8_t estadoRelesSolicitado = 0;

volatile uint8_t relayCommand  = 0;

static int16_t offset_X = 0;
static int16_t offset_Y = 0;

void inicializarSerial() {
    Serial2.begin(115200, SERIAL_8N1, 16, 17); // RX=GPIO16, TX=GPIO17
    //Serial.println("[SERIAL] Serial2 inicializado: RX=16, TX=17, 115200 baud");
}

bool recibirTrama(unsigned long timeout) {
    unsigned long inicio = millis();
    
    // PASO 1: Buscar byte de inicio 0xAA
    bool inicioEncontrado = false;
    while (!inicioEncontrado) {
        if (millis() - inicio > timeout) {
            return false; // Timeout
        }
        
        if (Serial2.available() > 0) {
            uint8_t byte = Serial2.read();
            if (byte == 0xAA) {
                buffer[0] = byte;
                inicioEncontrado = true;
                //Serial.println("[SERIAL] Byte de inicio 0xAA encontrado!");
            }
        }
    }
    
    // PASO 2: Leer los siguientes 14 bytes
    inicio = millis(); // Reset timeout
    while (Serial2.available() < TRAMA_SIZE - 1) {
        if (millis() - inicio > timeout) {
            //Serial.println("[SERIAL] Timeout esperando 14 bytes restantes");
            return false;
        }
    }
    
    Serial2.readBytes(&buffer[1], TRAMA_SIZE - 1); // Leer bytes 1-15

    if (buffer[0] == 0xAA && buffer[14] == 0x55) {
        adcTension   = (buffer[1] << 8) | buffer[2];
        adcCorriente = (buffer[3] << 8) | buffer[4];
        adcPresion   = (buffer[5] << 8) | buffer[6];
        rpm          = (buffer[7] << 8) | buffer[8];
        accelX       = ((buffer[9] << 8) | buffer[10]) - offset_X;
        accelY       = ((buffer[11] << 8) | buffer[12]) - offset_Y;
        estadoRelesRecibido = buffer[13];
    
        //Serial.println("[SERIAL] Trama VALIDA recibida");
        return true; // Trama válida recibida
    }
    
    //Serial.println("[SERIAL] Trama INVALIDA (inicio o fin incorrecto)");
    return false; // Trama inválida
}

void enviarComando() {
    uint8_t buffer[3];
    buffer[0] = 0xAA;
    buffer[1] = estadoRelesSolicitado;
    buffer[2] = 0x55;

    Serial2.write(buffer, 3);
}

