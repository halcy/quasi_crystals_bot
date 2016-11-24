#ifndef __QUANTIZE_H__
#define __QUANTIZE_H__

#include "../Graphics/Bitmap.h"

typedef Pixel Palette[256];

int QuantizeBitmap(const Bitmap *bitmap,uint8_t *output,Palette palette, const Bitmap* orig);

#endif
