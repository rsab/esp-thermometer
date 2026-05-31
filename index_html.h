#pragma once

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Thermometer</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }

    body {
      font-family: 'Segoe UI', sans-serif;
      background: #0f172a;
      color: #e2e8f0;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      min-height: 100vh;
    }

    h1 {
      font-size: 1.4rem;
      letter-spacing: 0.15em;
      text-transform: uppercase;
      color: #94a3b8;
      margin-bottom: 2rem;
    }

    .card {
      background: #1e293b;
      border-radius: 1.5rem;
      padding: 3rem 4rem;
      text-align: center;
      box-shadow: 0 20px 60px rgba(0,0,0,0.4);
    }

    .temp-value {
      font-size: 6rem;
      font-weight: 700;
      line-height: 1;
      color: #f8fafc;
      letter-spacing: -0.02em;
    }

    .temp-value span {
      font-size: 2.5rem;
      color: #38bdf8;
      vertical-align: super;
      margin-left: 0.2rem;
    }

    .status {
      margin-top: 1.5rem;
      font-size: 0.85rem;
      color: #475569;
    }

    .dot {
      display: inline-block;
      width: 8px;
      height: 8px;
      border-radius: 50%;
      background: #22c55e;
      margin-right: 6px;
      animation: pulse 2s infinite;
    }

    .dot.error { background: #ef4444; animation: none; }

    .config-link {
      margin-top: 2rem;
      font-size: 0.75rem;
      color: #334155;
      text-decoration: none;
    }
    .config-link:hover { color: #94a3b8; }

    @keyframes pulse {
      0%, 100% { opacity: 1; }
      50% { opacity: 0.3; }
    }
  </style>
</head>
<body>
  <h1>DS18B20 Thermometer</h1>
  <div class="card">
    <div class="temp-value" id="temp">--<span>°C</span></div>
    <div class="status"><span class="dot" id="dot"></span><span id="status">Connecting...</span></div>
  </div>
  <a class="config-link" href="/config">OTLP settings</a>

  <script>
    const tempEl   = document.getElementById('temp');
    const statusEl = document.getElementById('status');
    const dotEl    = document.getElementById('dot');

    function setTemp(value) {
      tempEl.innerHTML = value + '<span>°C</span>';
    }

    async function fetchTemp() {
      try {
        const res  = await fetch('/temperature');
        const data = await res.json();

        if (data.error) {
          setTemp('--');
          dotEl.className = 'dot error';
          statusEl.textContent = 'Sensor error';
        } else {
          setTemp(data.temperature.toFixed(1));
          dotEl.className = 'dot';
          statusEl.textContent = 'Updated ' + new Date().toLocaleTimeString();
        }
      } catch (e) {
        dotEl.className = 'dot error';
        statusEl.textContent = 'Connection lost';
      }
    }

    fetchTemp();
    setInterval(fetchTemp, 3000);
  </script>
</body>
</html>
)rawliteral";
