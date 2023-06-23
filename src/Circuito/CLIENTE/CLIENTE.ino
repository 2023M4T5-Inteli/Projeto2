#include <WiFi.h>
#include <WiFiClient.h>

//----------------------------------------------------------------//

#define velocidade_som 0.034
#define polegadas 0.40

//----------------------------------------------------------------//

const char* ssid = "Inteli-COLLEGE";  // Nome da Rede;

const char* senha = "QazWsx@123";  // Senha do WiFi da Rede;

const char* enderecoMac = "10.128.69.3";  // Endereço IP local do servidor

int portaServidor = 4002;  // Porta usada pelo servidor;

const int frequenciaMensagem = 5000;
const int reconecttar = 6000;
unsigned long timerMensagem;
int estadoQuebrado = 0;

WiFiClient client;  // Salva WiFiClient em client;

//-------------------------------------------------------------------//

// Número máximo de endereços MAC que podem ser armazenados

const int qntMacs = 8;

String macs[qntMacs] = { "FC5C45005FB8", "FC5C45006098", "FC5C45005CF8", "FC5C45004FC8", "FC5C45006358", "FC5C45004D88", "FC5C45006888","FC5C45006868" };  //lista de endereços macs

int numMacs = 0;

int contador = 0;

String mensagemParaHost;

String sala;

//-------------------------------------------------------------------//

// Definindo de variáveis para o sistema de quebra do ESP

long duracao;

float distancCm;

float distanciaInch;

const int pinoBuzzer = 25;

const int pinoTrig = 26;

const int pinoEcho = 27;

float potencia;

//-------------------------------------------------------------------//

void aguardaWifi() {
  // Entra num loop se o dispositivo (ESP32) não conecta a rede.
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Conectando ao WiFi...");
    delay(1000);
  }
}

//-------------------------------------------------------------------//

void informacaoDaRede() {
  // Retorna no Serial que o dispositivo esta conectado ao Wifi
  Serial.println("Conectado ao WiFi!");

  Serial.print("Endereço IP: ");  // Imprime o Endereço IP Local
  Serial.println(WiFi.localIP());

  String bssidStr = WiFi.BSSIDstr();           // Pega o endereço mac em tempo real
  Serial.print("Endereço MAC do roteador: ");  // Imprime o endereço MAC
  Serial.println(bssidStr);
  // Imprime a Potencia do WiFi;
  Serial.print("Potencia do sinal: ");
  Serial.println(potencia);
}

//-------------------------------------------------------------------//

void conectaServidor() {
  // Se o client não conecta no ip e na porta ele entra num loop que volta no Serial a mensagem : "Falha na conexão com o server."
  while (!client.connect(enderecoMac, portaServidor)) {
    Serial.println("Tentando conexão com o server.");
    // delay(1000);
  }
}

//-------------------------------------------------------------------//

String returnaMac() {

  String mensagem = WiFi.BSSIDstr();  // Pega a string do endereço mac
  //Salva o endereço MAC na varial mensagem

  mensagem.replace(":", "");  // Tira os dois pontos do endereço mac.

  // Envia o Endereço MAC e a potencia do sinal para o servidor
  return mensagem;
}

String concatenaMensagem(String x, int y) {

  float distancia = WiFi.RSSI();  // Salva na variável distancia a intensidade do sinal do wifi;

  String mensagemParaHost = x + "#" + distancia + "#" + y;  // Salva na variável mensagemParaHost = o argumento que é passado pela função + "#" + a intensidade do sinal do wifi;

  Serial.println(mensagemParaHost);  // Imprime no serial
  client.println(mensagemParaHost);  // Manda para o host a variável mensagemParaHost

  return mensagemParaHost;  // Retorna a string mensagemParaHost
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

    if (!encontrado) {  // Se for diferente de encontrado entra no "if"

      if (numMacs < qntMacs) {

        Serial.print("ISSO FAZ PARTE DE LISTAAAAAAAAAAA: ");
        Serial.println(macs[numMacs]);  // Joga o para o serial do client o endereço mac que ele está conectado;
        Serial.println("Endereço MAC adicionado à lista.");
      } else {
        Serial.println("A lista de endereços MAC está cheia.");
      }
    } else {
      Serial.println("Endereço MAC já existe na lista.");
    }
  } else {

    Serial.print("O Endereço  ");
    Serial.print(mensagem);
    Serial.println(" É inválido.");
  }

  if (macs[0] == mensagem) {

    Serial.println("marcos ta no 1");  // Imprime no serial

    sala = concatenaMensagem("1", estadoQuebrado);  // Guarda na variável "sala" o resultado da função concatenaMensagem();

    client.println(sala);  // Manda para o host a variável "sala"
  }

  if (macs[1] == mensagem) {

    Serial.println("marcos ta no 2");  // Imprime no serial

    sala = concatenaMensagem("2", estadoQuebrado);  // Guarda na variável "sala" o resultado da função concatenaMensagem();

    client.println(sala);  // Manda para o host a variável "sala"
  }

  if (macs[2] == mensagem) {

    Serial.println("marcos ta no 3");  // Imprime no serial

    sala = concatenaMensagem("3", estadoQuebrado);  // Guarda na variável "sala" o resultado da função concatenaMensagem();

    client.println(sala);  // Manda para o host a variável "sala"
  }

  if (macs[3] == mensagem) {

    Serial.println("marcos ta no 4");  // Imprime no serial

    sala = concatenaMensagem("4", estadoQuebrado);  // Guarda na variável "sala" o resultado da função concatenaMensagem();

    client.println(sala);  // Manda para o host a variável "sala"
  }

  if (macs[4] == mensagem) {

    Serial.println("marcos ta no 5");  // Imprime no serial

    sala = concatenaMensagem("5", estadoQuebrado);  // Guarda na variável "sala" o resultado da função concatenaMensagem();

    client.println(sala);  // Manda para o host a variável "sala"
  }

  if (macs[5] == mensagem) {

    Serial.println("marcos ta no 6");  // Imprime no serial

    sala = concatenaMensagem("6", estadoQuebrado);  // Guarda na variável "sala" o resultado da função concatenaMensagem();

    client.println(sala);  // Manda para o host a variável "sala"
  }

  if (macs[6] == mensagem) {

    Serial.println("marcos ta no 7");  // Imprime no serial

    sala = concatenaMensagem("7", estadoQuebrado);  // Guarda na variável "sala" o resultado da função concatenaMensagem();

    client.println(sala);  // Manda para o host a variável "sala"
  }
}

//-------------------------------------------------------------------//

// Esta função verifica a intensidade do sinal WiFi e reconecta o ESP32 se o sinal estiver fraco (abaixo de -77 dBm)
void reconect(float zona) {
  if (zona < -82) {
    ESP.restart();  // Reiniciar o ESP32
  }
}

//-------------------------------------------------------------------//

// Esta função realiza a detecção de quebra utilizando um sensor de distância
void quebrou() {
  // Ela realiza uma sequência de pulsos no pino de gatilho (Trig) do sensor para medir a distância
  digitalWrite(pinoTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(pinoTrig, HIGH);
  delayMicroseconds(10);

  digitalWrite(pinoTrig, LOW);

  duracao = pulseIn(pinoEcho, HIGH);         // Calcula a distância
  distancCm = duracao * velocidade_som / 2;  // Converte para centímetros
  distanciaInch = distancCm * polegadas;     // Converte para polegadas

  // Lógica que ativa o buzzer se a distância for maior que 7 cm
  if (distancCm > 7) {
    digitalWrite(pinoBuzzer, HIGH);  // Ativa o Buzzer
    delay(2000);                     // Atraso de 2 segundos
    digitalWrite(pinoBuzzer, LOW);
    estadoQuebrado = 1;
  }

  Serial.print("Distancia (cm): ");  // Imprime a distância em centímetros no monitor serial
  Serial.println(distancCm);

  Serial.print("Distancia (inch): ");  // Imprime a distância em polegadas no monitor serial
  Serial.println(distanciaInch);

  delay(1000);  // Atraso de 1 segundo
}

//-------------------------------------------------------------------//

void setup() {
  // Define a porta do serial que mostrara as informarções;
  Serial.begin(115200);

  // Função que faz a conecção com Wifi;
  WiFi.begin(ssid, senha);

  // Chama a função aguardaWifi();
  aguardaWifi();

  // Chama a função informacaoDaRede();
  informacaoDaRede();

  // Chama a função conectaServidor();
  conectaServidor();
}

//-------------------------------------------------------------------//

void loop() {
  // Verifica se o tempo decorrido desde a última mensagem é maior que a frequência desejada
  conectaServidor();  // Chama a função "conectaServidor()" para se conectar a um servidor

  identificaLocal(returnaMac());  // Chama a função "identificaLocal()" e passa o endereço MAC retornado pela função "returnaMac()"

  float zona = WiFi.RSSI();  // Obtém a potência do sinal WiFi

  reconect(zona);  // Chama a função "reconect()" passando a potência do sinal WiFi
}
