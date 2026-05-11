#include <NimBLEDevice.h>
#include <Arduino.h>
#include <BleGamepad.h>
#include <U8g2lib.h>
#include <Wire.h>

// I am using 'U8G2' library for the OLED display.
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, /* clock*/ 22, /* data*/ 21);

// 'NimBLE-Arduino' library by 'h2zero'
// You can change the *Device name, and *Manufacturer name as per your requirment.
// I have used these 'names' because steam has built in configuration for xbox controllers
// {*changing the names should not affect the performance of the controller*}
BleGamepad bleGamepad(/*Device name*/"Xbox Wireless Controller", /*Manufacturer*/"Microsoft", /*battery percentage*/100);

// Pin Definitions
const int JOY_L_X = 33;
const int JOY_L_Y = 32;
const int JOY_L_BT = 25;

const int JOY_R_X = 39;
const int JOY_R_Y = 36;
const int JOY_R_BT = 35;

const int POT_PIN = 34;

const int BTN_UP = 27;
const int BTN_SEL = 14;
const int BTN_DWN = 13;
const int BTN_4 = 17;

const int BUZZER = 26;

int temp = 0;
const char* connect = "OFFLINE";

void setup() {
  Serial.begin(115200);
  // Pin Modes
  pinMode(JOY_R_X, INPUT);
  pinMode(JOY_R_Y, INPUT);
  pinMode(JOY_L_X, INPUT);
  pinMode(JOY_L_Y, INPUT);
  pinMode(POT_PIN, INPUT);

  pinMode(JOY_L_BT, INPUT_PULLUP);
  pinMode(JOY_R_BT, INPUT);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_SEL, INPUT_PULLUP);
  pinMode(BTN_DWN, INPUT_PULLUP);
  pinMode(BTN_4, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  // Initialize OLED
  u8g2.begin();

  // Start Bluetooth
  bleGamepad.begin();
  
  // Startup Screen
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_profont17_tr); 
  u8g2.drawStr(10, 35, "Booting OS...");
  u8g2.sendBuffer();
  
  //Turn ON buzzer
  tone(BUZZER, 1500);
  delay(100);
  tone(BUZZER, 1000);
  delay(80);
  noTone(BUZZER);

  delay(1000); //Give the controller time to setup properly
}

void drawInterface(int lx, int ly, int rx, int ry, int pot, bool b1, bool b2, bool b3, bool b4, const char* connect) {
  u8g2.clearBuffer();
  
  // Header
  u8g2.setFont(u8g2_font_6x10_tf); 
  u8g2.drawStr(0, 10, "SYS:");
  u8g2.setCursor(40, 10);
  u8g2.print(connect);
  u8g2.drawFrame(35, 0, 50, 12); //Box around connection 

  // Left Joystick display
  u8g2.drawStr(0, 25, "LX:"); 
  u8g2.drawFrame(25, 18, 35, 7);
  u8g2.drawBox(25, 18, map(lx, 0, 32767, 0, 35), 7);
  
  u8g2.drawStr(0, 35, "LY:");
  u8g2.drawFrame(25, 28, 35, 7);
  u8g2.drawBox(25, 28, map(ly, 0, 32767, 0, 35), 7);

  // Right Joystick display
  u8g2.drawStr(65, 25, "RX:");
  u8g2.drawFrame(90, 18, 35, 7);
  u8g2.drawBox(90, 18, map(rx, 0, 32767, 0, 35), 7);
  
  u8g2.drawStr(65, 35, "RY:");
  u8g2.drawFrame(90, 28, 35, 7);
  u8g2.drawBox(90, 28, map(ry, 0, 32767, 0, 35), 7);

  // Potentiometer display
  u8g2.drawStr(0, 50, "PWR:");
  u8g2.drawFrame(30, 43, 98, 7);
  u8g2.drawBox(30, 43, map(pot, 0, 32767, 0, 98), 7);

  // Button display
  u8g2.drawStr(0, 62, b1 ? "[U]" : " U ");
  u8g2.drawStr(30, 62, b2 ? "[L]" : " L ");
  u8g2.drawStr(60, 62, b3 ? "[D]" : " D ");
  u8g2.drawStr(90, 62, b4 ? "[R]" : " R ");

  u8g2.sendBuffer();
}

//For stablization of values
int getStableAxis(int currentVal, int middleVal, int threshold) {
  int offset = abs(currentVal - middleVal);

  if (offset <= threshold) {
    return middleVal; 
  }
  return currentVal;
}

void loop() {
    // Read and Map Analog Inputs (0-4095 to 0-32767)
    int valLX = map(analogRead(JOY_L_X), 0, 4095, 32767, 0);
    int valLY = map(analogRead(JOY_L_Y), 0, 4095, 0, 32767);
    int valRX = map(analogRead(JOY_R_X), 0, 4095, 32767, 0);
    int valRY = map(analogRead(JOY_R_Y), 0, 4095, 0, 32767);
    
    // Stablize value
    valLX = getStableAxis(valLX, 17800, 300); //300 These are the values for my controller.
    valLY = getStableAxis(valLY, 15300, 200); //200 Make sure to calibrate yours.
    valRX = getStableAxis(valRX, 17600, 200); //200
    valRY = getStableAxis(valRY, 14700, 200); //200

    int valPot = map(analogRead(POT_PIN), 0, 4095, 0, 32767);

    // Read Buttons (Active Low)
    bool b1 = !digitalRead(BTN_UP);
    bool b2 = !digitalRead(BTN_SEL);
    bool b3 = !digitalRead(BTN_DWN);
    bool b4 = !digitalRead(BTN_4);

    if(bleGamepad.isConnected()) {
      //Set connection to ONLINE
      connect = "ONLINE";

      //Connected Buzzer
      if(temp == 0) {
      tone(BUZZER, 1000);
      delay(100);
      tone(BUZZER, 1500);
      delay(80);
      noTone(BUZZER);
      temp = 1;
      }

      // Send Bluetooth Data
      bleGamepad.setLeftThumb(valLX, valLY);
      bleGamepad.setRightThumb(valRX, valRY);
      bleGamepad.setSlider(valPot); // Using Potentiometer as a Trigger/Slider

      if(b1) bleGamepad.press(BUTTON_1); else bleGamepad.release(BUTTON_1);
      if(b2) bleGamepad.press(BUTTON_2); else bleGamepad.release(BUTTON_2);
      if(b3) bleGamepad.press(BUTTON_3); else bleGamepad.release(BUTTON_3);
      if(b4) bleGamepad.press(BUTTON_4); else bleGamepad.release(BUTTON_4);
    
      bleGamepad.sendReport();
    }
    else {
      //Ready BUZZER for reconnection
      temp = 0;
      //Set connection to OFFLINE
      connect = "OFFLINE";
    }

    // Update OLED Display
    drawInterface(valLX, valLY, valRX, valRY, valPot, b1, b2, b3, b4, connect);
  
  //Increase the "20" if the outputs keep fluctuating. It is set according to my controller.
  delay(20); 
}
