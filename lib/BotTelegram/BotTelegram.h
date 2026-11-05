/*
 * BotTelegram  -  Libreria propia que maneja TODOS los comandos de Telegram
 * ---------------------------------------------------------------------------
 * Recibe los mensajes del bot, interpreta el comando y actua sobre los
 * dispositivos (que recibe por referencia en el constructor).
 *
 * Comandos (punto 4 del enunciado):
 *   /start               -> bienvenida + teclado inline (botones)
 *   /clima               -> temperatura y humedad del DHT22
 *   /releon <seg>        -> prende el rele <seg> segundos y se apaga solo
 *   /releoff             -> apaga el rele
 *   /thingup             -> manda el voltaje del pote a ThingSpeak (+ alerta si supera umbral)
 *   /displayled          -> OLED muestra el estado de los LEDs
 *   /displaypot          -> OLED muestra el voltaje del potenciometro
 *   /displaydht          -> OLED muestra temperatura y humedad
 *   /display<otra cosa>  -> OLED avisa "comando no identificado"
 *
 * Tambien maneja lo que se ve en la OLED (el "modo" de pantalla), porque los
 * comandos /display son los que deciden que se muestra.
 */
#ifndef BOT_TELEGRAM_H
#define BOT_TELEGRAM_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#include "SensorDHT.h"
#include "Potenciometro.h"
#include "Led.h"
#include "PantallaOLED.h"
#include "ClienteThingSpeak.h"

// Referencias a los dispositivos que el bot puede consultar o controlar.
struct Dispositivos {
    SensorDHT &dht;
    Potenciometro &pote;
    Led &rele;
    Led &ledAzul;
    PantallaOLED &pantalla;
    ClienteThingSpeak &nube;
};

// Parametros del bot (vienen de config.h / secrets.h).
struct ConfigBot {
    unsigned long intervaloConsultaMs;   // cada cuanto se piden mensajes nuevos
    unsigned int releSegundosDefecto;    // /releon sin tiempo
    unsigned int releSegundosMax;        // tope de /releon
    unsigned long esperaEntreAlertasMs;  // anti-spam
    const char *chatAlertas;             // "" = ultimo chat que hablo
};

// Que esta mostrando la OLED en este momento.
enum class ModoPantalla { INICIO, LED, POTENCIOMETRO, DHT, ERROR };

class BotTelegram {
public:
    BotTelegram(const char *token, Dispositivos dispositivos, ConfigBot config);

    void iniciar();                      // en setup(), con el WiFi ya conectado
    void atender();                      // en loop(): lee y procesa mensajes nuevos

    void avisar(const String &texto);    // mensaje "espontaneo" (sin que lo pidan)
    void alertaVoltaje(float voltaje);   // alerta de seguridad del potenciometro
    void refrescarPantalla();            // redibuja la OLED segun el modo actual

private:
    WiFiClientSecure clienteSeguro;      // Telegram exige HTTPS (TLS)
    UniversalTelegramBot bot;
    Dispositivos d;
    ConfigBot cfg;

    String ultimoChat;                   // a quien mandar avisos si no hay chatAlertas
    unsigned long ultimaConsulta = 0;
    unsigned long ultimaAlerta = 0;
    bool huboAlerta = false;
    ModoPantalla modo = ModoPantalla::INICIO;
    String comandoInvalido;              // para mostrarlo en la OLED

    void procesarMensaje(const telegramMessage &msg);
    void cmdStart(const String &chat);
    void cmdClima(const String &chat);
    void cmdRele(const String &chat, const String &accion, const String &argumento);
    void cmdThingUp(const String &chat);
    void cmdMenuDisplay(const String &chat);
    void cmdDisplay(const String &chat, const String &subcomando);
    void cmdDesconocido(const String &chat, const String &comando);

    static String normalizar(String texto);
    static String estadoLed(const Led &led);
};

#endif
