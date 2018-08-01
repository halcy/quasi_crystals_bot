#include "Quantize.h"
#include "Neuquant.h"
#include "../Graphics/Drawing.h"
#include <math.h>
#include <stdio.h>

static void PixelLookup(const void *context,int pixel,int *r,int *g,int *b) {
    const Bitmap *bitmap=context;
    int x=pixel%bitmap->width;
    int y=pixel%bitmap->height;
    Pixel p=ReadPixelNoClip(bitmap,x,y);
    *r=ExtractRed(p);
    *g=ExtractGreen(p);
    *b=ExtractBlue(p);
}

static inline int PixelToIndex(Pixel p) {
    if(PixelAlphaShift==0) return p>>8;
    else return p&0xffffff;
}

static inline Pixel IndexToPixel(int index) {
    if(PixelAlphaShift==0) return (index<<8)|0xff;
    else return index|0xff000000;
}

float H(float q) {
    float value;

    if ( q > 0.008856 ) {
        value = pow ( q, 0.333333 );
        return value;
    }
    else {
        value = 7.787*q + 0.137931;
        return value;
    }
}

static inline const float labDist( int R_value, int G_value, int B_value, int R_value2, int G_value2, int B_value2) {
    float RGB[3];
    float XYZ[3];
    float Lab[3];
    float RGB2[3];
    float XYZ2[3];
    float Lab2[3];
    float adapt[3];
    float value;

    adapt[0] = 0.950467;
    adapt[1] = 1.000000;
    adapt[2] = 1.088969;

    RGB[0] = R_value * 0.003922;
    RGB[1] = G_value * 0.003922;
    RGB[2] = B_value * 0.003922;

    XYZ[0] = 0.412424 * RGB[0] + 0.357579 * RGB[1] + 0.180464 * RGB[2];
    XYZ[1] = 0.212656 * RGB[0] + 0.715158 * RGB[1] + 0.0721856 * RGB[2];
    XYZ[2] = 0.0193324 * RGB[0] + 0.119193 * RGB[1] + 0.950444 * RGB[2];

    Lab[0] = 116 * H( XYZ[1] / adapt[1] ) - 16;
    Lab[1] = 500 * ( H( XYZ[0] / adapt[0] ) - H ( XYZ[1] / adapt[1] ) );
    Lab[2] = 200 * ( H( XYZ[1] / adapt[1] ) - H ( XYZ[2] / adapt[2] ) );

    RGB2[0] = R_value2 * 0.003922;
    RGB2[1] = G_value2 * 0.003922;
    RGB2[2] = B_value2 * 0.003922;

    XYZ2[0] = 0.412424 * RGB2[0] + 0.357579 * RGB2[1] + 0.180464 * RGB2[2];
    XYZ2[1] = 0.212656 * RGB2[0] + 0.715158 * RGB2[1] + 0.0721856 * RGB2[2];
    XYZ2[2] = 0.0193324 * RGB2[0] + 0.119193 * RGB2[1] + 0.950444 * RGB2[2];

    Lab2[0] = 116 * H( XYZ2[1] / adapt[1] ) - 16;
    Lab2[1] = 500 * ( H( XYZ2[0] / adapt[0] ) - H ( XYZ2[1] / adapt[1] ) );
    Lab2[2] = 200 * ( H( XYZ2[1] / adapt[1] ) - H ( XYZ2[2] / adapt[2] ) );

    value = pow( (Lab[0] - Lab2[0]), 2 ) + pow( (Lab[1] - Lab2[1]), 2 ) + pow( (Lab[2] - Lab2[2]), 2 );

    return value;
}

static inline const float PixelDist(Pixel a, Pixel b) {    
//     return(labDist(ExtractRed(a), ExtractGreen(a), ExtractBlue(a), ExtractRed(b), ExtractGreen(b), ExtractBlue(b)));
        
    int dr = ExtractRed(a) - ExtractRed(b);
    int dg = ExtractGreen(a) - ExtractGreen(b);
    int db = ExtractBlue(a) - ExtractBlue(b);
    
    return(sqrt(dr*dr+dg*dg+db*db));
}

int PixelComp(const void* a, const void* b) {
    return(ExtractRed(*(Pixel*)a) + ExtractGreen(*(Pixel*)a) + ExtractBlue(*(Pixel*)a) - ExtractRed(*(Pixel*)b) - ExtractGreen(*(Pixel*)b) - ExtractBlue(*(Pixel*)b));
}

int QuantizeBitmap(const Bitmap *bitmap, uint8_t *output, Palette palette, const Bitmap* orig) {
    uint32_t *colourarray=calloc(256*256*256*4,1);
    uint32_t *colourarrayorig=calloc(256*256*256*4,1);
    uint32_t *colourcount=calloc(256*256*256*sizeof(int),1);
    int numcolours=0;

    Pixel* colourlist=(Pixel*)calloc(256*256*256*sizeof(Pixel),1);

    Pixel brightest = RGB(0, 0, 0);
    Pixel darkest = RGB(255, 255, 255);

    int palcount = 0;
    for(int y=0;y<orig->height;y++)
    for(int x=0;x<orig->width;x++)
    {
        Pixel p=ReadPixelNoClip(orig,x,y);
        int index=PixelToIndex(p);
        if(colourarrayorig[index] == 0) {
            palette[palcount] = p;
            palcount += 1;
        }
        colourarrayorig[index]++;
    }

    int palInitialSize = palcount;
    if(palcount > 150) {
        palInitialSize = 0;
    }
    for(int y=0;y<bitmap->height;y++)
    for(int x=0;x<bitmap->width;x++)
    {
        Pixel p=ReadPixelNoClip(bitmap,x,y);
        int index=PixelToIndex(p);
        colourcount[index]++;
        if(colourarray[index]==0) {
            colourlist[numcolours] = p;
            if(palcount > 150) {            
                if(colourarrayorig[index] != 0) {
                    // Can't be more than 256 colours in the orig image, so this is fine
                    palette[palInitialSize] = p;
                    palInitialSize++;
                }
            }
            numcolours++;
        }
        colourarray[index]++;
    }
        
    qsort(colourlist, numcolours, sizeof(Pixel), &PixelComp);

    int bestCountBright = 0;
    int bestCountDark = 0;
    for(int i = 0; i < numcolours / 25; i++) {
        if(colourcount[PixelToIndex(colourlist[i])] > bestCountBright) {
            bestCountBright = colourcount[PixelToIndex(colourlist[i])];
            brightest = colourlist[i];
        }

        if(colourcount[PixelToIndex(colourlist[numcolours - i - 1])] > bestCountDark) {
            bestCountDark = colourcount[PixelToIndex(colourlist[numcolours - i - 1])];
            darkest = colourlist[numcolours - i - 1];
        }
    }

    if(numcolours<=256)
    {
        int n=0;
        for(int i=0;i<256*256*256;i++)
        {
            if(colourarray[i])
            {
                palette[n]=IndexToPixel(i);
                colourarray[i]=n;
                n++;
            }
        }

        uint8_t *ptr=output;
        for(int y=0;y<bitmap->height;y++)
        for(int x=0;x<bitmap->width;x++)
        {
            Pixel p=ReadPixelNoClip(bitmap,x,y);
            int index=PixelToIndex(p);
            *ptr++=colourarray[index];
        }
    }
    else
    {
        palette[palInitialSize] = brightest;
        if(palInitialSize < 255) {
            palette[palInitialSize + 1] = darkest;
        }
        
        int index = palInitialSize + 2;
        float bestDiff = -1;
        
        // Terminates if index too big anyways, no need for more checks
        while(index < 256) {
            for(int col = 0; col < numcolours; col++) {
                Pixel p = colourlist[col];
                
                int found = 0;
                for(int i = 0; i < index; i++) {
                    if(palette[i] == p) {
                        found = 1;
                        break;
                    }
                }
                if(found == 1) {
                    continue;
                }
                
                float colDiff = 0;
                
                for(int i = 0; i < index; i++) {
                    colDiff += PixelDist(palette[i], p);
                }
                
                if(colDiff > bestDiff) {
                    palette[index] = p;
                    bestDiff = colDiff;
                }
            }
            index += 1;
        }
        
        for(int i=0;i<256*256*256;i++)
        {
            if(colourarray[i])
            {
                Pixel p=IndexToPixel(i);
                float bestDiff = PixelDist(palette[0], p);
                colourarray[i] = 0;
                for(int j = 1; j < 256; j++) {
                    float dist = PixelDist(palette[j], p);
                    if(dist < bestDiff) {
                        bestDiff = dist;
                        colourarray[i] = j;
                    }
                    
                }
            }
        }
        
        uint8_t *ptr=output;
        for(int y=0;y<bitmap->height;y++)
        for(int x=0;x<bitmap->width;x++)
        {
            Pixel p=ReadPixelNoClip(bitmap,x,y);
            int index=PixelToIndex(p);
            *ptr++=colourarray[index];
            //*ptr++=SearchNeuquantIndex(&quant,ExtractRed(p),ExtractGreen(p),ExtractBlue(p));
        }

        numcolours=256;
        
// 		Neuquant quant;
// 		InitializeNeuquantNetwork(&quant);
// 		NeuquantLearn(&quant,bitmap,PixelLookup,bitmap->width*bitmap->height,1);
// 		UnbiasNeuquantNetwork(&quant);
// 
// 		for(int i=0;i<256;i++)
// 		{
// 			int r,g,b;
// 			GetNeuquantPaletteEntry(&quant,i,&r,&g,&b);
// 			palette[i]=RGB(r,g,b);
// 		}
// 
// 		BuildNeuquantIndex(&quant);
// 
// 		for(int i=0;i<256*256*256;i++)
// 		{
// 			if(colourarray[i])
// 			{
// 				Pixel p=IndexToPixel(i);
// 				colourarray[i]=SearchNeuquantIndex(&quant,ExtractRed(p),ExtractGreen(p),ExtractBlue(p));
// 			}
// 		}
// 
// 		uint8_t *ptr=output;
// 		for(int y=0;y<bitmap->height;y++)
// 		for(int x=0;x<bitmap->width;x++)
// 		{
// 			Pixel p=ReadPixelNoClip(bitmap,x,y);
// 			int index=PixelToIndex(p);
// 			*ptr++=colourarray[index];
// 			//*ptr++=SearchNeuquantIndex(&quant,ExtractRed(p),ExtractGreen(p),ExtractBlue(p));
// 		}
// 
// 		numcolours=256;
        
        
    }

    free(colourlist);
    free(colourcount);
    free(colourarrayorig);
    free(colourarray);
    return 256;
}
