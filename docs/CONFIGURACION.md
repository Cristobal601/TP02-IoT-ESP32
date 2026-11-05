# Configuración: Telegram, ThingSpeak y `secrets.h`

↩︎ [Volver al README](../README.md)

Todos los datos privados van en **`include/secrets.h`**. Ese archivo **no se sube a GitHub** porque está en `.gitignore`.

```bash
# Crear secrets.h a partir de la plantilla
cp include/secrets.example.h include/secrets.h
```

> **Recomendación para el grupo:** usen **un solo bot y un solo canal de ThingSpeak**. Uno del grupo los crea y les pasa los datos a los demás **por privado** (WhatsApp, etc.), nunca por el repo.

---

## 1. Crear el bot de Telegram

1. En Telegram, buscá **@BotFather** (tiene el tilde azul).
2. Mandale `/newbot`.
3. Te pide un **nombre**, por ejemplo `TP02 ESP32 Grupo X`.
4. Te pide un **usuario**, que tiene que terminar en `bot`. Por ejemplo `tp02_grupox_bot`.
5. Te responde con un **token** parecido a `123456789:AAH-ejemplo-de-token...`.
6. Copialo en `secrets.h`:

```c
#define TELEGRAM_TOKEN "123456789:AAH-ejemplo-de-token..."
```

> ⚠️ El token es la "contraseña" del bot: con él cualquiera lo controla. Si se filtra, en @BotFather usá `/revoke` para generar uno nuevo.

### (Opcional) Chat fijo para las alertas

Las alertas automáticas van al **último chat que le habló al bot**. Si querés que vayan siempre a un chat fijo (por ejemplo, un grupo de Telegram):

1. Mandale cualquier mensaje a tu bot.
2. Abrí en el navegador `https://api.telegram.org/bot<TOKEN>/getUpdates`, reemplazando `<TOKEN>`.
3. Buscá `"chat":{"id":123456789`. Ese número es el chat ID. En los grupos es negativo, por ejemplo `-100...`.
4. Ponelo en `secrets.h`:

```c
#define TELEGRAM_CHAT_ID_ALERTAS "123456789"
```

Con esto, el ESP32 también avisa "ESP32 en línea" cada vez que arranca.

---

## 2. Crear el canal de ThingSpeak

1. Entrá a https://thingspeak.mathworks.com y creá una cuenta gratis (sirve el mail de la facu).
2. Andá a **Channels → My Channels → New Channel**.
3. Completá:
   - **Name:** `TP02 ESP32`
   - **Field 1:** `Temperatura` ✅
   - **Field 2:** `Humedad` ✅
   - **Field 3:** `Voltaje potenciometro` ✅ (marcá el checkbox para habilitarlo)
4. **Save Channel**.
5. Copiá estos dos datos:
   - **Channel ID:** es el número que aparece arriba del canal.
   - **Write API Key:** está en la pestaña **API Keys**.
6. En `secrets.h`:

```c
#define THINGSPEAK_CHANNEL_ID 1234567          // número, SIN comillas
#define THINGSPEAK_WRITE_KEY "ABCDEF1234567890" // con comillas
```

### Cómo se usan los fields

| Field | Dato | Cuándo se envía |
|---|---|---|
| field1 | Temperatura (°C) | Automático, cada 20 s |
| field2 | Humedad (%) | Automático, cada 20 s |
| field3 | Voltaje del pote (V) | Cuando se manda `/thingup` |

> ⚠️ **Límite de ThingSpeak gratis: 1 escritura cada 15 segundos.** El código lo respeta solo. Si mandás `/thingup` muy seguido, el envío queda encolado y sale apenas se puede.

---

## 3. WiFi de la placa real

En Wokwi no hace falta tocar nada, porque usa `Wokwi-GUEST` automáticamente. Para la placa UTN:

```c
#define WIFI_SSID "UTNFRC"   // o la red del lugar donde la prueben
#define WIFI_PASS ""
```

> El ESP32 **solo se conecta a redes de 2,4 GHz**: no ve las de 5 GHz. Si usan el hotspot del celular, configúrenlo en 2,4 GHz.

---

## 4. Ajustes en `include/config.h`

| Constante | Valor | Para qué |
|---|---|---|
| `UMBRAL_CRITICO_V` | `2.5` | A partir de qué voltaje se dispara la alerta |
| `HISTERESIS_V` | `0.2` | Cuánto tiene que bajar para considerarse "normalizado" |
| `RELE_SEGUNDOS_DEFECTO` | `5` | Tiempo de `/releon` si no se indica otro |
| `INTERVALO_THINGSPEAK_MS` | `20000` | Cada cuánto se suben temperatura y humedad |
| `INTERVALO_TELEGRAM_MS` | `1000` | Cada cuánto se consultan mensajes nuevos |
