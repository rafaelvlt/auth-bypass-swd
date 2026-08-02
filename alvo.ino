#include "HardwareSerial.h" // Inclui a definição da classe Serial
#define BAUDRATE 9600


// Define o tamanho máximo do buffer de entrada para a senha
const byte bufsiz = 32;
char buf[bufsiz];
boolean new_data = false;
boolean start = true;

// --- FUNÇÃO PRINCIPAL DE INICIALIZAÇÃO ---
void setup() {
   // Garante que o pino PC13 (o LED da placa) está configurado como saída
   pinMode(PC13, OUTPUT);
  
   // Espera 3 segundos para que o OpenOCD/GDB possa se conectar durante o boot
   delay(3000);
  
   // Inicia a porta Serial (UART) na velocidade de 9600 baud.
   Serial.begin(BAUDRATE);
}

// --- FUNÇÃO DE LOOP PRINCIPAL ---
void loop() {
   // Controle para imprimir "Login: " apenas uma vez no início
   if (start == true) {
       Serial.print("Login: ");
       start = false;
   }
  
   recv_data();
  
   // Se recebeu dados, chama a rotina de validação
   if (new_data == true)
       validate();
}

// --- FUNÇÃO PARA RECEBER DADOS DA PORTA SERIAL (UART) ---
void recv_data() {
 static byte i = 0;
   static char last_char;
   char end1 = '\n';
   char end2 = '\r';
   char rc;

   // Loop para ler todos os caracteres disponíveis na porta serial
   while (Serial.available() > 0 && new_data == false) {
       rc = Serial.read();
      
       // Evita ler caracteres duplicados de fim de linha (como \r\n)
       if ((rc == end1 || rc == end2) && (last_char == end2 || last_char == end1))
           return;
          
       last_char = rc;

       if (rc != end1 && rc != end2) {
           // Armazena o caractere no buffer
           buf[i++] = rc;
          
           // Proteção contra estouro de buffer (limita ao tamanho máximo)
           if (i >= bufsiz)
               i = bufsiz - 1;
       } else {
           // Caractere de fim de linha recebido: Termina a string e seta a flag de dados
           buf[i] = '\0'; // Termina a string
           i = 0;
           new_data = true;
       }
   }
}

// --- FUNÇÃO PARA VALIDAR A SENHA ---
void validate() {
   Serial.println(buf);
   new_data = false;
  
   // Ponto crucial de verificação: Onde o hack será executado
   if (strcmp(buf, "LSEC1337") == 0) // <--- O debugger GDB vai atacar esta linha
       Serial.println("ACCESS GRANTED");
   else {
       Serial.println("Access Denied.");
       Serial.print("Login: ");
   }
}
