#ifndef Influx_H
#define Influx_H

#include <InfluxDbClient.h>

void setupInfluxOptions(void);
bool validateInfluxConnection(void);
bool logPoint(Point &point);

#endif
