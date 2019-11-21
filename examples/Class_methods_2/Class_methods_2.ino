/**
  \file     Class_methods_2.ino
  \example  Class_methods_2.ino
  \brief    Example project demonstrating how to access non-static methods and data from within the class
  \details  This example shows how non-static data can be accessed from scheduler via static instance pointers.
            This workaround is required, because pointers to non-static member functions are not supported in C++.
            <br>Tasks can be executed cyclically or only once with or without a delay. 
            The starting time of a cyclic task is depenend on the current load of other tasks 
            and can be delayed by a few ms even though no delay was given.
  \author   Georg Icking-Konert
  \date     20.11.2019
*/

// include files
#include "Test_Class.h"
#include "Tasks.h"


// Create class instances
TestClass  item1(1);
TestClass  item2(2);


// Handlers for non-static class member functions
void item1_print(void) { item1.print(); }
void item2_print(void) { item2.print(); }


void setup()
{
  // open port to PC
  Serial.begin(115200);
  while(!Serial);

  Serial.println("Start tasks");
     
  // Init task scheduler
  Tasks_Init();
  Tasks_Add((Task) item1_print, 1000, 0);      // Use handlers for non-static member functions!
  Tasks_Add((Task) item2_print, 1000, 500);
  Tasks_Start();

} // setup()


void loop() {

  if (millis() > 3000) {
    Serial.println("Change tasks");
    Tasks_Delay((Task) item1_print, 2000);
    Tasks_Pause_Task((Task) item2_print);
    while(1);
  } 

} // loop()
