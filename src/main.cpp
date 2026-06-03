/*
Autores:
Augusto Vicente Santos
Diogo De Andrade Chelles
Luanda da Silva Leite
Lucas de Oliveira Donega

Data de entrega do projeto: 5 de junho de 2026
Versão: 1.0

Programa: Controle do ar Condicionado Via MQTT
Descrição: Controlaremos um ar condicionado via mqtt com um ESP32 e um infravermelho com LED, aplicando os modos FAN, AUTO e COOL, ligando e desligando o ar Condicionado. Quando um Json for enviado para o Nosso ESP32, esse microcontrolador vai ativar o infravermelho, enviando o sinal solicitado (pelo usuário) para o ar condicionado
*/

//*main.cpp

#include <Arduino.h>
#include <ArduinoJson.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Fujitsu.h>

#include "WiFiManager.h"
#include "MqttManager.h"
#include "DebugManager.h"

//====================================================
// CONSTANTES
//====================================================

const char TOPICO_COMANDO[] = "ArCondicionado/AC1/comando";

const uint8_t PINO_IR = 13;

const char *numeroDoAc = "A/C 1";

//====================================================
// INSTÂNCIAS
//====================================================

IRFujitsuAC ac(PINO_IR);

//====================================================
// PROTÓTIPOS
//====================================================

void tratarMensagemRecebida(const char *topico,
                            const String &mensagem);

void tratarJsonComando(const String &mensagem);

//====================================================
// SETUP
//====================================================

void setup()
{
    ac.begin();

    configurarDebug();

    conectarWiFi();

    configurarMQTT();

    registrarCallbackMensagem(tratarMensagemRecebida);

    conectarMQTT();

    ac.setModel(ARRAH2E);

    // Estado inicial

    ac.on();

    ac.setMode(kFujitsuAcModeCool);

    ac.setTemp(24);

    ac.setFanSpeed(kFujitsuAcFanAuto);

    ac.setSwing(kFujitsuAcSwingOff);

    debugInfo("Controle Fujitsu inicializado");
}

//====================================================
// LOOP
//====================================================

void loop()
{
    garantirWiFiConectado();

    garantirMQTTconectado();

    loopMQTT();
}

//====================================================
// CALLBACK MQTT
//====================================================

void tratarMensagemRecebida(const char *topico,
                            const String &mensagem)
{
    debugInfo("========================================");
    debugInfo("Mensagem MQTT recebida");
    debugInfo("========================================");

    if (topico == nullptr)
    {
        debugErro("Topico MQTT invalido");
        return;
    }

    debugInfo("Topico: " + String(topico));
    debugInfo("Payload: " + mensagem);

    if (strcmp(topico, TOPICO_COMANDO) == 0)
    {
        tratarJsonComando(mensagem);
        return;
    }

    debugErro("Topico nao tratado: " + String(topico));
}

//====================================================
// PROCESSAMENTO JSON
//====================================================

void tratarJsonComando(const String &mensagem)
{
    JsonDocument doc;

    DeserializationError erro =
        deserializeJson(doc, mensagem);

    if (erro)
    {
        debugErro("Erro ao interpretar JSON");
        debugErro(erro.c_str());
        return;
    }

    JsonObject ar =
        doc["ArCondicionado"];

    bool alterouEstado = false;

    //------------------------------------------------
    // POWER
    //------------------------------------------------

    if (ar["power"].is<bool>())
    {
        bool power = ar["power"];

        if (power)
        {
            ac.setCmd(kFujitsuAcCmdTurnOn);
            ac.send();
            debugInfo("Power ON");
        }
        else
        {
            ac.setCmd(kFujitsuAcCmdTurnOff);
            ac.send();
            debugInfo("Power OFF");
        }

        alterouEstado = true;
    }

    //------------------------------------------------
    // MODO
    //------------------------------------------------

    if (ar["modo"].is<int>())
    {
        int modo = ar["modo"];

        if (modo >= 0 && modo <= 4)
        {
            ac.setMode(modo);
            ac.send();

            debugInfo("Modo atualizado: " +
                      String(modo));

            alterouEstado = true;
        }
        else
        {
            debugErro("Modo invalido");
        }
    }

    //------------------------------------------------
    // TEMPERATURA
    //------------------------------------------------

    if (ar["temperatura"].is<int>())
    {
        int temperatura =
            ar["temperatura"];

        if (temperatura >= 16 &&
            temperatura <= 30)
        {
            ac.setTemp(temperatura);
            ac.send();

            debugInfo(
                "Temperatura: " +
                String(temperatura));

            alterouEstado = true;
        }
        else
        {
            debugErro(
                "Temperatura fora da faixa (16-30)");
        }
    }

    //------------------------------------------------
    // FAN
    //------------------------------------------------

    if (ar["fan"].is<int>())
    {
        int fan = ar["fan"];

        if (fan >= 0 && fan <= 4)
        {
            ac.setFanSpeed(fan);
            ac.send();

            debugInfo(
                "Fan: " +
                String(fan));

            alterouEstado = true;
        }
        else
        {
            debugErro("Velocidade fan invalida");
        }
    }

    //------------------------------------------------
    // ENVIA COMANDO IR
    //------------------------------------------------

    if (alterouEstado)
    {
        debugInfo("Enviando comando IR...");

        // ac.send();

        debugInfo(ac.toString());

        Serial.println();
        Serial.println("========== ESTADO AC ==========");
        Serial.println(ac.toString());
        Serial.println("===============================");
    }
    else
    {
        debugErro("Nenhum parametro valido recebido");
    }
}