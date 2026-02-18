# 🟦 Arduino Undimotriz

> **Módulo de adquisición y control** del sistema Undimotriz.

Firmware embebido para adquirir señales, controlar cargas por relés y enviar datos por puerto serie.

---

## 🎯 Objetivo

Este firmware corre en **Arduino Nano (ATmega328)** y se encarga de:

- Leer señales de tensión, corriente y presión.
- Medir velocidad por pulsos (RPM).
- Leer aceleración en X/Y con MPU6050.
- Aplicar filtrado básico para estabilizar datos.
- Ejecutar comandos de relés recibidos por puerto serie.
- Enviar una trama binaria periódica con datos crudos.

> **Importante**
> Este módulo **no convierte** a unidades físicas (V, A, bar, etc.).

---

## 📦 Alcance del módulo

✅ Incluye:

- Adquisición de señales.
- Control de 4 relés.
- Protección por sobretensión.
- Protocolo serie de datos y comando.

❌ No incluye:

- WiFi o TCP.
- Interfaz gráfica.
- Registro histórico de datos.

---

## 🧩 Estructura del proyecto

- `src/main.cpp`: inicialización y ciclo principal.
- `src/fsm.cpp`: máquina de estados.
- `src/adc.cpp`: lectura ADC y filtrado.
- `src/velocidad.cpp`: RPM por interrupción.
- `src/acelerometro.cpp`: lectura MPU6050.
- `src/comunicacion.cpp`: protocolo serial TX/RX.
- `src/rele.cpp`: control de relés y protección.
- `src/offsetCurrent.cpp`: compensación de offset de corriente.

---

## ⚙️ Funcionamiento

### Máquina de estados

1. `INIT`: inicializa módulos.
2. `ADQUISICION`: toma muestras de sensores.
3. `SINCRONIZACION`: espera evento de envío (cada 5 ms).
4. `TRANSMISION`: valida estado y envía trama.

En cada `loop` también se atienden comandos seriales de relés para reducir latencia.

### Entradas

- Tensión: `A3`
- Corriente: `A7`
- Presión: `A6`

Cada canal usa media móvil de 20 muestras.

### Compensación de corriente

Offset según cargas activas:

- 0 → 31
- 1 → 38
- 2 → 45
- 3 → 51
- 4 → 56

Si el valor queda por debajo del offset, se envía 0.

### Velocidad

- Pin `D2` por interrupción (`RISING`).
- 30 pulsos = 1 revolución.
- Si no hay pulsos por 0.5 s, RPM = 0.

### Acelerómetro

- `accelX`
- `accelY`

---

## 🔌 Protocolo serial

Velocidad: `115200` baudios.

### Telemetría TX (Arduino → receptor)

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

### Comando RX (receptor → Arduino)

Longitud: **3 bytes**

| Byte | Campo |
|------|-------|
| 0 | `0xAA` inicio |
| 1 | comando relés (`0x00` a `0x0F`) |
| 2 | `0x55` fin |

---

## 🛡️ Protección por sobretensión

Umbral: `MAX_ADC_TENSION = 700`, que representa 75V, sabieno que el capacitor de filtrado soporta 80V

Si se supera o iguala:

- se activa la lógica de protección definida en firmware,
- se recalcula offset de corriente,
- se marca el bit 7 en estado.

---

## 🧰 Entorno de compilación

- Framework: Arduino
- Entorno: PlatformIO
- Placa: `nanoatmega328new`
- Librería: `electroniccats/MPU6050 @ ^1.4.4`

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

- `115200` baudios,
- delimitadores `0xAA` y `0x55`,
- orden de bytes de 15/3 bytes.

---

## ✅ Estado actual

Versión funcional de laboratorio para adquisición, control y comunicación serial.