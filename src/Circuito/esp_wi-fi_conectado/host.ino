#include <WiFi.h>

const char* ssid = "SHARE-RESIDENTE";
const char* password = "Share@residente23";

void setup() {
  pinMode(4, OUTPUT);
  Serial.begin(9600);
  WiFi.begin(ssid, password);

  // Aguarda a conexão WiFi ser estabelecida
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(4, HIGH);
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }

  Serial.println("Conectado ao WiFi!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}
