#include <WiFi.h>
#include <WiFiClient.h>

//----------------------------------------------------------------//

#define velocidade_som 0.034
#define polegadas 0.40

//----------------------------------------------------------------//

const char* ssid = "Inteli-COLLEGE";
const char* password = "QazWsx@123";
const char* serverAddress = "10.128.71.32";  // Endereço IP local do servidor
int serverPort = 3002;                        // Porta usada pelo servidor
WiFiClient client;

//-------------------------------------------------------------------//

// Número máximo de endereços MAC que podem ser armazenados
const int MAX_MACS = 5;
String macs[MAX_MACS];
int numMacs = 0;
int cont = 0;
String complete;

//-------------------------------------------------------------------//

//Definindo vaiaveis para o sistema de quebra do ESP
long duracao;
float distancCm;
float distanciaInch;
const int pinoBuzzer = 25;
const int pinoTrig = 26;
const int pinoEcho = 27;
float potencia;
String sala;

//-------------------------------------------------------------------//

void wifiWait() {
  //Entra num loop se o dispositivo (ESP32) não conecta a rede.
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Conectando ao WiFi...");
    delay(1000);
  }
}

//-------------------------------------------------------------------//

void infoRede() {
  //Retorna no Serial que o disponitivo esta conectado e o Indereço IP Local
  Serial.println("Conectado ao WiFi!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  String bssidStr = WiFi.BSSIDstr();           // pega o endereço mac em tempo real
  Serial.print("Endereço MAC do roteador: ");  //printa o endereço MAC
  Serial.println(bssidStr);

  //Coleta o valor da potencia do sinal do WiFi
  potencia = WiFi.RSSI();
  //Funcao para enviar a potencia do sinal
  Serial.print("Potencia do sinal: ");
  Serial.println(potencia);
}

//-------------------------------------------------------------------//

void serverConect() {
  // Se o client não conecta no ip e na porta ele entra num loop que volta no Serial a mensagem : "Falha na conexão com o server."
  while (!client.connect(serverAddress, serverPort)) {
    Serial.println("Falha na conexão com o server.");
    delay(1000);
  }
}

//-------------------------------------------------------------------//

String returnMac() {
  String bssidStr = WiFi.BSSIDstr();  //pega a string do endereço mac

  //Salva o endereço MAC na varial bssidStr

  bssidStr.replace(":", "");

  //Verifica se o outro dispositivo mandou mensagem.
  client.available();

  // Envia o Endereço MAC e a potencia do sinal para o servidor
  return bssidStr;
}


//-------------------------------------------------------------------//
String fullmsn(String x) {
  //chama a função identificaLocal() e passa como parametro a string de retorno da função returnMac()
  // String mac = returnMac();
  // Serial.println(mac);
  complete = x + "#" + potencia;
  Serial.println(complete);
  client.println(complete);
  return complete;
}
//-------------------------------------------------------------------//

void identificaLocal(String mensagem) {
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
        client.println(macs[numMacs]);  // joga o para o serial do client
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

  if (mensagem == macs[cont]) {
    Serial.println("marcos ta no 1");
    sala = fullmsn("1");
    client.println(sala);
  }
  cont++;
  if (mensagem == macs[cont]) {
    Serial.println("marcos ta no 2");
    sala = fullmsn("2");
    client.println(sala);
  }
  cont++;
  if (mensagem == macs[cont]) {
    Serial.println("marcos ta no 3");
    sala = fullmsn("3");
    client.println(sala);
  }
  cont++;
  if (mensagem == macs[cont]) {
    Serial.println("marcos ta no 4");
    sala = fullmsn("4");
    client.println(sala);
  }
  cont = 0;
}

//-------------------------------------------------------------------//


void latitudeWifi(float & latitude,float & longitude ){
  WiFi.scanNetworks();

  int nRede = WiFi.scanComplete();
  if (nRede == -2){
    WiFi.scanNetworks(true);
    return;
  }  

  if (nRede == 0) {
    Serial.println("Nenhum ponto de acesso Wi-Fi encontrado");
    return;
  }

  int bestSignal = -999;
  int bestNetwork = -1;
  for (int i = 0; i < nRede; i++) {
    int signal = WiFi.RSSI(i);
    if (signal > bestSignal) {
      bestSignal = signal;
      bestNetwork = i;
    }
  }

  if (bestNetwork == -1) {
    Serial.println("Falha ao obter a localização");
    return;
  }

  latitude = WiFi.BSSID(bestNetwork)[0];
  longitude = WiFi.BSSID(bestNetwork)[1];

}











//-------------------------------------------------------------------//

// void quebrou() {
//   digitalWrite(pinoTrig, LOW);
//   delayMicroseconds(2);
//   digitalWrite(pinoTrig, HIGH);
//   delayMicroseconds(10);
//   digitalWrite(pinoTrig, LOW);
//   duracao = pulseIn(pinoEcho, HIGH);         // Calcule a distância
//   distancCm = duracao * velocidade_som / 2;  // Converter para centimetros
//   distanciaInch = distancCm * polegadas;     // Converter para polegadas
//   if (distancCm > 7) {
//     digitalWrite(pinoBuzzer, HIGH);
//     Serial.print("ESP32 retirado do dispositivo!!!");
//     delay(2000);
//     digitalWrite(pinoBuzzer, LOW);
//   }
//   Serial.print("Distancia (cm): ");  // Imprime a distância no Serial Monitor
//   Serial.println(distancCm);
//   Serial.print("Distancia (inch): ");
//   Serial.println(distanciaInch);
//   delay(1000);
// }

//-------------------------------------------------------------------//
void distanciaCalc(float x){

  Serial.println(x);
  if((x<-10)&&(x>=-40)){

    Serial.println("Zona segura A");
  }else if((x<-40)&&(x>=-65)){
    Serial.println("Zona Segura B");
  }else if((x<-65)&&(x>=-100)){
    Serial.println("Zona Segura C");
  }else{
    Serial.println("Fora da Zona");
  }

}


//-------------------------------------------------------------------//
void setup() {
  //define a porta do serial que mostrara as informarções
  Serial.begin(9600);
  // quebrou();

  WiFi.begin(ssid, password);
  //Chama a função wifiWait
  wifiWait();
  //Chama a função infoRede
  infoRede();
  //Chama a função serverConect
  //serverConect();




  // Obter a latitude e a longitude aproximadas
  // float latitude, longitude;
  // latitudeWifi(latitude, longitude);

  // Serial.print("Latitude: ");
  // Serial.println(latitude, 6);
  
  // Serial.print("Longitude: ");
  // Serial.println(longitude, 6);

  
}


void loop() {
    float nivelR= WiFi.RSSI();
  //Funcao para envio de notificacao caso o ESP seja danificado
  //  quebrou();

  //Mostra no serial que está enviando a mensagem para o servidor.
 // Serial.println("Enviando Mensagem ao Server:");

  float latitude, longitude;
  latitudeWifi(latitude, longitude);

  Serial.print("Latitude: ");
  Serial.println(latitude, 6);
  
  Serial.print("Longitude: ");
  Serial.println(longitude, 6);
  distanciaCalc(nivelR);

  delay(5000);

  // Verifica se há dados disponíveis no Serial
  //returnMac()
  //identificaLocal(returnMac());
  // fullmsn();
  //Serial.print(sala);
  //Passa a potencia do Wifi para o host
  //erial.println(potencia);

}