#include <EEPROM.h>
#define EEPROM_SIZE 1
const int RelayPin = 25;    // the number of the pushbutton pin

void setup() { 
  Serial.begin(115200);
  
  EEPROM.begin(EEPROM_SIZE);
  pinMode(RelayPin, INPUT_PULLUP);
}

void loop() {

  if (0 == EEPROM.read(0))
  {
    Serial.println("Waiting for relay triggering......");
    while(digitalRead(RelayPin));
    EEPROM.write(0,1);
    EEPROM.commit();
    Serial.println("EEPROM set successfully");
  }
  else if(1 == EEPROM.read(0))
  {
    delay(2000);
    EEPROM.write(0,0);
    EEPROM.commit(); 
    Serial.println("EEPROM reset Successfully");
  }
}
