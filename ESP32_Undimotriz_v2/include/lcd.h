#ifndef LCD_H
#define LCD_H

#include <Arduino.h>

// Configuración LCD I2C
#define LCD_I2C_ADDR 0x27
#define LCD_COLS 20
#define LCD_ROWS 4

// Inicializa el LCD
void inicializarLCD();

// Actualiza las líneas del LCD con los valores procesados
void actualizarLCD();

#endif