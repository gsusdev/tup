#include "app_main.h"

#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "app_settings.h"
#include "app_hal.h"
#include "app_link.h"
#include "app_protocol.h"

typedef struct time_t
{
 	int8_t hour;
  	int8_t minute;
   	int8_t second;
} time_t;


static struct
{
	app_protocol_masterOutputData_t masterOutput;
	app_protocol_slaveOutputData_t slaveOutput;
	StaticTask_t taskBuffer;
	TaskHandle_t taskHandle;
	StackType_t taskStack[APP_TASK_STACK_LEN];
	StaticSemaphore_t mutexBuffer;
	SemaphoreHandle_t mutexHandle;
	time_t timeToSend;
	bool isSendingTime;
	bool isInit;
	volatile bool isConnectClicked;
} app = {0};

static bool initLink();
static bool initTask();

static bool lock();
static void unlock();

static void onConnectHandler();
static void onFailHandler();

void app_display_showStatus(const char* text);
void app_display_showSlaveInfo(int8_t h, int8_t m, int8_t s, bool isButDown, int8_t clickCount);

static void updateSendingTime()
{
	if (app.isSendingTime)
	{
		app.masterOutput.hour = app.timeToSend.hour;
		app.masterOutput.minute = app.timeToSend.minute;
		app.masterOutput.second = app.timeToSend.second;
	}
	else
	{
		app.masterOutput.hour = 24;
		app.masterOutput.minute = 60;
		app.masterOutput.second = 60;
	}
}

void app_timeToSendUpdated(int8_t h, int8_t m, int8_t s)
{
	if (lock())
	{
		app.timeToSend.hour = h;
		app.timeToSend.minute = m;
		app.timeToSend.second = s;

		updateSendingTime();

		unlock();
	}
}

void app_connectButClicked()
{
	if (!app.isInit)
	{
		return;
	}

	app_display_showStatus("Connecting");
	if (!app_link_connect())
	{
		app_display_showStatus("Fail");
	}
}

void app_sendTimeFlagChanged(bool isSending)
{
	if (lock())
	{
		app.isSendingTime = isSending;
		updateSendingTime();

		unlock();
	}
}

bool app_init()
{
	// init task first in order to intialize the mutex and pass it to the link
	if (!initTask())
	{
		return false;
	}

	if (!initLink())
	{
		return false;
	}

	app.masterOutput.hour = 24;
	app.masterOutput.minute = 60;
	app.masterOutput.second = 60;

	app.isInit = true;

	return true;
}

static void onRetryProgressHandler(uint32_t attemptNumber, uint32_t maxAttemptCount, uint32_t remainingPauseTime_ms)
{
	static char text[32];

	if (remainingPauseTime_ms == 0)
	{
		if (attemptNumber > 1)
	    {
			const int sz = snprintf(text, sizeof(text), "Attempt %lu of %lu", attemptNumber, maxAttemptCount);
			if (sz < (int)sizeof(text))
			{
				app_display_showStatus(text);
			}
	    }
	}
	else
	{
		uint32_t remainingSeconds = remainingPauseTime_ms / 1000;
		if ((remainingPauseTime_ms % 1000) > 500)
		{
			++remainingSeconds;
		}

		const int sz = snprintf(text, sizeof(text), "Retry in %lu sec", remainingSeconds);
		if (sz < (int)sizeof(text))
		{
			app_display_showStatus(text);
		}
	}
}

static void onConnectHandler()
{
	app_display_showStatus("Connected");
}

static void onFailHandler()
{
	app_display_showStatus("Fail");
}

static bool lock()
{
	return true;
	const TickType_t delay_ticks = 100 / portTICK_PERIOD_MS;

	const bool result = xSemaphoreTake(app.mutexHandle, delay_ticks) == pdTRUE;
	return result;
}

static void unlock()
{
	return;
	xSemaphoreGive(app.mutexHandle);
}

static void showInfoFromSlave()
{
	if (!lock())
	{
		return;
	}

	uint8_t hour = app.slaveOutput.hour;
	uint8_t minute = app.slaveOutput.minute;
	uint8_t second = app.slaveOutput.second;
	uint8_t clickCount = app.slaveOutput.butClickCount;
	bool butDown = app.slaveOutput.isButtonDown;

	unlock();

	app_display_showSlaveInfo(hour, minute, second, butDown, clickCount);
}

static void taskFxn(void* parameters_p)
{
	(void)parameters_p;

	const TickType_t delay_ticks = 100 / portTICK_PERIOD_MS;

	for (;;)
	{
		if (app_link_isUpdated())
		{
			showInfoFromSlave();
			app_link_resetIsUpdated();
		}

		if (!app_link_isBusy())
		{
			app_link_sendToSlave();
		}

		vTaskDelay(delay_ticks);
	}
}

static bool initTask()
{
	SemaphoreHandle_t mutexHandle = xSemaphoreCreateMutexStatic(&app.mutexBuffer);
	if (mutexHandle == NULL)
	{
		return false;
	}

	TaskHandle_t taskHandle = xTaskCreateStatic(
		  taskFxn
	    , "TIME-BUT"
		, APP_TASK_STACK_LEN
		, NULL
		, APP_TASK_PRIORITY
		, app.taskStack
		, &app.taskBuffer);

	if (taskHandle == NULL)
	{
		vSemaphoreDelete(mutexHandle);
		return false;
	}

	app.taskHandle = taskHandle;
	app.mutexHandle = mutexHandle;

	return true;
}

static bool initLink()
{
	app_link_initStruct_t initStruct = {0};

	initStruct.masterData_p = &app.masterOutput;
	initStruct.slaveData_p = &app.slaveOutput;
	initStruct.onRetryProgress = onRetryProgressHandler;
	initStruct.onConnect = onConnectHandler;
	initStruct.onFail = onFailHandler;
	initStruct.mutex = app.mutexHandle;

	const bool result = app_link_init(&initStruct);
	return result;
}



