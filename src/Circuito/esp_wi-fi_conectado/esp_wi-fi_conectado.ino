#include <WiFi.h>

const char* ssid;
const char* password;

const char* serverAddress = "10.128.69.94"; // Endereço IP local do servidor
int serverPort = 80; // Porta usada pelo servidor

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);

  // Aguarda a conexão WiFi ser estabelecida
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Conectando ao WiFi...");
    delay(1000);
  }

  Serial.println("Conectado ao WiFi!");
}

void loop() {
  WiFiClient client;

  Serial.println("Conectando ao servidor...");
  
  // Tenta conectar ao servidor usando o endereço IP e a porta especificados
  if (client.connect(serverAddress, serverPort)) {
    Serial.println("Conectado ao servidor!");

    // Envia uma solicitação GET para o servidor
    client.println("GET / HTTP/1.1");
    client.println("Host: 10.128.69.94");
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("Falha na conexão com o servidor!");
  }

  delay(5000); // Aguarda 5 segundos antes de fazer a próxima tentativa de conexão
}
