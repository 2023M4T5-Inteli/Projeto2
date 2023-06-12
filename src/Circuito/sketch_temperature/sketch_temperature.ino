
#include <WiFi.h>
#include "UbidotsEsp32Mqtt.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>



const char *UBIDOTS_TOKEN = "BBFF-nroDIloBgvPC2MMff5Evh51fMMWxQr";
const char *WIFI_SSID = "Asgard DECO";      // Put here your Wi-Fi SSID
const char *WIFI_PASS = "28081907";      // Put here your Wi-Fi password
const char *DEVICE_LABEL = "PonderadaS6Mauricio";   // Replace with the device label to subscribe to
const char *VARIABLE_LABEL = "twmp"; // Replace with your variable label to subscribe to

const int PUBLISH_FREQUENCY = 5000; 

unsigned long timer;
uint8_t analogPin = 20; 

Adafruit_BME280 bme;

Ubidots ubidots(UBIDOTS_TOKEN);



void callback(char *topic, byte *payload, unsigned int length)
{
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
  // put your setup code here, to run once:
  Serial.begin(115200);

  bme.begin(0x76);
  
  // ubidots.setDebug(true);  // uncomment this to make debug messages available
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();

  timer = millis();
}

void loop()
{
  //float rssi = WiFi.RSSI();
  float temperature = bme.readTemperature();
  // put your main code here, to run repeatedly:
  if (!ubidots.connected())
  {
    ubidots.reconnect();
  }
  if ((millis() - timer) > PUBLISH_FREQUENCY) // triggers the routine every 5 seconds
  {
    ubidots.add(VARIABLE_LABEL, temperature); // Insert your variable Labels and the value to be sent
    ubidots.publish(DEVICE_LABEL);
    timer = millis();
  }
  ubidots.loop();
  Serial.println(temperature);
  delay(2000);
}
