#include <c8051f020.h>
#include "sys.h"

//-----------------------------------------------------------------------------
// SYSCLK_Init
//-----------------------------------------------------------------------------
//
// This routine initializes the system clock to use an 22.1184MHz crystal
// as its clock source.
//
void SYSCLK_Init(void) {
	int i;         // delay counter
	OSCXCN = 0x67; // start external oscillator with
	               // 22.1184MHz crystal

	for(i=0; i < 256; i++); // wait for oscillator to start

	while(!(OSCXCN & 0x80)); // Wait for crystal osc. to settle
	OSCICN = 0x88;           // select external oscillator as SYSCLK
                             // source and enable missing clock
                             // detector
}

//-----------------------------------------------------------------------------
// PORT_Init
//-----------------------------------------------------------------------------
//
// Configure the Crossbar and GPIO ports
//
void PORT_Init(void) {
	XBR0 |= 0x04;    // Enable UART0
	XBR2 |= 0x40;    // Enable crossbar and weak pull-ups
	P0MDOUT |= 0x01; // enable TX0 as a push-pull output
	P3MDOUT |= 0xFF; // enable P1/P2-pins as push-pull output
	P2MDOUT |= 0xFF;
}

//-----------------------------------------------------------------------------
// UART0_Init
//-----------------------------------------------------------------------------
//
// Configure the UART0 using Timer1, for <baudrate> and 8-N-1.
//
void UART0_Init(void) {
	SCON0 = 0x50;                // SCON0: mode 1, 8-bit UART, enable RX
	TMOD = 0x20;                 // TMOD: timer 1, mode 2, 8-bit reload
	TH1 = -(SYSCLK/BAUDRATE/16); // set Timer1 reload value for baudrate
	TR1 = 1;                     // start Timer1
	CKCON |= 0x10;               // Timer1 uses SYSCLK as time base
	PCON |= 0x80;                // SMOD00 = 1 (disable baud rate
	                             // divide-by-two)
	TI0 = 1;                     // Indicate TX0 ready
}
