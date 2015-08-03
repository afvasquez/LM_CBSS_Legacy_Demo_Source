

#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "croutine.h"

/* Demo file headers. */
#include "main.h"
#include "Blinker.h"

/* Priority definitions for most of the tasks in the demo application.  Some
tasks just use the idle priority. */
#define mainLED_TASK_PRIORITY			( tskIDLE_PRIORITY + 1 )
#define mainCOM_TEST_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define mainQUEUE_POLL_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define mainCHECK_TASK_PRIORITY			( tskIDLE_PRIORITY + 3 )

/* Baud rate used by the serial port tasks. */
#define mainCOM_TEST_BAUD_RATE			( ( unsigned long ) 38400 )

/* LED used by the serial port tasks.  This is toggled on each character Tx,
and mainCOM_TEST_LED + 1 is toggles on each character Rx. */
#define mainCOM_TEST_LED				( 4 )

/* LED that is toggled by the check task.  The check task periodically checks
that all the other tasks are operating without error.  If no errors are found
the LED is toggled.  If an error is found at any time the LED is never toggles
again. */
#define mainCHECK_TASK_LED				( 7 )

/* The period between executions of the check task. */
#define mainCHECK_PERIOD				( ( TickType_t ) 3000 / portTICK_PERIOD_MS  )

/* An address in the EEPROM used to count resets.  This is used to check that
the demo application is not unexpectedly resetting. */
#define mainRESET_COUNT_ADDRESS			( ( void * ) 0x50 )

/* The number of coroutines to create. */
#define mainNUM_FLASH_COROUTINES		( 3 )

#define F_CPU 16000000UL
#define BAUDRATE 4800        //The baudrate that we want to use
#define BAUD_PRESCALLER (((F_CPU / (BAUDRATE * 16UL))) - 1)    //The formula that does all the required maths

#define SLAT_LENGTH 159		// mm
#define CONV_SPEED	1.27	// mm/msec -> 250ft/min
//#define CONV_SPEED	1.07	// mm/msec -> 210ft/min

// mm/msec -> 210ft/min
//#define DIVERT_ONE_DELAY	( ( uint32_t ) 2919 )
//#define DIVERT_TWO_DELAY	( ( uint32_t ) 2919 )

// mm/msec -> 250ft/min
#define DIVERT_ONE_DELAY	( ( uint32_t ) 2450 )
#define DIVERT_TWO_DELAY	( ( uint32_t ) 2450 )

uint8_t	boxDivert[ NUM_TIMERS ] = { 0 };
uint8_t	boxSlats[ NUM_TIMERS ] = { 0 };
uint8_t inSystemBoxesList[ NUM_TIMERS ] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
uint8_t systemMemory[ NUM_TIMERS ] = { 0 };
uint8_t systemMemoryAvailable = NUM_TIMERS;
uint8_t divertTracker = 0;
uint8_t inSystemBoxes = 0;
uint8_t flagEOB = 0;
uint8_t i;
uint32_t sysTickCount = 0;
uint32_t sysBoxLeadEdge;
uint32_t sysBoxTimeLength;
uint32_t BoxDelayList[ NUM_TIMERS ] = { 0 };
float flHolder; 
void vSystemSetup( void );
void vTimerCallback( TimerHandle_t pxTimer );

/*
 * Called on boot to increment a count stored in the EEPROM.  This is used to
 * ensure the CPU does not reset unexpectedly.
 */
static void prvIncrementResetCount( void );

/*
 * The idle hook is used to scheduler co-routines.
 */
void vApplicationIdleHook( void );

// The BoxEntry is used to determine the size of boxes
void vBoxEntry( void ) ;
void USART_SendByte(uint8_t u8Data);


/*-----------------------------------------------------------*/

int main( void )
{
	// Run the System Setup Before doing anything!!
	vSystemSetup();
	
	// Perform Blinker setup
	blinkerSetup();
	
	/* In this port, to use preemptive scheduler define configUSE_PREEMPTION
	as 1 in portmacro.h.  To use the cooperative scheduler define
	configUSE_PREEMPTION as 0. */
	vTaskStartScheduler();
	
	return 0;
}
/*-----------------------------------------------------------*/

static void prvIncrementResetCount( void )
{
//unsigned char ucCount;
//
	//eeprom_read_block( &ucCount, mainRESET_COUNT_ADDRESS, sizeof( ucCount ) );
	//ucCount++;
	//eeprom_write_byte( mainRESET_COUNT_ADDRESS, ucCount );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	// Increment the distance variable
	//vCoRoutineSchedule();
}

void vSystemSetup( void ) 
{
	DDRB &= ~( 1 << PB4 );	// Set pin 4 of Port B as input
	
	// Set the changes for the Pin Change Interrupt
	PCICR |= (1 << PCIE0);    // set PCIE0 to enable PCMSK0 scan
	PCMSK0 |= (1 << PCINT4);  // set PCINT0 to trigger an interrupt on state change
	
	DDRD |= ( 1 << PD2 );
	DDRD |= ( 1 << PD7 );
	DDRL |= ( 1 << PL6 );
	
	DDRE |= 0b00000010;
	
	PORTD &= ~( 1 << PD7 );
	
	//////////////////////////////////////////////////////////////////////////
	// SERIAL Communication Setup
	//////////////////////////////////////////////////////////////////////////
	UBRR0H = (uint8_t)(BAUD_PRESCALLER>>8);
	UBRR0L = (uint8_t)(BAUD_PRESCALLER);
	
	UCSR0C = 0b00110110;
	UCSR0B = 0x08;	// Tx Enabled.
	/////////////////////////// END SERIAL COMMUNICATION SETUP ////////////////
	
	xTaskCreate( vBoxEntry, "Div",configMINIMAL_STACK_SIZE,NULL,3, NULL );
}

int8_t dirSwitch = 0x00;
void vBoxEntry( void ) 
{
	for (;;) {
		// Loop every 5ms
		PORTL ^= ( 1 << PL6 );
		if ( systemMemoryAvailable < NUM_TIMERS ) {
			for ( i=0;i<NUM_TIMERS;i++ ) {
				if ( inSystemBoxesList[i] == 16 ) {	// For the boxes in the busy state
					if ( sysTickCount > BoxDelayList[i] ) { // Delay has passed
						
							
						PORTD |= ( 1 << PD7 );
							
						// Wait until last byte has been transmitted
						while( ( UCSR0A & ( 1<< UDRE0 ) ) == 0);
						// Transmit data
						UDR0 = 0xF7;
							
						// Wait until last byte has been transmitted
						while( ( UCSR0A & ( 1<< UDRE0 ) ) == 0);
						// Transmit data
						UDR0 = 0xF7;
							
							
						// Wait until last byte has been transmitted
						while( ( UCSR0A & ( 1<< UDRE0 ) ) == 0);
						// Transmit data
						if ( !dirSwitch ) {
							UDR0 = 0x01;
							dirSwitch = 0xFF;
						} else {
							UDR0 = 0x01 + 0x80;
							dirSwitch = 0x00;
						}
							
						// Wait until last byte has been transmitted
						while( ( UCSR0A & ( 1<< UDRE0 ) ) == 0);
						// Transmit data
						UDR0 = boxSlats[i];
						//UDR0 = systemMemoryAvailable;
						
						while( ( UCSR0A & ( 1<< UDRE0 ) ) == 0);
						vTaskDelay(5);
							
						PORTD &= ~( 1 << PD7 );
						
						
						inSystemBoxesList[i] = i;
						systemMemoryAvailable++;
					}
				}
			}
		}
		
		//vTaskDelay(5);	// Come back and check in 5ms
		vTaskDelay(5);
	}
}

//////////////////////////////////////////////////////////////////////////
// Byte Transmission Function
//////////////////////////////////////////////////////////////////////////
void USART_SendByte(uint8_t u8Data) {
	// Wait until last byte has been transmitted
	while( ( UCSR0A & ( 1<< UDRE0 ) ) == 0);
	// Transmit data
	UDR0 = u8Data;
}

//////////////////////////////////////////////////////////////////////////
//	PCINT Interrupt Service Routine
//////////////////////////////////////////////////////////////////////////
volatile uint8_t portchistory = 0x00;     // default is LOW
ISR (PCINT0_vect)
{
	uint8_t changedbits;
	uint8_t holder;
	uint8_t j, k;
	
	holder = PINB;
	changedbits = holder & 0b00010000;
	changedbits ^= portchistory;
	portchistory = holder;
	portchistory &= 0b00010000;
	
	if(changedbits & (1 << PINB4)) {
		/* PCINT13 changed */
		if ( systemMemoryAvailable > 0 ) {
			if( ( portchistory & (1 << PINB4 ) ) ) {   /// ####### CONTROLS: Divert 90-Degrees
				// Low to High Transition
				// Start to transmit the correct protocol until timed out
			
			//// Wait until last byte has been transmitted
			//while( ( UCSR0A & ( 1<< UDRE0 ) ) == 0);
			//// Transmit data
			//UDR0 = 0xDE;
			
				// Start to keep track of the time it takes the box to clear the PE
				sysBoxLeadEdge = sysTickCount;
			} else {
			
				// Calculate the length of the box and determine the right amount
				// of slats that it would need to divert in the correct manner
				sysBoxLeadEdge = sysTickCount - sysBoxLeadEdge;
				sysBoxTimeLength = sysBoxLeadEdge;
				flHolder = ( float ) ( (( float ) CONV_SPEED ) * sysBoxLeadEdge );
				sysBoxLeadEdge = ( uint32_t ) flHolder;
				sysBoxLeadEdge = ( sysBoxLeadEdge / SLAT_LENGTH ) + 1;
				
				if ( sysBoxLeadEdge == 1 ) sysBoxLeadEdge = 2;
				
				
				// Fit data into entry
				for ( k=0;k<NUM_TIMERS;k++ ) {
					if ( inSystemBoxesList[k] != 16 ) {
						systemMemory[k] = inSystemBoxesList[k];
						inSystemBoxesList[k] = 16;	// Spot has been taken
						j = k;
						k = NUM_TIMERS + 1;	// Exit
					}
				}
				
				boxSlats[j] = sysBoxLeadEdge;
				
				
				if ( divertTracker == 0 || divertTracker == 1 ) {
					BoxDelayList[j] = sysTickCount + DIVERT_ONE_DELAY - sysBoxTimeLength;
					boxDivert[j] = 1;
				} else if ( divertTracker == 2 || divertTracker == 3 ) {
					BoxDelayList[j] = sysTickCount + DIVERT_ONE_DELAY - sysBoxTimeLength;
					boxDivert[j] = 2;
				}
				
				//if ( PINC & _BV(PINC7) ) {
					//
				//} else {
					//if ( divertTracker == 0 || divertTracker == 1 ) {
						//BoxDelayList[j] = sysTickCount + DIVERT_ONE_DELAY - sysBoxTimeLength;
						//boxDivert[j] = 1;
					//} else if ( divertTracker == 2 || divertTracker == 3 ) {
						//BoxDelayList[j] = sysTickCount + DIVERT_TWO_DELAY - sysBoxTimeLength;
						//boxDivert[j] = 2;
					//}
				//}
				
				if ( divertTracker == 3) {
					divertTracker = 0;
				} else {
					divertTracker++;
				}
				
				systemMemoryAvailable--;	// Increment the count of boxes inside the system
			}
		}
	}
}

void vApplicationTickHook( void ) {
	sysTickCount++;
}