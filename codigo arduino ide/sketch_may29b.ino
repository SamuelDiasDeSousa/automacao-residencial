//NÃO ESQUEÇAM AS BIBLIOTECAS

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <FS.h>             
#include <ESP8266WiFi.h>        
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>       
#include <ArduinoJson.h>        
#include <PubSubClient.h>
#include <EEPROM.h>

#define DEBUG                   //Se descomentar esta linha vai habilitar a 'impressão' na porta serial


//Coloque os valores padrões aqui, porém na interface eles poderão ser substituídos.
#define servidor_mqtt             "m14.cloudmqtt.com"  //URL do servidor MQTT
#define servidor_mqtt_porta       "10438"  //Porta do servidor (a mesma deve ser informada na variável abaixo)
#define servidor_mqtt_usuario     "rrmboowt"  //Usuário
#define servidor_mqtt_senha       "2kVScZxIkttB"  //Senha
#define mqtt_topico_sub           "esp8266/pincmd"    //Tópico para subscrever o comando a ser dado no pino declarado abaixo

//Declaração do pino que será utilizado e a memória alocada para armazenar o status deste pino na EEPROM
#define pino1                      16                   //Pino que executara a acao dado no topico "esp8266/pincmd" e terá seu status informado no tópico "esp8266/pinstatus"
#define pino2                      0
#define pino3                      10
#define pino4                      2
#define memoria_alocada           4                   //Define o quanto sera alocado na EEPROM (valores entre 4 e 4096 bytes)z


#define OLED_RESET LED_BUILTIN        //ADAFRUIT
Adafruit_SSD1306 display(OLED_RESET); //ADAFRUIT

static const unsigned char PROGMEM ligada[] =
{0x00, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x78, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00,
0x00, 0x04, 0x0F, 0xFF, 0xFF, 0xF0, 0x20, 0x00, 0x00, 0x00, 0x3F, 0xFF, 0xFF, 0xFC, 0x00, 0x00,
0x00, 0x00, 0x7F, 0xFF, 0xFF, 0xFE, 0x00, 0x00, 0x00, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x00,
0x00, 0x00, 0x7F, 0xFF, 0xFF, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xFF, 0xFF, 0xFC, 0x00, 0x00,
0x00, 0x04, 0x1F, 0xFF, 0xFF, 0xF8, 0x20, 0x00, 0x00, 0x00, 0x07, 0xFF, 0xFF, 0xE0, 0x00, 0x00,
0x00, 0x00, 0x03, 0xFF, 0xFF, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x3F, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static const unsigned char PROGMEM desligada[] =
{0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xFF, 0xFF, 0xE0, 0x00, 0x00,
0x00, 0x00, 0x3F, 0xFF, 0xFF, 0xFC, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x00,
0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x00,
0x00, 0x00, 0x7F, 0xFF, 0xFF, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x1F, 0xFF, 0xFF, 0xF8, 0x00, 0x00,
0x00, 0x00, 0x0F, 0xFF, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0xFF, 0xE0, 0x00, 0x00,
0x00, 0x00, 0x01, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0x80, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xE0, 0x00, 0x00, 0x00};

#if (SSD1306_LCDHEIGHT != 64)
#error ("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

WiFiClient espClient;                                 //Instância do WiFiClient
PubSubClient client(espClient);                       //Passando a instância do WiFiClient para a instância do PubSubClient

uint8_t statusAnt   =             0;                  //Variável que armazenará o status do pino que foi gravado anteriormente na EEPROM
bool precisaSalvar  =             false;              //Flag para salvar os dados

//Função para imprimir na porta serial
void imprimirSerial(bool linha, String mensagem){
  #ifdef DEBUG
    if(linha){
      Serial.println(mensagem);
    }else{
      Serial.print(mensagem);
    }
  #endif
}

//Função de retorno para notificar sobre a necessidade de salvar as configurações
void precisaSalvarCallback() {
  precisaSalvar = true;
}


//Função que reconecta ao servidor MQTT
void reconectar() {

  //Repete até conectar
  while (!client.connected()) {
   
    
    //Tentativa de conectar. Se o MQTT precisa de autenticação, será chamada a função com autenticação, caso contrário, chama a sem autenticação. 
    bool conectado = strlen(servidor_mqtt_usuario) > 0 ?
                     client.connect("ESP8266Client", servidor_mqtt_usuario, servidor_mqtt_senha) :
                     client.connect("ESP8266Client");

    if(conectado) {
      //Subscreve para monitorar os comandos recebidos
      client.subscribe(mqtt_topico_sub, 1); //QoS 1
    } else {
      //Aguarda 5 segundos para tentar novamente
      delay(5000);
    }
  }
}

//Função que verifica qual foi o último status do pino antes do ESP ser desligado
void lerStatusAnteriorPino(){
  EEPROM.begin(memoria_alocada);  //Aloca o espaco definido na memoria
  statusAnt = EEPROM.read(0);     //Le o valor armazenado na EEPROM e passa para a variável "statusAnt"
  if(statusAnt > 1){
    statusAnt = 0;                //Provavelmente é a primeira leitura da EEPROM, passando o valor padrão para o pino.
    EEPROM.write(0,statusAnt);
  }
  EEPROM.end();
}

//Função que grava status do pino na EEPROM
void gravarStatusPino(uint8_t statusPino){
  EEPROM.begin(memoria_alocada);
  EEPROM.write(0, statusPino);
  EEPROM.end();
}



//Função inicial (será executado SOMENTE quando ligar o ESP)
void setup() {

  //Fazendo o pino ser de saída, pois ele irá "controlar" algo.
  pinMode(pino1, OUTPUT);
  pinMode(pino2, OUTPUT);
  pinMode(pino3, OUTPUT);
  pinMode(pino4, OUTPUT);

  digitalWrite(pino1, HIGH);
  digitalWrite(pino2, HIGH);
  digitalWrite(pino3, HIGH);
  digitalWrite(pino4, HIGH);

  Serial.begin(9600);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  display.clearDisplay();
  display.drawBitmap(0, 0, desligada, 64, 20, 1);
  display.drawBitmap(64, 0, desligada, 64, 20, 1);
  display.drawBitmap(0, 32, desligada, 64, 20, 1);
  display.drawBitmap(64, 32, desligada, 64, 20, 1);
  nomes();
  display.display();
  
  //Formatando a memória interna
  //(descomente a linha abaixo enquanto estiver testando e comente ou apague quando estiver pronto)
  //SPIFFS.format();

  //Iniciando o SPIFSS (SPI Flash File System)
  if (SPIFFS.begin()) {
    if (SPIFFS.exists("/config.json")) {
      //Arquivo de configuração existe e será lido.
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        size_t size = configFile.size();
        
        //Alocando um buffer para armazenar o conteúdo do arquivo.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
            //Copiando as variáveis salvas previamente no aquivo json para a memória do ESP.
            strcpy(servidor_mqtt, json["servidor_mqtt"]);
            strcpy(servidor_mqtt_porta, json["servidor_mqtt_porta"]);
            strcpy(servidor_mqtt_usuario, json["servidor_mqtt_usuario"]);
            strcpy(servidor_mqtt_senha, json["servidor_mqtt_senha"]);
            strcpy(mqtt_topico_sub, json["mqtt_topico_sub"]);

        } else {
        }
      }
    }
  } else {
  }
  //Fim da leitura do sistema de arquivos SPIFSS

  //Parâmetros extras para configuração
  //Depois de conectar, parameter.getValue() vai pegar o valor configurado.
  //Os campos do WiFiManagerParameter são: id do parâmetro, nome, valor padrão, comprimento
  WiFiManagerParameter custom_mqtt_server("server", "Servidor MQTT", servidor_mqtt, 40);
  WiFiManagerParameter custom_mqtt_port("port", "Porta", servidor_mqtt_porta, 6);
  WiFiManagerParameter custom_mqtt_user("user", "Usuario", servidor_mqtt_usuario, 20);
  WiFiManagerParameter custom_mqtt_pass("pass", "Senha", servidor_mqtt_senha, 20);
  WiFiManagerParameter custom_mqtt_topic_sub("topic_sub", "Topico para subscrever", mqtt_topico_sub, 30);

  //Inicialização do WiFiManager. Uma vez iniciado não é necessário mantê-lo em memória.
  WiFiManager wifiManager;

  //Definindo a função que informará a necessidade de salvar as configurações
  wifiManager.setSaveConfigCallback(precisaSalvarCallback);
  
  //Adicionando os parâmetros para conectar ao servidor MQTT
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_pass);
  wifiManager.addParameter(&custom_mqtt_topic_sub);

  //Busca o ID e senha da rede wifi e tenta conectar.
  //Caso não consiga conectar ou não exista ID e senha,
  //cria um access point com o nome "AutoConnectAP" e a senha "senha123"
  //E entra em loop aguardando a configuração de uma rede WiFi válida.

  /*
  if (!wifiManager.autoConnect("Projeto", "samuel123")) {
    imprimirSerial(true, "Falha ao conectar. Excedeu o tempo limite para conexao.");
    delay(3000);
    //Reinicia o ESP e tenta novamente ou entra em sono profundo (DeepSleep)
    ESP.reset();
    delay(5000);
  }
  */
  
  WiFi.begin("samuel", "12345678");

  
  //Se chegou até aqui é porque conectou na WiFi!
  imprimirSerial(true, "Conectado!! :)");

  //Lendo os parâmetros atualizados
  strcpy(servidor_mqtt, custom_mqtt_server.getValue());
  strcpy(servidor_mqtt_porta, custom_mqtt_port.getValue());
  strcpy(servidor_mqtt_usuario, custom_mqtt_user.getValue());
  strcpy(servidor_mqtt_senha, custom_mqtt_pass.getValue());
  strcpy(mqtt_topico_sub, custom_mqtt_topic_sub.getValue());

  //Salvando os parâmetros informados na tela web do WiFiManager
  if (precisaSalvar) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["servidor_mqtt"] = servidor_mqtt;
    json["servidor_mqtt_porta"] = servidor_mqtt_porta;
    json["servidor_mqtt_usuario"] = servidor_mqtt_usuario;
    json["servidor_mqtt_senha"] = servidor_mqtt_senha;
    json["mqtt_topico_sub"] = mqtt_topico_sub;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
  }


  //Informando ao client do PubSub a url do servidor e a porta.
  int portaInt = atoi(servidor_mqtt_porta);
  client.setServer(servidor_mqtt, portaInt);
  client.setCallback(retorno);
  
  //Obtendo o status do pino antes do ESP ser desligado
  lerStatusAnteriorPino();
}

//Função de repetição (será executado INFINITAMENTE até o ESP ser desligado)
void loop() {
  if (!client.connected()) {
    reconectar();
  }
  client.loop();
}

int d[] = {1,1,1,1};

//Função que será chamada ao receber mensagem do servidor MQTT
void retorno(char* topico, byte* mensagem, unsigned int tamanho) {
  //Convertendo a mensagem recebida para string
  mensagem[tamanho] = '\0';
  String strMensagem = String((char*)mensagem);
  strMensagem.toLowerCase();
  //float f = s.toFloat();
  
  imprimirSerial(false, ". Mensagem: ");
  imprimirSerial(true, strMensagem);

  display.clearDisplay();
  
  if(strMensagem == "liga1"){
    digitalWrite(pino1, LOW);
    d[0] = 0;
  }
  if(strMensagem == "desliga1"){
    digitalWrite(pino1, HIGH);
    d[0] = 1;
  }

  if(strMensagem == "liga2"){
    digitalWrite(pino2, LOW);
    d[1] = 0;
  }
  if(strMensagem == "desliga2"){
    digitalWrite(pino2, HIGH);
    d[1] = 1;
  }

  if(strMensagem == "liga3"){
    digitalWrite(pino3, LOW);
    d[2] = 0;
  }
  if(strMensagem == "desliga3"){
    digitalWrite(pino3, HIGH);
    d[2] = 1;
  }

  if(strMensagem == "liga4"){
    digitalWrite(pino4, LOW);
    d[3] = 0;
  }
  if(strMensagem == "desliga4"){
    digitalWrite(pino4, HIGH);
    d[3] = 1;
  }

  if(strMensagem == "ligatodos"){
    digitalWrite(pino1, LOW);
    digitalWrite(pino2, LOW);
    digitalWrite(pino3, LOW);
    digitalWrite(pino4, LOW);

    d[0] = 0;
    d[1] = 0;
    d[2] = 0;
    d[3] = 0;
  }
  
  if(strMensagem == "desligatodos"){
    imprimirSerial(true, "DESLIGATODOS");
    strMensagem = "liga1";
    strMensagem = "liga2";
    strMensagem = "liga3";
    strMensagem = "liga4";
    digitalWrite(pino1, HIGH);
    digitalWrite(pino2, HIGH);
    digitalWrite(pino3, HIGH);
    digitalWrite(pino4, HIGH);

    d[0] = 1;
    d[1] = 1;
    d[2] = 1;
    d[3] = 1;
  }
  
  piscar(0,0,0,d[0]);
  piscar(64,0,1,d[1]);
  piscar(0,32,2,d[2]);
  piscar(64,32,3,d[3]);
  nomes();
  display.display();
}
//----------------------------------------------------------------------------------------------

//lampada piscando
int pos[] = {0,0,0,0};

void piscar(int x, int y, int p, int d){
    if(d == 1){
      display.drawBitmap(x, y, desligada, 64, 20, 1);
    }
    else{
      if(d == 0 && pos[p] == 0){
         display.drawBitmap(x, y, ligada, 64, 20, 1);
      }
      else{
          display.drawBitmap(x, y, desligada, 64, 20, 1);
      }
    }
}

//nomes das lampadas
void nomes(){
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(21,22);
  display.println("Sala");

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(75,22);
  display.println("Cozinha");

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(15,54);
  display.println("Quarto");

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(75,54);
  display.println("Banheiro");
}



