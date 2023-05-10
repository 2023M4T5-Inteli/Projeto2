#include <WiFi.h>
#include <WiFiClient.h>
const char* ssid = "Inteli-COLLEGE";
const char* password = "QazWsx@123";


const int naoConectadoLedVermelho = 4;
const int conectadoLedVerde = 0;
const int conectandoLedAmarelo = 19;


WiFiServer server(80);
WiFiClient client;
int i;
String salas[5] =  { 
  "FC:5C:45:00:4F:C8", 
  "FC:5C:45:00:4D:88", 
  "FC:5C:45:00:60:98", 
  "FC:5C:45:00:5C:F8", 
  "FC:5C:45:00:57:08" 
};

void setup() {
  pinMode(naoConectadoLedVermelho, OUTPUT);
  pinMode(conectandoLedAmarelo, OUTPUT);
  pinMode(conectadoLedVerde, OUTPUT);
  Serial.begin(9600);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(4, HIGH);
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("Conectado ao WiFi!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
  server.begin();  //inicia o server no  server port 80
  Serial.println("Servidor iniciado na porta 80.");
}
void loop() {
  // Aguarda conexão de um cliente
  if (!client || !client.connected()) {
    client = server.available();
    return;
  }
  // Verifica se o cliente enviou uma mensagem
  if (client.available()) {
    // Lê a mensagem enviada pelo cliente
    String mensagem = client.readStringUntil('\n');
    mensagem.trim();
    // Imprime a mensagem recebida
    Serial.print("Mensagem recebida: ");
    Serial.println(mensagem);
    delay(500);
    salas[0].replace(":", "");
    if (mensagem == salas[0]) {
      Serial.println("o Marcos está no atelie: 5 ");
      alertaConexao(conectadoLedVerde, "Dentro da área central");
    } else if (mensagem == salas[1]) {
      Serial.println("o Marcos está na escadaria ");
      alertaConexao(naoConectadoLedVermelho, "Fora da área central");
    } else if (mensagem == salas[2]) {
      Serial.println("o Marcos está no laboratorio ");
      alertaConexao(naoConectadoLedVermelho, "Fora da área central");
    } else if (mensagem == salas[3]) {
      Serial.println("o Marcos está no canto do 3 andar ");
      alertaConexao(naoConectadoLedVermelho, "Fora da área central");
    } else if (mensagem == salas[4]) {
      Serial.println("o Marcos está no 1 ");
      alertaConexao(naoConectadoLedVermelho, "Fora da área central");
    } else {
      Serial.println("Sumiu");
      alertaConexao(conectandoLedAmarelo, "Fora da área de cobertura");
    }
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