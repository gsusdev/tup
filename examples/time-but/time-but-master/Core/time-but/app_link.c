#include "app_link.h"

#include <assert.h>
#include <stdatomic.h>

#include "FreeRTOS.h"
#include "task.h"

#include "app_settings.h"
#include "app_hal.h"

#include "tup_instance.h"
#include "tup_port.h"

static struct
{
	tup_instance_t tup;
	volatile _Atomic bool isUpdated;
	const app_protocol_masterOutputData_t* masterData_p;
	app_protocol_slaveOutputData_t* slaveData_p;
	StaticTask_t taskBuffer;
	TaskHandle_t taskHandle;
	StackType_t taskStack[TUP_TASK_STACK_LEN];
	uint8_t tupWorkBuffer[TUP_RX_BUF_SIZE_BYTES * 2];
	uint8_t masterPayloadBuf[APP_PROTOCOL_MASTER_MESSAGE_SIZE_BYTES];
	bool isInit;
	bool isConnected;
} link = {0};

static const char instanceName[] = "TUP_MASTER";

static bool initUart();
static bool initTup();
static bool initTask();

static void received(const volatile void* buf_p, size_t receivedSize_bytes);
static void rxError(int error);
static void transmitted(size_t transmittedSize_bytes);

static void onReceiveDataHandler(const void volatile* buf_p, size_t size_bytes, bool isFinal, uintptr_t callbackValue);
static void onConnectHandler(uintptr_t callbackValue);

static void linkTransmitHandler(const void* buf_p, size_t size_bytes, uintptr_t callbackValue);

static void signalFireHandler(uintptr_t signal, uintptr_t callbackValue);
static bool signalWaitHandler(uintptr_t signal, uint32_t timeout_ms, uintptr_t callbackValue);

static uintptr_t enterCriticalHandler();
static void exitCriticalHandler(uintptr_t returnValue);
static uintptr_t enterCriticalIsrHandler();
static void exitCriticalIsrHandler(uintptr_t returnValue);

bool app_link_init(const app_link_initStruct_t* initStruct_p)
{
	assert(initStruct_p != NULL);

	bool ok = true;
	ok |= initStruct_p->masterData_p != NULL;
	ok |= initStruct_p->slaveData_p != NULL;

	if (!ok)
	{
		return false;
	}

	if (!initUart())
	{
		return false;
	}

	if (!initTup())
	{
		return false;
	}

	if (!initTask())
	{
		return false;
	}

	link.masterData_p = initStruct_p->masterData_p;
	link.slaveData_p = initStruct_p->slaveData_p;

	link.isInit = true;

	return true;
}

bool app_link_sendToSlave()
{
	const bool ok = app_protocol_encodeMasterOutput(link.masterData_p, link.masterPayloadBuf, sizeof(link.masterPayloadBuf));
	if (!ok)
	{
		return false;
	}

	const tup_error_t err = tup_sendData(&link.tup, link.masterPayloadBuf, sizeof(link.masterPayloadBuf));
	return (err == tup_error_ok);
}

bool app_link_isUpdated()
{
	if (!link.isInit)
	{
		return false;
	}

	return link.isUpdated;
}

bool app_link_isConnected()
{
	if (!link.isInit)
	{
		return false;
	}

	return link.isConnected;
}

void app_link_resetIsUpdated()
{
	if (!link.isInit)
	{
		return;
	}

	link.isUpdated = false;
}

static bool initUart()
{
	const bool result = app_hal_uartInit(TUP_PORT_BAUDRATE, received, rxError, transmitted);
	return result;
}

static bool initTup()
{
	tup_initStruct_t initStruct = {0};

	tup_port_setGetCurrentTimeHandler(app_hal_getTimestamp);
	tup_port_setLinkTransmitHandler(linkTransmitHandler);
	tup_port_setLogHandler(app_hal_log);
	tup_port_setSignalFireHandler(signalFireHandler);
	tup_port_setSignalWaitHandler(signalWaitHandler);
	tup_port_setEnterCriticalHandler(enterCriticalHandler);
	tup_port_setExitCriticalHandler(exitCriticalHandler);
	tup_port_setEnterCriticalIsrHandler(enterCriticalIsrHandler);
	tup_port_setExitCriticalIsrHandler(exitCriticalIsrHandler);

	initStruct.isMaster = true;
	initStruct.name = instanceName;

	initStruct.dataTimeout_ms = TUP_DATA_TIMEOUT_MS;
	initStruct.flushDuration_ms = TUP_FLUSH_DURATION_MS;

	initStruct.retryPause_ms = TUP_RETRY_PAUSE_MS;
	initStruct.rxBufSize_bytes = TUP_RX_BUF_SIZE_BYTES;
	initStruct.synTimeout_ms = TUP_SYN_TIMEOUT_MS;
	initStruct.tryCount = TUP_TRY_COUNT;

	initStruct.workBuffer_p = link.tupWorkBuffer;
	initStruct.workBufferSize_bytes = sizeof(link.tupWorkBuffer);
	initStruct.onReceiveData = onReceiveDataHandler;
	initStruct.onConnect = onConnectHandler;
	initStruct.signal = 1;

	tup_error_t err = tup_init(&link.tup, &initStruct);
	if (err != tup_error_ok)
	{
		return false;
	}

	err = tup_accept(&link.tup);
	if (err != tup_error_ok)
	{
		return false;
	}

	link.isInit = true;

	return true;
}

static void taskFxn(void* parameters_p)
{
	(void)parameters_p;

	tup_run(&link.tup);
}

static bool initTask()
{
	TaskHandle_t handle = xTaskCreateStatic(
		  taskFxn
	    , instanceName
		, TUP_TASK_STACK_LEN
		, NULL
		, TUP_TASK_PRIORITY
		, link.taskStack
		, &link.taskBuffer);

	if (handle == NULL)
	{
		return false;
	}

	link.taskHandle = handle;
	return true;
}

static void onReceiveDataHandler(const void volatile* buf_p, size_t size_bytes, bool isFinal, uintptr_t callbackValue)
{
	(void)callbackValue;

	tup_transfer_result_t result = TUP_ERROR_UNKNOWN;

	if (isFinal)
	{
		const bool ok = app_protocol_decodeSlaveOutput((const void*)buf_p, size_bytes, link.slaveData_p);
		if (ok)
		{
			link.isUpdated = true;
			result = TUP_OK;
		}
	}

	tup_setResult(&link.tup, result);
}

static void onConnectHandler(uintptr_t callbackValue)
{
	link.isConnected = true;
}

static void linkTransmitHandler(const void* buf_p, size_t size_bytes, uintptr_t callbackValue)
{
	(void)callbackValue;

	app_hal_uartSend(buf_p, size_bytes);
}

static void received(const volatile void* buf_p, size_t receivedSize_bytes)
{
	if (!link.isInit)
	{
		return;
	}

	tup_received(&link.tup, buf_p, receivedSize_bytes);
}

static void rxError(int error)
{
	if (!link.isInit)
	{
		return;
	}

	(void)error;

	tup_rxError(&link.tup);
}


static void transmitted(size_t transmittedSize_bytes)
{
	if (!link.isInit)
	{
		return;
	}

	tup_transmitted(&link.tup, transmittedSize_bytes);
}

static void signalFireHandler(uintptr_t signal, uintptr_t callbackValue)
{
	if (link.isInit)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		vTaskNotifyGiveFromISR(link.taskHandle, &xHigherPriorityTaskWoken);

		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

static bool signalWaitHandler(uintptr_t signal, uint32_t timeout_ms, uintptr_t callbackValue)
{
	if (!link.isInit)
	{
		return false;
	}

	const TickType_t timeout_ticks = timeout_ms / portTICK_PERIOD_MS;
	const uint32_t result = ulTaskNotifyTake(pdTRUE, timeout_ticks);

	return result > 0;
}

static uintptr_t enterCriticalHandler()
{
	taskENTER_CRITICAL();
	return 0;
}

static void exitCriticalHandler(uintptr_t returnValue)
{
	taskEXIT_CRITICAL();
}

static uintptr_t enterCriticalIsrHandler()
{
	const UBaseType_t result = taskENTER_CRITICAL_FROM_ISR();
	return (uintptr_t)result;
}

static void exitCriticalIsrHandler(uintptr_t returnValue)
{
	taskEXIT_CRITICAL_FROM_ISR((UBaseType_t)returnValue);
}
