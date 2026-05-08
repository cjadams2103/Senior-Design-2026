/*
  Description:  This code is for the 25-26 Hydroponic Senior Design Project. Like the name details, this code output the immediate 
                PH value or PH voltage into the Serial Monitor depending on which print statement is commented out. Recommend running this program first
                before the main program to obtain voltages for slope.

  Editor: Christopher Adams
  Date: 04/06/26

*/
int ph;
float voltage;
const float v7 = 1.91;
const float v4 = 2.2;
const float slope = (v4 - v7)/(4.0 - 7.0);
float phValue;

void setup()
  {
    Serial.begin(9600);
    pinMode(ph, INPUT);
  }

void loop()
  {
    ph = analogRead(A0);
    voltage = ph * (3.3/4095.0);
    phValue = 7.0 + ((voltage - v7) / slope);
    Serial.println(phValue);
    //Serial.println(voltage);
    delay(500);
  }
