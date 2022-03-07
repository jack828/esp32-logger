const char index_html[] PROGMEM = R"rawliteral(<!DOCTYPE html><html>
<body>
  <h1>ESP32 Logger</h1>
  <p>mDNS URL: <a href=\"http://%MDNS_HOSTNAME%.local\">%MDNS_HOSTNAME%.local</a></p>
  <p>IP: %IP%</p>
  <p>MAC: %MAC%</p>
  <p>Firmware: %FIRMWARE_VERSION%</p>
  <p>Uptime: %UPTIME%</p>
  <form method=\"post\">
    <h2>Node Management</h2>
    <div>
      <label for=\"name\">Name: </label>
      <input name=\"name\" value=\"%NAME%\" />
    </div>
    <div>
      <label for=\"location\">Location: </label>
      <input name=\"location\" value=\"%LOCATION%\" />
    </div>
    <button type=\"submit\">Save</button>
  </form>

  <div>
    <h2>Firmware</h2>
    <a href=\"/update\">Please use the ElegantOTA solution -></a>
  </div>
</body>
</html>)rawliteral";

const char update_html[] PROGMEM = R"rawliteral(<!DOCTYPE html><html>
<head>
  <meta http-equiv=\"refresh\" content=\"5;url=/\">
</head>
<body>
  <h1>Config updated.</h1>
</body>
</html>)rawliteral";
