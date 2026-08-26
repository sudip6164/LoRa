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
// LED
// ==========================================

// All 3 LEDs are controlled by the same GPIO
#define LED_PIN 32


// ==========================================
// LCD
// ==========================================

#define LCD_SDA 21
#define LCD_SCL 22

LiquidCrystal_I2C lcd(0x27, 16, 2);


// ==========================================
// SETUP
// ==========================================

void setup() {

  Serial.begin(115200);
  delay(1000);


  // ========================================
  // BUZZER
  // ========================================

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);


  // ========================================
  // LED
  // ========================================

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);


  // ========================================
  // LCD
  // ========================================

  Wire.begin(LCD_SDA, LCD_SCL);

  lcd.begin(16, 2);
  lcd.backlight();

  lcd.clear();

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
    // SHOW SOS MESSAGE
    // ========================================

    showSOSScreen(receivedData);


    // ========================================
    // PLAY SOS
    // BUZZER + ALL 3 LEDs
    // ========================================

    playSOS();


    // ========================================
    // SHOW SIGNAL INFORMATION
    // ========================================

    showSignalScreen(rssi, snr);

    delay(2000);


    // ========================================
    // RETURN TO IDLE
    // ========================================

    showWaitingScreen();
  }
}


// ==========================================
// LCD WAITING SCREEN
// ==========================================

void showWaitingScreen() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Waiting for SOS");

  lcd.setCursor(0, 1);
  lcd.print("Ready...");
}


// ==========================================
// LCD SOS SCREEN
// ==========================================

void showSOSScreen(String message) {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("!!! SOS !!!");

  lcd.setCursor(0, 1);

  if (message.length() > 16) {

    message = message.substring(0, 16);
  }

  lcd.print(message);
}


// ==========================================
// LCD SIGNAL SCREEN
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
// SOS MORSE PATTERN
// ==========================================

void playSOS() {

  // ========================================
  // S = ...
  // ========================================

  beep(150);
  delay(150);

  beep(150);
  delay(150);

  beep(150);


  // Pause between S and O
  delay(400);


  // ========================================
  // O = ---
  // ========================================

  beep(500);
  delay(200);

  beep(500);
  delay(200);

  beep(500);


  // Pause between O and S
  delay(400);


  // ========================================
  // S = ...
  // ========================================

  beep(150);
  delay(150);

  beep(150);
  delay(150);

  beep(150);


  // End
  delay(1000);
}


// ==========================================
// BEEP + 3 LEDS
// ==========================================

void beep(int duration) {

  // Turn ON buzzer
  digitalWrite(BUZZER_PIN, HIGH);

  // Turn ON all 3 LEDs
  // because they are all connected to GPIO 32
  digitalWrite(LED_PIN, HIGH);


  // Keep them ON for the beep duration
  delay(duration);


  // Turn OFF buzzer
  digitalWrite(BUZZER_PIN, LOW);

  // Turn OFF all 3 LEDs
  digitalWrite(LED_PIN, LOW);
}