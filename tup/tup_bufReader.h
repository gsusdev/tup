#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
	const uint8_t privateData[32];
} tup_bufReader_t;

bool tup_bufReader_init(tup_bufReader_t* descriptor_p, const void volatile* buf_p, size_t size_bytes, bool bigEndian);
bool tup_bufReader_reset(tup_bufReader_t* descriptor_p);
size_t tup_bufReader_getSize(const tup_bufReader_t* descriptor_p);
const void volatile* tup_bufReader_getBase(const tup_bufReader_t* descriptor_p);
const void volatile* tup_bufReader_getPtr(const tup_bufReader_t* descriptor_p);
uint32_t tup_bufReader_readU32(tup_bufReader_t* descriptor_p, bool* okFlag_p);
uint16_t tup_bufReader_readU16(tup_bufReader_t* descriptor_p, bool* okFlag_p);
uint8_t tup_bufReader_readU8(tup_bufReader_t* descriptor_p, bool* okFlag_p);
bool tup_bufReader_readBuf(tup_bufReader_t* descriptor_p, void* buf_out_p, size_t readSize_bytes, bool* okFlag_p);
bool tup_bufReader_skip(tup_bufReader_t* descriptor_p, size_t size_bytes, bool* okFlag_p);