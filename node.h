#ifndef Node_h
#define Node_h

#include "definitions.h"
#include "credentials.h"
#include "network.h"
#include <WString.h>
#include <WiFi.h>
#include <NTPClient.h>

class Node {
  private:
    String rootUri = ROOT_URI;
    String nodeId;
    long lastIdentified;
    int wifiStatus = WL_DISCONNECTED;
  public:
    Node();
    virtual void identify();
    virtual void log(String sensorType, double value);
    virtual int readLight();
    virtual void initWifi();
    virtual void connectWifi();
    virtual void checkWifi();
    virtual void sleep();
    virtual void wake();
};

#endif
