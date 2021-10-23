#include <DHT.h>
#include <WiFi.h>
#include <PubSubClient.h>

/* Definicoes do sensor DHT11 */
#define DHTPIN 26     /*GPIO que está ligado o pino de dados do sensor*/
#define DHTTYPE DHT11
/* Defines do MQTT */

#define TOPICO_PUBLISH_TEMPERATURA "0f101c78-e114-11eb-ba80-0242ac130004/casa/sala/sensor/temperatura/leitura/"
#define TOPICO_PUBLISH_UMIDADE "0f101c78-e114-11eb-ba80-0242ac130004/casa/sala/sensor/umidade/leitura/"

#define ID_MQTT "mqttesp_1cffc0de" /*id mqtt (para identificação de sessão)*/
#define RETAIN true

/* Variaveis, constantes e objetos globais */
DHT dht(DHTPIN, DHTTYPE);

const char* SSID = "SSID-WiFi"; /* SSID / nome da rede WI-FI que deseja se conectar*/
const char* PASSWORD = "Senha-WiFi"; /* Senha da rede WI-FI que deseja se conectar*/
 
const char* BROKER_MQTT = "broker.emqx.io"; /*URL do broker MQTT que se deseja utilizar*/
int BROKER_PORT = 1883; // Porta do Broker MQTT
const char* BROKER_USER = "emqx_test";
const char* BROKER_PASSWORD = "emqx_test";

/*Variáveis e objetos globais*/
WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient
 
/* Prototypes */
float faz_leitura_temperatura(void);
float faz_leitura_umidade(void);
void initWiFi(void);
void initMQTT(void);
void reconnectMQTT(void);
void reconnectWiFi(void);
void VerificaConexoesWiFIEMQTT(void);

/*Implementações*/

/* Função: faz a leitura de temperatura (sensor DHT11)*/

float faz_leitura_temperatura(void)
{
    float t = dht.readTemperature();
    float result;
     
    if (! (isnan(t)) )
        result = t;
    else
        result = -99.99;
 
    return result;
}

/* Função: faz a leitura de umidade relativa do ar (sensor DHT11)*/

float faz_leitura_umidade(void)
{
    float h = dht.readHumidity();    
    float result;
     
    if (! (isnan(h)) )
        result = h;
    else
        result = -99.99;
 
    return result;
}

/* Função: inicializa e conecta-se na rede WI-FI desejada */

void initWiFi(void) 
{
    delay(10); 
    reconnectWiFi();
}
 
/* Função: inicializa parâmetros de conexão MQTT(endereço do broker, porta e seta função de callback)*/

void initMQTT(void) 
{
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);   /*informa qual broker e porta deve ser conectado*/
}
 

/* Função: reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair) caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito. */

void reconnectMQTT(void) 
{
    while (!MQTT.connected()) 
    {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT,BROKER_USER,BROKER_PASSWORD)) 
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
        } 
        else
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentativa de conexao em 2s");
            delay(2000);
        }
    }
}
 
/* Função: verifica o estado das conexões WiFI e ao broker MQTT*/
void VerificaConexoesWiFIEMQTT(void)
{
    if (!MQTT.connected()) 
        reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita
      
     reconnectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}
 
/* Função: reconecta-se ao WiFi */

void reconnectWiFi(void) 
{
    /*se já está conectado a rede WI-FI, nada é feito. */
    /*Caso contrário, são efetuadas tentativas de conexão*/
    if (WiFi.status() == WL_CONNECTED)
        return;
          
    WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI
      
  }

/* Função de setup */
void setup() 
{
  Serial.begin(115200); 
 
    /* Inicializacao do sensor de temperatura */
    dht.begin();  
 
    /* Inicializa a conexao wi-fi */
    initWiFi();
 
    /* Inicializa a conexao ao broker MQTT */
    initMQTT();
}
 
/* Loop principal */
void loop() 
{
    char temperatura_str[10] = {0};
    char umidade_str[10]     = {0};
     
    /* garante funcionamento das conexões WiFi e ao broker MQTT */
    VerificaConexoesWiFIEMQTT();
 
    /* Compoe as strings a serem enviadas pro dashboard (campos texto) */
    sprintf(temperatura_str,"%.2f", faz_leitura_temperatura());
    sprintf(umidade_str,"%.2f", faz_leitura_umidade());
 
    /*  Envia as strings ao dashboard MQTT */
    MQTT.publish(TOPICO_PUBLISH_TEMPERATURA, temperatura_str, RETAIN);
    MQTT.publish(TOPICO_PUBLISH_UMIDADE, umidade_str, RETAIN);
   
    /* keep-alive da comunicação com broker MQTT */
    MQTT.loop();
 
    /* Refaz o ciclo após 2 segundos */
    delay(2000);
}
