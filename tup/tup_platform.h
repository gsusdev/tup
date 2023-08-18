#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

void tup_link_transmit(const void* buf_p, size_t size_bytes, uintptr_t callbackValue);

uint32_t tup_getCurrentTime_ms();

void tup_signal_fire(uintptr_t signal);
bool tup_signal_wait(uintptr_t signal, uint32_t timeout_ms);
