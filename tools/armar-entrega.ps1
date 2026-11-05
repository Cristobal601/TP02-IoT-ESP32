# =============================================================================
#  armar-entrega.ps1  -  Genera lo que se sube a la UV (tarea 4k01TP02/26)
# =============================================================================
#  Uso (desde la carpeta del repo, en una terminal de VS Code):
#      powershell -ExecutionPolicy Bypass -File tools\armar-entrega.ps1
#
#  Crea la carpeta  entrega\  con:
#    entrega\TP02_IoT\             -> sketch Arduino "plano": TP02_IoT.ino + todas
#                                     las librerias (.h/.cpp) + diagram.json +
#                                     libraries.txt. Se abre con Arduino IDE o se
#                                     copia a un proyecto de wokwi.com.
#    entrega\TP02_IoT_entrega.zip  -> el sketch + el proyecto PlatformIO completo
#                                     (sin la carpeta .pio). ESTE es el que se sube.
#
#  Por defecto el sketch queda configurado para la PLACA UTN (pantalla SH1106 y
#  la red WiFi de secrets.h).
# =============================================================================
param(
    [string]$Nombre = "TP02_IoT"
)

$ErrorActionPreference = "Stop"
$raiz    = Split-Path -Parent $PSScriptRoot
$salida  = Join-Path $raiz "entrega"
$sketch  = Join-Path $salida $Nombre
$proy    = Join-Path $salida "proyecto-platformio"

Write-Host "Armando entrega en $salida" -ForegroundColor Cyan

if (Test-Path $salida) { Remove-Item $salida -Recurse -Force }
New-Item -ItemType Directory -Path $sketch | Out-Null

# ---- 1) Sketch plano (.ino + librerias en la misma carpeta) -------------------
Copy-Item (Join-Path $raiz "src\main.cpp") (Join-Path $sketch "$Nombre.ino")
Get-ChildItem (Join-Path $raiz "lib") -Recurse -Include *.h, *.cpp |
    Copy-Item -Destination $sketch
Copy-Item (Join-Path $raiz "include\config.h")          $sketch
Copy-Item (Join-Path $raiz "include\secrets.example.h") $sketch
Copy-Item (Join-Path $raiz "diagram.json")              $sketch

$secrets = Join-Path $raiz "include\secrets.h"
if (Test-Path $secrets) {
    Copy-Item $secrets $sketch
} else {
    Write-Warning "No existe include\secrets.h: el sketch va a compilar pero el bot y ThingSpeak NO van a andar."
}

# libraries.txt: lo usa wokwi.com para bajar las librerias externas.
@"
# Librerias externas del proyecto (formato wokwi.com)
DHT sensor library
Adafruit Unified Sensor
Adafruit GFX Library
Adafruit SH110X
Adafruit SSD1306
UniversalTelegramBot
ArduinoJson@6.21.5
ThingSpeak
"@ | Set-Content -Path (Join-Path $sketch "libraries.txt") -Encoding ASCII

# ---- 2) Copia del proyecto PlatformIO (sin compilados) -------------------------
New-Item -ItemType Directory -Path $proy | Out-Null
foreach ($item in "src", "lib", "include", "platformio.ini", "wokwi.toml", "diagram.json", "README.md", "docs") {
    $origen = Join-Path $raiz $item
    if (Test-Path $origen) { Copy-Item $origen -Destination $proy -Recurse }
}

# ---- 3) Zip final -------------------------------------------------------------------
$zip = Join-Path $salida "${Nombre}_entrega.zip"
Compress-Archive -Path $sketch, $proy -DestinationPath $zip

Write-Host ""
Write-Host "Listo:" -ForegroundColor Green
Write-Host "  Sketch .ino : $sketch\$Nombre.ino"
Write-Host "  Para subir  : $zip"
if (Test-Path $secrets) {
    Write-Host ""
    Write-Host "OJO: la entrega incluye secrets.h (token del bot y API key)." -ForegroundColor Yellow
    Write-Host "     Es necesario para que funcione, pero NO lo compartan por otro lado." -ForegroundColor Yellow
}
