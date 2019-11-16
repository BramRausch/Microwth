#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_SSD1306.h>

#define UP 0
#define DOWN 13
#define LEFT 12
#define RIGHT 2
#define LIGHTS 15
#define OLED_RESET 14

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_BME280 bme;

const byte buttonPin[] = {UP,DOWN,LEFT,RIGHT}; // Array of button pins
byte hour, minute, second;  // Current time
byte hrs, mins;  // Variables to seperate the current time from the new settings
unsigned long selectTime =  0;
unsigned long timer1 =  0;
int select = 0;
int steps = 0;
byte setStep = 0;
bool toggle = 0;
int lightOn = 0;
int lightOff = 0;
int currentTime = 0;
float humidity;
float temp;
int pumpInt;
byte pumpTime;
int pumpSet;

void setup() {
  Serial.begin(9600);

  // Initialize IO
  Wire.begin(4, 5);
  for (byte i=0; i<sizeof(buttonPin); i++) pinMode(buttonPin[i], INPUT_PULLUP);
  pinMode(LIGHTS, OUTPUT);
  digitalWrite(LIGHTS, 0);

  // Retrieve settings from EEPROM
  EEPROM.begin(4);
  lightOn = EEPROM.read(0)*60+EEPROM.read(1);
  lightOff = EEPROM.read(2)*60+EEPROM.read(3);
  pumpInt = EEPROM.read(4)*60+EEPROM.read(5);  // Pump interval
  pumpTime = EEPROM.read(6);  // Pump amount
  pumpSet = EEPROM.read(7)*60+EEPROM.read(8);  // Time when the grow was started
  

  // Initialize display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  display.clearDisplay();

  // Initialize BME280
  bme.begin();
  humidity = bme.readHumidity();
  temp = bme.readTemperature();
    
  // Initialize clock
  initClock();
  readClockTime();
  checkTriggers();
}

void loop() {
  currentTime = hour*60+minute;
  checkTriggers();
  
  switch(select){
    case 0:  // display time
      readClockTime();
      displayTime(hour, minute);

      // Menu navigation options
      if (button(0, 0)) select = 30;  // up
      if (button(1, 0)) select = 10;  // down
    break;
    case 10:  // display sensor data
      humidity = bme.readHumidity();
      temp = bme.readTemperature();
      displayBME280(temp, humidity);
      
      // Menu navigation options
      if (button(0, 0)) select = 0;  // up
      if (button(1, 0)) select = 20;  // down
    break;
    case 20:  // Set time option text
      displayText("Set time", 2);

      // Menu navigation options
      if (button(0, 0)) select = 10;  // up
      if (button(1, 0)) select = 30;  // down
      if (button(3, 0)) select = 210;  // right
    break;
    case 30:  // Start grow option text
      displayText("Start grow", 2);

      // Menu navigation options
      if (button(0, 0)) select = 20;  // up
      if (button(1, 0)) select = 0;  // down
      if (button(3, 0)) select = 220;  // right
    break;
    case 210:  // set time
      hrs = hour;
      mins = minute;
      if (setStep == 0){
        setHour(button(0, 1) - button(1, 1));
        hour = hrs;
        displayBinkingTime(hour, minute, 0);

        // Menu navigation options
        if (button(2, 0)) select = 20;  // left
        if (button(3, 0)) setStep = 1;  // right
      } else if(setStep == 1){
        setMinute(button(0, 1) - button(1, 1));
        minute = mins;
        displayBinkingTime(hour, minute, 1);

        // Menu navigation options
        if (button(2, 0)) setStep = 0;  // left
        if (button(3, 0)) {  // right
          second = 0;
          setClockTime();
          setStep = 0;
          select = 0;  
        }
      }
    break;
    case 220:  // Setting lights on
      hrs = lightOn/60%24;
      mins = lightOn%60;
      if (setStep == 0){
        setHour(button(0, 1) - button(1, 1));
        lightOn = hrs*60+mins;
        displayBinkingTime(hrs, mins, 0);

        // Menu navigation options
        // if (button(2, 0)) select = 0;  // left can't go back
        if (button(3, 0)) setStep = 1;  // right
      } else if(setStep == 1){
        setMinute(button(0, 1) - button(1, 1));
        lightOn = hrs*60+mins;
        EEPROM.write(0, hrs);
        EEPROM.write(1, mins);
        EEPROM.commit();
        displayBinkingTime(hrs, mins, 1);

        // Menu navigation options
        if (button(2, 0)) setStep = 0;  // left
        if (button(3, 0)) {  // right
          setStep = 0;
          select = 230;  
        }
      }
    break;
    case 230:  // Setting lights off
      hrs = lightOff/60%24;
      mins = lightOff%60;
      if (setStep == 0){
        setHour(button(0, 1) - button(1, 1));
        lightOff = hrs*60+mins;
        displayBinkingTime(hrs, mins, 0);

        // Menu navigation options
        if (button(2, 0)) select = 220;  // left
        if (button(3, 0)) setStep = 1;  // right
      } else if(setStep == 1){
        setMinute(button(0, 1) - button(1, 1));
        lightOff = hrs*60+mins;
        
        displayBinkingTime(hrs, mins, 1);

        // Menu navigation options
        if (button(2, 0)) setStep = 0;  // left
        if (button(3, 0)) {  // right
          setStep = 0;
          select = 240;  

          // Only write EEPROM when ready
          EEPROM.write(2, hrs);
          EEPROM.write(3, mins);
          EEPROM.commit();
        }
      }
    break;
    case 240:  // Setting pump interval
      hrs = pumpInt/60%24;
      mins = pumpInt%60;
      if (setStep == 0){
        setHour(button(0, 1) - button(1, 1));
        pumpInt = hrs*60+mins;
        displayBinkingTime(hrs, mins, 0);

        // Menu navigation options
        if (button(2, 0)) select = 230;  // left
        if (button(3, 0)) setStep = 1;  // right
      } else if(setStep == 1){
        setMinute(button(0, 1) - button(1, 1));
        pumpInt = hrs*60+mins;
        
        displayBinkingTime(hrs, mins, 1);

        // Menu navigation options
        if (button(2, 0)) setStep = 0;  // left
        if (button(3, 0)) {  // right
          setStep = 0;
          select = 250;  

          // Only write EEPROM when ready
          EEPROM.write(4, hrs);
          EEPROM.write(5, mins);
          EEPROM.commit();
        }
      }
    break;
    case 250:  // Setting pump time
      pumpTime = (pumpTime + button(0, 1) - button(1, 1));

      String con = String(String(pumpTime, DEC) + " s");
      displayText(con, 2);

      if (button(2, 0)) select = 240;  // left
      if (button(3, 0)) {
        select = 0;  // right
        EEPROM.write(6, pumpTime);

        // Store current time
        pumpSet = currentTime;
        EEPROM.write(7, hour);
        EEPROM.write(8, minute);
        EEPROM.commit();
      }
    break;
  }
}

void setHour(int steps){
  if (steps == 1) {
    if (hrs == 23) {
      hrs = 0;
    } else{
      hrs++;
    }
  }
  if (steps == -1) {
    if (hrs == 0) {
      hrs = 23;
    } else{
      hrs--;
    }
  }
}

void setMinute(int steps){
  if (steps == 1) {
    if (mins == 59) {
      mins = 0;
    } else{
      mins++;
    }
  }
  if (steps == -1) {
    if (minute == 0) {
      mins = 59;
    } else{
      mins--;
    }
  }
}

void displayText(String text, byte textSize){
  display.clearDisplay();
  display.setTextSize(textSize);
  display.setTextColor(WHITE); // Draw white text
  display.setCursor((SCREEN_WIDTH - ((text.length() * 6) - 1) * textSize) / 2, 12);  // Center text
  display.print(text);
  display.display();
}

void displayBinkingTime(int hour, int minute, bool select){
  display.clearDisplay();
  display.setTextSize(0.5);
  
  display.setTextSize(2);
  display.setCursor((SCREEN_WIDTH - 62) / 2, 12);  // Center text
  display.setTextColor(WHITE);

  if((millis()-timer1 > 250 && toggle == 0) || (millis()-timer1 > 750 && toggle == 1)){
    timer1 = millis();
    toggle = !toggle;
  }

  if (!select && !toggle){
    display.setTextColor(BLACK);
  } else if (!select && toggle){
    display.setTextColor(WHITE);
  }
  print0(hour);
  display.setTextColor(WHITE);
  display.print(F(":"));
  if (select && !toggle){
    display.setTextColor(BLACK);
  } else if (select && toggle){
    display.setTextColor(WHITE);
  }
  print0(minute);
  display.display();
}

void displayTime(int hour, int minute){
  display.clearDisplay();
  display.setTextSize(2);        
  display.setTextColor(WHITE);  // Draw white text
  display.setCursor((SCREEN_WIDTH - 62) / 2, 12);  // Center text
  print0(hour);
  display.print(F(":"));
  print0(minute);
  display.display();
}

void displayBME280(float temp, float humidity){
  display.clearDisplay();
  display.setTextSize(2);        
  display.setTextColor(WHITE);   // Draw white text
  display.setCursor((SCREEN_WIDTH - 83) / 2, 12);  // Center text
  print0(temp);
  display.print("c ");
  print0(humidity);
  display.print("%");
  display.display();
}

void checkTriggers(){
  // Check light trigger time
  if (lightOn > lightOff){  // Light switch condition when the switch on time is larger then the light off time
    if ((currentTime >= lightOn && currentTime <= 1440) || (currentTime < lightOff && currentTime > 0)){
      digitalWrite(LIGHTS, HIGH);
    } else {
      digitalWrite(LIGHTS, LOW);
    }
  } else if(lightOn < lightOff){  // Light switch condition when the switch on time is smaller then the light off time
    if (currentTime >= lightOn && currentTime < lightOff){
      digitalWrite(LIGHTS, HIGH);
    } else {
      digitalWrite(LIGHTS, LOW);
    }
  } else if(lightOn == lightOff){  // Keep the light on when the on and off time are equal
    digitalWrite(LIGHTS, HIGH);
  }
 
  // Variables to keep track of the pump
  static bool pumpOn;
  static unsigned long int pumpTimer1;

  // change to : currentime-settime%interval
  if ((currentTime - pumpSet) % 1 == 0 && !pumpOn) {  // When an interval is reached from the time the interval has been set
    // turn pump on
    pumpOn = 1;
    pumpTimer1 = millis();
    Serial.println("pump on");
  } else if (pumpOn && millis()-pumpTimer1*1000 >= pumpTime) {  // If the pump has been on the set ammount off time
    // turn pump off
    Serial.println("pump off");
  } else if ((currentTime - pumpSet) % 480 != 0 && millis()-pumpTimer1*1000 && pumpOn){  // If the timer is done and the time has changed reset pumpOn
    pumpOn = 0;  
  }
}

boolean button(byte k, boolean repeatFlg) { 
  const  unsigned long debounce  =   50;
  const  unsigned long interval1 =  500;
  const  unsigned long interval2 =  100;

  static unsigned long buttonTime[sizeof(buttonPin)];
  static boolean       buttonFlg[sizeof(buttonPin)];
  static unsigned long repeatTime[sizeof(buttonPin)];
  static unsigned long interval[sizeof(buttonPin)];

  if (!digitalRead(buttonPin[k])) {      
    buttonTime[k]=millis();
    if (!buttonFlg[k]) {
      buttonFlg[k]=1;
      repeatTime[k]=millis();
      return(1);
    }
    if (repeatFlg && ((millis()-repeatTime[k])>interval[k])) {
      buttonFlg[k]=0;    
      interval[k]=interval2; 
    }
  }
  else {
    if ((millis()-buttonTime[k])>debounce) {
      buttonFlg[k]=0;
      interval[k]=interval1; 
    }
  }
  return(0);
}

void print0(int i) {  // Function to print leading zero
  if (i<10) display.print("0");
  display.print(i);
}

void initClock() {
  Wire.beginTransmission(0x68);             // DS3231 device address
  Wire.write(0x0E);                         // select register 0Eh
  Wire.write(0b00100000);
  Wire.endTransmission();
}

void readClockTime() {                                           // send request to receive data
  Wire.beginTransmission(0x68);     
  Wire.write(0x00);                         // start at register 00h
  Wire.endTransmission();
  Wire.requestFrom(0x68, 3);                // request 3 bytes (00h t/m 02h)
  while (Wire.available()<3) ;              // wait for data.. 
  second = bcd2dec(Wire.read() & 0x7F);     // mask 0111 1111
  minute = bcd2dec(Wire.read() & 0x7F);     // Convert BCD values to decimal
  hour   = bcd2dec(Wire.read() & 0x3F);     // mask 0011 1111
}

void setClockTime() {
  Wire.beginTransmission(0x68);             // DS3231 device address
  Wire.write(0x00);                         // select register 00h
  Wire.write(dec2bcd(second));
  Wire.write(dec2bcd(minute));
  Wire.write(dec2bcd(hour));
  Wire.endTransmission();
}

byte bcd2dec(byte num)  { return ((num/16 * 10) + num%16); }    // conversion bcd -> decimal
byte dec2bcd(byte num)  { return ((num/10 * 16) + num%10); }    // conversion decimal  -> bcd
