#include <SPI.h>
#include <LoRa.h>

#define LORA_SS    5
#define LORA_RST   14
#define LORA_DIO0  26

#define SOS_BUTTON 27

int packetNumber = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("================================");
  Serial.println("       LoRa SOS SENDER");
  Serial.println("================================");

  pinMode(SOS_BUTTON, INPUT_PULLUP);

  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(433E6)) {
    Serial.println("[ERROR] LoRa initialization failed!");
    while (1);
  }

  Serial.println("[OK] LoRa initialized");
  Serial.println("[OK] Frequency: 433 MHz");
  Serial.println("[OK] Button: GPIO 27");
  Serial.println("--------------------------------");
  Serial.println("Waiting for SOS button...");
  Serial.println();
}

void loop() {

  if (digitalRead(SOS_BUTTON) == LOW) {

    packetNumber++;

    Serial.println("================================");
    Serial.println("SOS BUTTON PRESSED");
    Serial.print("Sending packet #");
    Serial.println(packetNumber);

    LoRa.beginPacket();

    LoRa.print("SOS|");
    LoRa.print("37.7510,37.7510|");
    LoRa.print("ID:R01|");
    LoRa.print("COUNT:");
    LoRa.print(packetNumber);

    int result = LoRa.endPacket();

    if (result == 1) {
      Serial.println("[SUCCESS] Packet transmitted");
    } else {
      Serial.println("[ERROR] Packet transmission failed");
    }

    Serial.print("Packet number: ");
    Serial.println(packetNumber);

    Serial.println("--------------------------------");

    // Wait for button release
    while (digitalRead(SOS_BUTTON) == LOW) {
      delay(10);
    }

    Serial.println("Button released.");

    // 5-second cooldown
    Serial.println("5-second cooldown started...");

    for (int i = 5; i > 0; i--) {
      Serial.print("Next SOS available in ");
      Serial.print(i);
      Serial.println(" second(s)");
      delay(1000);
    }

    Serial.println("Cooldown finished.");
    Serial.println("Ready for next SOS.");
    Serial.println();
  }
}