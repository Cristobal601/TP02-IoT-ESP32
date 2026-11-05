/*
 * ===========================================================================
 *  TP Integrador - Tecnologias para la Automatizacion (TA) 4k01 - TP02/26
 *  UTN FRC
 * ===========================================================================
 *  Sistema embebido con ESP32 para supervision ambiental y actuacion remota:
 *    - DHT22 (temp/hum) ............... GPIO 33
 *    - Potenciometro (ADC) ............ GPIO 32
 *    - LED verde ("rele") ............. GPIO 23
 *    - LED azul (indicador de alerta) . GPIO 2
 *    - OLED I2C ....................... SDA 21 / SCL 22
 *    - Bot de Telegram (comandos + teclado inline + alertas)
 *    - ThingSpeak (envio periodico de temp/hum y voltaje con /thingup)
 *
 *  Este archivo solo ORQUESTA: crea los objetos y en el loop() le da un
 *  "turno" a cada uno. Toda la logica esta en las librerias de lib/:
 *    lib/Led, lib/SensorDHT, lib/Potenciometro, lib/PantallaOLED,
 *    lib/ConexionWiFi, lib/ClienteThingSpeak, lib/BotTelegram
 *
 *  Regla de oro: el loop() NUNCA se bloquea (nada de delay()). Cada tarea
 *  se fija con millis() si ya le toca ejecutarse; si no, sigue de largo.
 * ===========================================================================
 */
#include <Arduino.h>
#include "config.h"

#include "Led.h"
#include "SensorDHT.h"
#include "Potenciometro.h"
#include "PantallaOLED.h"
#include "ConexionWiFi.h"
#include "ClienteThingSpeak.h"
#include "BotTelegram.h"

// ---- Dispositivos (una libreria por dispositivo) ----------------------------
Led rele(PIN_LED_VERDE, "Rele (LED verde)");
Led ledAzul(PIN_LED_AZUL, "LED azul");
SensorDHT dht(PIN_DHT);
Potenciometro pote(PIN_POTE, UMBRAL_CRITICO_V, HISTERESIS_V);
PantallaOLED pantalla(OLED_DIRECCION);

// ---- Comunicaciones ----------------------------------------------------------
ConexionWiFi wifi(WIFI_SSID, WIFI_PASS);
ClienteThingSpeak nube(THINGSPEAK_CHANNEL_ID, THINGSPEAK_WRITE_KEY, INTERVALO_THINGSPEAK_MS);

BotTelegram bot(TELEGRAM_TOKEN,
                {dht, pote, rele, ledAzul, pantalla, nube},
                {INTERVALO_TELEGRAM_MS, RELE_SEGUNDOS_DEFECTO, RELE_SEGUNDOS_MAX,
                 ESPERA_ENTRE_ALERTAS_MS, TELEGRAM_CHAT_ID_ALERTAS});

// ---- Temporizadores del loop -----------------------------------------------------
unsigned long ultimaVigilancia = 0;
unsigned long ultimoRefrescoPantalla = 0;

void vigilarPotenciometro();

// =============================================================================
void setup() {
    Serial.begin(115200);
    Serial.println("\n=== TP02 IoT - ESP32 UTN ===");

    rele.iniciar();
    ledAzul.iniciar();
    pote.iniciar();
    dht.iniciar();
    pantalla.iniciar(PIN_OLED_SDA, PIN_OLED_SCL);

    pantalla.mostrar("TP02 IoT - UTN", "Conectando WiFi...", WIFI_SSID);
    if (!wifi.conectar()) {
        pantalla.mostrar("TP02 IoT - UTN", "Sin WiFi", "Reintentando...");
    }

    nube.iniciar();
    bot.iniciar();
    bot.refrescarPantalla();
    Serial.println("Sistema listo. Mandale /start al bot.");
}

// =============================================================================
void loop() {
    wifi.mantener();          // reconecta si se cayo la red
    dht.actualizar();         // lee el DHT22 cada 2,5 s

    if (rele.actualizar()) {  // true = se cumplio el tiempo y se apago solo
        bot.avisar("⏱️ El relé se apagó automáticamente.");
    }

    vigilarPotenciometro();   // alerta automatica si supera el umbral

    nube.actualizar(dht.temperatura(), dht.humedad(), dht.hayLectura());

    bot.atender();            // procesa comandos de Telegram

    if (millis() - ultimoRefrescoPantalla >= INTERVALO_PANTALLA_MS) {
        ultimoRefrescoPantalla = millis();
        bot.refrescarPantalla();
    }
}

// -----------------------------------------------------------------------------
// Revisa el potenciometro cada INTERVALO_VIGILANCIA_MS. Si cruza el umbral,
// el ESP32 manda la alerta por Telegram SIN que el usuario la pida, y prende
// el LED azul mientras dure la condicion critica.
// -----------------------------------------------------------------------------
void vigilarPotenciometro() {
    if (millis() - ultimaVigilancia < INTERVALO_VIGILANCIA_MS) return;
    ultimaVigilancia = millis();

    switch (pote.revisarUmbral()) {
        case EventoUmbral::SUPERADO:
            ledAzul.encender();
            Serial.println("[Pote] Umbral critico SUPERADO");
            bot.alertaVoltaje(pote.voltaje());
            break;
        case EventoUmbral::NORMALIZADO:
            ledAzul.apagar();
            Serial.println("[Pote] Voltaje normalizado");
            bot.avisar("✅ El voltaje del potenciómetro volvió a la normalidad (" +
                       String(pote.voltaje(), 2) + " V).");
            break;
        case EventoUmbral::NINGUNO:
            break;
    }
}
