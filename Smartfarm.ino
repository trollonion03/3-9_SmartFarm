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


//network setup
int status = WL_IDLE_STATUS;
char ssid[] = "test";
char pass[] = "test";
int keyIndex = 0; 
unsigned int localPort = 2390;
IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
const int NTP_PACKET_SIZE = 48; // NTP timestamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
WiFiUDP Udp;

//var
int high1 = 0;
int high2 = 0;
int high3 = 0;
int soilval1 = 0;
int soilval2 = 0;
int soilval3 = 0;
float hum = 0;
float temp = 0;


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
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }
  
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("Connected to WiFi");
  printWifiStatus();

  Serial.println("\nStarting connection to server...");
  Udp.begin(localPort);
 
  //LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0); //line 1
  lcd.print("Hello there!");
  lcd.setCursor(0,1); //line 2
  lcd.print("SmartFarm");

  //dht
  dht.begin();

}

void loop() {
  wifi11();
  waterpump();
  soil();
  lcds();
  dhts();
  //wifi
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);
  if (Udp.parsePacket()) {
    Serial.println("packet received");
    
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = ");
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);


    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print((epoch  % 86400L) / 3600 + 9); // print the hour (86400 equals secs per day)
    Serial.print(':');
    if (((epoch % 3600) / 60) < 10) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ((epoch % 60) < 10) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch % 60); // print the second
  }
  // wait ten seconds before asking for the time again
  delay(10000);
  
}

//네트워크
unsigned long sendNTPpacket(IPAddress& address) {
  //Serial.println("1");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  //Serial.println("2");
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  //Serial.println("3");

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  //Serial.println("4");
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  //Serial.println("5");
  Udp.endPacket();
  //Serial.println("6");
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void wifi11() {
  
  
}

//RTC


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
