#include <WiFi.h>
#include "UbidotsEsp32Mqtt.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

const char *UBIDOTS_TOKEN = "BBFF-YjSV5EL4gwTUk2dbqC0Fuc97Y0zgm9";  // Token de autenticação Ubidots
const char *WIFI_SSID = "Inteli-College";  // Nome da rede Wi-Fi
const char *WIFI_PASS = "QazWsx@123";  // Senha da rede Wi-Fi
const char *DEVICE_LABEL = "mauricio";  // Rótulo do dispositivo no Ubidots
const char *VARIABLE_LABEL = "twmp";  // Rótulo da variável a ser enviada para o Ubidots

const int PUBLISH_FREQUENCY = 5000;  // Frequência de publicação dos dados em milissegundos

unsigned long timer;
uint8_t analogPin = 20;  // Pino analógico utilizado

Adafruit_BME280 bme;  // Objeto para ler dados do sensor BME280

Ubidots ubidots(UBIDOTS_TOKEN);  // Objeto para interagir com o Ubidots

void callback(char *topic, byte *payload, unsigned int length)
{
  // Função de retorno de chamada para mensagens MQTT recebidas
  // Imprime a mensagem recebida no monitor serial
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setup()
{
  // Configuração inicial, executada uma vez:
  Serial.begin(115200);  // Inicia a comunicação serial

  bme.begin(0x76);  // Inicia o sensor BME280 com endereço 0x76
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();

  timer = millis();
}

void loop()
{
  float temperature = bme.readTemperature();
  // Código principal a ser executado repetidamente:
  if (!ubidots.connected()){
    ubidots.reconnect();
  }
  if ((millis() - timer) > PUBLISH_FREQUENCY) // Aciona a rotina a cada 5 segundos
  {
    ubidots.add(VARIABLE_LABEL, temperature); // Insira os rótulos das suas variáveis e o valor a ser enviado
    ubidots.publish(DEVICE_LABEL);
    timer = millis();
  }
  ubidots.loop();
  Serial.println(temperature);
  delay(2000);
}