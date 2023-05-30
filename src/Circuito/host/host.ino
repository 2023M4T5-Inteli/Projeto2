#include <WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "UbidotsEsp32Mqtt.h"

//--------------------------------------------------------------------------//

// Define o endereço do display LCD (verificar com o datasheet do módulo I2C)
#define LCD_ADDRESS 0x27

// Define o número de colunas e linhas do display LCD
#define LCD_COLUMNS 21
#define LCD_ROWS 22

//Define variaveis para a funcao do sensor ultrasonico
#define velocidade_som 0.034
#define polegadas 0.40

//--------------------------------------------------------------------------//

// Cria um objeto LiquidCrystal_I2C para controlar o display LCD
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

//--------------------------------------------------------------------------//

//Definindo vaiaveis para o sistema de quebra do ESP
long duracao;
float distancCm;
float distanciaInch;
const int pinoBuzzer = 25;
const int pinoTrig = 26;
const int pinoEcho = 27;

//--------------------------------------------------------------------------//

//Outras variaveis
int mac_adress;

//Nome e Senha da rede WIFI
const char *ssid = "Inteli-COLLEGE";
const char *password = "QazWsx@123";

//--------------------------------------------------------------------------//

//Contantes para comunicacao com Ubidots
const char *UBIDOTS_TOKEN = "BBFF-UXLQPo0lxDPXTuR1TE0f8gTIkUDJti";  // Put here your Ubidots TOKEN
const char *DEVICE_LABEL = "Kakinho";                               // Put here your Device label to which data  will be published
const char *VARIABLE_LABEL = "desconectado";                        // Put here your Variable label to which data  will be published
const char *VARIABLE_LABEL2 = "aguardando";                         // Put here your Variable label to which data  will be published
const char *VARIABLE_LABEL3 = "conectado";                          // Put here your Variable label to which data  will be published
const char *VARIABLE_LABEL4 = "status_estado_ESP";                  // Put here your Variable label to which data  will be published
const char *VARIABLE_LABEL5 = "MAC_ADRESS";                         // Put here your Variable label to which data  will be published
const char *VARIABLE_LABEL6 = "potencia";                           // Put here your Variable label to which data  will be published

//--------------------------------------------------------------------------//

const int PUBLISH_FREQUENCY = 5000;  // Update rate in milliseconds

unsigned long timer;

Ubidots ubidots(UBIDOTS_TOKEN);

//Inicia Server na porta 80
WiFiServer server(3002);

//Cria uma variavel client
WiFiClient client;

String mac;
String lugar;
String potencia;

//--------------------------------------------------------------------------//

//dado nome aos LEDS
const int naoConectadoLedVermelho = 15;
const int conectadoLedVerde = 0;
const int conectandoLedAmarelo = 2;

int i;

//--------------------------------------------------------------------------//

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
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

// void quebrou() {
//   digitalWrite(pinoTrig, LOW);
//   delayMicroseconds(2);
//   digitalWrite(pinoTrig, HIGH);
//   delayMicroseconds(10);
//   digitalWrite(pinoTrig, LOW);
//   duracao = pulseIn(pinoEcho, HIGH);         // Calcule a distância
//   distancCm = duracao * velocidade_som / 2;  // Converter para centimetros
//   distanciaInch = distancCm * polegadas;     // Converter para polegadas
//   if (distancCm > 7) {
//     digitalWrite(pinoBuzzer, HIGH);
//     Serial.print("ESP32 retirado do dispositivo!!!");
//     delay(2000);
//     digitalWrite(pinoBuzzer, LOW);
//   }
//   Serial.print("Distancia (cm): ");  // Imprime a distância no Serial Monitor
//   Serial.println(distancCm);
//   Serial.print("Distancia (inch): ");
//   Serial.println(distanciaInch);
//   delay(1000);
// }

//--------------------------------------------------------------------------//

void connectWifi() {
  //Se o ESP32 não conectou no WIFI ele entra no loop e gera a mensagem de "Conectando ao WiFi..."
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(naoConectadoLedVermelho, HIGH);
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
}

//--------------------------------------------------------------------------//

void infoNet() {
  //Quando o ESP32 se conecta no WIFI ele retorna no Serial que está conectado a rede e o IpLocal.
  Serial.println("Conectado ao WiFi!");
  digitalWrite(naoConectadoLedVermelho, LOW);
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

//--------------------------------------------------------------------------//

void iniciaServer() {
  //inicia o server na porta 80
  server.begin();
  Serial.println("Servidor iniciado na porta 3002.");
}

//--------------------------------------------------------------------------//

void verificaCliente() {
  // Aguarda conexão de um cliente (outro ESP32) e enquanto não conecta ele pisca o led amarelo na protoboard
  if (!client || !client.connected()) {
    client = server.available();
    digitalWrite(conectandoLedAmarelo, HIGH);
    delay(1000);
    digitalWrite(conectandoLedAmarelo, LOW);
    delay(1000);
    Serial.println("Cliente ainda não conectado");

    return;
  }
}

//--------------------------------------------------------------------------//


String macToDecimal(const String &mac) {
  String decimalAddress = "";

  for (int i = 0; i < mac.length(); i += 3) {
    String hexByte = mac.substring(i, i + 2);
    unsigned int decimalByte = strtol(hexByte.c_str(), NULL, 16);
    decimalAddress += String(decimalByte) + ".";
  }

  decimalAddress.remove(decimalAddress.length() - 1);  // Remove o último ponto

  return decimalAddress;
}

//--------------------------------------------------------------------------//

void mensagemClient() {
  //Verifica se o client(ESP32) mandou alguma mensagem.

  // Inicializa o LCD
  lcd.begin(16, 2);


  if (client.available()) {

    // Lê a mensagem enviada pelo cliente



    mac = client.readStringUntil('\n');
    lcd.setCursor(0, 0);
    lcd.print(mac);
    Serial.print("mac: ");
    Serial.println(mac);
    String newMac = macToDecimal(mac);
    newMac.replace(".", "");

    Serial.print("Endereço MAC em decimal: ");
    Serial.println(newMac);

    delay(5000);

    lugar = client.readStringUntil('\n');
    lcd.setCursor(0, 1);
    lcd.print(lugar);
    Serial.print("lugar: ");
    Serial.println(lugar);
    delay(5000);

    potencia = client.readStringUntil('\n');
    lcd.setCursor(0, 0);
    lcd.print(potencia);
    Serial.print("potencia: ");
    Serial.println(potencia);

    delay(5000);
  } else if (!client.available()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Roubado");
    lcd.setCursor(0, 1);
    lcd.print(lugar);

    Serial.println("Roubado");
    delay(3000);
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


void setup() {
  //Porta de saida para as informações(Porta Serial)
  Serial.begin(9600);

  // estabelecendo conexao com o broker ubidots
  ubidots.connectToWifi(ssid, password);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();

  timer = millis();

  //Define os pinos de saida do Buzzer e do Trig
  pinMode(pinoTrig, OUTPUT);
  pinMode(pinoBuzzer, OUTPUT);
  //Define o pino de entrada do Som
  pinMode(pinoEcho, INPUT);

  //definição dos LEDS
  pinMode(naoConectadoLedVermelho, OUTPUT);
  pinMode(conectandoLedAmarelo, OUTPUT);
  pinMode(conectadoLedVerde, OUTPUT);

  //Chama a função quebrou.
  // quebrou();

  //Conecta com o Wifi pegando o Nome e a Senha da rede
  WiFi.begin(ssid, password);

  //Chama a função connectWifi.
  connectWifi();

  //Desliga o Led vermelho assim que conecta ao Wifi
  digitalWrite(naoConectadoLedVermelho, LOW);

  //Chama a função infoNet.
  infoNet();

  //Chama a função iniciaServer.
  iniciaServer();

  //Chama a função LCD
  LCD();
}


void loop() {


  // put your main code here, to run repeatedly:
  verificaCliente();

  if (!ubidots.connected()) {

    ubidots.reconnect();
  }

  if (abs(millis() - timer) > PUBLISH_FREQUENCY)  // triggers the routine every 5 seconds
  {
    int desconectado = digitalRead(naoConectadoLedVermelho);
    int aguardando = digitalRead(conectandoLedAmarelo);
    int conectado = digitalRead(conectadoLedVerde);


    ubidots.add(VARIABLE_LABEL, desconectado);  // Insert your variable Labels and the value to be sent
    ubidots.publish(DEVICE_LABEL);

    ubidots.add(VARIABLE_LABEL2, aguardando);  // Insert your variable Labels and the value to be sent
    ubidots.publish(DEVICE_LABEL);

    ubidots.add(VARIABLE_LABEL3, conectado);  // Insert your variable Labels and the value to be sent
    ubidots.publish(DEVICE_LABEL);

    ubidots.add(VARIABLE_LABEL4, distancCm);  // Insert your variable Labels and the value to be sent
    ubidots.publish(DEVICE_LABEL);


    float smValue = 1011;



    ubidots.add(VARIABLE_LABEL5, smValue);  // Insert your variable Labels and the value to be sent
    ubidots.publish(DEVICE_LABEL);


    // ubidots.add(VARIABLE_LABEL6, potenciaInt);  // Insert your variable Labels and the value to be sent
    // ubidots.publish(DEVICE_LABEL);

    timer = millis();
    ubidots.loop();
  }
  //Chama a função quebrou.
  // quebrou();
  //Liga o Led Verde
  digitalWrite(conectadoLedVerde, HIGH);

  //Chama a função verificaCliente.
  //Chama a função mensagemCliente.
  mensagemClient();
}