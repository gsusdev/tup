#include "app_main.h"

#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"

#include "app_settings.h"
#include "app_hal.h"
#include "app_link.h"
#include "app_protocol.h"

static struct
{
	uint32_t currentTimestamp_ms;
	app_protocol_masterOutputData_t masterOutput;
	app_protocol_slaveOutputData_t slaveOutput;
	StaticTask_t taskBuffer;
	TaskHandle_t taskHandle;
	StackType_t taskStack[APP_TASK_STACK_LEN];
	bool isInit;
} app = {0};

static bool initLink();

bool app_init()
{
	if (!initLink())
	{
		return false;
	}

	if (!initTask)
	{
		return false;
	}

	app.isInit = true;

	return true;
}

static void taskFxn(void* parameters_p)
{
	(void)parameters_p;

	const TickType_t delay_ticks = 20u / portTICK_PERIOD_MS;

	for (;;)
	{
		vTaskDelay(delay_ticks);
	}
}

static bool initTask()
{
	TaskHandle_t handle = xTaskCreateStatic(
		  taskFxn
	    , "TIME-BUT"
		, APP_TASK_STACK_LEN
		, NULL
		, APP_TASK_PRIORITY
		, app.taskStack
		, &app.taskBuffer);

	if (handle == NULL)
	{
		return false;
	}

	app.taskHandle = handle;
	return true;
}

static bool initLink()
{
	app_link_initStruct_t initStruct = {0};

	initStruct.masterData_p = &app.masterOutput;
	initStruct.slaveData_p = &app.slaveOutput;

	const bool result = app_link_init(&initStruct);
	return result;
}
