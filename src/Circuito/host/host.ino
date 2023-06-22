#include <WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "UbidotsEsp32Mqtt.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

//--------------------------------------------------------------------------//

// Define o endereço do display LCD (verificar com o datasheet do módulo I2C)
#define LCD_ADDRESS 0x27
// Define o número de colunas e linhas do display LCD
#define LCD_COLUMNS 21
#define LCD_ROWS 22
// Define o endereço do BME280
#define BME280_ADDRESS 0x76
// Define variaveis para a funcao do sensor ultrasonico
#define velocidade_som 0.034
#define polegadas 0.40

//--------------------------------------------------------------------------//

// Cria um objeto LiquidCrystal_I2C para controlar o display LCD
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);
Adafruit_BME280 bme;

//--------------------------------------------------------------------------//

// Definindo vaiaveis para o sistema de quebra do ESP
long duracao;
float distancCm;
float distanciaInch;
const int pinoBuzzer = 25;  //Pino padrao do sensor
const int pinoTrig = 26;    //Pino padrao do sensor
const int pinoEcho = 27;    //Pino padrao do sensor

//--------------------------------------------------------------------------//

// Variáveis para contabilizar desconexões
int qtdDesconectado = 0;
bool estadoAnterior = false;

// Constante para o botão de reset do HOST
const int botaoResetHOST = 32;

// Variáveis para manipulação de strings
int separadorIndex = 0;
String mac;
String lugar;
String potencia = "";
String mensagem = "";
String clientMac = "";

// Variável para indicar se o cliente está desconectado
bool clientDesconectado = false;

// Variável para armazenar a temperatura
float temperatura;

// Nome e Senha da rede WIFI
const char *ssid = "Inteli-COLLEGE";
const char *senha = "QazWsx@123";

//--------------------------------------------------------------------------//

//Contantes para comunicacao com Ubidots
const char *UBIDOTS_TOKEN = "BBFF-YjSV5EL4gwTUk2dbqC0Fuc97Y0zgm9";  // Put here your Ubidots TOKEN
const char *DEVICE_LABEL = "klif";                                  // Put here your Device label to which data  will be published
const char *VARIABLE_LABEL = "desconectado";                        // Put here your Variable label to which data  will be published
const char *VARIABLE_LABEL2 = "aguardando";                         // Put here your Variable label to which data  will be published
const char *VARIABLE_LABEL3 = "conectado";                          // Put here your Variable label to which data  will be published
const char *VARIABLE_LABEL4 = "status_estado_ESP";                  // Put here your Variable label to which data  will be published
const char *VARIABLE_LABEL5 = "ID_SALA";                            // Put here your Variable label to which data  will be published
const char *VARIABLE_LABEL6 = "potencia";                           // Put here your Variable label to which data  will be published
const char *VARIABLE_LABEL7 = "Quantidade vezes desconectada";      // Put here your Variable label to which data  will be published
const char *VARIABLE_LABEL8 = "Zona";                               // Put here your Variable label to which data  will be published
const char *VARIABLE_LABEL9 = "temperatura";

//--------------------------------------------------------------------------//

// Constante para a frequência de atualização em milissegundos
unsigned long  int PUBLISH_FREQUENCY = 5000;
const int frequenciaDeReset = 10000;

// Variável para acompanhar o tempo
unsigned long timer;
unsigned long timerReset;

// Instância do cliente Ubidots com o token fornecido
Ubidots ubidots(UBIDOTS_TOKEN);

// Inicializa o servidor na porta 3002
WiFiServer server(4002);

// Cria uma variável do tipo cliente
WiFiClient client;

//--------------------------------------------------------------------------//

// Dando nome aos LEDs
const int naoConectadoLedVermelho = 15;  // LED vermelho indicando dispositivo não conectado
const int conectadoLedVerde = 0;         // LED verde indicando dispositivo conectado
const int conectandoLedAmarelo = 2;      // LED amarelo indicando dispositivo em processo de conexão

int i;  // Variável de uso geral (sem nome específico)

//--------------------------------------------------------------------------//

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Mensagem chegou [");
  Serial.print(topic);
  Serial.print("] ");

  // Imprime o payload da mensagem
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  Serial.println();  // Imprime uma nova linha
}

//--------------------------------------------------------------------------//

void botaoReset() {
  // Verifica se o botão de reset do HOST foi pressionado
  if (digitalRead(botaoResetHOST) == HIGH) {
    // Reinicia o ESP32
    ESP.restart();
  }
}

  //--------------------------------------------------------------------------//

  void LCD() {
    // Inicializa a comunicação I2C com o display LCD
    Wire.begin();
    // Inicializa o display LCD
    lcd.init();
    // Liga o backlight do display LCD
    lcd.backlight();
  }

  //--------------------------------------------------------------------------//

  void connectWifi() {
    // Se o ESP32 não estiver conectado ao WiFi, ele entra em um loop e exibe a mensagem "Conectando ao WiFi..."
    while (WiFi.status() != WL_CONNECTED) {
      digitalWrite(naoConectadoLedVermelho, HIGH);
      delay(1000);
      digitalWrite(naoConectadoLedVermelho, LOW);
      Serial.println("Conectando ao WiFi...");
    }
  }

  //--------------------------------------------------------------------------//

  void infoNet() {
    // Quando o ESP32 se conecta ao WiFi, ele exibe no monitor serial a mensagem de "Conectado ao WiFi!"
    Serial.println("Conectado ao WiFi!");

    // Desliga o LED representado por `naoConectadoLedVermelho`
    digitalWrite(naoConectadoLedVermelho, LOW);

    // Exibe no monitor serial o endereço IP local do ESP32
    Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP());
  }

  //--------------------------------------------------------------------------//

  //inicia o server na porta 3002
  void iniciaServer() {
    server.begin();
    Serial.println("Servidor iniciado na porta 3002.");
  }

  //--------------------------------------------------------------------------//

  // Aguarda conexão de um cliente (outro ESP32) e enquanto não conecta ele pisca o led amarelo na protoboard
  void verificaCliente() {
    lcd.begin(16, 2);
    if (!client || !client.connected()) {
      client = server.available();
      digitalWrite(conectandoLedAmarelo, HIGH);
      delay(1000);
      digitalWrite(conectandoLedAmarelo, LOW);
      delay(1000);
      Serial.println("Aguardando cliente");
      lcd.setCursor(0, 0);
      lcd.print("Aguardando");
      lcd.setCursor(0, 1);
      lcd.print("cliente");
      return;
    }
    digitalWrite(conectadoLedVerde, HIGH);
  }

  //--------------------------------------------------------------------------//

  void mensagemClient() {
    // Verifica se o client (ESP32) mandou alguma mensagem.
    if (!client.available()) {
      clientDesconectado = true;
    }

    // Inicializa o LCD
    lcd.begin(16, 2);

    if (client.available()) {
      clientDesconectado = false;

      Serial.println("Conectado");
      estadoAnterior = true;

      // Lê a mensagem enviada pelo cliente
      mensagem = client.readStringUntil('\n');

      separadorIndex = mensagem.indexOf("#");

      if (separadorIndex != -1) {
        clientMac = mensagem.substring(0, separadorIndex);
        potencia = mensagem.substring(separadorIndex + 1);
      }
    } else if ((!client.available()) && (estadoAnterior == true)) {
      estadoAnterior = false;
      qtdDesconectado++;
      digitalWrite(naoConectadoLedVermelho, HIGH);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ESP Desconectado");
      lcd.setCursor(0, 1);
      lcd.print(lugar);
      Serial.println("ESP Desconectado");
      Serial.println(qtdDesconectado);
      // delay(3000);
    }
  }

  //--------------------------------------------------------------------------//

  void alertaConexao(int led, char *message) {
    while (true) {
      Serial.println(message);
      digitalWrite(led, HIGH);
      delay(300);
      digitalWrite(led, LOW);
      delay(300);
    }
  }

  //--------------------------------------------------------------------------//
  void reconect(float zona){
    if (zona<=-70){
      ESP.restart();
    }
  }

  //--------------------------------------------------------------------------//
  int distanciaCalc(float x) {
    // Inicializa o LCD
    lcd.begin(16, 2);
    lcd.clear();

    int zonas;

    // Imprime o valor da distância no monitor serial
    Serial.println(x);

    if ((x < -10) && (x >= -40)) {
      // Zona segura A
      Serial.println("Zona segura A");
      Serial.println("Zonas = 1");
      lcd.setCursor(0, 0);
      lcd.print("Zona segura A");
      zonas = 1;
      return zonas;
    } else if ((x < -40) && (x >= -65)) {
      // Zona segura B
      Serial.println("Zona segura B");
      Serial.println("Zonas = 2");
      lcd.setCursor(0, 0);
      lcd.print("Zona segura B");
      zonas = 2;
      return zonas;
    } else if ((x < -65) && (x >= -100)) {
      // Zona segura C
      Serial.println("Zona segura C");
      Serial.println("Zonas = 3");
      lcd.setCursor(0, 0);
      lcd.print("Zona segura C");
      zonas = 3;
      return zonas;
    }
  }

  
  //--------------------------------------------------//

  void setup() {
    // Porta de saída para as informações (Porta Serial)
    Serial.begin(9600);

    // Estabelecendo conexão com o broker Ubidots
    ubidots.connectToWifi(ssid, senha);
    ubidots.setCallback(callback);
    ubidots.setup();
    ubidots.reconnect();
    timer = millis();

    // Define os pinos de saída do Buzzer e do Trig
    pinMode(pinoTrig, OUTPUT);
    pinMode(pinoBuzzer, OUTPUT);

    // Define o pino de entrada do Som
    pinMode(pinoEcho, INPUT);

    // Definição dos LEDs
    pinMode(naoConectadoLedVermelho, OUTPUT);
    pinMode(conectandoLedAmarelo, OUTPUT);
    pinMode(conectadoLedVerde, OUTPUT);

    // Chama a função quebrou.
    // quebrou();

    // Conecta-se ao Wi-Fi utilizando o nome e senha da rede
    WiFi.begin(ssid, senha);

    // Chama a função connectWifi.
    connectWifi();

    // Chama a função infoNet.
    infoNet();

    // Chama a função iniciaServer.
    iniciaServer();

    // Chama a função LCD
    LCD();
  }

  void loop() {
    lcd.begin(16, 2);

    // Coloque seu código principal aqui, para ser executado repetidamente:
    verificaCliente();
    mensagemClient();

    // Convertendo variáveis para inteiros
    long id = clientMac.toInt();
    long dbm = 100 - potencia.toInt() * (-1);
    int zona = potencia.toInt();

    Serial.println(dbm);
    Serial.print("Seu ESP32 está na sala ID: ");
    Serial.println(id);
    int zonaF = distanciaCalc(zona);
    reconect(zona);

    if (!ubidots.connected()) {
      ubidots.reconnect();
    }

    // Publica os valores no Ubidots a cada PUBLISH_FREQUENCY segundos
    if ((millis() - timer) > PUBLISH_FREQUENCY) {
      int desconectado = digitalRead(naoConectadoLedVermelho);
      int aguardando = digitalRead(conectandoLedAmarelo);
      int conectado = digitalRead(conectadoLedVerde);

      // Adiciona as variáveis e seus valores ao Ubidots
      addToUbidots();


      timer = millis();
      ubidots.loop();
    }
  }

  void addToUbidots(){
    
      // Adiciona as variáveis e seus valores ao Ubidots
      ubidots.add(VARIABLE_LABEL, clientDesconectado);
      ubidots.publish(DEVICE_LABEL);

      ubidots.add(VARIABLE_LABEL2, aguardando);
      ubidots.publish(DEVICE_LABEL);

      ubidots.add(VARIABLE_LABEL3, conectado);
      ubidots.publish(DEVICE_LABEL);

      ubidots.add(VARIABLE_LABEL4, distancCm);
      ubidots.publish(DEVICE_LABEL);

      ubidots.add(VARIABLE_LABEL5, id);
      ubidots.publish(DEVICE_LABEL);

      ubidots.add(VARIABLE_LABEL6, dbm);
      ubidots.publish(DEVICE_LABEL);

      ubidots.add(VARIABLE_LABEL7, qtdDesconectado);
      ubidots.publish(DEVICE_LABEL);

      ubidots.add(VARIABLE_LABEL8, zonaF);
      ubidots.publish(DEVICE_LABEL);

      ubidots.add(VARIABLE_LABEL9, temperatura);
      ubidots.publish(DEVICE_LABEL);
  }