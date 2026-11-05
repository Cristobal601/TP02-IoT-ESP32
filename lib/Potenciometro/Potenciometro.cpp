#include "Potenciometro.h"

Potenciometro::Potenciometro(uint8_t pin, float umbralV, float histeresisV)
    : pin(pin), umbralV(umbralV), histeresisV(histeresisV) {}

void Potenciometro::iniciar() {
    pinMode(pin, INPUT);
    analogReadResolution(12);                 // 0..4095
    analogSetPinAttenuation(pin, ADC_11db);   // rango completo ~0..3,3 V
    // GPIO32 pertenece al ADC1: se puede usar con el WiFi encendido.
    // (Los pines del ADC2 NO funcionan mientras el WiFi esta activo.)
}

int Potenciometro::leerCrudo() {
    long suma = 0;
    for (int i = 0; i < MUESTRAS; i++) {
        suma += analogRead(pin);
    }
    return suma / MUESTRAS;
}

float Potenciometro::voltaje() {
    return leerCrudo() * VREF / MAX_ADC;
}

int Potenciometro::porcentaje() {
    return map(leerCrudo(), 0, MAX_ADC, 0, 100);
}

EventoUmbral Potenciometro::revisarUmbral() {
    float v = voltaje();

    if (!alarmaActiva && v > umbralV) {
        alarmaActiva = true;
        return EventoUmbral::SUPERADO;
    }
    if (alarmaActiva && v < umbralV - histeresisV) {
        alarmaActiva = false;
        return EventoUmbral::NORMALIZADO;
    }
    return EventoUmbral::NINGUNO;
}

bool Potenciometro::enAlarma() const { return alarmaActiva; }

float Potenciometro::umbral() const { return umbralV; }
