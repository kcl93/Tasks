// include files
#include "Test_Class.h"
#include "Tasks.h"

// Create class instances
TestClass  item1(1);
TestClass  item2(2);

// For non-static class methods store pointer to lambda-function ONCE, see https://stackoverflow.com/questions/53091205/how-to-use-non-static-member-functions-as-callback-in-c
Task item1_print = PTR_NON_STATIC_METHOD(item1, print);
Task item2_print = PTR_NON_STATIC_METHOD(item2, print);


void setup()
{
  // open port to PC
  Serial.begin(115200);

  Serial.println("Start tasks");
     
  // Init task scheduler
  Tasks_Init();
  Tasks_Add(item1_print, 1000, 0);      // Use stored pointers for non-static member functions!
  Tasks_Add(item2_print, 1000, 500);
  Tasks_Start();

} // setup()


void loop() {

  if (millis() > 3000) {
    Serial.println("Change tasks");
    Tasks_Delay(item1_print, 2000);
    Tasks_Pause_Task(item2_print);
    while(1);
  }

} // loop()
