#ifndef Log_H
#define Log_H

#include <InfluxDbClient.h>

void validateInfluxConnection();
bool logPoint(Point &point);

#endif
