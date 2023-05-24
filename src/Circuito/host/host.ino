#include <WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Define o endereço do display LCD (verificar com o datasheet do módulo I2C)
#define LCD_ADDRESS 0x27

// Define o número de colunas e linhas do display LCD
#define LCD_COLUMNS 16
#define LCD_ROWS 2

// Cria um objeto LiquidCrystal_I2C para controlar o display LCD
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);


//Nome e Senha da rede WIFI
const char* ssid = "Inteli-COLLEGE";
const char* password = "QazWsx@123";

//Inicia Server na porta 80
WiFiServer server(3002);

//Cria uma variavel client
WiFiClient client;

//Lista de endereços MAC
String salas[5] = {"FC:5C:45:00:4F:C8","FC:5C:45:00:4D:88","FC:5C:45:00:60:98","FC:5C:45:00:5C:F8","FC:5C:45:00:57:08"};
String lista[20]={};

//dado nome aos LEDS
const int naoConectadoLedVermelho = 4;
const int conectadoLedVerde = 0;
const int conectandoLedAmarelo = 19;

int i; 

void LCD(){
  // Inicializa a comunicação I2C com o display LCD
  Wire.begin();
  // Inicializa o display LCD
  lcd.init();
  // Liga o backlight do display LCD
  lcd.backlight();
}



void connectWifi(){
  //Se o ESP32 não conectou no WIFI ele entra no loop e gera a mensagem de "Conectando ao WiFi..."
  while (WiFi.status() != WL_CONNECTED) {
  digitalWrite(naoConectadoLedVermelho, HIGH);
  delay(1000);
  Serial.println("Conectando ao WiFi...");
  }
}

void infoNet() {
  //Quando o ESP32 se conecta no WIFI ele retorna no Serial que está conectado a rede e o IpLocal.
  Serial.println("Conectado ao WiFi!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

}

void iniciaServer(){
  //inicia o server na porta 80
  server.begin();
  Serial.println("Servidor iniciado na porta 3002.");
}

void verificaCliente(){
  // Aguarda conexão de um cliente (outro ESP32) e enquanto não conecta ele pisca o led amarelo na protoboard
  if (!client || !client.connected()) {
    client = server.available();
    digitalWrite(conectandoLedAmarelo, HIGH);
    delay(1000);
    digitalWrite(conectandoLedAmarelo, LOW);
    delay(1000);

    return;
  }
}

void mensagemClient(){
  //Verifica se o client(ESP32) mandou alguma mensagem.  
  if (client.available()) {

  // Lê a mensagem enviada pelo cliente
  String mensagem = client.readStringUntil('\n');

  //retorna uma nova string sem os espaços em branco.
  mensagem.trim();

  // Imprime a mensagem recebida do client.
  Serial.print("Mensagem recebida: ");
  Serial.println(mensagem);

  //Da um delay de 0.5segundos
  delay(500);

  lcd.setCursor(0, 0);
  lcd.print(mensagem);
  // Posiciona o cursor na segunda linha, primeira coluna e imprime " "
  delay(5000);
  lcd.setCursor(0, 1);
  lcd.print(mensagem);

}



}

void alertaConexao(int led, char * message) {
  while (true) {
    Serial.println(message);
    digitalWrite(led, HIGH);
    delay(300);
    digitalWrite(led, LOW);
    delay(300);
  }
}


void setup() {
  //Porta de saida para as informações(Porta Serial)
  Serial.begin(9600); 

  //definição dos LEDS 
  pinMode(naoConectadoLedVermelho, OUTPUT);
  pinMode(conectandoLedAmarelo, OUTPUT);
  pinMode(conectadoLedVerde, OUTPUT);

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

  //Liga o Led Verde
  digitalWrite(conectadoLedVerde, HIGH); 
  //Chama a função verificaCliente.
  verificaCliente();
  //Chama a função mensagemCliente.
  mensagemClient();
  //Liga o Led amarelo
  digitalWrite(conectandoLedAmarelo, HIGH);

}