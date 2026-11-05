/*
 * config.h  -  Configuracion general del sistema
 * ---------------------------------------------------------------------------
 * Todo lo que se puede "tocar" sin meterse en la logica esta aca:
 * pines, tiempos, umbral critico, etc. Si el profe pide cambiar un pin o el
 * umbral en la defensa, se cambia SOLO en este archivo.
 */
#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ---- Datos privados (WiFi, token, API keys) -----------------------------------
#if __has_include("secrets.h")
  #include "secrets.h"
#else
  #warning "No existe include/secrets.h -> se usa secrets.example.h (bot y ThingSpeak NO van a andar)"
  #include "secrets.example.h"
#endif

// En el simulador la red siempre es Wokwi-GUEST, sin clave.
#ifdef WOKWI
  #undef WIFI_SSID
  #undef WIFI_PASS
  #define WIFI_SSID "Wokwi-GUEST"
  #define WIFI_PASS ""
#endif

// ---- Pines (punto 1 del enunciado) ---------------------------------------------
const uint8_t PIN_DHT        = 33;  // Sensor DHT22 (dato)
const uint8_t PIN_POTE       = 32;  // Potenciometro -> ADC1 (anda con WiFi prendido)
const uint8_t PIN_LED_VERDE  = 23;  // LED verde = "rele" / actuador principal
const uint8_t PIN_LED_AZUL   = 2;   // LED azul  = indicador de alerta (umbral superado)
const uint8_t PIN_OLED_SDA   = 21;  // I2C datos
const uint8_t PIN_OLED_SCL   = 22;  // I2C reloj
const uint8_t OLED_DIRECCION = 0x3C;

// ---- Tiempos (todo con millis(), nada de delay() en el loop) --------------------
const unsigned long INTERVALO_TELEGRAM_MS     = 1000;   // cada cuanto se consultan mensajes nuevos
const unsigned long INTERVALO_THINGSPEAK_MS   = 20000;  // envio periodico de temp/hum (minimo 15 s)
const unsigned long INTERVALO_VIGILANCIA_MS   = 500;    // cada cuanto se revisa el umbral del pote
const unsigned long INTERVALO_PANTALLA_MS     = 1000;   // refresco de la OLED
const unsigned long ESPERA_ENTRE_ALERTAS_MS   = 10000;  // anti-spam de alertas

// ---- Rele (/releon <segundos>) --------------------------------------------------
const unsigned int RELE_SEGUNDOS_DEFECTO = 5;     // si mandan /releon sin tiempo
const unsigned int RELE_SEGUNDOS_MAX     = 3600;  // tope de seguridad (1 hora)

// ---- Potenciometro (/thingup y alerta automatica) -------------------------------
const float UMBRAL_CRITICO_V = 2.5;  // por encima de esto se dispara la alerta
const float HISTERESIS_V     = 0.2;  // tiene que bajar a 2.3 V para "normalizar"

#endif
