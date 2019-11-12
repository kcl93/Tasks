Arduino Task Scheduler
----------------------

This library implements a simple, preemptive task scheduler that is executed in parallel to the 1ms timer interrupt 
used for the Arduino millis() function. It allows to define cyclic tasks or tasks that should be executed in the future
in parallel to the normal program execution inside the main loop.

The task scheduler is executed every 1ms. A possibly running task is interrupted by this and only resumed after all succeeding tasks have finished. This means 
that always the task started last has the highest priority. This effect needs to be kept in mind when programming a software using this library.

Notes:
  - Deadlocks can appear when one task waits for another taks which was started before.
  - Timing critical tasks may not execute properly when they are interrupted for too long by other tasks. Thus it is recommended to keep task execution as short as possible.
  - The Arduino MEGA leaves the interrupts state shortly after starting the task scheduler which makes the scheduler reentrant and allows any other interrupt (timer, UART, etc.) to be triggered.
  - The Arduino DUE enables other interrupts by using the lowest possible priority (15) for the task scheduler interrupt.

Warning: The Arduino Due does not support reeantrant interrupts due to HW limitations. This means that unlike the Arduino MEGA the DUE is not able to execute 
fast 1ms tasks several times before finishing a slower tasks with for example a 10ms timebase. This problem will be especially visible if the sum of the 
execution time of all tasks is greater than the execution period of the fastest task.

Supported Boards:
  - all boards using the Atmel ATMega328 controller, e.g. Arduino Uno and Nano
  - all boards using the Atmel ATMega2560 controller, e.g. Arduino Mega
  - all boards using the Atmel SAM3X8E controller, e.g. Arduino Due

Consumed interrupt:
  - Atmel ATMega328 & ATMega2560: Scheduler uses TIMER0_COMPA interrupt. This maintains millis() and analogWrite() functionality on T0 pins. However, 
    frequent changes of the duty cycle using analogWrite() lead to a jitter in scheduler timing.
  - Atmel SAM3X8E: Scheduler uses TC3 interrupt.

CPU runtime:
  - Atmel ATMega328 & ATMega2560:
    - 5&mu;s without pending tasks
    - 12&mu;s + task duration when tasks are executed
  - Atmel SAM3X8E:
    - tbd&mu;s without pending tasks
    - tbd&mu;s + task duration when tasks are executed

Have fun!

====================================

# Revision History

1.0 (2019-10-12): 
  - initial release
