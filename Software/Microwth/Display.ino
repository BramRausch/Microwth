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

void print0(int i) {  // Function to print leading zero
  if (i<10) display.print("0");
  display.print(i);
}
