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