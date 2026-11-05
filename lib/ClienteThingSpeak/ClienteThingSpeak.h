/*
 * ClienteThingSpeak  -  Libreria propia para subir datos a ThingSpeak (HTTP)
 * ---------------------------------------------------------------------------
 * Campos del canal:
 *   field1 = temperatura (°C)      -> envio periodico automatico
 *   field2 = humedad (%)           -> envio periodico automatico
 *   field3 = voltaje del pote (V)  -> cuando se manda /thingup
 *
 * LIMITE IMPORTANTE: la cuenta gratis de ThingSpeak acepta 1 escritura cada
 * 15 segundos. Si se escribe antes, el envio se rechaza. Esta clase lo
 * respeta: si se pide un envio "demasiado pronto", lo deja ENCOLADO y lo
 * manda apenas se pueda.
 */
#ifndef CLIENTE_THINGSPEAK_H
#define CLIENTE_THINGSPEAK_H

#include <Arduino.h>
#include <WiFi.h>

enum class ResultadoEnvio { ENVIADO, ENCOLADO, ERROR, SIN_CONFIGURAR };

class ClienteThingSpeak {
public:
    static const uint8_t FIELD_TEMPERATURA = 1;
    static const uint8_t FIELD_HUMEDAD = 2;
    static const uint8_t FIELD_VOLTAJE = 3;

    ClienteThingSpeak(unsigned long canal, const char *writeKey, unsigned long intervaloMs);

    void iniciar();

    // Llamar en cada vuelta del loop() con el ultimo clima medido.
    // Hace el envio periodico y despacha lo que haya quedado encolado.
    void actualizar(float temperatura, float humedad, bool climaValido);

    // Pedido de /thingup: manda el voltaje ya, o lo encola si no pasaron 15 s.
    ResultadoEnvio enviarVoltaje(float voltaje);

    unsigned long segundosParaPoderEnviar() const;
    int ultimoCodigo() const;   // 200 = OK (codigo HTTP que devuelve ThingSpeak)

private:
    WiFiClient cliente;         // HTTP comun (ThingSpeak acepta sin TLS)
    unsigned long canal;
    const char *writeKey;
    unsigned long intervaloMs;

    unsigned long ultimoEnvio = 0;
    bool huboEnvio = false;
    bool voltajePendiente = false;
    float voltaje = 0;
    float temperatura = NAN;
    float humedad = NAN;
    bool climaValido = false;
    int codigo = 0;

    static const unsigned long MIN_ENTRE_ENVIOS_MS = 16000;  // 15 s + margen

    bool configurado() const;
    bool puedeEnviar() const;
    bool escribir(bool incluirVoltaje);
};

#endif
