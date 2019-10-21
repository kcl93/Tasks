
/**
  \file
  \example  Multiple_Tasks.ino
  \brief    Example project demonstrating how to use the scheduler library.
  \details  This example implements the famous Blink demo using task scheduler library.
            <br>Tasks can be executed cyclically or only once with or without a delay. 
            The starting time of a cyclic task is depenend on the current load of other tasks 
            and can be delayed by a few ms even though no delay was given.
  \author   Kai Clemens Liebich
  \date     08.11.2018
*/

#include <Tasks.h>


// scheduler task
void blink(void) {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}


void setup()
{
  // set pin to output
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Configure task scheduler
  Tasks_Init();
  Tasks_Add(blink, 500, 0);
  Tasks_Start();
}



void loop()
{
  // dummy
}
