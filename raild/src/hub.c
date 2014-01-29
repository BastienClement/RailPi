#include "raild.h"

static bool  hub_is_ready = false;

static rbyte hub_sensors1 = 0x00;
static rbyte hub_sensors2 = 0x00;
static rbyte hub_sensors3 = 0x00;
static rbyte hub_switches = 0x00;

void set_hub_state(rhub_port port, rbyte value) {
	int sensors_base;
	rbyte *shadow;

	switch(port) {
		case RHUB_SENSORS1:
			sensors_base = 1;
			shadow = &hub_sensors1;
			break;

		case RHUB_SENSORS2:
			sensors_base = 9;
			shadow = &hub_sensors2;
			break;

		case RHUB_SENSORS3:
			sensors_base = 17;
			shadow = &hub_sensors3;
			break;

		case RHUB_SWITCHES:
			sensors_base = -1;
			shadow = &hub_switches;
			break;
	}

	if(hub_is_ready) {
		for(int i = 0; i < 8; i++) {
			int sensor_state = value & (1 << i);
			int shadow_state = *shadow & (1 << i);
			if(sensor_state != shadow_state) {
				if(sensors_base < 0) {
					lua_onswitchchanged(i, !!sensor_state);
				} else {
					lua_onsensorchanged(sensors_base + i, !!sensor_state);
				}
			}
		}
	}

	*shadow = value;
}

rbyte get_hub_state(rhub_port port) {
	switch(port) {
		case RHUB_SENSORS1: return hub_sensors1;
		case RHUB_SENSORS2: return hub_sensors2;
		case RHUB_SENSORS3: return hub_sensors3;
		case RHUB_SWITCHES: return hub_switches;
	}
}

void set_hub_readiness(bool r) {
	hub_is_ready = r;
	if(r) {
		lua_onready();
	} else {
		lua_ondisconnect();
	}
}

bool get_hub_readiness() {
	return hub_is_ready;
}
