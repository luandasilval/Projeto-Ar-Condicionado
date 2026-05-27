//*main.cpp

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>

#include "WiFiManager.h"
#include "MqttManager.h"
#include "DebugManager.h"

//*=======constantes==========
const int PinoLedRGB = 48;
const int QntLeds = 1;
const char TOPICO_COMANDO[] = "senai134/Guilherme/esp32/status";

const int pinoLampada = 17;

//*=========instancias========
Adafruit_NeoPixel ledRGB(
    QntLeds, PinoLedRGB,
    NEO_GRB + NEO_KHZ800);

void tratarMensagemRecebida(const char *topico, const String &mensagem);
void configurarLedRGB();
void alterarCorDoLedRGB(int vermelho, int verde, int azul);

void tratarJsonComando(const String &mensagem);

void setup()
{
  configurarDebug();
  conectarWifi();
  configurarMQTT();
  registrarCallbackMensagem(tratarMensagemRecebida);
  conectarMQTT();
  configurarLedRGB();
}

void loop()
{
  garantirWiFiConectado();
  garantirMQTTConectado();
  loopMQTT();

  alterarCorDoLedRGB(255, 0, 0);
  delay(2000);
  alterarCorDoLedRGB(0, 255, 0);
  delay(2000);
  alterarCorDoLedRGB(0, 255, 0);
  delay(2000);
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

void configurarLedRGB()
{
  ledRGB.begin();
  ledRGB.setBrightness(80);
  ledRGB.clear();
  ledRGB.show();

  debugInfo("LED RGB configurado no pino" + String(PinoLedRGB));
}

void alterarCorDoLedRGB(int vermelho, int verde, int azul)
{
  vermelho = constrain(vermelho, 0, 255);
  verde = constrain(verde, 0, 255);
  azul = constrain(azul, 0, 255);

  ledRGB.setPixelColor(0, ledRGB.Color(vermelho, verde, azul));
  ledRGB.show();

  debugInfo("Cor aplicada");
  debugInfo("R: " + String(vermelho));
  debugInfo("R: " + String(verde));
  debugInfo("R: " + String(azul));
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

