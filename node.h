#ifndef Node_h
#define Node_h

#include "stdint.h"
#include <InfluxDbClient.h>
#include <Preferences.h>

extern Point node;
extern uint64_t setupMillis;
extern Preferences config;

void setupNode(void);
void captureNodeFields(void);

#endif
