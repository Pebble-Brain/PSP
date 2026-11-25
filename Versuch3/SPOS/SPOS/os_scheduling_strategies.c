/*! \file

Scheduling strategies used by the Interrupt Service RoutineA from Timer 2 (in scheduler.c)
to determine which process may continue its execution next.

The file contains five strategies:
-even
-random
-round-robin
-inactive-aging
-run-to-completion
*/

#include "os_scheduling_strategies.h"

#include "defines.h"

#include <stdlib.h>

SchedulingInformation schedulingInfo;

/*!
 *  Reset the scheduling information for a specific strategy
 *  This is only relevant for RoundRobin and InactiveAging
 *  and is done when the strategy is changed through os_setSchedulingStrategy
 *
 *  \param strategy  The strategy to reset information for
 */
void os_resetSchedulingInformation(SchedulingStrategy strategy) {
	switch(strategy){
		case OS_SS_ROUND_ROBIN:
			schedulingInfo.timeSlice=os_getProcessSlot(os_getCurrentProc())->priority;
			break;
		case OS_SS_INACTIVE_AGING:
			for(int i =0; i<MAX_NUMBER_OF_PROCESSES; i++){
				schedulingInfo.age[i]=0;
			}
			break;
		default:
			break;
	}
    // This is optional in Versuch 2
}

/*!
 *  Reset the scheduling information for a specific process slot
 *  This is necessary when a new process is started to clear out any
 *  leftover data from a process that previously occupied that slot
 *
 *  \param id  The process slot to erase state for
 */
void os_resetProcessSchedulingInformation(ProcessID id){
	schedulingInfo.age[id]=0;
}

/*!
 *  This function implements the even strategy. Every process gets the same
 *  amount of processing time and is rescheduled after each scheduler call
 *  if there are other processes running other than the idle process.
 *  The idle process is executed if no other process is ready for execution
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed determined on the basis of the even strategy.
 */
ProcessID os_Scheduler_Even(const Process processes[], ProcessID current) {
	ProcessID nextfree = 0; // Default to idle if nothing else is found

	for(int i = 1; i<= MAX_NUMBER_OF_PROCESSES; i++){
		if(processes[(current+i)%MAX_NUMBER_OF_PROCESSES].state == OS_PS_READY && (current+i)%MAX_NUMBER_OF_PROCESSES != 0){
			nextfree = (current+i)%MAX_NUMBER_OF_PROCESSES;
			break;
		}
	}

	return nextfree;
}

/*!
 *  This function implements the random strategy. The next process is chosen based on
 *  the result of a pseudo random number generator.
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed determined on the basis of the random strategy.
 */
ProcessID os_Scheduler_Random(const Process processes[], ProcessID current) {
	// Record how many eligible processes are found
	uint8_t countFiltered = 0; 
	ProcessID filtered[MAX_NUMBER_OF_PROCESSES];

	// Search for eligible processes
	for(int i = 1; i< MAX_NUMBER_OF_PROCESSES; i++){
		if(processes[i].state==OS_PS_READY){
			filtered[countFiltered]=i;
			countFiltered++;
		}
	}
	if(countFiltered==0){
		return 0;
	}
	return filtered[rand()%countFiltered];
}

/*!
 *  This function implements the round-robin strategy. In this strategy, process priorities
 *  are considered when choosing the next process. A process stays active as long its time slice
 *  does not reach zero. This time slice is initialized with the priority of each specific process
 *  and decremented each time this function is called. If the time slice reaches zero, the even
 *  strategy is used to determine the next process to run.
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed determined on the basis of the round robin strategy.
 */
ProcessID os_Scheduler_RoundRobin(const Process processes[], ProcessID current) {
    if(schedulingInfo.timeSlice>0){
		schedulingInfo.timeSlice--;
		return current;
	}
	
	ProcessID candidate = os_Scheduler_Even(processes, current);
	if(candidate == 0) return 0;
	
	schedulingInfo.timeSlice=processes[current].priority;
	return candidate;
}

/*!
 *  This function realizes the inactive-aging strategy. In this strategy a process specific integer ("the age") is used to determine
 *  which process will be chosen. At first, the age of every waiting process is increased by its priority. After that the oldest
 *  process is chosen. If the oldest process is not distinct, the one with the highest priority is chosen. If this is not distinct
 *  as well, the one with the lower ProcessID is chosen. Before actually returning the ProcessID, the age of the process who
 *  is to be returned is reset to 0.
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed, determined based on the inactive-aging strategy.
 */
ProcessID os_Scheduler_InactiveAging(const Process processes[], ProcessID current) {
    ProcessID candidate = 0;
    uint16_t highestAge = 0;  // Use uint16_t to avoid overflow
    uint8_t highestPriority = 0;
    
    for(int i = 1; i < MAX_NUMBER_OF_PROCESSES; i++) {
        if(processes[i].state == OS_PS_READY) {
			// Age every process
            schedulingInfo.age[i] += processes[i].priority;
            
            // If any of the three conditions are true, select as a candidate
            if(schedulingInfo.age[i] > highestAge || // If it the oldest
               (schedulingInfo.age[i] == highestAge && processes[i].priority > highestPriority) || // If it is equally as old but lower priority
               (schedulingInfo.age[i] == highestAge && processes[i].priority == highestPriority && i < candidate)) { // If it has both equal age and priority but lower PID
                highestAge = schedulingInfo.age[i];
                highestPriority = processes[i].priority;
                candidate = i;
            }
        }
    }
    schedulingInfo.age[candidate] = 0; // Set age of next active process to 0
    return candidate;
}

/*!
 *  This function realizes the run-to-completion strategy.
 *  As long as the process that has run before is still ready, it is returned again.
 *  If  it is not ready, the even strategy is used to determine the process to be returned
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed, determined based on the run-to-completion strategy.
 */
ProcessID os_Scheduler_RunToCompletion(const Process processes[], ProcessID current) {
    if(processes[current].state==OS_PS_READY){
		return current;
	}
	return os_Scheduler_Even(processes, current);
}
