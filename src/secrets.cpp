
#include "secrets.h"
#include <Arduino.h>

//==============================
//            WiFi
//==============================

const char *WIFI_SSID = "SALA 09";

const char *WIFI_SENHA = "info@134";

// =============================
//            AWS
// =============================

const bool USAR_AWS_IOT = true;

const char *AWS_IOT_ENDPOINT = "a3cxr9ayydc4np-ats.iot.us-east-1.amazonaws.com";

const char AWS_CERTIFICADO_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

const char AWS_CERTIFICADO_CRT[] PROGMEM = R"CRT(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUWCyA6Aok9vtrn29FqYxTXJi+29gwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI2MDUyMDE5NTQx
N1oXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAL+7MxH1gep/nRpsi/yj
ZIkMBSnTk6K3DzLWy4gcUX/n7dHq+PU3KevxZtXTrWPVa5aCU0GwsYhjbE9b72zk
RCfP/HNmDUo+Q7/oytVcAn6zyYbxaAiqiiB4Z42okRZIB/VpDG1W1Jqfcm22b2n7
xg7Lr6pYxkql4zjCr4RQIXGdSsaziKkW3Cw8ypTM04eIGlkSNN39rSg+0thP/Bv2
sJ1VAOVD0vrgvhOd4knFqtMXJg1uvynCNVuWjainHzmB2G7QC0syG8GW0YjQzf+0
D/W7YuCV0rVvbLc0f2EIgYUMt84LTToK9gzBKeyMucyqOETlwgXCeQzx0x0yft88
Yj0CAwEAAaNgMF4wHwYDVR0jBBgwFoAU910vqAZsJA6wLbhYt8XLIAe5G88wHQYD
VR0OBBYEFJWdd84Q0RvKkCcmdYd3OGzD4UHHMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQAath3UG2pF/ClUpvFAXkQv276H
OGFVZk595/UlvPOdJda0v10c5cmRM87Yn6mto5xgY7BPBGQK4Txf9Wp8vmHqcGsZ
KlcDYyWU68wfWxWJ2aYz9bxUYWAFwevMWzPBU0w5DG+krFTXfLRojFgs15btgj7Z
WIrQsmBRhrOmk+zT9ZLGG4Z4mDMV6DS0OnBe2nKXLegOOd2qgS8k3NTZnb4o+Tra
Rq/nln0XT3kKrIJqcq4lw/F1oFOFEImu9Gho5j5c2Crqv95ZMQr9x/8A/hPd8S4R
HksSijj5Z9sbMQs5Bs7lD2wlLteftAjk2sUOCDvFy8kMxHqZ4SqzWlsQBcdS
-----END CERTIFICATE-----
)CRT";

const char AWS_CERTIFICADO_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
MIIEpAIBAAKCAQEAv7szEfWB6n+dGmyL/KNkiQwFKdOTorcPMtbLiBxRf+ft0er4
9Tcp6/Fm1dOtY9VrloJTQbCxiGNsT1vvbOREJ8/8c2YNSj5Dv+jK1VwCfrPJhvFo
CKqKIHhnjaiRFkgH9WkMbVbUmp9ybbZvafvGDsuvqljGSqXjOMKvhFAhcZ1KxrOI
qRbcLDzKlMzTh4gaWRI03f2tKD7S2E/8G/awnVUA5UPS+uC+E53iScWq0xcmDW6/
KcI1W5aNqKcfOYHYbtALSzIbwZbRiNDN/7QP9bti4JXStW9stzR/YQiBhQy3zgtN
Ogr2DMEp7Iy5zKo4ROXCBcJ5DPHTHTJ+3zxiPQIDAQABAoIBAQCk+WfVMSGLbCj7
HqFJTjJode5Pch0bqlYqkYXp5Sv/WcpZkMyM+t1YhovMu2n7eB0uEY+f+3CXAa2l
j16xnKxEcYMwrIQM+QyEqcM12QjQitSCB2BT7KITlGbODlhhCrPGZ5Bo2IjxTvdA
DfVKBXkDLCJH+kEM5DZilao+uFA+l/Kt/41QaIZvH4RbWCrji0BWtt3p+KnJluVz
e0tVkjdsEVirVs9xtUgPw3fRnh+vsbKG7OfCmwJD+4VKsCnij568UYGRFYC2vI9Q
xu9E3bYCUqouxea5DoluR07bU1vWubyYd2+bgLrply2AGEfsJn4Z9rwIghqljwyE
F4A5QZElAoGBAOc0YvfT3cr6WZSpLBHL5uGXgtm/9AE1LV8zDPXPMES1gbUQbY48
xsrko0BqU/X30cdfoLFRaGrrp5Wygb/Qq04/J2geaJxYL5nijAcR7O8c5qMF721i
BfHzEMT1Yw2hb1J+ga5RrlKns4PeNI5cDfEUSdrG17DjFt3+SWIjlpRvAoGBANRL
Fv3QWj6Gk5KN5MwVty96HzCfI/1WLleoCWg8c8aV3bmk9p12+V6MRvXxmCJIdXeg
X9lXPxIMCHniFTVp6WMtMUwmcLOTopQj9FwsY9FVXKYGjH6P+IrdadbWGh5WZjo7
dUzxbkOfg/xE0nqjWWJNukKwQDpe/V0hQMXkwYITAoGAKRl05l0xNh3ewsB35wn6
FuwaBJ9MvtI3KI8opKGaEPy6+/bZoslHBVYzdwXvuu+8yNZE+IBiMW+GsOO0kOaF
LTmoxadyLSi3/YD6J1+WYMMgdKZfc3BTYjk1J+3/F7M02NNINjaq6GKLVMuxSAUx
LUPQBSYHvZgAvqGvZdSxtd0CgYARKxFs7VCfBdshWwBJ0OfDmyiykewjLLkKtbgt
CdaDFNOdk82t5lU8ZDPI6ATZoOp/BiU6OmgLWZXF5Za5fxs0vg7ijalHnITlXhMA
8UYRQ8n+U0FBavs6n7VTfnUWRv51byGJW20DYv8Vg6tR56vu20JY6lAHaG8s/Od0
GY/cqQKBgQCXk2lomxGF/S8H1oOnVY4oci98z0zsXKOP5Fb8jCmMvkXOR/7gwFsL
SKFZaDT197F/rmfCjQ+yxeuQ+aufOdu5bnRow9tpBp9TaMe9ov1e5mgT5a/6QPDw
9aN0QU3j2PxDPoRtLqIfRJBpF1nKjndPfXx5VjiDQ5z/kzUT3WpH2w==
-----END RSA PRIVATE KEY-----
)KEY";

const int AWS_IOT_PORT = 8883;

const char *AWS_IOT_CLIENT_ID = "boo-esp32-ac1";

// =============================
//            TOPICOS
// =============================

const char *TOPICOS_PUBLICAR[] =
    {
        "senai134/equipe/boo/devices/ar_condicionado",
        "senai134/shared/projeto/ar_condicionado/status",
        "senai134/shared/projeto/ar_condicionado/log",
        "senai134/shared/projeto/ar_condicionado/resposta"
        "senai134/shared/projeto/ar_condicionado/handshake"};

const int TOTAL_TOPICOS_PUBLICAR = 5;

const char *TOPICOS_RECEBER[] =
    {
        "senai134/shared/projeto/ar_condicionado",
        "senai134/shared/projeto/ar_condicionado/ac_1"};

const int TOTAL_TOPICOS_RECEBER = 2;

// =============================
//            DEBUG
// =============================

// 0 = sem mensagens
// 1 = apenas erros
// 2 = todas as mensagens
const int DEBUG_NIVEL_INICIAL = 2;

// pino usado para forçar todas as mensagens
const int PINO_HABILITA_DEBUG_COMPLETO = 4;
