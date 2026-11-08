#include "BotTelegram.h"

// ---------------------------------------------------------------------------
// Teclados inline (botones debajo del mensaje).
// Formato JSON que pide Telegram: una lista de FILAS, cada fila una lista de
// botones. "callback_data" es el texto que le llega al ESP32 al tocar el boton
// (usamos el mismo comando, asi se procesa igual que si lo escribieran).
// ---------------------------------------------------------------------------
static const char TECLADO_INICIO[] = R"json([
  [{"text":"🌡️ Ver Clima","callback_data":"/clima"}],
  [{"text":"🔌 Encender Relé (5 s)","callback_data":"/releon 5"},
   {"text":"⏹️ Apagar Relé","callback_data":"/releoff"}],
  [{"text":"📤 Subir potenciómetro","callback_data":"/thingup"}],
  [{"text":"📟 Ver Display","callback_data":"/menudisplay"}]
])json";

static const char TECLADO_DISPLAY[] = R"json([
  [{"text":"💡 LEDs","callback_data":"/displayled"},
   {"text":"🎚️ Pote","callback_data":"/displaypot"},
   {"text":"🌡️ DHT22","callback_data":"/displaydht"}]
])json";

// Menu de comandos que aparece al tocar "/" en el chat.
static const char LISTA_COMANDOS[] = R"json([
  {"command":"start","description":"Menu principal con botones"},
  {"command":"clima","description":"Temperatura y humedad"},
  {"command":"releon","description":"Prender rele N segundos: /releon 10"},
  {"command":"releoff","description":"Apagar rele"},
  {"command":"thingup","description":"Subir voltaje del pote a ThingSpeak"},
  {"command":"displayled","description":"OLED: estado de los LEDs"},
  {"command":"displaypot","description":"OLED: potenciometro"},
  {"command":"displaydht","description":"OLED: temperatura y humedad"}
])json";

// ===========================================================================
BotTelegram::BotTelegram(const char *token, Dispositivos dispositivos, ConfigBot config)
    : bot(token, clienteSeguro), d(dispositivos), cfg(config) {}

void BotTelegram::iniciar() {
    // Certificado raiz de api.telegram.org (viene en la libreria).
    clienteSeguro.setCACert(TELEGRAM_CERTIFICATE_ROOT);

    // La libreria acepta respuestas de hasta 1500 bytes por defecto. Cuando se
    // toca un boton inline, Telegram reenvia el mensaje ENTERO que tenia el
    // teclado (el de /start pesa ~1700 bytes): no entra, falla el parseo, el
    // mensaje nunca se marca como leido y el bot queda trabado para siempre.
    // El ESP32 tiene RAM de sobra, asi que agrandamos el limite.
    bot.maxMessageLength = 6000;

    if (!bot.setMyCommands(LISTA_COMANDOS)) {
        Serial.println("[Telegram] No se pudo registrar el menu de comandos (token o red?)");
    }
    if (strlen(cfg.chatAlertas) > 0) {
        avisar("✅ ESP32 en línea. Mandá /start para ver el menú.");
    }
}

void BotTelegram::atender() {
    if (millis() - ultimaConsulta < cfg.intervaloConsultaMs) return;
    ultimaConsulta = millis();
    if (WiFi.status() != WL_CONNECTED) return;

    // getUpdates trae los mensajes posteriores al ultimo que ya procesamos.
    int cantidad = bot.getUpdates(bot.last_message_received + 1);
    while (cantidad > 0) {
        for (int i = 0; i < cantidad; i++) {
            procesarMensaje(bot.messages[i]);
        }
        cantidad = bot.getUpdates(bot.last_message_received + 1);
    }
}

// ---------------------------------------------------------------------------
// Despachador: separa "comando" y "argumento" y llama a la funcion que toca.
// ---------------------------------------------------------------------------
void BotTelegram::procesarMensaje(const telegramMessage &msg) {
    // Si vino de un boton inline, hay que "contestar" el callback para que
    // Telegram saque el relojito de carga del boton.
    if (msg.type == "callback_query") {
        bot.answerCallbackQuery(msg.query_id);
    }

    const String &chat = msg.chat_id;
    ultimoChat = chat;

    String texto = normalizar(msg.text);
    Serial.printf("[Telegram] %s: %s\n", msg.from_name.c_str(), texto.c_str());

    int espacio = texto.indexOf(' ');
    String comando = espacio < 0 ? texto : texto.substring(0, espacio);
    String argumento = espacio < 0 ? "" : texto.substring(espacio + 1);
    argumento.trim();

    if (comando == "/start" || comando == "/ayuda" || comando == "/help") {
        cmdStart(chat);
    } else if (comando == "/clima") {
        cmdClima(chat);
    } else if (comando.startsWith("/rele")) {
        // Acepta "/releon 10" y tambien "/rele on 10".
        String accion = comando.substring(5);
        if (accion.isEmpty()) {
            int sp = argumento.indexOf(' ');
            accion = sp < 0 ? argumento : argumento.substring(0, sp);
            argumento = sp < 0 ? "" : argumento.substring(sp + 1);
            argumento.trim();
        }
        cmdRele(chat, accion, argumento);
    } else if (comando == "/thingup") {
        cmdThingUp(chat);
    } else if (comando == "/menudisplay") {
        cmdMenuDisplay(chat);
    } else if (comando.startsWith("/display")) {
        // Acepta "/displayled" y tambien "/display led".
        String sub = comando.substring(8);
        if (sub.isEmpty()) sub = argumento;
        cmdDisplay(chat, sub);
    } else {
        cmdDesconocido(chat, comando);
    }

    refrescarPantalla();   // por si el comando cambio algo que se esta mostrando
}

// ---------------------------------------------------------------------------
void BotTelegram::cmdStart(const String &chat) {
    String texto =
        "👋 *Bienvenido al TP02 - ESP32 UTN*\n\n"
        "Usá los botones o escribí:\n"
        "/clima - temperatura y humedad\n"
        "/releon N - prende el relé N segundos\n"
        "/releoff - apaga el relé\n"
        "/thingup - sube el voltaje del pote a ThingSpeak\n"
        "/displayled - estado de los LEDs en la OLED\n"
        "/displaypot - potenciómetro en la OLED\n"
        "/displaydht - DHT22 en la OLED";
    bot.sendMessageWithInlineKeyboard(chat, texto, "Markdown", TECLADO_INICIO);
}

void BotTelegram::cmdClima(const String &chat) {
    if (!d.dht.hayLectura()) {
        bot.sendMessage(chat, "⚠️ Todavía no hay una lectura válida del DHT22. Probá en unos segundos.");
        return;
    }
    String texto = "🌡️ *Clima actual*\n";
    texto += "Temperatura: " + String(d.dht.temperatura(), 1) + " °C\n";
    texto += "Humedad: " + String(d.dht.humedad(), 1) + " %\n";
    texto += "(lectura de hace " + String(d.dht.segundosDesdeLectura()) + " s)";
    bot.sendMessage(chat, texto, "Markdown");
}

void BotTelegram::cmdRele(const String &chat, const String &accion, const String &argumento) {
    if (accion == "on") {
        long segundos = cfg.releSegundosDefecto;
        String nota = "";
        if (argumento.isEmpty()) {
            nota = "\n(No indicaste tiempo: uso " + String(segundos) + " s por defecto.)";
        } else {
            segundos = argumento.toInt();   // "abc" -> 0 -> invalido
        }
        if (segundos <= 0 || segundos > (long)cfg.releSegundosMax) {
            bot.sendMessage(chat, "❌ Tiempo inválido. Usá /releon <segundos> (de 1 a " +
                                      String(cfg.releSegundosMax) + ").");
            return;
        }
        d.rele.encenderPor(segundos * 1000UL);   // NO bloquea: se apaga desde loop()
        bot.sendMessage(chat, "🔌 Relé ENCENDIDO por " + String(segundos) +
                                  " s. Se apaga solo." + nota);
    } else if (accion == "off") {
        d.rele.apagar();
        bot.sendMessage(chat, "⏹️ Relé APAGADO.");
    } else {
        bot.sendMessage(chat, "❓ Uso: /releon <segundos>  o  /releoff");
    }
}

void BotTelegram::cmdThingUp(const String &chat) {
    float v = d.pote.voltaje();
    ResultadoEnvio r = d.nube.enviarVoltaje(v);

    String texto = "🎚️ Potenciómetro: " + String(v, 2) + " V (" + String(d.pote.porcentaje()) + " %)\n";
    switch (r) {
        case ResultadoEnvio::ENVIADO:
            texto += "📤 Enviado a ThingSpeak (field3).";
            break;
        case ResultadoEnvio::ENCOLADO:
            texto += "⏳ ThingSpeak acepta 1 envío cada 15 s: se manda en ~" +
                     String(d.nube.segundosParaPoderEnviar()) + " s.";
            break;
        case ResultadoEnvio::ERROR:
            texto += "❌ Error al enviar a ThingSpeak (código " + String(d.nube.ultimoCodigo()) + ").";
            break;
        case ResultadoEnvio::SIN_CONFIGURAR:
            texto += "⚠️ ThingSpeak sin configurar (completar secrets.h).";
            break;
    }
    texto += "\nUmbral crítico: " + String(d.pote.umbral(), 2) + " V";
    bot.sendMessage(chat, texto);

    // Evaluacion del umbral pedida por el enunciado.
    if (v > d.pote.umbral()) {
        alertaVoltaje(v);
    }
}

void BotTelegram::cmdMenuDisplay(const String &chat) {
    bot.sendMessageWithInlineKeyboard(chat, "📟 ¿Qué querés ver en la pantalla OLED?", "",
                                      TECLADO_DISPLAY);
}

void BotTelegram::cmdDisplay(const String &chat, const String &sub) {
    String que;
    if (sub == "led" || sub == "leds") {
        modo = ModoPantalla::LED;
        que = "el estado de los LEDs";
    } else if (sub == "pot" || sub == "pote" || sub == "potenciometro") {
        modo = ModoPantalla::POTENCIOMETRO;
        que = "el potenciómetro";
    } else if (sub == "dht" || sub == "dht22" || sub == "clima") {
        modo = ModoPantalla::DHT;
        que = "temperatura y humedad";
    } else {
        // Comando no identificado: se informa EN LA PANTALLA (lo pide el enunciado).
        modo = ModoPantalla::ERROR;
        comandoInvalido = "/display" + sub;
        bot.sendMessage(chat, "❓ Comando de display no identificado: /display" + sub +
                                  "\nOpciones: /displayled, /displaypot, /displaydht");
        return;
    }
    bot.sendMessage(chat, "📟 La OLED ahora muestra " + que + ".");
}

void BotTelegram::cmdDesconocido(const String &chat, const String &comando) {
    bot.sendMessage(chat, "🤔 No conozco el comando " + comando + ". Mandá /start para ver las opciones.");
}

// ---------------------------------------------------------------------------
// Avisos espontaneos (el ESP32 escribe sin que nadie le pregunte)
// ---------------------------------------------------------------------------
void BotTelegram::avisar(const String &texto) {
    String destino = strlen(cfg.chatAlertas) > 0 ? String(cfg.chatAlertas) : ultimoChat;
    if (destino.isEmpty()) {
        Serial.println("[Telegram] Aviso sin destinatario (nadie le hablo al bot todavia): " + texto);
        return;
    }
    if (WiFi.status() != WL_CONNECTED) return;
    bot.sendMessage(destino, texto);
}

void BotTelegram::alertaVoltaje(float voltaje) {
    // Anti-spam: como maximo una alerta cada esperaEntreAlertasMs.
    if (huboAlerta && millis() - ultimaAlerta < cfg.esperaEntreAlertasMs) return;
    huboAlerta = true;
    ultimaAlerta = millis();
    avisar("🚨 ALERTA DE SEGURIDAD\nEl potenciómetro marca " + String(voltaje, 2) +
           " V y supera el umbral crítico de " + String(d.pote.umbral(), 2) + " V.");
}

// ---------------------------------------------------------------------------
// Pantalla OLED segun el ultimo /display recibido
// ---------------------------------------------------------------------------
String BotTelegram::estadoLed(const Led &led) {
    if (!led.estaEncendido()) return "  Led apagado";
    if (led.estaTemporizado()) return "  Led encendido " + String(led.segundosRestantes()) + "s";
    return "  Led encendido";
}

void BotTelegram::refrescarPantalla() {
    switch (modo) {
        case ModoPantalla::INICIO:
            d.pantalla.mostrar("TP02 IoT - UTN",
                               WiFi.status() == WL_CONNECTED ? "WiFi conectado" : "WiFi desconectado",
                               "IP " + WiFi.localIP().toString(),
                               "Esperando comandos",
                               "de Telegram...");
            break;

        case ModoPantalla::LED:
            d.pantalla.mostrar("Estado de los LEDs",
                               "Verde (rele):", estadoLed(d.rele),
                               "Azul (alerta):", estadoLed(d.ledAzul));
            break;

        case ModoPantalla::POTENCIOMETRO: {
            float v = d.pote.voltaje();
            d.pantalla.mostrarValor("Potenciometro", String(v, 2) + " V",
                                    d.pote.enAlarma() ? "SUPERA UMBRAL!"
                                                      : "Umbral: " + String(d.pote.umbral(), 2) + " V");
            break;
        }

        case ModoPantalla::DHT:
            if (d.dht.hayLectura()) {
                d.pantalla.mostrar("Sensor DHT22",
                                   "Temp: " + String(d.dht.temperatura(), 1) + " C",
                                   "Hum:  " + String(d.dht.humedad(), 1) + " %",
                                   "",
                                   "hace " + String(d.dht.segundosDesdeLectura()) + " s");
            } else {
                d.pantalla.mostrar("Sensor DHT22", "Sin lectura valida", "Revisar conexion");
            }
            break;

        case ModoPantalla::ERROR:
            d.pantalla.mostrar("Comando invalido",
                               "No identificado:", comandoInvalido,
                               "Usar: led | pot |", "      dht");
            break;
    }
}

// ---------------------------------------------------------------------------
String BotTelegram::normalizar(String texto) {
    texto.trim();
    texto.toLowerCase();
    // En grupos Telegram manda "/clima@NombreDelBot": sacamos el "@...".
    int arroba = texto.indexOf('@');
    int espacio = texto.indexOf(' ');
    if (arroba > 0 && (espacio < 0 || arroba < espacio)) {
        texto = texto.substring(0, arroba) + (espacio < 0 ? "" : texto.substring(espacio));
    }
    return texto;
}
