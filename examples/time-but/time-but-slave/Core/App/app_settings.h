#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define TUP_PORT_BAUDRATE 115200u
#define TUP_HANDLE_INTERVAL_MS 10u
#define TUP_RX_BUF_SIZE_BYTES 64u
#define TUP_SYN_TIMEOUT_MS 1000u
#define TUP_DATA_TIMEOUT_MS 5000u
#define TUP_TRY_COUNT 5u
#define TUP_RETRY_PAUSE_MS 2000u
#define TUP_FLUSH_DURATION_MS 500u

#define BUT_HANDLE_INTERVAL_MS 10u
#define BUT_DEBOUNCE_PERIOD_MS 50u
