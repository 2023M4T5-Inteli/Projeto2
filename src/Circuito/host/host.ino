//Incluindo bibliotecas

#include <WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <PubSubClient.h>

//Incluindo constantes
#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 14
#define LCD_ROWS 12
#define velocidade_som 0.034
#define polegadas 0.40

//Definindo o endereço, o numero de colunas, e o numero de linhas do LCD.
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

//Definindo o nome e a senha da rede.
const char* ssid = "Inteli-COLLEGE";
const char* password = "QazWsx@123";


// variáveis para estabelecer conexão MQTT
const char* mqttServer = "industrial.api.ubidots.com";
const int mqttPort = 1883;
const char* mqttUser = "";
const char* mqttPassword = "646ba57f256c0f31fb1cfc96";

//Definindo o server (server80)
WiFiServer server(80);

//Definindo WifiClient na variavel client.
WiFiClient client;
PubSubClient client(client);

//Salvando os pinos dos leds em variaveis respectivas.
const int naoConectadoLedVermelho = 15;
const int conectadoLedVerde = 0;
const int conectandoLedAmarelo = 2;

long duracao;
float distancCm;
float distanciaInch;
const int pinoBuzzer = 25;
const int pinoTrig = 26;
const int pinoEcho = 27;

// função para lidar com as mensagens assíncronas do MQTT
void callback(char* topic, byte* payload, unsigned int length) {

  // Implemente o código para tratar as mensagens recebidas aqui
  Serial.print("Mensagem está ok");
  Serial.print(topic);
  for(int i = 0; i< length; i++){
    Serial.print((char)payload[i]);
  }
}


void quebrou() {
  digitalWrite(pinoTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(pinoTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinoTrig, LOW);

  duracao = pulseIn(pinoEcho, HIGH);  // Calcule a distância

  distancCm = duracao * velocidade_som / 2;  // Converter para centimetros
  distanciaInch = distancCm * polegadas;     // Converter para polegadas

  if (distancCm > 5) {
    digitalWrite(pinoBuzzer, HIGH);
    Serial.print("ESP32 retirado do dispositivo!!!");
    delay(2000);
    digitalWrite(pinoBuzzer, LOW);
  }

  Serial.print("Distancia (cm): ");  // Imprime a distância no Serial Monitor
  Serial.println(distancCm);
  Serial.print("Distancia (inch): ");
  Serial.println(distanciaInch);
  delay(1000);
}

void LCD() {
  Wire.begin();
  lcd.init();
  lcd.backlight();
}

void connectWifi() {
  //Enquanto não se conecta no Wifi entra num loop que liga o LED vermelho eretorna uma mensagem "conectando ao WIFI...".
  while (WiFi.status() != WL_CONNECTED) {
    quebrou();
    digitalWrite(naoConectadoLedVermelho, HIGH);
    delay(2000);
    Serial.println("Conectando ao WiFi...");
  }
}

void infoNet() {
  //Retorna no Serial que conectou no Wifi e o Endereço LocalIP conectado.
  Serial.println("Conectado ao WiFi!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void iniciaServer() {
  //Inicia o Server na porta 80.
  server.begin();
  //Printa no Serial "Servidor iniciado na porta 80."
  Serial.println("Servidor iniciado na porta 80.");
}


void conectaMQTT() {
  Serial.println("Conectando ao MQTT...");
  if (client.connect("ESP32Client", mqttUser, mqttPassword)) {
    Serial.println("Conexão via MQTT obtida");
  } else {
    Serial.print("Houve falha na conexão MQTT: ")
      Serial.print(client.state());
    Serial.println("Tentando conectar novamente...");
    delay(600);
  }
}

void enviaMensagemMQTT() {
  float valor = 25.5;  // Substitua pelo valor que você deseja enviar
  char mensagem[20];
  sprintf(mensagem, "%.2f", valor);

  client.publish("/topico/publicacao", mensagem);
}

void verificaCliente() {
  //Se o cliente não está conectado o Led amarelo fica piscando até o client(Outro ESP32).
  if (!client.connected()) {
    digitalWrite(conectandoLedAmarelo, HIGH);
    conectaMQTT();
    delay(1000);
    digitalWrite(conectandoLedAmarelo, LOW);
    delay(1000);
    Serial.println("verifica cliente nao conectado");
    return;
  }
}

void mensagemCliente() {
  //Verifica se o client está conectado.
  if (client.connected()) {
    //Verifica se o client mandou mensagem.
    if (client.available()) {
      //Limpa as informações do Display LCD
      lcd.clear();
      //Pega a mensagem que o client manda e quarda na varial "mac".
      String mac = client.readStringUntil('\n');
      //Exclui qualquer espaço "vazio" que exista na variavel "mac"
      mac.trim();
      //Printa no serial a Mensagem recibida do client.
      Serial.print("Mensagem recebida: ");
      Serial.println(mac);
      //Mostra na primeira linha da tela LCD a mensagem recebida do client
      lcd.setCursor(0, 0);
      lcd.print(mac);
      //Mostra na segunda linha da tela LCD a mensagem recebida do client
      lcd.setCursor(0, 1);
      lcd.print("MAC:FC5C45004FC8");
      //Da um delay de 1.5 segundos e limpa as informações do LCD
      delay(1500);
      lcd.clear();
    }
  }
  //Verifica se o client não está conectado.
  if (!client.connected()) {
    //Limpa as informações do Display LCD
    lcd.clear();
    //Mostra na primeira linha da tela LCD que o ESP saiu do Local
    lcd.setCursor(0, 0);
    lcd.print("ESP saiu");
    //Mostra na segunda linha da tela LCD de que local o ESP saiu do
    lcd.setCursor(0, 1);
    lcd.print("MAC:FC5C45004FC8");
    Serial.println("esp saiu; 2if desconectado");
  }
}

void alertaConexao(int led, char* message) {
  while (true) {
    Serial.println(message);
    digitalWrite(led, HIGH);
    delay(300);
    digitalWrite(led, LOW);
    delay(300);
  }
}

void setup() {
  //Seta o serial que sairá as informações de print.
  Serial.begin(9600);

  // inicia função MQTT
  client.begin(callback);
  client.ubidotsSubscribe("detecao");

  //Define os pinos de saida do Buzzer e do Trig COVAS
  pinMode(pinoTrig, OUTPUT);
  pinMode(pinoBuzzer, OUTPUT);
  //Define o pino de entrada do Som COVAS
  pinMode(pinoEcho, INPUT);

  //Define os Pinos dos LED's;
  pinMode(naoConectadoLedVermelho, OUTPUT);
  pinMode(conectandoLedAmarelo, OUTPUT);
  pinMode(conectadoLedVerde, OUTPUT);


  // conexão MQTT
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);



  //Chama a função quebrou.
  quebrou();
  //Conecta ao Wifi
  WiFi.begin(ssid, password);
  //Chama a função connectWifi
  connectWifi();
  //Desliga o LED vermelho.
  digitalWrite(naoConectadoLedVermelho, LOW);
  //Chama a função infoNet.
  infoNet();
  //Chama a função iniciaServer.
  iniciaServer();
  //Chama a função LCD.
  LCD();
}

void loop() {
  //Chama a função quebrou.
  quebrou();
  //Da um delay de 0.5 segundos.
  delay(500);
  //Liga o led verde
  digitalWrite(conectadoLedVerde, HIGH);
  //Liga o led Amarelo
  digitalWrite(conectandoLedAmarelo, HIGH);
  //Chama a função verificaCliente.
  verificaCliente();
  //Da um delay de 0.5 segundos.
  delay(500);
  //Chama a função mensagemCliente.
  mensagemCliente();
  //Da um delay de 0.5 segundos.
  delay(500);
}