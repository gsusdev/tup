#include "app_butFilter.h"

#include <string.h>
#include <assert.h>

#include "app_hal.h"
#include "app_utils.h"

typedef struct descriptor_t
{
	uint32_t debouncePeriod_ms;
	butFilter_inputStateGetter_t stateGetter;
	butFilter_eventHandler_t eventHandler;
	uintptr_t userCallbackValue;
	uint32_t unstableIsDownBegin_ms;
	bool isDown_unstable;
	bool isDown_stable;
	bool keyDownEvent;
	bool keyPressEvent;
	bool keyUpEvent;
} descriptor_t;

static_assert(sizeof(descriptor_t) <= sizeof(butFilter_t), "Adjust the \"privateData\" field size in the \"butFilter_t\" struct");

#define _DESCR(d, qual)                                 \
    assert(d != NULL);                                  \
    qual descriptor_t* descr_p = (qual descriptor_t*)d; \
    if (!checkDescr(descr_p))                           \
    {                                                   \
        return butFilter_error_invalidDescr;            \
    }

#define DESCR(d)  _DESCR(d, )
#define CDESCR(d) _DESCR(d, const)

static bool checkDescr(const descriptor_t* descr_p);

butFilter_error_t butFilter_init(butFilter_t* instance_p, const butFilter_initStruct* initStruct_p)
{
	assert(instance_p != NULL);
	assert(initStruct_p != NULL);

	bool ok = true;
	ok &= initStruct_p->stateGetter != NULL;
	if (!ok)
	{
		return butFilter_error_invalidInit;
	}


	descriptor_t* descr_p = (descriptor_t*)instance_p;

	memset(descr_p, 0, sizeof(*descr_p));

	descr_p->debouncePeriod_ms = initStruct_p->debouncePeriod_ms;
	descr_p->eventHandler = initStruct_p->eventHandler;
	descr_p->stateGetter = initStruct_p->stateGetter;
	descr_p->userCallbackValue = initStruct_p->userCallbackValue;

	return butFilter_error_ok;
}

butFilter_error_t butFilter_handle(butFilter_t* instance_p)
{
	DESCR(instance_p);

	bool curIsDown;
	if (!descr_p->stateGetter(&curIsDown))
	{
		return butFilter_error_hardware;
	}

	descr_p->keyDownEvent = false;
	descr_p->keyPressEvent = false;
	descr_p->keyUpEvent = false;

	const uint32_t timestamp = app_hal_getTimestamp();

	if (curIsDown != descr_p->isDown_unstable)
	{
		descr_p->isDown_unstable = curIsDown;
		descr_p->unstableIsDownBegin_ms = timestamp;
	}
	else
	{
		const uint32_t constantStateDuration = getTimeElapsed(descr_p->unstableIsDownBegin_ms, timestamp);
		if (constantStateDuration >= descr_p->debouncePeriod_ms)
		{
			if (!descr_p->isDown_stable && curIsDown)
			{
				descr_p->keyDownEvent = true;
				descr_p->keyPressEvent = true;
			}
			else if (descr_p->isDown_stable && !curIsDown)
			{
				descr_p->keyUpEvent = true;
			}
		}

		descr_p->isDown_stable = curIsDown;
	}

	if (descr_p->eventHandler != NULL)
	{
		if (descr_p->keyDownEvent)
		{
			descr_p->eventHandler(butFilter_event_down, descr_p->userCallbackValue);
		}

		if (descr_p->keyPressEvent)
		{
			descr_p->eventHandler(butFilter_event_press, descr_p->userCallbackValue);
		}

		if (descr_p->keyUpEvent)
		{
			descr_p->eventHandler(butFilter_event_up, descr_p->userCallbackValue);
		}
	}

	return butFilter_error_ok;
}

butFilter_error_t butFilter_hasEvent(const butFilter_t* instance_p, butFilter_event_t event, bool* result_out_p)
{
	CDESCR(instance_p);
	assert(result_out_p != NULL);

	switch (event)
	{
		case butFilter_event_down:
			*result_out_p = descr_p->keyDownEvent;
			break;

		case butFilter_event_press:
			*result_out_p = descr_p->keyPressEvent;
			break;

		case butFilter_event_up:
			*result_out_p = descr_p->keyUpEvent;
			break;

		default:
			return butFilter_error_invalidEvent;
	}

	return butFilter_error_ok;
}

butFilter_error_t butFilter_isDown(const butFilter_t* instance_p, bool* result_out_p)
{
	CDESCR(instance_p);

	assert(result_out_p != NULL);

	*result_out_p = descr_p->isDown_stable;
	return butFilter_error_ok;
}

static bool checkDescr(const descriptor_t* descr_p)
{
	//TODO: implement some descriptor checking logic
	return true;
}
