#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_BME280 bme;

const byte buttonPin[] = {0,13,12,2}; // UP, DOWN, LEFT, RIGHT
const byte lights = 15;
byte hour, minute, second;
byte hrs, mins;
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

void setup() {
  Serial.begin(9600);

  // Initialize IO
  Wire.begin(4, 5);
  for (byte i=0; i<sizeof(buttonPin); i++) pinMode(buttonPin[i], INPUT_PULLUP);
  pinMode(lights, OUTPUT);
  digitalWrite(lights, 0);

  // Retrieve lighting timing
  EEPROM.begin(4);
  lightOn = EEPROM.read(0)*60+EEPROM.read(1);
  lightOff = EEPROM.read(2)*60+EEPROM.read(3);

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
  checkSwitchTime();
}

void loop() {
  currentTime = hour*60+minute;
  checkSwitchTime();
  
  switch(select){
    case 0:  // display time
      readClockTime();
      displayTime(hour, minute);

      if (button(0, 0)) select = 20;  // up
      if (button(1, 0)) select = 10;  // down
    break;
    case 10:  // display sensor data
      humidity = bme.readHumidity();
      temp = bme.readTemperature();
      displayBME280(temp, humidity);
      Serial.println(temp);

      if (button(0, 0)) select = 0;  // up
      if (button(1, 0)) select = 20;  // down
    break;
    case 20:  // settings menu select
      displayText("Settings");

      if (button(0, 0)) select = 10;  // up
      if (button(1, 0)) select = 0;  // down
      if (button(3, 0)) select = 210;  // right
    break;
    case 210:  // settings - set time
      displayText("Set time");
      
      if (button(0, 0)) select = 230;  // up
      if (button(1, 0)) select = 220;  // down
      if (button(2, 0)) select = 20;  // left
      if (button(3, 0)) select = 215;  // right
    break;
    case 215:  // set time
      hrs = hour;
      mins = minute;
      if (setStep == 0){
        setHour(button(0, 1) - button(1, 1));
        hour = hrs;
        displayBinkingTime(hour, minute, 0);
        if (button(2, 0)) select = 210;  // left
        if (button(3, 0)) setStep = 1;  // right
      } else if(setStep == 1){
        setMinute(button(0, 1) - button(1, 1));
        minute = mins;
        displayBinkingTime(hour, minute, 1);
        if (button(2, 0)) setStep = 0;  // left
        if (button(3, 0)) {  // right
          second = 0;
          setClockTime();
          setStep = 0;
          select = 0;  
        }
      }
    break;
    case 220:  // settings - lights on
      displayText("Light on");
      
      if (button(0, 0)) select = 210;  // up
      if (button(1, 0)) select = 230;  // down
      if (button(2, 0)) select = 20;  // left
      if (button(3, 0)) select = 225;  // right
    break;
    case 225:  // set light on
      hrs = lightOn/60%24;
      mins = lightOn%60;
      if (setStep == 0){
        setHour(button(0, 1) - button(1, 1));
        lightOn = hrs*60+mins;
        displayBinkingTime(hrs, mins, 0);
        if (button(2, 0)) select = 220;  // left
        if (button(3, 0)) setStep = 1;  // right
      } else if(setStep == 1){
        setMinute(button(0, 1) - button(1, 1));
        lightOn = hrs*60+mins;
        EEPROM.write(0, hrs);
        EEPROM.write(1, mins);
        EEPROM.commit();
        displayBinkingTime(hrs, mins, 1);
        if (button(2, 0)) setStep = 0;  // left
        if (button(3, 0)) {  // right
          setStep = 0;
          select = 0;  
        }
      }
    break;
    case 230:  // settings - light off
      displayText("Light off");
    
      if (button(0, 0)) select = 220;  // up
      if (button(1, 0)) select = 210;  // down
      if (button(2, 0)) select = 20;  // left
      if (button(3, 0)) select = 235;  // right
    break;
    case 235:
      hrs = lightOff/60%24;
      mins = lightOff%60;
      if (setStep == 0){
        setHour(button(0, 1) - button(1, 1));
        lightOff = hrs*60+mins;
        displayBinkingTime(hrs, mins, 0);
        if (button(2, 0)) select = 230;  // left
        if (button(3, 0)) setStep = 1;  // right
      } else if(setStep == 1){
        setMinute(button(0, 1) - button(1, 1));
        lightOff = hrs*60+mins;
        EEPROM.write(2, hrs);
        EEPROM.write(3, mins);
        EEPROM.commit();
        displayBinkingTime(hrs, mins, 1);
        if (button(2, 0)) setStep = 0;  // left
        if (button(3, 0)) {  // right
          setStep = 0;
          select = 0;  
        }
      }
    break;
    case 240:  // setting - sensor calibrate

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

void displayText(String text){
  display.clearDisplay();
  display.setTextSize(2);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 12);
  display.print(text);
  display.display();
}

void displayBinkingTime(int hour, int minute, bool select){
  display.clearDisplay();
  display.setTextSize(2);      // Normal 1:1 pixel scale
  display.setCursor(0, 12);     // Start at top-left corner
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
  display.setTextSize(2);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 12);     // Start at top-left corner
  print0(hour);
  display.print(F(":"));
  print0(minute);
  display.display();
}

void displayBME280(float temp, float humidity){
  display.clearDisplay();
  display.setTextSize(2);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 12);     // Start at top-left corner
  print0(temp);
  display.print("c ");
  print0(humidity);
  display.print("%");
  display.display();
}

void checkSwitchTime(){
  if (lightOn > lightOff){
    if ((currentTime >= lightOn && currentTime <= 1440) || (currentTime < lightOff && currentTime > 0)){
      digitalWrite(lights, HIGH);
    } else {
      digitalWrite(lights, LOW);
    }
  } else if(lightOn < lightOff){
    if (currentTime >= lightOn && currentTime < lightOff){
      digitalWrite(lights, HIGH);
    } else {
      digitalWrite(lights, LOW);
    }
  } else if(lightOn == lightOff){
    digitalWrite(lights, HIGH);
  }  
}

boolean button(byte k, boolean repeatFlg)
{ 
  const  unsigned long debounce  =   50;
  const  unsigned long interval1 =  500;
  const  unsigned long interval2 =  100;

  static unsigned long buttonTime[sizeof(buttonPin)];
  static boolean       buttonFlg[sizeof(buttonPin)];
  static unsigned long repeatTime[sizeof(buttonPin)];
  static unsigned long interval[sizeof(buttonPin)];

  if (!digitalRead(buttonPin[k]))
  {                            // knop ingedrukt
    buttonTime[k]=millis();
    if (!buttonFlg[k])
    {
      buttonFlg[k]=1;
      repeatTime[k]=millis();
      return(1);
    }
    if (repeatFlg && ((millis()-repeatTime[k])>interval[k]))           // repeatFlg en interval controle
    {
      buttonFlg[k]=0;    
      interval[k]=interval2; 
    }
  }
  else 
  {                            // knop los
    if ((millis()-buttonTime[k])>debounce)
    {
      buttonFlg[k]=0;
      interval[k]=interval1; 
    }
  }
  return(0);
}

void print0(int i)            // lcd print parameter byte i, wanneer i<10, dan eerst een voorloop '0'
{
  if (i<10) display.print("0");
  display.print(i);
}

// DS3231 klokmodule..

void initClock()
{
  Wire.beginTransmission(0x68);             // DS3231 device address
  Wire.write(0x0E);                         // select register 0Eh
  Wire.write(0b00100000);                   // zie DS3231 datasheet, 1 Hz blok, bit 5 force temp conversion
  Wire.endTransmission();
}

void readClockTime()
{                                           // send request to receive data
  Wire.beginTransmission(0x68);     
  Wire.write(0x00);                         // start at register 00h
  Wire.endTransmission();
  Wire.requestFrom(0x68, 3);                // request 3 bytes (00h t/m 02h)
  while (Wire.available()<3) ;              // wait for data.. 
  second = bcd2dec(Wire.read() & 0x7F);     // masker 0111 1111
  minute = bcd2dec(Wire.read() & 0x7F);     // in DS3231 zijn gegevens opgeslagen in bcd-formaat, dus converteren! 
  hour   = bcd2dec(Wire.read() & 0x3F);     // masker 0011 1111
}

void setClockTime()
{
  Wire.beginTransmission(0x68);             // DS3231 device address
  Wire.write(0x00);                         // select register 00h
  Wire.write(dec2bcd(second));
  Wire.write(dec2bcd(minute));
  Wire.write(dec2bcd(hour));
  Wire.endTransmission();
}

// registerwaarden van DS3231 module zijn in bcd-formaat.. conversie functies:
byte bcd2dec(byte num)  { return ((num/16 * 10) + num%16); }    // conversie bcd-getal -> decimaal
byte dec2bcd(byte num)  { return ((num/10 * 16) + num%10); }    // conversie decimaal  -> bcd-getal
