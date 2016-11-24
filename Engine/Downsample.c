#include "Downsample.h"

#include "../Graphics/Drawing.h"
#include <math.h>

#define Gamma 2.2

void DownsampleBitmap(Bitmap *dest,const Bitmap *src,int factor)
{
	float gammatable[256];
	for(int i=0;i<256;i++) gammatable[i]=powf((float)i/255.0,Gamma);

	for(int y=0;y<dest->height;y++)
	for(int x=0;x<dest->width;x++)
	{
		float r=0,g=0,b=0;
		for(int dy=0;dy<factor;dy++)
		for(int dx=0;dx<factor;dx++)
		{
			Pixel p=ReadPixelNoClip(src,x*factor+dx,y*factor+dy);
			r+=gammatable[ExtractRed(p)];
			g+=gammatable[ExtractGreen(p)];
			b+=gammatable[ExtractBlue(p)];
		}

		DrawPixelNoClip(dest,x,y,RGB(
			powf(r/(float)(factor*factor),1.0/Gamma)*255,
			powf(g/(float)(factor*factor),1.0/Gamma)*255,
			powf(b/(float)(factor*factor),1.0/Gamma)*255
		));
	}
}
