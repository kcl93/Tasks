/**
  \file     Scheduling_1.ino
  \example  Scheduling_1.ino
  \brief    Example project demonstrating how to use the scheduler library.
  \details  This example shows how tasks can be setup in order to be executed in parallel to the main program.
            <br>Tasks can be executed cyclically or only once with or without a delay. 
            The starting time of a cyclic task is depenend on the current load of other tasks 
            and can be delayed by a few ms even though no delay was given.
  \author   Kai Clemens Liebich
  \date     08.11.2018
*/

#include <Tasks.h>


// helper routine
void print_time(int num) {
  Serial.print("task_");
  Serial.print(num);
  Serial.print(" ");
  Serial.println(millis());
}


// scheduler tasks
void task_1(void) { print_time(1); }
void task_2(void) { print_time(2); }
void task_3(void) { print_time(3); }
void task_4(void) { print_time(4); }
void task_5(void) { print_time(5); }


void setup()
{
  Serial.begin(115200);
  
  // Init task scheduler
  Tasks_Init();

  // print delay between calls to serial console
  Tasks_Add(task_1, 1000, 0);
  Tasks_Add(task_2, 1000, 100);
  Tasks_Add(task_3, 1000, 200);
  Tasks_Add(task_4, 1000, 300);
  Tasks_Add(task_5, 1000, 400);
  
  // Start task scheduler
  Tasks_Start();
}



void loop()
{
  // dummy
}
