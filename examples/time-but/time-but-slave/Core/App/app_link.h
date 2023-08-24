#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "app_protocol.h"

typedef struct app_link_initStruct_t
{
	app_protocol_masterOutputData_t* masterData_p;
	const app_protocol_slaveOutputData_t* slaveData_p;
} app_link_initStruct_t;

bool app_link_init(const app_link_initStruct_t* initStruct_p);
void app_link_handle();
bool app_link_isUpdated();
void app_link_resetIsUpdated();

