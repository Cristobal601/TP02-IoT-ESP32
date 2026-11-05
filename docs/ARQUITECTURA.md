# Arquitectura del código (y cómo defenderlo)

↩︎ [Volver al README](../README.md)

## 1. La idea en una frase

> **Cada dispositivo es una clase en `lib/`, la lógica de Telegram es otra clase, y el `main.cpp` solo les da "turno" a todos en un `loop()` que nunca se bloquea.**

Es lo que pidió el profe: **una librería por dispositivo** y **una librería para los comandos de Telegram**. Retoma lo de la Práctica 03 (la clase `SensorDHT` en `lib/`) y lo lleva a todo el sistema.

```
                     ┌──────────────────────────┐
                     │         main.cpp         │
                     │  setup(): inicia todo    │
                     │  loop():  da turnos      │
                     └────────────┬─────────────┘
          ┌───────────────┬───────┴───────┬────────────────┐
          ▼               ▼               ▼                ▼
   ┌────────────┐  ┌─────────────┐  ┌───────────┐  ┌───────────────┐
   │ConexionWiFi│  │ BotTelegram │  │ClienteThing│ │ dispositivos  │
   │            │  │ (comandos)  │──▶│  Speak    │  │ Led x2        │
   └────────────┘  └──────┬──────┘  └───────────┘  │ SensorDHT     │
                          │ usa por referencia      │ Potenciometro │
                          └────────────────────────▶│ PantallaOLED  │
                                                    └───────────────┘
```

---

## 2. Las librerías, una por una

Cada librería tiene un `.h` (qué se puede hacer, la "interfaz") y un `.cpp` (cómo se hace). Todas siguen el mismo patrón:
- **Constructor**: guarda la configuración (pin, umbral...). **No toca el hardware**.
- **`iniciar()`**: se llama en `setup()` y configura el hardware (`pinMode`, `begin()`...).
- **`actualizar()`**: se llama en cada vuelta de `loop()`. Hace su trabajo **solo si ya le toca**.

> ¿Por qué el hardware no se toca en el constructor? Los objetos globales se construyen **antes** de `setup()`, cuando el ESP32 todavía no está listo. En la práctica 03 el `sensor.begin()` estaba en el constructor y a veces funciona, pero no es seguro.

### `Led` (`lib/Led`)
Maneja una salida digital. Se usa **dos veces**: para el LED verde ("relé") y para el LED azul (alerta).

| Método | Qué hace |
|---|---|
| `encender()` / `apagar()` | `digitalWrite` HIGH/LOW y guarda el estado |
| `encenderPor(ms)` | Prende y anota la hora (`millis()`) y la duración |
| `actualizar()` | Si pasó la duración, apaga. Devuelve `true` en ese momento, para poder avisar |
| `estaEncendido()` | Para mostrar "Led encendido/apagado" |

**Punto clave para la defensa: `/releon 10` NO usa `delay(10000)`.** Si lo usara, el ESP32 quedaría 10 segundos "congelado": no respondería al bot, no leería el sensor y no vigilaría el umbral. En cambio:

```cpp
// Al recibir /releon 10:
inicio = millis();  duracion = 10000;  digitalWrite(pin, HIGH);

// En cada vuelta del loop (miles de veces por segundo):
if (millis() - inicio >= duracion) digitalWrite(pin, LOW);
```

### `SensorDHT` (`lib/SensorDHT`)
- El DHT22 **no se puede leer más de una vez cada ~2 s**. La clase lo lee cada 2,5 s y **guarda la última lectura válida**.
- Si una lectura falla (`NaN`), conserva la anterior.
- `hayLectura()` devuelve `false` hasta la primera lectura buena, para no mostrar `nan`.
- Usa el tipo `DHT22`. Ojo: en la Práctica 03 había quedado `DHT21`.

### `Potenciometro` (`lib/Potenciometro`)
- El ADC es de **12 bits**, así que devuelve de **0 a 4095**, que representa **0 a 3,3 V**.
- `voltaje = lectura × 3.3 / 4095`
- Promedia 8 lecturas porque el ADC del ESP32 es "ruidoso".
- **GPIO 32 es del ADC1.** Los pines del **ADC2 no andan con el WiFi prendido**, y es una pregunta típica de parcial.
- **Umbral con histéresis:**
  - Si sube de 2,5 V → evento `SUPERADO` (una sola vez).
  - Si baja de 2,3 V → evento `NORMALIZADO`.
  - Sin histéresis, si el valor "tiembla" en 2,50 V se mandarían decenas de alertas.

### `PantallaOLED` (`lib/PantallaOLED`)
Resuelve el criterio **"Resolución problema mensajes para el display"**:

| Problema | Solución |
|---|---|
| Wokwi solo tiene OLED **SSD1306**, la placa usa **SH1106** | Se elige con un `#define` desde `platformio.ini`. El resto del código no cambia |
| La fuente no tiene tildes, ñ ni ° (se ven como basura) | `adaptar()` reemplaza `á→a`, `ñ→n`, saca `°` y emojis |
| Solo entran **21 caracteres** por línea (128 px / 6 px) | `adaptar()` recorta |
| Quedan restos del mensaje anterior | Siempre se hace `clearDisplay()` antes de dibujar |
| Mensajes desordenados | Layout fijo: barra de título + 4 líneas, o título + valor grande |

### `ConexionWiFi` (`lib/ConexionWiFi`)
- `conectar()` en `setup()`: espera con **timeout** (20 s), así no se cuelga para siempre si no hay red.
- `mantener()` en `loop()`: si se cae la red, reintenta cada 10 s **sin bloquear**.

### `ClienteThingSpeak` (`lib/ClienteThingSpeak`)
- Usa la librería oficial `ThingSpeak.h`, que manda un **HTTP GET/POST** con la **Write API Key**.
- **Límite de 15 s entre escrituras** en la cuenta gratis: la clase lo controla. Si `/thingup` llega muy pronto, el valor queda **encolado** y se manda apenas se puede.
- Manda todos los fields juntos en **una sola escritura** (temp + hum + voltaje), para no gastar el límite.
- Devuelve el código HTTP: `200` = OK.

### `BotTelegram` (`lib/BotTelegram`): la librería de comandos
Es la más importante. Hace esto:
1. **`atender()`** consulta cada 1 s si hay mensajes nuevos con `bot.getUpdates(...)`. A esto se le llama *polling*.
2. **`procesarMensaje()`** es un *despachador*:
   - Normaliza el texto: minúsculas, sin espacios extra y sin `@NombreBot`.
   - Separa **comando** y **argumento**: `"/releon 10"` → `"/releon"` + `"10"`.
   - Llama a la función que corresponde: `cmdClima`, `cmdRele`, `cmdThingUp`, `cmdDisplay`...
3. **Teclado inline**: `/start` responde con `sendMessageWithInlineKeyboard(...)`. Cada botón tiene un `callback_data`, que es el mismo comando (por ejemplo `/clima`). Al tocar el botón, al ESP32 le llega un mensaje de tipo `callback_query` con ese texto y **se procesa igual que si lo hubieran escrito**. Hay que responder con `answerCallbackQuery` para que el botón deje de "cargar".
4. **Avisos espontáneos**: `avisar()` y `alertaVoltaje()` mandan mensajes **sin que nadie pregunte**. Van a `TELEGRAM_CHAT_ID_ALERTAS`, o si no está configurado, al último chat que habló. Tienen un anti-spam de 10 s.
5. **Modo de la OLED**: guarda qué se pidió con `/display...` (`LED`, `POTENCIOMETRO`, `DHT` o `ERROR`) y `refrescarPantalla()` lo redibuja cada 1 s. Así, si pedís `/displaypot` y movés el pote, el valor se actualiza en vivo.

**¿Por qué HTTPS?** La API de Telegram (`api.telegram.org`) solo acepta conexiones seguras. Por eso se usa `WiFiClientSecure` con el certificado raíz `TELEGRAM_CERTIFICATE_ROOT`. ThingSpeak, en cambio, acepta HTTP común (`WiFiClient`).

---

## 3. El `loop()`: multitarea cooperativa

```cpp
void loop() {
    wifi.mantener();          // ¿se cayó el WiFi? reintento cada 10 s
    dht.actualizar();         // ¿pasaron 2,5 s? leo el sensor
    if (rele.actualizar())    // ¿se cumplió el tiempo del relé? apago y aviso
        bot.avisar(...);
    vigilarPotenciometro();   // ¿pasaron 0,5 s? reviso el umbral
    nube.actualizar(...);     // ¿pasaron 20 s? subo temp/hum a ThingSpeak
    bot.atender();            // ¿pasó 1 s? leo mensajes de Telegram
    // cada 1 s: refresco la OLED
}
```

Cada tarea pregunta **"¿ya me toca?"** con `millis()`. Si no le toca, devuelve el control enseguida. El `loop()` da miles de vueltas por segundo y **todas las tareas avanzan "a la vez"**. Esto se llama **multitarea cooperativa** o **programación no bloqueante**.

> La única espera "bloqueante" es la de `getUpdates()` y `sendMessage()`, porque hacen una petición HTTPS y tardan unos cientos de ms. En Wokwi pueden tardar 1 a 3 s. Por eso el apagado del relé puede tener un pequeño desfase de hasta ~1 s. Es aceptable.

---

## 4. Flujo de cada comando

```
Usuario toca "Encender Relé (5 s)"
  → Telegram manda callback_query con data="/releon 5"
  → bot.atender() lo recibe en el próximo polling
  → procesarMensaje(): answerCallbackQuery + separa "/releon" y "5"
  → cmdRele("on","5") → rele.encenderPor(5000) → responde "Relé ENCENDIDO por 5 s"
  ... el loop sigue girando normalmente ...
  → 5 s después rele.actualizar() devuelve true → apaga → bot.avisar("se apagó")
```

```
Usuario mueve el pote por encima de 2,5 V (sin escribir nada)
  → vigilarPotenciometro() → pote.revisarUmbral() == SUPERADO
  → ledAzul.encender() + bot.alertaVoltaje(v) → "🚨 ALERTA DE SEGURIDAD"
  → baja de 2,3 V → NORMALIZADO → ledAzul.apagar() + aviso "volvió a la normalidad"
```

```
/displayxyz
  → cmdDisplay("xyz") → no coincide con led/pot/dht
  → modo = ERROR → la OLED muestra "Comando invalido / No identificado: /displayxyz"
```

---

## 5. Posibles preguntas en la defensa

**¿Por qué usan `millis()` y no `delay()`?**
Porque `delay()` frena todo el programa: mientras espera, el bot no responde y no se vigila el umbral. Con `millis()` el `loop()` sigue girando y cada tarea se ejecuta cuando le toca.

**¿Qué pasa con `millis()` cuando se desborda (a los ~49 días)?**
La resta `millis() - inicio` con `unsigned long` sigue dando bien por la aritmética modular. Por eso siempre se compara la *diferencia* y no `millis() > inicio + duracion`.

**¿Cómo se convierte la lectura del pote a voltaje?**
ADC de 12 bits: 0–4095 corresponde a 0–3,3 V, así que `V = lectura × 3.3 / 4095`.

**¿Por qué el pote está en GPIO 32 y no en cualquier pin analógico?**
Porque el GPIO 32 es del ADC1. El ADC2 lo usa el WiFi, así que no se puede leer con el WiFi prendido.

**¿Qué es la histéresis y para qué está?**
Son dos umbrales distintos, uno para "entrar" en alarma (2,5 V) y otro para "salir" (2,3 V). Evitan que el ruido cerca del umbral dispare muchas alertas seguidas.

**¿Cómo funciona el teclado inline?**
Se manda un JSON con filas de botones. Cada botón tiene `text` (lo que se ve) y `callback_data` (lo que recibe el ESP32). Al tocarlo llega un `callback_query` que el despachador procesa como un comando más.

**¿Por qué Telegram usa `WiFiClientSecure` y ThingSpeak `WiFiClient`?**
Telegram exige HTTPS (TLS), así que hay que validar el certificado. ThingSpeak acepta HTTP plano.

**¿Qué límite tiene ThingSpeak y cómo lo manejan?**
1 escritura cada 15 s en la cuenta gratis. La clase guarda la hora del último envío, y si se pide antes, encola el dato y lo manda apenas pasa el tiempo.

**¿Cómo se envían los datos a ThingSpeak?**
Por HTTP, con la Write API Key del canal: `setField(n, valor)` y después `writeFields(canal, key)`. Responde `200` si salió bien.

**¿Por qué en Wokwi la pantalla es SSD1306 y en la placa SH1106?**
Wokwi no tiene SH1106. Son controladores distintos con comandos distintos. La librería `PantallaOLED` elige el driver con un `#define` y el resto del código no se entera.

**¿Por qué un `secrets.h` aparte?**
Para no subir el token del bot ni la API Key a GitHub. Está en `.gitignore` y cada uno tiene su copia local.

**¿Qué es I²C?**
Un bus serie de 2 cables: **SDA** (datos, GPIO 21) y **SCL** (reloj, GPIO 22). Cada dispositivo tiene una dirección; la OLED usa `0x3C`. Es maestro/esclavo y el ESP32 es el maestro.

**¿Qué es el *polling* de Telegram?**
Es preguntarle periódicamente al servidor de Telegram "¿hay mensajes nuevos?" con `getUpdates(offset)`. El `offset = last_message_received + 1` evita procesar dos veces el mismo mensaje.

---

## 6. Sugerencia de reparto para la defensa (6 integrantes)

| Integrante | Tema que explica |
|---|---|
| 1 | Hardware, pines, `diagram.json`, Wokwi vs. placa (`platformio.ini`) |
| 2 | `Led` + `/releon` / `/releoff` (millis, no bloqueante) |
| 3 | `SensorDHT` + `/clima` + `ClienteThingSpeak` (límite 15 s) |
| 4 | `Potenciometro` + `/thingup` + alerta automática (ADC, histéresis) |
| 5 | `PantallaOLED` + `/display...` (I²C, problemas de mensajes) |
| 6 | `BotTelegram`: despachador, teclado inline, polling, HTTPS |

Igual **todos tienen que entender el `loop()`** (sección 3), porque es la base de todo.
