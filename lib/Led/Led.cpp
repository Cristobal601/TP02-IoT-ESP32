#include "Led.h"

Led::Led(uint8_t pin, const char *nombre) : pin(pin), nombreLed(nombre) {}

void Led::iniciar() {
    pinMode(pin, OUTPUT);
    apagar();
}

void Led::encender() {
    digitalWrite(pin, HIGH);
    encendido = true;
    temporizado = false;
}

void Led::apagar() {
    digitalWrite(pin, LOW);
    encendido = false;
    temporizado = false;
}

void Led::encenderPor(unsigned long ms) {
    encender();
    temporizado = true;
    inicio = millis();
    duracion = ms;
}

bool Led::actualizar() {
    // Patron no bloqueante: se pregunta "¿ya paso el tiempo?" en cada vuelta.
    // La resta (millis() - inicio) funciona bien incluso cuando millis() da la
    // vuelta (overflow a los ~49 dias) porque es aritmetica sin signo.
    if (temporizado && millis() - inicio >= duracion) {
        apagar();
        return true;
    }
    return false;
}

bool Led::estaEncendido() const { return encendido; }

bool Led::estaTemporizado() const { return temporizado; }

unsigned long Led::segundosRestantes() const {
    if (!temporizado) return 0;
    unsigned long transcurrido = millis() - inicio;
    if (transcurrido >= duracion) return 0;
    return (duracion - transcurrido + 999) / 1000;  // redondeo hacia arriba
}

const char *Led::nombre() const { return nombreLed; }
