#include <Arduino.h>
#include <WiFi.h>  // Include the Wi-Fi library for ESP32

// Replace with your network credentials
const char* ssid = "Test";        // Your WiFi SSID
const char* password = "876543218"; // Your WiFi Password

void setup() {
  Serial.begin(115200); // Start Serial Monitor at 115200 baud
  Serial.println("Connecting to WiFi...");

  WiFi.begin(ssid, password); // Start connecting to WiFi

  // Wait until connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // Print the IP Address assigned to ESP32
}

void loop() {
  // Periodically print the Wi-Fi signal strength
  Serial.print("WiFi Signal Strength (RSSI): ");
  Serial.println(WiFi.RSSI());
  delay(5000); // Wait for 5 seconds
}

