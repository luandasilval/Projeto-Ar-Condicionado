/*
Autores:
Augusto Vicente Santos
Diogo de Andrade Chelles
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

//*********************** CONSTANTES E VARIÁVEIS *****************************

const char TOPICO_COMANDO[] = "senai134/shared";

const char *numeroDoAc = "A/C 1";

const uint8_t PINO_IR = 42;

int tempDefault = 24;

//*********************** INSTÂNCIAS *****************************

IRFujitsuAC ac(PINO_IR);

//*********************** PROTÓTIPOS *****************************

void tratarMensagemRecebida(const char *topico,
                            const String &mensagem);

void tratarJsonComando(const String &mensagem);

//*********************** SETUP *****************************

void setup()
{
    ac.begin();

    configurarDebug();

    conectarWiFi();

    configurarMQTT();

    registrarCallbackMensagem(tratarMensagemRecebida);

    conectarMQTT();

    ac.setModel(ARRAH2E);

    //* Estado inicial do Ar Condicionado  ⇩

    ac.setCmd(kFujitsuAcCmdTurnOn);

    ac.setMode(kFujitsuAcModeCool);

    ac.setTemp(tempDefault);

    ac.setFanSpeed(kFujitsuAcFanAuto);

    ac.setSwing(kFujitsuAcSwingOff);

    debugInfo("Controle Fujitsu inicializado");
}

//*********************** LOOP *****************************

void loop()
{
    garantirWiFiConectado();

    garantirMQTTconectado();

    loopMQTT();
}

//*********************** CALLBACK MQTT *****************************

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

//*********************** TRATAMENTO JSON *****************************

void tratarJsonComando(const String &mensagem)
{
    JsonDocument doc;

    DeserializationError erro = deserializeJson(doc, mensagem);

    if (erro)
    {
        debugErro("Erro ao interpretar JSON");
        debugErro(erro.c_str());
        return;
    }

    if (!doc["ArCondicionado"].is<JsonObject>())
    {
        debugErro("Objeto ArCondicionado nao encontrado");
        return;
    }

    JsonObject objetoJsonAc = doc["ArCondicionado"];

    bool alterouEstado = false;

    //*********************** POWER *****************************

    if (objetoJsonAc["power"].is<bool>())
    {
        bool power = objetoJsonAc["power"];

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

    //*********************** DEFINIÇÃO DOS MODOS *****************************

    //! SERÃO IMPLEMENTADOS APENAS OS MODOS "0", "1" E "3".
    //! FALAR COM GRUPO "IHM" SOBRE.
    //! FALAR TAMBÉM SOBRE O STATUS DO APARELHO (SUGERIR UMA TELA DE STATUS DO AR CONDICIONADO)

    if (objetoJsonAc["modo"].is<int>())
    {
        int modo = objetoJsonAc["modo"];

        if (modo == 0 || modo == 1 || modo == 3)
        {
            ac.setMode(modo);
            ac.send();

            if (modo == 0)
            {
                debugInfo("Modo: Auto");
            }

            else if (modo == 1)
            {
                debugInfo("Modo: Cool");
            }

            else if (modo == 3)
            {
                debugInfo("Modo: Fan");
            }

            alterouEstado = true;
        }
        else
        {
            debugErro("Modo invalido");
        }
    }

    //*********************** DEFINIÇÃO DE TEMPERATURA *****************************

    if (objetoJsonAc["temperatura"].is<int>())
    {
        int temperatura = objetoJsonAc["temperatura"];

        if (temperatura >= 16 && temperatura <= 30)
        {
            ac.setTemp(temperatura);
            ac.send();

            debugInfo("Temperatura: " + String(temperatura));

            alterouEstado = true;
        }
        else
        {
            debugErro("Temperatura fora da faixa (\"16°C\" - \"30°C\")");
        }
    }

    //*********************** AUMENTAR TEMPERATURA DE 1 EM 1 GRAU *****************************

    if (objetoJsonAc["aumentar_temp"].is<bool>())
    {
        int aumentar_temp = objetoJsonAc["aumentar_temp"];

        if (aumentar_temp)
        {
            tempDefault++;

            if (tempDefault <= 30)
            {
                ac.setTemp(tempDefault);
                ac.send();

                debugInfo("Temperatura: " + String(tempDefault) + Serial.println("°C"));

                alterouEstado = true;
            }

            else
            {
                debugErro("Temperatura fora da faixa (\"16°C\" - \"30°C\")");
            }
        }

        else
        {
            debugErro("Comando inválido.");
            debugErro("Use: \"aumentar_temp\": true");
        }

        //*********************** DIMINUIR TEMPERATURA DE 1 EM 1 GRAU *****************************

        if (objetoJsonAc["diminuir_temp"].is<bool>())
        {
            int diminuir_temp = objetoJsonAc["diminuir_temp"];

            if (diminuir_temp)
            {
                tempDefault--;

                if (tempDefault >= 16)
                {
                    ac.setTemp(tempDefault);
                    ac.send();

                    debugInfo("Temperatura: " + String(tempDefault) + Serial.println("°C"));

                    alterouEstado = true;
                }

                else
                {
                    debugErro("Temperatura fora da faixa (\"16°C\" - \"30°C\")");
                }
            }

            else
            {
                debugErro("Comando inválido.");
                debugErro("Use: \"diminuir_temp\": true");
            }
        }

        //*********************** FAN *****************************

        if (objetoJsonAc["fan"].is<int>())
        {
            int fan = objetoJsonAc["fan"];

            if (fan >= 0 && fan <= 4)
            {

                if (objetoJsonAc["fan"] == 0)
                {
                    ac.setFanSpeed(kFujitsuAcFanAuto);
                    ac.send();

                    debugInfo("Fan: Auto");

                    alterouEstado = true;
                }

                else if (objetoJsonAc["fan"] == 1)
                {
                    ac.setFanSpeed(kFujitsuAcFanHigh);
                    ac.send();

                    debugInfo("Fan: High");

                    alterouEstado = true;
                }
                else if (objetoJsonAc["fan"] == 2)
                {
                    ac.setFanSpeed(kFujitsuAcFanMed);
                    ac.send();

                    debugInfo("Fan: Med");

                    alterouEstado = true;
                }
                else if (objetoJsonAc["fan"] == 3)
                {
                    ac.setFanSpeed(kFujitsuAcFanLow);
                    ac.send();

                    debugInfo("Fan: Low");

                    alterouEstado = true;
                }
               else if (objetoJsonAc["fan"])
                {
                    ac.setFanSpeed(kFujitsuAcFanQuiet);
                    ac.send();

                    debugInfo("Fan: Quiet");

                    alterouEstado = true;
                }
            }

            else
            {
                debugErro("Velocidade fan invalida");
            }
        }

        //*********************** PRINTA STATUS DO AC *****************************

        if (alterouEstado)
        {

            publicarMensagemNoTopico(0, "Alterado estado do Ar Condicionado");

            debugInfo(ac.toString());

            Serial.println();
            Serial.println("========== ESTADO AC ==========");
            Serial.println(ac.toString());
            Serial.println("===============================");
        }

        else
        {
            debugErro("Nenhum parâmetro válido recebido");
        }
    }
}

// ac.isNull();