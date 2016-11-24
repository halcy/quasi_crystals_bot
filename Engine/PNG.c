#include "PNG.h"
#include "../Graphics/Tools/PNG.h"

#include <stdio.h>

static void *PixelFunc(void *destination,uint8_t r,uint8_t g,uint8_t b,uint8_t a,int x,int y)
{
	Pixel *dest=destination;
	*dest=RGBA(r,g,b,a);
	return dest+1;
}

Bitmap *LoadPNG(const char *filename)
{
	FILE *fh=fopen(filename,"rb");
	if(!fh)
	{
		fprintf(stderr,"Could not open file \"%s\".\n",filename);
		exit(1);
	}

	fseek(fh,0,SEEK_END);
	long size=ftell(fh);
	fseek(fh,0,SEEK_SET);

	void *bytes=malloc(size);
	if(!bytes)
	{
		fprintf(stderr,"Out of memory.\n");
		exit(1);
	}

	if(fread(bytes,1,size,fh)!=size)
	{
		fprintf(stderr,"Reading from file \"%s\" failed.\n",filename);
		exit(1);
	}

	fclose(fh);

	PNGLoader loader;
	InitializePNGLoader(&loader,bytes,size);

	if(!LoadPNGHeader(&loader))
	{
		fprintf(stderr,"File \"%s\" is not a valid PNG file.\n",filename);
		exit(1);
	}

	Bitmap *bitmap=AllocateBitmap(loader.width,loader.height);
	if(!bitmap)
	{
		fprintf(stderr,"Out of memory.\n");
		exit(1);
	}

	if(!LoadPNGImageData(&loader,bitmap->pixels,bitmap->bytesperrow,PixelFunc))
	{
		fprintf(stderr,"Failed while loading PNG data from file \"%s\".\n",filename);
		exit(1);
	}

	free(bytes);

	return bitmap;
}

