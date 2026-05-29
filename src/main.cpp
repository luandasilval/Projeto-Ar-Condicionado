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

void modoFan();
//*=======constantes==========

const uint16_t pinLed = 18;

// Objeto Fujitsu
IRFujitsuAC ac(pinLed);

const char TOPICO_COMANDO[] = "senai134/esp32/comando";

const int boot = 0;
bool estadoAnteriorBoot = HIGH;
bool estadoAtualBoot = digitalRead(boot);

// void tratarMensagemRecebida(const char *topico, const String &mensagem);
// void tratarJsonComando(const String &mensagem);

void setup()
{
  configurarDebug();
  conectarWiFi();
  configurarMQTT();
  // registrarCallbackMensagem(tratarMensagemRecebida);
  conectarMQTT();

  pinMode(boot, INPUT_PULLUP);
}

void loop()
{
  garantirWiFiConectado();
  garantirMQTTconectado();
  loopMQTT();

  // Envia comando para o infravermelho
  ac.send();
  ac.begin();

  if (estadoAnteriorBoot == HIGH && estadoAtualBoot == 0)
  {
    ac.on();
    ac.send();
    estadoAnteriorBoot = estadoAtualBoot;
  }
}

void modoFan()
{
  // Temperatura
  ac.setTemp(23);

  switch (kFujitsuAcModeFan)
  {
  case 1: // FAN HIGH
    ac.setFanSpeed(kFujitsuAcFanHigh);
    Serial.println("Modo FAN HIGH enviado!");
    break;

  case 2: // FAN LOW
    ac.setFanSpeed(kFujitsuAcFanLow);
    Serial.println("Modo FAN LOW enviado!");
    break;

  case 3: // FAN QUIET
    ac.setFanSpeed(kFujitsuAcFanQuiet);
    Serial.println("Modo FAN QUIET enviado!");
    break;

  case 4: // FAN MEDIO
    ac.setFanSpeed(kFujitsuAcFanMed);
    Serial.println("Modo FAN MEDIO enviado!");
    break;

  case 5: // FAN AUTO
    ac.setFanSpeed(kFujitsuAcFanAuto);
    Serial.println("Modo FAN AUTO enviado!");
    break;
  }
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