#include "raild.h"

static int   uart0_filestream = -1;
static rbyte buffer[256];

typedef enum {
	UART_PROCESS_DISPATCH,
	UART_PROCESS_SENSORS1,
	UART_PROCESS_SENSORS2,
	UART_PROCESS_SENSORS3,
	UART_PROCESS_SWITCHES
} uart_process_state;

static uart_process_state state = UART_PROCESS_DISPATCH;

int setup_uart() {
	printf("[UART]\t Init UART channel\n");

	//OPEN THE UART
	//The flags (defined in fcntl.h):
	//	Access modes (use 1 of these):
	//		O_RDONLY - Open for reading only.
	//		O_RDWR - Open for reading and writing.
	//		O_WRONLY - Open for writing only.
	//
	//	O_NDELAY / O_NONBLOCK (same function) - Enables nonblocking mode. When set read requests on the file can return immediately with a failure status
	//											if there is no input immediately available (instead of blocking). Likewise, write requests can also return
	//											immediately with a failure status if the output can't be written immediately.
	//
	//	O_NOCTTY - When set and path identifies a terminal device, open() shall not cause the terminal device to become the controlling terminal for the process.
	uart0_filestream = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY | O_NONBLOCK);		//Open in non blocking read/write mode
	if(uart0_filestream == -1) {
		//ERROR - CAN'T OPEN SERIAL PORT
		printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
		exit(1);
	}

	//CONFIGURE THE UART
	//The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
	//	Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
	//	CSIZE:- CS5, CS6, CS7, CS8
	//	CLOCAL - Ignore modem status lines
	//	CREAD - Enable receiver
	//	IGNPAR = Ignore characters with parity errors
	//	ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comms!)
	//	PARENB - Parity enable
	//	PARODD - Odd parity (else even)
	struct termios options;
	tcgetattr(uart0_filestream, &options);
	options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;		//<Set baud rate
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	tcflush(uart0_filestream, TCIFLUSH);
	tcsetattr(uart0_filestream, TCSANOW, &options);

	return uart0_filestream;

	/*

	//----- CHECK FOR ANY RX BYTES -----
	while(uart0_filestream != -1)
	{
		// Read up to 255 characters from the port if they are there
		unsigned char rx_buffer[256];
		int rx_length = read(uart0_filestream, (void*)rx_buffer, 255);		//Filestream, buffer to store in, number of bytes to read (max)
		if (rx_length < 0)
		{
			//An error occured (will occur if there are no bytes)
		}
		else if (rx_length == 0)
		{
			//No data waiting
		}
		else
		{
			//Bytes received
			rx_buffer[rx_length] = '\0';
			printf("%i bytes read : %s\n", rx_length, rx_buffer);
		}
	}



	return 0;
	*/
}

static void uart_process(rbyte *buffer, int len) {
	for(int i = 0; i < len; i++) {
		rbyte c = buffer[i];
		switch(state) {
			case UART_PROCESS_DISPATCH:
				switch(c) {
					case HELLO:
						printf("[UART]\t Got HELLO from RailHub!\n");
						break;

					case SENSORS_1: state = UART_PROCESS_SENSORS1; break;
					case SENSORS_2: state = UART_PROCESS_SENSORS2; break;
					case SENSORS_3: state = UART_PROCESS_SENSORS3; break;
					case SWITCHES:  state = UART_PROCESS_SWITCHES; break;

					default:
						printf("[UART]\t Unknown opcode from RailHub: 0x%02x\n", (unsigned char) c);
				}
				break;

			case UART_PROCESS_SENSORS1:
				set_hub_state(RHUB_SENSORS1, c);
				state = UART_PROCESS_DISPATCH;
				break;

			case UART_PROCESS_SENSORS2:
				set_hub_state(RHUB_SENSORS2, c);
				state = UART_PROCESS_DISPATCH;
				break;

			case UART_PROCESS_SENSORS3:
				set_hub_state(RHUB_SENSORS3, c);
				state = UART_PROCESS_DISPATCH;
				break;

			case UART_PROCESS_SWITCHES:
				set_hub_state(RHUB_SWITCHES, c);
				state = UART_PROCESS_DISPATCH;
				break;

			default:
				printf("[UART]\t Input processor is in an unknown state. Aborting.\n");
				exit(1);
		}
	}
}

void uart_handle_event(int fd) {
	int len = read(uart0_filestream, (void *) buffer, 256);

	if(len == EAGAIN || len == 0) {
		return;
	} else if(len < 0) {
		perror("(uart) read");
		exit(1);
	} else {
		uart_process(buffer, len);
	}
}
