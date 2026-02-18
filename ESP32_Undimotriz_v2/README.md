# 🟨 ESP32 Undimotriz

> **Módulo pasarela** del sistema Undimotriz.

Firmware para recibir datos del Nano, procesarla para visualización local y reenviarla por TCP a la aplicación de operación.

---

## 🎯 Objetivo

Este firmware corre en **ESP32-WROOM-32** y se encarga de:

- Recibir trama serial cruda desde el Nano.
- Validar y parsear la trama.
- Calcular variables físicas para uso local.
- Mostrar valores en LCD I2C 20x4.
- Reenviar datos por TCP a Qt.
- Reenviar comandos de cargas desde Qt al Nano.

> **Importante**
> Este módulo no adquiere señales analógicas directas ni ejecuta control eléctrico de relés.

---

## 📦 Alcance del módulo

✅ Incluye:

- Puente `Serial2 ↔ TCP`.
- Procesamiento de variables para LCD.
- Red WiFi en modo Access Point.
- Reenvío de comandos de cargas.

❌ No incluye:

- Interfaz de operador de escritorio.
- Almacenamiento histórico.
- Lógica física de adquisición del Nano.

---

## 🧩 Estructura del proyecto

- `src/main.cpp`: arranque y lazo principal.
- `src/fsm.cpp`: máquina de estados.
- `src/serial.cpp`: recepción desde Nano y envío al Nano.
- `src/procesamiento.cpp`: conversiones físicas.
- `src/tcp.cpp`: AP WiFi y servidor TCP.
- `src/lcd.cpp`: salida a pantalla.

Headers en `include/`:

- `fsm.h`, `serial.h`, `procesamiento.h`, `tcp.h`, `lcd.h`

---

## ⚙️ Funcionamiento

### Flujo general

1. Recibe 15 bytes por `Serial2`.
2. Valida `0xAA` y `0x55`.
3. Extrae campos crudos.
4. Calcula variables físicas para LCD.
5. Actualiza pantalla.
6. Reenvía trama cruda por TCP.
7. Si llega comando desde Qt, lo valida y reenvía al Nano.

### Máquina de estados

1. `FSM_LEER_ARDUINO`
2. `FSM_CONVERTIR_MEDICIONES`
3. `FSM_TRANSMITIR_QT`
4. `FSM_ATENDER_COMANDOS`
5. `FSM_REFRESCAR_PANTALLA`
6. `FSM_DIAGNOSTICO_RED`

---

## 📡 Red WiFi y TCP

Modo: **Access Point fijo**

- SSID: `UNDIMOTRIZ`
- Password: `12345678`
- Puerto TCP: `80`
- Canal: `1`
- Máx. conexiones AP: `4`

Si Qt se desconecta, no se guarda histórico y la LCD sigue operativa.

---

## 🔌 Protocolos

### Datos seriales recibidos (Nano → ESP32)

Velocidad: `115200` baudios  
Longitud: **15 bytes**

| Byte | Campo |
|------|-------|
| 0 | `0xAA` inicio |
| 1 | `adcTension` MSB |
| 2 | `adcTension` LSB |
| 3 | `adcCorriente` MSB |
| 4 | `adcCorriente` LSB |
| 5 | `adcPresion` MSB |
| 6 | `adcPresion` LSB |
| 7 | `rpm` MSB |
| 8 | `rpm` LSB |
| 9 | `accelX` MSB |
| 10 | `accelX` LSB |
| 11 | `accelY` MSB |
| 12 | `accelY` LSB |
| 13 | estado relés + sobretensión |
| 14 | `0x55` fin |

Byte 13:

- bits `0..3`: estado de relés.
- bit `7`: bandera de sobretensión.

Si falla validación o hay timeout, la trama se descarta.

### Comando reenviado (Qt/ESP32 → Nano)

Longitud: **3 bytes**

| Byte | Campo |
|------|-------|
| 0 | `0xAA` inicio |
| 1 | comando relés (`0x00` a `0x0F`) |
| 2 | `0x55` fin |

---

## 🧮 Procesamiento local

Variables calculadas:

- `tension`, `corriente`, `potencia`
- `presion`, `altura`
- `angulo` con `atan2(accelY, accelX)`
- `rpmProcesado`

Filtro de RPM:

- si `rpm < 600`, actualiza,
- si `rpm >= 600`, conserva el último valor válido.

---

## 🖥️ LCD y diagnóstico

- LCD I2C `20x4`
- Dirección `0x27`

Muestra variables procesadas para supervisión local.

---

## 🧰 Entorno de compilación

En `platformio.ini`:

- `platform = espressif32`
- `board = esp32dev`
- `framework = arduino`
- `iakop/LiquidCrystal_I2C_ESP32 @ ^1.1.6`

Compilar:

```bash
pio run
```

Cargar:

```bash
pio run -t upload
```

Monitor serie:

```bash
pio device monitor -b 115200
```

---

## 🔗 Integración mínima

Para integrarse al sistema, mantener:

- enlace serial a `115200`,
- delimitadores `0xAA` y `0x55`,
- telemetría de 15 bytes,
- comando de 3 bytes.

---

## ✅ Estado actual

Versión funcional de laboratorio orientada a la comunicación inalámbrica con Qt.