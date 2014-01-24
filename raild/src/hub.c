#include "raild.h"

static bool  hub_is_ready = false;

static rbyte hub_sensors1 = 0x00;
static rbyte hub_sensors2 = 0x00;
static rbyte hub_sensors3 = 0x00;
static rbyte hub_switches = 0x00;

void set_hub_state(rhub_port port, rbyte value) {
	switch(port) {
		case RHUB_SENSORS1:
			hub_sensors1 = value;
			break;

		case RHUB_SENSORS2:
			hub_sensors2 = value;
			break;

		case RHUB_SENSORS3:
			hub_sensors3 = value;
			break;

		case RHUB_SWITCHES:
			hub_switches = value;
			break;
	}

	printf("%i set to 0x%02x\n", port, value);
}

void set_hub_readiness(bool r) {
	hub_is_ready = r;
}

bool get_hub_readiness() {
	return hub_is_ready;
}
