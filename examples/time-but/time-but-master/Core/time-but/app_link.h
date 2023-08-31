#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "app_protocol.h"

typedef void (*app_link_onRetryProgress_t)(uint32_t attemptNumber, uint32_t maxAttemptCount, uint32_t remainingPauseTime_ms);
typedef void (*app_link_onConnect_t)();
typedef void (*app_link_onFail_t)();

typedef struct app_link_initStruct_t
{
	app_link_onRetryProgress_t onRetryProgress;
	app_link_onConnect_t onConnect;
	app_link_onFail_t onFail;
	const app_protocol_masterOutputData_t* masterData_p;
	app_protocol_slaveOutputData_t* slaveData_p;
	SemaphoreHandle_t mutex;
} app_link_initStruct_t;

bool app_link_init(const app_link_initStruct_t* initStruct_p);
bool app_link_connect();
bool app_link_sendToSlave();
bool app_link_isConnected();
bool app_link_isBusy();
bool app_link_isUpdated();
void app_link_resetIsUpdated();

