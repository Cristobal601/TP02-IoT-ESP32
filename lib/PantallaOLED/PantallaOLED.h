/*
 * PantallaOLED  -  Libreria propia para la pantalla OLED 128x64 por I2C
 * ---------------------------------------------------------------------------
 * Funciona con los DOS controladores que nos vamos a cruzar:
 *   - SH1106  -> la pantalla de la placa UTN (igual que en las practicas)
 *   - SSD1306 -> la unica OLED 128x64 que existe en Wokwi
 * Se elige con un build_flag en platformio.ini (OLED_SH1106 / OLED_SSD1306).
 * Si no se define ninguno, se usa SH1106 (placa real).
 *
 * Resuelve los problemas tipicos de mensajes en el display:
 *   - La fuente por defecto NO tiene tildes, ñ ni °: se reemplazan (á->a, ñ->n...)
 *   - Entran 21 caracteres por linea: se recortan para que no se "rompa" el layout
 *   - Siempre se borra la pantalla antes de dibujar (no quedan restos)
 *
 * Layout (tamaño 1 = 6x8 px por letra):
 *   +-----------------------+
 *   | TITULO (barra blanca) |  y = 0..10
 *   | linea 1               |  y = 14
 *   | linea 2               |  y = 26
 *   | linea 3               |  y = 38
 *   | linea 4               |  y = 50
 *   +-----------------------+
 */
#ifndef PANTALLA_OLED_H
#define PANTALLA_OLED_H

#include <Arduino.h>
#include <Wire.h>

#if !defined(OLED_SSD1306) && !defined(OLED_SH1106)
  #define OLED_SH1106
#endif

#if defined(OLED_SH1106)
  #include <Adafruit_SH110X.h>
  typedef Adafruit_SH1106G DriverOLED;
  #define OLED_BLANCO SH110X_WHITE
  #define OLED_NEGRO  SH110X_BLACK
#else
  #include <Adafruit_SSD1306.h>
  typedef Adafruit_SSD1306 DriverOLED;
  #define OLED_BLANCO SSD1306_WHITE
  #define OLED_NEGRO  SSD1306_BLACK
#endif

class PantallaOLED {
public:
    static const uint8_t ANCHO = 128;
    static const uint8_t ALTO = 64;
    static const uint8_t CARACTERES_POR_LINEA = 21;

    explicit PantallaOLED(uint8_t direccion = 0x3C);

    bool iniciar(uint8_t pinSDA, uint8_t pinSCL);
    bool disponible() const;

    // Titulo + hasta 4 lineas de texto chico.
    void mostrar(const String &titulo,
                 const String &linea1 = "", const String &linea2 = "",
                 const String &linea3 = "", const String &linea4 = "");

    // Titulo + un valor grande en el centro + una linea al pie.
    void mostrarValor(const String &titulo, const String &valor, const String &pie = "");

    // Deja el texto apto para la OLED: sin tildes y recortado a 21 caracteres.
    static String adaptar(String texto);

private:
    DriverOLED display;
    uint8_t direccion;
    bool ok = false;

    void dibujarTitulo(const String &titulo);
};

#endif
