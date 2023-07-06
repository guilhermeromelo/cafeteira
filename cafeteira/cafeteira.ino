//CONFIG ------------------------------------------------------------
const int tamanhoPadrao=1;
const int intensidadePadrao=0;
const int debounceDelay = 1500;
const int debounceLongPressedDelay = 3000;
const int tempoAtualizacaoInfoTela1 = 2000;
const int tempoAtualizacaoInfoTela4 = 4000;
const int tempoPularProximaTela = 5000;
const int tempoTimeoutDesligarResistencia = 40000;

int pesosCafe[2] = {2, 4};

//FIM CONFIG --------------------------------------------------------



//BIBLIOTECAS -------------------------------------------------------
//DISPLAY
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
//SENSOR DE TEMPERATURA 
#include <OneWire.h>
#include <DallasTemperature.h>
//CELULA DE CARGA
#include "HX711.h"
//Carrega a biblioteca Servo
#include "Servo.h"
//Carrega biblioteca sensor ultrassonico
#include "Ultrasonic.h"
//FIM BIBLIOTECAS ---------------------------------------------------



//PINAGEM -----------------------------------------------------------
//CELULA DE CARGA
#define DT A3
#define SCK A2
//SENSOR DE TEMPERATURA
#define pinSensorTemp A0
//RELES
#define releResistencia 8
#define releBombaAgua 9
//SERVO MOTOR DISPENSER CAFE
#define pinServoCafe 10
//sensor ultrassonico
const int echoPin = 11; //PINO DIGITAL UTILIZADO PELO HC-SR04 ECHO(RECEBE)
const int trigPin = 12; //PINO DIGITAL UTILIZADO PELO HC-SR04 TRIG(ENVIA)
//FIM PINAGEM -------------------------------------------------------



//VARIAVEIS GLOBAIS -------------------------------------------------
LiquidCrystal_I2C lcd(0x27,20,4); // DEFINE O ENDEREÇO UTILIZADO PELO ADAPTADOR I2C
OneWire ourWire(pinSensorTemp); //CONFIGURA UMA INSTÂNCIA ONEWIRE PARA SE COMUNICAR COM O SENSOR DE TEMPERATURA
DallasTemperature sensorTemperatura(&ourWire); //BIBLIOTECA DallasTemperature UTILIZA A OneWire
HX711 escala; //CRIA A VARIÁVEL DE ESCALA DE MEDIÇÃO DA CELULA DE CARGA
Servo servo; // Criar um Objeto Servo
float peso = 0.0;
float pesoInicial = 0.0;
float temperatura = 0.0;
int nivelAgua = 0;
float tara = 0;
int multiplicadorTimerResistencia = 0;
int multiplicadorTimerCafe = 0;
Ultrasonic ultrasonic(trigPin,echoPin); //INICIALIZANDO OS PINOS DO ARDUINO
//BOTOES-------------------------------
// 0 - Botao Voltar
// 1 - Botao Seta Esquerda (<---)
// 2 - Botao Seta Direita  (--->)
// 3 - Botao Ok / Confirma
// 4 - Botao Emergencia
const int buttons[5] = {2,3,4,5,6};
bool acaoButtonsState[5] = {false,false,false,false,false};
int debounceArray[5] = {0,0,0,0,0};

bool acaoButtonTararState = false;
int debounceButtonTarar = 0;
//FIM BOTOES --------------------------
unsigned long loopMillis = 0;
unsigned long loopMillisResistencia = 0;
unsigned long loopMillisCafe = 0;
//CONTROLE
int tela = 1; //GUARDA TELA ATUAL
int cursor = 1;
int tamanho = 0; //0 - 50ml / 1 - 100ml / 2 - 200ml
int qtRepeticoesAgua = 0;
int modalidade = 0; //0 - COMUM / 1 - FORTE
int etapaCafe = 0;
int etapaAgua = 0;
//FIM VARIAVEIS GLOBAIS ---------------------------------------------



//FUNCOES TELA ------------------------------------------------------
//TELA 1 ------------------------------
void printHomePage(){
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("NERDPRESSO");
  lcd.setCursor(1, 1);
  lcd.print("PRESSIONE OK PARA");
  lcd.setCursor(1, 2);
  lcd.print("FAZER SEU CAFE...");
  lcd.setCursor(0, 3);
  lcd.print("T   C A   % P      g");
}

void atualizarInfoTela1(){
  lcd.setCursor(13, 3);
  deslocarDireitaPeso(peso*1000);
  lcd.setCursor(1,3);
  deslocarDireitaNumeroInteiro3Caracteres(temperatura);
  lcd.setCursor(7,3);
  deslocarDireitaNumeroInteiro3Caracteres(nivelAgua);
}
//FIM TELA 1 --------------------------
//TELA 2 ------------------------------
void printTela2(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  ESCOLHA SEU CAFE");
  lcd.setCursor(0, 1);
  lcd.print(">TAMANHO=<   > ml");
  lcd.setCursor(0, 2);
  lcd.print(" MODALIDADE=<      >");
  lcd.setCursor(0, 3);
  lcd.print(" INICIAR PREPARO <?>");
  atualizarTamanhoCafe();
  atualizarIntensidade();
}

void atualizarTamanhoCafe(){
  lcd.setCursor(10, 1);
  if(tamanho == 0) lcd.print(" 50");
  else if(tamanho == 1) lcd.print("100");
  else if(tamanho == 2) lcd.print("150");
  //else if(tamanho == 3) lcd.print("300");
}

void atualizarIntensidade(){
  lcd.setCursor(13, 2);
  if(modalidade == 0) lcd.print("C NORM");
  else if(modalidade == 1) lcd.print("C FORT");
  else if(modalidade == 2) lcd.print("A QUEN");
  else if(modalidade == 3) lcd.print("A NATU");
}

void atualizar_cursor_tela2(){
  for(int i=1; i<=3; i++){
    lcd.setCursor(0, i); //LINHA 1
    if(cursor == i) lcd.print(">");
    else lcd.print(" ");
  }
}
//FIM TELA 2 --------------------------
//TELA 3 ------------------------------
void printTela3(){
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("OTIMA ESCOLHA!!");
  lcd.setCursor(0, 1);
  lcd.print("INICIANDO PREPARO...");
  lcd.setCursor(3, 2);
  lcd.print("MUITO OBRIGADO");
  lcd.setCursor(0, 3);
  lcd.print("PELA SUA PREFERENCIA");
}
//FIM TELA 3 --------------------------
//TELA 4 ------------------------------
void printTela4(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PREPARANDO SEU CAFE!");
  lcd.setCursor(0, 1);
  lcd.print("STATUS: ADICIO. CAFE");
  lcd.setCursor(3, 2);
  lcd.print("PESO=       g");
  lcd.setCursor(0, 3);
  lcd.print("T. RESISTENCIA=    C");
}
void atualizarInfoTela4(){
  lcd.setCursor(9, 2);
  deslocarDireitaPeso(peso*1000);
  lcd.setCursor(16,3);
  deslocarDireitaNumeroInteiro3Caracteres(temperatura);
}
//FIM TELA 4 --------------------------
//TELA 5 ------------------------------
void printTela5(){
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("SEU CAFE ESTA PRONTO");
  lcd.setCursor(3, 2);
  lcd.print("VOLTE SEMPRE!!");
}
//FIM TELA 5 --------------------------

void deslocarDireitaNumeroInteiro3Caracteres(float num){
  if(num < 10) lcd.print("  ");
  else if(num < 100) lcd.print(" ");
  
  lcd.print(num,0);
}

void deslocarDireitaPeso(float peso){
  if(peso < -999){
    lcd.print("NEGAT.");
  } else {
    if(peso < -99){
      //nada
    } else if(peso < -9 && peso > -100){
      lcd.print(" ");
    } else if(peso < -0){
      lcd.print("  ");
    } else if(peso < 10){
      lcd.print("   ");
    } else if(peso < 100){
      lcd.print("  ");
    } else if(peso < 1000){
      lcd.print(" ");
    }
    lcd.print(peso,1);
  }
}

void esperarTrocaTela(int proximaTela){
  if(timeout(tempoPularProximaTela) || isButtonPressed(3)){
    mudarTela(proximaTela);
  }
}

void mudarTela(int proximaTela){
  tela = proximaTela;
  if(tela == 1) {
    Serial.println("SEM ESPERA");
    printHomePage();
  } else if (tela == 2) {
    Serial.println("SESCOLHENDO");
    cursor = 1;
    tamanho = tamanhoPadrao;
    modalidade = intensidadePadrao;
    printTela2();
  } else if (tela == 3) {
    Serial.println("SPREPARANDO");
    atualizarValorLoopMillis();
    printTela3();
  } 
  else if (tela == 4) {
    Serial.println("SPREPARANDO");
    etapaCafe = 0;
    etapaAgua = 0;
    printTela4();
  }
  else if (tela == 5) {
    Serial.println("SCAFÉ PRONTO");
    atualizarValorLoopMillis();
    printTela5();
  }
}
//FIM FUNCOES TELA --------------------------------------------------



//FUNCOES BOTOES ----------------------------------------------------
bool isButtonPressed(int num){
  int buttonCurrentState = digitalRead(buttons[num]);
  if(buttonCurrentState == LOW){
    if(debounceArray[num] >= debounceDelay && acaoButtonsState[num] == false){
      debounceArray[num] = 0;
      acaoButtonsState[num] = true;
      return true;
    } else {
      debounceArray[num] += 1;
      return false;
    }
  } else {
    acaoButtonsState[num] = false;
    debounceArray[num] = 0;
    return false;
  }
}

// void desejaTararBalanca(){
//   int buttonCurrentState = digitalRead(buttons[0]);
//   if(buttonCurrentState == LOW){
//     if(debounceButtonTarar >= debounceLongPressedDelay && acaoButtonTararState == false){
//       debounceButtonTarar = 0;
//       acaoButtonTararState = true;
//     } else {
//       debounceButtonTarar += 1;
//       return false;
//     }
//   } else {
//     acaoButtonTararState = false;
//     debounceButtonTarar = 0;
//     return false;
//   }
// }

void monitorarBotoesTela1(){
  if(isButtonPressed(3)){
    mudarTela(2);
  }
  // } else {
  //   desejaTararBalanca();
  // }
}

void monitorarBotoesTela2(){
  if(isButtonPressed(0)){ //BOTAO VOLTAR
    if(cursor == 1){ // O CURSOR ESTA NA PRIMEIRA LINHA
      mudarTela(1);
    } else { // O CURSOR ESTA NA SEGUNDA OU TERCEIRA LINHA
      cursor--;
      atualizar_cursor_tela2();
    }
  } else if(isButtonPressed(1)){ //BOTAO ESQUERDO
    if(cursor == 1){ // O CURSOR ESTA NA PRIMEIRA LINHA - CONFIGURA O TAMANHO DO CAFE
      if(tamanho > 0) tamanho--;
      atualizarTamanhoCafe();
    } else if(cursor ==2){ // O CURSOR ESTA NA SEGUNDA LINHA - CONFIGURA A INTENSIDADE DO CAFE
      if(modalidade > 0) modalidade--; 
      atualizarIntensidade();
    }
  } else if(isButtonPressed(2)){ //BOTAO DIREITO
    if(cursor == 1){ // O CURSOR ESTA NA PRIMEIRA LINHA - CONFIGURA O TAMANHO DO CAFE
      // if(tamanho < 3) tamanho++;
      if(tamanho < 2) tamanho++;
      atualizarTamanhoCafe();
    } else if(cursor == 2){ // O CURSOR ESTA NA SEGUNDA LINHA - CONFIGURA A INTENSIDADE DO CAFE
      if(modalidade < 3) modalidade++;
      atualizarIntensidade();
    }
  } else if(isButtonPressed(3)){ //BOTAO OK
    if(cursor == 3){ // O CURSOR ESTA NA TERCEIRA LINHA - INICIAR PREPARO DO CAFE
      mudarTela(3);
    } else { //  O CURSOR ESTA NA PRIMEIRA OU SEGUNDA LINHA - PULA PARA PROXIMA LINHA
      cursor++;
      atualizar_cursor_tela2();
    }
  }
}
//FIM FUNCOES BOTOES ------------------------------------------------



//FUNCOES SENSORES E ATUADORES --------------------------------------
void atualizarPeso(){
  escala.power_up();
  peso = escala.get_units(3);// - tara;
  escala.power_down();

  if(peso < 0) peso = 0;

  Serial.print("P");
  Serial.println(peso*1000);
}

void atualizarTemperatura(){
  sensorTemperatura.requestTemperatures();//SOLICITA QUE A FUNÇÃO INFORME A TEMPERATURA DO SENSOR
  temperatura = sensorTemperatura.getTempCByIndex(0);
  if(temperatura < 0) temperatura = 0;
  if(temperatura > 140) temperatura = 140;

  Serial.print("T");
  Serial.println(temperatura);
}

void atualizarNivelAgua(){
  //12,00 - vazio
  //2,81 - cheio
  float cmMsec;
  long microsec = ultrasonic.timing();
  cmMsec = ultrasonic.convert(microsec, Ultrasonic::CM);
  Serial.print("distancia ");
  Serial.println(cmMsec);
  nivelAgua = map(cmMsec,4.0,11.00,100,0);

  if(nivelAgua > 100) nivelAgua = 100;
  if(nivelAgua < 0) nivelAgua = 0;

  Serial.print("N");
  Serial.println(nivelAgua);
}

void switchServoCafe(bool ligar){
  if(ligar == true)
    servo.write(180);
  else
    servo.write(90);
    
    Serial.print("C");
    Serial.println(ligar);
}

void switchResistencia(bool ligar){
  Serial.print("Resistência: ");
  Serial.println(ligar);
  if(ligar == true)
    digitalWrite(releResistencia, HIGH);
  else
    digitalWrite(releResistencia, LOW);
    
    Serial.print("R");
    Serial.println(ligar);
}

void switchBombaAgua(bool ligar){
  Serial.print("Bomba: ");
  Serial.println(ligar);
  if(ligar == true)
    digitalWrite(releBombaAgua, HIGH);
  else
    digitalWrite(releBombaAgua, LOW);

    
    Serial.print("A");
    Serial.println(ligar);
}
//FIM FUNCOES SENSORES E ATUADORES --------------------------------------

//FABRICACAO CAFE ---------------------------------------------------
void fabricarCafe(){
  if(etapaCafe == 0){ //INICIAR DESPEJAR CAFE
    switchServoCafe(true);
    atualizarValorLoopMillisCafe();
    if(tamanho == 0) qtRepeticoesAgua = 1; //50ml
    if(tamanho == 1) qtRepeticoesAgua = 2; //100ml
    if(tamanho == 2) qtRepeticoesAgua = 3; //150ml
    pesoInicial = peso;

    if(modalidade == 0) multiplicadorTimerCafe = 1;
    if(modalidade == 1) multiplicadorTimerCafe = 2;
    etapaCafe++;
  } else if(etapaCafe == 1){ //COLOCANDO CAFE
    if(timeoutCafe(5000)){
      if(multiplicadorTimerCafe <= 0){
          if(colocouCafeSuficiente()){
            switchServoCafe(false);
            switchResistencia(true);
            etapaCafe++;
            atualizarValorLoopMillisResistencia();
            carregarMultiplicadorTimerResistencia(3);
        }
      } else {
        multiplicadorTimerCafe--;
        atualizarValorLoopMillisCafe();
      }
    }
  } else if(etapaCafe == 2){ //AQUECENDO RESISTENCIA
    if(timeoutResistencia(10000) /*temperatura >= 125*/){
      if(multiplicadorTimerResistencia == 0){
        pesoInicial = peso;
        switchBombaAgua(true);
        etapaCafe++;
      } else {
        multiplicadorTimerResistencia--;
        atualizarValorLoopMillisResistencia();
      }
    }
  } else if(etapaCafe == 3){ //COLOCANDO AGUA
    if(colocouTodaAgua(qtRepeticoesAgua)){//VOU COLOCAR DE 50 em 50ml
      switchBombaAgua(false);
      //qtRepeticoesAgua--;
      //atualizarValorLoopMillisResistencia();
      //carregarMultiplicadorTimerResistencia(3);
      //etapa = 2;
      // if(qtRepeticoesAgua == 0){
       
      // }

      //switchBombaAgua(false);
      switchResistencia(false);
      mudarTela(5);
    }
  }
}

void despejarAgua(){
  if(etapaAgua == 0){//LIGAR RESISTENCIA
    if(tamanho == 0) qtRepeticoesAgua = 1; //50ml
    if(tamanho == 1) qtRepeticoesAgua = 2; //100ml
    if(tamanho == 2) qtRepeticoesAgua = 3; //150ml

    if(modalidade == 2){//AGUA QUENTE
      switchResistencia(true);
      atualizarValorLoopMillisResistencia();
      carregarMultiplicadorTimerResistencia(3);
      etapaCafe = 2;
      etapaAgua++;
    } else { // AGUA NATU
      etapaAgua = 2;
      etapaCafe = 3;
      pesoInicial = peso;
      switchBombaAgua(true);
    }
  } else if(etapaAgua == 1){//OPCAO SELECIONADA AGUA QUENTE
    fabricarCafe();
  } else if(etapaAgua == 2){//OPCAO SELECIONADA AGUA NATURAL
    fabricarCafe();
  }
}

bool colocouCafeSuficiente(){
  return (peso - pesoInicial) * 1000 >= pesosCafe[modalidade];
}

bool colocouTodaAgua(int multiplicador){
  return (peso - pesoInicial) * 1000 >= (30 * multiplicador);
}
//FIM FABRICACAO CAFE -----------------------------------------------

bool timeout(int tempoEspera){
  return (loopMillis == 0 || ((millis() - loopMillis) > tempoEspera));
}

bool timeoutResistencia(int tempoEspera){
  return (loopMillisResistencia == 0 || ((millis() - loopMillisResistencia) > tempoEspera));
}

bool timeoutCafe(int tempoEspera){
  return (loopMillisCafe == 0 || ((millis() - loopMillisCafe) > tempoEspera));
}

void atualizarValorLoopMillis(){
  loopMillis = millis();
}

void atualizarValorLoopMillisResistencia(){
  loopMillisResistencia = millis();
}

void atualizarValorLoopMillisCafe(){
  loopMillisCafe = millis();
}

void carregarMultiplicadorTimerResistencia (int n){
  multiplicadorTimerResistencia = n;
}

void escutarPreparoSupervisorio() {
  if (Serial.available() > 0) {
    String line = Serial.readStringUntil('\n');

    line.trim();

    if (line.length() == 2) {
      tamanho = line.charAt(0) - '0';
      modalidade = line.charAt(1) - '0';

      mudarTela(3);
    }
  }
}

void setup() {
  Serial.begin(9600); //INICIALIZA A SERIAL

  //PINAGEM
  for(int i=0;i<5;i++){
    pinMode(buttons[i],INPUT_PULLUP);
  }

  pinMode(releBombaAgua, OUTPUT);
  pinMode(releResistencia, OUTPUT);
  pinMode(echoPin, INPUT); //DEFINE O PINO COMO ENTRADA (RECEBE)
  pinMode(trigPin, OUTPUT); //DEFINE O PINO COMO SAIDA (ENVIA)

  //ASSOCIAR PORTA DO SERVO A VARIAVEL SERVO
  servo.attach(pinServoCafe);

  //PREPARACAO CELULA DE CARGA
  escala.begin (DT, SCK); //INICIA CONTROLE CELULA DE CARGA
  sensorTemperatura.begin(); //INICIA O SENSOR
  delay(1000); //INTERVALO DE 1 SEGUNDO
  Serial.println(escala.read());   // Aguada até o dispositivo estar pronto
  escala.set_scale(-1810879);     // Substituir o valor encontrado para escala
  escala.tare();
  escala.power_up();

  //Inicializa o LCD e o backlight
  lcd.init();
  lcd.backlight();
  mudarTela(1);
}
 
void loop() {
  if(tela == 4){
    atualizarPeso();
    if(timeout(tempoAtualizacaoInfoTela4)){
      atualizarTemperatura();
      atualizarValorLoopMillis();
      atualizarInfoTela4();
    }
    if(modalidade == 0 || modalidade == 1) fabricarCafe();
    else despejarAgua();

  } else {
    //MONITORAR SENSORES E MOSTRAR NO DISPLAY - DEPENDE DO TEMPO DE ATUALIZAÇÃO
    if(tela == 1 && timeout(tempoAtualizacaoInfoTela1)){
      escutarPreparoSupervisorio();
      atualizarValorLoopMillis();
      atualizarPeso();
      atualizarTemperatura();
      atualizarNivelAgua();
      atualizarInfoTela1();
    }

    //MONITORAR BOTOES E REAGIR
    if(tela == 1) monitorarBotoesTela1(); //HOME PAGE
    else if(tela == 2) monitorarBotoesTela2(); //SELECIONAR CAFE
    else if(tela == 3) esperarTrocaTela(4); //INICIANDO PREPARO CAFE
    else if(tela == 5) esperarTrocaTela(1); //CAFE PRONTO
  }
}
