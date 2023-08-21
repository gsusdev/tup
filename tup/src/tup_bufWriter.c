// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "tup_bufWriter.h"

#include <assert.h>
#include <string.h>

#include "tup_endianness.h"

typedef struct descriptor_t
{
	void* buf_p;
    void* cur_p;
	size_t maxSize_bytes;	
    bool isBigEndian;
} descriptor_t;

static_assert(sizeof(descriptor_t) <= sizeof(tup_bufWriter_t), "Adjust the \"privateData\" field size in the \"tup_bufWriter_t\" struct");

#define _DESCR(d, ret, qual)                            \
    assert(d != NULL);                                  \
    qual descriptor_t* descr_p = (qual descriptor_t*)d; \
    if (!checkDescr(descr_p))                           \
    {                                                   \
        return ret;                                     \
    }

#define DESCR(d, ret)  _DESCR(d, ret, )
#define CDESCR(d, ret) _DESCR(d, ret, const)

static bool mayWrite(const descriptor_t* descr_p, size_t size_bytes);
static void advance(descriptor_t* descr_p, size_t size_bytes);
static bool checkDescr(const descriptor_t* descr_p);
static size_t getSize(const descriptor_t* descr_p);

bool tup_bufWriter_init(tup_bufWriter_t* descriptor_p, void* buf_p, size_t maxSize_bytes, bool bigEndian)
{
    assert(descriptor_p != NULL);
    
    if ((buf_p == NULL) || (maxSize_bytes == 0))
    {
        return false;
    }

    descriptor_t* descr_p = (descriptor_t*)descriptor_p;
    memset(descr_p, 0, sizeof(*descr_p));

    descr_p->buf_p = buf_p;
    descr_p->cur_p = buf_p;
    descr_p->maxSize_bytes = maxSize_bytes;
    descr_p->isBigEndian = bigEndian;    
    
    return true;
}

bool tup_bufWriter_reset(tup_bufWriter_t* descriptor_p)
{
    DESCR(descriptor_p, false);

    descr_p->cur_p = descr_p->buf_p;

    return true;
}

size_t tup_bufWriter_getSize(const tup_bufWriter_t* descriptor_p)
{
    CDESCR(descriptor_p, 0);

    const size_t result = getSize(descr_p);
    return result;
}

const void* tup_bufWriter_getBase(const tup_bufWriter_t* descriptor_p)
{
    CDESCR(descriptor_p, NULL);

    return descr_p->buf_p;
}

const void* tup_bufWriter_getPtr(const tup_bufWriter_t* descriptor_p)
{
    CDESCR(descriptor_p, NULL);

    return descr_p->cur_p;
}

bool tup_bufWriter_writeU32(tup_bufWriter_t* descriptor_p, uint32_t value)
{
    DESCR(descriptor_p, false);

    const size_t writeSize = sizeof(value);
    if (!mayWrite(descr_p, writeSize))
    {
        return false;
    }

    uint32_t* val_p = (uint32_t*)descr_p->cur_p;
    *val_p = tup_endianness_convert32(value, descr_p->isBigEndian);

    advance(descr_p, writeSize);

    return true;
}

bool tup_bufWriter_writeU16(tup_bufWriter_t* descriptor_p, uint16_t value)
{
    DESCR(descriptor_p, false);

    const size_t writeSize = sizeof(value);
    if (!mayWrite(descr_p, writeSize))
    {
        return false;
    }

    uint16_t* val_p = (uint16_t*)descr_p->cur_p;
    *val_p = tup_endianness_convert16(value, descr_p->isBigEndian);

    advance(descr_p, writeSize);

    return true;
}

bool tup_bufWriter_writeU8(tup_bufWriter_t* descriptor_p, uint8_t value)
{
    DESCR(descriptor_p, false);

    const size_t writeSize = sizeof(value);
    if (!mayWrite(descr_p, writeSize))
    {
        return false;
    }

    uint8_t* val_p = (uint8_t*)descr_p->cur_p;
    *val_p = value;

    advance(descr_p, writeSize);

    return true;
}

bool tup_bufWriter_writeBuf(tup_bufWriter_t* descriptor_p, const void volatile* value_p, size_t size_bytes)
{
    DESCR(descriptor_p, false);
        
    if (!mayWrite(descr_p, size_bytes))
    {
        return false;
    }

    memcpy(descr_p->cur_p, value_p, size_bytes);
    advance(descr_p, size_bytes);

    return true;
}

static size_t getSize(const descriptor_t* descr_p)
{
    const size_t result = (uintptr_t)descr_p->cur_p - (uintptr_t)descr_p->buf_p;
    return result;
}

static bool mayWrite(const descriptor_t* descr_p, size_t size_bytes)
{
    const size_t size = getSize(descr_p);    
    const size_t remainingSize = descr_p->maxSize_bytes - size;

    return size_bytes <= remainingSize;
}

static void advance(descriptor_t* descr_p, size_t size_bytes)
{
    descr_p->cur_p = (void*)((uintptr_t)descr_p->cur_p + size_bytes);
}

static bool checkDescr(const descriptor_t* descr_p)
{
    //TODO: implement some descriptor checking logic
    return true;
}
