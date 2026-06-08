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

//*========================================================================================
//*                      ⇩ ⇩ ⇩      CONSTANTES E VARIÁVEIS      ⇩ ⇩ ⇩
//*========================================================================================

const char TOPICO_COMANDO[] = "senai134/shared";

const char *numeroDoAc = "A/C 1";

const uint8_t PINO_IR = 42;

int tempDefault = 24;

//*========================================================================================
//*                          ⇩ ⇩ ⇩      INSTÂNCIAS      ⇩ ⇩ ⇩
//*========================================================================================

IRFujitsuAC ac(PINO_IR);

//*========================================================================================
//*                          ⇩ ⇩ ⇩      PROTÓTIPOS      ⇩ ⇩ ⇩
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

    ac.setCmd(kFujitsuAcCmdTurnOn);

    ac.setMode(kFujitsuAcModeCool);

    ac.setTemp(tempDefault);

    ac.setFanSpeed(kFujitsuAcFanAuto);

    ac.setSwing(kFujitsuAcSwingOff);

    debugInfo("Controle Fujitsu inicializado");

    publicarMensagemNoTopico(0, ac.toString().c_str());
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

    //? JsonObject objetoJsonAc = doc["arCondicionado"];

    bool alterouEstado = false;

    //*========================================================================================

    //*                            ⇩ ⇩ ⇩      POWER      ⇩ ⇩ ⇩

    //*========================================================================================

    //? if (objetoJsonAc["power"].is<bool>())
    if (doc["arCondicionado"]["power"].is<bool>())
    {
        //? bool power = objetoJsonAc["power"];
        bool power = doc["arCondicionado"]["power"].as<bool>();

        if (power)
        {
            ac.on();
            //? ac.setCmd(kFujitsuAcCmdTurnOn);
            ac.send();
            debugInfo("Power ON");
            publicarMensagemNoTopico(3, "Comando enviado.");
        }
        else
        {
            ac.off();
            //? ac.setCmd(kFujitsuAcCmdTurnOff);
            ac.send();
            debugInfo("Power OFF");
            publicarMensagemNoTopico(3, "Comando enviado.");
        }

        alterouEstado = true;
    }

    else
    {
        debugErro("Comando inválido.");
        debugErro("Utilize: \"power\": true\n\rou\n\r\"power\": false\n\r");

        publicarMensagemNoTopico(3, "Comando inválido.\n\rUtilize: \"power\": true\n\rou\n\r\"power\": false\n\r");
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
                debugInfo("Modo: Auto");
                publicarMensagemNoTopico(3, "Comando enviado.");
            }

            else if (modo == 1)
            {
                debugInfo("Modo: Cool");
                publicarMensagemNoTopico(3, "Comando enviado.");
            }

            else if (modo == 3)
            {
                debugInfo("Modo: Fan");
                publicarMensagemNoTopico(3, "Comando enviado.");
            }

            alterouEstado = true;
        }
        else
        {
            debugErro("Modo invalido");
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
            debugErro("Temperatura fora da faixa (16°C - 30°C)");
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
                publicarMensagemNoTopico(3, "Comando enviado.");

                alterouEstado = true;
            }

            else
            {
                tempDefault = 30;

                debugErro("Limite de temperatura máxima atingido: 30°C");

                publicarMensagemNoTopico(3, "Limite de temperatura máxima atingido: 30°C");
            }
        }

        else
        {
            debugErro("Comando inválido.");
            debugErro("Utilize: \"aumentar_temp\": true");
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

                debugInfo("Temperatura: " + String(tempDefault) + Serial.println("°C"));
                publicarMensagemNoTopico(3, "Comando enviado.");

                alterouEstado = true;
            }

            else
            {
                tempDefault = 16;

                debugErro("Limite de temperatura mínima atingido: 16°C");

                publicarMensagemNoTopico(3, "Limite de temperatura mínima atingido: 16°C");
            }
        }

        else
        {
            debugErro("Comando inválido.");
            debugErro("Utilize: \"diminuir_temp\": true");

            publicarMensagemNoTopico(3, "Comando inválido");
            publicarMensagemNoTopico(3, "Utilize: \"diminuir_temp\": true");
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

                debugInfo("Fan: Auto");
                publicarMensagemNoTopico(3, "Comando enviado.");

                alterouEstado = true;
            }

            else if (fan == 1)
            {
                ac.setFanSpeed(kFujitsuAcFanHigh);
                ac.send();

                debugInfo("Fan: High");
                publicarMensagemNoTopico(3, "Comando enviado.");

                alterouEstado = true;
            }
            else if (fan == 2)
            {
                ac.setFanSpeed(kFujitsuAcFanMed);
                ac.send();

                debugInfo("Fan: Med");
                publicarMensagemNoTopico(3, "Comando enviado.");

                alterouEstado = true;
            }
            else if (fan == 3)
            {
                ac.setFanSpeed(kFujitsuAcFanLow);
                ac.send();

                debugInfo("Fan: Low");
                publicarMensagemNoTopico(3, "Comando enviado.");

                alterouEstado = true;
            }
            else if (fan == 4)
            {
                ac.setFanSpeed(kFujitsuAcFanQuiet);
                ac.send();

                debugInfo("Fan: Quiet");
                publicarMensagemNoTopico(3, "Comando enviado.");

                alterouEstado = true;
            }
        }

        else
        {
            debugErro("Velocidade fan invalida");
            publicarMensagemNoTopico(3, "Velocidade fan invalida");
        }
    }

    //*========================================================================================

    //*                   ⇩ ⇩ ⇩      ENVIA E PRINTA STATUS DO AC      ⇩ ⇩ ⇩

    //*========================================================================================

    if (alterouEstado)
    {

        //? String statusAc = ac.toString();
        //? publicarMensagemNoTopico(0, statusAc.c_str());

        //* Status do AC  ⇩
        publicarMensagemNoTopico(1, ac.toString().c_str());

        Serial.println();
        Serial.println("========== ESTADO AC ==========");
        //? Serial.println(statusAc);
        Serial.println(ac.toString().c_str());
        Serial.println("===============================");
    }

    else
    {
        //* tópico "resposta"
        debugErro("Nenhum parâmetro válido recebido");
        publicarMensagemNoTopico(3, "Nenhum parâmetro válido recebido");
        Serial.println(ac.toString().c_str());

        //* tópico "status"  ⇩
        publicarMensagemNoTopico(1, "Nenhum parâmetro válido recebido");
        publicarMensagemNoTopico(1, ac.toString().c_str());
    }
}

// ac.isNull();