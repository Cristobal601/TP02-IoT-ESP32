/*
 * Potenciometro  -  Libreria propia para el potenciometro (entrada analogica)
 * ---------------------------------------------------------------------------
 * El ADC del ESP32 es de 12 bits: devuelve un valor de 0 a 4095 que
 * representa 0 V a 3,3 V.   voltaje = lectura * 3.3 / 4095
 *
 * Tambien vigila un UMBRAL CRITICO con histeresis:
 *   - Cuando el voltaje SUPERA el umbral         -> evento SUPERADO (1 sola vez)
 *   - Cuando BAJA de (umbral - histeresis)       -> evento NORMALIZADO
 * La histeresis evita que, si el valor "tiembla" justo en el umbral, se
 * disparen 50 alertas seguidas.
 *
 * Uso:
 *   Potenciometro pote(32, 2.5, 0.2);
 *   pote.iniciar();
 *   float v = pote.voltaje();
 *   if (pote.revisarUmbral() == EventoUmbral::SUPERADO) { ...alerta... }
 */
#ifndef POTENCIOMETRO_H
#define POTENCIOMETRO_H

#include <Arduino.h>

enum class EventoUmbral { NINGUNO, SUPERADO, NORMALIZADO };

class Potenciometro {
public:
    Potenciometro(uint8_t pin, float umbralV, float histeresisV);

    void iniciar();
    int leerCrudo();         // 0..4095 (promedio de varias muestras)
    float voltaje();         // 0.00..3.30 V
    int porcentaje();        // 0..100 %

    EventoUmbral revisarUmbral();  // llamar periodicamente desde loop()
    bool enAlarma() const;         // true mientras este por encima del umbral
    float umbral() const;

private:
    uint8_t pin;
    float umbralV;
    float histeresisV;
    bool alarmaActiva = false;

    static constexpr float VREF = 3.3;
    static const int MAX_ADC = 4095;
    static const int MUESTRAS = 8;   // promediar reduce el "ruido" del ADC
};

#endif
