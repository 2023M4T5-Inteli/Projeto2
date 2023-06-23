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
#define LCD_COLUMNS 16
#define LCD_ROWS 2
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

// Definindo variáveis para o sistema de quebra do ESP
long duracao;
float distancCm;
float distanciaInch;
const int pinoBuzzer = 25;  // Pino padrão do sensor de buzzer
const int pinoTrig = 26;    // Pino padrão do sensor de trigger
const int pinoEcho = 27;    // Pino padrão do sensor de echo

//--------------------------------------------------------------------------//

// Variáveis para contabilizar desconexões
int qtdDesconectado = 0;
bool estadoAnterior = false;

// Constante para o botão de reset do HOST
const int botaoResetHOST = 17;

// Variáveis para manipulação de strings
int separadorIndex = 0;

int separadorIndex2 =0;
String mac;
String lugar;
String potencia = "";
String mensagem = "";
String clientMac = "";
String estadoQuebrado = "";

// Variável para indicar se o cliente está desconectado
bool clientDesconectado = false;

// Variável para armazenar a temperatura
float temperatura;

// Nome e senha da rede Wi-Fi
const char *ssid = "Inteli-COLLEGE";
const char *senha = "QazWsx@123";

//--------------------------------------------------------------------------//

// Constantes para comunicação com Ubidots
const char *UBIDOTS_TOKEN = "BBFF-YjSV5EL4gwTUk2dbqC0Fuc97Y0zgm9"; // Insira aqui o seu TOKEN do Ubidots
const char *DEVICE_LABEL = "klif"; // Insira aqui o rótulo do seu dispositivo para o qual os dados serão publicados
const char *VARIABLE_LABEL = "desconectado"; // Insira aqui o rótulo da variável para a qual os dados serão publicados
const char *VARIABLE_LABEL2 = "aguardando"; // Insira aqui o rótulo da variável para a qual os dados serão publicados
const char *VARIABLE_LABEL3 = "conectado"; // Insira aqui o rótulo da variável para a qual os dados serão publicados
const char *VARIABLE_LABEL4 = "status_estado_ESP"; // Insira aqui o rótulo da variável para a qual os dados serão publicados
const char *VARIABLE_LABEL5 = "ID_SALA"; // Insira aqui o rótulo da variável para a qual os dados serão publicados
const char *VARIABLE_LABEL6 = "potencia"; // Insira aqui o rótulo da variável para a qual os dados serão publicados
const char *VARIABLE_LABEL7 = "Quantidade vezes desconectada"; // Insira aqui o rótulo da variável para a qual os dados serão publicados
const char *VARIABLE_LABEL8 = "Zona"; // Insira aqui o rótulo da variável para a qual os dados serão publicados
const char *VARIABLE_LABEL9 = "temperatura"; // Insira aqui o rótulo da variável para a qual os dados serão publicados
const char *VARIABLE_LABEL10 = "estadoQuebrado"; // Insira aqui o rótulo da variável para a qual os dados serão publicados

//--------------------------------------------------------------------------//

// Constante para a frequência de atualização em milissegundos
unsigned long int PUBLISH_FREQUENCY = 4000;
const int frequenciaDeReset = 6000;

// Variável para acompanhar o tempo
unsigned long timer;
// unsigned long timerReset;

// Instância do cliente Ubidots com o token fornecido
Ubidots ubidots(UBIDOTS_TOKEN);

// Inicializa o servidor na porta 4002
WiFiServer server(4002);

// Cria uma variável do tipo cliente
WiFiClient client;

//--------------------------------------------------------------------------//

// Dando nome aos LEDs
const int naoConectadoLedVermelho = 15;  // LED vermelho indicando dispositivo não conectado
const int conectadoLedVerde = 0;         // LED verde indicando dispositivo conectado

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
  // Liga o backlight do display LCD
  lcd.backlight();
  lcd.begin(16,2);
  
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

// Inicia o servidor na porta 3002
void iniciaServer() {
  server.begin();
  Serial.println("Servidor iniciado na porta 3002.");
}

//--------------------------------------------------------------------------//

// Aguarda a conexão de um cliente (outro ESP32)
void verificaCliente() {
  lcd.begin(16, 2);  // Inicializa o display LCD com 16 colunas e 2 linhas

  if (!client || !client.connected()) {  // Se não há cliente conectado ou a conexão foi perdida
    client = server.available();  // Aguarda a conexão de um novo cliente
    delay(1000);
    Serial.println("Aguardando cliente");  // Imprime a mensagem no monitor serial
    lcd.setCursor(0, 0);  // Posiciona o cursor do display LCD na primeira linha
    lcd.print("Aguardando");  // Escreve "Aguardando" na primeira linha do display LCD
    lcd.setCursor(0, 1);  // Posiciona o cursor do display LCD na segunda linha
    lcd.print("cliente");  // Escreve "cliente" na segunda linha do display LCD
    return;
  }
  lcd.clear();

  digitalWrite(conectadoLedVerde, HIGH);  // Acende o LED verde para indicar que um cliente está conectado
}

//--------------------------------------------------------------------------//

void mensagemClient() {
  // Verifica se o cliente (ESP32) enviou alguma mensagem
  if (!client.available()) {
    clientDesconectado = true;  // Define a variável clientDesconectado como true
  }

  // Inicializa o display LCD
  lcd.begin(16, 2);

  if (client.available()) {
    clientDesconectado = false;  // Define a variável clientDesconectado como false, indicando que o cliente está conectado

    Serial.println("Conectado");  // Imprime a mensagem no monitor serial
    estadoAnterior = true;  // Define o estado anterior como true

    // Lê a mensagem enviada pelo cliente
    mensagem = client.readStringUntil('\n');

    separadorIndex = mensagem.indexOf("#");
    separadorIndex2 = mensagem.indexOf("#", separadorIndex + 1);

    if (separadorIndex != -1 && separadorIndex2 != -1) {
      clientMac = mensagem.substring(0, separadorIndex);
      potencia = mensagem.substring(separadorIndex + 1,separadorIndex2);
      potencia = potencia.toInt();
      estadoQuebrado = mensagem.substring(separadorIndex2 + 1);

    }
  } else if ((!client.available()) && (estadoAnterior == true)) {
    estadoAnterior = false;  // Define o estado anterior como false, indicando que o cliente se desconectou
    qtdDesconectado++;  // Incrementa a quantidade de desconexões
    digitalWrite(naoConectadoLedVermelho, HIGH);  // Acende o LED vermelho para indicar que o ESP está desconectado
    lcd.clear();  // Limpa o display LCD
    lcd.setCursor(0, 0);  // Posiciona o cursor do display LCD na primeira linha
    lcd.print("ESP Desconectado");  // Escreve "ESP Desconectado" na primeira linha do display LCD
    lcd.setCursor(0, 1);  // Posiciona o cursor do display LCD na segunda linha
    lcd.print(lugar);  // Escreve o valor da variável lugar na segunda linha do display LCD
    Serial.println("ESP Desconectado");  // Imprime a mensagem no monitor serial
    Serial.println(qtdDesconectado);  // Imprime a quantidade de desconexões no monitor serial
  }
}

//--------------------------------------------------------------------------//

void alertaConexao(int led, char *message) {
  while (true) {
    Serial.println(message);  // Imprime a mensagem no monitor serial
    digitalWrite(led, HIGH);  // Acende o LED especificado
    delay(300);  // Aguarda 300 milissegundos
    digitalWrite(led, LOW);  // Desliga o LED especificado
    delay(300);  // Aguarda mais 300 milissegundos
  }
}

//--------------------------------------------------------------------------//

void reconect(float zona) {
  if (zona <= -70) {
    ESP.restart();  // Reinicia o ESP32
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
  } else if(x<-500 && x>1){
    Serial.println("Zona Perdida");
    Serial.println("Zonas = Perda");
    lcd.setCursor(0, 0);
    lcd.print("Zona segura Perdida");
    zonas = 0;
    return zonas;

  }
}

//--------------------------------------------------//

void setup() {
  // Porta de saída para as informações (Porta Serial)
  Wire.begin();
    // Inicialize o sensor BME280
  if (!bme.begin(BME280_ADDRESS)) {
    Serial.println("Não foi possível encontrar o sensor BME280. Verifique a conexão!");
    while (1);
  }

  // Chama a função LCD
  LCD();

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
  // pinMode(conectandoLedAmarelo, OUTPUT);
  pinMode(conectadoLedVerde, OUTPUT);

  // Conecta-se ao Wi-Fi utilizando o nome e senha da rede
  WiFi.begin(ssid, senha);

  // Chama a função connectWifi.
  connectWifi();

  // Chama a função infoNet.
  infoNet();

  // Chama a função iniciaServer.
  iniciaServer();

}

void loop() {
  lcd.begin(16, 2);

  // Verifica se há um cliente conectado e processa as mensagens recebidas
  verificaCliente();
  mensagemClient();

  // Leitura dos valores do BME280
  float temperatura = bme.readTemperature();

  Serial.print("Temp: ");
  Serial.print(temperatura);
  Serial.println(" C");

  // Convertendo variáveis para inteiros
  long id = clientMac.toInt();
  long dbm = 100 - potencia.toInt() * (-1);
  int zona = potencia.toInt();
  int estadoBuzzer = estadoQuebrado.toInt();

  Serial.print("estado quebrado: ");
  Serial.println(estadoQuebrado);

  Serial.println(dbm);
  Serial.print("Seu ESP32 está na sala ID: ");
  Serial.println(id);

  // Calcula a zona de distância com base na potência
  int zonaF = distanciaCalc(zona);
  Serial.print("A zonaF é: ");
  Serial.println(zonaF);

  // Verifica a necessidade de reconectar o ESP32 com base na zona
  reconect(zona);

  // Verifica a conexão com o servidor Ubidots e reconecta, se necessário
  if (!ubidots.connected()) {
    ubidots.reconnect();
  }

  // Publica os valores no Ubidots a cada PUBLISH_FREQUENCY segundos
  if ((millis() - timer) > PUBLISH_FREQUENCY) {
    int desconectado = digitalRead(naoConectadoLedVermelho);
    // int aguardando = digitalRead(conectandoLedAmarelo);
    int conectado = digitalRead(conectadoLedVerde);

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

    ubidots.add(VARIABLE_LABEL10, estadoBuzzer);
    ubidots.publish(DEVICE_LABEL);

    timer = millis();
    ubidots.loop();
  }
}
