# Checklist de pruebas antes de entregar

↩︎ [Volver al README](../README.md)

Hagan esta lista **en Wokwi** y después **en la placa UTN**. Tilden cada punto. En Wokwi, el pote se mueve haciendo clic en él y la temperatura y humedad del DHT22 se cambian haciendo clic en el sensor.

## Arranque
- [ ] Compila sin errores en `env:wokwi` y en `env:placa_utn`.
- [ ] En el Serial aparece `[WiFi] Conectado. IP: ...`.
- [ ] La OLED muestra "TP02 IoT - UTN / WiFi conectado / IP ...".

## Punto 3: ThingSpeak
- [ ] Cada ~20 s aparece `[ThingSpeak] OK -> T=... H=...` en el Serial.
- [ ] En el canal de ThingSpeak se ven los gráficos de field1 (temp) y field2 (hum).

## Punto 4: comandos
- [ ] `/start` muestra el mensaje con **botones**.
- [ ] El botón **Ver Clima** responde con temperatura y humedad.
- [ ] `/clima` responde con temperatura y humedad.
- [ ] Al cambiar la temperatura en Wokwi, `/clima` muestra el nuevo valor.
- [ ] `/releon 5` prende el LED verde, y a los 5 s se apaga solo y llega el aviso "se apagó automáticamente".
- [ ] Durante esos 5 s, el bot sigue respondiendo (`/clima`). Esto **prueba que es no bloqueante**.
- [ ] `/releoff` apaga el LED verde en el momento.
- [ ] `/releon abc` responde "Tiempo inválido".
- [ ] `/releon` sin número usa 5 s y lo avisa.
- [ ] `/thingup` con el pote abajo responde con el voltaje y "Enviado a ThingSpeak", y el dato aparece en field3.
- [ ] `/thingup` dos veces seguidas: la segunda dice "se manda en ~X s" y después llega a ThingSpeak.
- [ ] `/thingup` con el pote arriba de 2,5 V además manda **🚨 ALERTA DE SEGURIDAD**.
- [ ] Mover el pote arriba de 2,5 V **sin escribir nada** manda la alerta sola y prende el LED azul.
- [ ] Bajar el pote de 2,3 V avisa "volvió a la normalidad" y apaga el LED azul.
- [ ] `/displayled` muestra en la OLED "Led encendido/apagado" de los dos LEDs.
- [ ] Con `/displayled` en pantalla, un `/releon 10` muestra la cuenta regresiva.
- [ ] `/displaypot` muestra el voltaje grande y se actualiza al mover el pote.
- [ ] `/displaydht` muestra temperatura y humedad.
- [ ] `/displayxyz` muestra en la OLED **"Comando invalido / No identificado: /displayxyz"**.
- [ ] El botón **Ver Display** abre el submenú con los 3 botones y cada uno funciona.
- [ ] Un comando que no existe (`/hola`) responde "No conozco el comando".

## Entrega
- [ ] Corre `tools\armar-entrega.ps1` sin errores.
- [ ] El `.ino` generado compila en Arduino IDE (opcional).
- [ ] Subieron `entrega/TP02_IoT_entrega.zip` a la tarea **4k01TP02/26**.
- [ ] Todos los integrantes leyeron [ARQUITECTURA.md](ARQUITECTURA.md) para la defensa.
