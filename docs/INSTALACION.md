# Instalación del entorno (desde cero)

↩︎ [Volver al README](../README.md)

Es el mismo entorno que usamos en las prácticas de clase (ver también el PDF "VT Wokwi + VSCode" de la cátedra).

## 1. Programas a instalar

| Programa | Para qué | Dónde |
|---|---|---|
| **VS Code** | Editor | https://code.visualstudio.com |
| **Git** | Control de versiones (trabajar en grupo) | https://git-scm.com/downloads |
| Extensión **PlatformIO IDE** | Compilar y grabar el ESP32 | En VS Code: pestaña Extensiones → buscar "PlatformIO IDE" |
| Extensión **Wokwi Simulator** | Simular el circuito | En VS Code: pestaña Extensiones → buscar "Wokwi Simulator" |
| Driver USB **CP210x** o **CH340** | Solo para grabar la placa real | Lo usual es que Windows lo instale solo. Si no, buscá el driver del chip USB de la placa |

> Al abrir el repo, VS Code sugiere instalar las extensiones recomendadas (archivo `.vscode/extensions.json`). Aceptá.

## 2. Licencia de Wokwi (una sola vez)

1. En VS Code, `F1` y escribí **Wokwi: Request a New License**.
2. Se abre el navegador: iniciá sesión y aceptá.
3. Volvé a VS Code. Ya podés simular.

## 3. Configurar Git (una sola vez)

En una terminal (en VS Code: menú Terminal → New Terminal):

```bash
git config --global user.name "Tu Nombre"
git config --global user.email "tu-mail@ejemplo.com"
```

## 4. Bajar el proyecto

```bash
git clone <URL-DEL-REPO>
```

En VS Code: **File → Open Folder** y elegí la carpeta `TP02-IoT-ESP32`.

La primera vez, PlatformIO baja la plataforma del ESP32 y las librerías. **Puede tardar varios minutos.** Esperá a que termine la barra de progreso de abajo.

## 5. Primera compilación y simulación

1. Creá `include/secrets.h` (ver [CONFIGURACION.md](CONFIGURACION.md)).
2. Abajo de todo, verificá que diga **`env:wokwi`**.
3. Hacé clic en ✔️ (**Build**). Tiene que terminar en `SUCCESS`.
4. `F1` → **Wokwi: Start Simulator**.
5. Abrí `diagram.json` para ver el circuito simulado.
6. En la consola de Wokwi tienen que aparecer los mensajes `[WiFi] Conectado...`.

> ⚠️ **Wokwi simula el ÚLTIMO firmware compilado.** Si cambiás el código, volvé a compilar (✔️) antes de simular.

## 6. Grabar en la placa UTN

1. Conectá la placa por USB.
2. Cambiá el entorno a **`env:placa_utn`**.
3. Hacé clic en ➡️ (**Upload**). Si se queda en `Connecting...`, mantené apretado el botón **BOOT** de la placa hasta que empiece a grabar.
4. Hacé clic en 🔌 (**Monitor**) para ver el Serial a 115200 baudios.
