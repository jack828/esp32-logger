#ifndef Sensors_h
#define Sensors_h

#include <InfluxDbClient.h>
#include <Preferences.h>

extern Point sensors;
extern Preferences config;

void setupSensors(void);
void setSensorsTags(void);
void captureSensorFields(void);
void sensorsLoggerTask(void* parameters);

// Internal common sensor functions
double calculateVpd(double temperature, double humidity);

#endif
