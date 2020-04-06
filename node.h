#ifndef Node_h
#define Node_h

#include "definitions.h"
#include "network.h"
#include <WString.h>
#include <WiFi.h>
#include <NTPClient.h>

class Node {
  private:
    String rootUri = ROOT_URI;
    String nodeId;
    long lastIdentified;
  public:
    int identifier;
    Node();
    virtual void identify();
};

#endif
