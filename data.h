const char index_html[] PROGMEM = "<!DOCTYPE html><html>"
"<body>"
  "<h1>ESP32 Logger</h1>"
  "<p>MAC: %MAC%</p>"
  "<p>Firmware: %FIRMWARE_VERSION%</p>"
  "<p>Uptime: %UPTIME%</p>"
  "<form method=\"post\">"
    "<h2>Node Management</h2>"
    "<div>"
      "<label for=\"name\">Name: </label>"
      "<input name=\"name\" value=\"%NAME%\" />"
    "</div>"
    "<div>"
      "<label for=\"location\">Location: </label>"
      "<input name=\"location\" value=\"%LOCATION%\" />"
    "</div>"
    "<button type=\"submit\">Save</button>"
  "</form>"

  "<div>"
    "<h2>Firmware</h2>"
    "<a href=\"/update\">Please use the ElegantOTA solution -></a>"
  "</div>"
"</body>"
"</html>";
