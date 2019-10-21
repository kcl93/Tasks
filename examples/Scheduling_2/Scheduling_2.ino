/**
  \file     Scheduling_2.ino
  \example  Scheduling_2.ino
  \brief    Example project demonstrating how to use the scheduler library.
  \details  This example shows how tasks can be setup in order to be executed in parallel to the main program.
            <br>Tasks can be executed cyclically or only once with or without a delay. 
            The starting time of a cyclic task is depenend on the current load of other tasks 
            and can be delayed by a few ms even though no delay was given.
  \author   Kai Clemens Liebich
  \date     08.11.2018
*/

#include <Tasks.h>

// global defines
#define PERIOD_PRINT 500
#define PERIOD_LED   300


// global variables
int16_t g_period = 0;       // pass variable from loop() to background task via globals


// scheduler tasks
void print_pause(void);
void toggle_LED(void);


// helper routine
void print_help()
{
  Serial.println("scheduler command keys:");
  Serial.println("  h = print this help");
  Serial.println("  1 = decrease delay");
  Serial.println("  2 = increase delay");
  Serial.println("  3 = pause task");
  Serial.println("  4 = resume task");
  Serial.println("  5 = pause scheduler");
  Serial.println("  6 = resume scheduler");  
}


void setup()
{
  Serial.begin(115200);               // open connection to PC
  pinMode(LED_BUILTIN, OUTPUT);       // set LED to output

  // print help
  print_help();
  
  // Init task scheduler
  Tasks_Init();

  // print delay between calls to serial console
  Tasks_Add(print_pause, PERIOD_PRINT, 500);
  Tasks_Add(toggle_LED, PERIOD_LED, 500);

  // Start task scheduler
  Tasks_Start();
}


void loop()
{
  // control scheduler via serial console
  if (Serial.available())
  { 
    char cmd = Serial.read();
    switch (cmd) {

      case 'h':
        print_help();
        break;

      case '1':
        Serial.println("decrease delay");
        g_period = PERIOD_PRINT / 2;
        Tasks_Delay(print_pause, 1); // force next execution
        break;

      case '2':
        Serial.println("increase delay");
        g_period = PERIOD_PRINT * 2;
        Tasks_Delay(print_pause, 1); // force next execution
        break;

      case '3':
        Serial.println("pause task");
        Tasks_Pause_Task(print_pause);
        break;

      case '4':
        Serial.println("resume task");
        Tasks_Start_Task(print_pause);
        break;        

      case '5':
        Serial.println("pause scheduler");
        Tasks_Pause();
        break;

      case '6':
        Serial.println("resume scheduler");
        Tasks_Start();
        break;        

      default:
        break;
    }
  }
  
} // loop()


void print_pause(void)
{
  static uint32_t old_time = millis() - PERIOD_PRINT;
  if (g_period != 0)
    Tasks_Delay(print_pause, g_period);
  else {
    Serial.print("pause = ");
    Serial.println(millis() - old_time);
  }
  
  g_period = 0;
  old_time = millis();
}


void toggle_LED(void)
{
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));  
}
