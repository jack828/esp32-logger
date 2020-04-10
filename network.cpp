#include "network.h"

Response getRequest(String uri) {
  HTTPClient http;
  Response response;

  Serial.println("[ HTTP ] [ GET ] begin: " + uri);

  http.begin(uri);

  int statusCode = http.GET();
  response.statusCode = statusCode;

  if (statusCode > 0) {
    Serial.printf("[ HTTP ] [ GET ] code: %d\n", statusCode);

    if (statusCode == HTTP_CODE_OK) {
      response.body = http.getString();
      Serial.printf("[ HTTP ] [ GET ] payload: %d\n", response.body.length());
      Serial.println(response.body);
    }
  } else {
    Serial.printf(
      "[ HTTP ] [ GET ] failed, error: %s\n",
      http.errorToString(statusCode).c_str()
    );
  }

  http.end();
  return response;
}

Response postRequest(String uri, String payload) {
  HTTPClient http;
  Response response;

  Serial.println("[ HTTP ] [ POST ] begin: " + uri);

  http.addHeader("Content-Type", "text/plain");
  http.begin(uri);

  // FIXME I get a weird core panic when actually using this.
  // For now, it'll be through GET params
  int statusCode = http.POST("");
  response.statusCode = statusCode;

  if (statusCode > 0) {
    Serial.printf("[ HTTP ] [ POST ] code: %d\n", statusCode);

    if (statusCode == HTTP_CODE_OK) { }
    response.body = http.getString();
  } else {
    Serial.printf(
      "[ HTTP ] [ POST ] failed, error: %s\n",
      http.errorToString(statusCode).c_str()
    );
  }

  http.end();
  return response;
}
