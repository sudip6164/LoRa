# Implementation Plan: Receiver -> Django REST API (JSON)

## Goal

When the LoRa receiver gets an SOS, it must forward that data as **JSON** to a **Django REST API**.
Django saves it to the database, and a **dashboard** displays live SOS alerts.

---

## Architecture

```
[Sender ESP32] --LoRa--> [Receiver ESP32] --WiFi/HTTP POST JSON--> [Django REST API] --> [Database]
                        (parses pipe string,                      |
                         builds JSON)                             v
                                                              [Dashboard page]
                                                              (auto-refresh)
```

Data keeps flowing exactly as now:

```
Sender -> Receiver :  SOS|NAME:Suvarna|LOCATION:37.7510,37.7510|ID:R01|COUNT:n   (LoRa 433 MHz)
Receiver -> Sender :  ACK|SOS RECEIVED                                           (LoRa, unchanged)
Receiver -> Django :  JSON over HTTP POST                                        (NEW)
```

LoRa link is untouched. The new part is only **receiver -> WiFi -> Django**.

---

## Phase 1 - Django Project + Model

1. Create Django project (or use existing one) + app `alerts`.
2. Install Django REST Framework: `pip install djangorestframework`.
3. Model `SOSAlert`:

| Field | Type | Source |
|---|---|---|
| `name` | CharField | parsed from `NAME:` |
| `latitude` | FloatField | parsed from `LOCATION:` (first value) |
| `longitude` | FloatField | parsed from `LOCATION:` (second value) |
| `device_id` | CharField | parsed from `ID:` |
| `packet_count` | IntegerField | parsed from `COUNT:` |
| `rssi` | IntegerField | measured by receiver |
| `snr` | FloatField | measured by receiver |
| `raw_message` | TextField | full pipe string (safety copy) |
| `status` | CharField | default `"PENDING"` (dashboard can mark RESOLVED) |
| `received_at` | DateTimeField | `auto_now_add=True` |

4. `python manage.py makemigrations && python manage.py migrate`.

## Phase 2 - REST API Endpoint

1. Serializer `SOSAlertSerializer` (ModelSerializer).
2. Endpoint: `/api/sos/`
   - `POST` -> create alert (used by ESP32). Return `201` with saved object.
   - `GET` -> list alerts, newest first (used by dashboard).
   - Optional: `GET /api/sos/<id>/`, `PATCH` to change `status`.
3. Validation: reject bad JSON with `400`; missing fields get defaults where safe.
4. Simple protection: require header `X-API-Key: <secret>` checked in the view
   (plain token is enough for now, no user login needed).
5. Run server on LAN so ESP32 can reach it:
   `python manage.py runserver 0.0.0.0:8000`

### Expected JSON from receiver

```json
{
  "name": "Suvarna",
  "latitude": 37.7510,
  "longitude": 37.7510,
  "device_id": "R01",
  "packet_count": 3,
  "rssi": -82,
  "snr": 9.5,
  "raw_message": "SOS|NAME:Suvarna|LOCATION:37.7510,37.7510|ID:R01|COUNT:3"
}
```

## Phase 3 - Receiver ESP32 Changes (`lora_reciever.ino.ino`)

1. Add WiFi credentials (SSID/password) + Django server URL constant:
   `http://192.168.x.x:8000/api/sos/`
2. Add `WiFi.begin()` in `setup()` with reconnect handling (non-blocking retry).
3. New function `parsePipeMessage(String)` -> extracts NAME / LOCATION / ID / COUNT
   into variables (split on `|` then on `:`).
4. New function `sendToServer(rssi, snr, raw)` ->
   - builds the JSON above (manual string build is fine; ArduinoJson optional),
   - `HTTPClient` POST with `Content-Type: application/json`
     and `X-API-Key` header,
   - timeout ~5 s, prints response code to Serial,
   - LCD shows `Sending to server...` then `Server OK!` / `Server FAIL!`.
5. Flow inside `loop()` after receiving SOS:

```
showSOS -> playSOS buzzer -> showSignalScreen -> sendToServer() -> waitForAckButton() -> idle
```

(HTTP first, then the local ACK button step - order can be swapped if preferred.)
6. Optional hardening: if POST fails (WiFi down), retry once; optionally buffer
   last unsent alert in `Preferences` (NVS) and resend when WiFi returns.

## Phase 4 - Dashboard

Simplest reliable option first (no extra frontend framework):

1. Django template view `/dashboard/` listing alerts in a table:
   time, name, device, location, RSSI/SNR, status badge.
2. Auto-refresh with tiny JS polling `/api/sos/` every 3-5 s
   (later upgrade path: HTMX or WebSockets/Channels for push updates).
3. New alerts highlighted (e.g., red row for `status=PENDING`),
   button per row to mark RESOLVED (`PATCH`).

## Phase 5 - Testing Checklist

1. `curl -X POST http://localhost:8000/api/sos/ -H "Content-Type: application/json" -H "X-API-Key: ..." -d '{...}'` -> 201, row in DB.
2. `curl http://localhost:8000/api/sos/` -> JSON list works.
3. ESP32 Serial shows: WiFi connected -> `HTTP code: 201`.
4. Full chain test: press sender button -> receiver LCD/buzzer -> bulb blinks (ACK) -> alert appears on dashboard within ~5 s.
5. Failure tests: wrong API key -> 403; WiFi off at receiver -> Serial error, system still does local ACK flow.

---

## Open Questions (answer before coding)

1. Does the Django project already exist, or do I create it from scratch here?
2. Which database: SQLite (default, fine to start) or PostgreSQL?
3. What are the WiFi SSID/password and the PC's LAN IP where Django runs?
4. Should the dashboard be plain Django templates (recommended start) or a separate React/frontend app (needs CORS enabled)?
