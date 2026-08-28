#include <SPI.h>
#include <LoRa.h>

// ==========================================
// LORA PINS  (AI-Thinker RA-02 = SX1278, 433 MHz)
//   RA-02 NSS -> 5, NRESET -> 14, DIO0 -> 26
// ==========================================
#define LORA_SS    5
#define LORA_RST   14
#define LORA_DIO0  26

// ==========================================
// SOS BUTTON  (active LOW, internal pull-up)
// ==========================================
#define SOS_BUTTON 27

// ==========================================
// ACK INDICATOR LED  (on-board LED on most ESP32 = GPIO 2)
// Many ESP32 clones are active-LOW (LED lights when pin is LOW) -> keep 1.
// If your LED is ON when the pin is HIGH, change the 1 to 0.
// ==========================================
#define LED_PIN        2
#define LED_ACTIVE_LOW  1

void ledOff()   { digitalWrite(LED_PIN, LED_ACTIVE_LOW ? HIGH : LOW); }
void ledBlink(int n, int ms) {
  for (int i = 0; i < n; i++) {
    digitalWrite(LED_PIN, LED_ACTIVE_LOW ? LOW  : HIGH);
    delay(ms);
    digitalWrite(LED_PIN, LED_ACTIVE_LOW ? HIGH : LOW);
    delay(ms);
  }
}

#define ACK_WAIT_TIME  35000   // ms to listen for receiver's ACK (>= receiver's 30s)

// ==========================================
// DEVICE INFO  (edit per sender unit)
// ==========================================
const String DEVICE_NAME   = "Suvarna";
const String DEVICE_ID     = "R01";
const float  DEVICE_LAT    = 27.7172;
const float  DEVICE_LNG    = 85.3240;

int packetNumber = 0;

// Interrupt flag: set the instant the button is pressed (no polling delay).
volatile bool sosRequested = false;

void IRAM_ATTR onButtonPress() {
  static unsigned long lastPress = 0;
  unsigned long now = millis();
  if (now - lastPress > 500) {   // simple debounce
    sosRequested = true;
    lastPress = now;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("================================");
  Serial.println("       LoRa SOS SENDER");
  Serial.println("================================");

  pinMode(SOS_BUTTON, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  ledOff();
  attachInterrupt(digitalPinToInterrupt(SOS_BUTTON), onButtonPress, FALLING);

  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(433E6)) {
    Serial.println("[ERROR] LoRa initialization failed!");
    while (1);
  }

  LoRa.setSpreadingFactor(12);
  LoRa.setTxPower(17);

  Serial.println("[OK] LoRa initialized");
  Serial.println("[OK] Frequency: 433 MHz");
  Serial.print  ("[OK] Sender: ");
  Serial.println(DEVICE_NAME);
  Serial.println("[OK] After sending, I wait for the receiver's ACK");
  Serial.println("--------------------------------");
  Serial.println("Waiting for SOS button...");
  Serial.println();
}

void loop() {

  if (sosRequested) {
    sosRequested = false;

    packetNumber++;

    Serial.println("================================");
    Serial.println("SOS BUTTON PRESSED");
    Serial.print("Sending SOS from: ");
    Serial.println(DEVICE_NAME);
    Serial.print("Packet #");
    Serial.println(packetNumber);

    String payload = "SOS|";
    payload += "NAME:" + DEVICE_NAME + "|";
    payload += "LOCATION:" + String(DEVICE_LAT, 4) + "," + String(DEVICE_LNG, 4) + "|";
    payload += "ID:" + DEVICE_ID + "|";
    payload += "COUNT:" + String(packetNumber);

    LoRa.beginPacket();
    LoRa.print(payload);
    int result = LoRa.endPacket();

    if (result == 1) {
      Serial.println("[SUCCESS] SOS transmitted");
    } else {
      Serial.println("[ERROR] SOS transmission failed");
    }

    Serial.print("Message: ");
    Serial.println(payload);
    Serial.println("--------------------------------");

    // Wait for the receiver to send back "ACK|SOS RECEIVED"
    waitForAck();

    // Wait for button release
    while (digitalRead(SOS_BUTTON) == LOW) {
      delay(10);
    }

    Serial.println("Button released.");
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

// ==========================================
// LISTEN FOR ACK FROM RECEIVER
//   The receiver replies with "ACK|SOS RECEIVED"
//   after its operator presses the confirm button.
// ==========================================
void waitForAck() {
  Serial.println("[WAIT] Listening for ACK from receiver (up to 35s)...");
  Serial.println("[WAIT] LED blinks only when the ACK arrives...");

  LoRa.receive();              // put the RA-02 into RX mode
  ledOff();                    // LED OFF while waiting

  unsigned long startTime = millis();
  bool gotAck = false;

  while (millis() - startTime < ACK_WAIT_TIME) {
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      String data = "";
      while (LoRa.available()) {
        data += (char)LoRa.read();
      }
      Serial.print("[RX] Packet: ");
      Serial.println(data);

      if (data.startsWith("ACK")) {
        gotAck = true;
        break;
      }
    }
    delay(20);
  }

  if (gotAck) {
    Serial.println("[OK] ACK received - receiver confirmed the alert!");
    // LED blinks ONLY when the ACK is received (i.e. sent by the receiver)
    ledBlink(6, 150);
  } else {
    Serial.println("[WARN] No ACK received (timeout). Receiver may be offline.");
    ledOff();
  }
}
