/**
  Example class to demonstrate how to access non-static class variables via scheduler.
  Workaround via static pointers and functions is required for scheduler, because pointers 
  to non-static member functions or access to non-static data are not supported in C++.
*/

// include files
#include "Arduino.h"

class TestClass {

  private:
  
    // non-static data to access
    uint8_t             idx = 0;
  
  public:
    // static instance pointer. Required to access non-static data in static functions
    static TestClass*   ptr_TestClass_1;
    static TestClass*   ptr_TestClass_2;

  public:
                        TestClass(uint8_t Idx);
 
    // non-static method to call externally via wrapper
    void                print(void);

    // static member function, one per instance (required for scheduler)
    static void         print_delayed_1(void);
    static void         print_delayed_2(void);
};
