#include "raild.h"

/*
 * Logger utilities
 */

#define BUFFER_LENGTH 512

static char buffer[BUFFER_LENGTH];
static char prefix_buffer[BUFFER_LENGTH];
static char time_buffer[64];

//
// Generates the time tag for output
//
static char* time_tag() {
	time_t now = time(0);
	struct tm *now_struct = localtime(&now);
	snprintf(time_buffer, 64, "%02i:%02i:%02i", now_struct->tm_hour, now_struct->tm_min, now_struct->tm_sec);
	return time_buffer;
}

//
// Outputs a line
//
void logger(const char *prefix, const char* message) {
	snprintf(buffer, BUFFER_LENGTH, "%s  %-5s  %s", time_tag(), prefix, message);
	printf("%s\n", buffer);
}

//
// Output an error
//
void logger_error(const char* message) {
	snprintf(buffer, BUFFER_LENGTH, "%s  ERROR  %s", time_tag(), message);
	printf("%s\n", buffer);
}

//
// Light output
//
void logger_light(const char* message) {
	snprintf(buffer, BUFFER_LENGTH, "%15s  %s", "", message);
	printf("%s\n", buffer);
}

//
// Concat two strings
//
char* logger_prefix(const char* prefix, const char* message) {
	snprintf(prefix_buffer, BUFFER_LENGTH, "%s %s", prefix, message);
	return prefix_buffer;
}
