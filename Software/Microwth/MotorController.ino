bool beginMotorController(){
    Wire.beginTransmission(0x41);  // Select PCA9536
    Wire.write(0x01);  // Select output registry
    Wire.write(0xF0);  // configure all output as LOW
    Wire.endTransmission(); 

    Wire.beginTransmission(0x41);  // Select PCA9536
    Wire.write(0x03);  // Select control registry
    Wire.write(0xF0);  // set all pins to output
    return Wire.endTransmission();   
}

bool writeMotorController(byte output){
    Wire.beginTransmission(0x41);  // Select PCA9536
    Wire.write(0x01);  // Select output registry
    Wire.write(0xF0 | output);  // set 
    return Wire.endTransmission(); 
}
