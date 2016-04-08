#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <DHT.h>
#include <MQ2.h>
#include <MQ135.h>
#include <MQ7_LG.h>
#include <ArduinoJson.h>
#include "Adafruit_BMP085.h" 
    
#define DHTPIN 4    
#define DHTTYPE DHT11   
#define MQ2PIN A2
#define MQ135PIN A0
#define MQ7_VRL_PIN             A1
#define PRINT_PERIOD            2000

MQ7_LG mq7(MQ7_VRL_PIN);
MQ2 mq2(MQ2PIN);
MQ135 mq135(MQ135PIN);
Adafruit_BMP085 bmp;    
DHT             dht(DHTPIN, DHTTYPE); 

/*
  The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 */
 
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

float          temp = 1000;
float          hum = 1000;
int32_t        pressure = 1000;
float          min_temp = 1000;
float          max_temp = -1000;
float          min_humidity = 1000;
float          max_humidity = -1000;
int32_t min_pressure = 1000000;
int32_t max_pressure = -1000;
int            tempct = 0;

 
float        lpg = 0;
float        co = 0;
float        smoke = 0;
float        co2 = 0;
float        co1 = 0;
int          counter = 0;
void setup(void) {
  
  // Initialisiere RTC
  Wire.begin();
  Serial.begin(9600);
  bmp.begin();  // BMP085 starten
  dht.begin();  // DHT starten
  mq2.begin();
  mq7.init();
  lcd.begin(16, 2);
}

void loop() {
  int t;
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  if(tempct%2==0){
    t=(int) dht.readTemperature(false);
  } else { 
    t=(int) bmp.readTemperature();
  }
  if(counter%6==0){
    show_text("Temperature",t+" C");
  } 
  root["temp"] = t; 

   tempct++;
  
  float h = dht.readHumidity();
  
  if(counter%6==1){
   show_text("Humidity",String(h)+" %");
   } 
   root["hum"] = hum;
  
  
  int32_t p=bmp.readPressure();
  
  if(counter%6==2){
   show_text("Pressure",String(p)+" Pa");
  }

  root["pressure"] = pressure;
  
   float _lpg = mq2.readLPG();
   float _co = mq2.readCO();
   float _smoke = mq2.readSmoke();
   float _co2  = mq135.getPPM();
   float _co1 = mq7.read();
   
   if(counter%6==3){
     show_text(String(_co)+" CO",String(_lpg)+" LPG");
   }
   if(counter%6==4){
     show_text(String(_co2)+" CO2",String(_smoke)+" SMOKE");
   }
   if(counter%6==5){
     show_text("ZELJKO DZAFIC","TFZR.UNS.AC.RS");
   }
   
  root["lpg"] = _lpg;
  root["co"] = _co;
  root["smoke"] = _smoke;
  root["co2"] = _co2;
  root["co1"] = _co1; 
  root.printTo(Serial);
  
  delay(3000); 
  counter++;  
 
}
void show_text(String text1, String text2) {
  // Print a message to the LCD.
  lcd.clear();
  lcd.print(text1);
  lcd.setCursor(0, 1);
  lcd.print(text2);
}
