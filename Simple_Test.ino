/*  PS2Keyboard library example
  
  PS2Keyboard now requries both pins specified for begin()

  keyboard.begin(data_pin, irq_pin);
  
  Valid irq pins:
     Arduino Uno:  2, 3
     Arduino Due:  All pins, except 13 (LED)
     Arduino Mega: 2, 3, 18, 19, 20, 21
     Teensy 3.0:   All pins, except 13 (LED)
     Teensy 2.0:   5, 6, 7, 8
     Teensy 1.0:   0, 1, 2, 3, 4, 6, 7, 16
     Teensy++ 2.0: 0, 1, 2, 3, 18, 19, 36, 37
     Teensy++ 1.0: 0, 1, 2, 3, 18, 19, 36, 37
     Sanguino:     2, 10, 11
  
  for more information you can read the original wiki in arduino.cc
  at http://www.arduino.cc/playground/Main/PS2Keyboard
  or http://www.pjrc.com/teensy/td_libs_PS2Keyboard.html
  
  Like the Original library and example this is under LGPL license.
  
  Modified by Cuninganreset@gmail.com on 2010-03-22
  Modified by Paul Stoffregen <paul@pjrc.com> June 2010
*/
   
#include <PS2Keyboard.h>
#include <LiquidCrystal_I2C.h>

const int DataPin = D5 ;
const int IRQpin =  D6;

PS2Keyboard keyboard;
LiquidCrystal_I2C  lcd(0x27, 16, 2);

char lcdBuffer[32];
int cur = 0;

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(DataPin, INPUT_PULLUP);
  pinMode(IRQpin, INPUT_PULLUP);

  keyboard.begin(DataPin, IRQpin);
  Serial.println("Keyboard Test:");

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("keyboard Ready!");
  delay(2000);
  lcd.clear();

  memset(lcdBuffer, 0, sizeof(lcdBuffer));
  
}

void loop() {
  if (keyboard.available()) {
    
    // read the next key
    char c = keyboard.read();

    // Handle Backspace
    if (c == PS2_DELETE && cur > 0) {
      cur--;
      lcdBuffer[cur] = '\0';
    } 
    // Handle Regular Characters (and prevent overflow)
    else if (c >= 32 && c <= 126 && cur < 16) {
      lcdBuffer[cur] = c;
      cur++;
      lcdBuffer[cur] = '\0'; // Always keep it null-terminated
    }
    // Handle Enter (Clear screen)
    else if (c == PS2_ENTER) {
      cur = 0;
      memset(lcdBuffer, 0, sizeof(lcdBuffer));
      lcd.clear();
    }

    // Update LCD
    lcd.setCursor(0, 0);
    lcd.print("                "); // Clear the line with spaces
    lcd.setCursor(0, 0);
    lcd.print(lcdBuffer);
  }
    
   /* // check for some of the special keys
    if (c == PS2_ENTER) {
      Serial.println();
    } else if (c == PS2_TAB) {
      Serial.print("[Tab]");
    } else if (c == PS2_ESC) {
      Serial.print("[ESC]");
    } else if (c == PS2_PAGEDOWN) {
      Serial.print("[PgDn]");
    } else if (c == PS2_PAGEUP) {
      Serial.print("[PgUp]");
    } else if (c == PS2_LEFTARROW) {
      Serial.print("[Left]");
    } else if (c == PS2_RIGHTARROW) {
      Serial.print("[Right]");
    } else if (c == PS2_UPARROW) {
      Serial.print("[Up]");
    } else if (c == PS2_DOWNARROW) {
      Serial.print("[Down]");
    } else if (c == PS2_DELETE) {
      Serial.print("[Del]");
    } else {
      
      // otherwise, just print all normal characters
      Serial.print(c); 
    }*/
}
