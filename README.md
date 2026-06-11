# 🌡️ Controle Inteligente de Ar-Condicionado via MQTT

## 📖 Descrição

Este projeto tem como objetivo automatizar o controle de aparelhos de ar-condicionado em ambientes escolares, permitindo o gerenciamento remoto dos equipamentos por meio da internet. A solução busca proporcionar maior conforto térmico para alunos e colaboradores, além de contribuir para a redução de custos operacionais.

---

## 🛠️ Tecnologias Utilizadas

- ESP32
- MQTT
- MQTTFX
- AWS
- JSON
- LED Infravermelho (IR)
- LED Indicador de Wi-Fi
- Biblioteca Fujitsu AC Remote

---

## ⚙️ Funcionamento

Os comandos são enviados pelo MQTTFX em formato JSON para um broker MQTT hospedado na AWS. O ESP32 recebe essas informações através da rede Wi-Fi, interpreta os comandos e utiliza um transmissor infravermelho para enviar as instruções ao ar-condicionado.

O LED indicador permanece aceso enquanto o ESP32 estiver conectado à rede Wi-Fi.

---

## 🔄 Fluxo do Sistema

```
MQTTFX
   ↓
 JSON
   ↓
 Broker MQTT (AWS)
   ↓
 ESP32
   ↓
 Infravermelho
   ↓
 Ar-Condicionado
```

---

## 🧠 Integração das Tecnologias

- **JSON:** organiza os dados dos comandos.
- **MQTT:** transporta as mensagens pela internet.
- **AWS:** hospeda o broker MQTT.
- **ESP32:** processa os comandos recebidos.
- **Infravermelho:** transmite os sinais para o aparelho.
- **Biblioteca Fujitsu:** gera os códigos compatíveis com o ar-condicionado.

Essa integração permite controlar o equipamento remotamente de qualquer local com acesso à internet.

---

## 👥 Autores

- Augusto Vicente Santos
- Diogo De Andrade Chelles
- Guilherme Fellippe Alves Silveira
- Luanda da Silva Leite
- Lucas de Oliveira Donega

---

## 📅 Informações do Projeto

**Versão:** 1.0

**Data de Entrega:** Junho de 2026