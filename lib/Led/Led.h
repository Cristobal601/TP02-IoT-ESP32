/*
 * Led  -  Libreria propia para una salida digital (LED o rele)
 * ---------------------------------------------------------------------------
 * Ademas de prender/apagar, permite "encender por X tiempo" y que se apague
 * SOLA sin bloquear el programa (no usa delay(), usa millis()).
 *
 * Uso:
 *   Led rele(23, "Rele");
 *   rele.iniciar();              // en setup()
 *   rele.encenderPor(5000);      // prende 5 s
 *   rele.actualizar();           // en loop(): si ya pasaron los 5 s, apaga
 */
#ifndef LED_H
#define LED_H

#include <Arduino.h>

class Led {
public:
    Led(uint8_t pin, const char *nombre);

    void iniciar();                       // pinMode(OUTPUT) y arranca apagado
    void encender();                      // prende (y cancela cualquier temporizador)
    void apagar();                        // apaga (y cancela cualquier temporizador)
    void encenderPor(unsigned long ms);   // prende y programa el apagado automatico

    // Llamar en cada vuelta del loop(). Devuelve true SOLO en la vuelta en que
    // el temporizador se cumplio y el LED se apago solo (sirve para avisar).
    bool actualizar();

    bool estaEncendido() const;
    bool estaTemporizado() const;          // true si esta prendido "por X segundos"
    unsigned long segundosRestantes() const;
    const char *nombre() const;

private:
    uint8_t pin;
    const char *nombreLed;
    bool encendido = false;
    bool temporizado = false;
    unsigned long inicio = 0;    // millis() en que se encendio
    unsigned long duracion = 0;  // cuanto tiempo tiene que quedar prendido
};

#endif
