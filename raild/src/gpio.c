#include "raild.h"

/*
 * Interface to the GPIO 17 pin used as FAILURE command
 */

void setup_gpio() {
	system("echo out > /sys/class/gpio/gpio17/direction");
	system("echo 0 > /sys/class/gpio/gpio17/value");
}

void set_gpio(bool state) {
	if(state) {
		system("echo 1 > /sys/class/gpio/gpio17/value");
	} else {
		system("echo 0 > /sys/class/gpio/gpio17/value");
	}
}
