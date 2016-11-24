#ifndef __HELPERS_H__
#define __HELPERS_H__

#include "../Graphics/Bitmap.h"
#include "../Graphics/Drawing.h"

#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif

#define PI M_PI




typedef Pixel PixelLoopFunction(float x,float y,float time);

static inline float ScreenXToPixelLoop(Bitmap *screen,int sx)
{
	if(screen->width>screen->height) return (float)(2*sx-screen->width+1)/(float)screen->width;
	else return (float)(2*sx-screen->width+1)/(float)screen->height;
}

static inline float ScreenYToPixelLoop(Bitmap *screen,int sy)
{
	if(screen->width>screen->height) return -(float)(2*sy-screen->height+1)/(float)screen->width;
	else return -(float)(2*sy-screen->height+1)/(float)screen->height;
}

static inline int PixelLoopXToScreen(Bitmap *screen,float x)
{
	if(screen->width>screen->height) return (x*screen->width+screen->width-1)/2.0;
	else return (x*screen->height+screen->width-1)/2.0;
}

static inline int PixelLoopYToScreen(Bitmap *screen,float y)
{
	if(screen->width>screen->height) return (-y*screen->width+screen->height-1)/2.0;
	else return (-y*screen->height+screen->height-1)/2.0;
}

static inline void RunPixelLoop(Bitmap *screen,float time,PixelLoopFunction *func)
{
	for(int sy=0;sy<screen->height;sy++)
	for(int sx=0;sx<screen->width;sx++)
	{
		float x=ScreenXToPixelLoop(screen,sx);
		float y=ScreenYToPixelLoop(screen,sy);

		DrawPixelNoClip(screen,sx,sy,func(x,y,time));
	}
}




static inline Pixel RGBf(float r,float g,float b) { return RGB(r*255,g*255,b*255); }
static inline Pixel RGBAf(float r,float g,float b,float a) { return RGBA(r*255,g*255,b*255,a*255); }
static inline Pixel RGBvec3(vec3_t rgb) { return RGBf(rgb.x,rgb.y,rgb.z); }
static inline Pixel RGBAvec4(vec4_t rgba) { return RGBAf(rgba.x,rgba.y,rgba.z,rgba.w); }

static inline Pixel HSVf(float h,float s,float v)
{
	h-=floorf(h);
	h*=6;

	float f=h-floorf(h);
	float p=v*(1-s);
	float q=v*(1-f*s);
	float t=v*(1-s+f*s);

	switch((int)h)
	{
		default:
		case 0: return RGBf(v,t,p);
		case 1: return RGBf(q,v,p);
		case 2: return RGBf(p,v,t);
		case 3: return RGBf(p,q,v);
		case 4: return RGBf(t,p,v);
		case 5: return RGBf(v,p,q);
	}
}

static inline Pixel HSLf(float h,float s,float l)
{
	if(l==0) return RGB(0,0,0);

	float tmp;
	if(l<=0.5) tmp=s*l;
	else tmp=s*(1-l);

	return HSVf(h,2*tmp/(l+tmp),l+tmp);
}



static inline void FindSamplePosition(const Bitmap *bitmap,float u,float v,
int *x,int *y,float *xblend,float *yblend);
static inline vec4_t SampleAndBlendPixels(const Bitmap *bitmap,int x1,int y1,
int x2,int y2,float xblend,float yblend);
static inline vec4_t SampleAndBlendPixelsNoClip(const Bitmap *bitmap,int x1,int y1,
int x2,int y2,float xblend,float yblend);
static inline vec4_t BlendSampledPixels(Pixel p1,Pixel p2,Pixel p3,Pixel p4,float xblend,float yblend);
static inline float BlendSampledComponents(int c1,int c2,int c3,int c4,float xblend,float yblend);
static inline int ClampBitmapCoordinate(int x,int max);

static inline vec4_t SampleBitmap(const Bitmap *bitmap,vec2_t pos)
{
	int x1,y1;
	float xblend,yblend;
	FindSamplePosition(bitmap,pos.x,pos.y,&x1,&y1,&xblend,&yblend);

	int x2=x1+1;
	int y2=y1+1;

	return SampleAndBlendPixels(bitmap,x1,y1,x2,y2,xblend,yblend);
}

static inline vec4_t SampleTiledBitmap(const Bitmap *bitmap,vec2_t pos)
{
	int x1,y1;
	float xblend,yblend;
	FindSamplePosition(bitmap,pos.x-floorf(pos.x),pos.y-floorf(pos.y),&x1,&y1,&xblend,&yblend);

	if(x1<0) x1+=bitmap->width;
	if(y1<0) y1+=bitmap->height;
	int x2=(x1+1)%bitmap->width;
	int y2=(y1+1)%bitmap->height;

	return SampleAndBlendPixelsNoClip(bitmap,x1,y1,x2,y2,xblend,yblend);
}

static inline vec4_t SampleClampedBitmap(const Bitmap *bitmap,vec2_t pos)
{
	int x1,y1;
	float xblend,yblend;
	FindSamplePosition(bitmap,pos.x,pos.y,&x1,&y1,&xblend,&yblend);

	int x2=x1+1;
	int y2=y1+1;
	x1=ClampBitmapCoordinate(x1,bitmap->width);
	y1=ClampBitmapCoordinate(y1,bitmap->height);
	x2=ClampBitmapCoordinate(x2,bitmap->width);
	y2=ClampBitmapCoordinate(y2,bitmap->height);

	return SampleAndBlendPixelsNoClip(bitmap,x1,y1,x2,y2,xblend,yblend);
}

static inline void FindSamplePosition(const Bitmap *bitmap,float u,float v,
int *x,int *y,float *xblend,float *yblend)
{
	u=u*bitmap->width-0.5;
	v=v*bitmap->height-0.5;
	float ufloor=floorf(u);
	float vfloor=floorf(v);
	*x=(int)ufloor;
	*y=(int)vfloor;
	*xblend=u-ufloor;
	*yblend=v-vfloor;
}

static inline vec4_t SampleAndBlendPixels(const Bitmap *bitmap,int x1,int y1,
int x2,int y2,float xblend,float yblend)
{
	Pixel p1=ReadPixel(bitmap,x1,y1);
	Pixel p2=ReadPixel(bitmap,x2,y1);
	Pixel p3=ReadPixel(bitmap,x1,y2);
	Pixel p4=ReadPixel(bitmap,x2,y2);
	return BlendSampledPixels(p1,p2,p3,p4,xblend,yblend);
}

static inline vec4_t SampleAndBlendPixelsNoClip(const Bitmap *bitmap,int x1,int y1,
int x2,int y2,float xblend,float yblend)
{
	Pixel p1=ReadPixelNoClip(bitmap,x1,y1);
	Pixel p2=ReadPixelNoClip(bitmap,x2,y1);
	Pixel p3=ReadPixelNoClip(bitmap,x1,y2);
	Pixel p4=ReadPixelNoClip(bitmap,x2,y2);
	return BlendSampledPixels(p1,p2,p3,p4,xblend,yblend);
}

static inline vec4_t BlendSampledPixels(Pixel p1,Pixel p2,Pixel p3,Pixel p4,float xblend,float yblend)
{
	return vec4(
		BlendSampledComponents(ExtractRed(p1),ExtractRed(p2),ExtractRed(p3),ExtractRed(p4),xblend,yblend),
		BlendSampledComponents(ExtractGreen(p1),ExtractGreen(p2),ExtractGreen(p3),ExtractGreen(p4),xblend,yblend),
		BlendSampledComponents(ExtractBlue(p1),ExtractBlue(p2),ExtractBlue(p3),ExtractBlue(p4),xblend,yblend),
		BlendSampledComponents(ExtractAlpha(p1),ExtractAlpha(p2),ExtractAlpha(p3),ExtractAlpha(p4),xblend,yblend)
	);
}

static inline float BlendSampledComponents(int c1,int c2,int c3,int c4,float xblend,float yblend)
{
	float blend1=(float)c1/255.0+(float)(c2-c1)/255.0*xblend;
	float blend2=(float)c3/255.0+(float)(c4-c3)/255.0*xblend;
	return blend1+(blend2-blend1)*yblend;
}

static inline int ClampBitmapCoordinate(int x,int max)
{
	if(x<0) return 0;
	if(x>=max) return max-1;
	return x;
}

#endif
