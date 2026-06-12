/*
  Autores:

  Augusto Vicente Santos
  Diogo de Andrade Chelles
  Guilherme Fellipe Alves Silveira
  Luanda da Silva Leite
  Lucas de Oliveira Donega

  Data de entrega do projeto: 5 de junho de 2026
  Versão: 1.0

* Programa: Controle do ar Condicionado Via MQTT
* Descrição: Controlaremos um ar condicionado via mqtt com um ESP32 e um infravermelho com LED, aplicando os modos FAN, AUTO e COOL, ligando e desligando o ar Condicionado. Quando um Json for enviado para o Nosso ESP32, esse microcontrolador vai ativar o infravermelho, enviando o sinal solicitado (pelo usuário) para o ar condicionado
*/

#include <Arduino.h>
#include <ArduinoJson.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Fujitsu.h>
#include <time.h>

#include "WiFiManager.h"
#include "MqttManager.h"
#include "DebugManager.h"

//*========================================================================================
//*                      ⇩ ⇩ ⇩      CONSTANTES E VARIÁVEIS      ⇩ ⇩ ⇩
//*========================================================================================

const char TOPICO_COMANDO[] = "senai134/shared/projeto/ar_condicionado";

const uint8_t PINO_AC1 = 13;
const uint8_t PINO_AC2 = 16;

const uint8_t AC_POWER_OFF = 0;
const uint8_t AC_POWER_ON = 1;
const uint8_t AC_AUTO_MODE = 2;
const uint8_t AC_COOL_MODE = 3;
const uint8_t AC_FAN_MODE = 4;
const uint8_t AC_FAN_LOW = 5;
const uint8_t AC_FAN_MEDIUM = 6;
const uint8_t AC_FAN_HIGH = 7;
const uint8_t AC_FAN_QUIET = 8;
const uint8_t AC_TEMP_DOWN = 9;
const uint8_t AC_TEMP_UP = 10;

int tempAC1 = 24;
int tempAC2 = 24;

const long FUSO_HORARIO_BRASILIA = -3 * 3600;

//*========================================================================================
//*                           ⇩ ⇩ ⇩      INSTÂNCIAS      ⇩ ⇩ ⇩
//*========================================================================================

IRFujitsuAC ac1(PINO_AC1);
IRFujitsuAC ac2(PINO_AC2);

//*========================================================================================
//*                           ⇩ ⇩ ⇩      PROTÓTIPOS      ⇩ ⇩ ⇩
//*========================================================================================

void tratarMensagemRecebida(const char *topico, const String &mensagem);

void tratarJsonComando(const String &mensagem);

void publicarErroComHandshake(const String &mensagem);

void publicarSucessoComHandshakeStatus(const String &nomeAc, const String &comandoAc, const String &mensagem, const String &statusAc);

String obterTimestamp();

//*========================================================================================
//*                             ⇩ ⇩ ⇩      SETUP      ⇩ ⇩ ⇩
//*========================================================================================

void setup()
{
    ac1.begin();
    ac2.begin();

    configurarDebug();

    conectarWiFi();

    // Sincronização de horário via NTP  ⇩⇩
    configTime(FUSO_HORARIO_BRASILIA, 0, "pool.ntp.org", "time.nist.gov");
    debugInfo("Aguardando sincronizacao NTP...");
    while (time(nullptr) < 1000000000UL)
    {
        delay(100);
    }
    debugInfo("NTP sincronizado.");

    configurarMQTT();

    registrarCallbackMensagem(tratarMensagemRecebida);

    conectarMQTT();

    ac1.setModel(ARRAH2E);
    ac2.setModel(ARRAH2E);
}

//*========================================================================================
//*                             ⇩ ⇩ ⇩      LOOP      ⇩ ⇩ ⇩
//*========================================================================================

void loop()
{
    garantirWiFiConectado();

    garantirMQTTconectado();

    loopMQTT();
}

//*========================================================================================

//*                      ⇩ ⇩ ⇩      TIMESTAMP      ⇩ ⇩ ⇩

//*========================================================================================

String obterTimestamp()
{
    time_t agora = time(nullptr);
    struct tm *timeinfo = localtime(&agora);

    char buffer[25];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

    return String(buffer);
}

//*========================================================================================

//*                          ⇩ ⇩ ⇩      HANDSHAKE     ⇩ ⇩ ⇩

//*========================================================================================

void publicarErroComHandshake(const String &mensagem)
{
    JsonDocument docHandshake;

    docHandshake["statusComando"]["timestamp"] = obterTimestamp();
    docHandshake["statusComando"]["situacao"] = mensagem;

    String handshakeFormatado;
    serializeJsonPretty(docHandshake, handshakeFormatado);

    publicarMensagemNoTopico(0, handshakeFormatado.c_str());
}

void publicarSucessoComHandshakeStatus(const String &nomeAc, const String &comandoAc, const String &mensagem, const String &statusAc)
{
    JsonDocument docHandshake;

    docHandshake["statusComando"]["timestamp"] = obterTimestamp();
    docHandshake["statusComando"]["dispositivo"] = nomeAc;
    docHandshake["statusComando"]["comando"] = comandoAc;
    docHandshake["statusComando"]["situacao"] = mensagem;
    docHandshake["statusComando"]["estado"] = "Status do controle " + nomeAc + ": " + statusAc;

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
        publicarErroComHandshake("JSON invalido. Erro na deserializacao.");
        return;
    }

    JsonObject objetoJsonAC;

    IRFujitsuAC *acSelecionado = nullptr;

    int *tempSelecionado = nullptr;

    String nomeAc;

    if (doc["arCondicionado1"].is<JsonObject>())
    {
        objetoJsonAC = doc["arCondicionado1"];
        acSelecionado = &ac1;
        tempSelecionado = &tempAC1;
        nomeAc = "AC1";
    }
    else if (doc["arCondicionado2"].is<JsonObject>())
    {
        objetoJsonAC = doc["arCondicionado2"];
        acSelecionado = &ac2;
        tempSelecionado = &tempAC2;
        nomeAc = "AC2";
    }
    else
    {
        publicarErroComHandshake("Nenhum ar condicionado encontrado no JSON. Utilize: \"arCondicionado1\" ou \"arCondicionado2\"");
        return;
    }

    //*========================================================================================

    //*                        ⇩ ⇩ ⇩      JSON RECEBIMENTO      ⇩ ⇩ ⇩

    //*========================================================================================

    if (objetoJsonAC["comando"].is<int>())
    {
        int comando = objetoJsonAC["comando"].as<int>();

        String nomeComando;

        if (comando == AC_POWER_OFF)
        {
            nomeComando = "AC_POWER_OFF";

            acSelecionado->setCmd(kFujitsuAcCmdTurnOff);
            acSelecionado->send();

            String status = acSelecionado->toString();

            publicarSucessoComHandshakeStatus(nomeAc, nomeComando, "Power: Off", status.c_str());

            return;
        }
        else if (comando == AC_POWER_ON)
        {
            nomeComando = "AC_POWER_ON";

            acSelecionado->setSwing(kFujitsuAcSwingOff);
            acSelecionado->setMode(kFujitsuAcModeCool);
            acSelecionado->setFanSpeed(kFujitsuAcFanHigh);
            acSelecionado->setTemp(*tempSelecionado);
            acSelecionado->setCmd(kFujitsuAcCmdTurnOn);
            acSelecionado->send();

            String status = acSelecionado->toString();

            publicarSucessoComHandshakeStatus(nomeAc, nomeComando, "Power: On", status.c_str());

            return;
        }
        else if (comando == AC_AUTO_MODE)
        {
            nomeComando = "AC_AUTO_MODE";

            acSelecionado->setMode(kFujitsuAcModeAuto);
            acSelecionado->send();

            String status = acSelecionado->toString();

            publicarSucessoComHandshakeStatus(nomeAc, nomeComando, "Modo: Auto", status.c_str());

            return;
        }
        else if (comando == AC_COOL_MODE)
        {
            nomeComando = "AC_COOL_MODE";

            acSelecionado->setMode(kFujitsuAcModeCool);
            acSelecionado->send();

            String status = acSelecionado->toString();

            publicarSucessoComHandshakeStatus(nomeAc, nomeComando, "Modo: Cool", status.c_str());

            return;
        }
        else if (comando == AC_FAN_MODE)
        {
            nomeComando = "AC_FAN_MODE";

            acSelecionado->setMode(kFujitsuAcModeFan);
            acSelecionado->send();

            String status = acSelecionado->toString();

            publicarSucessoComHandshakeStatus(nomeAc, nomeComando, "Modo: Fan", status.c_str());

            return;
        }
        else if (comando == AC_FAN_LOW)
        {
            nomeComando = "AC_FAN_LOW";

            acSelecionado->setFanSpeed(kFujitsuAcFanLow);
            acSelecionado->send();

            String status = acSelecionado->toString();

            publicarSucessoComHandshakeStatus(nomeAc, nomeComando, "Velocidade Fan: Low", status.c_str());

            return;
        }
        else if (comando == AC_FAN_MEDIUM)
        {
            nomeComando = "AC_FAN_MEDIUM";

            acSelecionado->setFanSpeed(kFujitsuAcFanMed);
            acSelecionado->send();

            String status = acSelecionado->toString();

            publicarSucessoComHandshakeStatus(nomeAc, nomeComando, "Velocidade Fan: Med", status.c_str());

            return;
        }
        else if (comando == AC_FAN_HIGH)
        {
            nomeComando = "AC_FAN_HIGH";

            acSelecionado->setFanSpeed(kFujitsuAcFanHigh);
            acSelecionado->send();

            String status = acSelecionado->toString();

            publicarSucessoComHandshakeStatus(nomeAc, nomeComando, "Velocidade Fan: High", status.c_str());

            return;
        }
        else if (comando == AC_FAN_QUIET)
        {
            nomeComando = "AC_FAN_QUIET";

            acSelecionado->setFanSpeed(kFujitsuAcFanQuiet);
            acSelecionado->send();

            String status = acSelecionado->toString();

            publicarSucessoComHandshakeStatus(nomeAc, nomeComando, "Velocidade Fan: Quiet", status.c_str());

            return;
        }
        else if (comando == AC_TEMP_DOWN)
        {
            nomeComando = "AC_TEMP_DOWN";

            if (*tempSelecionado > 16)
            {
                (*tempSelecionado)--;
                acSelecionado->setTemp(*tempSelecionado);
                acSelecionado->send();

                String status = acSelecionado->toString();

                publicarSucessoComHandshakeStatus(nomeAc, nomeComando, "Temp: - 1 C", status.c_str());

                return;
            }
            else
            {
                publicarErroComHandshake("Limite de temperatura minima atingido: 16 C");
                return;
            }
        }
        else if (comando == AC_TEMP_UP)
        {
            nomeComando = "AC_TEMP_UP";

            if (*tempSelecionado < 30)
            {
                (*tempSelecionado)++;
                acSelecionado->setTemp(*tempSelecionado);
                acSelecionado->send();

                String status = acSelecionado->toString();

                publicarSucessoComHandshakeStatus(nomeAc, nomeComando, "Temp: + 1 C", status.c_str());

                return;
            }
            else
            {
                publicarErroComHandshake("Limite de temperatura maxima atingido: 30 C");
                return;
            }
        }

        publicarErroComHandshake("Comando invalido: " + String(comando));
    }
    else
    {
        publicarErroComHandshake("Campo \"comando\" ausente ou invalido.");
        return;
    }
}

//*======================================================================================================================================

//* EXEMPLO DE COMANDO JSON  ⇩⇩

//* {"arCondicionado1":{"comando":1}}