/*

Autores:

Augusto Vicente Santos
Diogo de Andrade Chelles
Guilherme Fellipe
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

const char *numeroDoAc = "A/C 1";

const uint8_t PINO_IR = 13;

int tempDefault = 24;

//*========================================================================================
//*                           ⇩ ⇩ ⇩      INSTÂNCIAS      ⇩ ⇩ ⇩
//*========================================================================================

IRFujitsuAC ac(PINO_IR);

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
    ac.begin();

    configurarDebug();

    conectarWiFi();

    configurarMQTT();

    registrarCallbackMensagem(tratarMensagemRecebida);

    conectarMQTT();

    ac.setModel(ARRAH2E);

    //* Estado inicial do Ar Condicionado  ⇩

    // ac.setCmd(kFujitsuAcCmdTurnOn);

    // ac.setMode(kFujitsuAcModeCool);

    // ac.setTemp(tempDefault);

    // ac.setFanSpeed(kFujitsuAcFanAuto);

    // ac.setSwing(kFujitsuAcSwingOff);

    debugInfo("Controle Fujitsu inicializado");

    publicarMensagemNoTopico(1, ac.toString().c_str());
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

    if (!doc["arCondicionado"].is<JsonObject>())
    {
        debugErro("Objeto \"arCondicionado\" não encontrado");
        return;
    }

    bool alterouEstado = false;

    //*========================================================================================

    //*                            ⇩ ⇩ ⇩      POWER      ⇩ ⇩ ⇩

    //*========================================================================================

    if (doc["arCondicionado"]["power"].is<bool>())
    {
        bool power = doc["arCondicionado"]["power"].as<bool>();

        if (power)
        {
            ac.setCmd(kFujitsuAcCmdTurnOn);
            ac.send();
            publicarMensagemNoTopico(3, "Comando enviado.\n\rPower: ON");
        }
        else
        {
            ac.setCmd(kFujitsuAcCmdTurnOff);
            ac.send();
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

    if (doc["arCondicionado"]["modo"].is<int>())
    {
        int modo = doc["arCondicionado"]["modo"].as<int>();

        if (modo == 0 || modo == 1 || modo == 3)
        {
            ac.setMode(modo);
            ac.send();

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

    if (doc["arCondicionado"]["temperatura"].is<int>())
    {
        tempDefault = doc["arCondicionado"]["temperatura"].as<int>();

        if (tempDefault >= 16 && tempDefault <= 30)
        {
            ac.setTemp(tempDefault);
            ac.send();

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

    if (doc["arCondicionado"]["aumentar_temp"].is<bool>())
    {
        int aumentar_temp = doc["arCondicionado"]["aumentar_temp"].as<bool>();

        if (aumentar_temp)
        {
            tempDefault++;

            if (tempDefault <= 30)
            {
                ac.setTemp(tempDefault);
                ac.send();

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

    if (doc["arCondicionado"]["diminuir_temp"].is<bool>())
    {
        int diminuir_temp = doc["arCondicionado"]["diminuir_temp"].as<bool>();

        if (diminuir_temp)
        {
            tempDefault--;

            if (tempDefault >= 16)
            {
                ac.setTemp(tempDefault);
                ac.send();

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

    if (doc["arCondicionado"]["fan"].is<int>())
    {
        int fan = doc["arCondicionado"]["fan"].as<int>();

        if (fan >= 0 && fan <= 4)
        {

            if (fan == 0)
            {
                ac.setFanSpeed(kFujitsuAcFanAuto);
                ac.send();

                publicarMensagemNoTopico(3, "Comando enviado.\n\rFan: Auto");

                alterouEstado = true;
            }

            else if (fan == 1)
            {
                ac.setFanSpeed(kFujitsuAcFanHigh);
                ac.send();

                publicarMensagemNoTopico(3, "Comando enviado.\n\rFan: High");

                alterouEstado = true;
            }
            else if (fan == 2)
            {
                ac.setFanSpeed(kFujitsuAcFanMed);
                ac.send();

                publicarMensagemNoTopico(3, "Comando enviado.\n\rFan: Med");

                alterouEstado = true;
            }
            else if (fan == 3)
            {
                ac.setFanSpeed(kFujitsuAcFanLow);
                ac.send();

                publicarMensagemNoTopico(3, "Comando enviado.\n\rFan: Low");

                alterouEstado = true;
            }
            else if (fan == 4)
            {
                ac.setFanSpeed(kFujitsuAcFanQuiet);
                ac.send();

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
        publicarMensagemNoTopico(1, ac.toString().c_str());
    }

    else
    {
        //* tópico "status"  ⇩⇩⇩
        publicarMensagemNoTopico(1, ac.toString().c_str());

        //* tópico "resposta"  ⇩⇩⇩
        publicarMensagemNoTopico(3, "Nenhum parâmetro válido recebido");
        publicarMensagemNoTopico(3, ac.toString().c_str());
    }
}