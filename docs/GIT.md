# Git y GitHub para el grupo (guía rápida)

↩︎ [Volver al README](../README.md)

## Conceptos en 30 segundos

| Palabra | Qué es |
|---|---|
| **Repositorio (repo)** | La carpeta del proyecto con todo su historial de cambios |
| **Commit** | Una "foto" guardada de los cambios, con un mensaje que dice qué se hizo |
| **Push** | Subir tus commits a GitHub |
| **Pull** | Bajar los commits que subieron los demás |
| **Branch (rama)** | Una copia paralela para trabajar sin romper lo que anda |
| **Pull Request (PR)** | Pedido para unir tu rama a `main`, para que alguien la revise antes |

## Primera vez

```bash
git clone <URL-DEL-REPO>
cd TP02-IoT-ESP32
cp include/secrets.example.h include/secrets.h   # y completarlo con los datos que pasen por privado
```

## El ciclo de todos los días

```bash
# 1. ANTES de tocar nada: traer lo último
git pull

# 2. Trabajar... (editar, compilar, probar en Wokwi)

# 3. Ver qué cambiaste
git status

# 4. Guardar los cambios en un commit
git add .
git commit -m "Agrego comando /displaydht en la OLED"

# 5. Subirlo
git push
```

> **Regla de oro:** siempre hacé `git pull` antes de empezar y `git push` cuando terminás. Así nadie trabaja sobre una versión vieja.

## Trabajar con ramas (recomendado)

Si cada uno toca una parte distinta, conviene usar una rama por persona o por tarea:

```bash
git checkout -b feature/display-oled      # crea la rama y se cambia a ella
# ... trabajar, hacer commits ...
git push -u origin feature/display-oled   # sube la rama a GitHub
```

Después, en GitHub, se hace clic en **"Compare & pull request"**. Otro del grupo lo revisa y lo une a `main` con **Merge**.

Para volver a `main` y actualizarla:

```bash
git checkout main
git pull
```

## Si aparece un conflicto

Pasa cuando dos personas cambiaron **las mismas líneas** del mismo archivo. Git marca el archivo así:

```
<<<<<<< HEAD
lo que tenías vos
=======
lo que subió el otro
>>>>>>> origin/main
```

1. Abrí el archivo. VS Code muestra botones: *Accept Current*, *Accept Incoming* y *Accept Both*.
2. Dejá la versión correcta y borrá las marcas `<<<<<<<`, `=======` y `>>>>>>>`.
3. Compilá para asegurarte de que anda.
4. Terminá con:

```bash
git add .
git commit -m "Resuelvo conflicto en BotTelegram.cpp"
git push
```

**Para evitar conflictos:** repártanse los archivos. Cada librería está en su propia carpeta de `lib/`, así que es fácil que cada uno toque solo la suya.

## Lo que NUNCA hay que subir

- `include/secrets.h`: tiene el token y las keys. Ya está en `.gitignore`.
- `.pio/`: archivos compilados, pesan mucho y se regeneran solos.
- `entrega/`: lo genera el script.

Si por error se subió un token, **no alcanza con borrarlo**: queda en el historial. Hay que **revocarlo** en @BotFather (`/revoke`) y generar uno nuevo.

## Desde VS Code sin comandos

La pestaña **Source Control** (ícono de ramitas, `Ctrl+Shift+G`) permite hacer todo con clics:
- Escribís el mensaje y hacés clic en **Commit**.
- Con **Sync Changes** hacés pull + push.
- Abajo a la izquierda ves y cambiás la rama actual.

## Mensajes de commit claros

Bien:
- `Agrego histéresis al umbral del potenciómetro`
- `Corrijo tildes en la OLED`
- `Documento /thingup en el README`

Mal:
- `cambios`
- `asdf`
- `ahora sí anda`
