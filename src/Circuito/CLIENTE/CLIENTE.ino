#include <WiFi.h>
#include <WiFiClient.h>

const char* ssid = "Inteli-COLLEGE";
const char* password = "QazWsx@123";
const char* serverAddress = "10.128.69.94"; // Endereço IP local do servidor
int serverPort = 80; // Porta usada pelo servidor
WiFiClient client;


void setup() {
  Serial.begin(9600);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Conectando ao WiFi...");
    delay(1000);
  }


  Serial.println("Conectado ao WiFi!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  String bssidStr = WiFi.BSSIDstr(); // pega o endereço mac em tempo real
  Serial.print("Endereço MAC do roteador: "); //print
  Serial.println(bssidStr);//print


    // Conecta ao servidor
  while (!client.connect(serverAddress, 80)) { // Substitua pelo endereço IP do host
    Serial.println("Falha na conexão com o server.");
    delay(1000);
  }

 



}


void loop() {

  Serial.println("Enviando Mensagem ao Server:");


  String bssidStr = WiFi.BSSIDstr(); //pega o endereço mac


  String mensagem = bssidStr; // acho q o erro ta aqui, o bssidStr não está atualizando quando eu uso o replace
  mensagem.replace(":", "");//tira os ":" do Mac Adress

  client.available();
    // Envia mensagem para o servidor

  client.println(mensagem); // joga o para o serial do client 
  Serial.println(mensagem); // printa no Serial 
  delay(500);

}
