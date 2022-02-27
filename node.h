#ifndef Node_h
#define Node_h

#include <InfluxDbClient.h>
#include <Preferences.h>

extern Point node;
extern uint64_t setupMillis;
extern Preferences config;

void setNodeTags(void);
void captureNodeFields(void);
void nodeLoggerTask(void* parameters);

#endif
