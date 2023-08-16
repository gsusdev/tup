// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "tup_bufReader.h"

#include <assert.h>
#include <string.h>

#include "tup_endianness.h"

typedef struct
{
    const void volatile* buf_p;
    const void volatile* cur_p;
    size_t maxSize_bytes;
    bool isBigEndian;
} descriptor_t;

static_assert(sizeof(descriptor_t) <= sizeof(tup_bufReader_t), "Adjust the \"privateData\" field size in the \"tup_bufReader_t\" struct");

#define _DESCR(d, ret, qual)                            \
    assert(d != NULL);                                  \
    qual descriptor_t* descr_p = (qual descriptor_t*)d; \
    if (!checkDescr(descr_p))                           \
    {                                                   \
        return ret;                                     \
    }

#define DESCR(d, ret)  _DESCR(d, ret, )
#define CDESCR(d, ret) _DESCR(d, ret, const)

static bool mayRead(const descriptor_t* descr_p, size_t size_bytes);
static void advance(descriptor_t* descr_p, size_t size_bytes);
static bool checkDescr(const descriptor_t* descr_p);
static size_t getSize(const descriptor_t* descr_p);

bool tup_bufReader_init(tup_bufReader_t* descriptor_p, const void volatile* buf_p, size_t size_bytes, bool bigEndian)
{
    assert(descriptor_p != NULL);

    if ((buf_p == NULL) || (size_bytes == 0))
    {
        return false;
    }

    descriptor_t* descr_p = (descriptor_t*)descriptor_p;

    descr_p->buf_p = buf_p;
    descr_p->cur_p = buf_p;
    descr_p->maxSize_bytes = size_bytes;
    descr_p->isBigEndian = bigEndian;

    return true;
}

bool tup_bufReader_reset(tup_bufReader_t* descriptor_p)
{
    DESCR(descriptor_p, false);

    descr_p->cur_p = descr_p->buf_p;

    return true;
}

size_t tup_bufReader_getSize(const tup_bufReader_t* descriptor_p)
{
    CDESCR(descriptor_p, 0);

    const size_t result = getSize(descr_p);
    return result;
}

const void volatile* tup_bufReader_getBase(const tup_bufReader_t* descriptor_p)
{
    CDESCR(descriptor_p, NULL);

    return descr_p->buf_p;
}

const void volatile* tup_bufReader_getPtr(const tup_bufReader_t* descriptor_p)
{
    CDESCR(descriptor_p, NULL);

    return descr_p->cur_p;
}

uint32_t tup_bufReader_readU32(tup_bufReader_t* descriptor_p, bool* okFlag_p)
{
    assert(descriptor_p != NULL);
    descriptor_t* descr_p = (descriptor_t*)descriptor_p;
    if (!checkDescr(descr_p))
    {
        if (okFlag_p != NULL)
        {
            *okFlag_p = false;
        }

        return 0;
    }

    uint32_t result = 0;
    const size_t readSize = sizeof(result);

    if (!mayRead(descr_p, readSize))
    {
        if (okFlag_p != NULL)
        {
            *okFlag_p = false;
        }

        return 0;
    }

    const uint32_t* val_p = (const uint32_t*)descr_p->cur_p;
    result = tup_endianness_convert32(*val_p, descr_p->isBigEndian);

    advance(descr_p, readSize);

    return result;
}

uint16_t tup_bufReader_readU16(tup_bufReader_t* descriptor_p, bool* okFlag_p)
{
    assert(descriptor_p != NULL);
    descriptor_t* descr_p = (descriptor_t*)descriptor_p;
    if (!checkDescr(descr_p))
    {
        if (okFlag_p != NULL)
        {
            *okFlag_p = false;
        }

        return 0;
    }

    uint16_t result = 0;
    const size_t readSize = sizeof(result);

    if (!mayRead(descr_p, readSize))
    {
        if (okFlag_p != NULL)
        {
            *okFlag_p = false;
        }

        return 0;
    }

    const uint16_t* val_p = (const uint16_t*)descr_p->cur_p;
    result = tup_endianness_convert16(*val_p, descr_p->isBigEndian);

    advance(descr_p, readSize);

    return result;
}

uint8_t tup_bufReader_readU8(tup_bufReader_t* descriptor_p, bool* okFlag_p)
{
    assert(descriptor_p != NULL);
    descriptor_t* descr_p = (descriptor_t*)descriptor_p;
    if (!checkDescr(descr_p))
    {
        if (okFlag_p != NULL)
        {
            *okFlag_p = false;
        }

        return 0;
    }

    uint8_t result = 0;
    const size_t readSize = sizeof(result);

    if (!mayRead(descr_p, readSize))
    {
        if (okFlag_p != NULL)
        {
            *okFlag_p = false;
        }

        return 0;
    }

    const uint8_t* val_p = (const uint8_t*)descr_p->cur_p;
    result = tup_endianness_convert32(*val_p, descr_p->isBigEndian);

    advance(descr_p, readSize);

    return result;
}

bool tup_bufReader_readBuf(tup_bufReader_t* descriptor_p, void* buf_out_p, size_t readSize_bytes, bool* okFlag_p)
{
    assert(descriptor_p != NULL);
    descriptor_t* descr_p = (descriptor_t*)descriptor_p;
    if (!checkDescr(descr_p))
    {
        if (okFlag_p != NULL)
        {
            *okFlag_p = false;
        }

        return false;
    }
        
    if (!mayRead(descr_p, readSize_bytes))
    {
        if (okFlag_p != NULL)
        {
            *okFlag_p = false;
        }

        return false;
    }

    memcpy(buf_out_p, descr_p->cur_p, readSize_bytes);
    advance(descr_p, readSize_bytes);

    return true;
}

bool tup_bufReader_skip(tup_bufReader_t* descriptor_p, size_t size_bytes, bool* okFlag_p)
{
    assert(descriptor_p != NULL);
    descriptor_t* descr_p = (descriptor_t*)descriptor_p;
    if (!checkDescr(descr_p))
    {
        if (okFlag_p != NULL)
        {
            *okFlag_p = false;
        }

        return false;
    }

    if (!mayRead(descr_p, size_bytes))
    {
        if (okFlag_p != NULL)
        {
            *okFlag_p = false;
        }

        return false;
    }

    advance(descr_p, size_bytes);

    return true;
}

static size_t getSize(const descriptor_t* descr_p)
{
    const size_t result = (uintptr_t)descr_p->cur_p - (uintptr_t)descr_p->buf_p;
    return result;
}

static bool mayRead(const descriptor_t* descr_p, size_t size_bytes)
{
    const size_t size = getSize(descr_p);
    const size_t remainingSize = descr_p->maxSize_bytes - size;

    return size_bytes <= remainingSize;
}

static void advance(descriptor_t* descr_p, size_t size_bytes)
{
    descr_p->cur_p = (const void*)((uintptr_t)descr_p->cur_p + size_bytes);
}

static bool checkDescr(const descriptor_t* descr_p)
{
    //TODO: implement some descriptor checking logic
    return true;
}
