# LoRa SOS System - Changes (Two-Way ACK)

## What Was Changed

Added **two-way communication** so the sender knows its SOS was received.

### Before

- Sender: button press -> send SOS -> nothing back
- Receiver: get SOS -> LCD + buzzer only

### After (New Flow)

1. **Sender** -> presses button (GPIO 27) -> sends `SOS|NAME:Suvarna|LOCATION:37.7510,37.7510|ID:R01|COUNT:<n>`
2. **Receiver** -> gets SOS -> shows SOS screen + plays buzzer + shows RSSI/SNR (same as before)
3. **Receiver** -> person presses new **ACK button (GPIO 33)** -> sends `ACK|SOS RECEIVED` back
4. **Sender** -> receives ACK -> prints confirmation -> **blinks small bulb (LED) 5 times**

---

## Sender (`lora_sender.ino`)

| Change | Detail |
|---|---|
| Added bulb pin | `BULB_PIN 2` (small LED) |
| Added blink config | 5 blinks, 300 ms ON / 300 ms OFF |
| Added listening | Sender now also listens for packets all the time |
| New function `checkForAck()` | Reads incoming packet; if it starts with `ACK` -> blinks bulb |
| New function `blinkBulb()` | Blinks LED 5 times |
| Refactored sending | Old loop code moved into `sendSOS()` |
| ACK check during waits | Button-release wait and 5 s cooldown now poll for ACK instead of plain delay |

## Receiver (`lora_reciever.ino.ino`)

| Change | Detail |
|---|---|
| Added ACK button | `ACK_BUTTON 33`, INPUT_PULLUP |
| Added confirm timeout | 30 seconds to press button after SOS |
| New function `waitForAckButton()` | Shows confirm screen, waits for press, debounced |
| New function `sendAck()` | Sends `ACK|SOS RECEIVED` back over LoRa |
| New screens | `SOS Received / Press to Confirm` and `ACK Sent! / Sender notified` |
| Timeout handling | If no press in 30 s -> LCD shows `No ACK sent (timeout)` -> idle |

---

## New Wiring

| Part | Pin | Notes |
|---|---|---|
| Bulb (LED) anode | ESP32 GPIO 2 | Through ~220 ohm resistor to GND (sender board) |
| Bulb (LED) cathode | GND | |
| ACK button leg 1 | ESP32 GPIO 33 | Uses internal pullup (receiver board) |
| ACK button leg 2 | GND | Press connects to LOW |

No other wiring changed.

---

## Message Format

```
Sender   ->  SOS|NAME:Suvarna|LOCATION:37.7510,37.7510|ID:R01|COUNT:n
Receiver ->  ACK|SOS RECEIVED
```

Both at 433 MHz, same LoRa settings as before.
