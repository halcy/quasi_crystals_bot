#ifndef __LZW_H__
#define __LZW_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define LZWMaximumCodeLength 12
#define LZWMaximumNumberOfCodes (1<<LZWMaximumCodeLength)

typedef struct LZWEncoder
{
	uint64_t bits;
	int numbits;

	int code;

	int initialnumcodes,numcodes;
	int initialcodelength,codelength;
	int maximumcodelength;
	int resetcode;

	struct
	{
		uint16_t next[256];
	} codes[LZWMaximumNumberOfCodes];
} LZWEncoder;

void InitializeLZWEncoder(LZWEncoder *self,int minimumcodelength,
int maximumcodelength,int numliterals,int numreservedcodes,int resetcode);
void ResetLZWEncoder(LZWEncoder *self);
void EmitLZWByte(LZWEncoder *self,uint8_t byte);
void EmitLZWEndCode(LZWEncoder *self,int endcode);
void EmitLZWCode(LZWEncoder *self,int code);
int NextLZWOutputByte(LZWEncoder *self,bool flush);

#endif
