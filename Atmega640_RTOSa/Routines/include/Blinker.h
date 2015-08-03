/*
 * Blinker.h
 *
 * Created: 3/6/2015 4:41:00 PM
 *  Author: avasquez
 */ 


#ifndef BLINKER_H_
#define BLINKER_H_

// Define the port where the blinking LED is connected
#define blinkerPORT_DDR	DDRL 
#define blinkerPORT		PORTL
#define blinkerPIN		( 7 )

// Define the rate at which the blinker is to do so
extern uint16_t blinkerRate;

// Public function prototypes
void blinkerSetup( void );


#endif /* BLINKER_H_ */