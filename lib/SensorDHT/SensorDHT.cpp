#include "SensorDHT.h"

// OJO: el tipo es DHT22 (en una practica de clase quedo DHT21 por error).
SensorDHT::SensorDHT(uint8_t pin) : sensor(pin, DHT22) {}

void SensorDHT::iniciar() {
    // begin() se llama aca y no en el constructor: los objetos globales se
    // construyen ANTES de setup(), cuando el hardware todavia no esta listo.
    sensor.begin();
}

void SensorDHT::actualizar() {
    if (!primerIntento && millis() - ultimoIntento < INTERVALO_MS) return;
    primerIntento = false;
    ultimoIntento = millis();

    float t = sensor.readTemperature();
    float h = sensor.readHumidity();

    if (isnan(t) || isnan(h)) {
        // Lectura fallida: se conserva la ultima buena.
        Serial.println("[DHT22] Error de lectura (se mantiene el ultimo valor valido)");
        return;
    }

    temp = t;
    hum = h;
    valida = true;
    ultimaLecturaOk = millis();
}

bool SensorDHT::hayLectura() const { return valida; }

float SensorDHT::temperatura() const { return temp; }

float SensorDHT::humedad() const { return hum; }

unsigned long SensorDHT::segundosDesdeLectura() const {
    if (!valida) return 0;
    return (millis() - ultimaLecturaOk) / 1000;
}
