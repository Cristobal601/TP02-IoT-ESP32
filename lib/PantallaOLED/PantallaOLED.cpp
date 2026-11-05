#include "PantallaOLED.h"

PantallaOLED::PantallaOLED(uint8_t direccion)
    : display(ANCHO, ALTO, &Wire, -1), direccion(direccion) {}

bool PantallaOLED::iniciar(uint8_t pinSDA, uint8_t pinSCL) {
    Wire.begin(pinSDA, pinSCL);

#if defined(OLED_SH1106)
    ok = display.begin(direccion, true);
#else
    ok = display.begin(SSD1306_SWITCHCAPVCC, direccion);
#endif

    if (!ok) {
        Serial.println("[OLED] No se encontro la pantalla (revisar cables / direccion I2C)");
        return false;
    }
    display.clearDisplay();
    display.setTextColor(OLED_BLANCO);
    display.setTextWrap(false);   // nosotros controlamos el largo de cada linea
    display.display();
    return true;
}

bool PantallaOLED::disponible() const { return ok; }

void PantallaOLED::dibujarTitulo(const String &titulo) {
    // Barra blanca con el titulo en negro (texto "invertido").
    display.fillRect(0, 0, ANCHO, 11, OLED_BLANCO);
    display.setTextColor(OLED_NEGRO);
    display.setTextSize(1);
    display.setCursor(2, 2);
    display.print(adaptar(titulo));
    display.setTextColor(OLED_BLANCO);
}

void PantallaOLED::mostrar(const String &titulo, const String &linea1, const String &linea2,
                           const String &linea3, const String &linea4) {
    if (!ok) return;
    display.clearDisplay();
    dibujarTitulo(titulo);

    const String *lineas[] = {&linea1, &linea2, &linea3, &linea4};
    for (int i = 0; i < 4; i++) {
        display.setCursor(0, 14 + i * 12);
        display.print(adaptar(*lineas[i]));
    }
    display.display();
}

void PantallaOLED::mostrarValor(const String &titulo, const String &valor, const String &pie) {
    if (!ok) return;
    display.clearDisplay();
    dibujarTitulo(titulo);

    // Valor grande (tamaño 2 = 12x16 px por letra), centrado.
    String v = adaptar(valor);
    display.setTextSize(2);
    int anchoTexto = v.length() * 12;
    display.setCursor(max(0, (ANCHO - anchoTexto) / 2), 22);
    display.print(v);

    display.setTextSize(1);
    display.setCursor(0, 50);
    display.print(adaptar(pie));
    display.display();
}

String PantallaOLED::adaptar(String texto) {
    // La fuente de Adafruit GFX solo tiene ASCII basico. Los caracteres
    // UTF-8 (tildes, ñ, °) se verian como basura, asi que se reemplazan.
    const char *originales[] = {"á", "é", "í", "ó", "ú", "Á", "É", "Í", "Ó", "Ú",
                                "ñ", "Ñ", "ü", "Ü", "°", "¿", "¡"};
    const char *reemplazos[] = {"a", "e", "i", "o", "u", "A", "E", "I", "O", "U",
                                "n", "N", "u", "U", "", "", ""};
    for (size_t i = 0; i < sizeof(originales) / sizeof(originales[0]); i++) {
        texto.replace(originales[i], reemplazos[i]);
    }

    // Cualquier otro caracter no ASCII (por ej. emojis) se cambia por '?'.
    String limpio;
    for (size_t i = 0; i < texto.length(); i++) {
        uint8_t c = texto[i];
        if (c < 0x80) {
            limpio += (char)c;
        } else if ((c & 0xC0) == 0xC0) {
            limpio += '?';   // primer byte de un caracter multibyte
        }                    // los bytes de continuacion (10xxxxxx) se descartan
    }

    if (limpio.length() > CARACTERES_POR_LINEA) {
        limpio = limpio.substring(0, CARACTERES_POR_LINEA);
    }
    return limpio;
}
