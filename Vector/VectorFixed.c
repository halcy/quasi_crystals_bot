#include "VectorFixed.h"

ivec3_t ivec3cross(ivec3_t a,ivec3_t b)
{
	return ivec3(imul(a.y,b.z)-imul(a.z,b.y),
	            imul(a.z,b.x)-imul(a.x,b.z),
	            imul(a.x,b.y)-imul(a.y,b.x));
}

