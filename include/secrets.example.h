/*
 * secrets.example.h  -  PLANTILLA de datos privados
 * ---------------------------------------------------------------------------
 * 1) Copiá este archivo y renombralo a  secrets.h  (en esta misma carpeta).
 * 2) Completá secrets.h con sus datos reales.
 *
 * secrets.h esta en el .gitignore: NO se sube a GitHub. Asi nadie publica
 * el token del bot ni la API Key de ThingSpeak por accidente.
 *
 * Si secrets.h no existe, el proyecto igual compila usando ESTA plantilla
 * (con avisos), pero el bot y ThingSpeak no van a funcionar.
 */
#ifndef SECRETS_H
#define SECRETS_H

// ---- WiFi de la placa real (en Wokwi se usa "Wokwi-GUEST" automaticamente) --
#define WIFI_SSID "UTNFRC"
#define WIFI_PASS ""

// ---- Telegram -----------------------------------------------------------------
// Token que te da @BotFather al crear el bot (ver docs/CONFIGURACION.md)
#define TELEGRAM_TOKEN "PEGAR_ACA_EL_TOKEN_DEL_BOT"

// Chat al que se mandan las alertas automaticas (opcional).
// Si lo dejan vacio "", las alertas van al ultimo chat que le hablo al bot.
#define TELEGRAM_CHAT_ID_ALERTAS ""

// ---- ThingSpeak -----------------------------------------------------------------
// Channel ID (numero) y Write API Key del canal (ver docs/CONFIGURACION.md)
#define THINGSPEAK_CHANNEL_ID 0
#define THINGSPEAK_WRITE_KEY "PEGAR_ACA_LA_WRITE_API_KEY"

#endif
