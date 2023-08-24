#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum
{
	  butFilter_event_down
	, butFilter_event_press
	, butFilter_event_up
} butFilter_event_t;

typedef bool (*butFilter_inputStateGetter_t)(bool* isPressed_out_p);
typedef void (*butFilter_eventHandler_t)(butFilter_event_t event, uintptr_t callbackValue);

typedef enum
{
	  butFilter_error_ok
	, butFilter_error_invalidDescr
	, butFilter_error_invalidEvent
	, butFilter_error_invalidInit
	, butFilter_error_hardware
} butFilter_error_t;

typedef struct
{
	uint32_t debouncePeriod_ms;
	butFilter_inputStateGetter_t stateGetter;
	butFilter_eventHandler_t eventHandler;
	uintptr_t userCallbackValue;
} butFilter_initStruct;

typedef struct
{
	uint8_t privateData[28];
} butFilter_t;

butFilter_error_t butFilter_init(butFilter_t* instance_p, const butFilter_initStruct* initStruct_p);
butFilter_error_t butFilter_handle(butFilter_t* instance_p);
butFilter_error_t butFilter_hasEvent(const butFilter_t* instance_p, butFilter_event_t event, bool* result_out_p);
butFilter_error_t butFilter_isDown(const butFilter_t* instance_p, bool* result_out_p);
