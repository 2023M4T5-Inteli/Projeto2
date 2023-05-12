#include <WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 14
#define LCD_ROWS 12
#define velocidade_som 0.034
#define polegadas 0.40

LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

const char* ssid = "Inteli-COLLEGE";
const char* password = "QazWsx@123";
String receivedMac = "";

WiFiServer server(80);
WiFiClient client;

const int naoConectadoLedVermelho = 15;
const int conectadoLedVerde = 0;
const int conectandoLedAmarelo = 2;

long duracao;
float distancCm;
float distanciaInch;
const int pinoBuzzer = 25;
const int pinoTrig = 26;
const int pinoEcho = 27;

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
  while (WiFi.status() != WL_CONNECTED) {
    quebrou();
    digitalWrite(naoConectadoLedVermelho, HIGH);
    delay(5000);
    Serial.println("Conectando ao WiFi...");
    
  }
}

void infoNet() {
  Serial.println("Conectado ao WiFi!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void iniciaServer() {
  server.begin();
  Serial.println("Servidor iniciado na porta 80.");
}

void verificaCliente() {
  if (!client.connected()) {
    client = server.available();
    digitalWrite(conectandoLedAmarelo, HIGH);
    delay(1000);
    digitalWrite(conectandoLedAmarelo, LOW);
    delay(1000);
    return;
  }
}

void mensagemCliente(String receivedMac) {
  if (client.available()) {

    String receivedMac = client.readStringUntil('\n');
    receivedMac.trim();
    Serial.print("Mensagem recebida: ");
    Serial.println(receivedMac);
    lcd.setCursor(0, 0);
    lcd.print(receivedMac);
    lcd.setCursor(0, 1);
    lcd.print(receivedMac);
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
  Serial.begin(115200);
  pinMode(pinoTrig, OUTPUT);
  pinMode(pinoEcho, INPUT);
  pinMode(pinoBuzzer, OUTPUT);
  pinMode(naoConectadoLedVermelho, OUTPUT);
  pinMode(conectandoLedAmarelo, OUTPUT);
  pinMode(conectadoLedVerde, OUTPUT);
  quebrou();
  WiFi.begin(ssid, password);
  connectWifi();
  digitalWrite(naoConectadoLedVermelho, LOW);
  infoNet();
  iniciaServer();
  LCD();
}

void loop() {
  quebrou();
  digitalWrite(conectadoLedVerde, HIGH);
  verificaCliente();
  mensagemCliente(receivedMac);
  digitalWrite(conectandoLedAmarelo, HIGH);
}