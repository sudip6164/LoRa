#include <SPI.h>
#include <LoRa.h>

// ==========================================
// LORA PINS
// ==========================================

#define LORA_SS    5
#define LORA_RST   14
#define LORA_DIO0  26


// ==========================================
// SOS BUTTON
// ==========================================

#define SOS_BUTTON 27


// ==========================================
// SMALL BULB (LED)
// ==========================================

#define BULB_PIN        2
#define BLINK_COUNT     5
#define BLINK_ON_TIME   300
#define BLINK_OFF_TIME  300


int packetNumber = 0;

void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("================================");
  Serial.println("       LoRa SOS SENDER");
  Serial.println("================================");


  // ========================================
  // BUTTON
  // ========================================

  pinMode(SOS_BUTTON, INPUT_PULLUP);


  // ========================================
  // SMALL BULB
  // ========================================

  pinMode(BULB_PIN, OUTPUT);
  digitalWrite(BULB_PIN, LOW);


  // ========================================
  // LORA
  // ========================================

  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(433E6)) {
    Serial.println("[ERROR] LoRa initialization failed!");
    while (1);
  }

  Serial.println("[OK] LoRa initialized");
  Serial.println("[OK] Frequency: 433 MHz");
  Serial.println("[OK] Sender: Suvarna");
  Serial.println("[OK] Button: GPIO 27");
  Serial.println("[OK] Bulb: GPIO 2");
  Serial.println("--------------------------------");
  Serial.println("Waiting for SOS button...");
  Serial.println();
}


// ==========================================
// MAIN LOOP
// ==========================================

void loop() {

  // ========================================
  // LISTEN FOR ACK FROM RECEIVER
  // ========================================

  checkForAck();


  // ========================================
  // SEND SOS ON BUTTON PRESS
  // ========================================

  if (digitalRead(SOS_BUTTON) == LOW) {

    sendSOS();


    // ========================================
    // WAIT FOR BUTTON RELEASE
    // (still listening for ACK)
    // ========================================

    while (digitalRead(SOS_BUTTON) == LOW) {
      checkForAck();
      delay(10);
    }

    Serial.println("Button released.");


    // ========================================
    // 5 SECOND COOLDOWN
    // (still listening for ACK)
    // ========================================

    Serial.println("5-second cooldown started...");

    for (int i = 5; i > 0; i--) {
      Serial.print("Next SOS available in ");
      Serial.print(i);
      Serial.println(" second(s)");

      unsigned long start = millis();
      while (millis() - start < 1000) {
        checkForAck();
        delay(10);
      }
    }

    Serial.println("Cooldown finished.");
    Serial.println("Ready for next SOS.");
    Serial.println();
  }
}


// ==========================================
// SEND SOS
// ==========================================

void sendSOS() {

  packetNumber++;

  Serial.println("================================");
  Serial.println("SOS BUTTON PRESSED");
  Serial.print("Sending SOS from: Suvarna");
  Serial.println();
  Serial.print("Packet #");
  Serial.println(packetNumber);

  // Create LoRa packet
  LoRa.beginPacket();

  LoRa.print("SOS|");
  LoRa.print("NAME:Suvarna|");
  LoRa.print("LOCATION:37.7510,37.7510|");
  LoRa.print("ID:R01|");
  LoRa.print("COUNT:");
  LoRa.print(packetNumber);

  int result = LoRa.endPacket();

  if (result == 1) {
    Serial.println("[SUCCESS] SOS transmitted");
  } else {
    Serial.println("[ERROR] SOS transmission failed");
  }

  Serial.println("Message:");
  Serial.print("SOS|NAME:Suvarna|LOCATION:37.7510,37.7510|ID:R01|COUNT:");
  Serial.println(packetNumber);

  Serial.println("--------------------------------");

  Serial.println("[WAIT] Blinking bulb when receiver confirms...");
}


// ==========================================
// CHECK FOR ACK FROM RECEIVER
// ==========================================

void checkForAck() {

  int packetSize = LoRa.parsePacket();

  if (!packetSize) {
    return;
  }


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

  Serial.println();
  Serial.println("===== PACKET RECEIVED =====");
  Serial.print("Message: ");
  Serial.println(receivedData);
  Serial.print("RSSI: ");
  Serial.print(rssi);
  Serial.println(" dBm");
  Serial.print("SNR: ");
  Serial.print(snr);
  Serial.println(" dB");


  // ========================================
  // IS IT AN ACK?
  // ========================================

  if (receivedData.startsWith("ACK")) {

    Serial.println("[OK] RECEIVER CONFIRMED SOS RECEIVED!");
    Serial.println("[OK] Blinking bulb...");
    Serial.println("===========================");

    blinkBulb();

    Serial.println("[OK] Bulb done. Waiting for next SOS.");

  } else {
    Serial.println("[INFO] Unknown packet ignored.");
    Serial.println("===========================");
  }
}


// ==========================================
// BLINK THE SMALL BULB
// ==========================================

void blinkBulb() {

  for (int i = 0; i < BLINK_COUNT; i++) {

    digitalWrite(BULB_PIN, HIGH);
    delay(BLINK_ON_TIME);

    digitalWrite(BULB_PIN, LOW);
    delay(BLINK_OFF_TIME);
  }
}
