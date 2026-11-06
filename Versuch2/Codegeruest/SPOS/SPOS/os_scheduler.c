/*! \file
 *  \brief Scheduling module for the OS.
 *
 * Contains everything needed to realise the scheduling between multiple processes.
 * Also contains functions to start the execution of programs.
 *
 *  \author Lehrstuhl Informatik 11 - RWTH Aachen
 */

#include "os_scheduler.h"

#include "lcd.h"
#include "os_core.h"
#include "os_input.h"
#include "os_scheduling_strategies.h"
#include "os_taskman.h"
#include "util.h"

#include <avr/interrupt.h>
#include <stdbool.h>

//----------------------------------------------------------------------------
// Private Types
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Globals
//----------------------------------------------------------------------------

//! Array of states for every possible process
Process os_processes[MAX_NUMBER_OF_PROCESSES];

//! Index of process that is currently executed (default: idle)
ProcessID currentProc;

//----------------------------------------------------------------------------
// Private variables
//----------------------------------------------------------------------------

//! Currently active scheduling strategy
 SchedulingStrategy currentStrat;

//! Count of currently nested critical sections
uint8_t criticalSectionCount;

//----------------------------------------------------------------------------
// Private function declarations
//----------------------------------------------------------------------------

//! ISR for timer compare match (scheduler)
ISR(TIMER2_COMPA_vect)
__attribute__((naked));

//----------------------------------------------------------------------------
// Function definitions
//----------------------------------------------------------------------------

/*!
 *  Timer interrupt that implements our scheduler. Execution of the running
 *  process is suspended and the context saved to the stack. Then the periphery
 *  is scanned for any input events. If everything is in order, the next process
 *  for execution is derived with an exchangeable strategy. Finally the
 *  scheduler restores the next process for execution and releases control over
 *  the processor to that process.
 */
ISR(TIMER2_COMPA_vect) {
	saveContext();
	os_processes[currentProc].sp.as_int = (SP-35);
	SP = BOTTOM_OF_ISR_STACK;
	if(os_getInput()==0b1001){
		os_waitForNoInput();
		os_taskManMain();
	}
	os_processes[currentProc].state=OS_PS_READY;
	switch (currentStrat){
		case OS_SS_RANDOM:
			currentProc = os_Scheduler_Random(os_processes, currentProc);
			break;
		case OS_SS_EVEN:
			currentProc = os_Scheduler_Even(os_processes, currentProc);
			break;
		default:
			currentProc = os_Scheduler_Even(os_processes, currentProc);
			break;
	}
	os_processes[currentProc].state = OS_PS_RUNNING;
	SP = os_processes[currentProc].sp.as_int;
	restoreContext();
}

/*!
 *  This is the idle program. The idle process owns all the memory
 *  and processor time no other process wants to have.
 */
void idle(void){
	while(1){
		lcd_writeChar('.');
		delayMs(DEFAULT_OUTPUT_DELAY);
	}
}

/*!
 *  This function is used to register the given program for execution.
 *  A stack will be provided if the process limit has not yet been reached.
 *  This function is multitasking safe. That means that programs can repost
 *  themselves, simulating TinyOS 2 scheduling (just kick off interrupts ;) ).
 *
 *  \param program  The function of the program to start.
 *  \param priority A priority ranging 0..255 for the new process:
 *                   - 0 means least favourable
 *                   - 255 means most favourable
 *                  Note that the priority may be ignored by certain scheduling
 *                  strategies.
 *  \return The index of the new process or INVALID_PROCESS as specified in
 *          defines.h on failure
 */
ProcessID os_exec(Program *program, Priority priority) {
	os_enterCriticalSection();
	//find free spot on process array
	int freePID = -1;
	for(int i = 0; i<MAX_NUMBER_OF_PROCESSES; i++){
		if(os_processes[i].state == OS_PS_UNUSED){
			freePID = i;
			break;
		}
	}
	if(freePID == -1){
		os_leaveCriticalSection();
		return INVALID_PROCESS;
	}
	
	//check pointer
	if(program == NULL){
		os_leaveCriticalSection();
		return INVALID_PROCESS;
	}
	
	//add correct values to the new process at PID i
	os_processes[freePID].state = OS_PS_READY;
	os_processes[freePID].program= program;
	os_processes[freePID].priority = priority;
	
	//init stack
	uint8_t *cur = (uint8_t *)PROCESS_STACK_BOTTOM(freePID);
	*cur=(uintptr_t)program;
	*(cur-1)=(uint8_t)((uintptr_t)program>>8);
	
	for(int i =2; i<35; i++){
		*(cur-i)=0;
	}
	
	os_processes[freePID].sp.as_ptr=cur-35;
	os_processes[freePID].checksum = os_getStackChecksum(freePID);
	//return pid
	os_leaveCriticalSection();
	return freePID;
}

/*!
 *  If all processes have been registered for execution, the OS calls this
 *  function to start the idle program and the concurrent execution of the
 *  applications.
 */
void os_startScheduler(void) {
	currentProc = 0;
	os_processes[0].state = OS_PS_RUNNING;
	SP = os_processes[0].sp.as_int;
	restoreContext();
}

/*!
 *  In order for the Scheduler to work properly, it must have the chance to
 *  initialize its internal data-structures and register.
 */
void os_initScheduler(void){
	for(int i = 0; i<MAX_NUMBER_OF_PROCESSES; i++){
		os_processes[i].state = OS_PS_UNUSED;
	}
	//init leerlauf process (unklar welche priority)
	os_exec(idle, DEFAULT_PRIORITY);
	
	struct program_linked_list_node *cur = autostart_head;
	while(cur != NULL){
		if(cur->program == idle){
			cur = cur->next;
			continue;
		}
		os_exec(cur->program, DEFAULT_PRIORITY);
		cur = cur->next;
	}
}

/*!
 *  A simple getter for the slot of a specific process.
 *
 *  \param pid The processID of the process to be handled
 *  \return A pointer to the memory of the process at position pid in the os_processes array.
 */
Process *os_getProcessSlot(ProcessID pid) {
    return os_processes + pid;
}

/*!
 *  A simple getter to retrieve the currently active process.
 *
 *  \return The process id of the currently active process.
 */
ProcessID os_getCurrentProc(void) {
	return currentProc;
}

/*!
 *  Sets the current scheduling strategy.
 *
 *  \param strategy The strategy that will be used after the function finishes.
 */
void os_setSchedulingStrategy(SchedulingStrategy strategy){
	currentStrat = strategy;
}

/*!
 *  This is a getter for retrieving the current scheduling strategy.
 *
 *  \return The current scheduling strategy.
 */
SchedulingStrategy os_getSchedulingStrategy(void) {
	return currentStrat;
}

/*!
 *  Enters a critical code section by disabling the scheduler if needed.
 *  This function stores the nesting depth of critical sections of the current
 *  process (e.g. if a function with a critical section is called from another
 *  critical section) to ensure correct behaviour when leaving the section.
 *  This function supports up to 255 nested critical sections.
 */
void os_enterCriticalSection(void) {
	uint8_t temp = SREG;
	SREG &= 0b01111111;
	criticalSectionCount++;
	TIMSK2 &= ~(1<<OCIE2A);
	SREG = temp;
}

/*!
 *  Leaves a critical code section by enabling the scheduler if needed.
 *  This function utilizes the nesting depth of critical sections
 *  stored by os_enterCriticalSection to check if the scheduler
 *  has to be reactivated.
 */
void os_leaveCriticalSection(void){
	uint8_t temp = SREG;
	SREG &= 0b01111111;
	criticalSectionCount--;
	if(criticalSectionCount==0)
		TIMSK2 |= (1<<OCIE2A);
	SREG = temp;
}

/*!
 *  Calculates the checksum of the stack for a certain process.
 *
 *  \param pid The ID of the process for which the stack's checksum has to be calculated.
 *  \return The checksum of the pid'th stack.
 */
StackChecksum os_getStackChecksum(ProcessID pid) {
	uint8_t *cur = (uint8_t*)PROCESS_STACK_BOTTOM(pid);
	uint8_t res = 0;
	for(int i =0; i<STACK_SIZE_PROC; i++){
		res ^= cur[-i];
	}
	return res;
}
