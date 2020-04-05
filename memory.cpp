#include "definitions.h"
#include <EEPROM.h>

int writeDeviceId(String idStr) {
  int idAddress[ID_SIZE] = ID_ADDRESS;
  const char* id = idStr.c_str();

  for (int i = 0; i <= ID_SIZE; i++) {
    EEPROM.write(idAddress[i], id[i]);
    Serial.print(EEPROM.read(idAddress[i]));
  }
  Serial.println();
  EEPROM.commit();
  return 0;
}

String readDeviceId() {
  int idAddress[ID_SIZE] = ID_ADDRESS;
  char id[ID_SIZE];

  for (int i = 0; i < ID_SIZE; i++) {
    id[i] = (char) EEPROM.read(idAddress[i]);
  }

  return String(id);
}
