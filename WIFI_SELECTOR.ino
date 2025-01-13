#include <WiFi.h>
// hello world
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("WiFi Selection");
  Serial.println("Scanning for WiFi Networks...");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(500);

  int networkCount = WiFi.scanNetworks();
  Serial.printf("%d networks found:\n", networkCount);

  for (int i = 0; i < networkCount; i++) {
    Serial.printf("%d. %s (Signal Strength: %d dBm)\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
  }

  if (networkCount == 0) {
    Serial.println("No networks found. Restart and try again.");
    while (true);
  }

  Serial.println("\nEnter the number of the network you want to connect to:");
}

void loop() {
  static int step = 0;
  static String selectedSSID = "";
  static String password = "";

  if (step == 0 && Serial.available()) {
    int networkIndex = Serial.parseInt() - 1;

    if (networkIndex >= 0 && networkIndex < WiFi.scanNetworks()) {
      selectedSSID = WiFi.SSID(networkIndex);
      Serial.printf("Selected Network: %s\n", selectedSSID.c_str());
      Serial.println("Enter WiFi Password:");
      step = 1;
    } else {
      Serial.println("Invalid selection, try again:");
    }
  } else if (step == 1 && Serial.available()) {
    password = Serial.readStringUntil('\n');
    password.trim();

    if (password.isEmpty()) {
      Serial.println("Password cannot be empty. Please try again:");
      return; // Retry password input
    }

    Serial.printf("Connecting to %s...\n", selectedSSID.c_str());
    WiFi.begin(selectedSSID.c_str(), password.c_str());

    int timeout = 15000; // 15-second timeout
    unsigned long startAttemptTime = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
      delay(500);
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected!");
      Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
      step = 2; // Move to a connected state
    } else {
      Serial.println("\nConnection failed. Please try again.");
      step = 0; // Restart the process
    }
  }
}
