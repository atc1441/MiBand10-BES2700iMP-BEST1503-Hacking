#pragma once
#include "smf_common.h"

typedef struct {
	uint32_t addr;
	uint32_t size;
	uint16_t width;
	uint16_t height;
	uint8_t format;
	int8_t refs;
	uint8_t wi;
	uint8_t ri;
}smf_display_t;
EXTERNC smf_display_t* smf_get_display();

EXTERNC void smf_display_start();
EXTERNC void smf_display_stop();
