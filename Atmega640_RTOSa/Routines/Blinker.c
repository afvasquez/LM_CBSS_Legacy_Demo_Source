/*
 * Blinker.c
 *
 * Created: 3/9/2015 1:36:53 PM
 *  Author: avasquez
 */ 
//////////////////////////////////////////////////////////////////////////
//	Blinker Debug Protocol
//	Author: Andres Vasquez
//	Date: 03/06/2015
//	Purpose: This Program blinks a given LED at half a second rate
//////////////////////////////////////////////////////////////////////////

// ##### INCLUDE the following
#include <stdlib.h>

// ***** Include the following
#include "FreeRTOS.h"
#include "task.h"

// ------ include the following locals
#include "Blinker.h"
#include "main.h"

// Define the global variables to be used
uint16_t blinkerRate = 250;

// Task function Prototype
void vBlinker( void );

//////////////////////////////////////////////////////////////////////////
//	Blinker Setup Function
void blinkerSetup( void ) {
	// Set the pin as defined in the header file
	blinkerPORT_DDR |= (1 << blinkerPIN);
	
	// Initialize the pin with the ON value
	blinkerPORT |= (1 << blinkerPIN);
	
	// Create the Task to be started after scheduler is asked to run
	xTaskCreate( vBlinker, "Blin", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	
}


void vBlinker( void ) {
	// Simply blink forever
	for (;;)
	{
		vTaskDelay( ( TickType_t ) blinkerRate );
		blinkerPORT ^= (1 << blinkerPIN);	
	}
}
