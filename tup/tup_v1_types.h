#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    TUP_OK = 0
    , TUP_ERROR_LEN = 1
    , TUP_ERROR_UNKNOWN = 2
    , TUP_ERROR_NOMEMORY = 3
    , TUP_ERROR_CRC32 = 4
} tup_transfer_result_t;

typedef enum
{
    TUP_NO_ERROR = 0
    , TUP_SYNC_NO_ACK_ERROR = 200
    , TUP_SYNC_BAD_ACK_ERROR = 201
    , TUP_SYNC_WRONG_PKG_ERROR = 202
    , TUP_DATA_NO_ACK_ERROR = 210
    , TUP_DATA_BAD_ACK_ERROR = 211
    , TUP_DATA_WRONG_PKG_ERROR = 212
    , TUP_FIN_NO_ACK_ERROR = 220
    , TUP_FIN_BAD_ACK_ERROR = 221
    , TUP_FIN_WRONG_PKG_ERROR = 222
} tup_transfer_fail_t;

#define TUP_VERSION_1 1u