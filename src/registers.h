#pragma once
#include <stdint.h>

#define REGISTER_COUNT 5

#define A 0
#define B 1
#define IP 2
#define rSTATUS 3
#define rFLAGS 4

#define FLAG_ZERO 1
#define FLAG_CARRY 2
#define FLAG_OVERFLOW 4

static uint64_t REGISTERS[REGISTER_COUNT];
