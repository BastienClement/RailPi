#include <c8051f020.h> // SFR declarations
#include <stdio.h>
#include "sys.h"
#include <hub_opcodes.h>

#define WATCHDOG WDTCN = 0xA5;

#define SEND_DATA(opcode, payload) \
	putchar(opcode); \
	putchar(payload);

void handle_input() reentrant {
	switch(_getkey()) {
		//
		// Sensors
		//
		case GET_SENSORS_1: SEND_DATA(SENSORS_1, P4); break;
		case GET_SENSORS_2: SEND_DATA(SENSORS_2, P5); break;
		case GET_SENSORS_3: SEND_DATA(SENSORS_3, P6); break;
		
		//
		// Switches manipulation
		//
		case GET_SWITCHES: SEND_DATA(SWITCHES, P2); break;
		case SET_SWITCHES:
			P2 = _getkey();
			break;
		
		//
		// Reset
		//
		case RESET:
			((void (code *)(void)) 0x0000)();
	}
}

void watch_sensors() {
	uint8 _shadow;
	uint8 switches = P2;
	uint8 sensors1 = P4;
	uint8 sensors2 = P5;
	uint8 sensors3 = P6;
	
	SEND_DATA(SWITCHES,  P2);
	SEND_DATA(SENSORS_1, P4);
	SEND_DATA(SENSORS_2, P5);
	SEND_DATA(SENSORS_3, P6);
	
	putchar(READY);
	
	#define DO_SHADOW(shadow, port, opcode, action) \
		if(_shadow = port, _shadow != shadow) { \
			shadow = _shadow; \
			action; \
			SEND_DATA(opcode, shadow); \
		}
		
	while(1) {
		WATCHDOG;
		
		DO_SHADOW(switches, P2, SWITCHES,);
		DO_SHADOW(sensors1, P4, SENSORS_1,);
		DO_SHADOW(sensors2, P5, SENSORS_2,);
		DO_SHADOW(sensors3, P6, SENSORS_3,);
		
		if(RI0 == 1) {
			handle_input();
		}
	}
}

//-----------------------------------------------------------------------------
// MAIN Routine
//-----------------------------------------------------------------------------
void main(void) {
	WDTCN = 0xFF;
	WATCHDOG;
	
	SYSCLK_Init(); // initialize oscillator
	PORT_Init();   // initialize crossbar and GPIO
	UART0_Init();  // initialize UART0
	
	putchar(HELLO);
	watch_sensors();
}
