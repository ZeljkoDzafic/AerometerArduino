#include <SPI.h>
#include <Wire.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ST7735.h"
#include "Adafruit_BMP085.h"
#include <DHT.h>
#include <MQ2.h>
#include <MQ135.h>
#include <MQ7_LG.h>
#include <ArduinoJson.h>
    
#define DHTPIN 4
#define DHTTYPE DHT11   
#define CS   10     
#define DC   9     
#define RST  8      
#define MQ2PIN A2
#define MQ135PIN A0
#define NIGHTMODE true
    

#define MQ7_VRL_PIN             A1
#define PRINT_PERIOD            2000
    
MQ7_LG mq7(MQ7_VRL_PIN);
MQ2 mq2(MQ2PIN);
MQ135 mq135(MQ135PIN);
Adafruit_ST7735 tft = Adafruit_ST7735(CS, DC, RST); 
Adafruit_BMP085 bmp;    
DHT             dht(DHTPIN, DHTTYPE); 


boolean night_mode = true;
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

void setup(void) {
  
  // Initialisiere RTC
  Wire.begin();
  Serial.begin(9600);
  bmp.begin();  // BMP085 starten
  dht.begin();  // DHT starten
  mq2.begin();
  mq7.init();
  
  // Display
  tft.initR(INITR_BLACKTAB);     
  
  display_show();
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
  if(isnan(t)){
  }
    else if((int)t!=(int)temp){
   show_temp(temp,true);
   temp=t;
   if(min_temp>temp)min_temp=temp;
   if(max_temp<temp)max_temp=temp;
   show_temp(temp,false);
   root["temp"] = t; 
 

  }
  tempct++;
  
  float h = dht.readHumidity();
  if(isnan(h)){}
  else if(h!=hum){
   show_hum(hum,true);
   hum=h;
   if(min_humidity>hum)min_humidity=hum;
   if(max_humidity<hum)max_humidity=hum;
   show_hum(hum,false);
   root["hum"] = hum;
  }
  
  int32_t p=bmp.readPressure();
  if(p!=pressure){
   show_pressure(pressure,true);
   pressure=p;
   if(min_pressure>pressure)min_pressure=pressure;
   if(max_pressure<pressure)max_pressure=pressure;
   show_pressure(pressure,false);
  }

  root["pressure"] = pressure;
  
   float _lpg = mq2.readLPG();
   float _co = mq2.readCO();
   float _smoke = mq2.readSmoke();
   float _co2  = mq135.getPPM();
   float _co1 = mq7.read();
  
   show_lpg_co_smoke_co2(lpg,co, smoke,co2,co1,true);

   lpg = _lpg;
   co = _co;
   smoke = _smoke;
   co2 = _co2;
   co1 = _co1;
   
  root["lpg"] = lpg;
  root["co"] = co;
  root["smoke"] = smoke;
  root["co2"] = co2;
  root["co1"] = co1; 
  root.printTo(Serial);
  show_lpg_co_smoke_co2(lpg,co, smoke,co2,co1,false);
  
  delay(5000);  
 
}

void show_temp(float temp,boolean clear){
  
  int clearcolor=night_mode?ST7735_BLACK:ST7735_WHITE;
  int textcolor=night_mode?ST7735_WHITE:ST7735_BLACK;
  
  byte xs=10;
  byte ys=15;
  String htemp=String((int)temp);
  //byte xss=(temp<10?:temp
  
  set_text(xs,ys,htemp,clear?clearcolor:textcolor,1);
  tft.write(247); // das Â°-Zeichen  
  tft.print("C");
  set_text(xs+68,ys+1,String((int)max_temp),clear?clearcolor:ST7735_RED,1);
  tft.write(247);tft.print("C");
  set_text(xs+68,ys+11,String((int)min_temp),clear?clearcolor:ST7735_BLUE,1);
  tft.write(247);tft.print("C");
}

void show_hum(float hum,boolean clear){

  int clearcolor=night_mode?ST7735_BLACK:ST7735_WHITE;
  int textcolor=night_mode?ST7735_WHITE:ST7735_BLACK;

  byte xs=10;
  byte ys=60;
  
  set_text(xs,ys,String((int)hum)+"%",clear?clearcolor:textcolor,1);
  set_text(xs+68,ys+1,String((int)max_humidity)+"%",clear?clearcolor:ST7735_GREEN,1);
  set_text(xs+68,ys+11,String((int)min_humidity)+"%",clear?clearcolor:ST7735_YELLOW,1);
}


void show_lpg_co_smoke_co2(float lpg,float co,float smoke, float co2,float co1, boolean clear){

  int clearcolor=night_mode?ST7735_BLACK:ST7735_WHITE;
  int textcolor=night_mode?ST7735_WHITE:ST7735_BLACK;

  byte xs=11;
  byte ys=85;
  
  set_text(xs,ys,String((int)lpg)+" LPG",clear?clearcolor:ST7735_CYAN,1);
  set_text(xs,ys+13,String((int)co)+" CO",clear?clearcolor:ST7735_GREEN,1);
  set_text(xs,ys+26,String((int)smoke)+" SMOKE",clear?clearcolor:ST7735_YELLOW,1);
  set_text(xs,ys+39,String((int)co2)+" CO2",clear?clearcolor:ST7735_MAGENTA,1);
  
  set_text(xs,ys+52,String(co1)+" CO",clear?clearcolor:ST7735_WHITE,1);
}

void show_pressure(float pressure,boolean clear){
  
  int clearcolor=night_mode?ST7735_BLACK:ST7735_WHITE;
  int textcolor=night_mode?ST7735_WHITE:ST7735_BLACK;
  
  byte xs=10;
  byte ys=45;
  
  set_text(xs,ys,String((int32_t)pressure)+" Pa",clear?clearcolor:textcolor,1);
  
  if(max_pressure>=100000)xs-=8; 
  set_text(xs+68,ys-5,String((int32_t)max_pressure)+" Pa",clear?clearcolor:ST7735_CYAN,1);
  set_text(xs+68,ys+5,String((int32_t)min_pressure)+" Pa",clear?clearcolor:ST7735_MAGENTA,1);//
}


void set_text(int x,int y,String text,int color,int size){
  
  tft.setTextSize(size);
  tft.setCursor(x,y);
  tft.setTextColor(color);
  tft.print(text);
}

void display_show(){

  tft.fillScreen(NIGHTMODE?ST7735_BLACK:ST7735_WHITE); 
  set_text(2,2,"Zeljko Dzafic",ST7735_BLUE,1);  
  set_text(14,147,"www.tfzr.uns.ac.rs",ST7735_GREEN,1);
  

}


