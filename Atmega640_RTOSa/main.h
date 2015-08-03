/*
 * main.h
 *
 * Created: 5/4/2015 4:19:46 PM
 *  Author: avasquez
 */ 


#ifndef MAIN_H_
#define MAIN_H_

#define NUM_TIMERS	9

extern uint8_t	timerArgsA[ NUM_TIMERS ];
extern uint8_t	timerArgsB[ NUM_TIMERS ];
extern uint8_t	uartList[ NUM_TIMERS ];
extern uint8_t inSystemBoxesList[ NUM_TIMERS ];
extern uint8_t  systemMemoryAvailable;
extern uint32_t sysTickCount;
extern uint32_t BoxDelayList[ NUM_TIMERS ];
extern uint8_t	boxSlats[ NUM_TIMERS ];
extern uint8_t	boxDivert[ NUM_TIMERS ];

#endif /* MAIN_H_ */