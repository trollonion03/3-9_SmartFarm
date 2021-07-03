/********************Eunhasu smartfarm(v1)***************************
 * Programmed by Trollonion03(https://github.com/trollonion03)
 * based on Arduino UNO wifi rev2
 * DHT-22 - D2
 * L298 - D3,4,5,6
 * RTC - D7,8,9
 * Soil Moisture Sensor - A0,1,2
 * OLED/LCD - A4,5
 * etc - D10,11,12
 *******************************************************************/
#include <DHT.h>//D3
#include <SPI.h>
#include <WiFiNINA.h>
#include <Wire.h>
#include <WiFiUdp.h>
#include <LiquidCrystal_I2C.h>
#include <ThreeWire.h>  
#include <RtcDS1302.h>


//network setup
/*
int status = WL_IDLE_STATUS;
char ssid[] = "test";
char pass[] = "test";
int keyIndex = 0; 
*/

//var
int high1 = 0;
int high2 = 0;
int high3 = 0;
int soilval1 = 0;
int soilval2 = 0;
int soilval3 = 0;
int lortcdate = 0;
int lortchour = 0;
int lortcmin = 0;
int lortcsec = 0;
float hum = 0;
float temp = 0;

//rtc
ThreeWire myWire(7,8,9); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);
#define countof(a) (sizeof(a) / sizeof(a[0]))

//soil
#define A0Pin 0
#define A1Pin 1
#define A2Pin 2

//LCD
LiquidCrystal_I2C lcd(0x27,16, 2); //0x3F, or 0x27

//DHT
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  
  //netowork setup
  
 
  //LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0); //line 1
  lcd.print("Hello there!");
  lcd.setCursor(0,1); //line 2
  lcd.print("SmartFarm");

  //dht
  dht.begin();

  //rtc
  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  printDateTime(compiled);
  Serial.println();

  if (!Rtc.IsDateTimeValid()) { 
    Rtc.SetDateTime(compiled);
  }

  if (Rtc.GetIsWriteProtected()) {
    Rtc.SetIsWriteProtected(false);
  }

  if (!Rtc.GetIsRunning()) {
    Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled) {
    Rtc.SetDateTime(compiled);
  }
    
}

void loop() {
  waterpump();
  soil();
  lcds();
  dhts();
  lortcs(); 
}

//네트워크

//RTC
void lortcs() {
  RtcDateTime now = Rtc.GetDateTime();

  printDateTime(now);
  Serial.println();
  
  if (!now.IsValid()) {
      // Common Causes:
      //    1) the battery on the device is low or even missing and the power line was disconnected
      Serial.println("RTC lost confidence in the DateTime!");
  }
  delay(10000); // ten seconds
}

void printDateTime(const RtcDateTime& dt)
{
  char datestring[20];

  snprintf_P(datestring, 
          countof(datestring),
          PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
          dt.Month(),
          dt.Day(),
          dt.Year(),
          dt.Hour(),
          dt.Minute(),
          dt.Second() );
  Serial.print(datestring);
  lortcdate = dt.Day();
  lortchour = dt.Hour();
  lortcmin = dt.Minute();
  lortcsec = dt.Second();
}



//펌프모터
void waterpump() {
  if(high1 == 1) {
    digitalWrite(3, HIGH);
    analogWrite(4, 255);
    delay(3000);
    analogWrite(4, 0); 
  }
  else if(high2 == 1) {
    digitalWrite(3, HIGH);
    analogWrite(5, 255);
    delay(3000);
    analogWrite(5, 0);
  }
  else if(high3 == 1) {
    digitalWrite(3, HIGH);
    analogWrite(6, 255);
    delay(3000);
    analogWrite(6, 0);
  }
}

void soil() {
  soilval1 = analogRead(A0Pin);
  soilval2 = analogRead(A1Pin);
  soilval3 = analogRead(A2Pin);

  if(soilval1 > 900) {
    high1 = 1;
  }
  else if(soilval2 > 900) {
    high2 = 1;
  }
  else if(soilval3 > 900) {
    high3 = 1;
  }
  else {
    high1 = 0;
    high2 = 0;
    high3 = 0;
  }
}

void dhts() {
  hum = dht.readHumidity();
  temp = dht.readTemperature();
}

void lcds() {

  
}
