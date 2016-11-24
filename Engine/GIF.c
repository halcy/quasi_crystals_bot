#include "GIF.h"
#include "LZW.h"

static void WriteLZWData(FILE *fh,uint8_t *pixels,int numpixels,int numbits);
static void DrainLZWToFile(FILE *fh,LZWEncoder *lzw,uint8_t buffer[256],bool flush);
static void Write(FILE *fh,void *bytes,size_t length);

void WriteGIFHeader(FILE *fh,int width,int height,Palette palette,int numcolours,int background)
{
	uint8_t flags=0;

	int palettesize=0;
	if(palette && numcolours>0)
	{
		int exp;
		if(numcolours>128) exp=8;
		else if(numcolours>64) exp=7;
		else if(numcolours>32) exp=6;
		else if(numcolours>16) exp=5;
		else if(numcolours>8) exp=4;
		else if(numcolours>4) exp=3;
		else if(numcolours>2) exp=2;
		else exp=1;

		flags|=0x80|0x70|0x00|(exp-1); // GCT present, 8-bit entries, not sorted, number of entries.
		palettesize=1<<exp;
	}

	uint8_t header[]=
	{
		'G','I','F','8','9','a',
		width&0xff,width>>8,
		height&0xff,height>>8,
		flags,
		background,
		0, // Pixel Aspect Ratio
	};

	Write(fh,header,sizeof(header));
	for(int i=0;i<palettesize;i++)
	{
		uint8_t entry[3]={0,0,0};
		if(i<numcolours)
		{
			entry[0]=ExtractRed(palette[i]);
			entry[1]=ExtractGreen(palette[i]);
			entry[2]=ExtractBlue(palette[i]);
		}
		Write(fh,entry,3);
	}
}

void WriteGIFTrailer(FILE *fh)
{
	Write(fh,(uint8_t[1]){0x3b},1);
}

void WriteGIFGraphicControlExtension(FILE *fh,int disposal,int delay,int transparent)
{
	int flags=(disposal<<2)|0x00; // Disposal Method, No User Input
	if(transparent>=0) flags|=0x01;

	uint8_t gce[]=
	{
		0x21,0xf9, // Extension Introducer, Graphic Control Label
		0x04, // Block size
		flags,
		delay&0xff,delay>>8,
		transparent>=0?transparent:0, // Transparent Color Index
		0x00, // Block Terminator
	};

	Write(fh,gce,sizeof(gce));
}

void WriteGIFApplicationExtension(FILE *fh,char name[8],uint8_t authentication[3],uint8_t *content,int length)
{
	uint8_t appext[]=
	{
		0x21,0xff, // Extension Introducer, Application Label
		0x0b, // Block size
		name[0],name[1],name[2],name[3],name[4],name[5],name[6],name[7],
		authentication[0],authentication[1],authentication[2],
	};

	Write(fh,appext,sizeof(appext));

	while(length)
	{
		int blocklen=length;
		if(blocklen>255) blocklen=255;

		Write(fh,(uint8_t[1]){blocklen},1);
		Write(fh,content,blocklen);

		content+=blocklen;
		length-=blocklen;
	}
	Write(fh,(uint8_t[1]){0x00},1);
}

void WriteGIFNetscapeApplicationExtension(FILE *fh,int loopcount)
{
	uint8_t block[]=
	{
		1,
		loopcount>>8,loopcount&0xff,
	};

	WriteGIFApplicationExtension(fh,"NETSCAPE",(uint8_t *)"2.0",block,sizeof(block));
}


void WriteGIFImage(FILE *fh,int x,int y,int width,int height,Palette palette,int numcolours,uint8_t *pixels)
{
	int exp;
	if(numcolours>128) exp=8;
	else if(numcolours>64) exp=7;
	else if(numcolours>32) exp=6;
	else if(numcolours>16) exp=5;
	else if(numcolours>8) exp=4;
	else if(numcolours>4) exp=3;
	else if(numcolours>2) exp=2;
	else exp=1;

	uint8_t flags=0;
	if(palette && numcolours>0)
	{
		flags|=0x80|0x00|0x00|(exp-1); // LCT present, not interlaced, not sorted, number of entries.
	}

	uint8_t header[]=
	{
		0x2c, // Image Separator
		x&0xff,x>>8,
		y&0xff,y>>8,
		width&0xff,width>>8,
		height&0xff,height>>8,
		flags,
	};

	Write(fh,header,sizeof(header));

	int palettesize=1<<exp;
	for(int i=0;i<palettesize;i++)
	{
		uint8_t entry[3]={0,0,0};
		if(i<numcolours)
		{
			entry[0]=ExtractRed(palette[i]);
			entry[1]=ExtractGreen(palette[i]);
			entry[2]=ExtractBlue(palette[i]);
		}
		Write(fh,entry,3);
	}

	// Code lengths are limited to 3 bits or longer.
	if(exp<2) WriteLZWData(fh,pixels,width*height,2);
	else WriteLZWData(fh,pixels,width*height,exp);
}

static void WriteLZWData(FILE *fh,uint8_t *pixels,int numpixels,int numbits)
{
	Write(fh,(uint8_t[1]){numbits},1);

	int numliterals=1<<numbits;
	int clearcode=numliterals;
	int endcode=numliterals+1;

	LZWEncoder *lzw=malloc(sizeof(LZWEncoder));
	InitializeLZWEncoder(lzw,numbits+1,12,numliterals,2,clearcode);

	uint8_t buffer[256]={0};

	for(int i=0;i<numpixels;i++)
	{
		EmitLZWByte(lzw,pixels[i]);
		DrainLZWToFile(fh,lzw,buffer,false);
	}

	EmitLZWEndCode(lzw,endcode);
	DrainLZWToFile(fh,lzw,buffer,true);

	free(lzw);
}

static void DrainLZWToFile(FILE *fh,LZWEncoder *lzw,uint8_t buffer[256],bool flush)
{
	for(;;)
	{
		int byte=NextLZWOutputByte(lzw,flush);
		if(byte<0) break;

		buffer[0]++;
		buffer[buffer[0]]=byte;

		if(buffer[0]==255)
		{
			Write(fh,buffer,256);
			buffer[0]=0;
		}
	}

	if(flush)
	{
		if(buffer[0]!=0) Write(fh,buffer,buffer[0]+1);
		Write(fh,(uint8_t[1]){0x00},1);
		buffer[0]=0;
	}
}

static void Write(FILE *fh,void *bytes,size_t length)
{
	if(fwrite(bytes,1,length,fh)!=length)
	{
		fprintf(stderr,"Error writing to file.\n");
		exit(1);
	}
}
