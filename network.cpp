#include "network.h"

String getRequest(String uri) {
  HTTPClient http;
  String result = "";

  Serial.println("[ HTTP ] [ GET ] begin: " + uri);

  http.begin(uri);

  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.printf("[ HTTP ] [ GET ] code: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      result = payload;
      Serial.printf("[ HTTP ] [ GET ] payload: %d\n", payload.length());
      Serial.println(payload);
    }
  } else {
    Serial.printf("[ HTTP ] [ GET ] failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
  return result;
}
