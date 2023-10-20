# Guia para Acesso Remoto de Dados por WiFi

Neste repositório, você encontrará um guia abrangente sobre como configurar e acessar dados de sensores de temperatura, umidade e luminosidade de forma remota, fazendo uso de um ESP32 e uma conexão WiFi. Este projeto é direcionado a entusiastas, estudantes e desenvolvedores que desejam coletar dados de sensores e acessá-los de qualquer lugar com acesso à Internet.

## Introdução

Nesta fase do projeto, nosso objetivo é desenvolver um protótipo para medir parâmetros como temperatura, umidade e luminosidade. Os componentes essenciais para essa empreitada incluem:

- ESP32
- Protoboard
- Resistor de 10kΩ
- Sensor de temperatura e umidade (DHT11 ou DHT22)
- Sensor de luminosidade (LDR)

Caso não tenha os componentes físicos em mãos, você pode simular o projeto no [site Wokwi](https://wokwi.com) para compreender e seguir o processo.

A montagem deve ficar assim: ![MONTAGEM](https://cdn.discordapp.com/attachments/1075818265739280436/1164754095786164245/image.png?ex=65445cc8&is=6531e7c8&hm=3f408fff7c9e7aa4f614a1fb4f5926390f7f60ab462d97422fbabab5ea1b59a4&)

## Explorando os Componentes

O ESP32 permite a conexão à Internet e o registro dos dados coletados pelos sensores na nuvem. Isso viabiliza o acesso remoto às informações coletadas e a capacidade de controlar o equipamento à distância. Com a configuração apropriada, você poderá ligar/desligar o LED do ESP32 e coletar dados dos sensores. Para realizar essas operações, você pode utilizar o aplicativo MyMQTT ou uma aplicação no Google Colab.

O protoboard é importante pois nela serão feitas as conexões entre o ESP32 e os sensores. O sensor DHT22 (ou DHT11) medirá a temperatura e a umidade, enquanto o sensor LDR medirá a luminosidade.

## Configuração do Código

Após montar o hardware conforme as instruções, é hora de trabalhar no código. Você pode iniciar com o código fornecido neste repositório como base.

- Personalize o valor "lamp108" para a porta que você deseja usar para acessar a rede.
- As siglas "L, U, T" no final das portas correspondem a Luminosidade, Umidade e Temperatura, respectivamente.
- Insira o nome e senha da sua rede WiFi nas linhas 41 e 42.

Após essas modificações, execute o código e aguarde a mensagem que indica que o ESP32 está tentando se conectar à rede.


## Acesso aos Dados

Para acessar os dados coletados, usaremos o Google Colab. Primeiro, instale a biblioteca Paho MQTT para se comunicar com o broker na porta 1883. Após a instalação, importe a biblioteca Paho MQTT e acrescente o código necessário.

Após seguir esses passos, o projeto estará completo. Agora você pode acessar os dados de temperatura, umidade e luminosidade de qualquer lugar com acesso à Internet.

Divirta-se!
