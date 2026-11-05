#include "ConexionWiFi.h"

ConexionWiFi::ConexionWiFi(const char *ssid, const char *pass) : ssid(ssid), pass(pass) {}

bool ConexionWiFi::conectar(unsigned long timeoutMs) {
    Serial.printf("[WiFi] Conectando a \"%s\"", ssid);
    WiFi.mode(WIFI_STA);   // modo estacion (cliente de un router)
    WiFi.begin(ssid, pass);

    unsigned long inicio = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - inicio < timeoutMs) {
        delay(500);   // aceptable: solo pasa en setup(), antes de arrancar el loop
        Serial.print(".");
    }
    Serial.println();

    if (!conectado()) {
        Serial.println("[WiFi] No se pudo conectar (se va a reintentar desde el loop)");
        return false;
    }
    Serial.printf("[WiFi] Conectado. IP: %s\n", ip().c_str());
    return true;
}

void ConexionWiFi::mantener() {
    if (conectado()) return;
    if (millis() - ultimoReintento < REINTENTO_MS) return;
    ultimoReintento = millis();
    Serial.println("[WiFi] Sin conexion, reintentando...");
    WiFi.reconnect();
}

bool ConexionWiFi::conectado() const { return WiFi.status() == WL_CONNECTED; }

String ConexionWiFi::ip() const { return WiFi.localIP().toString(); }

const char *ConexionWiFi::red() const { return ssid; }
