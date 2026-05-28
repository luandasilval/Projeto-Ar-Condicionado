// WiFiManager.cpp
#include <Arduino.h>
#include "WiFiManager.h"
#include "secrets.h"
#include <WiFi.h>
#include "DebugManager.h"
#include <WiFiClientSecure.h>

void conectarWiFi()
{
  debugInfo("==============================");
  debugInfo("Iniciando conexão WiFi...");
  debugInfo("==============================");

  // Configura o ESP32 como station, ou seja
  // ele vai se conectar a um roteador existente
  WiFi.mode(WIFI_STA);

  WiFi.begin(WIFI_SSID, WIFI_SENHA);

  debugInfo("Conectando");

  int tentativas = 0;
  const int maxTentativa = 30;

  while (WiFi.status() != WL_CONNECTED && tentativas < maxTentativa)
  {
    debugInfoSemLinha(".");
    delay(500);
    tentativas++;
  }

  debugInfoSemLinha("\n\r");

  if (WiFi.status() == WL_CONNECTED)
  {
    debugInfo("WiFi conectado com sucesso");
    debugInfoSemLinha("Endereço IP:");
    debugInfoSemLinha(WiFi.localIP().toString());
    debugInfoSemLinha("\n\r");
  }

  else
  {
    debugErro("Falha ao conectar ao WiFi.");
    debugErro("Verifique o SSID, senha e sinal de rede.");
  }
}

void garantirWiFiConectado()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    debugInfo("WiFi desconectado. Tentando reconectar...");
    conectarWiFi();
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    debugErro("Não foi possível reconectar ao WiFi");
  }
}

bool wifiEstaConectado()
{
  return WiFi.status() == WL_CONNECTED;
}