#ifndef Node_h
#define Node_h

#include "../definitions.h"

typedef enum  {
  TEMPERATURE = 0,
  HUMIDITY,
  PRESSURE,
  LIGHT
} node_sensor_t;

char* sensor_to_string(node_sensor_t sensor);

void node_identify(void);
void node_log(char* sensorType, double value);
void node_sleep(void);
void node_wake(void);

#endif
