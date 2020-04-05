#include "definitions.h"
#include <HTTPClient.h>

String getRequest(String uri) {
  HTTPClient http;

  Serial.println("[ HTTP ] [ GET ] begin: " + uri);

  http.begin(uri);

  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.printf("[ HTTP ] [ GET ] code: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
    }
  } else {
    Serial.printf("[ HTTP ] [ GET ] failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
  String result = "";
  return result;
}
