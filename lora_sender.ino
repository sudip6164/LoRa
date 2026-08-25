#include <SPI.h>
#include <LoRa.h>

#define LORA_SS    5
#define LORA_RST   14
#define LORA_DIO0  26

#define SOS_BUTTON 27

int counter = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("LoRa SOS Sender");

  // Button setup
  // Button connected between GPIO 27 and GND
  pinMode(SOS_BUTTON, INPUT_PULLUP);

  // Set LoRa pins for ESP32
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  // Start LoRa at 433 MHz
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  Serial.println("LoRa initialized successfully!");
  Serial.println("Waiting for SOS button...");
}

void loop() {

  // Button is pressed when GPIO goes LOW
  if (digitalRead(SOS_BUTTON) == LOW) {

    Serial.println("SOS BUTTON PRESSED!");

    LoRa.beginPacket();

    LoRa.print("SOS|");
    LoRa.print("37.7510,37.7510|");
    LoRa.print("ID:R01|");
    LoRa.print("COUNT:");
    LoRa.print(counter);

    LoRa.endPacket();

    Serial.println("SOS SENT!");

    counter++;

    // Wait until button is released
    while (digitalRead(SOS_BUTTON) == LOW) {
      delay(10);
    }

    // Small debounce delay
    delay(200);
  }
}