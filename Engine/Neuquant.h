/* NeuQuant Neural-Net Quantization Algorithm Interface
 * ----------------------------------------------------
 *
 * Copyright (c) 1994 Anthony Dekker
 *
 * NEUQUANT Neural-Net quantization algorithm by Anthony Dekker, 1994.
 * See "Kohonen neural networks for optimal colour quantization"
 * in "Network: Computation in Neural Systems" Vol. 5 (1994) pp 351-367.
 * for a discussion of the algorithm.
 * See also  http://members.ozemail.com.au/~dekker/NEUQUANT.HTML
 *
 * Any party obtaining a copy of these files from the author, directly or
 * indirectly, is granted, free of charge, a full and unrestricted irrevocable,
 * world-wide, paid up, royalty-free, nonexclusive right and license to deal
 * in this software and documentation files (the "Software"), including without
 * limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons who receive
 * copies from any such party to do so, with the only requirement being
 * that this copyright notice remain intact.
 */

#ifndef __NEUQUANT_H__
#define __NEUQUANT_H__

#define NeuquantNetworkSize 256 // Number of colours used
#define NeuquantRadiusSize (NeuquantNetworkSize>>3)

typedef struct Neuquant
{
	int network[NeuquantNetworkSize][4];
	int netindex[256];
	int bias[NeuquantNetworkSize];
	int freq[NeuquantNetworkSize];
	int radpower[NeuquantRadiusSize];
} Neuquant;

typedef void NeuquantLookupFunction(const void *data,int pixel,int *r,int *g,int *b);

void InitializeNeuquantNetwork(Neuquant *self);
void UnbiasNeuquantNetwork(Neuquant *self);
void GetNeuquantPaletteEntry(Neuquant *self,int n,int *r,int *g,int *b);
void BuildNeuquantIndex(Neuquant *self);
int SearchNeuquantIndex(Neuquant *self,int r,int g,int b);
void NeuquantLearn(Neuquant *self,const void *data,NeuquantLookupFunction *func,int numpixels,int samplefac);

#endif
