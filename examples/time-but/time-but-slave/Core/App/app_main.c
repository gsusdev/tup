#include "app_main.h"

#include <string.h>

#include "app_settings.h"
#include "app_hal.h"
#include "app_link.h"
#include "app_protocol.h"
#include "app_butFilter.h"
#include "app_utils.h"

static struct
{
	bool isInit;
	uint32_t currentTimestamp_ms;
	uint32_t lastLinkHandle_ms;
	uint32_t lastButHandle_ms;
	app_protocol_masterOutputData_t masterOutput;
	app_protocol_slaveOutputData_t slaveOutput;
	butFilter_t butFilter;
} app = {0};

static bool initButton();
static void handleButton();

static bool initLink();
static void handleLink();

static void setTime();
static void getTime();

static bool isHandlingNeeded(uint32_t lastTime, uint32_t interval_ms);

bool app_init()
{
	if (!initButton())
	{
		return false;
	}

	if (!initLink())
	{
		return false;
	}

	memset(&app.slaveOutput, 0, sizeof(app.slaveOutput));

	app.slaveOutput.isBigEndian = isBigEndian();
	app.slaveOutput.mcuIdSize = app_hal_getMcuId(app.slaveOutput.mcuId, sizeof(app.slaveOutput.mcuId));

	app.isInit = true;

	return true;
}

void app_handle()
{
	if (!app.isInit)
	{
		return;
	}

	app.currentTimestamp_ms = app_hal_getTimestamp();

	handleButton();
	handleLink();
}

static bool initLink()
{
	app_link_initStruct_t initStruct = {0};

	initStruct.masterData_p = &app.masterOutput;
	initStruct.slaveData_p = &app.slaveOutput;

	const bool result = app_link_init(&initStruct);
	return result;
}

static void handleLink()
{
	const bool handlingNeeded = isHandlingNeeded(app.lastLinkHandle_ms, TUP_HANDLE_INTERVAL_MS);

	if (handlingNeeded)
	{
		app.lastLinkHandle_ms = app.currentTimestamp_ms;
		getTime();
	}

	app_link_handle();

	if (app_link_isUpdated())
	{
		app_link_resetIsUpdated();
		setTime();
	}

}

static bool initButton()
{
	butFilter_initStruct initStruct = {0};

	initStruct.debouncePeriod_ms = BUT_DEBOUNCE_PERIOD_MS;
	initStruct.stateGetter = app_hal_getButtonState;

	const butFilter_error_t err = butFilter_init(&app.butFilter, &initStruct);

	return err == butFilter_error_ok;
}

static void handleButton()
{
	if (!app.isInit)
	{
		return;
	}

	if (isHandlingNeeded(app.lastButHandle_ms, BUT_HANDLE_INTERVAL_MS))
	{
		butFilter_handle(&app.butFilter);
		app.lastButHandle_ms = app.currentTimestamp_ms;

		bool isEvent;

		butFilter_error_t err = butFilter_hasEvent(&app.butFilter, butFilter_event_press, &isEvent);
		if ((err == butFilter_error_ok) && isEvent)
		{
			++app.slaveOutput.butClickCount;
		}

		bool isDown;
		err = butFilter_isDown(&app.butFilter, &isDown);
		if (err == butFilter_error_ok)
		{
			app.slaveOutput.isButtonDown = isDown;
		}
	}
}

static bool isHandlingNeeded(uint32_t lastTime, uint32_t interval_ms)
{
	const uint32_t elapsed = getTimeElapsed(lastTime, app.currentTimestamp_ms);
	const bool result = elapsed >= interval_ms;

	return result;
}

static void setTime()
{
	bool correct = true;

	correct &= app.masterOutput.hour < 24;
	correct &= app.masterOutput.minute < 60;
	correct &= app.masterOutput.second < 60;

	if (correct)
	{
		app_hal_setTime(2023, 1, 1, app.masterOutput.hour, app.masterOutput.minute, app.masterOutput.second, 0);
	}
}

static void getTime()
{
	uint8_t hour;
	uint8_t minute;
	uint8_t second;

	if (app_hal_getTime(NULL, NULL, NULL, &hour, &minute, &second, NULL))
	{
		app.slaveOutput.hour = hour;
		app.slaveOutput.minute = minute;
		app.slaveOutput.second = second;
	}
}
