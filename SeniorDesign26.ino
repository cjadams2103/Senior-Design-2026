/*

Description:  This code is for 2025-2026 Hydroponic Senior Design Project.
              The code measures PH and TDS and outputs the reading on display 
              and uploads reading to Thingspeak for long term reading

Editor: Christopher Adams

Date: 04/19/26

Note: This code is for the basic operation of reading PH and TDS and uploading to
      Thingspeak. There are commented out sections not critical to basic operation
      and require additional parts

      In order for the code to work correctly, you must include Wifi SSD, WIFI Password
      Thingspeak Channel ID and Thingspeak Write API

*/

#include <WiFiClientSecure.h>
#include "secrets.h"
#include "ThingSpeak.h"     // Always include thingspeak header file after other header files and custom macros
#include <Adafruit_GFX.h>   // For 128x64 Display ID:0x3C
#include <Adafruit_SSD1306.h> // |    |     |     |     |
#include <PS2Keyboard.h>  //For PS/2 Keyboard
#include <LiquidCrystal_I2C.h>  //For 16x2 Display ID:0x27
#include <Wire.h>

#define ph_sensor_pin A3
#define tds_sensor_pin A2
#define vref 3.3
#define offset 0
//#define TS_ENABLE_SSL
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64


const float v7 = 2.16;  //Voltage at PH of 7
const float v4 = 2.43;  //Voltage at PH of 4
const float slope = (v4 - v7)/(4.0 - 7.0);  //Slope for PH equation
float avgVoltage = 0, temperature = 25, tdsValue = 0;

int analogBufferIndex = 0, copyIndex = 0;
unsigned long int ph_avgValue, tds_avgValue;
int keyIndex = 0; // your network key Index number (needed only for WEP)
const int DataPin = D5;
const int IRQpin =  D6;
int cur = 0;

char lcdBuffer[32];
char wificheck = 'z';
char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

unsigned long myChannelNumber = SECRET_CH_ID;

WiFiClient  client;
String myStatus = "";
PS2Keyboard keyboard;
LiquidCrystal_I2C  lcd(0x27, 16, 2);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


void setup() {
  Serial.begin(115200);
  delay(2000);

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  pinMode(ph_sensor_pin, INPUT);
  pinMode(tds_sensor_pin, INPUT);

  /*
  pinMode(DataPin, INPUT_PULLUP);
  pinMode(IRQpin, INPUT_PULLUP);

  keyboard.begin(DataPin, IRQpin);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Keyboard Ready!");
  delay(2000);
  lcd.clear();
  memset(lcdBuffer, 0, sizeof(lcdBuffer));
  */

  while (!Serial) {
    ;
  }
  

  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  //client.setInsecure(); // Needed for ESP8266/ESP32 SSL connection
}

void loop() {

  /*
   while(wificheck == 'z') {
    lcd.setCursor(0,0);
    lcd.print("a: New | b: Old ");
    
    if (keyboard.available()) {
      char c = keyboard.read(); // NEW READ INSIDE THE LOOP
      if (c == 'a') {
        wificheck = 'a';
        lcd.clear();
        Serial.println("Selected Mode A");
      } 
      else if (c == 'b') {
        wificheck = 'b';
        lcd.clear();
        Serial.println("Selected Mode B");
      }
    }
    yield(); // Keeps the ESP32 background tasks (WiFi) alive
  }
  */

  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");

    //Displays connecting status when not connected to Serial Monitor
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(10, 10);
    display.println("Connecting");
    display.display();
    Serial.println(SECRET_SSID);

    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network.
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(10, 10);
    display.println("Connected");
    display.display();
    Serial.println(SECRET_SSID);
  }

  // --- 1. pH SENSOR READING (Corrected) ---
  int buf[30]; 
  for(int i=0; i<30; i++){ //Get 30 sample values
    buf[i]=analogRead(ph_sensor_pin); 
    Serial.println(buf[i]);
    delay(10);
  }
  for(int i=0; i<29; i++){ //sort the analog from small to large
    for(int j=i+1; j<30; j++){
      if(buf[i]>buf[j]){
        int temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }
    }
  }

  ph_avgValue=0;
  for(int i=10; i<20; i++){ //take the average value of 10 center samples
    ph_avgValue+=buf[i];
  }

  ph_avgValue /= 10;

   // <-- FIX: Use VREF (3.3V) and 4095.0 (for 12-bit ADC on ESP32)
  float phVoltage = ((float)ph_avgValue * vref / 4095.0); 
  Serial.println(phVoltage);
  float phValue = 7.0 + ((phVoltage - v7) / slope); //convert the voltage into pH

  // --- 2. TDS SENSOR READING (Simplified to prevent crash) ---
  int tdsBuffer[10];
  for (int i = 0; i < 10; i++) {
    tdsBuffer[i] = analogRead(tds_sensor_pin);
    delay(10);
  }
  
  // Get average of 10 samples
  tds_avgValue = 0;
  for (int i = 0; i < 10; i++) {
    tds_avgValue += tdsBuffer[i];
  }
  tds_avgValue /= 10;

   // <-- FIX: Use VREF (3.3V) and 4095.0 (for 12-bit ADC on ESP32)
  avgVoltage = tds_avgValue * (float)vref / 4095.0; 
  float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0); 
  float compensationVolatge = avgVoltage / compensationCoefficient; 
  tdsValue = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * 0.5;

  // Print values to Serial Monitor
  Serial.print("TDSValue:");
  Serial.print(tdsValue, 0);
  Serial.println("ppm");
  Serial.print(" pH:");
  Serial.print(phValue,2); 
  Serial.println(""); 

  // --- 3. SEND TO THINGSPEAK AND OUTPUT TO DISPLAY ---
  
  ThingSpeak.setField(1, phValue);
  ThingSpeak.setField(2, tdsValue);
  String output = "Ph: " + String(phValue) + "\n\n" + "Tds: " + String(tdsValue);

  //Displays PH and TDS locally while not connected to Serial Monitor
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 10);
  display.println(output);
  display.display();

  // Set a status message
  if(phValue > tdsValue){
    myStatus = String("field1 is greater than field2"); 
  }
  else if(phValue < tdsValue){
    myStatus = String("field1 is less than field2");
  }
  else{
    myStatus = String("field1 equals field2");
  }
  
  ThingSpeak.setStatus(myStatus);
  
  // Write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  delay(20000); // Wait 20 seconds to update the channel again 
}















