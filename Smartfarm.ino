
/********************Eunhasu smartfarm(v1.1)************************
 * Programmed by Trollonion03(https://github.com/trollonion03)     *
 * based on Arduino UNO wifi rev2                                  *
 * DHT-22 - D2                                                     *
 * L298 - D3,4,5,6                                                 *
 * RTC - D7,8,9                                                    *
 * Soil Moisture Sensor - A0,1,2                                   *
 * OLED/LCD - A4,5                                                 *
 * etc - D10,11,12                                                 *
 *******************************************************************/
#include <DHT.h>
#include <Firebase_Arduino_WiFiNINA.h>
#include "fbse1.h"
#include <SPI.h>
#include <Wire.h>
#include <WiFiUdp.h>
#include <LiquidCrystal_I2C.h>
#include <ThreeWire.h>  
#include <RtcDS1302.h>


//network setup
#define FIREBASE_HOST FIREBASE_PROJECT_SECRET
#define FIREBASE_AUTH FIREBASE_DATABASE_SECRET
#define WIFI_SSID SECRET_SSID
#define WIFI_PASSWORD SECRET_PASS

FirebaseData firebaseData;

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
int date1 = 0;
int date2 = 0;
int date3 = 0;
float hum;
float temp;
String path = "/trollonionfarm";

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
  
  //Netowork setup
  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED){
    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH, WIFI_SSID, WIFI_PASSWORD);
  Firebase.reconnectWiFi(true);

 
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
  Serial.println(dht.readHumidity());
  Serial.println(dht.readTemperature());
  waterpump();
  fbase();
  soil();
  lcds();
  lortcs();
  calucatedate(); 
}


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

void calucatedate() {
  //Method_1(unstable, choose one of them)
  /*int loc1 = 0;
  int locc1 = 0;
  int loc2 = 0;
  int locc2 = 0;
  int loc3 = 0;
  int locc3 = 0;
  int val1 = 0;
  int val2 = 0;
  int val3 = 0;
  
  if(val1 == 0) {
   loc1 = lortcdate + 4;
   val1 = 1; 
  }
  else if(val2 == 0) {
   loc2 = lortcdate + 4;
   val2 = 1;
  }
  else if(val3 == 0) {
    loc3 = lortcdate + 4;
    val3 = 1;
  }

  if(loc1 == lortcdate) {
    digitalWrite(3, LOW);
    analogWrite(4, HIGH);
    delay(3000);
    analogWrite(4, LOW);
    val1 = 0;
  }
  else if(loc2 == lortcdate) {
    digitalWrite(3, LOW);
    analogWrite(5, HIGH);
    delay(3000);
    analogWrite(5, LOW);
    val2 = 0;
  }
  else if(loc3 == lortcdate) {
    digitalWrite(3, LOW);
    analogWrite(6, HIGH);
    delay(3000);
    analogWrite(6, LOW);
  } */

  //Method_2(recommended method)
  if(lortcdate%4 == 0 && lortchour == 15 && lortcmin == 11) {
    digitalWrite(3, LOW);
    analogWrite(4, HIGH);
    analogWrite(5, HIGH);
    analogWrite(6, HIGH);
    delay(9000);
    analogWrite(4, LOW);
    analogWrite(5, LOW);
    analogWrite(6, LOW);
  }

  //Method_3(It is just a joke. If you're going to use this seriously, see a doctor : not in hospital, in lab or academy for yor mind.)
  /*if(lortcdate == 4) {
    digitalWrite(3, LOW);
    analogWrite(4, HIGH);
    analogWrite(5, HIGH);
    analogWrite(6, HIGH);
    delay(9000);
    analogWrite(4, LOW);
    analogWrite(5, LOW);
    analogWrite(6, LOW);
  }*/
}

//Motor
void waterpump() {
  if(high1 == 1) {
    digitalWrite(3, LOW);
    digitalWrite(4, HIGH); 
  }
  else if(high2 == 1) {
    digitalWrite(3, LOW);
    digitalWrite(5, HIGH);
  }
  else if(high3 == 1) {
    digitalWrite(3, LOW);
    digitalWrite(6, HIGH);
  }
}

void soil() {
  soilval1 = analogRead(A0Pin);
  soilval2 = analogRead(A1Pin);
  soilval3 = analogRead(A2Pin);

  if(soilval1 > 800) {
    high1 = 1;
  }
  else if(soilval2 > 800) {
    high2 = 1;
  }
  else if(soilval3 > 800) {
    high3 = 1;
  }
  else {
    high1 = 0;
    high2 = 0;
    high3 = 0;
  }
}


void lcds() {
  hum = dht.readHumidity();
  delay(10);
  temp = dht.readTemperature();
  
  lcd.init();
  lcd.setCursor(0,0); //hum
  lcd.print("humidity");
  lcd.setCursor(0,1); //line 2
  lcd.print(hum);
  delay(1000);
  lcd.init();
  lcd.setCursor(0,0); //temp
  lcd.print("TEMP");
  lcd.setCursor(0,1); //line 2
  lcd.print(temp);
  delay(1000);
  lcd.init();  
  lcd.setCursor(0,0); //soil1
  lcd.print("soil1");
  lcd.setCursor(0,1); //line 2
  lcd.print(soilval1);
  delay(1000);
  lcd.init();
  lcd.setCursor(0,0); //soil2
  lcd.print("soil2");
  lcd.setCursor(0,1); //line 2
  lcd.print(soilval2);
  delay(1000);
  lcd.init();
  lcd.setCursor(0,0); //soil3
  lcd.print("soil3");
  lcd.setCursor(0,1); //line 2
  lcd.print(soilval3);
  delay(2000);
}

void fbase() {
  if (Firebase.setFloat(firebaseData, path + "/Hum",hum)) {
    Serial.println("humidity OK");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.print("VALUE: ");
    if (firebaseData.dataType() == "float")
      Serial.println(firebaseData.floatData());
  }
  if (Firebase.setFloat(firebaseData, path + "/Temp",temp)) {
    Serial.println("TEMP OK");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.print("VALUE: ");
    if (firebaseData.dataType() == "float")
      Serial.println(firebaseData.floatData());
  }
  if (Firebase.setInt(firebaseData, path + "/Soil1",soilval1)) {
    Serial.println("soil1 OK");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.print("VALUE: ");
    if (firebaseData.dataType() == "int")
      Serial.println(firebaseData.intData());
  }
  if (Firebase.setInt(firebaseData, path + "/Soil2",soilval2)) {
    Serial.println("soil2 OK");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.print("VALUE: ");
    if (firebaseData.dataType() == "int")
      Serial.println(firebaseData.intData());
  }
  if (Firebase.setInt(firebaseData, path + "/Soil3",soilval3)) {
    Serial.println("soil3 OK");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.print("VALUE: ");
    if (firebaseData.dataType() == "int")
      Serial.println(firebaseData.intData());
  }
  else {
    Serial.println();
    Serial.println("FAIL: " + firebaseData.errorReason());
  }
}
