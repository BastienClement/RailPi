#include <c8051f020.h> // SFR declarations
#include <stdio.h>
#include "sys.h"
#include <hub_opcodes.h>

uint16 keepalive = 0xFFFF;
uint8  keepalive_timeout = 0;

#define WATCHDOG WDTCN = 0xA5;

#define SEND_DATA(opcode, payload) \
	WATCHDOG; \
	putchar_raw(opcode); \
	putchar_raw(payload);

char putchar_raw(char c)  {
	while (!TI0);
	TI0 = 0;
	return (SBUF0 = c);
}

sbit led = P1^6;

void handle_input() reentrant {
	switch(_getkey()) {
		//
		// Sensors
		//
		case GET_SENSORS_1: SEND_DATA(SENSORS_1, ~P4); break;
		case GET_SENSORS_2: SEND_DATA(SENSORS_2, ~P5); break;
		case GET_SENSORS_3: SEND_DATA(SENSORS_3, ~P6); break;
		
		//
		// Switches manipulation
		//
		case GET_SWITCHES: SEND_DATA(SWITCHES, P2); break;
		
		case SET_SWITCHES:
			P2 = _getkey();
			break;
		
		case SET_SWITCH_ON:
			P2 |= (1 << _getkey());
			break;
		
		case SET_SWITCH_OFF:
			P2 &= ~(1 << _getkey());
			break;
	
		//
		// Power control
		//
		case POWER_ON:
			led = 1;
			break;
		
		case POWER_OFF:
			led = 0;
			break;
		
		//
		// Keep-Alive
		//
		case KEEP_ALIVE:
			keepalive_timeout = 0;
			break;
		
		//
		// Reset
		//
		case RESET:
			((void (code *)(void)) 0x0000)();
	}
}

void dead() {
	led = 0;
	
	while(1) {
		WATCHDOG;
		if(RI0 == 1) {
			handle_input();
		}
	}
}

void watch_sensors() {
	uint8 _shadow;
	uint8 switches = P2;
	uint8 sensors1 = ~P4;
	uint8 sensors2 = ~P5;
	uint8 sensors3 = ~P6;
	
	SEND_DATA(SENSORS_1, ~P4);
	SEND_DATA(SENSORS_2, ~P5);
	SEND_DATA(SENSORS_3, ~P6);
	
	putchar_raw(READY);
	
	#define DO_SHADOW(shadow, port, opcode, action) \
		if(_shadow = (port), _shadow != shadow) { \
			shadow = _shadow; \
			action; \
			WATCHDOG; \
			SEND_DATA(opcode, shadow); \
		}
		
	while(1) {
		WATCHDOG;
		
		DO_SHADOW(switches, P2, SWITCHES,);
		DO_SHADOW(sensors1, ~P4, SENSORS_1,);
		DO_SHADOW(sensors2, ~P5, SENSORS_2,);
		DO_SHADOW(sensors3, ~P6, SENSORS_3,);
		
		if(RI0 == 1) {
			handle_input();
		}
		
		if(--keepalive == 0) {
			if(++keepalive_timeout > 2) {
				dead();
			} else {
				putchar_raw(KEEP_ALIVE);
				keepalive = 0xFFFF;
			}
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
	
	led = 1;
	P2  = 0x00;
	
	putchar_raw(HELLO);
	watch_sensors();
}
