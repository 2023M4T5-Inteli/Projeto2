// Definir os pinos dos LEDs e do botão
int buttonPin = 2;
int greenLedPin = 4;
int redLedPin = 13;


// Variáveis para guardar o estado dos LEDs
bool greenLedState = false;
bool redLedState = true;  // Começa com o LED vermelho aceso


// Variável para guardar o estado atual do botão
bool currentButtonState = LOW;
// Variável para guardar o estado anterior do botão
bool previousButtonState = LOW;


void setup() {
  // Configurar os pinos como entrada ou saída
  pinMode(buttonPin, INPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
}


class SerialUtils {
  public:
    static void printMessage(String message) {
      Serial.println(message);
    }
};


void loop() {
  // Ler o estado atual do botão
  currentButtonState = digitalRead(buttonPin);


  // Verificar se o botão foi pressionado
  if (currentButtonState == HIGH && previousButtonState == LOW) {
    // Inverter o estado dos LEDs
    greenLedState = !greenLedState;
    redLedState = !redLedState;


    // Atualizar o estado dos LEDs de acordo com as variáveis
    digitalWrite(greenLedPin, greenLedState);
    digitalWrite(redLedPin, redLedState);
  }


  // Atualizar o estado anterior do botão
  previousButtonState = currentButtonState;


  SerialUtils::printMessage("A temperatura do dispositivo é 32 graus celsius");
  SerialUtils::printMessage("O ESP32 possui as seguintes dimensões: 18 x 25,5 mm.");
  // Esperar um curto intervalo de tempo para evitar pressionamentos múltiplos
  delay(200);
}
