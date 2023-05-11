#include <WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2

LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

const char* ssid = "Inteli-COLLEGE";
const char* password = "QazWsx@123";
String receivedMac = "";

WiFiServer server(80);
WiFiClient client;

const int naoConectadoLedVermelho = 4;
const int conectadoLedVerde = 0;
const int conectandoLedAmarelo = 19;

void LCD() {
  Wire.begin();
  lcd.init();
  lcd.backlight();
}

void connectWifi() {
  while (WiFi.status() != WL_CONNECTED) {
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
  Serial.begin(9600);
  pinMode(naoConectadoLedVermelho, OUTPUT);
  pinMode(conectandoLedAmarelo, OUTPUT);
  pinMode(conectadoLedVerde, OUTPUT);
  WiFi.begin(ssid, password);
  connectWifi();
  digitalWrite(naoConectadoLedVermelho, LOW);
  infoNet();
  iniciaServer();
  LCD();
}

void loop() {
  digitalWrite(conectadoLedVerde, HIGH);
  verificaCliente();
  mensagemCliente(receivedMac);
  digitalWrite(conectandoLedAmarelo, HIGH);
}
