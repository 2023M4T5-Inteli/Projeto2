#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Replace with your Wi-Fi credentials
const char *ssid = "Inteli-COLLEGE";
const char *password = "QazWsx@123";

// Replace with your Ubidots token and variable ID
const char *ubidotsToken = "BBFF-nroDIloBgvPC2MMff5Evh51fMMWxQr";
const char *variableID = "bme280testetemp";

// Create an instance of the BME280 sensor
Adafruit_BME280 bme;

// Ubidots HTTP endpoint
const char* ubidotsURL = "http://industrial.api.ubidots.com/api/v1.6/devices/esp32";

void setup() {
  // Start the serial communication
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize the BME280 sensor
  if (!bme.begin(0x76, &Wire)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
}

void loop() {
  // Read temperature from BME280 sensor
  float temperature = bme.readTemperature();

  // Print temperature to serial monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  // Send temperature to Ubidots
  sendToUbidots(temperature);

  delay(5000);
}

void sendToUbidots(float temperature) {
  // Create JSON payload
  String payload = "{\"temperature\":" + String(temperature) + "}";

  // Create HTTP client
  HTTPClient http;

  // Set Ubidots API endpoint
  http.begin(ubidotsURL);

  // Set Ubidots token and content type header
  http.addHeader("X-Auth-Token", ubidotsToken);
  http.addHeader("Content-Type", "application/json");

  // Send POST request to Ubidots
  int httpResponseCode = http.POST(payload);

  // Check if request was successful
  if (httpResponseCode > 0) {
    Serial.print("Ubidots response code: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("Error sending data to Ubidots. HTTP error code: ");
    Serial.println(httpResponseCode);
  }

  // Free resources
  http.end();
}
