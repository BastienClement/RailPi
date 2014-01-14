#define SYSCLK 22118400 // SYSCLK frequency in Hz
#define BAUDRATE 115200 // Baud rate of UART in bps

//-----------------------------------------------------------------------------
// Fixed length types
//-----------------------------------------------------------------------------

typedef unsigned char   uint8;
typedef signed char     int8;
typedef unsigned int    uint16;
typedef signed int      int16;
typedef unsigned long   uint32;
typedef signed long     int32;

//-----------------------------------------------------------------------------
// 16-bit SFR Definitions for ‘F02x
//-----------------------------------------------------------------------------

sfr16 DP = 0x82; // data pointer
sfr16 TMR3RL = 0x92; // Timer3 reload value
sfr16 TMR3 = 0x94; // Timer3 counter
sfr16 ADC0 = 0xbe; // ADC0 data
sfr16 ADC0GT = 0xc4; // ADC0 greater than window
sfr16 ADC0LT = 0xc6; // ADC0 less than window
sfr16 RCAP2 = 0xca; // Timer2 capture/reload
sfr16 T2 = 0xcc; // Timer2
sfr16 RCAP4 = 0xe4; // Timer4 capture/reload
sfr16 T4 = 0xf4; // Timer4
sfr16 DAC0 = 0xd2; // DAC0 data
sfr16 DAC1 = 0xd5; // DAC1 data

//-----------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------

typedef bit bool;
#define true  1
#define false 0

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------

void SYSCLK_Init(void);
void PORT_Init(void);
void UART0_Init(void);
