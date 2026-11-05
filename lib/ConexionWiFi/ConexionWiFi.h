/*
 * ConexionWiFi  -  Libreria propia para el modulo WiFi del ESP32
 * ---------------------------------------------------------------------------
 * - conectar(): se usa UNA vez en setup(). Espera hasta conectarse o hasta
 *   que se cumpla el timeout (asi no se cuelga para siempre).
 * - mantener(): se usa en loop(). Si se cayo la red, reintenta cada 10 s
 *   sin bloquear el resto del programa.
 */
#ifndef CONEXION_WIFI_H
#define CONEXION_WIFI_H

#include <Arduino.h>
#include <WiFi.h>

class ConexionWiFi {
public:
    ConexionWiFi(const char *ssid, const char *pass);

    bool conectar(unsigned long timeoutMs = 20000);
    void mantener();
    bool conectado() const;
    String ip() const;
    const char *red() const;

private:
    const char *ssid;
    const char *pass;
    unsigned long ultimoReintento = 0;
    static const unsigned long REINTENTO_MS = 10000;
};

#endif
