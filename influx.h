#ifndef Influx_H
#define Influx_H

#include <InfluxDbClient.h>

bool validateInfluxConnection();
bool logPoint(Point &point);

#endif
