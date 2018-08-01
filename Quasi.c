#include "All.h"

#define RGBi(r,g,b) (RGBf(((float)(r))/255.0f,((float)(g))/255.0f,((float)(b))/255.0f))
Pixel palette[256];
float a;
float b;
float size = 0.0;
float params[16];

void Configure(float* paramsIn, int* paletteIn)
{
    SetDuration(3.0);
    SetAspect(1.0 / 1.0);
    for(int i = 0; i < 256; i++) {
        palette[i] = RGBi(paletteIn[i * 3], paletteIn[i * 3 + 1], paletteIn[i * 3 + 2]);
    }
    
    for(int i = 0; i < 16; i++) {
        params[i] = paramsIn[i];
    }
    printf("Lin %f / Pol %f / OfLin %f / OfPol %f\n", params[0], params[1], params[2], params[3]);
    printf("Pal entry 0: %d %d %d\n", paletteIn[0], paletteIn[1], paletteIn[2]);
    printf("Pal entry 255: %d %d %d\n", paletteIn[768 - 3], paletteIn[768 - 2], paletteIn[768 - 1]);
}

void Initialize(int width,int height)
{
 //   palette[0] = RGBi(102,26,26);
 //   palette[1] = RGBi(128,72,32);
 //   palette[2] = RGBi(222,116,40);
 //   palette[3] = RGBi(115,80,59);
 //   palette[4] = RGBi(82,56,41);
    size = width;
}

float quasi(vec2_t pos, const float phase) {
    float theta = atan2(-pos.y,pos.x);
    float r = log(vec2abs(pos));
    float C=0.0f;
    float lin = params[0];
    float pol = params[1];
    float outflow_lin = params[2];
    float outflow_pol = params[3];
    
    float planewaves = params[4];
    float stripes = params[5];
    float phasespeed = params[6];
    //printf("pol pw %f / str %f / ps %f\n", planewaves, stripes, phasespeed);
    if(pol != 0.0) {
        for(float t=0.0f; t<planewaves; t++) {
            C += pol * cos((theta * cos(t * (M_PI / planewaves)) - r * sin(t * (M_PI / planewaves))) * stripes + phase * phasespeed);
        }
    }
    
    planewaves = params[7];
    stripes = params[8];
    phasespeed = params[9];
    //printf("lin pw %f / str %f / ps %f\n", planewaves, stripes, phasespeed);
    if(lin != 0.0) {
        for(float t=0.0f; t<planewaves; t++) {        
            C += lin * cos((pos.x * cos(t * (M_PI / planewaves)) + pos.y * sin(t * (M_PI / planewaves))) * 2.0f * M_PI * stripes + phase * phasespeed);
        }
    }
    
    planewaves = params[10];
    stripes = params[11];
    phasespeed = params[12];
    //printf("of pol pw %f / str %f / ps %f\n", planewaves, stripes, phasespeed);
    if(outflow_pol != 0.0) {
        for(float t=0.0f; t<planewaves; t++) {
            C += outflow_pol * cos(fabs(theta * cos(t * (M_PI / planewaves)) - r * sin(t * (M_PI / planewaves))) * stripes + phase * phasespeed);
        }
    }
    
    planewaves = params[13];
    stripes = params[14];
    phasespeed = params[15];
    //printf("of lin pw %f / str %f / ps %f\n", planewaves, stripes, phasespeed);
    if(outflow_lin != 0.0) {
        for(float t=0.0f; t<planewaves; t++) {        
            C += outflow_lin * cos(fabs(pos.x * cos(t * (M_PI / planewaves)) + pos.y * sin(t * (M_PI / planewaves))) * 2.0f * M_PI * stripes + phase * phasespeed);
        }
    }
    float c=((C+planewaves)/(planewaves*2.0f));
    return c;
}

Pixel PixelLoop(float x, float y, float time)
{
    float crystal = quasi(vec2(x, y), time * 2.0f * M_PI);
    int crystal_quant = (int)(crystal*255.0);

    crystal_quant = crystal_quant < 0 ? 0 : crystal_quant;
    crystal_quant = crystal_quant > 255 ? 255 : crystal_quant;

    return(palette[crystal_quant]);
}

void Render(Bitmap *screen,float time) { RunPixelLoop(screen,time,PixelLoop); }

/* Decisions:
 * - same param / diff param
 * - pure / mixed
 *     - mixed: mixmode type / flow
 *     - pure: normal / flow
 * - directions for all
 * - params a 1-15 weighted low, b 1-60 weighted low
 * - palette full / threshold
 *     - if threshold, pick one
 * - palette size (2 to 256, weighted low)
 * - which palette (TODO add stripey)
 * - hue rotation (0 to 360)
 * - component order
 */
