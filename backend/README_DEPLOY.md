# LoRa SOS Backend - Deploy Guide

Django REST backend for the LoRa SOS system. Receives JSON from the ESP32
receiver, saves it to a database, and shows a live dashboard.

## What is inside

| URL | Method | Purpose |
|---|---|---|
| `/api/sos/` | POST | ESP32 receiver sends SOS JSON here (needs `X-API-Key` header) |
| `/api/sos/` | GET | JSON list of all alerts (newest first) |
| `/api/sos/<id>/` | GET / PATCH | Read one alert / update its status |
| `/dashboard/` | GET | Live dashboard (auto-refresh every 5 s) |
| `/admin/` | GET | Django admin |
| `/` | - | Redirects to dashboard |

---

## Step 1 - Edit settings.py

Open `config/settings.py` and change only the **EDIT THESE** section:

```python
SECRET_KEY = '<long random string>'       # required
DEBUG = False                              # on cPanel it must be False
ALLOWED_HOSTS = ['yourdomain.com']         # your domain or subdomain
SOS_API_KEY = '<your-secret-api-key>'      # same key goes in the ESP32 code
```

## Step 2 - Test on your PC first (optional but recommended)

```bash
cd backend
python -m pip install -r requirements.txt
python manage.py migrate
python manage.py runserver
```

- Dashboard: http://127.0.0.1:8000/dashboard/
- Test POST:

```bash
curl -X POST http://127.0.0.1:8000/api/sos/ \
  -H "Content-Type: application/json" \
  -H "X-API-Key: CHANGE-ME-sos-api-key" \
  -d "{\"name\":\"Suvarna\",\"latitude\":37.751,\"longitude\":37.751,\"device_id\":\"R01\",\"packet_count\":1,\"rssi\":-82,\"snr\":9.5,\"raw_message\":\"SOS|NAME:Suvarna\"}"
```

## Step 3 - Upload to cPanel

1. Zip the whole `backend` folder and upload it via **File Manager** into your
   home directory (e.g. `/home/USERNAME/sos_backend`). Extract it there.
2. Open cPanel -> **Setup Python App** (under Software).
3. Create a new app:
   - Python version: highest available (3.8+)
   - Application root: `sos_backend`
   - Application URL: your domain/subdomain
   - Application startup file: `passenger_wsgi.py`
4. Click **Create**. Note the "Enter virtual environment" command shown.

## Step 4 - Install and prepare (in cPanel Terminal)

```bash
source /home/USERNAME/virtualenv/sos_backend/3.x/bin/activate
cd ~/sos_backend
pip install -r requirements.txt
python manage.py migrate
python manage.py createsuperuser     # admin login (optional)
python manage.py collectstatic       # for /admin styling
```

## Step 5 - Restart and verify

1. Back in **Setup Python App**, click **Restart**.
2. Visit `https://yourdomain.com/dashboard/` -> should show the empty table.
3. Visit `/admin/`, log in, add/edit alerts manually if needed.

## Step 6 - Point the ESP32 at it

In `lora_reciever.ino.ino` set:

```cpp
const char* SERVER_URL = "http://yourdomain.com/api/sos/";
const char* API_KEY    = "same-key-as-settings-SOS_API_KEY";
```

(Wiring/code for WiFi + HTTPClient will be added next.)

---

## Security notes

- Keep `DEBUG = False` in production.
- Change both `SECRET_KEY` and `SOS_API_KEY` from the defaults.
- If your host has AutoSSL (HTTPS), you can use `https://` in `SERVER_URL`;
  the ESP32 code will skip certificate verification.
- GET endpoints are open so the dashboard can poll; writes need the API key.

## Troubleshooting

| Problem | Fix |
|---|---|
| 500 error / Passenger message | Check cPanel Python app error log; confirm startup file is `passenger_wsgi.py` |
| `ModuleNotFoundError: rest_framework` | venv not activated when running pip/migrate; redo Step 4 with `source ...activate` |
| ESP32 gets 400 | Bad JSON - check Serial output of receiver |
| ESP32 gets 403 | Wrong/missing `X-API-Key` header vs settings |
| Dashboard shows "Server offline" | API down or wrong domain path |
