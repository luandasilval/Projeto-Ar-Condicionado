#ifndef SECRETS_H
#define SECRETS_H

// Configurações do WiFi
extern const char *WIFI_SSID;
extern const char *WIFI_SENHA;

extern const char *TOPICOS_PUBLICAR[];

extern const int TOTAL_TOPICOS_PUBLICAR;

extern const char *TOPICOS_RECEBER[];

extern const int TOTAL_TOPICOS_RECEBER;

// =================================================
// DEBUG
// =================================================

// 0 = sem mensagens
// 1 = apenas erros
// 2 = todas as mensagens
extern const int DEBUG_NIVEL_INICIAL;

// Pino usado para forçar todas as mensagens
extern const int PINO_HABILITA_DEBUG_COMPLETO;

// =================================================
// AWS
// =================================================

extern const bool USAR_AWS_IOT;

extern const char AWS_CERTIFICADO_CA[];

extern const char AWS_CERTIFICADO_CRT[];

extern const char AWS_CERTIFICADO_PRIVATE[];

extern const char *AWS_IOT_ENDPOINT;

extern const int AWS_IOT_PORT;

extern const char *AWS_IOT_CLIENT_ID;

#endif