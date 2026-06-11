// MqttManager.cpp

#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <Arduino.h>
#include <PubSubClient.h>
#include <DebugManager.h>

#include "secrets.h"
#include "WiFiManager.h"
#include "MqttManager.h"

WiFiClient wifiCliente;
WiFiClientSecure wifiClientSecure;
PubSubClient mqttClient;

CallbackMensagemMQTT callbackDaAplicacao = nullptr;

void registrarCallbackMensagem(CallbackMensagemMQTT callback)
{
    callbackDaAplicacao = callback;

    if (callbackDaAplicacao != nullptr)
    {
        debugInfo("Callback da aplicação registrado com sucesso.");
    }

    else
    {
        debugErro("Callback da aplicação não foi registrado.");
    }
}

const char *obterTopicoPublicacao(int indiceTopico)
{
    if (indiceTopico < 0 || indiceTopico >= TOTAL_TOPICOS_PUBLICAR)
    {
        debugErro("Indice inválido para tópico de publicação: " + String(indiceTopico));
        return "";
    }
    return TOPICOS_PUBLICAR[indiceTopico];
}

const char *obterTopicoRecebimento(int indiceTopico)
{
    if (indiceTopico < 0 || indiceTopico >= TOTAL_TOPICOS_RECEBER)
    {
        debugErro("Indice inválido para tópico de recebimento: " + String(indiceTopico));
        return "";
    }
    return TOPICOS_RECEBER[indiceTopico];
}

void callbackInternoMQTT(char *topico, byte *payload, unsigned int tamanho)
{
    String mensagem = "";

    for (unsigned int i = 0; i < tamanho; i++)
    {
        mensagem += (char)payload[i];
    }

    debugInfo("=============================================================================");
    debugInfo("Mensagem MQTT recebida");
    debugInfo("=============================================================================");
    debugInfo("Tópico: " + String(topico));
    debugInfo("Mensagem: " + mensagem);

    if (callbackDaAplicacao != nullptr)
        callbackDaAplicacao(topico, mensagem);

    else
        debugErro("Mensagem foi recebida, mas nenhuma callback da aplicação foi registrado.");
}

void configurarMQTT()
{
    debugInfo("=============================================================================");
    debugInfo(" Configurando MQTT...");
    debugInfo("=============================================================================");

    if (USAR_AWS_IOT)
    {
        debugInfo("Modo selecionado: MQTT com TLS.");

        wifiClientSecure.setCACert(AWS_CERTIFICADO_CA);
        wifiClientSecure.setCertificate(AWS_CERTIFICADO_CRT);
        wifiClientSecure.setPrivateKey(AWS_CERTIFICADO_PRIVATE);

        mqttClient.setClient(wifiClientSecure);
        mqttClient.setServer(AWS_IOT_ENDPOINT, AWS_IOT_PORT);

        debugInfo("Endpoint AWS IoT: " + String(AWS_IOT_ENDPOINT));
        debugInfo("Porta AWS IoT: " + String(AWS_IOT_PORT));
    }
    else if (MQTT_TLS)
    {
        debugInfo("Modo selecionado: MQTT com TLS.");

        if (strlen(MQTT_CERTIFICADO_CA) > 100)
        {
            debugInfo("Certificado CA do broker MQTT configurado.");
            wifiClientSecure.setCACert(MQTT_CERTIFICADO_CA);
        }
        else
        {
            debugErro("Certificado não configurado. Usando setInsecure apenas para teste.");
            wifiClientSecure.setInsecure();
        }

        mqttClient.setClient(wifiClientSecure);
        mqttClient.setServer(MQTT_BROKER, MQTT_PORTA);

        debugInfo("Broker MQTT: " + String(MQTT_BROKER));
        debugInfo("Porta MQTT: " + String(MQTT_PORTA));
    }

    else // Conectar ao broker público sem certificado
    {
        debugInfo("Modo Selecionado: MQTT sem TLS.");

        mqttClient.setClient(wifiCliente);
        mqttClient.setServer(MQTT_BROKER, MQTT_PORTA);

        debugInfo("Broker MQTT: " + String(MQTT_BROKER));
        debugInfo("Porta MQTT: " + String(MQTT_PORTA));
    }

    mqttClient.setCallback(callbackInternoMQTT); //* Quando chegar mensagem, execute a função "callbackInternoMQTT"
    debugInfo("Callback interno no MQTT configurado");
}

void conectarMQTT()
{
    if (!wifiEstaConectado())
    {
        debugErro("MQTT não pode conectar porque o WiFi está desconectado");
        return;
    }

    debugInfo("====================================================================");
    debugInfo("Iniciando conexão MQTT...");
    debugInfo("====================================================================");

    int tentativasMQTT = 0;
    const int maxTentativasMQTT = 5;

    while (!mqttClient.connected() && tentativasMQTT < maxTentativasMQTT)
    {
        debugInfo("Tentando conectar ao broker MQTT. Tentativa: " + String(tentativasMQTT));
        bool conectado = false;

        if (USAR_AWS_IOT)
        {
            conectado = mqttClient.connect(AWS_IOT_CLIENT_ID);
        }

        if (conectado)
        {
            debugInfo("MQTT conectado com sucesso.");

            int totalTopicos = obterTotalTopicosRecebimento();

            debugInfo("Total de tópicos para inscrição: " + String(totalTopicos));

            for (int i = 0; i < totalTopicos; i++)
            {
                const char *topico = obterTopicoRecebimento(i);

                bool inscrito = mqttClient.subscribe(topico);

                if (inscrito)
                {
                    debugInfo("Inscrito no tópico: " + String(topico));
                }

                else
                {
                    debugErro("Falha ao se inscrever no tópico: " + String(topico));
                }
            }

            debugInfo(
                "ESP32 conectado ao MQTT");
        }

        else
        {
            debugErro("Falha ao conectar no MQTT. Código de erro: " + String(mqttClient.state()));
            tentativasMQTT++;
            delay(2000);
        }
    }

    if (!mqttClient.connected())
    {
        debugErro("Não foi possível conectar ao broker MQTT após " + String(maxTentativasMQTT) + " tentativas");
    }
}

int obterTotalTopicosRecebimento()
{
    return TOTAL_TOPICOS_RECEBER;
}

void garantirMQTTconectado()
{
    if (!wifiEstaConectado())
    {
        debugErro("MQTT não reconectado porque o WiFi está desconectado.");
        return;
    }

    if (!mqttClient.connected())
    {
        debugErro("MQTT desconectado. Tentando reconectar...");
        conectarMQTT();
    }
}

void loopMQTT()
{
    mqttClient.loop();
}

void publicarMensagem(const char *topico, const char *mensagem)
{
    if (!mqttClient.connected())
    {
        debugErro("Não foi possível publicar. MQTT desconectado.");
        return;
    }

    bool publicado = mqttClient.publish(topico, mensagem);

    if (publicado)
    {
        debugInfo("Mensagem publicada via MQTT.");
        debugInfo("Tópico: " + String(topico));
        debugInfo("Mensagem: " + String(mensagem));
    }

    else
    {
        debugErro("Falha ao publicar mensagem no tópico.");
    }
}

void publicarMensagemNoTopico(int indiceTopico, const char *mensagem)
{
    const char *topico = obterTopicoPublicacao(indiceTopico);

    if (strlen(topico) == 0)
    {
        debugErro("Não foi possível publicar. Indice de tópico inválido: " + String(indiceTopico));
        return;
    }

    publicarMensagem(topico, mensagem);
}

bool mqttEstaConectado()
{
    return mqttClient.connected();
}