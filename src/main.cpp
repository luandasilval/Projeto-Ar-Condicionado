#include <Arduino.h>
#include <ArduinoJson.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Fujitsu.h>
#include <Preferences.h>
 
#include "WiFiManager.h"
#include "MqttManager.h"
#include "DebugManager.h"
 
//*========================================================================================
//*                      ⇩ ⇩ ⇩      TABELA DE COMANDOS      ⇩ ⇩ ⇩
//*
//*   Formato do JSON:  {"arCondicionadoX": {"comando": N}}
//*
//*   Valor │ Ação
//*  ───────┼──────────────────────────────
//*     1   │ Power (toggle liga/desliga)
//*     2   │ Modo Auto
//*     3   │ Modo Cool
//*     4   │ Modo Fan
//*     5   │ Fan Auto
//*     6   │ Fan High
//*     7   │ Fan Med
//*     8   │ Fan Low
//*     9   │ Fan Quiet
//*    10   │ Temperatura +1°C
//*    11   │ Temperatura -1°C
//*
//*========================================================================================
 
enum Comando {
    CMD_POWER        = 1,
    CMD_MODO_AUTO    = 2,
    CMD_MODO_COOL    = 3,
    CMD_MODO_FAN     = 4,
    CMD_FAN_AUTO     = 5,
    CMD_FAN_HIGH     = 6,
    CMD_FAN_MED      = 7,
    CMD_FAN_LOW      = 8,
    CMD_FAN_QUIET    = 9,
    CMD_TEMP_SUBIR   = 10,
    CMD_TEMP_DESCER  = 11
};
 
//*========================================================================================
//*              ⇩ ⇩ ⇩      TÓPICO MQTT (lido da NVS)      ⇩ ⇩ ⇩
//*========================================================================================
 
Preferences prefs;
char TOPICO_COMANDO[64];
 
void carregarConfiguracoes()
{
    prefs.begin("config", false);
    String topico = prefs.getString("topico", "senai134/shared/projeto/ar_condicionado");
    topico.toCharArray(TOPICO_COMANDO, sizeof(TOPICO_COMANDO));
    prefs.end();
    debugInfo("Topico MQTT carregado: " + String(TOPICO_COMANDO));
}
 
void salvarTopico(const char* novoTopico)
{
    prefs.begin("config", false);
    prefs.putString("topico", novoTopico);
    prefs.end();
}
 
//*========================================================================================
//*              ⇩ ⇩ ⇩      PINOS, INSTÂNCIAS E ESTADO      ⇩ ⇩ ⇩
//*
//*  ⚠️  ESP32-S3: GPIOs 26 e 27 não existem neste chip.
//*     Ajuste os pinos conforme o hardware antes de compilar.
//*     Pinos seguros: 1-21, 33-48 (evite 0, 19, 20, 45, 46)
//*========================================================================================
 
const uint8_t PINOS_AC[4] = { 13, 16, 33, 34 };
 
IRFujitsuAC* acs[4];
 
int  tempDefault[4] = { 24, 24, 24, 24 };
bool powerEstado[4] = { false, false, false, false };
 
//*========================================================================================
//*                           ⇩ ⇩ ⇩      PROTÓTIPOS      ⇩ ⇩ ⇩
//*========================================================================================
 
void tratarMensagemRecebida(const char* topico, const String& mensagem);
void tratarJsonComando(const String& mensagem);
int  indiceAcDoJson(JsonDocument& doc, JsonObject& obj);
void publicarErro(const String& msg);
void executarComando(int idx, int comando);
 
//*========================================================================================
//*                             ⇩ ⇩ ⇩      SETUP      ⇩ ⇩ ⇩
//*========================================================================================
 
void setup()
{
    configurarDebug();
    carregarConfiguracoes();
 
    for (int i = 0; i < 4; i++)
    {
        acs[i] = new IRFujitsuAC(PINOS_AC[i]);
        acs[i]->begin();
        acs[i]->setModel(ARRAH2E);
    }
 
    conectarWiFi();
    configurarMQTT();
    registrarCallbackMensagem(tratarMensagemRecebida);
    conectarMQTT();
 
    debugInfo("Controle Fujitsu inicializado");
 
    for (int i = 0; i < 4; i++)
    {
        publicarMensagemNoTopico(1, acs[i]->toString().c_str());
    }
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
 
void tratarMensagemRecebida(const char* topico, const String& mensagem)
{
    debugInfo("========================================");
    debugInfo("Mensagem MQTT recebida");
    debugInfo("========================================");
 
    if (topico == nullptr)
    {
        publicarErro("Topico MQTT invalido");
        return;
    }
 
    debugInfo("Topico: " + String(topico));
    debugInfo("Payload: " + mensagem);
 
    if (strcmp(topico, TOPICO_COMANDO) == 0)
    {
        tratarJsonComando(mensagem);
        return;
    }
 
    publicarErro("Topico nao tratado: " + String(topico));
}
 
//*========================================================================================
//*                           ⇩ ⇩ ⇩      HELPERS      ⇩ ⇩ ⇩
//*========================================================================================
 
int indiceAcDoJson(JsonDocument& doc, JsonObject& obj)
{
    const char* chaves[4] = {
        "arCondicionado1",
        "arCondicionado2",
        "arCondicionado3",
        "arCondicionado4"
    };
 
    for (int i = 0; i < 4; i++)
    {
        if (doc[chaves[i]].is<JsonObject>())
        {
            obj = doc[chaves[i]];
            return i;
        }
    }
    return -1;
}
 
void publicarErro(const String& msg)
{
    debugErro(msg);
    publicarMensagemNoTopico(3, msg.c_str());
}
 
//*========================================================================================
//*                      ⇩ ⇩ ⇩      EXECUÇÃO DO COMANDO      ⇩ ⇩ ⇩
//*========================================================================================
 
void executarComando(int idx, int comando)
{
    IRFujitsuAC* ac  = acs[idx];
    String nomeAc    = "AC" + String(idx + 1);
    String descricao = "";
 
    switch (comando)
    {
        //* ---- POWER (toggle) -----------------------------------------------
        case CMD_POWER:
            powerEstado[idx] = !powerEstado[idx];
            if (powerEstado[idx])
            {
                ac->setCmd(kFujitsuAcCmdTurnOn);
                ac->setSwing(kFujitsuAcSwingOff);
                descricao = "Power: ON";
            }
            else
            {
                ac->setCmd(kFujitsuAcCmdTurnOff);
                descricao = "Power: OFF";
            }
            break;
 
        //* ---- MODOS -----------------------------------------------------------
        case CMD_MODO_AUTO:
            ac->setMode(kFujitsuAcModeAuto);
            descricao = "Modo: Auto";
            break;
 
        case CMD_MODO_COOL:
            ac->setMode(kFujitsuAcModeCool);
            descricao = "Modo: Cool";
            break;
 
        case CMD_MODO_FAN:
            ac->setMode(kFujitsuAcModeFan);
            descricao = "Modo: Fan";
            break;
 
        //* ---- VELOCIDADE DO FAN -----------------------------------------------
        case CMD_FAN_AUTO:
            ac->setFanSpeed(kFujitsuAcFanAuto);
            descricao = "Fan: Auto";
            break;
 
        case CMD_FAN_HIGH:
            ac->setFanSpeed(kFujitsuAcFanHigh);
            descricao = "Fan: High";
            break;
 
        case CMD_FAN_MED:
            ac->setFanSpeed(kFujitsuAcFanMed);
            descricao = "Fan: Med";
            break;
 
        case CMD_FAN_LOW:
            ac->setFanSpeed(kFujitsuAcFanLow);
            descricao = "Fan: Low";
            break;
 
        case CMD_FAN_QUIET:
            ac->setFanSpeed(kFujitsuAcFanQuiet);
            descricao = "Fan: Quiet";
            break;
 
        //* ---- TEMPERATURA +1°C ------------------------------------------------
        case CMD_TEMP_SUBIR:
            if (tempDefault[idx] < 30)
            {
                tempDefault[idx]++;
                ac->setTemp(tempDefault[idx]);
                descricao = "Temperatura: +" + String(tempDefault[idx]) + "C";
            }
            else
            {
                publicarErro(nomeAc + " | Limite maximo de temperatura atingido: 30C");
                return;
            }
            break;
 
        //* ---- TEMPERATURA -1°C ------------------------------------------------
        case CMD_TEMP_DESCER:
            if (tempDefault[idx] > 16)
            {
                tempDefault[idx]--;
                ac->setTemp(tempDefault[idx]);
                descricao = "Temperatura: -" + String(tempDefault[idx]) + "C";
            }
            else
            {
                publicarErro(nomeAc + " | Limite minimo de temperatura atingido: 16C");
                return;
            }
            break;
 
        //* ---- COMANDO DESCONHECIDO --------------------------------------------
        default:
            publicarErro("Comando invalido: " + String(comando) +
                         " | Use valores de 1 a 11");
            return;
    }
 
    //* Envia sinal IR uma unica vez, com todos os parametros ja configurados
    ac->send();
 
    debugInfo(nomeAc + " | " + descricao);
 
    JsonDocument resposta;
    resposta["handshake"]["situacao"] = true;
    String jsonResposta;
    serializeJson(resposta, jsonResposta);
 
    publicarMensagemNoTopico(4, jsonResposta.c_str());
    publicarMensagemNoTopico(1, ac->toString().c_str());
}
 
//*========================================================================================
//*                        ⇩ ⇩ ⇩      TRATAMENTO JSON      ⇩ ⇩ ⇩
//*========================================================================================
 
void tratarJsonComando(const String& mensagem)
{
    JsonDocument doc;
    DeserializationError erro = deserializeJson(doc, mensagem);
 
    if (erro)
    {
        publicarErro("Erro ao interpretar JSON: " + String(erro.c_str()));
        return;
    }
 
    JsonObject acObjetoJson;
    int idx = indiceAcDoJson(doc, acObjetoJson);
 
    if (idx == -1)
    {
        publicarErro("Nenhum ar condicionado encontrado no JSON");
        return;
    }
 
    if (!acObjetoJson["comando"].is<int>())
    {
        publicarErro("Campo \"comando\" ausente ou invalido. Envie um inteiro de 1 a 11");
        return;
    }
 
    int comando = acObjetoJson["comando"].as<int>();
 
    executarComando(idx, comando);
}