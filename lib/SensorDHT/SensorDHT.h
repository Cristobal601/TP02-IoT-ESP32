/*
 * SensorDHT  -  Libreria propia para el sensor DHT22 (temperatura y humedad)
 * ---------------------------------------------------------------------------
 * El DHT22 es LENTO: no se le puede pedir una lectura mas de 1 vez cada ~2 s.
 * Por eso esta clase lee cada 2,5 s y GUARDA la ultima lectura valida.
 * Cuando alguien pide la temperatura (bot, pantalla, ThingSpeak) se devuelve
 * el valor guardado, sin volver a molestar al sensor.
 *
 * Uso:
 *   SensorDHT dht(33);
 *   dht.iniciar();          // en setup()
 *   dht.actualizar();       // en loop()
 *   if (dht.hayLectura()) Serial.println(dht.temperatura());
 */
#ifndef SENSOR_DHT_H
#define SENSOR_DHT_H

#include <Arduino.h>
#include <DHT.h>

class SensorDHT {
public:
    explicit SensorDHT(uint8_t pin);

    void iniciar();
    void actualizar();                      // lee el sensor si ya paso el intervalo

    bool hayLectura() const;                // false hasta la primera lectura buena
    float temperatura() const;              // °C (ultima lectura valida)
    float humedad() const;                  // %  (ultima lectura valida)
    unsigned long segundosDesdeLectura() const;

private:
    DHT sensor;
    float temp = NAN;
    float hum = NAN;
    bool valida = false;
    unsigned long ultimoIntento = 0;
    unsigned long ultimaLecturaOk = 0;
    bool primerIntento = true;

    static const unsigned long INTERVALO_MS = 2500;
};

#endif
