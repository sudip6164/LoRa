#include <SPI.h>
#include <LoRa.h>

#define LORA_SS    5
#define LORA_RST   14
#define LORA_DIO0  26

#define BUZZER_PIN 25

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.println();
  Serial.println("LoRa Receiver");

  // Set LoRa pins
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  // Start LoRa at 433 MHz
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  Serial.println("LoRa Receiver Started!");
  Serial.println("Waiting for packets...");
}

void loop() {

  // Check if a packet has arrived
  int packetSize = LoRa.parsePacket();

  if (packetSize) {

    Serial.println();
    Serial.println("===== PACKET RECEIVED =====");

    // Read packet
    String receivedData = "";

    while (LoRa.available()) {
      receivedData += (char)LoRa.read();
    }

    Serial.print("Message: ");
    Serial.println(receivedData);

    // Display signal strength
    Serial.print("RSSI: ");
    Serial.print(LoRa.packetRssi());
    Serial.println(" dBm");

    // Display signal quality
    Serial.print("SNR: ");
    Serial.print(LoRa.packetSnr());
    Serial.println(" dB");

    Serial.println("===========================");

    // Buzzer alert
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1000);
    digitalWrite(BUZZER_PIN, LOW);
  }
}