# 🧭 Sistema Undimotriz

> Documento central del proyecto.

Este README resume el sistema completo, sus módulos, herramientas necesarias y créditos del equipo.

---

## 📌 Nombre del proyecto

**Sistema Undimotriz — Plataforma Integrada de Adquisición, Pasarela y Monitoreo**

---

## 🧩 Resumen del sistema

El proyecto está compuesto por **tres módulos** que trabajan en cadena:

1. **Arduino Undimotriz v3**
   - Adquiere señales de sensores.
   - Controla relés de cargas.
   - Envía telemetría cruda por serial.

2. **ESP32 Undimotriz v2**
   - Recibe telemetría del Arduino.
   - Procesa variables para visualización local.
   - Publica datos por TCP y reenvía comandos.

3. **Qt Undimotriz**
   - Visualiza variables en tiempo real.
   - Permite control manual de cargas.
   - Registra mediciones en CSV.

---

## 🔄 Flujo de datos (alto nivel)

`Sensores + Cargas ↔ Arduino → ESP32 (Serial) → Qt (TCP)`

`Qt → ESP32 (TCP) → Arduino (Serial) → Relés`

---

## 🧰 Herramientas necesarias

Estas son las herramientas para **compilar, cargar y ejecutar** todo el sistema:

### Firmware Arduino y ESP32

- **Visual Studio Code**
- **Extensión PlatformIO IDE**
- **Toolchain de PlatformIO** (instalación automática)
- **Cable USB** para carga de firmware en cada placa

### Aplicación de escritorio

- **Qt 6.x** (módulos Widgets + Network)
- **Qt Creator**
- **Compilador C++17** compatible con tu kit de Qt

### Hardware mínimo

- **Arduino Nano (ATmega328)**
- **ESP32-WROOM-32**
- Sensores del banco (tensión/corriente/presión/velocidad/aceleración)
- Etapa de relés y cargas
- LCD I2C 20x4

---

## ✅ Qué explica cada README

Para mantener documentación limpia y sin duplicaciones:

- `README Arduino`: funcionamiento interno de adquisición, control y protocolo serial.
- `README ESP32`: funcionamiento de pasarela, red, procesamiento local y puente de comandos.
- `README Qt`: funcionamiento de interfaz, conversión, control de cargas y registro CSV.

Este README general se enfoca solo en **visión global, requisitos y créditos**.

---

## 👥 Créditos del proyecto

### Integrantes

- **Calero Costa, Diego Andrés**
- **Ochoa Cruz, David**

### Profesor

- **Ing. Oscar Pugliese**

### Ayudante

- **Diego Pirotta**

---

## 📎 Notas finales

- Mantener sincronizados los tres módulos en protocolo (`15/3 bytes`, `0xAA/0x55`, `115200`).
- Cualquier cambio de calibración o formato de trama debe reflejarse en los tres README técnicos.

---

## 🟢 Estado del proyecto

Proyecto integrado funcional para laboratorio, con arquitectura modular y operación en tiempo real.


