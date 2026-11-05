#include "ClienteThingSpeak.h"
#include <ThingSpeak.h>

ClienteThingSpeak::ClienteThingSpeak(unsigned long canal, const char *writeKey,
                                     unsigned long intervaloMs)
    : canal(canal), writeKey(writeKey), intervaloMs(intervaloMs) {}

void ClienteThingSpeak::iniciar() {
    ThingSpeak.begin(cliente);
    if (!configurado()) {
        Serial.println("[ThingSpeak] Sin configurar (completar include/secrets.h)");
    }
}

void ClienteThingSpeak::actualizar(float t, float h, bool valido) {
    temperatura = t;
    humedad = h;
    climaValido = valido;

    if (!configurado() || !puedeEnviar()) return;

    if (voltajePendiente) {
        escribir(true);   // primero lo que pidio el usuario con /thingup
    } else if (climaValido && (!huboEnvio || millis() - ultimoEnvio >= intervaloMs)) {
        escribir(false);  // envio periodico de temperatura y humedad
    }
}

ResultadoEnvio ClienteThingSpeak::enviarVoltaje(float v) {
    if (!configurado()) return ResultadoEnvio::SIN_CONFIGURAR;
    voltaje = v;
    if (!puedeEnviar()) {
        voltajePendiente = true;
        return ResultadoEnvio::ENCOLADO;
    }
    return escribir(true) ? ResultadoEnvio::ENVIADO : ResultadoEnvio::ERROR;
}

unsigned long ClienteThingSpeak::segundosParaPoderEnviar() const {
    if (puedeEnviar()) return 0;
    return (MIN_ENTRE_ENVIOS_MS - (millis() - ultimoEnvio) + 999) / 1000;
}

int ClienteThingSpeak::ultimoCodigo() const { return codigo; }

bool ClienteThingSpeak::configurado() const { return canal != 0; }

bool ClienteThingSpeak::puedeEnviar() const {
    return !huboEnvio || millis() - ultimoEnvio >= MIN_ENTRE_ENVIOS_MS;
}

bool ClienteThingSpeak::escribir(bool incluirVoltaje) {
    // Se marca el intento aunque falle, para respetar el limite de 15 s.
    ultimoEnvio = millis();
    huboEnvio = true;
    if (incluirVoltaje) voltajePendiente = false;

    if (WiFi.status() != WL_CONNECTED) {
        codigo = -1;
        Serial.println("[ThingSpeak] Sin WiFi, no se envia");
        return false;
    }

    // Se mandan juntos todos los campos disponibles en UNA sola escritura.
    if (climaValido) {
        ThingSpeak.setField(FIELD_TEMPERATURA, temperatura);
        ThingSpeak.setField(FIELD_HUMEDAD, humedad);
    }
    if (incluirVoltaje) {
        ThingSpeak.setField(FIELD_VOLTAJE, voltaje);
    }

    codigo = ThingSpeak.writeFields(canal, writeKey);

    if (codigo == 200) {
        Serial.printf("[ThingSpeak] OK -> T=%.1f C  H=%.1f %%%s\n", temperatura, humedad,
                      incluirVoltaje ? (String("  V=") + String(voltaje, 2)).c_str() : "");
        return true;
    }
    Serial.printf("[ThingSpeak] Error, codigo %d\n", codigo);
    return false;
}
