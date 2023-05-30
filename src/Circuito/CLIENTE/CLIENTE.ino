#include <WiFi.h>
#include <WiFiClient.h>

#define velocidade_som 0.034
#define polegadas 0.40

const char* ssid = "Inteli-COLLEGE";
const char* password = "QazWsx@123";
const char* serverAddress = "10.128.71.32";  // Endereço IP local do servidor
int serverPort = 3002;                        // Porta usada pelo servidor
WiFiClient client;
int cont = 0;

// Número máximo de endereços MAC que podem ser armazenados
const int MAX_MACS = 5;

String macs[MAX_MACS];

int numMacs = 0;

//Definindo vaiaveis para o sistema de quebra do ESP
long duracao;
float distancCm;
float distanciaInch;
const int pinoBuzzer = 25;
const int pinoTrig = 26;
const int pinoEcho = 27;

float potencia;

void wifiWait() {
  //Entra num loop se o dispositivo (ESP32) não conecta a rede.
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Conectando ao WiFi...");
    delay(1000);
  }
}

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

void serverConect() {
  // Se o client não conecta no ip e na porta ele entra num loop que volta no Serial a mensagem : "Falha na conexão com o server."
  while (!client.connect(serverAddress, serverPort)) {
    Serial.println("Falha na conexão com o server.");
    delay(1000);
  }
}

String returnMac() {
  String bssidStr = WiFi.BSSIDstr();  //pega a string do endereço mac

  //Salva o endereço MAC na varial mensagem
  String mensagem = bssidStr;  // acho q o erro ta aqui, o bssidStr não está atualizando quando eu uso o replace

  mensagem.replace(":", "");

  //Verifica se o outro dispositivo mandou mensagem.
  client.available();

  // Envia o Endereço MAC e a potencia do sinal para o servidor
  client.println(mensagem);
  Serial.println(mensagem);  // printa no Serial
  return mensagem;
}


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
    client.println("marcos ta no 1");
  }
  cont++;
  if (mensagem == macs[cont]) {
    Serial.println("marcos ta no 2");
    client.println("marcos ta no 2");
  }
  cont++;
  if (mensagem == macs[cont]) {
    Serial.println("marcos ta no 3");
    client.println("marcos ta no 3");
  }
  cont++;
  if (mensagem == macs[cont]) {
    Serial.println("marcos ta no 4");
    client.println("marcos ta no 4");
  }
  cont = 0;
}

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
  serverConect();
}


void loop() {

  //Funcao para envio de notificacao caso o ESP seja danificado
//  quebrou();
 
  //Mostra no serial que está enviando a mensagem para o servidor.
  Serial.println("Enviando Mensagem ao Server:");

  // Verifica se há dados disponíveis no Serial
  returnMac();
  delay(10000);

  


  identificaLocal(returnMac());
  delay(10000);

  client.println(potencia);
  Serial.println(potencia);
  delay(10000);

}