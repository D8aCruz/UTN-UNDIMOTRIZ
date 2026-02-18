# 🟩 Qt Undimotriz

> **Módulo de operación y registro** del sistema Undimotriz.

Aplicación de escritorio en **Qt (C++17)** para monitoreo en tiempo real, control manual de cargas y registro de mediciones.

---

## 🎯 Objetivo

La aplicación se encarga de:

- Conectarse por TCP al sistema.
- Recibir datos binarios.
- Convertir datos crudos a unidades físicas para visualización.
- Mostrar variables y estado de cargas en la interfaz.
- Enviar comandos ON/OFF de cargas.
- Registrar mediciones en CSV.

> **Importante**
> Esta capa no adquiere señales de sensores ni configura la red AP del sistema.

---

## 📦 Alcance de la aplicación

✅ Incluye:

- GUI de operación.
- Parseo de tramas.
- Conversión de variables.
- Control de 4 cargas.
- Grabación de datos.

❌ No incluye:

- Control eléctrico directo de relés.
- Funciones de firmware embebido.
- Persistencia automática en base de datos.

---

## ⚙️ Flujo funcional

1. El usuario conecta por TCP.
2. La app recibe tramas de 15 bytes.
3. Valida formato y parsea campos.
4. Calcula variables para la UI.
5. Actualiza estado de cargas y alertas.
6. Si el usuario acciona cargas, envía comando de 3 bytes.
7. Si la grabación está activa, registra y exporta CSV.

---

## 🌐 Red y conexión

Parámetros actuales:

- Host: `192.168.4.1`
- Puerto: `80`
- Modo: conexión fija

Comportamiento:

- al conectar, habilita mediciones y control,
- al desconectar, deshabilita controles y limpia visualización.

---

## 🔌 Protocolos

### Telemetría RX (ESP32 → Qt)

Longitud: **15 bytes**

| Byte | Campo |
|------|-------|
| 0 | `0xAA` inicio |
| 1 | tensión ADC MSB |
| 2 | tensión ADC LSB |
| 3 | corriente ADC MSB |
| 4 | corriente ADC LSB |
| 5 | presión ADC MSB |
| 6 | presión ADC LSB |
| 7 | RPM MSB |
| 8 | RPM LSB |
| 9 | accelX MSB |
| 10 | accelX LSB |
| 11 | accelY MSB |
| 12 | accelY LSB |
| 13 | estado digital |
| 14 | `0x55` fin |

Byte 13:

- bits `0..3`: estado de cargas.
- bit `7`: alerta de sobretensión.

Se procesa solo si la trama es válida (`0xAA` ... `0x55`).

### Comando TX (Qt → ESP32/Nano)

Longitud: **3 bytes**

| Byte | Campo |
|------|-------|
| 0 | `0xAA` inicio |
| 1 | estado de cargas (`0x00` a `0x0F`) |
| 2 | `0x55` fin |

Mapeo:

- bit 0 → CARGA 1
- bit 1 → CARGA 2
- bit 2 → CARGA 3
- bit 3 → CARGA 4

Convención:

- bit = 1 → activada
- bit = 0 → desactivada

---

## 🛡️ Sobretensión

La alerta llega por bit 7 de datos.

Si está activa, la interfaz restringe el control manual para no contradecir el modo de protección del sistema.

---

## 📊 Variables mostradas

- Tensión (V)
- Corriente (A)
- Potencia (W)
- Presión (bar)
- Altura equivalente (cm)
- RPM
- Ángulo (deg) con `atan2(accelY, accelX)`
- Estado ON/OFF de cargas
- Estado de conexión y alerta

---

## 🧮 Conversión en Qt

- tensión y corriente por factor lineal,
- presión por cadena ADC → señal acondicionada → bar,
- altura derivada de presión,
- potencia = tensión × corriente,
- ángulo por trigonometría 2D.

Criterio RPM:

- si `RPM <= 600`, se actualiza,
- si supera 600, mantiene último valor válido.

---

## 💾 Registro de mediciones

Parámetros:

- período: **5 ms**,
- máximo por sesión: **60000** muestras,
- salida: **CSV** separado por `;`,
- codificación: UTF-8 con BOM.

Columnas:

- `Tiempo(ms)`, `Tension(V)`, `Corriente(A)`, `Potencia(W)`
- `Presion(Bar)`, `Altura(cm)`, `RPM`, `Angulo(deg)`

---

## 🧩 Estructura del proyecto

- `main.cpp`: inicio de aplicación.
- `mainwindow.h/.cpp`: UI, TCP, parseo, control y CSV.
- `pressureconverter.h/.cpp`: conversión de presión y altura.
- `mainwindow.ui`: interfaz.
- `resources.qrc`: recursos.
- `Qt-Undimotriz.pro`: configuración qmake.

---

## 🧰 Entorno de compilación

Requisitos:

- Qt 6.x (Widgets + Network)
- Compilador C++17

Pasos:

1. Abrir `Qt-Undimotriz.pro` en Qt Creator.
2. Seleccionar kit.
3. Compilar y ejecutar.

---

## 📝 Notas

- La pestaña Log es para diagnóstico técnico.
- El botón visual XLS no exporta XLSX.
- El formato operativo de exportación es CSV.

---

## 🔗 Integración mínima

Para operar correctamente:

- enlace TCP activo,
- telemetría de 15 bytes con delimitadores válidos,
- comando de cargas en 3 bytes.

---

## ✅ Estado actual

Versión funcional de laboratorio enfocada en operación y captura de datos.