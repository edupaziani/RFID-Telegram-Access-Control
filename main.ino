#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include "secrets.h" // Credentials file (SSID, Password, BotToken, ChatID)

// RFID Pin Mapping (ESP32 DevKit V1)
#define SS_PIN 5
#define RST_PIN 4
#define SCK_PIN 18
#define MOSI_PIN 23
#define MISO_PIN 19

// Objects initialization
MFRC522 rfid(SS_PIN, RST_PIN);

// User data structure for access control
struct User {
  String id;
  String name;
  bool isPresent;
  unsigned long entryTime;
  unsigned long sessionDuration;
  unsigned long totalTime;
};

// Global Variables
const int MAX_USERS = 20;
User users[MAX_USERS];
int totalUsers = 0;

// Report and logic control flags
bool isShowingReport = false;
unsigned long reportStartTime = 0;
const unsigned long REPORT_TIMEOUT_MS = 10000; // 10 seconds
bool reportDisplayed = false;

void setup() {
  Serial.begin(115200);

  // Initialize SPI Bus and RFID Module
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  rfid.PCD_Init();

  // Connect to Wi-Fi network
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  Serial.println(WiFi.localIP());
  
  Serial.println("Access Control System Started. Waiting for tags...");
}

void loop() {
  // Check for new RFID tags
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    delay(500);
    return;
  }

  // Look for user index based on UID
  int index = findUserIndexByUID(rfid.uid.uidByte, rfid.uid.size);

  if (index >= 0) {
    registerMovement(index);
    sendTelegramNotification("🔄 Movement recorded for " + users[index].name);
  } else {
    // Register new user if UID is not found
    String uidStr = generateUIDString(rfid.uid.uidByte, rfid.uid.size);
    registerNewUser(uidStr, "User_" + String(totalUsers + 1));
    sendTelegramNotification("🆕 New user registered: " + uidStr);
  }

  // Halt RFID communication to avoid multiple reads
  rfid.PICC_HaltA();
  delay(2000);
}

// Converts raw UID bytes to a readable HEX string (e.g., AA:BB:CC:DD)
String generateUIDString(byte *uid, byte size) {
  String uidStr = "";
  for (byte i = 0; i < size; i++) {
    uidStr += String(uid[i], HEX);
    if (i < size - 1) uidStr += ":";
  }
  uidStr.toUpperCase();
  return uidStr;
}

// Searches for a user in the array based on their UID
int findUserIndexByUID(byte *uid, byte size) {
  String uidStr = generateUIDString(uid, size);
  for (int i = 0; i < totalUsers; i++) {
    if (users[i].id == uidStr) return i;
  }
  return -1;
}

// Adds a new user to the local database
void registerNewUser(String id, String name) {
  if (totalUsers < MAX_USERS) {
    users[totalUsers].id = id;
    users[totalUsers].name = name;
    users[totalUsers].isPresent = true;
    users[totalUsers].entryTime = millis();
    users[totalUsers].sessionDuration = 0;
    users[totalUsers].totalTime = 0;
    
    Serial.println("User registered successfully!");
    totalUsers++;
  } else {
    Serial.println("Error: Maximum user limit reached!");
  }
}

// Logic for Entry/Exit and session time calculation
void registerMovement(int index) {
  if (users[index].isPresent) {
    // Calculate session time upon exit
    unsigned long currentSessionTime = millis() - users[index].entryTime;
    users[index].totalTime += currentSessionTime;
    users[index].isPresent = false;
    Serial.println("EXIT: " + users[index].name);
  } else {
    // Mark entry
    users[index].isPresent = true;
    users[index].entryTime = millis();
    Serial.println("ENTRY: " + users[index].name);
  }
}

// Sends data to Telegram Bot via HTTP GET
void sendTelegramNotification(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://api.telegram.org/bot" + botToken +
                 "/sendMessage?chat_id=" + chatId +
                 "&text=" + urlEncode(message);
    http.begin(url);
    int httpCode = http.GET();
    
    if (httpCode > 0) {
      Serial.printf("Notification sent. Status code: %d\n", httpCode);
    } else {
      Serial.printf("Failed to send message. Error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.println("WiFi not connected. Skipping Telegram notification.");
  }
}

// Basic URL encoding for HTTP safety
String urlEncode(String str) {
  String encoded = "";
  for (int i = 0; i < str.length(); i++) {
    char c = str.charAt(i);
    if (isalnum(c)) {
      encoded += c;
    } else {
      encoded += '%';
      char hex1 = (c >> 4) & 0xF;
      char hex2 = c & 0xF;
      encoded += char(hex1 > 9 ? hex1 - 10 + 'A' : hex1 + '0');
      encoded += char(hex2 > 9 ? hex2 - 10 + 'A' : hex2 + '0');
    }
  }
  return encoded;
}
