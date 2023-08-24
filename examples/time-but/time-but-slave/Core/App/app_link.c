#include "app_link.h"

#include <assert.h>
#include <stdatomic.h>

#include "app_settings.h"
#include "app_hal.h"
#include "app_utils.h"

#include "tup_instance.h"
#include "tup_port.h"

static struct
{
	volatile _Atomic bool isSignalFired;
	bool isUpdated;
	bool isInit;
	tup_instance_t tup;
	uint32_t lastHandlingTime_ms;
	app_protocol_masterOutputData_t* masterData_p;
	const app_protocol_slaveOutputData_t* slaveData_p;
	uint8_t tupWorkBuffer[TUP_RX_BUF_SIZE_BYTES * 2];
	uint8_t slavePayloadBuf[APP_PROTOCOL_SLAVE_MESSAGE_SIZE_BYTES];
} link = {0};

static const char instanceName[] = "TUP_SLAVE";

static bool initUart();
static bool initTup();

static void received(const volatile void* buf_p, size_t receivedSize_bytes);
static void rxError(int error);
static void transmitted(size_t transmittedSize_bytes);

static void onSendResultHandler(uintptr_t callbackValue);
static void onReceiveDataHandler(const void volatile* buf_p, size_t size_bytes, bool isFinal, uintptr_t callbackValue);

static void linkTransmitHandler(const void* buf_p, size_t size_bytes, uintptr_t callbackValue);
static void signalFireHandle(uintptr_t signal, uintptr_t callbackValue);

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

	link.masterData_p = initStruct_p->masterData_p;
	link.slaveData_p = initStruct_p->slaveData_p;

	link.isInit = true;

	return true;
}

void app_link_handle()
{
	if (!link.isInit)
	{
		return;
	}

	const uint32_t timestamp = app_hal_getTimestamp();
	const uint32_t elapsed = getTimeElapsed(link.lastHandlingTime_ms, timestamp);

	bool signalFired = true;
	bool needHandling = atomic_compare_exchange_strong(&link.isSignalFired, &signalFired, false);
	needHandling |= (elapsed >= TUP_HANDLE_INTERVAL_MS);

	if (needHandling)
	{
		tup_handle(&link.tup);
		link.lastHandlingTime_ms = timestamp;
	}
}

bool app_link_isUpdated()
{
	if (!link.isInit)
	{
		return false;
	}

	return link.isUpdated;
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
	tup_port_setSignalFireHandler(signalFireHandle);

	initStruct.isMaster = false;
	initStruct.name = instanceName;

	initStruct.dataTimeout_ms = TUP_DATA_TIMEOUT_MS;
	initStruct.flushDuration_ms = TUP_FLUSH_DURATION_MS;

	initStruct.retryPause_ms = TUP_RETRY_PAUSE_MS;
	initStruct.rxBufSize_bytes = TUP_RX_BUF_SIZE_BYTES;
	initStruct.synTimeout_ms = TUP_SYN_TIMEOUT_MS;
	initStruct.tryCount = TUP_TRY_COUNT;

	initStruct.workBuffer_p = link.tupWorkBuffer;
	initStruct.workBufferSize_bytes = sizeof(link.tupWorkBuffer);
	initStruct.onSendResult = onSendResultHandler;
	initStruct.onReceiveData = onReceiveDataHandler;
	initStruct.signal = 1;

	const tup_error_t err = tup_init(&link.tup, &initStruct);
	if (err != tup_error_ok)
	{
		return false;
	}

	link.isInit = true;

	return true;
}

static void linkTransmitHandler(const void* buf_p, size_t size_bytes, uintptr_t callbackValue)
{
	(void)callbackValue;

	app_hal_uartSend(buf_p, size_bytes);
}

static void signalFireHandle(uintptr_t signal, uintptr_t callbackValue)
{
	if (!link.isInit)
	{
		return;
	}

	(void)signal;
	(void)callbackValue;

	link.isSignalFired = true;
}

static void onSendResultHandler(uintptr_t callbackValue)
{
	(void)callbackValue;

	const bool ok = app_protocol_encodeSlaveOutput(link.slaveData_p, link.slavePayloadBuf, sizeof(link.slavePayloadBuf));
	if (!ok)
	{
		return;
	}

	tup_sendData(&link.tup, link.slavePayloadBuf, sizeof(link.slavePayloadBuf));
}

static void onReceiveDataHandler(const void volatile* buf_p, size_t size_bytes, bool isFinal, uintptr_t callbackValue)
{
	(void)callbackValue;

	tup_transfer_result_t result = TUP_ERROR_UNKNOWN;

	if (isFinal)
	{
		const bool ok = app_protocol_decodeMasterOutput((const void*)buf_p, size_bytes, link.masterData_p);
		if (ok)
		{
			link.isUpdated = true;
			result = TUP_OK;
		}
	}

	tup_setResult(&link.tup, result);
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
