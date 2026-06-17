# 🌡️ Controle Inteligente de Ar-Condicionado via MQTT

## 📖 Descrição

Este projeto tem como objetivo automatizar o controle de aparelhos de ar-condicionado em ambientes escolares, permitindo o gerenciamento remoto dos equipamentos via internet. A solução propõe extinguir operações manuais como o deslocamento e manuseio do controle do aparelho (ar-condicionado) por parte do usuário.

---

## 🛠️ Tecnologias Utilizadas

- ESP32-S3 DevkitC-1 N16R8
- Protoboard (Matriz de Contatos)
- Cabo Jumper (também conhecido como cabo "Dupont")
- Resistores de 33Ω (Ohms)
- Resistor de 330Ω (Ohms)
- LED Infravermelho (TSAL4400)
- LED DIP
- Visual Studio Code (Editor de texto)
- PlatformIO (IDE)
- JSON (Formato de texto)
- Linguagens de programação "C", "C++"
- MQTT (Protocolo)
- AWS IoT Core
- Biblioteca "IRremoteESP8266"
- Biblioteca "ArduinoJson"
- Biblioteca "PubSubClient"

---

## ⚙️ Funcionamento

Os comandos são enviados por um dispositivo - cadastrado no broker como "publisher" - em formato JSON para um broker MQTT hospedado na AWS. O ESP32 recebe essas informações através da rede Wi-Fi, interpreta os comandos e utiliza um transmissor infravermelho para enviar comandos ao ar-condicionado.

O LED DIP permanece aceso enquanto o ESP32 estiver conectado ao Broker MQTT.

---

## 🔄 Fluxo do Sistema

```
 Publisher
   ↓
 Broker MQTT (AWS)
   ↓
 ESP32 (Subscriber)
   ↓
 LED Infravermelho (envia sinal infravermelho)
   ↓
 Ar-Condicionado (recebe sinal infravermelho)
```

---

## 🧠 Integração das Tecnologias

- **JSON:** organiza comandos em forma de texto.
- **MQTT:** transporta as mensagens pela internet.
- **AWS:** hospeda o broker MQTT.
- **ESP32:** processa os comandos recebidos.
- **Infravermelho:** transmite os sinais para o aparelho.
- **Biblioteca IRremoteESP8266:** fornece métodos suportados pelo ar-condicionado utilizado (Fujitsu).

Essa integração permite controlar o equipamento remotamente, de qualquer local em que haja acesso à internet.

---

## 👥 Autores

- Augusto Vicente Santos
- Diogo de Andrade Chelles
- Guilherme Fellippe Alves Silveira
- Luanda da Silva Leite
- Lucas de Oliveira Donega

---

## 📅 Informações do Projeto

**Versão:** 1.0

**Data de Entrega:** Junho de 2026