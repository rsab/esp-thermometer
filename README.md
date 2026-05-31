# ESP8266 WiFi Thermometer

Hobby project to have a temperature monitoring on Grafana Cloud for a fish tank.

Reads temperature from a DS18B20 sensor and serves it via a browser UI. Pushes metrics every 30 seconds over OTLP/HTTP.

## Connection diagram

```
NodeMCU ESP8266                         DS18B20
╔══════════════════╗                  ┌─────────┐
║                  ║                  │  ╔═══╗  │
║  3V3 ───────────────────────────────╫──╢VCC╟  │
║                  ║         │        │  ╚═══╝  │
║                  ║        4.7kΩ     │         │
║                  ║         │        │  ╔════╗ │
║   D4 ───────────────────────────────╫──╢DATA╟ │
║ (GPIO2)          ║                  │  ╚════╝ │
║                  ║                  │  ╔════╗ │
║  GND ───────────────────────────────╫──╢GND ╟ │
║                  ║                  │  ╚════╝ │
╚══════════════════╝                  └─────────┘

Wire colors (standard DS18B20):
  Red    → VCC  (3.3V)
  Black  → GND
  Yellow → DATA (D4)

Pull-up: 4.7kΩ resistor between DATA and VCC (required)
```

## Hardware

| Component | Details |
|-----------|---------|
| Microcontroller | ESP8266 NodeMCU v2 |
| Sensor | DS18B20 (1-Wire digital temperature) |
| Data pin | D4 (GPIO2) |
| Pull-up resistor | 4.7kΩ between DATA and VCC |
| Power | 3.3V from NodeMCU onboard regulator |

## Software

- **Arduino IDE** (not PlatformIO)
- Libraries: `ESP8266WiFi`, `ESP8266WebServer`, `ESP8266HTTPClient`, `WiFiClientSecure`, `WiFiManager`, `ArduinoJson`, `OneWire`, `DallasTemperature`

## Flashing

No credentials go in the source code. Just compile and flash — configuration is done via a captive portal on first boot.

```bash
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 esp-thermometer.ino
arduino-cli upload --fqbn esp8266:esp8266:nodemcuv2 --port /dev/ttyUSB0 esp-thermometer.ino
```

Or use Arduino IDE: select **Tools → Board → ESP8266 → NodeMCU 1.0 (ESP-12E Module)**, then upload.

## First-boot setup

1. Power on the device — it will start a WiFi access point named **`Thermometer-Setup`**
2. Connect to it from your phone or laptop
3. A captive portal opens automatically (if it doesn't, navigate to `http://192.168.4.1`)
4. Fill in:
   - **WiFi SSID and password**
   - **OTLP Endpoint URL** — e.g. `https://otlp-gateway-prod-eu-north-0.grafana.net/otlp/v1/metrics`
   - **OTLP Authorization** — the full `Basic <token>` header value from your Grafana Cloud connection details
5. Save — the device restarts and connects to your network

Credentials are stored on the device filesystem (`/config.json`) and loaded automatically on every subsequent boot. The portal times out after 3 minutes if nothing is entered; the device will restart and try again.

## Usage

After setup, the device connects to WiFi and starts serving. Check the serial monitor at **115200 baud** for the IP address:

```
Syncing NTP.... done
HTTP server started
IP: 192.168.1.42
OTLP push: HTTP 200
```

Then open `http://<ip>` in any browser on the same network.

### Endpoints

| Endpoint | Description |
|----------|-------------|
| `GET /` | Dark-theme web UI, auto-refreshes every 3s |
| `GET /temperature` | JSON: `{"temperature": 23.45}` |
| `GET /config` | Form to edit OTLP endpoint and authorization, takes effect immediately |
| `GET /reset` | Clears saved config and reboots into setup portal |

## Extracting values from the env file

The `env` file uses the OTLP exporter format. Map it to the config form like this:

```
OTEL_EXPORTER_OTLP_ENDPOINT  →  Endpoint URL  (append /v1/metrics)
OTEL_EXPORTER_OTLP_HEADERS   →  Authorization (URL-decode %20 to a space)
```

Example `env` file:
```
OTEL_EXPORTER_OTLP_ENDPOINT="https://otlp-gateway-prod-eu-north-0.grafana.net/otlp"
OTEL_EXPORTER_OTLP_HEADERS="Authorization=Basic%20M..."
```

| Config form field | Value to enter |
|-------------------|----------------|
| Endpoint URL | `https://otlp-gateway-prod-eu-north-0.grafana.net/otlp/v1/metrics` |
| Authorization | `Basic M...` (replace `%20` with a space, drop the `Authorization=` prefix) |

## Reconfiguring

To change OTLP settings, open `http://<device-ip>/config` — no restart needed, changes take effect immediately. There is also an "OTLP settings" link at the bottom of the main page.

To change WiFi credentials, open `http://<device-ip>/reset`. The device will erase all config and reboot into the `Thermometer-Setup` portal.

