#include <WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Definições dos endereços e parâmetros do LCD
#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 14
#define LCD_ROWS 12

// Inicialização do objeto LiquidCrystal_I2C
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

// Configurações de rede Wi-Fi
const char* ssid = "Inteli-COLLEGE";
const char* password = "QazWsx@123";

// Variáveis para armazenar o endereço MAC recebido
String receivedMac = "";

// Servidor Wi-Fi e cliente
WiFiServer server(80);
WiFiClient client;

// Pinos e variáveis para o sensor de distância
const int pinoBuzzer = 25;
const int pinoTrig = 26;
const int pinoEcho = 27;
long duracao;
float distancCm;
float distanciaInch;

// LEDs indicadores
const int naoConectadoLedVermelho = 15;
const int conectadoLedVerde = 0;
const int conectandoLedAmarelo = 2;

// Constantes para cálculos
const float velocidade_som = 0.034;
const float polegadas = 0.40;

// Função para medir a distância
void quebrou() {
  // Envio do sinal ultrassônico
  digitalWrite(pinoTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(pinoTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinoTrig, LOW);

  // Leitura da duração do eco
  duracao = pulseIn(pinoEcho, HIGH);

  // Cálculo da distância em centímetros e polegadas
  distancCm = duracao * velocidade_som / 2;
  distanciaInch = distancCm * polegadas;

  // Verificação se a distância é maior que 5 cm
  if (distancCm > 5) {
    digitalWrite(pinoBuzzer, HIGH);
    Serial.print("ESP32 retirado do dispositivo!!!");
    delay(2000);
    digitalWrite(pinoBuzzer, LOW);
  }

  // Imprime a distância no monitor serial
  Serial.print("Distância (cm): ");
  Serial.println(distancCm);
  Serial.print("Distância (inch): ");
  Serial.println(distanciaInch);
  delay(1000);
}

// Função para inicializar o LCD
void LCD() {
  Wire.begin();
  lcd.init();
  lcd.backlight();
}

// Função para conectar à rede Wi-Fi
void connectWifi() {
  while (WiFi.status() != WL_CONNECTED) {
    quebrou();
    digitalWrite(naoConectadoLedVermelho, HIGH);
    delay(5000);
    Serial.println("Conectando ao WiFi...");
  }
}

// Função para exibir informações da rede no monitor serial
void infoNet() {
  Serial.println("Conectado ao WiFi!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

// Função para iniciar o servidor Wi-Fi
void iniciaServer() {
  server.begin();
  Serial.println("Servidor iniciado na porta 80.");
}

// Função para verificar se há um cliente conectado
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

// Função para ler mensagens enviadas pelo cliente e exibi-las no LCD
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

// Função para exibir um alerta de conexão intermitente
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
  // Inicialização do monitor serial
  Serial.begin(115200);

  // Configuração dos pinos
  pinMode(pinoTrig, OUTPUT);
  pinMode(pinoEcho, INPUT);
  pinMode(pinoBuzzer, OUTPUT);
  pinMode(naoConectadoLedVermelho, OUTPUT);
  pinMode(conectandoLedAmarelo, OUTPUT);
  pinMode(conectadoLedVerde, OUTPUT);

  // Chamada da função quebrou() para medir a distância inicial
  quebrou();

  // Conexão à rede Wi-Fi
  WiFi.begin(ssid, password);
  connectWifi();

  // Desliga o LED vermelho de não conectado
  digitalWrite(naoConectadoLedVermelho, LOW);

  // Exibe informações da rede no monitor serial
  infoNet();

  // Inicializa o servidor
  iniciaServer();

  // Inicializa o LCD
  LCD();
}

void loop() {
  // Mede a distância
  quebrou();

  // Acende o LED verde indicando conexão
  digitalWrite(conectadoLedVerde, HIGH);

  // Verifica se há um cliente conectado
  verificaCliente();

  // Lê as mensagens enviadas pelo cliente e as exibe no LCD
  mensagemCliente(receivedMac);

  // Acende o LED amarelo indicando conexão em andamento
  digitalWrite(conectandoLedAmarelo, HIGH);
}
