/*

Autores:

Augusto Vicente Santos
Diogo de Andrade Chelles
Guilherme Fellipe Alves Silveira
Luanda da Silva Leite
Lucas de Oliveira Donega

Data de entrega do projeto: 5 de junho de 2026
Versão: 1.0

Programa: Controle do ar Condicionado Via MQTT
Descrição: Controlaremos um ar condicionado via mqtt com um ESP32 e um infravermelho com LED, aplicando os modos FAN, AUTO e COOL, ligando e desligando o ar Condicionado. Quando um Json for enviado para o Nosso ESP32, esse microcontrolador vai ativar o infravermelho, enviando o sinal solicitado (pelo usuário) para o ar condicionado
*/

// main.cpp

#include <Arduino.h>
#include <ArduinoJson.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Fujitsu.h>

#include "WiFiManager.h"
#include "MqttManager.h"
#include "DebugManager.h"

//*========================================================================================
//*                      ⇩ ⇩ ⇩      CONSTANTES E VARIÁVEIS      ⇩ ⇩ ⇩
//*========================================================================================

const char TOPICO_COMANDO[] = "senai134/shared/projeto/ar_condicionado";

const uint8_t PINO_AC1 = 13;
const uint8_t PINO_AC2 = 16;
const uint8_t PINO_AC3 = 27;
const uint8_t PINO_AC4 = 26;

int tempDefault = 24;

//*========================================================================================
//*                           ⇩ ⇩ ⇩      INSTÂNCIAS      ⇩ ⇩ ⇩
//*========================================================================================

IRFujitsuAC ac1(PINO_AC1);
IRFujitsuAC ac2(PINO_AC2);
IRFujitsuAC ac3(PINO_AC3);
IRFujitsuAC ac4(PINO_AC4);

//*========================================================================================
//*                           ⇩ ⇩ ⇩      PROTÓTIPOS      ⇩ ⇩ ⇩
//*========================================================================================

void tratarMensagemRecebida(const char *topico, const String &mensagem);

void tratarJsonComando(const String &mensagem);

//*========================================================================================
//*                             ⇩ ⇩ ⇩      SETUP      ⇩ ⇩ ⇩
//*========================================================================================

void setup()
{
    ac1.begin();
    ac2.begin();
    ac3.begin();
    ac4.begin();

    configurarDebug();

    conectarWiFi();

    configurarMQTT();

    registrarCallbackMensagem(tratarMensagemRecebida);

    conectarMQTT();

    ac1.setModel(ARRAH2E);
    ac2.setModel(ARRAH2E);
    ac3.setModel(ARRAH2E);
    ac4.setModel(ARRAH2E);

    //* Estado inicial do Ar Condicionado  ⇩

    // ac.setCmd(kFujitsuAcCmdTurnOn);

    // ac.setMode(kFujitsuAcModeCool);

    // ac.setTemp(tempDefault);

    // ac.setFanSpeed(kFujitsuAcFanAuto);

    // ac.setSwing(kFujitsuAcSwingOff);

    debugInfo("Controle Fujitsu inicializado");

    publicarMensagemNoTopico(1, ac1.toString().c_str());
    publicarMensagemNoTopico(1, ac2.toString().c_str());
    publicarMensagemNoTopico(1, ac3.toString().c_str());
    publicarMensagemNoTopico(1, ac4.toString().c_str());
}

//*========================================================================================
//*                              ⇩ ⇩ ⇩      LOOP      ⇩ ⇩ ⇩
//*========================================================================================

void loop()
{
    garantirWiFiConectado();

    garantirMQTTconectado();

    loopMQTT();
}

//*========================================================================================

//*                         ⇩ ⇩ ⇩      CALLBACK MQTT      ⇩ ⇩ ⇩

//*========================================================================================

void tratarMensagemRecebida(const char *topico, const String &mensagem)
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

//*========================================================================================

//*                        ⇩ ⇩ ⇩      TRATAMENTO JSON      ⇩ ⇩ ⇩

//*========================================================================================

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

    JsonObject acObjetoJson;

    IRFujitsuAC *acSelecionado = nullptr;

    String nomeAc;

    if (doc["arCondicionado1"].is<JsonObject>())
    {
        acObjetoJson = doc["arCondicionado1"];
        acSelecionado = &ac1;
        nomeAc = "AC1";
    }
    else if (doc["arCondicionado2"].is<JsonObject>())
    {
        acObjetoJson = doc["arCondicionado2"];
        acSelecionado = &ac2;
        nomeAc = "AC2";
    }
    else if (doc["arCondicionado3"].is<JsonObject>())
    {
        acObjetoJson = doc["arCondicionado3"];
        acSelecionado = &ac3;
        nomeAc = "AC3";
    }
    else if (doc["arCondicionado4"].is<JsonObject>())
    {
        acObjetoJson = doc["arCondicionado4"];
        acSelecionado = &ac4;
        nomeAc = "AC4";
    }
    else
    {
        debugErro("Nenhum ar condicionado encontrado no JSON");
        return;
    }

    bool alterouEstado = false;

    //*========================================================================================

    //*                            ⇩ ⇩ ⇩      POWER      ⇩ ⇩ ⇩

    //*========================================================================================

    if (acObjetoJson["power"].is<bool>())
    {
        bool power = acObjetoJson["power"].as<bool>();

        if (power)
        {
            acSelecionado->setCmd(kFujitsuAcCmdTurnOn);
            acSelecionado->setSwing(kFujitsuAcSwingOff);
            acSelecionado->send();
            publicarMensagemNoTopico(3, "Comando enviado.\n\rPower: ON");
        }
        else
        {
            acSelecionado->setCmd(kFujitsuAcCmdTurnOff);
            acSelecionado->send();
            publicarMensagemNoTopico(3, "Comando enviado.\n\rPower: OFF");
        }

        alterouEstado = true;
    }

    //*========================================================================================

    //*                        ⇩ ⇩ ⇩      DEFINIÇÃO DOS MODOS      ⇩ ⇩ ⇩

    //*========================================================================================

    //! SERÃO IMPLEMENTADOS APENAS OS MODOS "0", "1" E "3".
    //! FALAR COM GRUPO "IHM" SOBRE.
    //! FALAR TAMBÉM SOBRE O STATUS DO APARELHO (SUGERIR UMA TELA DE STATUS DO AR CONDICIONADO)

    if (acObjetoJson["modo"].is<int>())
    {
        int modo = acObjetoJson["modo"].as<int>();

        if (modo == 0 || modo == 1 || modo == 3)
        {
            acSelecionado->setMode(modo);
            acSelecionado->send();

            if (modo == 0)
            {
                publicarMensagemNoTopico(3, "Comando enviado.\n\rModo: Auto");
            }

            else if (modo == 1)
            {
                publicarMensagemNoTopico(3, "Comando enviado.\n\rModo: Cool");
            }

            else if (modo == 3)
            {
                publicarMensagemNoTopico(3, "Comando enviado.\n\rModo: Fan");
            }

            alterouEstado = true;
        }
        else
        {
            debugErro("Modo invalido\n\rUtilize: \"modo\": 0\n\r\"modo\": 1\n\rou\n\r\"modo\": 3");
        }
    }

    //*========================================================================================

    //*                     ⇩ ⇩ ⇩      DEFINIÇÃO DA TEMPERATURA      ⇩ ⇩ ⇩

    //*========================================================================================

    if (acObjetoJson["temperatura"].is<int>())
    {
        tempDefault = acObjetoJson["temperatura"].as<int>();

        if (tempDefault >= 16 && tempDefault <= 30)
        {
            acSelecionado->setTemp(tempDefault);
            acSelecionado->send();

            debugInfo("Temperatura: " + Serial.print(tempDefault) + Serial.println("°C"));
            publicarMensagemNoTopico(3, "Comando enviado.");

            alterouEstado = true;
        }
        else
        {
            publicarMensagemNoTopico(3, "Temperatura fora da faixa (16°C - 30°C)");
        }
    }

    //*========================================================================================

    //*                ⇩ ⇩ ⇩      AUMENTAR TEMPERATURA DE 1 EM 1 GRAU      ⇩ ⇩ ⇩

    //*========================================================================================

    if (acObjetoJson["aumentar_temp"].is<bool>())
    {
        int aumentar_temp = acObjetoJson["aumentar_temp"].as<bool>();

        if (aumentar_temp)
        {
            tempDefault++;

            if (tempDefault <= 30)
            {
                acSelecionado->setTemp(tempDefault);
                acSelecionado->send();

                debugInfo("Temperatura atual: " + String(tempDefault) + Serial.println("°C"));
                publicarMensagemNoTopico(3, "Comando enviado.\n\r+1°C");

                alterouEstado = true;
            }

            else
            {
                tempDefault = 30;

                publicarMensagemNoTopico(3, "Limite de temperatura máxima atingido: 30°C");
            }
        }

        else
        {
            publicarMensagemNoTopico(3, "Comando inválido\n\rUtilize: \"aumentar_temp\": true");
        }
    }

    //*========================================================================================

    //*                ⇩ ⇩ ⇩      DIMINUIR TEMPERATURA DE 1 EM 1 GRAU      ⇩ ⇩ ⇩

    //*========================================================================================

    if (acObjetoJson["diminuir_temp"].is<bool>())
    {
        int diminuir_temp = acObjetoJson["diminuir_temp"].as<bool>();

        if (diminuir_temp)
        {
            tempDefault--;

            if (tempDefault >= 16)
            {
                acSelecionado->setTemp(tempDefault);
                acSelecionado->send();

                debugInfo("Temperatura atual: " + String(tempDefault) + Serial.println("°C"));
                publicarMensagemNoTopico(3, "Comando enviado.\n\r-1°C");

                alterouEstado = true;
            }

            else
            {
                tempDefault = 16;

                publicarMensagemNoTopico(3, "Limite de temperatura mínima atingido: 16°C");
            }
        }

        else
        {
            publicarMensagemNoTopico(3, "Comando inválido\n\rUtilize: \"diminuir_temp\": true");
        }
    }

    //*========================================================================================

    //*                                ⇩ ⇩ ⇩      FAN      ⇩ ⇩ ⇩

    //*========================================================================================

    if (acObjetoJson["fan"].is<int>())
    {
        int fan = acObjetoJson["fan"].as<int>();

        if (fan >= 0 && fan <= 4)
        {

            if (fan == 0)
            {
                acSelecionado->setFanSpeed(kFujitsuAcFanAuto);
                acSelecionado->send();

                publicarMensagemNoTopico(3, "Comando enviado.\n\rFan: Auto");

                alterouEstado = true;
            }

            else if (fan == 1)
            {
                acSelecionado->setFanSpeed(kFujitsuAcFanHigh);
                acSelecionado->send();

                publicarMensagemNoTopico(3, "Comando enviado.\n\rFan: High");

                alterouEstado = true;
            }
            else if (fan == 2)
            {
                acSelecionado->setFanSpeed(kFujitsuAcFanMed);
                acSelecionado->send();

                publicarMensagemNoTopico(3, "Comando enviado.\n\rFan: Med");

                alterouEstado = true;
            }
            else if (fan == 3)
            {
                acSelecionado->setFanSpeed(kFujitsuAcFanLow);
                acSelecionado->send();

                publicarMensagemNoTopico(3, "Comando enviado.\n\rFan: Low");

                alterouEstado = true;
            }
            else if (fan == 4)
            {
                acSelecionado->setFanSpeed(kFujitsuAcFanQuiet);
                acSelecionado->send();

                publicarMensagemNoTopico(3, "Comando enviado.\n\rFan: Quiet");

                alterouEstado = true;
            }
        }

        else
        {
            publicarMensagemNoTopico(3, "Velocidade fan inválida");
        }
    }

    //*========================================================================================

    //*                   ⇩ ⇩ ⇩      ENVIA STATUS DO AC      ⇩ ⇩ ⇩

    //*========================================================================================

    if (alterouEstado)
    {

        JsonDocument resposta;

        resposta["handshake"]["situacao"] = true;

        String jsonResposta;

        serializeJson(resposta, jsonResposta);

        publicarMensagemNoTopico(4, jsonResposta.c_str());

        //* Status do AC  ⇩
        publicarMensagemNoTopico(1, acSelecionado->toString().c_str());
    }

    else
    {
        //* tópico "status"  ⇩⇩⇩
        publicarMensagemNoTopico(1, acSelecionado->toString().c_str());

        //* tópico "resposta"  ⇩⇩⇩
        publicarMensagemNoTopico(3, "Nenhum parâmetro válido recebido");
        publicarMensagemNoTopico(3, acSelecionado->toString().c_str());
    }
}