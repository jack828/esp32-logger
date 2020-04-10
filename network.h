#ifndef Network_h
#define Network_h

#include "definitions.h"
#include <HTTPClient.h>
#include <WString.h>

typedef struct Response {
  int statusCode;
  String body;
};

Response getRequest(String uri);
Response postRequest(String uri, String payload);

#endif
