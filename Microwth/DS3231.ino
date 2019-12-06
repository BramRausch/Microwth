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
