//CONFIG ------------------------------------------------------------
const int tamanhoPadrao=1;
const int intensidadePadrao=0;
const int debounceDelay = 1500;
const int tempoAtualizacaoInfoTela1 = 2000;
const int tempoAtualizacaoInfoTela4 = 1000;
const int tempoPularProximaTela = 5000;
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
//FIM PINAGEM -------------------------------------------------------



//VARIAVEIS GLOBAIS -------------------------------------------------
LiquidCrystal_I2C lcd(0x27,20,4); // DEFINE O ENDEREÇO UTILIZADO PELO ADAPTADOR I2C
OneWire ourWire(pinSensorTemp); //CONFIGURA UMA INSTÂNCIA ONEWIRE PARA SE COMUNICAR COM O SENSOR DE TEMPERATURA
DallasTemperature sensorTemperatura(&ourWire); //BIBLIOTECA DallasTemperature UTILIZA A OneWire
HX711 escala; //CRIA A VARIÁVEL DE ESCALA DE MEDIÇÃO DA CELULA DE CARGA
Servo servo; // Criar um Objeto Servo
float peso = 0.0;
float temperatura = 0.0;
//BOTOES-------------------------------
// 0 - Botao Voltar
// 1 - Botao Seta Esquerda (<---)
// 2 - Botao Seta Direita  (--->)
// 3 - Botao Ok / Confirma
// 4 - Botao Emergencia
const int buttons[5] = {2,3,4,5,6};
int lastButtonsState[5] = {LOW,LOW,LOW,LOW,LOW};
bool acaoButtonsState[5] = {false,false,false,false,false};
int debounceArray[5] = {0,0,0,0,0};
//FIM BOTOES --------------------------
unsigned long loopMillis = 0;
//CONTROLE
int tela = 1; //GUARDA TELA ATUAL
int cursor = 1;
int tamanho = 0; //0 - 50ml / 1 - 100ml / 2 - 200ml / 3 - 300ml
int intensidade = 0; //0 - COMUM / 1 - FORTE
//FIM VARIAVEIS GLOBAIS ---------------------------------------------



//FUNCOES TELA ------------------------------------------------------
//TELA 1 ------------------------------
void printHomePage(){
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("NOME DA CAFETEIRA!");
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
  lcd.print(" INTENSIDADE=<     >");
  lcd.setCursor(0, 3);
  lcd.print(" INICIAR PREPARO <?>");
  atualizarTamanhoCafe();
  atualizarIntensidade();
}

void atualizarTamanhoCafe(){
  lcd.setCursor(10, 1);
  if(tamanho == 0) lcd.print(" 50");
  else if(tamanho == 1) lcd.print("100");
  else if(tamanho == 2) lcd.print("200");
  else if(tamanho == 3) lcd.print("300");
}

void atualizarIntensidade(){
  lcd.setCursor(14, 2);
  if(intensidade == 0) lcd.print("COMUM");
  else if(intensidade == 1) lcd.print("FORTE");
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
  if((loopMillis == 0 || (millis() - loopMillis) > tempoPularProximaTela) || isButtonPressed(3)){
    tela = proximaTela;
    mudarTela();
  }
}

void mudarTela(){
  if(tela == 1) printHomePage();
  else if (tela == 2) {
    cursor = 1;
    tamanho = tamanhoPadrao;
    intensidade = intensidadePadrao;
    printTela2();
  } else if (tela == 3) {
    atualizarValorLoopMillis();
    printTela3();
  } 
  else if (tela == 4) {
    atualizarValorLoopMillis();
    printTela4();
  }
  else if (tela == 5) {
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

void monitorarBotoesTela1(){
  if(isButtonPressed(3)){
    tela = 2;
    mudarTela();
  }
}

void monitorarBotoesTela2(){
  if(isButtonPressed(0)){ //BOTAO VOLTAR
    if(cursor == 1){ // O CURSOR ESTA NA PRIMEIRA LINHA
      tela =  1;
      mudarTela();
    } else { // O CURSOR ESTA NA SEGUNDA OU TERCEIRA LINHA
      cursor--;
      atualizar_cursor_tela2();
    }
  } else if(isButtonPressed(1)){ //BOTAO ESQUERDO
    if(cursor == 1){ // O CURSOR ESTA NA PRIMEIRA LINHA - CONFIGURA O TAMANHO DO CAFE
      if(tamanho > 0) tamanho--;
      atualizarTamanhoCafe();
    } else if(cursor ==2){ // O CURSOR ESTA NA SEGUNDA LINHA - CONFIGURA A INTENSIDADE DO CAFE
      if(intensidade > 0) intensidade--; 
      atualizarIntensidade();
    }
  } else if(isButtonPressed(2)){ //BOTAO DIREITO
    if(cursor == 1){ // O CURSOR ESTA NA PRIMEIRA LINHA - CONFIGURA O TAMANHO DO CAFE
      if(tamanho < 3) tamanho++;
      atualizarTamanhoCafe();
    } else if(cursor == 2){ // O CURSOR ESTA NA SEGUNDA LINHA - CONFIGURA A INTENSIDADE DO CAFE
      if(intensidade < 1) intensidade++;
      atualizarIntensidade();
    }
  } else if(isButtonPressed(3)){ //BOTAO OK
    if(cursor == 3){ // O CURSOR ESTA NA TERCEIRA LINHA - INICIAR PREPARO DO CAFE
      tela = 3;
      mudarTela();
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
  peso = escala.get_units();
  escala.power_down();
}

void atualizarTemperatura(){
  sensorTemperatura.requestTemperatures();//SOLICITA QUE A FUNÇÃO INFORME A TEMPERATURA DO SENSOR
  temperatura = sensorTemperatura.getTempCByIndex(0);
}

void switchServoCafe(bool ligar){
  if(ligar == true)
    servo.write(130);
  else
    servo.write(90);
}

void switchResistencia(bool ligar){
  if(ligar == true)
    digitalWrite(releResistencia, HIGH);
  else
    digitalWrite(releResistencia, LOW);
}

void switchBombaAgua(bool ligar){
  if(ligar == true)
    digitalWrite(releBombaAgua, HIGH);
  else
    digitalWrite(releBombaAgua, LOW);
}
//FIM FUNCOES SENSORES E ATUADORES --------------------------------------

void atualizarValorLoopMillis(){
  loopMillis = millis();
}

void setup() {
  Serial.begin(9600); //INICIALIZA A SERIAL

  //PINAGEM
  for(int i=0;i<5;i++){
    pinMode(buttons[i],INPUT_PULLUP);
  }

  pinMode(releBombaAgua, OUTPUT);
  pinMode(releResistencia, OUTPUT);

  //ASSOCIAR PORTA DO SERVO A VARIAVEL SERVO
  servo.attach(pinServoCafe);

  //PREPARACAO CELULA DE CARGA
  escala.begin (DT, SCK); //INICIA CONTROLE CELULA DE CARGA
  sensorTemperatura.begin(); //INICIA O SENSOR
  delay(1000); //INTERVALO DE 1 SEGUNDO
  Serial.println(escala.read());   // Aguada até o dispositivo estar pronto
  escala.set_scale(-1810879);     // Substituir o valor encontrado para escala
  escala.tare();                // O peso é chamado de Tare.

  //Inicializa o LCD e o backlight
  lcd.init();
  lcd.backlight();
  tela = 1;
  printHomePage();
}
 
void loop() {
  if(tela == 4){
    if((loopMillis == 0 || (millis() - loopMillis) > tempoAtualizacaoInfoTela4)){
      atualizarValorLoopMillis();
      atualizarPeso();
      atualizarTemperatura();
      atualizarInfoTela4();
    }


    switchResistencia(true);
    switchServoCafe(true);
    delay(6000);

    switchServoCafe(false);
    switchBombaAgua(true);
    delay(6000);

    switchResistencia(false);    
    switchBombaAgua(false);

    delay(2000);
    tela++;
    mudarTela();

  } else {
    //MONITORAR SENSORES E MOSTRAR NO DISPLAY - DEPENDE DO TEMPO DE ATUALIZAÇÃO
    if(tela == 1 && (loopMillis == 0 || (millis() - loopMillis) > tempoAtualizacaoInfoTela1)){
      atualizarValorLoopMillis();
      atualizarPeso();
      atualizarTemperatura();
      atualizarInfoTela1();
    }

    //MONITORAR BOTOES E REAGIR
    if(tela == 1) monitorarBotoesTela1(); //HOME PAGE
    else if(tela == 2) monitorarBotoesTela2(); //SELECIONAR CAFE
    else if(tela == 3) esperarTrocaTela(4); //INICIANDO PREPARO CAFE
    else if(tela == 5) esperarTrocaTela(1); //CAFE PRONTO
  }
}
