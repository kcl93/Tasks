
/**
  \file
  \example  Multiple_Tasks.ino
  \brief    Example project demonstrating how to use the scheduler library.
  \details  This example extends the famous Blink demo for 2 pins using task scheduler library.
            Note that the slow task is blocking for longer than the fast task period!
            <br>Tasks can be executed cyclically or only once with or without a delay. 
            The starting time of a cyclic task is depenend on the current load of other tasks 
            and can be delayed by a few ms even though no delay was given.
  \author   Kai Clemens Liebich
  \date     08.11.2018
*/

#include <Tasks.h>

// define test pins
#define PIN1    8
#define PIN2    9


// scheduler task 1: toggle PIN1 every 10ms and block for 9ms
void toggle_1(void) {
  digitalWrite(PIN1, !digitalRead(PIN1));
  delay(5);
}


// scheduler task 2: toggle PIN1 every 1ms
void toggle_2(void) {
  digitalWrite(PIN2, !digitalRead(PIN2));
}


void setup()
{
  // set pin to output
  pinMode(PIN1, OUTPUT);
  pinMode(PIN2, OUTPUT);
  
  // Configure task scheduler
  Tasks_Init();
  Tasks_Add((Task) toggle_1, 10, 0);
  Tasks_Add((Task) toggle_2, 1, 0);
  Tasks_Start();
}



void loop()
{
  // dummy
}
