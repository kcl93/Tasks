/**
    \file       Tasks.h
    \brief      Library providing a simple task scheduler for multitasking.
    \details    This library implements a very basic scheduler that is executed in parallel to the 1ms timer 
                interrupt used for the millis() function.
                It enables users to define cyclic tasks or tasks that should be executed in the future in 
                parallel to the normal program execution inside the main loop.
                <br>The task scheduler is executed every 1ms.
                <br>The currently running task is always interrupted by this and only continued to be executed
                after all succeeding tasks have finished.
                This means that always the task started last has the highest priority.
                This effect needs to be kept in mind when programming a software using this library.
                <br>Deadlocks can appear when one task waits for another taks which was started before.
                Additionally it is likely that timing critical tasks will not execute properly when they are
                interrupted for too long by other tasks.
                Thus it is recommended to keep the tasks as small and fast as possible.
                <br>The Arduino ATMega (8-bit AVR) leaves the interrupts state shortly after starting the task scheduler which
                makes the scheduler reentrant and allows any other interrupt (timer, UART, etc.) to be triggered.
                The Arduino SAM (32-bit ARM) enables other interrupts by using the lowest possible priority (15) for the 
                task scheduler interrupt.
                <br><b>Warning</b> The Arduino SAM does not support reeantrant interrupts due to HW limitations.
                This means that unlike the Arduino ATMega the DUE is not able to execute fast 1ms tasks several 
                times before finishing a slower tasks with for example a 10ms timebase.
                This problem will be especially visible if the sum of the execution time of all tasks is greater
                than the execution period of the fastest task.
    \author     Kai Clemens Liebich, Georg Icking-Konert
    \date       2019-11-20
    \version    1.3
*/


/*-----------------------------------------------------------------------------
        MODULE DEFINITION FOR MULTIPLE INCLUSION
-----------------------------------------------------------------------------*/
#ifndef TASKS_H
#define TASKS_H


/*-----------------------------------------------------------------------------
        INCLUDE FILES
-----------------------------------------------------------------------------*/
#include <Arduino.h>


/*-----------------------------------------------------------------------------
        COMPILER OPTIONS
-----------------------------------------------------------------------------*/
#pragma GCC optimize ("O2")


/*-----------------------------------------------------------------------------
        GLOBAL MACROS
-----------------------------------------------------------------------------*/
#define MAX_TASK_CNT    8   //!< Maximum number of parallel tasks
//#define PTR_NON_STATIC_METHOD(instance, method)    [instance](){instance.method();}    //!< Get pointer to non-static member function via lambda function, see https://stackoverflow.com/questions/53091205/how-to-use-non-static-member-functions-as-callback-in-c


/*-----------------------------------------------------------------------------
        GLOBAL CLASS
-----------------------------------------------------------------------------*/

typedef void (*Task)(void); //!< Example prototype for a function than can be executed as a task



/**
    \brief      Initialize timer and reset the tasks scheduler at first call.
    \details    This function initializes the related timer and clears the task scheduler at first call.
                <br><br>Used HW blocks:
                <br>- Arduino ATMega: TIMER0_COMPA_vect
                <br>- Arduino SAM: TC3
*/
void Tasks_Init(void);



/**
    \brief      Reset the tasks schedulder.
    \details    This function clears the task scheduler. Use with caution!
                <br><br>Used HW blocks:
                <br>- Arduino ATMega: TIMER0_COMPA_vect
                <br>- Arduino SAM: TC3
*/
void Tasks_Clear(void);



/**
    \brief      Add a task to the task scheduler.
    \details    A new task is added to the scheduler with a given execution period and delay until first execution.
                <br>If no delay is given the task is executed at once or after starting the task scheduler 
                (see Tasks_Start())
                <br>If a period of 0ms is given, the task is executed only once and then removed automatically.
                <br>To avoid ambiguities, a function can only be added once to the scheduler.
                Trying to add it a second time will reset and overwrite the settings of the existing task.
                <br>For non-static member function use address from PTR_NON_STATIC_METHOD() macro. 
                <br><br>Used HW blocks:
                <br>- Arduino ATMega: TIMER0_COMPA_vect
                <br>- Arduino SAM: TC3
    \param[in]  func    Function to be executed.<br>The function prototype should be similar to this:
                        "void userFunction(void)"
    \param[in]  period  Execution period of the task in ms (0 to 32767; 0 = task only executes once) 
    \param[in]  delay   Delay until first execution of task in ms (0 to 32767)
    \return     true in case of success,
                false in case of failure (max. number of tasks reached, or duplicate function)
    \note       The maximum number of tasks is defined as <tt>MAX_TASK_CNT</tt> in file <tt>Tasks.h</tt>
*/
bool Tasks_Add(Task func, int16_t period, int16_t delay = 0);



/**
    \brief      Remove a task from the task scheduler.
    \details    Remove the specified task from the scheduler and free the slot again.
                <br>For non-static member function use address from PTR_NON_STATIC_METHOD() macro. 
                <br><br>Used HW blocks:
                <br>- Arduino ATMega: TIMER0_COMPA_vect
                <br>- Arduino SAM: TC3
    \param[in]  func    Function name that should be removed.
    \return     true in case of success, 
                false in case of failure (e.g. function not in not in scheduler table)
*/
bool Tasks_Remove(Task func);



/**
    \brief      Delay execution of a task
    \details    The task is delayed starting from the last 1ms timer tick which means the delay time 
                is accurate to -1ms to 0ms.
                <br>This overwrites any previously set delay setting for this task and thus even allows
                earlier execution of a task.
                Delaying the task by <2ms forces it to be executed during the next 1ms timer tick.
                This means that the task might be called at any time anyway in case it was added multiple 
                times to the task scheduler.
                <br>For non-static member function use address from PTR_NON_STATIC_METHOD() macro. 
                <br><br>Used HW blocks:
                <br>- Arduino ATMega: TIMER0_COMPA_vect
                <br>- Arduino SAM: TC3
    \param[in]  func    Function that should be delayed
    \param[in]  delay   Delay in ms (0 to 32767)
    \return     true in case of success, 
                false in case of failure (e.g. function not in not in scheduler table)
*/
bool Tasks_Delay(Task func, int16_t delay);



/**
    \brief      Enable or disable the execution of a task
    \details    Temporary pause or resume function for execution of single tasks by scheduler.
                This will not stop the task in case it is currently being executed but just prevents 
                the task from being executed again in case its state is set to 'false' (inactive).
                <br>For non-static member function use address from PTR_NON_STATIC_METHOD() macro. 
                <br><br>Used HW blocks:
                <br>- Arduino ATMega: TIMER0_COMPA_vect
                <br>- Arduino SAM: TC3
    \param[in]  func    Function to be paused/resumed.
                <br>The function prototype should be similar to this: "void userFunction(void)"
    \param[in]  state New function state (false=pause, true=resume)
    \return     'true' in case of success, else 'false' (e.g. function not in not in scheduler table)

*/
bool Tasks_SetState(Task func, bool state);



/**
    \brief      Activate a task in the scheduler
    \details    Resume execution of the specified task. Possible parallel tasks are not affected. 
                This is a simple inlined function setting the 'state' argument for Tasks_SetState().
                <br>For non-static member function use address from PTR_NON_STATIC_METHOD() macro. 
                <br><br>Used HW blocks:
                <br>- Arduino ATMega: TIMER0_COMPA_vect
                <br>- Arduino SAM: TC3
    \param[in]  func    Function to be activated 
    \return     true in case of success, 
                false in case of failure (e.g. function not in not in scheduler table)
*/
inline bool Tasks_Start_Task(Task func)
    {
        return Tasks_SetState(func, true);
    }



/**
    \brief      Deactivate a task in the scheduler
    \details    Pause execution of the specified task. Possible parallel tasks are not affected. 
                This is a simple inlined function setting the 'state' argument for Tasks_SetState().
                <br>For non-static member function use address from PTR_NON_STATIC_METHOD() macro. 
                <br><br>Used HW blocks:
                <br>- Arduino ATMega: TIMER0_COMPA_vect
                <br>- Arduino SAM: TC3
    \param[in]  func    Function to be paused 
    \return     true in case of success, 
                false in case of failure (e.g. function not in not in scheduler table)
*/
inline bool Tasks_Pause_Task(Task func)
    {
        return Tasks_SetState(func, false);
    }



/**
    \brief      Start the task scheduler
    \details    Resume execution of the scheduler. All active tasks are resumed. 
                <br><br>Used HW blocks:
                <br>- Arduino ATMega: TIMER0_COMPA_vect
                <br>- Arduino SAM: TC3
*/
void Tasks_Start(void);



/**
    \brief      Pause the task scheduler
    \details    Pause execution of the scheduler. All tasks are paused. 
                <br><br>Used HW blocks:
                <br>- Arduino ATMega: TIMER0_COMPA_vect
                <br>- Arduino SAM: TC3
*/
void Tasks_Pause(void);


#endif        //TASKS_H