#include <WiFi.h>
#include "time.h"
#include "esp_sntp.h"
#include <Wire.h>
#include <hd44780.h>         // Include HD44780 library for I2C LCD
#include <hd44780ioClass/hd44780_I2Cexp.h>  // Include the I2C expansion for HD44780

// LCD Setup
hd44780_I2Cexp lcd(0x27, 20, 4);  // I2C Address 0x27 for 20x4 LCD

// NTP Server Configuration
const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 3600 * 8;       // Change depending on your time zone
const int daylightOffset_sec = 3600;      // Daylight savings offset in seconds

// Time Zone Configuration (optional)
const char *time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";

// Location Information
const char *location = " "; // Enter your location here

// Buffers to store last displayed date and time
char lastDisplayedDate[20] = " ";
char lastDisplayedTime[20] = " ";

// Function to Print Local Time to LCD
void printLocalTime() {
  struct tm timeinfo;

  // Check if time is available
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time!");
    return;
  }

  // Prepare date and time strings
  char currentDate[20];
  char currentTime[20];
  strftime(currentDate, sizeof(currentDate), "%B %d %Y", &timeinfo);
  strftime(currentTime, sizeof(currentTime), "%A %H:%M:%S", &timeinfo);

  // Update Date on LCD if it has changed
  if (strcmp(lastDisplayedDate, currentDate) != 0) {
    lcd.setCursor(0, 0);
    lcd.print(" ");  // Clear row
    lcd.setCursor(0, 0);
    lcd.print(currentDate);
    strncpy(lastDisplayedDate, currentDate, sizeof(lastDisplayedDate));
  }

  // Update Time on LCD if it has changed
  if (strcmp(lastDisplayedTime, currentTime) != 0) {
    lcd.setCursor(0, 1);
    lcd.print(" ");  // Clear row
    lcd.setCursor(0, 1);
    lcd.print(currentTime);
    strncpy(lastDisplayedTime, currentTime, sizeof(lastDisplayedTime));
  }

  // Display Location (static, does not change)
  lcd.setCursor(0, 2);
  lcd.print(" ");  // Clear row
  lcd.setCursor(0, 2);
  lcd.print(location);
}

// Callback Function for NTP Time Synchronization
void timeSyncCallback(struct timeval *t) {
  Serial.println("NTP Time Synchronized!");
  printLocalTime();
}

// Wi-Fi Selection and Connection
void connectToWiFi() {
  Serial.println("Scanning for Wi-Fi Networks...");
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
  int networkIndex = -1;

  while (networkIndex == -1) {
    if (Serial.available()) {
      String input = Serial.readStringUntil('\n');
      networkIndex = input.toInt() - 1;

      if (networkIndex >= 0 && networkIndex < networkCount) {
        String selectedSSID = WiFi.SSID(networkIndex);
        Serial.printf("Selected Network: %s\n", selectedSSID.c_str());
        Serial.println("Enter WiFi Password:");

        // Wait for the password
        while (!Serial.available());
        String password = Serial.readStringUntil('\n');
        password.trim();

        if (!password.isEmpty()) {
          Serial.printf("Connecting to %s...\n", selectedSSID.c_str());
          WiFi.begin(selectedSSID.c_str(), password.c_str());

          int timeout = 15000; // 15-second timeout
          unsigned long startAttemptTime = millis();

          while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
            delay(500);
            Serial.print(".");
          }

          if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nWi-Fi Connected!");
            Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
            return;
          } else {
            Serial.println("\nConnection failed. Please try again.");
          }
        } else {
          Serial.println("Password cannot be empty. Please try again:");
        }
        networkIndex = -1; // Restart the selection
      } else {
        Serial.println("Invalid selection, try again:");
        networkIndex = -1;
      }
    }
  }
}

// Setup Function
void setup() {
  // Serial and LCD Initialization
  Serial.begin(115200);
  lcd.begin(20, 4);  // Initialize LCD (20 columns and 4 rows)
  lcd.setBacklight(1);  // Turn on the backlight

  // Wi-Fi Selection and Connection
  connectToWiFi();

  // Configure NTP and Time Synchronization
  esp_sntp_servermode_dhcp(1);  // Optional: Enable DHCP for NTP server
  sntp_set_time_sync_notification_cb(timeSyncCallback);  // Set callback
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);  // Configure NTP
}

// Loop Function
void loop() {
  delay(1000);        // Update time every second
  printLocalTime();   // Print updated time to LCD
}
