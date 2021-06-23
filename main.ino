/*****************************************
 *  TERMOMETRO CON LCD PARA SEGUIMIENTO  *
 *  DE REACCIONES CON TAG DE TIEMPO Y    *
 *  ENVIO DE DATOS POR SERIAL            *
 *  VERSION: 0.3beta                     *
 *  POR: Miguel Orozco                   *
 *  git: https://github.com/Necrovalle/termometro_campana.git
 *****************************************/

//****************************** LIBRERIAS
#include <Wire.h>
#include "LiquidCrystal_I2C.h"      
LiquidCrystal_I2C lcd(0x27, 20, 4);
#include "SparkFunDS1307RTC.h"
#include <SPI.h>


//********************** DECLARACIONES GLOBALES
#define MAX6675_SO 12 //Driver termopar
#define MAX6675_SCK 11//Driver termopar
#define MAX6675_CS1 10 //Driver termopar 1
#define MAX6675_CS2 9 //Driver termopar 2
#define PRINT_USA_DATE
#define SQW_INPUT_PIN 2   // Input pin to read SQW
#define SQW_OUTPUT_PIN 21 // LED to indicate SQW's state
String Ver = "0.6a";        //version
boolean Config = false;     //Bandera de configuracion
boolean Config_serial=false;//Bandera de configuracion de envio por serie
boolean Serie_send = false; //Bandera de envio por serie
float Temp1 = 0.0;          //Temperatura leida 1
float Temp2 = 0.0;          //temperatura leida 2
String Time;
float Tmin = 10.00;         //Temperatura minima de operacion
float Tmax = 100.00;        //Temperatura maxima de operacion
float Delt = 6.00;          //Incremento por linea en la grafica
float Data[] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
//vector de datos historicos
int NS = 10;              //multiplicador de tiempo en grafica
int CS = 1;               //Contador de datos leidos     
byte N0[] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111
};
byte N1[] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111
};
byte N2[] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B11111
};
byte N3[] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B11111,
  B11111
};
byte N4[] = {
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};
byte N5[] = {
  B00000,
  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};
byte N6[] = {
  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};
byte N7[] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};


void setup() 
{
  Serial.begin(9600);
  pinMode(MAX6675_SO, INPUT);
  pinMode(MAX6675_SCK, OUTPUT);
  pinMode(MAX6675_CS1, OUTPUT);
  digitalWrite(MAX6675_CS1, HIGH);
  pinMode(MAX6675_CS2, OUTPUT);
  digitalWrite(MAX6675_CS2, HIGH);
  rtc.begin();
  rtc.writeSQW(SQW_SQUARE_1);
  //rtc.autoTime();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(5,0);
  lcd.print("Termo-Chem");
  lcd.setCursor(1,1);
  Serial.println("[C] por serie para configuracion");
  lcd.print("[C] por serie para"); 
  lcd.setCursor(5,2);
  lcd.print("configuracion");
  lcd.setCursor(1,3);
  lcd.print("V" + Ver );
  delay(2500);
  lcd.clear();
  lcd.createChar(0,N0);
  lcd.createChar(1,N1);
  lcd.createChar(2,N2);
  lcd.createChar(3,N3);
  lcd.createChar(4,N4);
  lcd.createChar(5,N5);
  lcd.createChar(6,N6);
  lcd.createChar(7,N7);
  //Cambiar fecha y hora
  // e.g. 13:19:00 | Sunday May 6, 2021:
  //rtc.setTime(ss, mm, h, S, DD, MM, AA);  // Uncomment to manually set time
  //rtc.setTime(00, 34, 15, 6, 11, 6, 21);
  //rtc.set24Hour(); // Use rtc.set12Hour to set to 12-hour mode
}

void loop() 
{
  if (Serial.available() > 0) {
    //Configiraciones
    char ENT;
    ENT = Serial.read();
    Configuracion(ENT);
  } else {
    if (!Config){
      //Funcion de despliegue de info en LCD
      salidaLCD();
      if (Serie_send){
        //Funcionde despliegue de info por Serial
        salidaSerie();
      }
      delay(2000); 
    }
  }
}

//********************************* FUNCIONES PROPIAS
void Configuracion(char CMD){
  String Separador="---------------------------";
  switch (CMD){
    case 'C':
      if (!Config){
        Serial.println(Separador);
        Serial.println("Termo-Chem Configuracion:");
        Serial.println("[T]: Configurar reloj");
        Serial.println("[S]: Configurar envio de datos");
        Config = true;
        lcd.clear();
        lcd.setCursor(2,0);
        lcd.print("<<Configurando>>"); 
      }
      break;

    case 'T':
      if (Config){
        Serial.println(Separador);
        Serial.println("Configurar hora del RTC:");
      }
      break;

    case 'S':
      if (Config){
        Serial.println(Separador);
        Serial.println("Configurar envio por serie:");
        Serial.println("[0]: Solo LCD");
        Serial.println("[1]: LCD y Serie");
        Config_serial = true;
        //Config = false;
      }
      break;

    case '0':
      if (Config_serial){
        lcd.setCursor(2,1);
        lcd.print("Solo LCD");
        Serial.println("Solo LCD [OK]");
        Serie_send = false;
        Config_serial = false;
        Config = false;
        delay(2000);
      }
      break;

    case '1':
      if (Config_serial){
        lcd.setCursor(2,1);
        lcd.print("LCD y Serial");
        Serial.println("LCD y Serial [OK]");
        Serie_send = true;
        Config_serial = false;
        Config = false;
        delay(2000);
      }
      break;

  }
}

String obtenerTiempo(){
  String TimeRTC;
  rtc.update();
  TimeRTC = String(rtc.hour()) + ":"; // Print hour
  if (rtc.minute() < 10)
    TimeRTC = TimeRTC + '0'; // Print leading '0' for minute
    TimeRTC = TimeRTC + String(rtc.minute()) + ":"; // Print minute
    if (rtc.second() < 10)
      TimeRTC = TimeRTC + '0'; // Print leading '0' for second
      TimeRTC = TimeRTC + String(rtc.second()); // Print second
      if (rtc.is12Hour()) // If we're in 12-hour mode
      {
        // Use rtc.pm() to read the AM/PM state of the hour
        if (rtc.pm()) TimeRTC = TimeRTC + " PM"; // Returns true if PM
        else TimeRTC = TimeRTC = " AM";
      }
      return TimeRTC;
}

void salidaLCD(){
  lcd.clear();
  Temp1 = leer_termopar(1);
  delay(2);
  Temp2 = leer_termopar(2);
  delay(2);
  Temp1 = Temp1 + leer_termopar(1);
  delay(2);
  Temp2 = Temp2 + leer_termopar(2);
  delay(2);
  Temp1 = Temp1 + leer_termopar(1);
  delay(2);
  Temp2 = Temp2 + leer_termopar(2);
  Temp1 = Temp1/3.0;
  Temp2 = Temp2/3.0;
  Time = obtenerTiempo();
  lcd.setCursor(0,0);
  lcd.print("T1= ");
  lcd.setCursor(3,0);
  lcd.print(Temp1, 1);
  lcd.setCursor(8,0);
  lcd.print("C");

  lcd.setCursor(11,0);
  lcd.print("T2= ");
  lcd.setCursor(14,0);
  lcd.print(Temp2, 1);
  lcd.setCursor(19,0);
  lcd.print("C");
  
  lcd.setCursor(10,1);
  lcd.print("H=");
  lcd.setCursor(12,1);
  lcd.print(Time);
  CS++;
  if (CS > NS){
    graficado(Temp1);
    CS = 1;  
  } else {
    graficadoHist();
  }    
}

void salidaSerie(){
  Serial.print("T1=");
  Serial.print(Temp1);
  Serial.print(",");
  Serial.print("T2=");
  Serial.print(Temp2);
  Serial.println(Time);
}


void graficado(float Dat){
  int numChar;
  for (int i=0; i<19; i++){
    Data[i] = Data[i+1];
  }
  Data[19]=Dat;
  lcd.setCursor(0,3);
  for (int j=0; j<20; j++){
    numChar = int((Data[j]-10.0)/6);
    if (numChar > 7){
      lcd.write(char(7));
    } else {
      if (numChar < 1){
        lcd.write(char(0));
      } else {
        lcd.write(char(numChar)); 
      }
    }
  }
  lcd.setCursor(0,2);
  for (int k=0; k<20; k++){
    numChar = int((Data[k]-10.0)/6);
    if (numChar > 7){
      lcd.write(numChar - 8);
    } else {
      lcd.print(" ");
    }
  }
}

void graficadoHist(){
  int numChar;
  lcd.setCursor(0,3);
  for (int j=0; j<20; j++){
    numChar = int((Data[j]-10.0)/6);
    if (numChar > 7){
      lcd.write(char(7));
    } else {
      if (numChar < 1){
        lcd.write(char(0));
      } else {
        lcd.write(char(numChar)); 
      }
    }
  }
  lcd.setCursor(0,2);
  for (int k=0; k<20; k++){
    numChar = int((Data[k]-10.0)/6);
    if (numChar > 7){
      lcd.write(numChar - 8);
    } else {
      lcd.print(" ");
    }
  }
}

double leer_termopar(int NT){
    uint16_t v;

    switch (NT){
      case 1:
        digitalWrite(MAX6675_CS1, LOW);
        break;

      case 2:
        digitalWrite(MAX6675_CS2, LOW);
        break;

      default:
        return NAN;
        break;
    }
    delay(1);
    // Read in 16 bits,
    //  15    = 0 always
    //  14..2 = 0.25 degree counts MSB First
    //  2     = 1 if thermocouple is open circuit
    //  1..0  = uninteresting status
    v = shiftIn(MAX6675_SO, MAX6675_SCK, MSBFIRST);
    v <<= 8;
    v |= shiftIn(MAX6675_SO, MAX6675_SCK, MSBFIRST);
    digitalWrite(MAX6675_CS1, HIGH);
    digitalWrite(MAX6675_CS2, HIGH);
    if (v & 0x4)
    {
        return NAN; // Bit 2 indicates if the thermocouple is disconnected
    }
    // The lower three bits (0,1,2) are discarded status bits
    v >>= 3;
    // The remaining bits are the number of 0.25 degree (C) counts
    return (v * 0.25) - 1.0;   //correccion por voltaje de alimentacion
}
