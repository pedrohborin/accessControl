/*
.    ____________________________________________________________________________________________________________________________________
.
.    PROJETO: Abertura de uma fechadura elétrica por meio de aproximação RFID passivo.
.    ____________________________________________________________________________________________________________________________________
.   
.    Trabalho de Conclusão de Curso para a Faculdade FATECs (UniCEUB)
.    ____________________________________________________________________________________________________________________________________
.
.    Este código é um exemplo de controle de acesso utilizando a biblioteca MFRC522.h, ESP8266, RFID-RC522, relé e uma Fechadura Elétrica
.
.    Fluxo de trabalho:
.
.                                                +-----------+
.                                                |  LER TAG  |
.                                                +-----+-----+
.                                                      |
.                                                      |
.                                +---------------------+-----------------------------------+
.                                |                                                         |
.                        +-------v------+                                          +-------v-------+
.                        |  TAG MESTRE  |                                          |  OUTRAS TAGS  |
.                        +-------+------+                                          +-------+-------+
.                                |                                                         |
.                                |                                                         |
.                          +-----v-----+                                       +-----------+-----------+
.                          |  LER TAG  |                                       |                       |
.                          +-----+-----+                                       |                       |
.                                |                                    +--------v--------+    +---------v----------+
.                                |                                    |  TAG CONHECIDA  |    |  TAG DESCONHECIDA  |
.            +-------------------+---------------------+              +--------+--------+    +---------+----------+
.            |                   |                     |                       |                       |
.            |                   |                     |                       |                       |
.    +-------v------+   +--------v--------+  +---------v----------+     +------v------+           +----v----+
.    |  TAG MESTRE  |   |  TAG CONHECIDA  |  |  TAG DESCONHECIDA  |     |  AUTORIZAR  |           |  NEGAR  |
.    +-------+------+   +--------+--------+  +---------+----------+     +-------------+           +---------+
.            |                   |                     |
.            |                   |                     |
.     +------v-----+       +-----v-----+        +------v------+
.     |  CANCELAR  |       |  EXCLUIR  |        |  ADICIONAR  |
.     +------------+       +-----------+        +-------------+
.
.
.    Toda vez que o código chegar na base do Fluxograma, ele irá armar o sistema e retornar para a primeira etapa
.
.
.    >>>>>>>>>>>   FUNCIONAMENTO   <<<<<<<<<<<
.
.    O primeiro cartão lido será o cartão mestre. Ele irá servir para cadastrar e excluir os cartões de acesso.
.    Caso deseje trocar o cartão mestre, será necessário reiniciar o ESP8266 pelo botão RST.
.
.
.    >>>>>>>>>>>   INTERFACE INTUITIVA    <<<<<<<<<<<
.
.    O sistema foi arquitetado de modo que o usuário consiga usá-lo de forma intuitiva.
.
.    - ABERTURA - Para abrir a fechadura, basta aproximar um cartão cadastrado no leitor RFID e o mesmo já irá garantir o acesso acionando a fechadura elétrica.
.
.    - GERENCIAMENTO - Para cadastrar/excluir um cartão, basta aproximar o cartão mestre no leitor e esperar a indicação de luzes para aproximar o cartão desejado.
.    O próprio sistema identificará aquele cartão e determinará se é necessário excluí-lo ou cadastrá-lo. O usuário fica livre de qualquer burocracia envolvida no gerenciamento dos cartões.
.
.
.    >>>>>>>>>>>   SEGURANÇA   <<<<<<<<<<<
.
.    O sistema se mostra seguro pois se é utilizado a UID para armazenar o cadastro das TAGs RFID.
.    Considerando o baixo número para limite de cartões registrados (apenas 5), se torna difícil qualquer tipo de tentativa de força bruta, além da dificuldade de clonagem do UID.
.
*/

#include <SPI.h>        // Protocolo SPI utilizado pelo módulo RC522
#include <MFRC522.h>    // Biblioteca para Dispositivos Mifare RC522

// Definição dos pinos do ESP8266 NodeMCU v3 para maior facilidade no código
#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15

// Definição dos pinos e variáveis de código
#define SS_PIN D4     // Pino SDA do RFID-RC522
#define RST_PIN D3    // Pino RST do RFID-RC522
#define LED_B D0      // Pino LED Azul
#define LED_G D1      // Pino LED Verde
#define LED_R D2      // Pino LED Vermelho
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Cria uma instância do MFRC522
#define relay D8      // Pino relé

// Definição de variáveis de controle
char cartaoMestre[] = "00000000000";  // Cria uma variável do tipo char array para armazenar o cartão mestre
char vazio[] = "00000000000";         // Variável do tipo char array para comparar se o cartão mestre está vazio
int temMestre;                        // Variável para marcar a existência de um cartão mestre
int admin;                            // Variável para marcar a funcionalidade de Administrador ativo
char cartoes[5][12];                  // Cria um array bi-dimensional para armazenar até 5 cartões de acesso


// ------------------------------------------------------------
// ---------------------    VOID SETUP    ---------------------
// ------------------------------------------------------------

void setup() 
{
  Serial.begin(9600);       // Inicia a porta serial na taxa de dados de 9600bps
  SPI.begin();              // Inicia a SPI bus
  mfrc522.PCD_Init();       // Inicia o MFRC522
  pinMode(relay, OUTPUT);   // Define o relé
  pinMode(LED_B, OUTPUT);   // Define o LED azul
  pinMode(LED_G, OUTPUT);   // Define o LED verde
  pinMode(LED_R, OUTPUT);   // Define o LED vermelho

  // Primeira mensagem a ser printada no serial quando o sistema for ativado
  Serial.println();
  Serial.println("  ____________________________________  ");
  Serial.println(" |                                    | ");
  Serial.println(" |                                    | ");
  Serial.println(" |     APROXIME UM CARTÃO MESTRE      | ");
  Serial.println(" |                                    | ");
  Serial.println(" |____________________________________| ");

  // Inicia o sistema sem um cartão mestre e sem o modo Administrador
  temMestre = 0;
  admin = 0;

}


// ------------------------------------------------------------
// -------------------  FUNÇÕES DECLARADAS --------------------
// ------------------------------------------------------------

// Sequência de LEDs RED GREEN BLUE
void lendoCartao(){
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, LOW);
  digitalWrite(LED_R, HIGH);
  delay(150);
  digitalWrite(LED_R, LOW);
  delay(150);
  digitalWrite(LED_R, HIGH);
  delay(150);
  digitalWrite(LED_R, LOW);
  delay(150);
  digitalWrite(LED_R, HIGH);
  delay(150);
  digitalWrite(LED_R, LOW);
  delay(150);
  digitalWrite(LED_R, HIGH);
  delay(150);
  digitalWrite(LED_R, LOW);
}

// Liga LED verde
void verde(){
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_G, HIGH);
  digitalWrite(LED_B, LOW);
}

// Liga LED vermelho
void vermelho(){
  digitalWrite(LED_R, HIGH);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, LOW);
}

// Liga LED azul
void azul(){
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, HIGH);
}

// LEDs representando a espera de um cartão mestre para cadastrar
void esperandoMestre(){
  digitalWrite(LED_R, HIGH);
  delay(300);
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_B, HIGH);
  delay(300);
  digitalWrite(LED_B, LOW);
}

// LEDs representando o cadastro de um cartão mestre
void mestreRegistrado(){
  digitalWrite(LED_B, HIGH);
  digitalWrite(LED_R, HIGH);
  digitalWrite(LED_G, LOW);
  delay(300);
  digitalWrite(LED_G, HIGH);
  delay(300);
  digitalWrite(LED_G, LOW);
  delay(300);
  digitalWrite(LED_G, HIGH);
  delay(300);
  digitalWrite(LED_G, LOW);
  delay(300);
  digitalWrite(LED_G, HIGH);
  delay(300);
  digitalWrite(LED_B, LOW);
  digitalWrite(LED_R, LOW);
}

// LEDs representando o acesso autorizado à fechadura
void acessoAutorizado(){
  digitalWrite(LED_G, HIGH);
  digitalWrite(LED_B, HIGH);
  digitalWrite(LED_R, LOW);
  delay(1500);
  digitalWrite(LED_B, LOW);
}

// LEDs representando o acesso negado à fechadura
void acessoNegado(){
  digitalWrite(LED_G, HIGH);
  digitalWrite(LED_B, LOW);
  digitalWrite(LED_R, HIGH);
  delay(1500);
  digitalWrite(LED_R, LOW);
}

// LEDs representando a espera de um cartão se aproximar
void esperandoCartao(){
  digitalWrite(LED_R, HIGH);
  digitalWrite(LED_B, LOW);
  digitalWrite(LED_G, HIGH);
  delay(300);
  digitalWrite(LED_G, LOW);
  delay(300);
}

// LEDs representando o sistema calculando a possibilidade de uma adição ou exclusão de um cartão
void tarefas(){
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, LOW);
  delay(300);
  digitalWrite(LED_G, HIGH);
  delay(300);
  digitalWrite(LED_G, LOW);
  delay(300);
  digitalWrite(LED_G, HIGH);
  delay(300);
  digitalWrite(LED_G, LOW);
  delay(300);
  digitalWrite(LED_G, HIGH);
  delay(300);
  digitalWrite(LED_G, LOW);
  delay(300);
}

// LEDs representando que o número de cartões cadastrados está no seu limite
void cartoesCheio(){
  digitalWrite(LED_B, HIGH);
  digitalWrite(LED_G, HIGH);
  digitalWrite(LED_R, HIGH);
  delay(1500);
}

// LEDs representando a exclusão de um cartão de acesso
void cartaoExcluido(){
  digitalWrite(LED_B, LOW);
  digitalWrite(LED_G, HIGH);
  digitalWrite(LED_R, HIGH);
  delay(300);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_R, LOW);
  delay(300);
  digitalWrite(LED_G, HIGH);
  digitalWrite(LED_R, HIGH);
  delay(300);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_R, LOW);
  delay(300);
  digitalWrite(LED_G, HIGH);
  digitalWrite(LED_R, HIGH);
  delay(300);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_R, LOW);
  delay(300);
}

// LEDs representando o cadastro de um cartão de acesso
void cartaoCadastrado(){
  digitalWrite(LED_B, HIGH);
  digitalWrite(LED_G, HIGH);
  digitalWrite(LED_R, LOW);
  delay(300);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, LOW);
  delay(300);
  digitalWrite(LED_G, HIGH);
  digitalWrite(LED_B, HIGH);
  delay(300);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, LOW);
  delay(300);
  digitalWrite(LED_G, HIGH);
  digitalWrite(LED_B, HIGH);
  delay(300);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, LOW);
  delay(300);
}

// LEDs representando o cancelamento de uma operação de adição/exclusão de cartão de acesso
void cancelarTarefa(){
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, LOW);
  delay(300);
  digitalWrite(LED_R, HIGH);
  delay(150);
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_G, HIGH);
  delay(150);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, HIGH);
  delay(150);
  digitalWrite(LED_B, LOW);
}

// Função para printar no serial o aviso do sistema sendo armado novamente
void sistemaArmado(){
  Serial.println("  _________________________________ ");
  Serial.println(" |                                 |");
  Serial.println(" |         SISTEMA ARMADO          |");
  Serial.println(" |_________________________________|");
  Serial.println();
}



// ------------------------------------------------------------
// ---------------------    VOID LOOP    ----------------------
// ------------------------------------------------------------

void loop() 
{

  // ------------------------------------------------------------
  // --------------------  SISTEMA INICIADO  --------------------
  // ------------------------------------------------------------

  // Aciona o LED verde representando o funcionamento do sistema
  verde();



  // ------------------------------------------------------------
  // -------  VERIFICA A EXISTÊNCIA DE UM CARTÃO MESTRE  --------
  // ------------------------------------------------------------

  // Verifica se existe um cartão mestre
  // Caso exista, 'temMestre' recebe 1, caso não exista, recebe 0
  temMestre = 0;
  for(int j=0;j<11;j++)
  {
    if(cartaoMestre[j] != vazio[j])
    {
      temMestre = 1;
      break;
    }
  }

  // Caso não tenha um cartão mestre, o sistema acionará os LEDs
  // representando a espera de um cartão mestre para cadastro
  if(!temMestre)
  {
    esperandoMestre();
  }



  // ------------------------------------------------------------
  // ------------------  LÊ CARTÕES PRÓXIMOS  -------------------
  // ------------------------------------------------------------

  // Procura por novos cartões
  if (!mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }

  // Se tiver um novo cartão, então ele será lido
  if (!mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }



  // ------------------------------------------------------------
  // -----------------  ADQUIRE DADOS DO CARTÃO  ----------------
  // ------------------------------------------------------------

  // -------- FUNÇÃO PARA VERIFICAR CARTÕES REGISTRADOS ---------
  // --- REMOVA O COMENTÁRIO PARA ACESSAR ESSAS INFORMAÇÕES -----

  /*
  // Percorrer o array e imprimir cada cartão
  Serial.println("Cartoes registrados:");
  for (int i = 0; i < 5; i++)
  {
      if (strcmp(cartoes[i], "") != 0)
      {
          Serial.print(" - ");
          Serial.println(cartoes[i]);
      }
  }
  Serial.println();
  */

  // Lê o UID da TAG e printa no serial
  Serial.println();
  Serial.println("  ____________________________________  ");
  Serial.println(" |                                    | ");
  Serial.println(" |  CARTÃO DE APROXIMAÇÃO DETECTADO   | ");
  Serial.print(" |       UID TAG :");
  String content= "";
  byte letter;
  // Loop para adquirir a UID do cartão aproximado
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println("        | ");
  Serial.println(" |____________________________________| ");
  Serial.println();
  content.toUpperCase();

  // Armazena a UID em um char array Buf[]
  // Essa operação é necessária para a transformação de uma String em um char array
  // Com isso podemos trabalhar com a variável utilizando operações próprias para um char array
  String stringOne = content.substring(1);
  char Buf[12];
  stringOne.toCharArray(Buf, 12);



  // ------------------------------------------------------------
  // -------------------  REGISTRA O MESTRE  --------------------
  // ------------------------------------------------------------

  // Print para representar a mensagem a ser mostrada no Serial Monitor
  Serial.print(" Mensagem : ");
  
  // Se não existir um cartão mestre, registrará o cartão aproximado como cartão mestre
  // e retornará o código para o começo do loop
  if(!temMestre)
  {
    for(int i=0;i<12;i++)
    {
      cartaoMestre[i] = Buf[i];
    }

    Serial.println("Cartão mestre registrado com sucesso.");
    Serial.print(" UID TAG : ");
    Serial.println(content.substring(1));
    mestreRegistrado();
    Serial.println();
    sistemaArmado();
    return;
  }

  // Checa se é o cartão mestre sendo aproximado
  // Se for, 'admin' recebe 1, caso não, recebe 0
  admin = 1;
  for(int j=0;j<11;j++){
    if(cartaoMestre[j] != Buf[j]){
      admin = 0;
      break;
    }
  }

  // Serial Print e LEDs visuais para mostrar a detecção ou não do cartão mestre
  if(admin){
    Serial.println("Cartão mestre detectado");
    Serial.println();
    Serial.println("  _________________________________ ");
    Serial.println(" |                                 |");
    Serial.println(" |   Acionado modo Administrador   |");
    Serial.println(" |_________________________________|");
    Serial.println();
    lendoCartao();
    digitalWrite(LED_R, HIGH);
    delay(2000);
  }else{
    Serial.println("Cartão de acesso detectado");
  }



  // ------------------------------------------------------------
  // -----------------  ABA PARA CARTÃO COMUM  ------------------
  // ------------------------------------------------------------

  // Caso não seja o cartão administrador, o sistema irá validar
  // Se o cartão aproximado tem acesso para abrir a fechadura
  if(!admin)
  {
    // Checa se o cartão é válido
    // caso seja, 'pass' recebe 1, caso não, recebe 0
    int k; int pass = 0;
    for (int k = 0; k < 5; k++)
    {
        if (strcmp(Buf, cartoes[k]) == 0)
        {
            pass = 1;
            break;
        }
    }

    // Print de ação tomada
    Serial.print(" Operação : ");

    // Se o cartão for válido, abre a fechadura e retorna
    if(pass)
    {
      Serial.println("Acesso AUTORIZADO");
      Serial.println();
      digitalWrite(relay, HIGH);
      acessoAutorizado();
      digitalWrite(relay, LOW);
    }
    else // caso não seja válido, nega o acesso e retorna
    {
      Serial.println("Acesso NEGADO");
      Serial.println();
      acessoNegado();
    }
    sistemaArmado();
    return;
  }



  // ------------------------------------------------------------
  // ----------------  ABA PARA CARTÃO MESTRE  ------------------
  // ------------------------------------------------------------

  // Se o cartão aproximado for detectado como o cartão mestre,
  // entramos no modo "Administrador", onde é possível adicionar/excluir
  // até 5 cartões de acesso
  if(admin)
  {
    char registro[12];    // char array para armazenar o cartão que será lido
    int task = 0;         // variável de auxílio para sair do while()

    // Mensagem de operação
    Serial.println(" Mensagem : Aproxime um cartão para Cadastro/Exclusão");

    // Enquanto não for detectado um cartão RFID, 'task' permanecerá 0
    while(task == 0)
    {
      // Procura por novos cartões
      if(!mfrc522.PICC_IsNewCardPresent())
      {
        esperandoCartao();
      }else
      {
        // Se tiver um novo cartão, então ele será lido
        if(mfrc522.PICC_ReadCardSerial())
        {

          // Lê o UID da TAG e printa no serial
          Serial.println();
          Serial.println(" Aviso : Cartão para registro detectado");
          lendoCartao();
          Serial.print(" UID tag :");
          String content= "";
          byte letter;
          // Loop para adquirir a UID do cartão aproximado
          for (byte i = 0; i < mfrc522.uid.size; i++) 
          {
            Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
            Serial.print(mfrc522.uid.uidByte[i], HEX);
            content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
            content.concat(String(mfrc522.uid.uidByte[i], HEX));
          }
          Serial.println();Serial.println();
          Serial.print(" Mensagem : ");
          content.toUpperCase();

          // Armazena a String UID no char array registro[]
          String stringOne = content.substring(1);
          stringOne.toCharArray(registro, 12);
          tarefas();
          task = 1; // saída do While()
        }
      }
    }

    // Checa se é o cartão mestre sendo aproximado
    // Se for, 'cartaoAdmin' recebe 1, caso não, recebe 0
    int cartaoAdmin = 1;
    for(int j=0;j<11;j++)
    {
      if(cartaoMestre[j] != registro[j])
      {
        cartaoAdmin = 0;
        break;
      }
    }

    // Caso seja o cartão mestre, cancela a ação e retorna para o void loop()
    if(cartaoAdmin)
    {
      Serial.println("Cancelando ação...");
      Serial.println();
      cancelarTarefa();
      sistemaArmado();
      return;
    }

    // Checa se o array de cartoes está cheio
    // Se cartoes_cheios chegar a 5, indica que está cheio
    int m;
    int cartoes_cheios = 0;
    for(m = 0; m < 5; m++)
    {
      if(strcmp(cartoes[m], "") != 0)
      {
        cartoes_cheios++;
      }
    }

    // Caso não tenha espaço no array, aciona o aviso e retorna
    if(cartoes_cheios == 5)
    {
      // Aviso de lotação e retorna
      Serial.println("Armazém de cartões cheio! Cancelando ação...");
      Serial.println();
      cartoesCheio();
      sistemaArmado();
      return;
    }
    // Caso tenha espaço, prosseguir para exclusão/adição
    else
    {
      // Loop para procurar existência do cartão no array de cartoes[]
      // Percorrer todo o array para verificar se o cartão já existe
      // Caso exista um cartão, o mesmo será excluído
      for (int c = 0; c < 5; c++)
      {
        // Entrada para exclusão de cartão existente
        if (strcmp(registro, cartoes[c]) == 0) {
            strcpy(cartoes[c], "");
            Serial.print(" Cartão [ ");
            Serial.print(registro);
            Serial.println(" ] excluído com sucesso.");
            Serial.println();
            cartaoExcluido();
            sistemaArmado();
            return;
          }
      }
      // Se não houver cartão duplicado, procura uma posição vazia para adicionar o novo cartão
      for (int c = 0; c < 5; c++)
      {
        // Entrada para adição de cartão novo
        if (strcmp(cartoes[c], "") == 0)
        {
              strcpy(cartoes[c], registro);
              Serial.print("Cartão [ ");
              Serial.print(registro);
              Serial.println(" ] cadastrado com sucesso.");
              Serial.println();
              cartaoCadastrado();
              sistemaArmado();
              return;
        }
      }
      return; // após a adição/exclusão, retorna
    }
  }
}
