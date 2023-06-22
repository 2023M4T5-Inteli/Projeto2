#include <EEPROM.h>
#include <SPI.h>
#include <MFRC522.h>

// Define os pinos dos LEDs
constexpr uint8_t redLed = 16;
constexpr uint8_t greenLed = 4;
constexpr uint8_t blueLed = 0;

constexpr uint8_t relay = 32;     // Define o pino do relé
constexpr uint8_t wipeB = 15;     // Pino do botão para o modo de limpeza

boolean match = false;            // Inicializa a variável de correspondência do cartão como falso
boolean programMode = false;      // Inicializa o modo de programação como falso
boolean replaceMaster = false;

uint8_t successRead;              // Variável inteira para armazenar se a leitura do leitor foi bem-sucedida

byte storedCard[4];               // Armazena um ID lido da EEPROM
byte readCard[4];                 // Armazena o ID escaneado lido do módulo RFID
byte masterCard[4];               // Armazena o ID do cartão mestre lido da EEPROM

// Cria uma instância do MFRC522.
constexpr uint8_t RST_PIN = 2;    // Configurável, veja o layout típico dos pinos acima
constexpr uint8_t SS_PIN = 5;     // Configurável, veja o layout típico dos pinos acima

MFRC522 mfrc522(SS_PIN, RST_PIN);

//--------------------------------------------------//

void setup() {
  EEPROM.begin(1024);  // Inicializa a biblioteca EEPROM com 1024 bytes de espaço de armazenamento

  // Configuração dos pinos do Arduino
  pinMode(redLed, OUTPUT);    // Define os pinos dos LEDs como saídas
  pinMode(greenLed, OUTPUT);
  pinMode(blueLed, OUTPUT);
  pinMode(wipeB, INPUT_PULLUP);   // Ativa o resistor pull-up interno no pino do botão
  pinMode(relay, OUTPUT);    // Define o pino do relé como saída

  digitalWrite(relay, HIGH);    // Certifica-se de que a porta está trancada (relé desligado)
  digitalWrite(redLed, LOW);    // Certifica-se de que o LED vermelho está desligado
  digitalWrite(greenLed, LOW);  // Certifica-se de que o LED verde está desligado
  digitalWrite(blueLed, LOW);   // Certifica-se de que o LED azul está desligado

  // Configuração do protocolo
  Serial.begin(9600);     // Inicializa a comunicação serial com o PC (baud rate de 9600)
  SPI.begin();            // Inicializa a comunicação SPI
  mfrc522.PCD_Init();     // Inicializa o hardware do leitor MFRC522

  Serial.println(F("Controle de Acesso v0.1"));    // Mensagem de depuração
  ShowReaderDetails();   // Mostra detalhes do leitor de cartões PCD - MFRC522

  // Limpeza da EEPROM - Se o botão (wipeB) for pressionado durante a inicialização, a EEPROM será apagada
  if (digitalRead(wipeB) == LOW) {   // Quando o botão é pressionado, o pino deve ficar em nível baixo (conectado ao terra)
    digitalWrite(redLed, HIGH);   // Acende o LED vermelho para informar ao usuário que a limpeza será realizada
    Serial.println(F("Botão de formatação pressionado"));
    Serial.println(F("Você tem 10 segundos para cancelar"));
    Serial.println(F("Isso irá apagar todos os registros e não pode ser desfeito"));
    bool buttonState = monitorWipeButton(10000);   // Dá ao usuário tempo suficiente para cancelar a operação
    if (buttonState == true && digitalRead(wipeB) == LOW) {   // Se o botão ainda estiver pressionado, apaga a EEPROM
      Serial.println(F("Iniciando formatação da EEPROM"));
      for (uint16_t x = 0; x < EEPROM.length(); x = x + 1) {   // Percorre todos os endereços da EEPROM
        if (EEPROM.read(x) == 0) {
          // Não faz nada se o endereço da EEPROM já estiver vazio, para economizar tempo e reduzir as escritas na EEPROM
        } else {
          EEPROM.write(x, 0);    // Caso contrário, escreve 0 para limpar o endereço da EEPROM (leva 3.3mS)
        }
      }
      Serial.println(F("EEPROM formatada com sucesso"));
      digitalWrite(redLed, LOW);   // Acende o LED vermelho para visualizar a limpeza bem-sucedida
      delay(200);
      digitalWrite(redLed, HIGH);
      delay(200);
      digitalWrite(redLed, LOW);
      delay(200);
      digitalWrite(redLed, HIGH);
      delay(200);
      digitalWrite(redLed, LOW);
    } else {
      Serial.println(F("Formatação cancelada"));   // Mostra que o botão de limpeza não foi pressionado durante 15 segundos
      digitalWrite(redLed, LOW);
    }
  }

  // Verifica se o cartão mestre está definido, caso contrário, permite que o usuário escolha um cartão mestre
  // Isso também é útil para redefinir o cartão mestre
  // É possível manter outros registros da EEPROM, escrevendo um valor diferente de 143 no endereço 1 da EEPROM
  // O endereço 1 da EEPROM deve conter o número mágico '143'
  if (EEPROM.read(1) != 143) {
    Serial.println(F("Cartão Mestre não definido"));
    Serial.println(F("Leia um chip para definir o Cartão Mestre"));
    do {
      successRead = getID();   // Define successRead como 1 quando a leitura do leitor for bem-sucedida, caso contrário, define como 0
      digitalWrite(blueLed, HIGH);   // Acende o LED azul para indicar que o cartão mestre precisa ser definido
      delay(200);
      digitalWrite(blueLed, LOW);
      delay(200);
    } while (!successRead);   // O programa não prosseguirá até que uma leitura bem-sucedida seja obtida
    for (uint8_t j = 0; j < 4; j++) {   // Loop 4 vezes
      EEPROM.write(2 + j, readCard[j]);   // Escreve o UID do cartão lido na EEPROM, começando no endereço 3
    }
    EEPROM.write(1, 143);   // Escreve na EEPROM que o cartão mestre foi definido
    Serial.println(F("Cartão Mestre definido"));
  }

  Serial.println(F("-------------------"));
  Serial.println(F("UID do Cartão Mestre"));
  for (uint8_t i = 0; i < 4; i++) {   // Lê o UID do Cartão Mestre da EEPROM
    masterCard[i] = EEPROM.read(2 + i);   // Armazena o UID do Cartão Mestre na matriz masterCard
    Serial.print(masterCard[i], HEX);
  }
  Serial.println("");
  Serial.println(F("-------------------"));
  Serial.println(F("Tudo está pronto"));
  Serial.println(F("Aguardando pelos chips para serem lidos"));
<<<<<<< Updated upstream
  cycleLeds();   // Feedback visual para o usuário com os LEDs
=======
  // cycleLeds();    // Everything ready lets give user some feedback by cycling leds
>>>>>>> Stashed changes

  EEPROM.commit();   // Salva as alterações na EEPROM
}

//--------------------------------------------------//

void loop() {
  do {
    successRead = getID();   // Define successRead como 1 quando um cartão é lido com sucesso, caso contrário, define como 0

    // Quando o dispositivo está em uso, se o botão de limpeza for pressionado por 10 segundos, inicializa a limpeza do Cartão Mestre
    if (digitalRead(wipeB) == LOW) {   // Verifica se o botão está pressionado
      // Visualiza que a operação normal foi interrompida ao pressionar o botão de limpeza. O LED vermelho é um aviso ao usuário
      digitalWrite(redLed, HIGH);    // Certifica-se de que o LED vermelho está desligado
      digitalWrite(greenLed, LOW);   // Certifica-se de que o LED verde está desligado
      digitalWrite(blueLed, LOW);    // Certifica-se de que o LED azul está desligado
      // Dá um feedback ao usuário
      Serial.println(F("Botão de formatação pressionado"));
      Serial.println(F("O Cartão Mestre será apagado em 10 segundos"));
      bool buttonState = monitorWipeButton(10000);   // Dá ao usuário tempo suficiente para cancelar a operação
      if (buttonState == true && digitalRead(wipeB) == LOW) {   // Se o botão ainda estiver pressionado, apaga a EEPROM
        EEPROM.write(1, 0);   // Reseta o número mágico
        EEPROM.commit();
        Serial.println(F("Cartão Mestre desvinculado do dispositivo"));
        Serial.println(F("Pressione o botão de reset para reprogramar o Cartão Mestre"));
        while (1);
      }
      Serial.println(F("Desvinculação do Cartão Mestre cancelada"));
    }

    if (programMode) {
      cycleLeds();   // Modo de Programa: os LEDs vermelho, verde e azul piscam esperando a leitura de um novo cartão
    } else {
      normalModeOn();   // Modo Normal: o LED azul de energia está ligado, os outros estão desligados
    }
  } while (!successRead);   // O programa não prosseguirá enquanto uma leitura bem-sucedida não for obtida

  if (programMode) {
    if (isMaster(readCard)) {   // Quando no modo de programa, verifica se o cartão mestre foi lido novamente para sair do modo de programa
      Serial.println(F("Leitura do Cartão Mestre"));
      Serial.println(F("Saindo do modo de programação"));
      Serial.println(F("-----------------------------"));
      programMode = false;
      return;
    } else {
      if (findID(readCard)) {   // Se o cartão lido for conhecido, exclui-o
        Serial.println(F("Conheço este chip, removendo..."));
        deleteID(readCard);
        Serial.println("-----------------------------");
        Serial.println(F("Leia um chip para adicionar ou remover da EEPROM"));
      } else {   // Se o cartão lido não for conhecido, adiciona-o
        Serial.println(F("Não conheço este chip, incluindo..."));
        writeID(readCard);
        Serial.println(F("-----------------------------"));
        Serial.println(F("Leia um chip para adicionar ou remover da EEPROM"));
      }
    }
  } else {
    if (isMaster(readCard)) {   // Se o ID do cartão lido corresponder ao ID do Cartão Mestre, entra no modo de programa
      programMode = true;
      Serial.println(F("Olá Mestre - Modo de programação iniciado"));
      uint8_t count = EEPROM.read(0);   // Lê o primeiro byte da EEPROM que armazena o número de IDs na EEPROM
      Serial.print(F("Existem "));   // Exibe o número de registros na EEPROM
      Serial.print(count);
      Serial.print(F(" registro(s) na EEPROM"));
      Serial.println("");
      Serial.println(F("Leia um chip para adicionar ou remover da EEPROM"));
      Serial.println(F("Leia o Cartão Mestre novamente para sair do modo de programação"));
      Serial.println(F("-----------------------------"));
    } else {
      if (findID(readCard)) {   // Se não estiver no modo de programa, verifica se o cartão está na EEPROM
        Serial.println(F("Bem-vindo, você pode passar"));
        Serial.println(F("Trava do servo destravada"));
        granted(3000);   // Abre a fechadura por 300 ms
      } else {   // Se não estiver na EEPROM, mostra que o ID não é válido
        Serial.println(F("Você não pode passar"));
        Serial.println(F("Trava do servo travada"));
        denied();
      }
    }
  }
}

<<<<<<< Updated upstream
//--------------------------------------------------//

void granted(uint16_t setDelay) {
  digitalWrite(blueLed, LOW);    // Desliga o LED azul
  digitalWrite(redLed, LOW);     // Desliga o LED vermelho
  digitalWrite(greenLed, HIGH);  // Liga o LED verde
  digitalWrite(relay, LOW);      // Destrava a porta
  delay(2000);                   // Mantém a fechadura aberta pelo número de segundos especificado em setDelay
  digitalWrite(relay, HIGH);     // Trava a porta novamente
=======
/////////////////////////////////////////  Access Granted    ///////////////////////////////////
void granted ( uint16_t setDelay) {
  digitalWrite(blueLed, LOW);   // Turn off blue LED
  digitalWrite(redLed, LOW);  // Turn off red LED
  digitalWrite(greenLed, HIGH);   // Turn on green LED
  digitalWrite(relay, LOW);     // Unlock door!
  // myServo.write(180);
  delay(3000);          // Hold door lock open for given seconds
  // digitalWrite(relay, HIGH);    // Relock door
  // myServo.write(0);
  // delay(1000);            // Hold green LED on for a second
  ESP.restart();
>>>>>>> Stashed changes
}

//--------------------------------------------------//

void denied() {
  digitalWrite(greenLed, LOW);  // Desliga o LED verde
  digitalWrite(blueLed, LOW);   // Desliga o LED azul
  digitalWrite(redLed, HIGH);   // Liga o LED vermelho
  delay(1000);                  // Mantém o LED vermelho ligado por 1 segundo
}

//--------------------------------------------------//

uint8_t getID() {
  if (!mfrc522.PICC_IsNewCardPresent()) {   // Verifica se há um novo cartão presente no leitor
    return 0;                              // Retorna 0 se nenhum cartão foi detectado
  }
  if (!mfrc522.PICC_ReadCardSerial()) {     // Lê o serial do cartão PICC
    return 0;                              // Retorna 0 se a leitura falhar
  }
  
  Serial.println(F("UID do chip lido:"));
  for (uint8_t i = 0; i < 4; i++) {         // Lê os 4 bytes do UID do cartão
    readCard[i] = mfrc522.uid.uidByte[i];   // Armazena os bytes do UID no array readCard
    Serial.print(readCard[i], HEX);         // Imprime cada byte do UID em hexadecimal
  }
  Serial.println("");
  
  mfrc522.PICC_HaltA();   // Para a leitura do cartão
  
  return 1;               // Retorna 1 para indicar que a leitura do cartão foi bem-sucedida
}

void ShowReaderDetails() {
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);   // Obtém a versão do software do MFRC522
  Serial.print(F("Versao do software MFRC522: 0x"));
  Serial.println(v, HEX);
  
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else
    Serial.print(F(" (desconhecido),provavelmente um clone chines?"));
  
  Serial.println("");
  
  // Quando 0x00 ou 0xFF é retornado, a comunicação provavelmente falhou
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("ALERTA: Falha na comunicacao, o modulo MFRC522 esta conectado corretamente?"));
    Serial.println(F("SISTEMA ABORTADO: Verifique as conexoes."));
    
    // Visualize que o sistema foi interrompido
    digitalWrite(greenLed, LOW);  // Desliga o LED verde
    digitalWrite(blueLed, LOW);   // Desliga o LED azul
    digitalWrite(redLed, HIGH);   // Liga o LED vermelho
    
    while (true);   // Loop infinito para evitar que o programa prossiga
  }
}

//--------------------------------------------------//

void cycleLeds() {
  digitalWrite(redLed, LOW);       // Desliga o LED vermelho
  digitalWrite(greenLed, HIGH);    // Liga o LED verde
  digitalWrite(blueLed, LOW);      // Desliga o LED azul
  delay(200);
  
  digitalWrite(redLed, LOW);       // Desliga o LED vermelho
  digitalWrite(greenLed, LOW);     // Desliga o LED verde
  digitalWrite(blueLed, HIGH);     // Liga o LED azul
  delay(200);
  
  digitalWrite(redLed, HIGH);      // Liga o LED vermelho
  digitalWrite(greenLed, LOW);     // Desliga o LED verde
  digitalWrite(blueLed, LOW);      // Desliga o LED azul
  delay(200);
}

//--------------------------------------------------//

void normalModeOn() {
  digitalWrite(blueLed, HIGH);   // Liga o LED azul para indicar que o dispositivo está pronto para ler um cartão
  digitalWrite(redLed, LOW);     // Certifica-se de que o LED vermelho está desligado
  digitalWrite(greenLed, LOW);   // Certifica-se de que o LED verde está desligado
  digitalWrite(relay, HIGH);     // Certifica-se de que a porta está trancada
}

//--------------------------------------------------//

void readID(uint8_t number) {
  uint8_t start = (number * 4) + 2;  // Calcula a posição inicial
  for (uint8_t i = 0; i < 4; i++) {  // Loop 4 vezes para obter os 4 bytes
    storedCard[i] = EEPROM.read(start + i);  // Atribui os valores lidos da EEPROM ao array
  }
}

//--------------------------------------------------//

void writeID(byte a[]) {
  if (!findID(a)) { // Antes de escrever na EEPROM, verifique se já encontramos esse cartão antes!
    uint8_t num = EEPROM.read(0); // Obtenha o número de espaços usados, a posição 0 armazena o número de cartões ID
    uint8_t start = (num * 4) + 6; // Determine onde começa o próximo slot
    num++; // Incremente o contador em um
    EEPROM.write(0, num); // Escreva o novo valor do contador
    for (uint8_t j = 0; j < 4; j++) { // Loop 4 vezes
      EEPROM.write(start + j, a[j]); // Escreva os valores do array na EEPROM na posição correta
    }
    EEPROM.commit();
    successWrite();
    Serial.println(F("ID adicionado na EEPROM com sucesso"));
  } else {
    failedWrite();
    Serial.println(F("Erro! O ID do chip está incorreto ou há um problema com a EEPROM"));
  }
}

//--------------------------------------------------//

void deleteID(byte a[]) {
  if (!findID(a)) { // Antes de excluir da EEPROM, verifique se temos este cartão!
    failedWrite(); // Se não tivermos
    Serial.println(F("Erro! O ID do chip está incorreto ou há um problema com a EEPROM"));
  } else {
    uint8_t num = EEPROM.read(0); // Obtenha o número de espaços usados, a posição 0 armazena o número de cartões ID
    uint8_t slot; // Determine o número do slot do cartão
    uint8_t start; // = (num * 4) + 6; // Determine onde começa o próximo slot
    uint8_t looping; // O número de vezes que o loop é repetido
    uint8_t j;
    uint8_t count = EEPROM.read(0); // Leia o primeiro byte da EEPROM que armazena o número de cartões
    slot = findIDSLOT(a); // Determine o número do slot do cartão a ser excluído
    start = (slot * 4) + 2;
    looping = ((num - slot) * 4);
    num--; // Decrementar o contador em um
    EEPROM.write(0, num); // Escreva o novo valor do contador
    for (j = 0; j < looping; j++) { // Loop de deslocamento do cartão
      EEPROM.write(start + j, EEPROM.read(start + 4 + j)); // Desloque os valores do array para 4 posições anteriores na EEPROM
    }
    for (uint8_t k = 0; k < 4; k++) { // Loop de deslocamento
      EEPROM.write(start + j + k, 0);
    }
    EEPROM.commit();
    successDelete();
    Serial.println(F("ID removido da EEPROM com sucesso"));
  }
}

//--------------------------------------------------//

boolean checkTwo(byte a[], byte b[]) {
  boolean match = false; // Inicialmente, assumimos que os arrays não correspondem

  if (a[0] != 0) // Verifique se há algo no array a
    match = true; // Assumimos que eles correspondem

  for (uint8_t k = 0; k < 4; k++) { // Loop 4 vezes
    if (a[k] != b[k]) // Se a[k] for diferente de b[k], definimos match como false
      match = false;
  }

  if (match) {
    return true; // Se os arrays correspondem, retornamos true
  } else {
    return false; // Se os arrays não correspondem, retornamos false
  }
}

//--------------------------------------------------//

uint8_t findIDSLOT(byte find[]) {
  uint8_t count = EEPROM.read(0); // Lê o primeiro byte da EEPROM que armazena o número de cartões ID
  for (uint8_t i = 1; i <= count; i++) { // Loop uma vez para cada entrada da EEPROM
    readID(i); // Lê um ID da EEPROM, que é armazenado em storedCard[4]
    if (checkTwo(find, storedCard)) { // Verifica se o storedCard lido da EEPROM é igual ao cartão ID find[] passado
      return i; // Retorna o número do slot do cartão
      break; // Para de procurar, pois encontramos o cartão
    }
  }
}

//--------------------------------------------------//

boolean findID(byte find[]) {
  uint8_t count = EEPROM.read(0); // Lê o primeiro byte da EEPROM que armazena o número de cartões ID
  for (uint8_t i = 1; i <= count; i++) { // Loop uma vez para cada entrada da EEPROM
    readID(i); // Lê um ID da EEPROM, que é armazenado em storedCard[4]
    if (checkTwo(find, storedCard)) { // Verifica se o storedCard lido da EEPROM é igual ao cartão ID find[] passado como argumento
      return true; // Se houver uma correspondência, retorna true
      break; // Para de procurar, pois encontramos o cartão
    }
  }
  return false; // Se nenhum cartão correspondente for encontrado, retorna false
}

//--------------------------------------------------//

void successWrite() {
  digitalWrite(blueLed, LOW);   // Certifica-se de que o LED azul está desligado
  digitalWrite(redLed, LOW);    // Certifica-se de que o LED vermelho está desligado
  digitalWrite(greenLed, LOW);  // Certifica-se de que o LED verde está ligado
  delay(200);
  digitalWrite(greenLed, HIGH); // Certifica-se de que o LED verde está ligado
  delay(200);
  digitalWrite(greenLed, LOW);  // Certifica-se de que o LED verde está desligado
  delay(200);
  digitalWrite(greenLed, HIGH); // Certifica-se de que o LED verde está ligado
  delay(200);
  digitalWrite(greenLed, LOW);  // Certifica-se de que o LED verde está desligado
  delay(200);
  digitalWrite(greenLed, HIGH); // Certifica-se de que o LED verde está ligado
  delay(200);
}

//--------------------------------------------------//

void failedWrite() {
  digitalWrite(blueLed, LOW);   // Certifica-se de que o LED azul está desligado
  digitalWrite(redLed, LOW);    // Certifica-se de que o LED vermelho está desligado
  digitalWrite(greenLed, LOW);  // Certifica-se de que o LED verde está desligado
  delay(200);
  digitalWrite(redLed, HIGH);   // Certifica-se de que o LED vermelho está ligado
  delay(200);
  digitalWrite(redLed, LOW);    // Certifica-se de que o LED vermelho está desligado
  delay(200);
  digitalWrite(redLed, HIGH);   // Certifica-se de que o LED vermelho está ligado
  delay(200);
  digitalWrite(redLed, LOW);    // Certifica-se de que o LED vermelho está desligado
  delay(200);
  digitalWrite(redLed, HIGH);   // Certifica-se de que o LED vermelho está ligado
  delay(200);
}

//--------------------------------------------------//

void successDelete() {
  digitalWrite(blueLed, LOW);   // Certifica-se de que o LED azul está desligado
  digitalWrite(redLed, LOW);    // Certifica-se de que o LED vermelho está desligado
  digitalWrite(greenLed, LOW);  // Certifica-se de que o LED verde está desligado
  delay(200);
  digitalWrite(blueLed, HIGH);  // Certifica-se de que o LED azul está ligado
  delay(200);
  digitalWrite(blueLed, LOW);   // Certifica-se de que o LED azul está desligado
  delay(200);
  digitalWrite(blueLed, HIGH);  // Certifica-se de que o LED azul está ligado
  delay(200);
  digitalWrite(blueLed, LOW);   // Certifica-se de que o LED azul está desligado
  delay(200);
  digitalWrite(blueLed, HIGH);  // Certifica-se de que o LED azul está ligado
  delay(200);
}

//--------------------------------------------------//

boolean isMaster(byte test[]) {
  if (checkTwo(test, masterCard))
    return true;
  else
    return false;
}

bool monitorWipeButton(uint32_t interval) {
  uint32_t now = (uint32_t)millis();
  while ((uint32_t)millis() - now < interval) {
    // Verifica a cada meio segundo
    if (((uint32_t)millis() % 500) == 0) {
      if (digitalRead(wipeB) != LOW)
        return false;
    }
  }
  return true;
}
