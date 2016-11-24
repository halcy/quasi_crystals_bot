#ifndef __GIF_H__
#define __GIF_H__

#include "Quantize.h"
#include <stdio.h>

#define GIFUndefinedDisposal 0x00
#define GIFNoDisposal 0x01
#define GIFRestoreToBackgroundDisposal 0x02
#define GIFRestoreToPreviousDisposal 0x02

void WriteGIFHeader(FILE *fh,int width,int height,Palette palette,int numcolours,int background);
void WriteGIFTrailer(FILE *fh);
void WriteGIFGraphicControlExtension(FILE *fh,int disposal,int delay,int transparent);
void WriteGIFApplicationExtension(FILE *fh,char name[8],uint8_t authentication[3],uint8_t *content,int length);
void WriteGIFNetscapeApplicationExtension(FILE *fh,int loopcount);
void WriteGIFImage(FILE *fh,int x,int y,int width,int height,Palette palette,int numcolours,uint8_t *pixels);

#endif
