//*main.cpp

#include <Arduino.h>
#include "WiFiManager.h"
#include "MqttManager.h"
#include "DebugManager.h"
#include <ArduinoJson.h>

//*=======constantes==========

const char TOPICO_COMANDO[] = "senai134/esp32/comando";

void tratarMensagemRecebida(const char *topico, const String &mensagem);
void tratarJsonComando(const String &mensagem);

void setup()
{
  configurarDebug();
  conectarWiFi();
  configurarMQTT();
  registrarCallbackMensagem(tratarMensagemRecebida);
  conectarMQTT();
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

  if (erro)
  {
    debugErro("Erro ao interpretar o JSON");
    debugErro(erro.c_str());
    return;
  }

  if (!doc["Led"].is<JsonObject>())
  {
    debugInfo("Não encontrado o comando para o led RGB");
  }
  else
  {
    if (!doc["led"]["r"].is<int>() ||
        !doc["led"]["g"].is<int>() ||
        !doc["led"]["b"].is<int>())
    {
      debugErro("Json Inválido. Use led.r, led.g e led.b");
    }
    else
    {
      int vermelho = doc["led"]["r"].as<int>();
      int verde = doc["led"]["r"].as<int>();
      int azul = doc["led"]["r"].as<int>();
    }
  }
}
