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

#define LED_PIN 32


// ==========================================
// LCD
// ==========================================

#define LCD_SDA 21
#define LCD_SCL 22

LiquidCrystal_I2C lcd(0x27, 16, 2);


// ==========================================
// SOS TIMER
// ==========================================

const unsigned long SOS_DISPLAY_TIME = 60000;

// Time between LCD information screens
const unsigned long SCREEN_CHANGE_TIME = 3000;

unsigned long lastSOS = 0;
unsigned long lastScreenChange = 0;

bool sosActive = false;

int currentScreen = 0;


// ==========================================
// SOS DATA
// ==========================================

String senderName = "";
String location = "";
String deviceID = "";
String count = "";


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

  // ========================================
  // CHECK FOR LORA PACKET
  // ========================================

  int packetSize = LoRa.parsePacket();


  if (packetSize) {

    Serial.println();
    Serial.println("===== PACKET RECEIVED =====");


    // ======================================
    // READ MESSAGE
    // ======================================

    String receivedData = "";

    while (LoRa.available()) {

      receivedData += (char)LoRa.read();
    }


    // ======================================
    // SIGNAL INFORMATION
    // ======================================

    int rssi = LoRa.packetRssi();

    float snr = LoRa.packetSnr();


    // ======================================
    // SERIAL MONITOR
    // ======================================

    Serial.print("Message: ");
    Serial.println(receivedData);

    Serial.print("RSSI: ");
    Serial.print(rssi);
    Serial.println(" dBm");

    Serial.print("SNR: ");
    Serial.print(snr);
    Serial.println(" dB");

    Serial.println("===========================");


    // ======================================
    // PARSE MESSAGE
    // ======================================

    parseMessage(receivedData);


    // ======================================
    // START SOS DISPLAY
    // ======================================

    sosActive = true;

    lastSOS = millis();

    lastScreenChange = millis();

    currentScreen = 0;


    // Show first screen immediately
    showCurrentScreen();


    // ======================================
    // PLAY SOS
    // ======================================

    playSOS();
  }


  // ========================================
  // CHANGE LCD SCREEN
  // ========================================

  if (sosActive) {

    if (millis() - lastScreenChange >= SCREEN_CHANGE_TIME) {

      lastScreenChange = millis();

      currentScreen++;

      if (currentScreen > 2) {

        currentScreen = 0;
      }

      showCurrentScreen();
    }


    // ======================================
    // 1-MINUTE TIMEOUT
    // ======================================

    if (millis() - lastSOS >= SOS_DISPLAY_TIME) {

      sosActive = false;

      showWaitingScreen();

      Serial.println("SOS display timeout.");
      Serial.println("Waiting for new SOS...");
    }
  }
}


// ==========================================
// PARSE LORA MESSAGE
// ==========================================

void parseMessage(String message) {

  senderName = "";
  location = "";
  deviceID = "";
  count = "";


  // ----------------------------------------
  // NAME
  // ----------------------------------------

  int nameStart = message.indexOf("NAME:");

  if (nameStart != -1) {

    nameStart += 5;

    int nameEnd = message.indexOf("|", nameStart);

    if (nameEnd == -1) {

      nameEnd = message.length();
    }

    senderName = message.substring(nameStart, nameEnd);
  }


  // ----------------------------------------
  // LOCATION
  // ----------------------------------------

  int locationStart = message.indexOf("LOCATION:");

  if (locationStart != -1) {

    locationStart += 9;

    int locationEnd = message.indexOf("|", locationStart);

    if (locationEnd == -1) {

      locationEnd = message.length();
    }

    location = message.substring(locationStart, locationEnd);
  }


  // ----------------------------------------
  // ID
  // ----------------------------------------

  int idStart = message.indexOf("ID:");

  if (idStart != -1) {

    idStart += 3;

    int idEnd = message.indexOf("|", idStart);

    if (idEnd == -1) {

      idEnd = message.length();
    }

    deviceID = message.substring(idStart, idEnd);
  }


  // ----------------------------------------
  // COUNT
  // ----------------------------------------

  int countStart = message.indexOf("COUNT:");

  if (countStart != -1) {

    countStart += 6;

    int countEnd = message.indexOf("|", countStart);

    if (countEnd == -1) {

      countEnd = message.length();
    }

    count = message.substring(countStart, countEnd);
  }
}


// ==========================================
// SHOW CURRENT LCD SCREEN
// ==========================================

void showCurrentScreen() {

  lcd.clear();


  // ========================================
  // SCREEN 1
  // ========================================

  if (currentScreen == 0) {

    lcd.setCursor(0, 0);

    lcd.print("SOS | NAME:");

    lcd.setCursor(0, 1);

    lcd.print(senderName);
  }


  // ========================================
  // SCREEN 2
  // ========================================

  else if (currentScreen == 1) {

    lcd.setCursor(0, 0);

    lcd.print("LOCATION:");

    lcd.setCursor(0, 1);

    lcd.print(location);
  }


  // ========================================
  // SCREEN 3
  // ========================================

  else if (currentScreen == 2) {

    lcd.setCursor(0, 0);

    lcd.print("ID:");
    lcd.print(deviceID);

    lcd.setCursor(0, 1);

    lcd.print("COUNT:");
    lcd.print(count);
  }
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

  delay(400);


  // ========================================
  // O = ---
  // ========================================

  beep(500);
  delay(200);

  beep(500);
  delay(200);

  beep(500);

  delay(400);


  // ========================================
  // S = ...
  // ========================================

  beep(150);
  delay(150);

  beep(150);
  delay(150);

  beep(150);

  delay(1000);
}


// ==========================================
// BEEP + LED
// ==========================================

void beep(int duration) {

  digitalWrite(BUZZER_PIN, HIGH);

  digitalWrite(LED_PIN, HIGH);

  delay(duration);

  digitalWrite(BUZZER_PIN, LOW);

  digitalWrite(LED_PIN, LOW);
}