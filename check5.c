//Autor: Fábio Henrique Cabrini
//Resumo: Esse programa possibilita ligar e desligar o led onboard, além de mandar o status para o Broker MQTT possibilitando o Helix saber
//se o led está ligado ou desligado.
//Revisões:
//Rev1: 26-08-2023 Código portado para o ESP32 e para realizar a leitura de luminosidade e publicar o valor em um tópico aprorpiado do broker 
//Autor Rev1: Lucas Demetrius Augusto 
//Rev2: 28-08-2023 Ajustes para o funcionamento no FIWARE Descomplicado
//Autor Rev2: Fábio Henrique Cabrini

#include <WiFi.h>
#include <PubSubClient.h> // Importa a Biblioteca PubSubClient
#include "DHT.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//defines:
//defines de id mqtt e tópicos para publicação e subscribe denominado TEF(Telemetria e Monitoramento de Equipamentos)
#define TOPICO_SUBSCRIBE    "/TEF/lamp111/cmd"        //tópico MQTT de escuta
#define TOPICO_PUBLISH      "/TEF/lamp111/attrs"      //tópico MQTT de envio de informações para Broker
#define TOPICO_PUBLISH_2    "/TEF/lamp111/attrs/l"    //tópico MQTT de envio de informações para Broker
#define TOPICO_PUBLISH_3    "/TEF/lamp111/attrs/h"    //tópico MQTT de envio de informações para Broker
#define TOPICO_PUBLISH_4    "/TEF/lamp111/attrs/t"    //tópico MQTT de envio de informações para Broker
                                                      //IMPORTANTE: recomendamos fortemente alterar os nomes
                                                      //            desses tópicos. Caso contrário, há grandes
                                                      //            chances de você controlar e monitorar o ESP32
                                                      //            de outra pessoa.
#define ID_MQTT  "fiware_111"   //id mqtt (para identificação de sessão)
                                 //IMPORTANTE: este deve ser único no broker (ou seja, 
                                 //            se um client MQTT tentar entrar com o mesmo 
                                 //            id de outro já conectado ao broker, o broker 
                                 //            irá fechar a conexão de um deles).
                                 // o valor "n" precisa ser único!
// DHT 11 (Sensor de umidade e temperatura)                                
#define DHTTYPE DHT11   // DHT 11
#define DHTPIN 4 
DHT dht(DHTPIN, DHTTYPE);

// Display LCD
LiquidCrystal_I2C lcd(0x27,16,2);
 
// WIFI
const char* SSID = "FIAP-IBM"; // SSID / nome da rede WI-FI que deseja se conectar
const char* PASSWORD = "Challenge@23!"; // Senha da rede WI-FI que deseja se conectar
  
// MQTT
const char* BROKER_MQTT = "46.17.108.113"; //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883; // Porta do Broker MQTT
 
int D4 = 2;
int D25 = 25;
int D26 = 26;
int D27 = 27;

//Variáveis e objetos globais
WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient
char EstadoSaida = '0';  //variável que armazena o estado atual da saída
  
//Prototypes
void initSerial();
void initWiFi();
void initMQTT();
void reconectWiFi(); 
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void VerificaConexoesWiFIEMQTT(void);
void InitOutput(void);
 
/* 
 *  Implementações das funções
 */
void setup() 
{
    //inicializações:
    InitOutput();
    initSerial();
    initWiFi();
    initMQTT();
    dht.begin();
    lcd.init();
    delay(5000);
    MQTT.publish(TOPICO_PUBLISH, "s|off");
    lcd.backlight();
}
  
//Função: inicializa comunicação serial com baudrate 115200 (para fins de monitorar no terminal serial 
//        o que está acontecendo.
//Parâmetros: nenhum
//Retorno: nenhum
void initSerial() 
{
    Serial.begin(115200);
}
 
//Função: inicializa e conecta-se na rede WI-FI desejada
//Parâmetros: nenhum
//Retorno: nenhum
void initWiFi() 
{
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
     
    reconectWiFi();
}
  
//Função: inicializa parâmetros de conexão MQTT(endereço do 
//        broker, porta e seta função de callback)
//Parâmetros: nenhum
//Retorno: nenhum
void initMQTT() 
{
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);   //informa qual broker e porta deve ser conectado
    MQTT.setCallback(mqtt_callback);            //atribui função de callback (função chamada quando qualquer informação de um dos tópicos subescritos chega)
}
  
//Função: função de callback 
//        esta função é chamada toda vez que uma informação de 
//        um dos tópicos subescritos chega)
//Parâmetros: nenhum
//Retorno: nenhum
void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
    String msg;
     
    //obtem a string do payload recebido
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }
    
    Serial.print("- Mensagem recebida: ");
    Serial.println(msg);
    
    //toma ação dependendo da string recebida:
    //verifica se deve colocar nivel alto de tensão na saída D0:
    //IMPORTANTE: o Led já contido na placa é acionado com lógica invertida (ou seja,
    //enviar HIGH para o output faz o Led apagar / enviar LOW faz o Led acender)
    if (msg.equals("lamp111@on|"))
    {
        digitalWrite(D4, HIGH);
        EstadoSaida = '1';
    }
 
    //verifica se deve colocar nivel alto de tensão na saída D0:
    if (msg.equals("lamp111@off|"))
    {
        digitalWrite(D4, LOW);
        EstadoSaida = '0';
    }
}
  
//Função: reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
//        em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
//Parâmetros: nenhum
//Retorno: nenhum
void reconnectMQTT() 
{
    while (!MQTT.connected()) 
    {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) 
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE); 
        } 
        else
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");
            delay(2000);
        }
    }
}
  
//Função: reconecta-se ao WiFi
//Parâmetros: nenhum
//Retorno: nenhum
void reconectWiFi() 
{
    //se já está conectado a rede WI-FI, nada é feito. 
    //Caso contrário, são efetuadas tentativas de conexão
    if (WiFi.status() == WL_CONNECTED)
        return;
         
    WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI
     
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }
   
    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}
 
//Função: verifica o estado das conexões WiFI e ao broker MQTT. 
//        Em caso de desconexão (qualquer uma das duas), a conexão
//        é refeita.
//Parâmetros: nenhum
//Retorno: nenhum
void VerificaConexoesWiFIEMQTT(void)
{
    if (!MQTT.connected()) 
        reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita
     
     reconectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}
 
//Função: envia ao Broker o estado atual do output 
//Parâmetros: nenhum
//Retorno: nenhum
void EnviaEstadoOutputMQTT(void)
{
    if (EstadoSaida == '1')
    {
      MQTT.publish(TOPICO_PUBLISH, "s|on");
      Serial.println("- Led Ligado");
    }
    if (EstadoSaida == '0')
    {
      MQTT.publish(TOPICO_PUBLISH, "s|off");
      Serial.println("- Led Desligado");
    }
    Serial.println("- Estado do LED onboard enviado ao broker!");
    delay(1000);
}
 
//Função: inicializa o output em nível lógico baixo
//Parâmetros: nenhum
//Retorno: nenhum
void InitOutput(void)
{
    pinMode(D25, OUTPUT);
    pinMode(D26, OUTPUT);
    pinMode(D27, OUTPUT);
    digitalWrite(D25, LOW);
    digitalWrite(D26, LOW);
    digitalWrite(D27, LOW);
    
    //IMPORTANTE: o Led já contido na placa é acionado com lógica invertida (ou seja,
    //enviar HIGH para o output faz o Led apagar / enviar LOW faz o Led acender)
    pinMode(D4, OUTPUT);
    digitalWrite(D4, HIGH);
    
    boolean toggle = false;

    for(int i = 0; i <= 10; i++)
    {
        toggle = !toggle;
        digitalWrite(D4,toggle);
        digitalWrite(D25,toggle);
        digitalWrite(D26,toggle);
        digitalWrite(D27,toggle);
        delay(200);           
    }

    digitalWrite(D4, LOW);
    digitalWrite(D25, LOW);
    digitalWrite(D26, LOW);
    digitalWrite(D27, LOW);
}
 
 
//programa principal
void loop() 
{   
    const int potPin = 34;
    
    char msgBuffer[4];
    //garante funcionamento das conexões WiFi e ao broker MQTT
    VerificaConexoesWiFIEMQTT();
 
    //envia o status de todos os outputs para o Broker no protocolo esperado
    EnviaEstadoOutputMQTT();

    //luminosidade
    int sensorValue = analogRead(potPin);   // Ler o pino Analógico onde está o LDR, lembrando que o divisor de tensão é Vin = Vout (R2/(R1 + R2))
    float voltage = sensorValue * (3.3 / 4096.0);   // Converter a leitura analógica (que vai de 0 - 4095) para uma voltagem (0 - 3.3V), quanto de acordo com a intensidade de luz no LDR a voltagem diminui.
    float luminosity = map(sensorValue, 2000, 4095, 0, 100); // Normalizar o valor da luminosidade entre 0% e 100%
    Serial.print("Voltage: ");
    Serial.print(voltage);
    Serial.print("V - ");
    Serial.print("Luminosidade: ");
    Serial.print(luminosity);
    Serial.println("%");

    //Publicação tópico MQTT Luminosidade
    dtostrf(luminosity, 4, 2, msgBuffer);
    MQTT.publish(TOPICO_PUBLISH_2,msgBuffer);

    //Escrita no display do valor da Luminosidade
    lcd.setCursor(0,0);
    lcd.print("L:");
    lcd.setCursor(2,0);
    lcd.print(msgBuffer);
    lcd.setCursor(6,0);
    lcd.print("%");

    // Controle dos Leds indicadores de luminosidade
    if(luminosity >= 80)
    {
      digitalWrite(D25, LOW);
      digitalWrite(D26, LOW);
      digitalWrite(D27, HIGH);
    }
    else if(luminosity < 80 && luminosity > 60)
    {
      digitalWrite(D25, LOW);
      digitalWrite(D26, HIGH);
      digitalWrite(D27, LOW);
    }
    else if(luminosity <= 60)
    {
      digitalWrite(D25, HIGH);
      digitalWrite(D26, LOW);
      digitalWrite(D27, LOW);
    }
 
    // Leitura do sensor DHT11
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    // Check if any reads failed and exit early (to try again).
    if (isnan(humidity) || isnan(temperature))
    {
      Serial.println(F("Falha leitura do sensor DHT-11!"));
    }

    Serial.print(F("Umidade: "));
    Serial.print(humidity);
    Serial.print(F("% Temperatura: "));
    Serial.print(temperature);
    Serial.println(F("°C "));
    
    //Publicação tópico MQTT Umidade
    dtostrf(humidity, 4, 2, msgBuffer);
    MQTT.publish(TOPICO_PUBLISH_3,msgBuffer);
    
    //Escrita no display do valor da Umidade
    lcd.setCursor(7,0);
    lcd.print(" U:");
    lcd.setCursor(10,0);
    lcd.print(msgBuffer);
    lcd.setCursor(14,0);
    lcd.print("% ");

    //Publicação tópico MQTT Temperatura
    dtostrf(temperature, 4, 2, msgBuffer);
    MQTT.publish(TOPICO_PUBLISH_4,msgBuffer);
    
    //Escrita no display do valor da Temperatura
    lcd.setCursor(0,1);
    lcd.print("T:");
    lcd.setCursor(2,1);
    lcd.print(msgBuffer);
    lcd.setCursor(5,1);
    lcd.print("C           ");
    
    //keep-alive da comunicação com broker MQTT
    MQTT.loop();
}
