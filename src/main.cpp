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

int tempAC1 = 24;
int tempAC2 = 24;

bool powerAC1 = false;
bool powerAC2 = false;

//*========================================================================================
//*                           ⇩ ⇩ ⇩      INSTÂNCIAS      ⇩ ⇩ ⇩
//*========================================================================================

IRFujitsuAC ac1(PINO_AC1);
IRFujitsuAC ac2(PINO_AC2);

enum valoresDosComandos
{
    AC_POWER = 0,
    AC_AUTO_MODE = 1,
    AC_COOL_MODE = 2,
    AC_FAN_MODE = 3,
    AC_FAN_LOW = 4,
    AC_FAN_MEDIUM = 5,
    AC_FAN_HIGH = 6,
    AC_FAN_QUIET = 7,
    AC_TEMP_DOWN = 8,
    AC_TEMP_UP = 9
};

//*========================================================================================
//*                           ⇩ ⇩ ⇩      PROTÓTIPOS      ⇩ ⇩ ⇩
//*========================================================================================

void tratarMensagemRecebida(const char *topico, const String &mensagem);

void tratarJsonComando(const String &mensagem);

void publicarErroComHandshake(const String &mensagem);

void publicarSucessoComHandshake(const String &mensagem);


//*========================================================================================
//*                             ⇩ ⇩ ⇩      SETUP      ⇩ ⇩ ⇩
//*========================================================================================

void setup()
{
    ac1.begin();
    ac2.begin();

    configurarDebug();

    conectarWiFi();

    configurarMQTT();

    registrarCallbackMensagem(tratarMensagemRecebida);

    conectarMQTT();

    ac1.setModel(ARRAH2E);
    ac2.setModel(ARRAH2E);

    //* Estado inicial do Ar Condicionado  ⇩

    publicarMensagemNoTopico(0, "Controle Fujitsu inicializado");

    publicarMensagemNoTopico(0, ac1.toString().c_str());
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

//*                         ⇩ ⇩ ⇩      HANDSHAKE ERRO      ⇩ ⇩ ⇩

//*========================================================================================

void publicarErroComHandshake(const String &mensagem)
{
    publicarMensagemNoTopico(0, mensagem.c_str());
 
    JsonDocument docHandshake;
    docHandshake["statusComando"]["comando"]  = false;
    docHandshake["statusComando"]["situacao"] = mensagem;
 
    String handshakeFormatado;
    serializeJsonPretty(docHandshake, handshakeFormatado);
    publicarMensagemNoTopico(0, handshakeFormatado.c_str());
}

void publicarSucessoComHandshake(const String &mensagem)
{
    publicarMensagemNoTopico(0, mensagem.c_str());

    JsonDocument docHandshake;
    docHandshake["statusComando"]["comando"]  = true;
    docHandshake["statusComando"]["situacao"] = mensagem;

    String handshakeFormatado;
    serializeJsonPretty(docHandshake, handshakeFormatado);
    publicarMensagemNoTopico(0, handshakeFormatado.c_str());
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

    debugErro("\n\rTopico nao tratado: " + String(topico));
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

    JsonObject objetoJsonAC;

    IRFujitsuAC *acSelecionado = nullptr;

    bool *powerSelecionado = nullptr;

    int *tempSelecionado = nullptr;

    String nomeAc;

    if (doc["arCondicionado1"].is<JsonObject>())
    {
        objetoJsonAC = doc["arCondicionado1"];
        acSelecionado = &ac1;
        powerSelecionado = &powerAC1;
        tempSelecionado = &tempAC1;
        nomeAc = "AC1";
    }
    else if (doc["arCondicionado2"].is<JsonObject>())
    {
        objetoJsonAC = doc["arCondicionado2"];
        acSelecionado = &ac2;
        powerSelecionado = &powerAC2;
        tempSelecionado = &tempAC2;
        nomeAc = "AC2";
    }
    else
    {
        publicarErroComHandshake("\n\rNenhum ar condicionado encontrado no JSON\n\r");
        return;
    }

    bool alterouEstado = false;

    //*========================================================================================

    //*                            ⇩ ⇩ ⇩      recebimento      ⇩ ⇩ ⇩

    //*========================================================================================

    if (objetoJsonAC["comando"].is<int>())
    {
        int comando = objetoJsonAC["comando"].as<int>();

        if (comando == AC_POWER)
        {
            *powerSelecionado = !(*powerSelecionado); // inverte o estado atual

            if (*powerSelecionado)
            {
                acSelecionado->setCmd(kFujitsuAcCmdTurnOn);
                acSelecionado->send();
                publicarSucessoComHandshake("Power: ON");
            }
            else
            {
                acSelecionado->setCmd(kFujitsuAcCmdTurnOff);
                acSelecionado->send();
                publicarSucessoComHandshake("Power: OFF");
            }

            alterouEstado = true;
        }

        else if (comando == AC_AUTO_MODE)
        {
            acSelecionado->setMode(kFujitsuAcModeAuto);
            acSelecionado->send();
            alterouEstado = true;
            publicarSucessoComHandshake("Modo: Auto");
        }
        else if (comando == AC_COOL_MODE)
        {
            acSelecionado->setMode(kFujitsuAcModeCool);
            acSelecionado->send();
            alterouEstado = true;
            publicarSucessoComHandshake("Modo: Cool");
        }
        else if (comando == AC_FAN_MODE)
        {
            acSelecionado->setMode(kFujitsuAcModeFan);
            acSelecionado->send();
            alterouEstado = true;
            publicarSucessoComHandshake("Modo: Fan");
        }
        else if (comando == AC_FAN_LOW)
        {
            acSelecionado->setFanSpeed(kFujitsuAcFanLow);
            acSelecionado->send();
            alterouEstado = true;
            publicarSucessoComHandshake("Fan: Low");
        }
        else if (comando == AC_FAN_MEDIUM)
        {
            acSelecionado->setFanSpeed(kFujitsuAcFanMed);
            acSelecionado->send();
            alterouEstado = true;
            publicarSucessoComHandshake("Fan: Med");
        }
        else if (comando == AC_FAN_HIGH)
        {
            acSelecionado->setFanSpeed(kFujitsuAcFanHigh);
            acSelecionado->send();
            alterouEstado = true;
            publicarSucessoComHandshake("Fan: High");
        }
        else if (comando == AC_FAN_QUIET)
        {
            acSelecionado->setFanSpeed(kFujitsuAcFanQuiet);
            acSelecionado->send();
            alterouEstado = true;
            publicarSucessoComHandshake("Fan: Quiet");
        }

        else if (comando == AC_TEMP_DOWN)
        {
            if (*tempSelecionado > 16)
            {
                (*tempSelecionado)--;
                acSelecionado->setTemp(*tempSelecionado);
                acSelecionado->send();
                alterouEstado = true;
                publicarSucessoComHandshake("Temp: + 1");
            }
            else
            {
                publicarErroComHandshake("Limite minimo de temperatura atingido: 16C");
            }
        }

        else if (comando == AC_TEMP_UP)
        {
            if (*tempSelecionado < 30)
            {
                (*tempSelecionado)++;
                acSelecionado->setTemp(*tempSelecionado);
                acSelecionado->send();
                alterouEstado = true;
                publicarSucessoComHandshake("Temp: - 1");
            }
            else
            {
                publicarErroComHandshake("Limite maximo de temperatura atingido: 30C");
            }
        }
    }

    //*========================================================================================

    //*                   ⇩ ⇩ ⇩      ENVIA STATUS DO AC      ⇩ ⇩ ⇩

    //*========================================================================================

    if (alterouEstado)
    {
        publicarSucessoComHandshake("Sucesso.");

        //* Status do AC  ⇩

        publicarMensagemNoTopico(0, acSelecionado->toString().c_str());
    }

    else
    {
        publicarErroComHandshake("\n\rFalha ao receber comando.");
    }
}

//*======================================================================================================================================

//* COMANDOS JSON  ⇩⇩

/**
 * * {
 * *    "arCondicionadoX": {
 * *       "comando": X
 * *    }
 * * }
 */

//* HANDSHAKE  ⇩⇩

/**
 * * {
 * *    "statusComando": {
 * *       "comando": X,
 * *       "situacao": X
 * *    }
 * * }
 */

//*======================================================================================================================================