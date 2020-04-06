#ifndef Node_h
#define Node_h

#include "definitions.h"
#include "network.h"
#include <WString.h>
#include <WiFi.h>

class Node {
  private:
    String rootUri;
    String nodeId;
    long lastIdentified;
  public:
    int identifier;
    Node(String rootUri);
    virtual void identify();
};

#endif
