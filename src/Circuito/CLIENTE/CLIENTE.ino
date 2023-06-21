#include <WiFi.h>
#include <WiFiClient.h>
//----------------------------------------------------------------//
#define velocidade_som 0.034
#define polegadas 0.40
//----------------------------------------------------------------//
const char* ssid = "Inteli-COLLEGE";  //Nome da Rede;

const char* senha = "QazWsx@123";  //Senha do WiFi da Rede;

const char* enderecoMac = "10.128.67.66";  // Endereço IP local do servidor

int portaServidor = 8888;  // Porta usada pelo servidor;

const int frequenciaMensagem = 5000;
unsigned long timerMensagem;

WiFiClient client;  //Salva WiFiClient em client;
//-------------------------------------------------------------------//
// Número máximo de endereços MAC que podem ser armazenados
const int qntMacs = 5;

String macs[qntMacs] = { "FC5C45005FB8", "FC5C45006098", "FC5C45005CF8", "FC5C450062", "FC5C450063" };  //lista de endereços macs

int numMacs = 0;

int contador = 0;

String mensagemParaHost;

String sala;
//-------------------------------------------------------------------//
//Definindo vaiaveis para o sistema de quebra do ESP

long duracao;

float distancCm;

float distanciaInch;

const int pinoBuzzer = 25;

const int pinoTrig = 26;

const int pinoEcho = 27;

float potencia;

//-------------------------------------------------------------------//

void aguardaWifi() {
  //Entra num loop se o dispositivo (ESP32) não conecta a rede.
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Conectando ao WiFi...");
    delay(1000);
  }
}

//-------------------------------------------------------------------//
void informacaoDaRede() {

  //Retorna no Serial que o disponitivo esta conectado ao Wifi
  Serial.println("Conectado ao WiFi!");

  Serial.print("Endereço IP: ");  //Printa o Endereço IP Local
  Serial.println(WiFi.localIP());

  String bssidStr = WiFi.BSSIDstr();           // Pega o endereço mac em tempo real
  Serial.print("Endereço MAC do roteador: ");  //printa o endereço MAC
  Serial.println(bssidStr);

  //Printa a Potencia do WiFi;

  Serial.print("Potencia do sinal: ");
  Serial.println(potencia);
}
//-------------------------------------------------------------------//

void conectaServidor() {
  // Se o client não conecta no ip e na porta ele entra num loop que volta no Serial a mensagem : "Falha na conexão com o server."
  while (!client.connect(enderecoMac, portaServidor)) {
    Serial.println("Falha na conexão com o server.");
    delay(1000);
  }
}

//-------------------------------------------------------------------//
String returnaMac() {

  String mensagem = WiFi.BSSIDstr();  //pega a string do endereço mac
  //Salva o endereço MAC na varial mensagem

  mensagem.replace(":", "");  //tira os dois pontos do endereço mac.

  // Envia o Endereço MAC e a potencia do sinal para o servidor
  return mensagem;
}

String concatenaMensagem(String x) {


  float distancia = WiFi.RSSI();  //Salva na variavel distancia a intesidade do sinal do wifi;

  String mensagemParaHost = x + "#" + distancia;  //Salva na variavel mensagemParaHost = o argumento que é passado pela função + "#" + a intesidade do sinal do wifi;

  Serial.println(mensagemParaHost);  //printa no serial
  client.println(mensagemParaHost);  //manda para o host a variavel mensagemParaHost

  return mensagemParaHost;  //retorna a string mensagemParaHost
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

    if (!encontrado) {  //se for diferente de encontrado entra no "if"

      if (numMacs < qntMacs) {

        Serial.print("ISSO FAZ PARTE DE LISTAAAAAAAAAAA: ");
        Serial.println(macs[numMacs]);  // joga o para o serial do client o endereço mac que ele está conectado;



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



  if (macs[0] == mensagem) {

    Serial.println("marcos ta no 1");  //Printa no serial

    sala = concatenaMensagem("1");  //guarda na variavel "sala" o resultado da função concatenaMensagem();

    client.println(sala);  // Manda para o host a variavel "sala"
  }



  if (macs[1] == mensagem) {

    Serial.println("marcos ta no 2");  //Printa no serial

    sala = concatenaMensagem("2");  //guarda na variavel "sala" o resultado da função concatenaMensagem();

    client.println(sala);  // Manda para o host a variavel "sala"
  }


  if (macs[2] == mensagem) {

    Serial.println("marcos ta no 3");  //Printa no serial

    sala = concatenaMensagem("3");  //guarda na variavel "sala" o resultado da função concatenaMensagem();

    client.println(sala);  // Manda para o host a variavel "sala"
  }



  if (macs[3] == mensagem) {

    Serial.println("marcos ta no 4");  //printa no serial

    sala = concatenaMensagem("4");  //guarda na variavel "sala" o resultado da função concatenaMensagem();

    client.println(sala);  // Manda para o host a variavel "sala"
  }
}
//-------------------------------------------------------------------//
  void reconect(float zona){
    if (zona<-80){
      daley(2000)
      ESP.restart();
    }
  }




//-------------------------------------------------------------------//

void quebrou() {

  digitalWrite(pinoTrig, LOW);
  delayMicroseconds(2);

  digitalWrite(pinoTrig, HIGH);
  delayMicroseconds(10);

  digitalWrite(pinoTrig, LOW);

  duracao = pulseIn(pinoEcho, HIGH);         // Calcule a distância
  distancCm = duracao * velocidade_som / 2;  // Converter para centimetros
  distanciaInch = distancCm * polegadas;     // Converter para polegadas

  //logica que diz que se a distancia for maior que 7cm o Buzzer irá apitar;
  if (distancCm > 7) {

    digitalWrite(pinoBuzzer, HIGH);  //ativa o Buzzer
                                     //  Serial.print("ESP32 retirado do dispositivo!!!");//Printa msg no serial

    delay(2000);  //da um delay de 2 s
    digitalWrite(pinoBuzzer, LOW);
  }

  Serial.print("Distancia (cm): ");  //printa no Serial a distancia em cm;
  Serial.println(distancCm);

  Serial.print("Distancia (inch): ");  //printa no Serial a distancia em Inch;
  Serial.println(distanciaInch);

  delay(1000);  // Dá um delay de 1s
}

//-------------------------------------------------------------------//

void setup() {
  //define a porta do serial que mostrara as informarções;
  Serial.begin(115200);

  //Função que faz a conecção com Wifi;
  WiFi.begin(ssid, senha);

  //Chama a função aguardaWifi();

  aguardaWifi();

  //Chama a função informacaoDaRede();

  informacaoDaRede();

  //Chama a função conectaServidor();

  conectaServidor();
}

//-------------------------------------------------------------------//

void loop() {

  //Função para envio de notificacao caso o ESP seja danificado;
  // quebrou();

  //Mostra no serial que está enviando a mensagem para o servidor.

  // Serial.println("Enviando Mensagem ao Server:");

  //chama a função identificaLocal() e passamos como argumento a função returnMac() que retorna uma string;
  if (abs(millis() - timerMensagem) > frequenciaMensagem) {
    identificaLocal(returnaMac());
    timerMensagem = millis();
  float zona = WiFi.RSSI();
  Serial.println(zona);
  reconect(zona);
  conectaServidor();
  }

  // Prita no Serial qual id da Sala ele está;
  // Serial.print(sala);

  //Prita no Serial qual a potencia do Wifi da sala onde ele está ;
  // Serial.println(potencia);
}