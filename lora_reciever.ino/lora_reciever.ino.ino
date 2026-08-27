#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// ==========================================
// LORA PINS  (AI-Thinker RA-02 = SX1278, 433 MHz)
//   RA-02 NSS -> 5, NRESET -> 14, DIO0 -> 26
// ==========================================
#define LORA_SS    5
#define LORA_RST   14
#define LORA_DIO0  26

// ==========================================
// BUZZER
// ==========================================
#define BUZZER_PIN 25

// ==========================================
// LCD
// ==========================================
#define LCD_SDA 21
#define LCD_SCL 22
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ==========================================
// ACK BUTTON
// ==========================================
#define ACK_BUTTON      33
#define ACK_WAIT_TIME   30000   // 30 seconds to confirm

// ==========================================
// WIFI  (edit with your network)
// ==========================================
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// ==========================================
// BACKEND  (data goes to /api/sos/, not /dashboard/)
// ==========================================
const char* SERVER   = "https://backend.nirvix.com";
const char* ENDPOINT = "/api/sos/";
const char* API_KEY  = "wRJLAb4lVXwWRGEWZiMA2xF4v2cu71dk";

WiFiClientSecure wifiClient;
HTTPClient http;

// Parsed SOS fields
struct SOSData {
  String name;
  float  latitude;
  float  longitude;
  String device_id;
  int    packet_count;
  bool   valid;
};

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  pinMode(ACK_BUTTON, INPUT_PULLUP);

  Wire.begin(LCD_SDA, LCD_SCL);
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Waiting for SOS");
  lcd.setCursor(0, 1); lcd.print("Ready...");

  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("LoRa FAILED!");
    lcd.setCursor(0, 1); lcd.print("Check module");
    while (1);
  }
  LoRa.setSpreadingFactor(12);
  Serial.println("LoRa Receiver Started!");

  wifiClient.setInsecure();
  connectWiFi();
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.println();
    Serial.println("===== PACKET RECEIVED =====");

    String receivedData = "";
    while (LoRa.available()) receivedData += (char)LoRa.read();

    int rssi = LoRa.packetRssi();
    float snr = LoRa.packetSnr();

    Serial.print("Message: "); Serial.println(receivedData);
    Serial.print("RSSI: ");    Serial.print(rssi); Serial.println(" dBm");
    Serial.print("SNR: ");     Serial.print(snr); Serial.println(" dB");
    Serial.println("===========================");

    showSOSScreen(receivedData);
    playSOS();
    showSignalScreen(rssi, snr);
    delay(2000);

    // ---- SEND TO BACKEND ----
    connectWiFi();
    SOSData sos = parsePacket(receivedData);
    if (sos.valid) {
      bool sent = postToBackend(sos, rssi, snr, receivedData);
      showPostStatus(sent);
    } else {
      Serial.println("[ERROR] Could not parse SOS packet");
      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("BAD PACKET");
      delay(1500);
    }

    waitForAckButton();
    showWaitingScreen();
  }
}

// ==========================================
// WIFI
// ==========================================
void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  Serial.print("Connecting to WiFi ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 20) {
    delay(500); Serial.print("."); tries++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[OK] WiFi connected");
  } else {
    Serial.println("\n[WARN] WiFi not connected - alerts not sent");
  }
}

// ==========================================
// PARSE PACKET
//   SOS|NAME:<name>|LOCATION:<lat>,<lng>|ID:<id>|COUNT:<n>
// ==========================================
SOSData parsePacket(String data) {
  SOSData sos;
  sos.valid = false;
  sos.latitude = 0.0; sos.longitude = 0.0; sos.packet_count = 0;
  int start = 0;
  while (start < data.length()) {
    int sep = data.indexOf('|', start);
    if (sep == -1) sep = data.length();
    String token = data.substring(start, sep);
    int colon = token.indexOf(':');
    if (colon != -1) {
      String key = token.substring(0, colon);
      String val = token.substring(colon + 1);
      if (key == "NAME")         sos.name = val;
      else if (key == "ID")      sos.device_id = val;
      else if (key == "COUNT")   sos.packet_count = val.toInt();
      else if (key == "LOCATION") {
        int comma = val.indexOf(',');
        if (comma != -1) {
          sos.latitude  = val.substring(0, comma).toFloat();
          sos.longitude = val.substring(comma + 1).toFloat();
        }
      }
    }
    start = sep + 1;
  }
  if (sos.name.length() > 0 && sos.device_id.length() > 0) sos.valid = true;
  return sos;
}

// ==========================================
// POST TO BACKEND
// ==========================================
bool postToBackend(SOSData sos, int rssi, float snr, String raw) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[SKIP] WiFi down, not sending");
    return false;
  }
  String url = String(SERVER) + String(ENDPOINT);
  http.begin(wifiClient, url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-API-Key", API_KEY);

  String json = "{";
  json += "\"name\":\"" + sos.name + "\",";
  json += "\"latitude\":" + String(sos.latitude, 6) + ",";
  json += "\"longitude\":" + String(sos.longitude, 6) + ",";
  json += "\"device_id\":\"" + sos.device_id + "\",";
  json += "\"packet_count\":" + String(sos.packet_count) + ",";
  json += "\"rssi\":" + String(rssi) + ",";
  json += "\"snr\":" + String(snr, 2) + ",";
  json += "\"raw_message\":\"" + raw + "\"";
  json += "}";

  Serial.println("POST " + url);
  Serial.println(json);

  int httpCode = http.POST(json);
  String response = http.getString();
  http.end();

  if (httpCode == HTTP_CODE_CREATED || httpCode == HTTP_CODE_OK) {
    Serial.println("[OK] Alert sent (HTTP " + String(httpCode) + ")");
    return true;
  } else {
    Serial.println("[ERROR] POST failed: HTTP " + String(httpCode));
    Serial.println(response);
    return false;
  }
}

// ==========================================
// WAIT FOR ACK BUTTON PRESS
// ==========================================
void waitForAckButton() {
  showConfirmScreen();
  Serial.println("[WAIT] Press button to send ACK back to sender...");
  Serial.println("[WAIT] Timeout in 30 seconds.");
  unsigned long startTime = millis();
  bool acked = false;
  while (millis() - startTime < ACK_WAIT_TIME) {
    if (digitalRead(ACK_BUTTON) == LOW) {
      delay(50);
      if (digitalRead(ACK_BUTTON) == LOW) {
        sendAck();
        acked = true;
        break;
      }
    }
    delay(10);
  }
  if (!acked) {
    Serial.println("[WARN] No ACK sent (timeout).");
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("No ACK sent");
    lcd.setCursor(0, 1); lcd.print("(timeout)");
    delay(1500);
  }
}

// ==========================================
// SEND ACK BACK TO SENDER
// ==========================================
void sendAck() {
  Serial.println("[TX] Sending ACK to sender...");
  LoRa.beginPacket();
  LoRa.print("ACK|");
  LoRa.print("SOS RECEIVED");
  int result = LoRa.endPacket();
  if (result == 1) Serial.println("[SUCCESS] ACK transmitted");
  else            Serial.println("[ERROR] ACK transmission failed");
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("ACK Sent!");
  lcd.setCursor(0, 1); lcd.print("Sender notified");
  delay(2000);
}

// ==========================================
// WAITING SCREEN
// ==========================================
void showWaitingScreen() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Waiting for SOS");
  lcd.setCursor(0, 1); lcd.print("Ready...");
}

// ==========================================
// SOS SCREEN
// ==========================================
void showSOSScreen(String message) {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("!!! SOS !!!");
  lcd.setCursor(0, 1);
  if (message.length() > 16) message = message.substring(0, 16);
  lcd.print(message);
}

// ==========================================
// SIGNAL SCREEN
// ==========================================
void showSignalScreen(int rssi, float snr) {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("RSSI:"); lcd.print(rssi); lcd.print(" dBm");
  lcd.setCursor(0, 1); lcd.print("SNR:");  lcd.print(snr, 1); lcd.print(" dB");
}

// ==========================================
// POST STATUS SCREEN
// ==========================================
void showPostStatus(bool ok) {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(ok ? "ALERT SENT OK" : "SEND FAILED");
  lcd.setCursor(0, 1); lcd.print(ok ? "Backend updated" : "Check WiFi/API");
  delay(1500);
}

// ==========================================
// CONFIRM SCREEN
// ==========================================
void showConfirmScreen() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("SOS Received");
  lcd.setCursor(0, 1); lcd.print("Press to Confirm");
}

// ==========================================
// SOS BUZZER
// ==========================================
void playSOS() {
  beep(150); delay(150); beep(150); delay(150); beep(150); delay(400);
  beep(500); delay(200); beep(500); delay(200); beep(500); delay(400);
  beep(150); delay(150); beep(150); delay(150); beep(150); delay(1000);
}

// ==========================================
// BEEP
// ==========================================
void beep(int duration) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(duration);
  digitalWrite(BUZZER_PIN, LOW);
}
