//CONFIG ------------------------------------------------------------
const int tamanhoPadrao=1;
const int intensidadePadrao=0;
const int debounceDelay = 1500;
const int tempoAtualizacaoPeso = 2000;
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
//FIM BIBLIOTECAS ---------------------------------------------------



//PINAGEM -----------------------------------------------------------
//CELULA DE CARGA
#define DT A0
#define SCK A1
//SENSOR DE TEMPERATURA
#define pinSensorTemp 7
//FIM PINAGEM -------------------------------------------------------



//VARIAVEIS GLOBAIS -------------------------------------------------
LiquidCrystal_I2C lcd(0x27,20,4); // DEFINE O ENDEREÇO UTILIZADO PELO ADAPTADOR I2C
OneWire ourWire(pinSensorTemp); //CONFIGURA UMA INSTÂNCIA ONEWIRE PARA SE COMUNICAR COM O SENSOR DE TEMPERATURA
DallasTemperature sensorTemperatura(&ourWire); //BIBLIOTECA DallasTemperature UTILIZA A OneWire
HX711 escala; //CRIA A VARIÁVEL DE ESCALA DE MEDIÇÃO DA CELULA DE CARGA
float peso = 0.0;
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
  lcd.print("TRES=   C PESO=    g");
}

void atualizarInfoTela1(){
  lcd.setCursor(15, 3);
  lcd.print(peso*1000, 0);
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
  lcd.setCursor(0, 0);
  lcd.print("Tela 3");
}
//FIM TELA 3 --------------------------
void mudarTela(){
  if(tela == 1) printHomePage();
  else if (tela == 2) {
    cursor = 1;
    tamanho = tamanhoPadrao;
    intensidade = intensidadePadrao;
    printTela2();
  } else if (tela == 3) printTela3();
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



//FUNCOES SENSORES --------------------------------------------------
void atualizarPeso(){
  escala.power_up();  
  peso = escala.get_units();
  escala.power_down();
}
//FIM FUNCOES SENSORES --------------------------------------------------



void setup() {
  Serial.begin(9600); //INICIALIZA A SERIAL

  //PINAGEM
  for(int i=0;i<5;i++){
    pinMode(buttons[i],INPUT_PULLUP);
  }  

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
  //MONITORAR SENSORES E MOSTRAR NO DISPLAY - DEPENDE DO TEMPO DE ATUALIZAÇÃO
  if((tela == 1) //|| tela fabricando o café .....
    && (loopMillis == 0 || (millis() - loopMillis) > tempoAtualizacaoPeso)){
    loopMillis = millis();
    atualizarPeso();
    atualizarInfoTela1();
  }

  //MONITORAR BOTOES E REAGIR
  if(tela == 1){
    //HOME PAGE
    monitorarBotoesTela1();
  } else if(tela == 2){
    //SELECIONAR CAFE
    monitorarBotoesTela2();
  } else {
    if(isButtonPressed(0)){//SOMENTE PARA TESTE
      tela --;
      mudarTela();
    }
  }
}
