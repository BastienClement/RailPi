#include "raild.h"

/*
 * RailHub interface for Raild
 *
 * This file provides the RailHub ports cache.
 *
 * Since the communication with RailHub is asynchronous, everything
 * is kept in Raild memory and used when requested by Lua scripts.
 *
 * Every times something changes on one of the RailHub ports, Raild
 * is notified and this cache is updated.
 */

// Flag indicating if RailHub is connected and in Ready state
static bool  hub_is_ready = false;

// Flag indicating if the circuit is powered
static bool  power        = true;

// Cache for every sensors and switches
static rbyte hub_sensors1 = 0x00;
static rbyte hub_sensors2 = 0x00;
static rbyte hub_sensors3 = 0x00;
static rbyte hub_switches = 0x00;

static void sync_power() {
	set_gpio(hub_is_ready && power);
	uart_setpower(hub_is_ready && power);
}

//
// Update the Raild cache with fresh informations from RailHub
// This function also fires associated Lua events
//
void set_hub_state(rhub_port port, rbyte value) {
	// The number of the first sensor on this port
	// -1 in the case of switches
	int sensors_base;

	// Pointer to the appropriate cache value
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

	// Update the cache
	rbyte old_value = *shadow;
	*shadow = value;

	// Only send events when RailHub is ready
	// Prevents a lot of event flood during synchronization with RailHub
	if(hub_is_ready) {
		// Check every bit on this port
		for(int i = 0; i < 8; i++) {
			int sensor_state = value & (1 << i);
			int shadow_state = old_value & (1 << i);

			// If the bit received is different from the cached one
			if(sensor_state != shadow_state) {
				if(sensors_base < 0) {
					// -1 base is used to identify the switches-port
					lua_onswitchchanged(i, !!sensor_state);
				} else {
					// ... else it's a regular sensors-port
					lua_onsensorchanged(sensors_base + i, !!sensor_state);
				}
			}
		}
	}
}

//
// Returns the cached value for one of the RailHub port
//
rbyte get_hub_state(rhub_port port) {
	switch(port) {
		case RHUB_SENSORS1: return hub_sensors1;
		case RHUB_SENSORS2: return hub_sensors2;
		case RHUB_SENSORS3: return hub_sensors3;
		case RHUB_SWITCHES: return hub_switches;
	}
}

//
// Updates the ready state of RailHub and fires the corresponding event
//
void set_hub_readiness(bool r) {
	hub_is_ready = r;
	sync_power();
	if(r) {
		lua_onready();
	} else {
		lua_ondisconnect();
	}
}

//
// Returns the RailHub ready state
//
bool get_hub_readiness() {
	return hub_is_ready;
}

//
// Set the circuit power state
//
void set_power(bool p) {
	power = p;
	sync_power();
}

//
// Returns if the circuit is powered
//
bool get_power() {
	return power;
}
