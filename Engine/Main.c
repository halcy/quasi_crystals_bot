#include "Quantize.h"
#include "Downsample.h"
#include "GIF.h"
#include "../Graphics/Bitmap.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void Configure(float* params, int* paletteIn);
void Initialize(int width,int height);
void Render(Bitmap *screen,float time);

static void RenderGIFAnimation(int width,int height);
static void ParseArguments(int argc,const char **argv);
static int ParseIntArgument(int argc,const char **argv,int *index);
static float ParseFloatArgument(int argc,const char **argv,int *index);
static void PrintUsageAndExit(const char *name);

float Aspect;
float Duration;
int Size;
float FPS;
int Oversampling;
const char *Filename;

int main(int argc,const char **argv)
{
	Aspect=1.0/1.0;
	Duration=1.0;
	Size=256;
	FPS=19;
	Oversampling=1;

	const char *start=strrchr(argv[0],'/');
	if(!start) start=strrchr(argv[0],'\\');
	if(!start) start=argv[0]-1;
	start++;

	const char *end=strrchr(start,'.');
	if(!end) end=start+strlen(start);

	size_t length=end-start;
	char filename[length+5];
	memcpy(filename,start,length);
	strcpy(filename+length,".gif");

	Filename=filename;


	int firstArg = 1;

	float paramsIn[16];
    for(int i = 0; i < 16; i++) {
		paramsIn[i] = atof(argv[firstArg + i]);
	}
	
	int paletteIn[768];
	for(int i = 0; i < 768; i++) {
		paletteIn[i] = atoi(argv[firstArg + i + 16]);
	}

	Configure(paramsIn, paletteIn);
    
	ParseArguments(argc - (256 * 3 + 16), argv + (256 * 3 + 16));
    
	int width,height;
	if(Aspect>1)
	{
		width=Size;
		height=(Size/Aspect+0.5);
	}
	else
	{
		width=(Size*Aspect+0.5);
		height=Size;
	}


	RenderGIFAnimation(width,height);

	return 0;
}

static void RenderGIFAnimation(int width,int height)
{
	fprintf(stderr,"Initializing renderer...\n");
	Initialize(width*Oversampling,height*Oversampling);

	Bitmap *screen=AllocateBitmap(width*Oversampling,height*Oversampling);

	Bitmap *downsampled=NULL;
	if(Oversampling>1) downsampled=AllocateBitmap(width,height);

	uint8_t *quantized=malloc(width*height);

	FILE *fh=fopen(Filename,"wb");
    fprintf(stderr,"Output file %s\n",Filename);
	WriteGIFHeader(fh,width,height,NULL,0,0);
	WriteGIFNetscapeApplicationExtension(fh,0xffff);

	int gifduration=Duration*100+0.5;
	int numframes=Duration*FPS+0.5;

	int lastgiftime=gifduration*(numframes-1)/numframes-gifduration;
	for(int i=0;i<numframes;i++)
	{
		float animtime=(float)i/(float)numframes;
		int giftime=gifduration*i/numframes;

		fprintf(stderr,"Frame %d/%d: Rendering, ",i+1,numframes);
		fflush(stderr);

		Render(screen,animtime);

		Bitmap *result=screen;
		if(Oversampling>1)
		{
			fprintf(stderr,"downsampling, ");
			fflush(stderr);

			DownsampleBitmap(downsampled,screen,Oversampling);
			result=downsampled;
		}

		fprintf(stderr,"quantizing, ");
		fflush(stderr);

		Palette palette;
		int numcolours=QuantizeBitmap(result,quantized,palette,screen);

		WriteGIFGraphicControlExtension(fh,GIFNoDisposal,giftime-lastgiftime,-1);
		WriteGIFImage(fh,0,0,width,height,palette,numcolours,quantized);

		fprintf(stderr,"done.\n");
		fflush(stderr);

		lastgiftime=giftime;
	}

	WriteGIFTrailer(fh);
	fclose(fh);

	FreeBitmap(screen);
	FreeBitmap(downsampled);
	free(quantized);
}

static void ParseArguments(int argc,const char **argv)
{
	const char *filename=NULL;
	for(int i=1;i<argc;i++)
	{
		if(argv[i][0]=='-')
		{
			switch(argv[i][1])
			{
				case 's':
					Size=ParseIntArgument(argc,argv,&i);
				break;

				case 'f':
					FPS=ParseFloatArgument(argc,argv,&i);
				break;

				case 'o':
					Oversampling=ParseIntArgument(argc,argv,&i);
				break;

				default:
					PrintUsageAndExit(argv[0]);
				break;
			}
		}
		else
		{
			if(!filename) filename=argv[i];
			else PrintUsageAndExit(argv[0]);
		}
	}
	if(filename) Filename=filename;
}

static int ParseIntArgument(int argc,const char **argv,int *index)
{
	if(argv[*index][2]!=0)
	{
		return atoi(&argv[*index][2]);
	}
	else
	{
		(*index)++;
		if(*index>=argc) PrintUsageAndExit(argv[0]);
		return atoi(argv[*index]);
	}
}

static float ParseFloatArgument(int argc,const char **argv,int *index)
{
	if(argv[*index][2]!=0)
	{
		return atof(&argv[*index][2]);
	}
	else
	{
		(*index)++;
		if(*index>=argc) PrintUsageAndExit(argv[0]);
		return atof(argv[*index]);
	}
}

static void PrintUsageAndExit(const char *name)
{
	fprintf(stderr,"Usage: %s [options] [filename.gif]\n",name);
	fprintf(stderr,"(Filename defaults to \"%s\" if not explicitly specified.)\n",Filename);	
	fprintf(stderr,"Options:\n");
	fprintf(stderr,"  -s size      Size in pixels. (Default: %d)\n",Size);
	fprintf(stderr,"  -f framerate Framerate in frames per second. (Default: %.1f)\n",FPS);
	fprintf(stderr,"  -o factor    Oversampling factor. (Default: %d)\n",Oversampling);
	fprintf(stderr,"  -h           Print this help message.\n");
	exit(1);
}

void SetAspect(float aspect) { Aspect=aspect; }
void SetDuration(float duration) { Duration=duration; }
void SetDefaultSize(int size) { Size=size; }
void SetDefaultFramerate(float fps) { FPS=fps; }
void SetDefaultOversampling(int factor) { Oversampling=factor; }

