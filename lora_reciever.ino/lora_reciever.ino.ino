#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ==========================================
// LORA PINS
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


void setup() {

  Serial.begin(115200);
  delay(1000);


  // ========================================
  // BUZZER
  // ========================================

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);


  // ========================================
  // ACK BUTTON
  // ========================================

  pinMode(ACK_BUTTON, INPUT_PULLUP);


  // ========================================
  // LCD
  // ========================================

  Wire.begin(LCD_SDA, LCD_SCL);

  lcd.begin(16, 2);
  lcd.backlight();

  lcd.clear();

  // Idle screen
  lcd.setCursor(0, 0);
  lcd.print("Waiting for SOS");

  lcd.setCursor(0, 1);
  lcd.print("Ready...");


  // ========================================
  // LORA
  // ========================================

  Serial.println();
  Serial.println("LoRa Receiver");

  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);


  if (!LoRa.begin(433E6)) {

    Serial.println("Starting LoRa failed!");

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("LoRa FAILED!");

    lcd.setCursor(0, 1);
    lcd.print("Check module");

    while (1);
  }


  Serial.println("LoRa Receiver Started!");
  Serial.println("Waiting for packets...");
}


// ==========================================
// MAIN LOOP
// ==========================================

void loop() {

  int packetSize = LoRa.parsePacket();


  if (packetSize) {

    Serial.println();
    Serial.println("===== PACKET RECEIVED =====");


    // ========================================
    // READ MESSAGE
    // ========================================

    String receivedData = "";

    while (LoRa.available()) {
      receivedData += (char)LoRa.read();
    }


    // ========================================
    // SIGNAL INFORMATION
    // ========================================

    int rssi = LoRa.packetRssi();
    float snr = LoRa.packetSnr();


    // ========================================
    // SERIAL MONITOR
    // ========================================

    Serial.print("Message: ");
    Serial.println(receivedData);

    Serial.print("RSSI: ");
    Serial.print(rssi);
    Serial.println(" dBm");

    Serial.print("SNR: ");
    Serial.print(snr);
    Serial.println(" dB");

    Serial.println("===========================");


    // ========================================
    // SHOW SOS
    // ========================================

    showSOSScreen(receivedData);


    // ========================================
    // PLAY SOS
    // ========================================

    playSOS();


    // ========================================
    // SHOW SIGNAL
    // ========================================

    showSignalScreen(rssi, snr);

    delay(2000);


    // ========================================
    // WAIT FOR RECEIVER TO CONFIRM (BUTTON)
    // ========================================

    waitForAckButton();


    // ========================================
    // RETURN TO IDLE
    // ========================================

    showWaitingScreen();
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

      // Small debounce delay
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

    lcd.setCursor(0, 0);
    lcd.print("No ACK sent");
    lcd.setCursor(0, 1);
    lcd.print("(timeout)");

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

  if (result == 1) {
    Serial.println("[SUCCESS] ACK transmitted");
  } else {
    Serial.println("[ERROR] ACK transmission failed");
  }


  // Show ACK sent screen
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("ACK Sent!");

  lcd.setCursor(0, 1);
  lcd.print("Sender notified");

  delay(2000);
}


// ==========================================
// WAITING SCREEN
// ==========================================

void showWaitingScreen() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Waiting for SOS");

  lcd.setCursor(0, 1);
  lcd.print("Ready...");
}


// ==========================================
// SOS SCREEN
// ==========================================

void showSOSScreen(String message) {

  lcd.clear();

  // First line
  lcd.setCursor(0, 0);
  lcd.print("!!! SOS !!!");

  // Second line
  lcd.setCursor(0, 1);

  if (message.length() > 16) {
    message = message.substring(0, 16);
  }

  lcd.print(message);
}


// ==========================================
// SIGNAL SCREEN
// ==========================================

void showSignalScreen(int rssi, float snr) {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("RSSI:");
  lcd.print(rssi);
  lcd.print(" dBm");

  lcd.setCursor(0, 1);
  lcd.print("SNR:");
  lcd.print(snr, 1);
  lcd.print(" dB");
}


// ==========================================
// CONFIRM SCREEN
// ==========================================

void showConfirmScreen() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("SOS Received");
  lcd.setCursor(0, 1);
  lcd.print("Press to Confirm");
}


// ==========================================
// SOS BUZZER
// ==========================================

void playSOS() {

  // S = ...
  beep(150);
  delay(150);

  beep(150);
  delay(150);

  beep(150);

  delay(400);


  // O = ---
  beep(500);
  delay(200);

  beep(500);
  delay(200);

  beep(500);

  delay(400);


  // S = ...
  beep(150);
  delay(150);

  beep(150);
  delay(150);

  beep(150);

  delay(1000);
}


// ==========================================
// BEEP
// ==========================================

void beep(int duration) {

  digitalWrite(BUZZER_PIN, HIGH);

  delay(duration);

  digitalWrite(BUZZER_PIN, LOW);
}
