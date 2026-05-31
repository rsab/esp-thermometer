#pragma once

const char CONFIG_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>OTLP Config</title>
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
      font-size: 1.2rem;
      letter-spacing: .15em;
      text-transform: uppercase;
      color: #94a3b8;
      margin-bottom: 2rem;
    }

    form {
      background: #1e293b;
      border-radius: 1.5rem;
      padding: 2rem 2.5rem;
      width: min(480px, 90vw);
      box-shadow: 0 20px 60px rgba(0,0,0,.4);
    }

    label {
      display: block;
      font-size: .8rem;
      color: #94a3b8;
      margin-bottom: .4rem;
    }

    input {
      width: 100%;
      padding: .6rem .8rem;
      background: #0f172a;
      border: 1px solid #334155;
      border-radius: .5rem;
      color: #f8fafc;
      font-size: .9rem;
      margin-bottom: 1.2rem;
    }

    button {
      width: 100%;
      padding: .75rem;
      background: #38bdf8;
      border: none;
      border-radius: .5rem;
      color: #0f172a;
      font-weight: 700;
      font-size: 1rem;
      cursor: pointer;
    }

    button:hover { background: #7dd3fc; }

    a {
      display: block;
      text-align: center;
      margin-top: 1rem;
      color: #475569;
      font-size: .8rem;
      text-decoration: none;
    }
  </style>
</head>
<body>
  <h1>OTLP Configuration</h1>
  <form method="POST">
    <label>Endpoint URL</label>
    <input name="otlp_url" value="%URL%">
    <label>Authorization (leave blank to keep current)</label>
    <input type="password" name="otlp_auth" placeholder="Basic ...">
    <button type="submit">Save</button>
  </form>
  <a href="/">&#8592; Back</a>
</body>
</html>
)rawliteral";
