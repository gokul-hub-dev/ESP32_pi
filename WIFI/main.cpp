#include <Arduino.h>
void setup() {
  // Start the Serial Monitor at a baud rate of 115200
  Serial.begin(115200);
  
  // Print an initial message to the Serial Monitor
  Serial.println("ESP32 is ready. Please enter a message:");
}
void loop() {
  Serial.println("working.........!!!");
  delay(1000);
}
