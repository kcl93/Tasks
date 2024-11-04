/**
    \file       Tasks.cpp
    \copybrief  Tasks.h
    \details    For more details please refer to Tasks.h
*/

#include "Tasks.h"


// check Arduino controller only once
#if !defined(__AVR__) && !defined(__SAM3X8E__)
    #error board not supported, error
#endif


/**************************************/
/******* start skip in doxygen ********/
/**************************************/
/// @cond INTERNAL

// measure speed via pin D9(=PB1). Is configured as OUTPUT in Scheduler_Start()
#define TASKS_MEASURE_PIN   0
#if (TASKS_MEASURE_PIN)
    #define SET_PIN         (PORTB |= B00000010)
    #define CLEAR_PIN       (PORTB &= ~B00000010)
    #define TOGGLE_PIN      (PORTB ^= B00000010)
#endif


// macro to pause / resume interrupt (interrupts are only reactivated in case they have been active in the beginning)
uint8_t oldISR = 0;
#if defined(__AVR__)
	#define PAUSE_INTERRUPTS    { oldISR = SREG; noInterrupts(); }
	#define RESUME_INTERRUPTS   { SREG = oldISR; interrupts();     }
#elif defined(__SAM3X8E__)
	#define PAUSE_INTERRUPTS    { oldISR = ((__get_PRIMASK() & 0x1) == 0 && (__get_FAULTMASK() & 0x1) == 0); noInterrupts(); }
	#define RESUME_INTERRUPTS   { if (oldISR != 0) { interrupts(); } }
#endif


// task container
struct SchedulingStruct
{
    Task    func;       // function to call
    bool    active;     // task is active
    bool    running;    // task is currently being executed
    int16_t period;     // period of task (0 = call only once)
    int16_t time;       // time of next call
};


// global variables for scheduler
struct SchedulingStruct SchedulingTable[MAX_TASK_CNT] = { {(Task)NULL, false, false, 0, 0} }; // array containing all tasks
bool    SchedulingActive;   // false = Scheduling stopped, true = Scheduling active (no configuration allowed)
int16_t _timebase;          // 1ms counter (on ATMega 1.024ms, is compensated)
int16_t _nexttime;          // time of next task call 
uint8_t _lasttask;          // last task in the tasks array (cauting! This variable starts is not counting from 0 to x but from 1 to x meaning that a single tasks will be at SchedulingTable[0] but _lasttask will have the value '1')


#if defined(__SAM3X8E__)
    /*
    Code from https://forum.arduino.cc/index.php?topic=130423.0
    ISR/IRQ     TC      Channel Due pins
    TC0         TC0     0       2, 13
    TC1         TC0     1       60, 61
    TC2         TC0     2       58
    TC3         TC1     0       none
    TC4         TC1     1       none
    TC5         TC1     2       none
    TC6         TC2     0       4, 5
    TC7         TC2     1       3, 10
    TC8         TC2     2       11, 12
    */
    void startTasksTimer(Tc *tc, uint32_t channel, IRQn_Type irq, uint32_t frequency)
    {
        pmc_set_writeprotect(false);
        pmc_enable_periph_clk((uint32_t)irq);
        TC_Configure(tc, channel, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK1);
        uint32_t rc = (SystemCoreClock >> 1)/frequency; //2 because we selected TIMER_CLOCK4 above
        //TC_SetRA(tc, channel, (rc >> 1)); //50% high, 50% low
        TC_SetRC(tc, channel, rc);
        TC_Start(tc, channel);
        tc->TC_CHANNEL[channel].TC_IER=TC_IER_CPCS;
        tc->TC_CHANNEL[channel].TC_IDR=~TC_IER_CPCS;
        NVIC_SetPriority(SysTick_IRQn, 8);
        NVIC_SetPriority(irq, 15);
        NVIC_EnableIRQ(irq);
    }
#endif // __SAM3X8E__


void Scheduler_update_nexttime(void)
{
    // stop interrupts, store old setting
    PAUSE_INTERRUPTS;
    
    // find time of next task execution    
    _nexttime = _timebase + INT16_MAX; // Max. possible delay of the next time
    for (uint8_t i = 0; i < _lasttask; i++)
    {
        if ((SchedulingTable[i].active == true) && (SchedulingTable[i].func != NULL))
        {
            //Serial.print(i); Serial.print("    "); Serial.println(SchedulingTable[i].time);

            if ((int16_t)(SchedulingTable[i].time - _nexttime) < 0)
            {
                _nexttime = SchedulingTable[i].time;
            }
        }
    }

    //Serial.print("timebase: "); Serial.println(_timebase);
    //Serial.print("nexttime: "); Serial.println(_nexttime);
    //Serial.println();

    //Serial.print(_timebase); Serial.print("    "); Serial.println(_nexttime - _timebase);
    
    // resume stored interrupt setting
    RESUME_INTERRUPTS;

} // Scheduler_update_nexttime()


/// @endcond
/************************************/
/******* end skip in doxygen ********/
/************************************/


void Tasks_Init(void)
{
    /*static bool flagDone = false;
    
    // clear tasks schedule only for first call to avoid issues when calling multiple times
    if (flagDone == false) {
        flagDone = true;
        Tasks_Clear();    
    }*/

} // Tasks_Init()



void Tasks_Clear(void)
{
    uint8_t i;
    
    // stop interrupts, store old setting
    PAUSE_INTERRUPTS;
    
    // init scheduler
    SchedulingActive = false;
    _timebase = 0;
    _nexttime = 0;
    _lasttask = 0;
    for(i = 0; i < MAX_TASK_CNT; i++)
    {
        //Reset scheduling table
        SchedulingTable[i].func = NULL;
        SchedulingTable[i].active = false;
        SchedulingTable[i].running = false;
        SchedulingTable[i].period = 0;
        SchedulingTable[i].time = 0;
    } // loop over scheduler slots
    
    // resume stored interrupt setting
    RESUME_INTERRUPTS;
    
} // Tasks_Clear()



bool Tasks_Add(Task func, int16_t period, int16_t delay)
{
    // Check range of period and delay
    if ((period < 0) || (delay < 0))
        return false;
    
    // workaround for 1.024ms timer period of Arduino ATMega
    #if defined(__AVR__)
        delay = (uint16_t)(((((int32_t)delay) * 250) + 128) >> 8); // delay = delay / 1.024 <-- with up/down rounding
        period = (uint16_t)(((((int32_t)period) * 250) + 128) >> 8); // period = period / 1.024 <-- with up/down rounding
    #endif
    
    // Check if task already exists and update it in this case
    for(uint8_t i = 0; i < _lasttask; i++)
    {
        // stop interrupts when accessing any element within the scheduler (also neccessary for if checks!), store old setting
        PAUSE_INTERRUPTS;

        // same function found
        if (SchedulingTable[i].func == func)
        {
            SchedulingTable[i].active    = true;
            SchedulingTable[i].running = false;
            SchedulingTable[i].period    = period;
            SchedulingTable[i].time        = _timebase + delay;
            
            // resume stored interrupt setting
            RESUME_INTERRUPTS;

            // find time for next task execution
            Scheduler_update_nexttime();

            // return success        
            return true;
        }

        // resume stored interrupt setting
        RESUME_INTERRUPTS;

    } // loop over scheduler slots
    
    // find free scheduler slot
    for (uint8_t i = 0; i < MAX_TASK_CNT; i++)
    {
        // stop interrupts when accessing any element within the scheduler (also neccessary for if checks!), store old setting
        PAUSE_INTERRUPTS;

        // free slot found    
        if (SchedulingTable[i].func == NULL)
        {
            // add task to scheduler table
            SchedulingTable[i].func        = func;
            SchedulingTable[i].active    = true;
            SchedulingTable[i].running = false;
            SchedulingTable[i].period    = period;
            SchedulingTable[i].time        = _timebase + delay;
            
            // update _lasttask
            if (i >= _lasttask)
                _lasttask = i + 1;

            // resume stored interrupt setting
            RESUME_INTERRUPTS;

            // find time for next task execution
            Scheduler_update_nexttime();

            // return success            
            return true;

        } // if free slot found

        // resume stored interrupt setting
        RESUME_INTERRUPTS;

    } // loop over scheduler slots

    // did not change anything, thus no scheduler_update_nexttime neccessary
    // no free slot found -> error
    return false;

} // Tasks_Add()



bool Tasks_Remove(Task func)
{
    // find function in scheduler table
    for (uint8_t i = 0; i < _lasttask; i++)
    {
        // stop interrupts when accessing any element within the scheduler (also neccessary for if checks!), store old setting
        PAUSE_INTERRUPTS;
    
        // function pointer found in list    
        if (SchedulingTable[i].func == func)
        {
            // remove task from scheduler table
            SchedulingTable[i].func        = NULL;
            SchedulingTable[i].active    = false;
            SchedulingTable[i].running = false;
            SchedulingTable[i].period    = 0;
            SchedulingTable[i].time        = 0;
            
            // update _lasttask
            if (i == (_lasttask - 1))
            {
                _lasttask--;
                while(_lasttask != 0)
                {
                    if(SchedulingTable[_lasttask - 1].func != NULL)
                    {
                        break;
                    }
                    _lasttask--;
                }
            }

            // resume stored interrupt setting
            RESUME_INTERRUPTS;

            // find time for next task execution
            Scheduler_update_nexttime();

            // return success
            return true;

        } // if function found

        // resume stored interrupt setting
        RESUME_INTERRUPTS;

    } // loop over scheduler slots

    // did not change anything, thus no scheduler_update_nexttime neccessary
    // function not in scheduler -> error
    return false;

} // Tasks_Remove()



bool Tasks_Delay(Task func, int16_t delay)
{
    // Check range of delay
    if (delay < 0)
        return false;
    
    // Workaround for 1.024ms timer period of Arduino MEGA
    #if defined(__AVR__)
        delay = (uint16_t)(((((int32_t)delay) * 250) + 128) >> 8); // delay = delay / 1.024 <-- with up/down rounding
    #endif
    
    // find function in scheduler table
    for (uint8_t i = 0; i < _lasttask; i++)
    {
        // stop interrupts, store old setting
        PAUSE_INTERRUPTS;
        
        // function pointer found in list
        if (SchedulingTable[i].func == func)
        {
            // if task is currently running, delay next call
            if (SchedulingTable[i].running == true)
                SchedulingTable[i].time = SchedulingTable[i].time - SchedulingTable[i].period;
        
            // set time to next execution
            SchedulingTable[i].time = _timebase + delay;

            // resume stored interrupt setting
            RESUME_INTERRUPTS;

            // find time for next task execution
            Scheduler_update_nexttime();

            // return success
            return true;

        } // if function found

        // resume stored interrupt setting
        RESUME_INTERRUPTS;

    } // loop over scheduler slots
    
    // did not change anything, thus no scheduler_update_nexttime neccessary
    // function not in scheduler -> error
    return false;
    
} // Tasks_Delay()



bool Tasks_SetState(Task func, bool state)
{
    // find function in scheduler table
    for (uint8_t i = 0; i < _lasttask; i++)
    {
        // stop interrupts when accessing any element within the scheduler (also neccessary for if checks!), store old setting
        PAUSE_INTERRUPTS;
            
        // function pointer found in list        
        if(SchedulingTable[i].func == func)
        {
            // set new function state            
            SchedulingTable[i].active = state;
            SchedulingTable[i].time = _timebase + SchedulingTable[i].period;

            // resume stored interrupt setting
            RESUME_INTERRUPTS;

            // find time for next task execution
            Scheduler_update_nexttime();

            // return success            
            return true;

        } // if function found
        
        // resume stored interrupt setting
        RESUME_INTERRUPTS;
	
    } // loop over scheduler slots
    
    // did not change anything, thus no scheduler_update_nexttime neccessary
    // function not in scheduler -> error
    return false;
    
} // Tasks_SetState()



void Tasks_Start(void)
{
    #if (TASKS_MEASURE_PIN)
        pinMode(9, OUTPUT);
    #endif

    // enable scheduler
    SchedulingActive = true;
    //_timebase = 0;        // unwanted delay after resume, see time-print() output! -> likely delete
    
    _nexttime = _timebase;  // Scheduler should perform a full check of all tasks after the next start
    
    // enable timer interrupt
    #if defined(__AVR__)
        TIMSK0 |= (1<<OCIE0A);                      // Enable OC0A Interrupt
    #elif defined(__SAM3X8E__)
        startTasksTimer(TC1, 0, TC3_IRQn, 1000);    // TC1 channel 0, the IRQ for that channel and the desired frequency
    #endif

    // find time for next task execution
    Scheduler_update_nexttime();
    
} // Tasks_Start()



void Tasks_Pause(void)
{
    // pause scheduler
    SchedulingActive = false;
    //_timebase = 0; // unwanted delay after resume, see time-print() output! -> likely delete 
    
    // disable timer interrupt
    #if defined(__AVR__)
        TIMSK0 &= ~(1<<OCIE0A); //Disable OC0A Interrupt
    #elif defined(__SAM3X8E__)
        NVIC_DisableIRQ(TC3_IRQn);
    #endif

} // Tasks_Pause()



/**************************************/
/******* start skip in doxygen ********/
/**************************************/
/// @cond INTERNAL

#if defined(__AVR__)
    ISR(TIMER0_COMPA_vect)  // Timer0 interrupt is called each 1.024ms before the OVL interrupt used for millis()
#elif defined(__SAM3X8E__)
    void TC3_Handler(void)
#else
    void Scheduler_dummy_handler(void) // avoid compiler error for unsupported boards
#endif
{
    uint8_t i;
    
    // measure speed via GPIO
    #if (TASKS_MEASURE_PIN)
        SET_PIN;
    #endif
    
    #if defined(__SAM3X8E__)
        TC1->TC_CHANNEL[0].TC_SR; //Read status register to delete status flags
    #endif
    
    // Skip if scheduling was stopped or is in the process of being stopped
    if (SchedulingActive == false) {
        #if (TASKS_MEASURE_PIN) // measure speed via GPIO
            CLEAR_PIN;
        #endif
        return;
    }
    
    // increase 1ms counter    
    _timebase++;

    // no task is pending -> return immediately
    if ((int16_t)(_nexttime - _timebase) > 0) {
        #if (TASKS_MEASURE_PIN) // measure speed via GPIO
            CLEAR_PIN;
        #endif
        return;
    }

    // loop over scheduler slots
    for(i = 0; i < _lasttask; i++)
    {
        // disable interrupts
        noInterrupts();

        // function pointer found in list, function is active and not running (arguments ordered to provide maximum speed
        if ((SchedulingTable[i].active == true) && (SchedulingTable[i].running == false) && (SchedulingTable[i].func != NULL))
        {
            // function period has passed
            if((int16_t)(SchedulingTable[i].time - _timebase) <= 0)
            {
                // execute task
                SchedulingTable[i].running = true;                                  // avoid dual function call
                SchedulingTable[i].time = _timebase + SchedulingTable[i].period;    // set time of next call
                
                // re-enable interrupts
                interrupts();

                // execute function
                SchedulingTable[i].func();
                
                // disable interrupts
                noInterrupts();
                
                // re-allow function call by scheduler                     
                SchedulingTable[i].running = false;
                
                // if function period is 0, remove it from scheduler after execution                     
                if(SchedulingTable[i].period == 0)
                {
                    SchedulingTable[i].func = NULL;
                }
                
                // re-enable interrupts
                interrupts();
            } // if function period has passed
        } // if function found
        
        // re-enable interrupts
        interrupts();
    
    } // loop over scheduler slots

    // find time for next task execution
    Scheduler_update_nexttime();
 
    // measure speed viaGPIO
    #if (TASKS_MEASURE_PIN)
        CLEAR_PIN;
    #endif
    
} // ISR()


/// @endcond
/************************************/
/******* end skip in doxygen ********/
/************************************/
