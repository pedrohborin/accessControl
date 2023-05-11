# Controle de Acesso RFID

### Trabalho de Conclusão de Curso FATECs (UniCEUB)

Este código é um exemplo de controle de acesso utilizando a biblioteca MFRC522.h, ESP8266, RFID-RC522, relé e uma Fechadura Elétrica.

## Fluxo de trabalho:

                                                  +-----------+
                                                  |  LER TAG  |
                                                  +-----+-----+
                                                        |
                                                        |
                                  +---------------------+-----------------------------------+
                                  |                                                         |
                          +-------v------+                                          +-------v-------+
                          |  TAG MESTRE  |                                          |  OUTRAS TAGS  |
                          +-------+------+                                          +-------+-------+
                                  |                                                         |
                                  |                                                         |
                            +-----v-----+                                       +-----------+-----------+
                            |  LER TAG  |                                       |                       |
                            +-----+-----+                                       |                       |
                                  |                                    +--------v--------+    +---------v----------+
                                  |                                    |  TAG CONHECIDA  |    |  TAG DESCONHECIDA  |
              +-------------------+---------------------+              +--------+--------+    +---------+----------+
              |                   |                     |                       |                       |
              |                   |                     |                       |                       |
      +-------v------+   +--------v--------+  +---------v----------+     +------v------+           +----v----+
      |  TAG MESTRE  |   |  TAG CONHECIDA  |  |  TAG DESCONHECIDA  |     |  AUTORIZAR  |           |  NEGAR  |
      +-------+------+   +--------+--------+  +---------+----------+     +-------------+           +---------+
              |                   |                     |
              |                   |                     |
       +------v-----+       +-----v-----+        +------v------+
       |  CANCELAR  |       |  EXCLUIR  |        |  ADICIONAR  |
       +------------+       +-----------+        +-------------+
Toda vez que o código chegar na base do Fluxograma, ele irá armar o sistema e retornar para a primeira etapa

## Funcionamento

O sistema foi arquitetado de modo que o usuário consiga usá-lo de forma intuitiva.

- ABERTURA - Para abrir a fechadura, basta aproximar um cartão cadastrado no leitor RFID e o mesmo já irá garantir o acesso acionando a fechadura elétrica.

- GERENCIAMENTO - Para cadastrar/excluir um cartão, basta aproximar o cartão mestre no leitor e esperar a indicação de luzes para aproximar o cartão desejado. O próprio sistema identificará aquele cartão e determinará se é necessário excluí-lo ou cadastrá-lo. O usuário fica livre de qualquer burocracia envolvida no gerenciamento dos cartões.

## Segurança

O sistema se mostra seguro porque utiliza o UID para armazenar o cadastro das TAGs RFID. Considerando o baixo número para limite de cartões registrados (apenas 5), se torna difícil qualquer tipo de tentativa de força bruta, além da dificuldade de clonagem do UID.
