//Chama as bibliotecas
#include <WiFi.h>
#include <WiFiClient.h>

//Salva o nome a rede e o IP em constantes.
const char* ssid = "Inteli-COLLEGE";
const char* password = "QazWsx@123";
const char* serverAddress = "10.128.66.252"; // Endereço IP local do servidor

//Salva na variavel serverPort a porta 80 do servidor
int serverPort = 443; // Porta usada pelo servidor

//Salva Wifi Client em client.
WiFiClient client;

//Salva 0 na variavel cont
int cont = 0;


// Número máximo de endereços MAC que podem ser armazenados
const int MAX_MACS = 5;

String macs[MAX_MACS];

//Salva 0 na variavel numMacs
int numMacs = 0;

void wifiWait(){
  //Entra num loop se o dispositivo (ESP32) não conecta a rede.
    while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Conectando ao WiFi...");
    delay(1000);
  }
}

void infoRede(){
  //Retorna no Serial que o disponitivo esta conectado e o Indereço IP Local
  Serial.println("Conectado ao WiFi!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  String bssidStr = WiFi.BSSIDstr(); // pega o endereço mac em tempo real
  Serial.print("Endereço MAC do roteador: "); //printa o endereço MAC
  Serial.println(bssidStr);

}

void serverConect(){

  // Se o client não conecta no ip e na porta ele entra num loop que volta no Serial a mensagem : "Falha na conexão com o server."
  while (!client.connect(serverAddress, serverPort)) {
    Serial.println("Falha na conexão com o server.");
    delay(500);
  }
}

/*String returnMac(){

  String bssidStr = WiFi.BSSIDstr(); //pega a string do endereço mac

  //Salva o endereço MAC na varial mensagem
  String mensagem = bssidStr; // acho q o erro ta aqui, o bssidStr não está atualizando quando eu uso o replace
  
  mensagem.replace(":","");
  Serial.println(mensagem); // printa no Serial 

  return mensagem;
  

}*/

//void digitarMsn(){
  


//}

/*void identificaLocal(String mensagem){
   // Verifica se a mensagem é um endereço MAC válido
    if (mensagem.length() == 12) {
      // Verifica se o endereço MAC já está na lista
      bool encontrado = false;
      for (int i = 0; i < numMacs; i++) {
        if (mensagem == macs[i]) {
          encontrado = true;
          break;
        }
      }

      if (!encontrado) {
        // Adiciona o endereço MAC à lista
        if (numMacs < MAX_MACS) {
          macs[numMacs] = mensagem;
          numMacs++;
          delay(500);
          Serial.println("Endereço MAC adicionado à lista.");
        } else {
          Serial.println("A lista de endereços MAC está cheia.");
        }
      } else {
        Serial.println("Endereço MAC já existe na lista.");
      }
    } else {
      Serial.println("Endereço MAC inválido.");
    }
 
    //Verifica se o outro dispositivo mandou mensagem.
    
    if(mensagem == macs[cont]){
    Serial.println("ESP32 na SALA");
    client.println("ESP32 na SALA");
    } 
    cont++;
    if(mensagem == macs[cont]){
      Serial.println("ESP32 = OB");
      client.println("ESP32 = OB");
    } 
    cont++;
    if(mensagem == macs[cont]){
      Serial.println("ESP32 = ANDAR 3");
      client.println("ESP32 = ANDAR 3");
    } 
    cont++;
    if(mensagem == macs[cont]){
      Serial.println("ESP32 = ANDAR 2");
      client.println("ESP32 = ANDAR 2");
    }
    cont= 0;

    // Envia o Endereço MAC para o servidor

}*/


void setup() {
  //define a porta do serial que mostrara as informarções
  Serial.begin(9600);

  WiFi.begin(ssid, password);
  //Chama a função wifiWait
  wifiWait();
  //Chama a função infoRede
  infoRede();
  //Chama a função serverConect
  serverConect();
}


void loop() {
  //Mostra no serial que esta enviando a mensagem para o server.
  Serial.println("Enviando Mensagem ao Server:");

  //Chama a função returnMac
  //Chama a função identificaLocal
 // returnMac();
  //delay(1000);
  //identificaLocal(returnMac());


 // Serial.println("digite uma mensagem:");
  //if (Serial.available() >0 ) {
  //  String digita = Serial.readStringUntil('\n');
   // client.println(digita + '\n');
 // }
  
  //Manda para o client que o "ESP está na sala".
  //client.println("ESP em sala");

  //Salva o endereço MAC na variavel mac;
  String mac = WiFi.BSSIDstr();
  //Pega o endereço MAC e remove os pontos.
  mac.replace(":","");
  //printa o endereço MAC no Serial
  Serial.println(mac);
  client.println("ESP32 = OB");
  Serial.println("...");
  //Verifica se o cliente desconectou e se desconectou printa no Serial "eita saiu" 
  delay(1000);
}
