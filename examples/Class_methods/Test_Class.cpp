// include files
#include "Test_Class.h"
#include "Arduino.h"

TestClass::TestClass(uint8_t Idx) {
  this->idx = Idx;
}

void TestClass::print(void) {
  Serial.print("  instance ");
  Serial.print(this->idx);
  Serial.print(": ");
  Serial.println(millis());
}
