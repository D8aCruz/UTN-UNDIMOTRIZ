#include "lcd.h"
#include "serial.h"         // adcTension, adcCorriente, adcPresion, rpm
#include "procesamiento.h"  // tension, corriente, presion, altura, angulo
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);

static unsigned long ultimoRefresco = 0;
const unsigned long intervalo = 500;  // refresco cada 500 ms

// Flag para decidir si mostrar cuentas crudas o valores convertidos
static bool mostrarCuentas = false;

void inicializarLCD() {
  lcd.init();
  lcd.backlight();
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("   UTN UNDIMOTRIZ   ");
}

void actualizarLCD() {
  if (millis() - ultimoRefresco >= intervalo) {
    char buf[21];
    // Línea 1: Título centrado
    lcd.setCursor(0, 0);
    lcd.print("   UTN UNDIMOTRIZ   ");

    // Línea 2: Tensión (V) | RPM
    // Formato: "  230.9 V   2500 RPM" (20 chars exactos)
    snprintf(buf, sizeof(buf), "%6.1f V %7.0f RPM", tension, rpmProcesado);
    lcd.setCursor(0, 1);
    lcd.print(buf);

    // Línea 3: Corriente (A) | Ángulo (deg)
    // Formato: "  1.234 A     45 DEG" (20 chars exactos)
    snprintf(buf, sizeof(buf), "%6.3f A %7.0f DEG", corriente, angulo);
    lcd.setCursor(0, 2);
    lcd.print(buf);

    // Línea 4: Potencia (W) | Presión (bar)
    // Formato: "  28.46 W  300 CM" (20 chars exactos)
    snprintf(buf, sizeof(buf), "%6.2f W %7.0f CM", potencia, altura);
    lcd.setCursor(0, 3);
    lcd.print(buf);    

    ultimoRefresco = millis();
    imprimirDebugPresion();
  }
}