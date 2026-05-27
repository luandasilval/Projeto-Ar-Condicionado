#include <Arduino.h>
#include <WiFiManager.h>
#include "secrets.h"
#include <WiFi.h>

void conectarWifi()
{
  Serial.println("==================");
  Serial.println("iniciando conexao WiFi...");
  Serial.println("==================");

  // Configura o ESP32 como station, ou seja
  // ele vai se conectar a um roteador existente.

  WiFi.mode(WIFI_STA);

  WiFi.begin(WIFI_SSID, WIFI_SENHA);

  Serial.print("conectando");

  int tentativas = 0;
  const int maxTentativa = 30;

  while (WiFi.status() != WL_CONNECTED && tentativas < maxTentativa)
  {
    Serial.print(".");
    delay(500);
    tentativas++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED)

  {
    Serial.println("WiFi conectado com sucesso!");
    Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP());
  }

  else
  {
    Serial.println("Falha ao conectar ao WiFi.");
    Serial.println("Verifique o SSID, senha e sinal de rede.");
  }
}

void garantirWiFiConectado()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi desconectado. Tentando reconectar...");
    conectarWifi();
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Não foi possivel reconectar ao WiFi.");
  }
}

bool wifiEstaConectado()
{
    return WiFi.status() == WL_CONNECTED;
}