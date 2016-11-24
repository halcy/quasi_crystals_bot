/* NeuQuant Neural-Net Quantization Algorithm
 * ------------------------------------------
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


#include "Neuquant.h"

/* four primes near 500 - assume no image has a length so large */
/* that it is divisible by all four primes */
#define prime1		499
#define prime2		491
#define prime3		487
#define prime4		503

#define maxnetpos	(NeuquantNetworkSize-1)
#define netbiasshift	4			/* bias for colour values */
#define ncycles		100			/* no. of learning cycles */

/* defs for freq and bias */
#define intbiasshift    16			/* bias for fractions */
#define intbias		(((int) 1)<<intbiasshift)
#define gammashift  	10			/* gamma = 1024 */
#define gamma   	(((int) 1)<<gammashift)
#define betashift  	10
#define beta		(intbias>>betashift)	/* beta = 1/1024 */
#define betagamma	(intbias<<(gammashift-betashift))

/* defs for decreasing radius factor */
#define radiusbiasshift	6			/* at 32.0 biased by 6 bits */
#define radiusbias	(((int) 1)<<radiusbiasshift)
#define initradius	(NeuquantRadiusSize*radiusbias)	/* and decreases by a */
#define radiusdec	30			/* factor of 1/30 each cycle */ 

/* defs for decreasing alpha factor */
#define alphabiasshift	10			/* alpha starts at 1.0 */
#define initalpha	(((int) 1)<<alphabiasshift)
int alphadec;					/* biased by 10 bits */

/* radbias and alpharadbias used for radpower calculation */
#define radbiasshift	8
#define radbias		(((int) 1)<<radbiasshift)
#define alpharadbshift  (alphabiasshift+radbiasshift)
#define alpharadbias    (((int) 1)<<alpharadbshift)

void InitializeNeuquantNetwork(Neuquant *self)
{
	for(int i=0;i<NeuquantNetworkSize;i++)
	{
		self->network[i][0]=self->network[i][1]=self->network[i][2]=(i<<(netbiasshift+8))/NeuquantNetworkSize;
		self->freq[i]=intbias/NeuquantNetworkSize;
		self->bias[i]=0;
	}
}

void UnbiasNeuquantNetwork(Neuquant *self)
{
	int i,j,temp;

	for (i=0; i<NeuquantNetworkSize; i++) {
		for (j=0; j<3; j++) {
			/* OLD CODE: network[i][j] >>= netbiasshift; */
			/* Fix based on bug report by Juergen Weigert jw@suse.de */
			temp = (self->network[i][j] + (1 << (netbiasshift - 1))) >> netbiasshift;
			if (temp > 255) temp = 255;
			self->network[i][j] = temp;
		}
		self->network[i][3] = i;			/* record colour no */
	}
}

void GetNeuquantPaletteEntry(Neuquant *self,int n,int *r,int *g,int *b)
{
	if(b) *b=self->network[n][0];
	if(g) *g=self->network[n][1];
	if(r) *r=self->network[n][2];
}

void BuildNeuquantIndex(Neuquant *self)
{
	int i,j,smallpos,smallval;
	int *p,*q;
	int previouscol,startpos;

	previouscol = 0;
	startpos = 0;
	for (i=0; i<NeuquantNetworkSize; i++) {
		p = self->network[i];
		smallpos = i;
		smallval = p[1];			/* index on g */
		/* find smallest in i..NeuquantNetworkSize-1 */
		for (j=i+1; j<NeuquantNetworkSize; j++) {
			q = self->network[j];
			if (q[1] < smallval) {		/* index on g */
				smallpos = j;
				smallval = q[1];	/* index on g */
			}
		}
		q = self->network[smallpos];
		/* swap p (i) and q (smallpos) entries */
		if (i != smallpos) {
			j = q[0];   q[0] = p[0];   p[0] = j;
			j = q[1];   q[1] = p[1];   p[1] = j;
			j = q[2];   q[2] = p[2];   p[2] = j;
			j = q[3];   q[3] = p[3];   p[3] = j;
		}
		/* smallval entry is now in position i */
		if (smallval != previouscol) {
			self->netindex[previouscol] = (startpos+i)>>1;
			for (j=previouscol+1; j<smallval; j++) self->netindex[j] = i;
			previouscol = smallval;
			startpos = i;
		}
	}
	self->netindex[previouscol] = (startpos+maxnetpos)>>1;
	for (j=previouscol+1; j<256; j++) self->netindex[j] = maxnetpos; /* really 256 */
}

int SearchNeuquantIndex(Neuquant *self,int r,int g,int b)
{
	int i,j,dist,a,bestd;
	int *p;
	int best;

	bestd = 1000;		/* biggest possible dist is 256*3 */
	best = -1;
	i = self->netindex[g];	/* index on g */
	j = i-1;		/* start at netindex[g] and work outwards */

	while ((i<NeuquantNetworkSize) || (j>=0)) {
		if (i<NeuquantNetworkSize) {
			p = self->network[i];
			dist = p[1] - g;		/* inx key */
			if (dist >= bestd) i = NeuquantNetworkSize;	/* stop iter */
			else {
				i++;
				if (dist<0) dist = -dist;
				a = p[0] - b;   if (a<0) a = -a;
				dist += a;
				if (dist<bestd) {
					a = p[2] - r;   if (a<0) a = -a;
					dist += a;
					if (dist<bestd) {bestd=dist; best=p[3];}
				}
			}
		}
		if (j>=0) {
			p = self->network[j];
			dist = g - p[1]; /* inx key - reverse dif */
			if (dist >= bestd) j = -1; /* stop iter */
			else {
				j--;
				if (dist<0) dist = -dist;
				a = p[0] - b;   if (a<0) a = -a;
				dist += a;
				if (dist<bestd) {
					a = p[2] - r;   if (a<0) a = -a;
					dist += a;
					if (dist<bestd) {bestd=dist; best=p[3];}
				}
			}
		}
	}
	return(best);
}


/* Search for biased BGR values
   ---------------------------- */

static int contest(Neuquant *self,int b,int g,int r)
{
	/* finds closest neuron (min dist) and updates freq */
	/* finds best neuron (min dist-bias) and returns position */
	/* for frequently chosen neurons, freq[i] is high and bias[i] is negative */
	/* bias[i] = gamma*((1/NeuquantNetworkSize)-freq[i]) */

	int i,dist,a,biasdist,betafreq;
	int bestpos,bestbiaspos,bestd,bestbiasd;
	int *p,*f,*n;

	bestd = ~(((int) 1)<<31);
	bestbiasd = bestd;
	bestpos = -1;
	bestbiaspos = bestpos;
	p = self->bias;
	f = self->freq;

	for (i=0; i<NeuquantNetworkSize; i++) {
		n = self->network[i];
		dist = n[0] - b;   if (dist<0) dist = -dist;
		a = n[1] - g;   if (a<0) a = -a;
		dist += a;
		a = n[2] - r;   if (a<0) a = -a;
		dist += a;
		if (dist<bestd) {bestd=dist; bestpos=i;}
		biasdist = dist - ((*p)>>(intbiasshift-netbiasshift));
		if (biasdist<bestbiasd) {bestbiasd=biasdist; bestbiaspos=i;}
		betafreq = (*f >> betashift);
		*f++ -= betafreq;
		*p++ += (betafreq<<gammashift);
	}
	self->freq[bestpos] += beta;
	self->bias[bestpos] -= betagamma;
	return bestbiaspos;
}


/* Move neuron i towards biased (b,g,r) by factor alpha
   ---------------------------------------------------- */

static void altersingle(Neuquant *self,int alpha,int i,int b,int g,int r)
{
	register int *n;

	n = self->network[i];				/* alter hit neuron */
	*n -= (alpha*(*n - b)) / initalpha;
	n++;
	*n -= (alpha*(*n - g)) / initalpha;
	n++;
	*n -= (alpha*(*n - r)) / initalpha;
}


/* Move adjacent neurons by precomputed alpha*(1-((i-j)^2/[r]^2)) in radpower[|i-j|]
   --------------------------------------------------------------------------------- */

static void alterneigh(Neuquant *self,int rad,int i,int b,int g,int r)
{
	int j,k,lo,hi,a;
	int *p, *q;

	lo = i-rad;   if (lo<-1) lo=-1;
	hi = i+rad;   if (hi>NeuquantNetworkSize) hi=NeuquantNetworkSize;

	j = i+1;
	k = i-1;
	q = self->radpower;
	while ((j<hi) || (k>lo)) {
		a = (*(++q));
		if (j<hi) {
			p = self->network[j];
			*p -= (a*(*p - b)) / alpharadbias;
			p++;
			*p -= (a*(*p - g)) / alpharadbias;
			p++;
			*p -= (a*(*p - r)) / alpharadbias;
			j++;
		}
		if (k>lo) {
			p = self->network[k];
			*p -= (a*(*p - b)) / alpharadbias;
			p++;
			*p -= (a*(*p - g)) / alpharadbias;
			p++;
			*p -= (a*(*p - r)) / alpharadbias;
			k--;
		}
	}
}

void NeuquantLearn(Neuquant *self,const void *data,NeuquantLookupFunction *func,int numpixels,int samplefac)
{
	int alphadec=30+((samplefac-1)/3);
	int samplepixels=numpixels/samplefac;
	int delta=samplepixels/ncycles;
	int alpha=initalpha;
	int radius=initradius;
	
	int rad=radius>>radiusbiasshift;
	if(rad<=1) rad=0;
	for(int i=0;i<rad;i++) self->radpower[i]=alpha*(((rad*rad-i*i)*radbias)/(rad*rad));

	int step;
	if(samplefac==1) step=1;
	else if(numpixels%prime1!=0) step=prime1;
	else if(numpixels%prime2!=0) step=prime2;
	else if(numpixels%prime3!=0) step=prime3;
	else step=prime4;

	int pixel=0;
	for(int i=0;i<samplepixels;i++)
	{
		int r,g,b;
		func(data,pixel,&r,&g,&b);
		b<<=netbiasshift;
		g<<=netbiasshift;
		r<<=netbiasshift;
		int j=contest(self,b,g,r);

		altersingle(self,alpha,j,b,g,r);
		if(rad) alterneigh(self,rad,j,b,g,r);

		pixel=(pixel+step)%numpixels;
	
		if((i+1)%delta==0)
		{	
			alpha-=alpha/alphadec;
			radius-=radius/radiusdec;
			rad=radius>>radiusbiasshift;
			if(rad<=1) rad=0;
	
			for(int j=0;j<rad;j++) self->radpower[j]=alpha*(((rad*rad-j*j)*radbias)/(rad*rad));
		}
	}
}
