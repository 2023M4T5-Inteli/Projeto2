// #include <Arduino.h>
#include <EEPROM.h>
#include <SPI.h>
#include <MFRC522.h>

constexpr uint8_t redLed = 16;   // Define o pino do LED vermelho
constexpr uint8_t greenLed = 4;  // Define o pino do LED verde
constexpr uint8_t blueLed = 0;   // Define o pino do LED azul

constexpr uint8_t relay = 32;    // Define o pino do relé
constexpr uint8_t wipeB = 15;    // Define o pino do botão para o modo de limpeza

boolean match = false;            // Inicializa a variável "match" como falso (sem correspondência de cartão)
boolean programMode = false;      // Inicializa o modo de programação como falso
boolean replaceMaster = false;    // Inicializa a substituição do cartão mestre como falso

uint8_t successRead;              // Variável inteira para armazenar se a leitura do leitor foi bem-sucedida

byte storedCard[4];               // Armazena um ID lido da EEPROM
byte readCard[4];                 // Armazena o ID lido do módulo RFID
byte masterCard[4];               // Armazena o ID do cartão mestre lido da EEPROM

// Cria uma instância do MFRC522.
constexpr uint8_t RST_PIN = 2;    // Configurável, consulte o layout típico dos pinos acima
constexpr uint8_t SS_PIN = 5;     // Configurável, consulte o layout típico dos pinos acima

MFRC522 mfrc522(SS_PIN, RST_PIN);

//--------------------------------------------------------------------------//

void setup() {
  EEPROM.begin(1024);   // Inicia a comunicação com a EEPROM

  // Configuração dos pinos do Arduino
  pinMode(redLed, OUTPUT);      // Define o pino do LED vermelho como saída
  pinMode(greenLed, OUTPUT);    // Define o pino do LED verde como saída
  pinMode(blueLed, OUTPUT);     // Define o pino do LED azul como saída
  pinMode(wipeB, INPUT_PULLUP); // Define o pino do botão como entrada com resistor pull-up interno
  pinMode(relay, OUTPUT);       // Define o pino do relé como saída
  digitalWrite(relay, HIGH);    // Certifica-se de que a porta está trancada
  digitalWrite(redLed, LOW);    // Certifica-se de que o LED está desligado
  digitalWrite(greenLed, LOW);  // Certifica-se de que o LED está desligado
  digitalWrite(blueLed, LOW);   // Certifica-se de que o LED está desligado

  // Configuração do protocolo
  Serial.begin(9600);     // Inicia a comunicação serial com o PC
  SPI.begin();            // Inicializa o protocolo SPI usado pelo MFRC522
  mfrc522.PCD_Init();     // Inicializa o hardware do MFRC522

  Serial.println(F("Controle de Acesso v0.1"));   // Para fins de depuração
  ShowReaderDetails();    // Mostra detalhes do leitor de cartões PCD - MFRC522

  // Limpeza da EEPROM - Se o botão (wipeB) for pressionado durante a inicialização, a EEPROM será apagada
  if (digitalRead(wipeB) == LOW) {  // Quando o botão é pressionado, o pino deve ficar baixo, o botão está conectado ao terra
    digitalWrite(redLed, HIGH);    // O LED vermelho permanece aceso para informar o usuário que a limpeza será realizada
    Serial.println(F("Botão de formatação pressionado"));
    Serial.println(F("Você tem 10 segundos para cancelar"));
    Serial.println(F("Isso irá apagar todos os registros e não poderá ser desfeito"));
    bool buttonState = monitorWipeButton(10000);  // Dê tempo suficiente para o usuário cancelar a operação
    if (buttonState == true && digitalRead(wipeB) == LOW) {  // Se o botão ainda estiver pressionado, realiza a limpeza da EEPROM
      Serial.println(F("Iniciando formatação da EEPROM"));
      for (uint16_t x = 0; x < EEPROM.length(); x = x + 1) {  // Loop até o final dos endereços da EEPROM
        if (EEPROM.read(x) == 0) {
          // Não faz nada, já está apagado, avança para o próximo endereço para economizar tempo e reduzir gravações na EEPROM
        }
        else {
          EEPROM.write(x, 0);    // Se não estiver apagado, escreve 0 para limpar, leva 3,3 ms
        }
      }
      Serial.println(F("EEPROM formatada com sucesso"));
      digitalWrite(redLed, LOW);   // Visualiza uma limpeza bem

//--------------------------------------------------------------------------//

void loop() {
  do {
    successRead = getID();  // Define successRead como 1 quando lemos um cartão com sucesso, caso contrário, define como 0
    // Quando o dispositivo está em uso, se o botão de limpeza for pressionado por 10 segundos, inicialize a limpeza do cartão Mestre
    if (digitalRead(wipeB) == LOW) {  // Verifica se o botão está pressionado
      // Visualiza que a operação normal foi interrompida pressionando o botão de limpeza. O vermelho é como um aviso ao usuário
      digitalWrite(redLed, HIGH);    // Certifica-se de que o LED está desligado
      digitalWrite(greenLed, LOW);   // Certifica-se de que o LED está desligado
      digitalWrite(blueLed, LOW);    // Certifica-se de que o LED está desligado
      // Dê algum feedback
      Serial.println(F("Botão de formatação pressionado"));
      Serial.println(F("O cartão Mestre será apagado em 10 segundos"));
      bool buttonState = monitorWipeButton(10000);  // Dê tempo suficiente para o usuário cancelar a operação
      if (buttonState == true && digitalRead(wipeB) == LOW) {  // Se o botão ainda estiver pressionado, limpa a EEPROM
        EEPROM.write(1, 0);      // Redefine o Número Mágico.
        EEPROM.commit();
        Serial.println(F("Cartão Mestre desvinculado do dispositivo"));
        Serial.println(F("Pressione o botão de reset da placa para reprogramar o cartão Mestre"));
        while (1);
      }
      Serial.println(F("Desvinculação do cartão Mestre cancelada"));
    }
    if (programMode) {
      cycleLeds();              // O modo de programação cicla entre vermelho, verde e azul esperando para ler um novo cartão
    }
    else {
      normalModeOn();     // Modo normal, LED azul de energia aceso, os demais estão apagados
    }
  } while (!successRead);   // O programa não prosseguirá enquanto não fizermos uma leitura bem-sucedida

  if (programMode) {
    if (isMaster(readCard)) {   // Quando no modo de programação, verifica primeiro se o cartão mestre foi lido novamente para sair do modo de programação
      Serial.println(F("Leitura do cartão Mestre"));
      Serial.println(F("Saindo do modo de programação"));
      Serial.println(F("-----------------------------"));
      programMode = false;
      return;
    }
    else {
      if (findID(readCard)) {    // Se o cartão lido for conhecido, exclua-o
        Serial.println(F("Conheço este chip, removendo..."));
        deleteID(readCard);
        Serial.println("-----------------------------");
        Serial.println(F("Leia um chip para adicionar ou remover da EEPROM"));
      }
      else {                    // Se o cartão lido não for conhecido, adicione-o
        Serial.println(F("Não conheço este chip, incluindo..."));
        writeID(readCard);
        Serial.println("-----------------------------");
        Serial.println(F("Leia um chip para adicionar ou remover da EEPROM"));
      }
    }
  }
  else {
    if (isMaster(readCard)) {    // Se o ID do cartão lido corresponder ao ID do Cartão Mestre - entrar no modo de programação
      programMode = true;
      Serial.println(F("Olá Mestre - Modo de programação iniciado"));
      uint8_t count = EEPROM.read(0);   // Lê o primeiro byte da EEPROM que armazena o número de IDs na EEPROM
      Serial.print(F("Existem "));     // armazena o número de IDs na EEPROM
      Serial.print(count);
      Serial.print(F(" registro(s) na EEPROM"));
      Serial.println("");
      Serial.println(F("Leia um chip para adicionar ou remover da EEPROM"));
      Serial.println(F("Leia o cartão Mestre novamente para sair do modo de programação"));
      Serial.println(F("-----------------------------"));
    }
    else {
      if (findID(readCard)) {    // Se não, verifique se o cartão está na EEPROM
        Serial.println(F("Bem-vindo, você pode passar"));
        Serial.println(F("Servo unlocked"));
        granted(3000);           // Abra a fechadura por 300 ms
      }
      else {                    // Se não, mostre que o ID não é válido
        Serial.println(F("Você não pode passar"));
        Serial.println(F("Servo locked"));
        denied();
      }
    }
  }
}

//--------------------------------------------------------------------------//

void granted(uint16_t setDelay) {
  digitalWrite(blueLed, LOW);     // Desliga o LED azul
  digitalWrite(redLed, LOW);      // Desliga o LED vermelho
  digitalWrite(greenLed, HIGH);   // Liga o LED verde
  digitalWrite(relay, LOW);       // Destrava a porta
  delay(3000);                    // Mantém a fechadura aberta pelo tempo especificado em segundos
  ESP.restart();                  // Reinicia o dispositivo
}

//--------------------------------------------------------------------------//

void denied() {
  digitalWrite(greenLed, LOW);   // Desliga o LED verde
  digitalWrite(blueLed, LOW);    // Desliga o LED azul
  digitalWrite(redLed, HIGH);    // Liga o LED vermelho
  delay(1000);                   // Aguarda por 1 segundo
}

//--------------------------------------------------------------------------//

uint8_t getID() {
  // Preparando-se para ler os PICCs
  if (!mfrc522.PICC_IsNewCardPresent()) {   // Se um novo PICC for colocado no leitor RFID, continue
    return 0;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {     // Desde que um PICC tenha sido colocado, obtenha o número de série e continue
    return 0;
  }
  // Existem PICCs Mifare que têm UID de 4 bytes ou 7 bytes, cuidado se você estiver usando um PICC de 7 bytes
  // Devemos assumir que todo PICC tem UID de 4 bytes
  // Até que suportemos PICCs de 7 bytes
  Serial.println(F("UID do chip lido:"));
  for (uint8_t i = 0; i < 4; i++) {    //
    readCard[i] = mfrc522.uid.uidByte[i];
    Serial.print(readCard[i], HEX);
  }
  Serial.println("");
  mfrc522.PICC_HaltA(); // Interrompe a leitura
  return 1;
}

void ShowReaderDetails() {
  // Obter a versão do software MFRC522
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print(F("Versão do software MFRC522: 0x"));
  Serial.println(v, HEX);
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else
    Serial.print(F(" (desconhecido), provavelmente um clone chinês?"));
  Serial.println("");
  // Quando 0x00 ou 0xFF são retornados, a comunicação provavelmente falhou
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("ALERTA: Falha na comunicação, o módulo MFRC522 está conectado corretamente?"));
    Serial.println(F("SISTEMA ABORTADO: Verifique as conexões."));
    // Visualize que o sistema está parado
    digitalWrite(greenLed, LOW);  // Certifique-se de que o LED verde esteja desligado
    digitalWrite(blueLed, LOW);   // Certifique-se de que o LED azul esteja desligado
    digitalWrite(redLed, HIGH);   // Acenda o LED vermelho
    while (true);
  }
}

//--------------------------------------------------------------------------//

void cycleLeds() {
  digitalWrite(redLed, LOW);  // Certifique-se de que o LED vermelho esteja desligado
  digitalWrite(greenLed, HIGH);   // Certifique-se de que o LED verde esteja ligado
  digitalWrite(blueLed, LOW);   // Certifique-se de que o LED azul esteja desligado
  delay(200);
  digitalWrite(redLed, LOW);  // Certifique-se de que o LED vermelho esteja desligado
  digitalWrite(greenLed, LOW);  // Certifique-se de que o LED verde esteja desligado
  digitalWrite(blueLed, HIGH);  // Certifique-se de que o LED azul esteja ligado
  delay(200);
  digitalWrite(redLed, HIGH);   // Certifique-se de que o LED vermelho esteja ligado
  digitalWrite(greenLed, LOW);  // Certifique-se de que o LED verde esteja desligado
  digitalWrite(blueLed, LOW);   // Certifique-se de que o LED azul esteja desligado
  delay(200);
}

//--------------------------------------------------------------------------//

void normalModeOn() {
  digitalWrite(blueLed, HIGH);  // LED azul LIGADO e pronto para ler o cartão
  digitalWrite(redLed, LOW);  // Certifique-se de que o LED vermelho esteja desligado
  digitalWrite(greenLed, LOW);  // Certifique-se de que o LED verde esteja desligado
  digitalWrite(relay, HIGH);    // Certifique-se de que a porta esteja trancada
}

//--------------------------------------------------------------------------//

void readID(uint8_t number) {
  uint8_t start = (number * 4) + 2;    // Determine a posição inicial
  for (uint8_t i = 0; i < 4; i++) {     // Loop 4 vezes para obter os 4 bytes
    storedCard[i] = EEPROM.read(start + i);   // Atribui os valores lidos da EEPROM ao array
  }
}

//--------------------------------------------------------------------------//

void writeID(byte a[]) {
  if (!findID(a)) {    // Antes de escrever na EEPROM, verifique se já vimos este cartão antes!
    uint8_t num = EEPROM.read(0);    // Obtenha o número de espaços usados, a posição 0 armazena o número de cartões ID
    uint8_t start = (num * 4) + 6;   // Descubra onde começa o próximo slot
    num++;    // Incrementa o contador em um
    EEPROM.write(0, num);    // Escreva o novo número no contador
    for (uint8_t j = 0; j < 4; j++) {    // Loop 4 vezes
      EEPROM.write(start + j, a[j]);   // Escreva os valores do array na EEPROM na posição correta
    }
    EEPROM.commit();
    successWrite();
    Serial.println(F("ID adicionado na EEPROM com sucesso"));
  }
  else {
    failedWrite();
    Serial.println(F("Erro! Tem alguma coisa errada com o ID do chip ou problema na EEPROM"));
  }
}

//--------------------------------------------------------------------------//

void deleteID(byte a[]) {
  if (!findID(a)) {    // Antes de excluir da EEPROM, verifique se temos este cartão!
    failedWrite();    // Se não tiver
    Serial.println(F("Erro! Tem alguma coisa errada com o ID do chip ou problema na EEPROM"));
  }
  else {
    uint8_t num = EEPROM.read(0);    // Obtenha o número de espaços usados, a posição 0 armazena o número de cartões ID
    uint8_t slot;     // Descubra o número do slot do cartão
    uint8_t start;    // = (num * 4) + 6; // Descubra onde começa o próximo slot
    uint8_t looping;  // O número de vezes que o loop se repete
    uint8_t j;
    uint8_t count = EEPROM.read(0);   // Leia o primeiro byte da EEPROM que armazena o número de cartões
    slot = findIDSLOT(a);    // Descubra o número do slot do cartão a ser excluído
    start = (slot * 4) + 2;
    looping = ((num - slot) * 4);
    num--;    // Decrementa o contador em um
    EEPROM.write(0, num);    // Escreva o novo número no contador
    for (j = 0; j < looping; j++) {     // Loop o número de vezes para mover os cartões
      EEPROM.write(start + j, EEPROM.read(start + 4 + j));   // Desloque os valores do array para 4 posições antes na EEPROM
    }
    for (uint8_t k = 0; k < 4; k++) {    // Loop para limpar as posições vazias
      EEPROM.write(start + j + k, 0);
    }
    EEPROM.commit();
    successDelete();
    Serial.println(F("ID removido da EEPROM com sucesso"));
  }
}

//--------------------------------------------------------------------------//

boolean checkTwo(byte a[], byte b[]) {
  boolean match = false;    // Variável booleana para indicar se os arrays a[] e b[] são iguais
  
  if (a[0] != 0)    // Verifique se há algo no array primeiro
    match = true;   // Suponha que eles sejam iguais inicialmente
  
  for (uint8_t k = 0; k < 4; k++) {   // Loop 4 vezes
    if (a[k] != b[k])     // Se a[k] for diferente de b[k], defina match como falso, se um falhar, todos falham
      match = false;
  }
  
  if (match) {
    return true;    // Retorna true se match ainda for verdadeiro
  }
  else {
    return false;   // Retorna false se match for falso
  }
}

//--------------------------------------------------------------------------//

uint8_t findIDSLOT(byte find[]) {
  uint8_t count = EEPROM.read(0);   // Lê o primeiro byte da EEPROM que armazena o número de cartões
  for (uint8_t i = 1; i <= count; i++) {   // Loop uma vez para cada entrada na EEPROM
    readID(i);   // Lê um ID da EEPROM, que é armazenado em storedCard[4]
    if (checkTwo(find, storedCard)) {   // Verifica se o storedCard lido da EEPROM é o mesmo que o ID find[] passado
      return i;   // Retorna o número do slot do cartão
      break;      // Para de procurar, pois encontramos o ID
    }
  }
}

//--------------------------------------------------------------------------//

boolean findID(byte find[]) {
  uint8_t count = EEPROM.read(0);   // Lê o primeiro byte da EEPROM que armazena o número de cartões
  for (uint8_t i = 1; i <= count; i++) {   // Loop uma vez para cada entrada na EEPROM
    readID(i);   // Lê um ID da EEPROM, que é armazenado em storedCard[4]
    if (checkTwo(find, storedCard)) {   // Verifica se o storedCard lido da EEPROM é o mesmo que o ID find[] passado
      return true;   // Retorna true se o ID for encontrado
      break;   // Para de procurar, pois encontramos o ID
    }
  }
  return false;   // Retorna false se o ID não for encontrado
}

//--------------------------------------------------------------------------//

// Pisca o LED verde 3 vezes para indicar uma gravação bem-sucedida na EEPROM
void successWrite() {
digitalWrite(blueLed, LOW); // Certifica-se de que o LED azul está desligado
digitalWrite(redLed, LOW); // Certifica-se de que o LED vermelho está desligado
digitalWrite(greenLed, LOW); // Certifica-se de que o LED verde está ligado
delay(200);
digitalWrite(greenLed, HIGH); // Certifica-se de que o LED verde está ligado
delay(200);
digitalWrite(greenLed, LOW); // Certifica-se de que o LED verde está desligado
delay(200);
digitalWrite(greenLed, HIGH); // Certifica-se de que o LED verde está ligado
delay(200);
digitalWrite(greenLed, LOW); // Certifica-se de que o LED verde está desligado
delay(200);
digitalWrite(greenLed, HIGH); // Certifica-se de que o LED verde está ligado
delay(200);
}

//--------------------------------------------------------------------------//

// Pisca o LED vermelho 3 vezes para indicar uma gravação falha na EEPROM
void failedWrite() {
  digitalWrite(blueLed, LOW); // Certifica-se de que o LED azul está desligado
  digitalWrite(redLed, LOW); // Certifica-se de que o LED vermelho está desligado
  digitalWrite(greenLed, LOW); // Certifica-se de que o LED verde está desligado
  delay(200);
  digitalWrite(redLed, HIGH); // Certifica-se de que o LED vermelho está ligado
  delay(200);
  digitalWrite(redLed, LOW); // Certifica-se de que o LED vermelho está desligado
  delay(200);
  digitalWrite(redLed, HIGH); // Certifica-se de que o LED vermelho está ligado
  delay(200);
  digitalWrite(redLed, LOW); // Certifica-se de que o LED vermelho está desligado
  delay(200);
  digitalWrite(redLed, HIGH); // Certifica-se de que o LED vermelho está ligado
  delay(200);
}

//--------------------------------------------------------------------------//

// Pisca o LED azul 3 vezes para indicar uma exclusão bem-sucedida na EEPROM
void successDelete() {
  digitalWrite(blueLed, LOW); // Certifica-se de que o LED azul está desligado
  digitalWrite(redLed, LOW); // Certifica-se de que o LED vermelho está desligado
  digitalWrite(greenLed, LOW); // Certifica-se de que o LED verde está desligado
  delay(200);
  digitalWrite(blueLed, HIGH); // Certifica-se de que o LED azul está ligado
  delay(200);
  digitalWrite(blueLed, LOW); // Certifica-se de que o LED azul está desligado
  delay(200);
  digitalWrite(blueLed, HIGH); // Certifica-se de que o LED azul está ligado
  delay(200);
  digitalWrite(blueLed, LOW); // Certifica-se de que o LED azul está desligado
  delay(200);
  digitalWrite(blueLed, HIGH); // Certifica-se de que o LED azul está ligado
  delay(200);
}

//--------------------------------------------------------------------------//

// Verifica se o ID passado é o cartão de programação mestre
boolean isMaster(byte test[]) {
if (checkTwo(test, masterCard))
return true;
else
return false;
}

// Monitora o botão de limpar com um determinado intervalo
bool monitorWipeButton(uint32_t interval) {
uint32_t now = (uint32_t)millis();
while ((uint32_t)millis() - now < interval) {
// verifica a cada meio segundo
if (((uint32_t)millis() % 500) == 0) {
if (digitalRead(wipeB) != LOW)
return false;
}
}
return true;
}
