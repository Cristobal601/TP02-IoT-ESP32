# TP02 IoT: ESP32 + Telegram + ThingSpeak

**Tecnologías para la Automatización (TA), 4k01, UTN FRC.** Trabajo Práctico Integrador 4k01TP02/26.

Es un sistema embebido con **ESP32** que:
- mide temperatura y humedad (DHT22) y las sube a **ThingSpeak**,
- se maneja desde un **bot de Telegram** con botones,
- prende un "relé" (LED verde) por X segundos sin bloquear el programa,
- vigila un potenciómetro y **manda una alerta sola** si el voltaje pasa un umbral crítico,
- muestra en una **pantalla OLED** lo que se le pida desde Telegram.

> 📅 **Entrega:** grupos pares el **miércoles 11/11**, grupos impares el **miércoles 18/11**.
> Se sube el proyecto Wokwi + el `.ino` a la tarea **4k01TP02/26** de la UV.

---

## Índice

1. [Inicio rápido (para el que no sabe nada)](#1-inicio-rápido)
2. [Qué hace cada comando](#2-comandos-del-bot)
3. [Hardware y pines](#3-hardware-y-pines)
4. [Cómo está organizado el código](#4-estructura-del-repo)
5. [Simular en Wokwi vs. grabar en la placa UTN](#5-wokwi-vs-placa-utn)
6. [Cómo generar la entrega](#6-generar-la-entrega)
7. [Problemas comunes](#7-problemas-comunes)
8. [Documentación detallada](#8-documentación-detallada)

---

## 1. Inicio rápido

> Si es la primera vez, arrancá por **[docs/INSTALACION.md](docs/INSTALACION.md)**. Ahí se explica cómo instalar VS Code, PlatformIO, Wokwi y Git paso a paso.

```bash
# 1. Bajar el repo
git clone <URL-DEL-REPO>
cd TP02-IoT-ESP32

# 2. Crear el archivo de secretos a partir de la plantilla
#    (Windows: copiá include/secrets.example.h y renombrá la copia a include/secrets.h)
cp include/secrets.example.h include/secrets.h
```

3. Completá `include/secrets.h` con el **token del bot** y los datos de **ThingSpeak**. El paso a paso está en [docs/CONFIGURACION.md](docs/CONFIGURACION.md).
4. Abrí la carpeta en **VS Code**. PlatformIO baja las librerías solo.
5. Compilá con el ✔️ de la barra de abajo (o `Ctrl+Alt+B`).
6. Para simular, presioná `F1`, elegí **Wokwi: Start Simulator** y abrí `diagram.json`.
7. En Telegram, buscá tu bot y mandale **/start**.

---

## 2. Comandos del bot

| Comando | Qué hace | Ejemplo de respuesta |
|---|---|---|
| `/start` | Bienvenida y **teclado inline** (botones debajo del mensaje) | Botones: Ver Clima · Encender Relé · Apagar Relé · Subir potenciómetro · Ver Display |
| `/clima` | Últimos valores del DHT22 | `Temperatura: 24.3 °C / Humedad: 40.0 %` |
| `/releon <seg>` | Prende el relé (LED verde) `<seg>` segundos y **se apaga solo** | `/releon 10` → `Relé ENCENDIDO por 10 s` |
| `/releoff` | Apaga el relé en el momento | `Relé APAGADO` |
| `/thingup` | Lee el pote, lo pasa a **voltaje** y lo sube a ThingSpeak (field3). Si supera el umbral → **alerta** | `Potenciómetro: 2.71 V` + `🚨 ALERTA DE SEGURIDAD` |
| `/displayled` | La OLED muestra `Led encendido/apagado` de cada LED | (en la OLED) |
| `/displaypot` | La OLED muestra el voltaje del potenciómetro | (en la OLED) |
| `/displaydht` | La OLED muestra temperatura y humedad | (en la OLED) |
| `/display<otra cosa>` | La OLED muestra **"Comando invalido"** | `/displayxyz` → error en pantalla |

**Además, el ESP32 manda mensajes por su cuenta, sin que nadie se los pida:**
- 🚨 Si el potenciómetro **supera el umbral crítico** (2,5 V por defecto), manda una alerta y prende el **LED azul**.
- ✅ Cuando el voltaje vuelve a la normalidad, avisa y apaga el LED azul.
- ⏱️ Cuando se cumple el tiempo de `/releon`, avisa que el relé se apagó.

**Otros detalles:**
- También acepta `/rele on 10` y `/display led`, con espacio.
- No distingue mayúsculas de minúsculas.
- En grupos de Telegram funciona con `/clima@NombreBot`.

---

## 3. Hardware y pines

| Componente | Pin ESP32 | Notas |
|---|---|---|
| DHT22 (dato) | **GPIO 33** | VCC a 3V3, GND a GND |
| Potenciómetro (cursor) | **GPIO 32** | Es ADC1, así que anda con el WiFi prendido |
| LED verde = "relé" | **GPIO 23** | Con resistencia de 220 Ω |
| LED azul = alerta | **GPIO 2** | En la placa suele ser el LED integrado |
| OLED SDA | **GPIO 21** | I²C, dirección `0x3C` |
| OLED SCL | **GPIO 22** | |

> **¿Por qué un LED es el "relé"?** El enunciado habla de un relé o actuador conectado a una salida digital, pero en la lista de hardware no hay ningún relé. Desde el código, un relé se maneja igual que un LED (`digitalWrite`). Por eso el **LED verde** representa al actuador principal. Si la placa tiene un relé real, alcanza con cambiar `PIN_LED_VERDE` en `include/config.h`.

Los pines, tiempos y el umbral están todos en **`include/config.h`**.

---

## 4. Estructura del repo

```
TP02-IoT-ESP32/
├── platformio.ini            ← placa, librerías y los 2 entornos (wokwi / placa_utn)
├── wokwi.toml                ← le dice a Wokwi qué firmware simular
├── diagram.json              ← circuito de Wokwi
├── include/
│   ├── config.h              ← PINES, TIEMPOS, UMBRAL (lo que se puede tocar)
│   ├── secrets.example.h     ← plantilla de datos privados (sí va al repo)
│   └── secrets.h             ← tus datos privados (NO va al repo, está en .gitignore)
├── lib/                      ← LIBRERÍAS PROPIAS (una por dispositivo + la del bot)
│   ├── Led/                  ← salida digital con apagado temporizado no bloqueante
│   ├── SensorDHT/            ← DHT22 con caché de la última lectura válida
│   ├── Potenciometro/        ← ADC → voltaje + detección de umbral con histéresis
│   ├── PantallaOLED/         ← OLED SH1106/SSD1306, sin tildes, 21 caracteres
│   ├── ConexionWiFi/         ← conexión y reconexión automática
│   ├── ClienteThingSpeak/    ← envíos HTTP respetando el límite de 15 s
│   └── BotTelegram/          ← TODOS los comandos de Telegram + modo de la OLED
├── src/
│   └── main.cpp              ← solo orquesta: crea los objetos y les da turno en loop()
├── tools/
│   └── armar-entrega.ps1     ← genera el .ino y el .zip para subir a la UV
└── docs/                     ← documentación para el grupo
```

**Idea principal (lo que pidió el profe):**
- **Una librería por dispositivo** (`Led`, `SensorDHT`, `Potenciometro`, `PantallaOLED`).
- **Una librería que maneja los comandos de Telegram** (`BotTelegram`).
- El `main.cpp` queda cortito: crea los objetos y llama a `actualizar()` / `atender()`.

La explicación completa de cada librería y del `loop()` está en **[docs/ARQUITECTURA.md](docs/ARQUITECTURA.md)**.

---

## 5. Wokwi vs. placa UTN

En `platformio.ini` hay **dos entornos**. Se elige en la barra de abajo de VS Code (donde dice `env:...`).

| | `env:wokwi` (por defecto) | `env:placa_utn` |
|---|---|---|
| Para qué | Simular en VS Code | Grabar en la placa real |
| WiFi | `Wokwi-GUEST` (automático) | La de `secrets.h` (`WIFI_SSID` / `WIFI_PASS`) |
| Pantalla | SSD1306 (la que tiene Wokwi) | SH1106 (la de la placa, como en las prácticas) |

**Grabar en la placa:**
1. Conectá la placa por USB.
2. Elegí `env:placa_utn`.
3. Hacé clic en ➡️ (**Upload**).
4. Hacé clic en 🔌 (**Serial Monitor**, 115200 baudios) para ver los mensajes.

> Si la pantalla de la placa muestra cualquier cosa, probablemente sea una SSD1306. En `platformio.ini`, en `[env:placa_utn]`, cambiá `-D OLED_SH1106` por `-D OLED_SSD1306`.

---

## 6. Generar la entrega

```powershell
powershell -ExecutionPolicy Bypass -File tools\armar-entrega.ps1
```

El script crea:
- `entrega/TP02_IoT/TP02_IoT.ino`: el sketch con todas las librerías en la misma carpeta. Se abre con Arduino IDE o se copia a wokwi.com.
- `entrega/TP02_IoT_entrega.zip`: **el archivo que se sube a la UV**. Tiene el sketch y el proyecto PlatformIO con `diagram.json`.

El sketch generado queda configurado para la **placa UTN**.

---

## 7. Problemas comunes

| Síntoma | Causa probable | Solución |
|---|---|---|
| Al compilar: `No existe include/secrets.h` | No creaste `secrets.h` | Copiá `secrets.example.h` como `secrets.h` y completalo |
| El bot no responde | Token mal copiado, o no hay internet | Revisá `TELEGRAM_TOKEN`. En el Serial Monitor tiene que aparecer `[WiFi] Conectado` |
| El bot responde al rato | Wokwi es lento con HTTPS | Es normal en el simulador (1 a 3 s). En la placa real anda más rápido |
| ThingSpeak no muestra nada | Channel ID o Write Key mal, o falta crear los fields | Ver [docs/CONFIGURACION.md](docs/CONFIGURACION.md). En Serial debe aparecer `[ThingSpeak] OK` |
| `/thingup` dice "se manda en ~X s" | ThingSpeak gratis acepta **1 envío cada 15 s** | Es correcto: queda encolado y se envía solo |
| La OLED muestra símbolos raros | Driver equivocado | Cambiá `OLED_SH1106` ↔ `OLED_SSD1306` en `platformio.ini` |
| La OLED no muestra nada | Cables o dirección I²C | SDA=21, SCL=22, dirección `0x3C` |
| DHT22 "sin lectura válida" | Pin o alimentación | DHT22 en GPIO 33 y VCC a 3V3 |
| El pote siempre marca 0 o 3,3 V | Cable del cursor (SIG) mal | SIG va a GPIO 32 y los extremos van a 3V3 y GND |

---

## 8. Documentación detallada

| Archivo | Qué tiene |
|---|---|
| [docs/INSTALACION.md](docs/INSTALACION.md) | Instalar VS Code, PlatformIO, Wokwi y Git desde cero |
| [docs/CONFIGURACION.md](docs/CONFIGURACION.md) | Crear el bot con BotFather, sacar el chat ID y crear el canal de ThingSpeak |
| [docs/ARQUITECTURA.md](docs/ARQUITECTURA.md) | Cómo funciona cada librería, el loop no bloqueante y **preguntas para la defensa** |
| [docs/GIT.md](docs/GIT.md) | Cómo trabajar en grupo con Git y GitHub sin pisarse |
| [docs/PRUEBAS.md](docs/PRUEBAS.md) | Checklist para probar todo antes de entregar |
