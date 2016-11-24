#include "LZW.h"

void InitializeLZWEncoder(LZWEncoder *self,int minimumcodelength,
int maximumcodelength,int numliterals,int numreservedcodes,int resetcode)
{
	self->bits=0;
	self->numbits=0;

	self->code=-1;

	self->initialnumcodes=numliterals+numreservedcodes;
	self->initialcodelength=minimumcodelength;
	self->maximumcodelength=maximumcodelength;
	self->resetcode=resetcode;

	ResetLZWEncoder(self);
}

void ResetLZWEncoder(LZWEncoder *self)
{
	self->numcodes=self->initialnumcodes;
	self->codelength=self->initialcodelength;
	memset(self->codes,0,sizeof(self->codes[0])*(1<<self->maximumcodelength));
}

void EmitLZWByte(LZWEncoder *self,uint8_t byte)
{
	if(self->code<0)
	{
		self->code=byte;
		return;
	}

	uint16_t newcode=self->codes[self->code].next[byte];
	if(newcode)
	{
		self->code=newcode;
	}
	else
	{
		EmitLZWCode(self,self->code);
		self->codes[self->code].next[byte]=self->numcodes++;
		self->code=byte;

		if(self->numcodes>(1<<self->codelength))
		{
			if(self->codelength<self->maximumcodelength)
			{
				self->codelength++;
			}
			else
			{
				EmitLZWCode(self,self->resetcode);
				ResetLZWEncoder(self);
			}
		}
	}
}

void EmitLZWEndCode(LZWEncoder *self,int endcode)
{
	EmitLZWCode(self,self->code);
	if(endcode) EmitLZWCode(self,endcode);
}

void EmitLZWCode(LZWEncoder *self,int code)
{
	self->bits|=((uint64_t)code)<<self->numbits;
	self->numbits+=self->codelength;
}

int NextLZWOutputByte(LZWEncoder *self,bool flush)
{
	if(self->numbits>=8)
	{
		int output=self->bits&0xff;
		self->bits>>=8;
		self->numbits-=8;
		return output;
	}
	else if(flush && self->numbits>0)
	{
		int output=self->bits&0xff;
		self->bits=0;
		self->numbits=0;
		return output;
	}
	else
	{
		return -1;
	}
}
