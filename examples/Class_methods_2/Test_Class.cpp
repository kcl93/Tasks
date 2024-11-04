/**
  Example class to demonstrate how to access non-static class variables via scheduler.
  Workaround via static pointers and functions is required for scheduler, because pointers 
  to non-static member functions or access to non-static data are not supported in C++.
*/

// include files
#include "Arduino.h"
#include "Tasks.h"
#include "Test_Class.h"

// allocate static class variables
TestClass*  TestClass::ptr_TestClass_1 = NULL;
TestClass*  TestClass::ptr_TestClass_2 = NULL;


TestClass::TestClass(uint8_t Idx) {
  
  // store instance index
  idx = Idx;

  // store pointer to instance in respective static pointer. Required to access non-static data in static functions
  if (idx == 1)
    ptr_TestClass_1 = this;
  else if (idx == 2)
    ptr_TestClass_2 = this;

} // constructor


void TestClass::print(void) {
  
  // print data
  Serial.print("  instance ");
  Serial.print(idx);
  Serial.print(" print(): ");
  Serial.println(millis());
  
  // within non-static method, call respective static method delayed
  if (idx == 1)
    Tasks_Add((Task) (TestClass::print_delayed_1), 0, 250);
   else if (idx == 2)
    Tasks_Add((Task) (TestClass::print_delayed_2), 0, 250);
 
} // print()


void TestClass::print_delayed_1(void) {
  
  // print data
  Serial.print("  instance ");
  Serial.print(ptr_TestClass_1->idx);
  Serial.print(" print_delayed(): ");
  Serial.println(millis());
  
} // print_delayed_1()


void TestClass::print_delayed_2(void) {
  
  // print data
  Serial.print("  instance ");
  Serial.print(ptr_TestClass_2->idx);
  Serial.print(" print_delayed(): ");
  Serial.println(millis());
  
} // print_delayed_2()
