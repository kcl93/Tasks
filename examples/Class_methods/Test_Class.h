/**
  Example class with non-static member functions to be called
  by scheduler via external wrapper function, see Class_method.ino
*/

// include files
#include "Arduino.h"

class TestClass {
  public:
    uint8_t   idx=0;
              TestClass(uint8_t Idx);
    void      print(void);
};
