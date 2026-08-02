
#define RXD2 16 // GPIO 16 (Pino de Recepção da ponte)
#define TXD2 17 // GPIO 17 (Pino de Transmissão da ponte)
#define LED_BUILTIN 2
#define BAUDRATE_ALVO 9600

void setup() {

 Serial.begin(9600);
 // 2. Inicia a comunicação com o dispositivo-alvo (Serial2)
 // 8N1 é o formato padrão (8 bits de dados, sem paridade, 1 stop bit).
 Serial2.begin(BAUDRATE_ALVO, SERIAL_8N1, RXD2, TXD2);
  Serial.println(">>> Ponte Serial (UART) 3.3V Pronta <<<");
 Serial.print("Velocidade do Alvo (BAUDRATE_ALVO) configurada para: ");
 Serial.println(BAUDRATE_ALVO);
}

void loop() {
 // Se o PC (via USB) enviar dados, repasse-os para o Alvo (Serial2)
 if (Serial.available()) {
   Serial2.write(Serial.read());
 }

 // Se o Alvo (Serial2) enviar dados, repasse-os para o PC (Serial)
 if (Serial2.available()) {
   Serial.write(Serial2.read());
 }
}