#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

uint32_t getTimeElapsed(uint32_t fromTime_ms, uint32_t toTime_ms);
bool isBigEndian();
