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

//* ********** CONSTANTES **********

const char TOPICO_COMANDO[] = "ArCondicionado/AC1/comando";

const uint8_t PINO_IR = 13;

const char *numeroDoAc = "A/C 1";

//* ********** INSTÂNCIAS **********

IRFujitsuAC ac(PINO_IR);

void tratarMensagemRecebida(const char *topico, const String &mensagem);
void tratarJsonComando(const String &mensagem);

void setup()
{
  configurarDebug(); //* FUNÇÕES "BEGIN" E "DELAY" JÁ INCLUSAS
  conectarWiFi();
  configurarMQTT();
  registrarCallbackMensagem(tratarMensagemRecebida);
  conectarMQTT();
  ac.begin();
  pinMode(13, INPUT_PULLUP);
}

void loop()
{
  garantirWiFiConectado();
  garantirMQTTconectado();
  loopMQTT();
}

void tratarMensagemRecebida(const char *topico, const String &mensagem)
{
  debugInfo("==============================");
  debugInfo("Mensagem recebida na aplicação");
  debugInfo("==============================");

  if (topico == nullptr)
  {
    debugErro("Tópico MQTT inválido");
    return;
  }

  debugInfo("Tõpico:" + String(topico));
  debugInfo("Tõpico:" + mensagem);

  if (strcmp(topico, TOPICO_COMANDO) == 0)
  {
    tratarJsonComando(mensagem);
    return;
  }

  debugErro("Tópico não tratado:" + String(topico));
}

void tratarJsonComando(const String &mensagem)
{
  JsonDocument doc;

  DeserializationError erro = deserializeJson(doc, mensagem);

  //* DEBUG "ERRO"  ⇩

  if (erro)
  {
    debugErro("Erro ao interpretar o JSON");
    debugErro(erro.c_str());
    return;
  }

  //* DEBUG VERIFICAÇÃO JSON OBJECT  ⇩

  if (!doc["ArCondicionado"].is<JsonObject>())
  {
    debugErro("Não encontrado comando para o Ar Condicionado.");
    return;
  }

  else //* escopo que será utilizado para tratar as mensagens no formato JSON
  {

    //*********************** ESTADOS "POWER" (HIGH & LOW) *****************************

    //* DEBUG ESTADOS "POWER"  ⇩

    if (!doc["ArCondicionado"]["power"].is<bool>())
    {
      debugErro("Comando Inválido.");
      debugErro("Utilize \"power\":\"true\" ou \"false\".");
      return;
    }

    //* FUNÇÕES "POWER"  ⇩

    else
    {
      bool estadoAC = doc["ArCondicionado"]["power"].as<bool>();

      if (estadoAC == true)
      {
        ac.setCmd(kFujitsuAcCmdTurnOn);
        ac.send();
        Serial.printf("%s: ON\n\r", numeroDoAc);
        return;
      }

      else if (estadoAC == false)
      {
        ac.setCmd(kFujitsuAcCmdTurnOff);
        ac.send();
        Serial.printf("%s: OFF\n\r", numeroDoAc);
        return;
      }
    }

    // TODO: if()...
  }
}