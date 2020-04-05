#include "definitions.h"
#include <EEPROM.h>

int writeDeviceId(char* id) {
  int idAddress[ID_SIZE] = ID_ADDRESS;

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
    Serial.print("i    '");
    Serial.print(i);
    Serial.println("'");
    Serial.print("addr '");
    Serial.print(idAddress[i]);
    Serial.println("'");
    Serial.print("resu '");
    Serial.print(id[i]);
    Serial.println("'");
  }

  // Serial.println((char*) id);
  // Serial.println();
  return String(id);
}
