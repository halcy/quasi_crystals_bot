#include "MatrixFixed.h"

imat3x3_t imat3x3rotate(int angle,ivec3_t axis)
{
	int32_t sine=isin(angle);
	int32_t cosine=icos(angle);
	int32_t one_minus_cosine=F(1)-cosine;

	axis=ivec3norm(axis);

	return imat3x3(
	cosine + imul(imul(one_minus_cosine,axis.x),axis.x),
	imul(imul(one_minus_cosine,axis.x),axis.y) + imul(axis.z,sine),
	imul(imul(one_minus_cosine,axis.x),axis.z) - imul(axis.y,sine),

	imul(imul(one_minus_cosine,axis.x),axis.y) - imul(axis.z,sine),
	cosine + imul(imul(one_minus_cosine,axis.y),axis.y),
	imul(imul(one_minus_cosine,axis.y),axis.z) + imul(axis.x,sine),

	imul(imul(one_minus_cosine,axis.x),axis.z) + imul(axis.y,sine),
	imul(imul(one_minus_cosine,axis.y),axis.z) - imul(axis.x,sine),
	cosine + imul(imul(one_minus_cosine,axis.z),axis.z));
}

imat3x3_t imat3x3inverselookat(ivec3_t eye,ivec3_t center,ivec3_t up)
{
	ivec3_t backward=ivec3norm(ivec3sub(eye,center));
	ivec3_t right=ivec3norm(ivec3cross(up,backward));
	ivec3_t actualup=ivec3norm(ivec3cross(backward,right));

	return imat3x3cols(right,actualup,backward);
}

imat2x2_t imat2x2mul(imat2x2_t a,imat2x2_t b)
{
	return imat2x2(imul(a.m[0],b.m[0])+imul(a.m[2],b.m[1]),
	              imul(a.m[0],b.m[2])+imul(a.m[2],b.m[3]),

	              imul(a.m[1],b.m[0])+imul(a.m[3],b.m[1]),
	              imul(a.m[1],b.m[2])+imul(a.m[3],b.m[3]));
}

imat3x3_t imat3x3mul(imat3x3_t a,imat3x3_t b)
{
	return imat3x3(imul(a.m[0],b.m[0])+imul(a.m[3],b.m[1])+imul(a.m[6],b.m[2]),
	              imul(a.m[0],b.m[3])+imul(a.m[3],b.m[4])+imul(a.m[6],b.m[5]),
	              imul(a.m[0],b.m[6])+imul(a.m[3],b.m[7])+imul(a.m[6],b.m[8]),

	              imul(a.m[1],b.m[0])+imul(a.m[4],b.m[1])+imul(a.m[7],b.m[2]),
	              imul(a.m[1],b.m[3])+imul(a.m[4],b.m[4])+imul(a.m[7],b.m[5]),
	              imul(a.m[1],b.m[6])+imul(a.m[4],b.m[7])+imul(a.m[7],b.m[8]),

	              imul(a.m[2],b.m[0])+imul(a.m[5],b.m[1])+imul(a.m[8],b.m[2]),
	              imul(a.m[2],b.m[3])+imul(a.m[5],b.m[4])+imul(a.m[8],b.m[5]),
	              imul(a.m[2],b.m[6])+imul(a.m[5],b.m[7])+imul(a.m[8],b.m[8]));
}

imat4x4_t imat4x4mul(imat4x4_t a,imat4x4_t b)
{
        imat4x4_t res;
        for(int i=0;i<16;i++)
        {
                int row=i&3,column=i&12;
                int32_t val=0;
                for(int j=0;j<4;j++) val+=imul(a.m[row+j*4],b.m[column+j]);
                res.m[i]=val;
        }
        return res;
}

imat3x2_t imat3x2affinemul(imat3x2_t a,imat3x2_t b)
{
	return imat3x2(imul(a.m[0],b.m[0])+imul(a.m[2],b.m[1]),
	              imul(a.m[0],b.m[2])+imul(a.m[2],b.m[3]),
	              imul(a.m[0],b.m[4])+imul(a.m[2],b.m[5])+a.m[4],

	              imul(a.m[1],b.m[0])+imul(a.m[3],b.m[1]),
	              imul(a.m[1],b.m[2])+imul(a.m[3],b.m[3]),
	              imul(a.m[1],b.m[4])+imul(a.m[3],b.m[5])+a.m[5]);
}

imat3x3_t imat3x3affinemul(imat3x3_t a,imat3x3_t b)
{
	return imat3x3(imul(a.m[0],b.m[0])+imul(a.m[3],b.m[1]),
	              imul(a.m[0],b.m[3])+imul(a.m[3],b.m[4]),
	              imul(a.m[0],b.m[6])+imul(a.m[3],b.m[7])+a.m[6],

	              imul(a.m[1],b.m[0])+imul(a.m[4],b.m[1]),
	              imul(a.m[1],b.m[3])+imul(a.m[4],b.m[4]),
	              imul(a.m[1],b.m[6])+imul(a.m[4],b.m[7])+a.m[7],

	              0,0,F(1));
}

imat4x3_t imat4x3affinemul(imat4x3_t a,imat4x3_t b)
{
	return imat4x3(imul(a.m[0],b.m[0])+imul(a.m[3],b.m[1])+imul(a.m[6],b.m[2]),
	              imul(a.m[0],b.m[3])+imul(a.m[3],b.m[4])+imul(a.m[6],b.m[5]),
	              imul(a.m[0],b.m[6])+imul(a.m[3],b.m[7])+imul(a.m[6],b.m[8]),
	              imul(a.m[0],b.m[9])+imul(a.m[3],b.m[10])+imul(a.m[6],b.m[11])+a.m[9],

	              imul(a.m[1],b.m[0])+imul(a.m[4],b.m[1])+imul(a.m[7],b.m[2]),
	              imul(a.m[1],b.m[3])+imul(a.m[4],b.m[4])+imul(a.m[7],b.m[5]),
	              imul(a.m[1],b.m[6])+imul(a.m[4],b.m[7])+imul(a.m[7],b.m[8]),
	              imul(a.m[1],b.m[9])+imul(a.m[4],b.m[10])+imul(a.m[7],b.m[11])+a.m[10],

	              imul(a.m[2],b.m[0])+imul(a.m[5],b.m[1])+imul(a.m[8],b.m[2]),
	              imul(a.m[2],b.m[3])+imul(a.m[5],b.m[4])+imul(a.m[8],b.m[5]),
	              imul(a.m[2],b.m[6])+imul(a.m[5],b.m[7])+imul(a.m[8],b.m[8]),
	              imul(a.m[2],b.m[9])+imul(a.m[5],b.m[10])+imul(a.m[8],b.m[11])+a.m[11]);
}

imat4x4_t imat4x4affinemul(imat4x4_t a,imat4x4_t b)
{
	return imat4x4(imul(a.m[0],b.m[0])+imul(a.m[4],b.m[1])+imul(a.m[8],b.m[2]),
	              imul(a.m[0],b.m[4])+imul(a.m[4],b.m[5])+imul(a.m[8],b.m[6]),
	              imul(a.m[0],b.m[8])+imul(a.m[4],b.m[9])+imul(a.m[8],b.m[10]),
	              imul(a.m[0],b.m[12])+imul(a.m[4],b.m[13])+imul(a.m[8],b.m[14])+a.m[12],

	              imul(a.m[1],b.m[0])+imul(a.m[5],b.m[1])+imul(a.m[9],b.m[2]),
	              imul(a.m[1],b.m[4])+imul(a.m[5],b.m[5])+imul(a.m[9],b.m[6]),
	              imul(a.m[1],b.m[8])+imul(a.m[5],b.m[9])+imul(a.m[9],b.m[10]),
	              imul(a.m[1],b.m[12])+imul(a.m[5],b.m[13])+imul(a.m[9],b.m[14])+a.m[13],

	              imul(a.m[2],b.m[0])+imul(a.m[6],b.m[1])+imul(a.m[10],b.m[2]),
	              imul(a.m[2],b.m[4])+imul(a.m[6],b.m[5])+imul(a.m[10],b.m[6]),
	              imul(a.m[2],b.m[8])+imul(a.m[6],b.m[9])+imul(a.m[10],b.m[10]),
	              imul(a.m[2],b.m[12])+imul(a.m[6],b.m[13])+imul(a.m[10],b.m[14])+a.m[14],

	              0,0,0,F(1));
}

int32_t imat4x4det(imat4x4_t m)
{
	int32_t a0=imul(m.m[0],m.m[5])-imul(m.m[1],m.m[4]);
	int32_t a1=imul(m.m[0],m.m[6])-imul(m.m[2],m.m[4]);
	int32_t a2=imul(m.m[0],m.m[7])-imul(m.m[3],m.m[4]);
	int32_t a3=imul(m.m[1],m.m[6])-imul(m.m[2],m.m[5]);
	int32_t a4=imul(m.m[1],m.m[7])-imul(m.m[3],m.m[5]);
	int32_t a5=imul(m.m[2],m.m[7])-imul(m.m[3],m.m[6]);
	int32_t b0=imul(m.m[8],m.m[13])-imul(m.m[9],m.m[12]);
	int32_t b1=imul(m.m[8],m.m[14])-imul(m.m[10],m.m[12]);
	int32_t b2=imul(m.m[8],m.m[15])-imul(m.m[11],m.m[12]);
	int32_t b3=imul(m.m[9],m.m[14])-imul(m.m[10],m.m[13]);
	int32_t b4=imul(m.m[9],m.m[15])-imul(m.m[11],m.m[13]);
	int32_t b5=imul(m.m[10],m.m[15])-imul(m.m[11],m.m[14]);
	return imul(a0,b5)-imul(a1,b4)+imul(a2,b3)+imul(a3,b2)-imul(a4,b1)+imul(a5,b0);
}

imat3x3_t imat3x3inverse(imat3x3_t m)
{
	imat3x3_t res;
	int32_t det=imat3x3det(m); // singular if det==0

	res.m[0]=idiv((imul(m.m[4],m.m[8])-imul(m.m[5],m.m[7])),det);
	res.m[3]=-idiv((imul(m.m[3],m.m[8])-imul(m.m[5],m.m[6])),det);
	res.m[6]=idiv((imul(m.m[3],m.m[7])-imul(m.m[4],m.m[6])),det);

	res.m[1]=-idiv((imul(m.m[1],m.m[8])-imul(m.m[2],m.m[7])),det);
	res.m[4]=idiv((imul(m.m[0],m.m[8])-imul(m.m[2],m.m[6])),det);
	res.m[7]=-idiv((imul(m.m[0],m.m[7])-imul(m.m[1],m.m[6])),det);

	res.m[2]=idiv((imul(m.m[1],m.m[5])-imul(m.m[2],m.m[4])),det);
	res.m[5]=-idiv((imul(m.m[0],m.m[5])-imul(m.m[2],m.m[3])),det);
	res.m[8]=idiv((imul(m.m[0],m.m[4])-imul(m.m[1],m.m[3])),det);

	return res;
}

imat4x4_t imat4x4inverse(imat4x4_t m)
{
	imat4x4_t res;

	int32_t a0=imul(m.m[0],m.m[5])-imul(m.m[1],m.m[4]);
	int32_t a1=imul(m.m[0],m.m[6])-imul(m.m[2],m.m[4]);
	int32_t a2=imul(m.m[0],m.m[7])-imul(m.m[3],m.m[4]);
	int32_t a3=imul(m.m[1],m.m[6])-imul(m.m[2],m.m[5]);
	int32_t a4=imul(m.m[1],m.m[7])-imul(m.m[3],m.m[5]);
	int32_t a5=imul(m.m[2],m.m[7])-imul(m.m[3],m.m[6]);
	int32_t b0=imul(m.m[8],m.m[13])-imul(m.m[9],m.m[12]);
	int32_t b1=imul(m.m[8],m.m[14])-imul(m.m[10],m.m[12]);
	int32_t b2=imul(m.m[8],m.m[15])-imul(m.m[11],m.m[12]);
	int32_t b3=imul(m.m[9],m.m[14])-imul(m.m[10],m.m[13]);
	int32_t b4=imul(m.m[9],m.m[15])-imul(m.m[11],m.m[13]);
	int32_t b5=imul(m.m[10],m.m[15])-imul(m.m[11],m.m[14]);
	int32_t det=imul(a0,b5)-imul(a1,b4)+imul(a2,b3)+imul(a3,b2)-imul(a4,b1)+imul(a5,b0);
	// singular if det==0

	res.m[0]=idiv((imul(m.m[5],b5)-imul(m.m[6],b4)+imul(m.m[7],b3)),det);
	res.m[4]=-idiv((imul(m.m[4],b5)-imul(m.m[6],b2)+imul(m.m[7],b1)),det);
	res.m[8]=idiv((imul(m.m[4],b4)-imul(m.m[5],b2)+imul(m.m[7],b0)),det);
	res.m[12]=-idiv((imul(m.m[4],b3)-imul(m.m[5],b1)+imul(m.m[6],b0)),det);

	res.m[1]=-idiv((imul(m.m[1],b5)-imul(m.m[2],b4)+imul(m.m[3],b3)),det);
	res.m[5]=idiv((imul(m.m[0],b5)-imul(m.m[2],b2)+imul(m.m[3],b1)),det);
	res.m[9]=-idiv((imul(m.m[0],b4)-imul(m.m[1],b2)+imul(m.m[3],b0)),det);
	res.m[13]=idiv((imul(m.m[0],b3)-imul(m.m[1],b1)+imul(m.m[2],b0)),det);

	res.m[2]=idiv((imul(m.m[13],a5)-imul(m.m[14],a4)+imul(m.m[15],a3)),det);
	res.m[6]=-idiv((imul(m.m[12],a5)-imul(m.m[14],a2)+imul(m.m[15],a1)),det);
	res.m[10]=idiv((imul(m.m[12],a4)-imul(m.m[13],a2)+imul(m.m[15],a0)),det);
	res.m[14]=-idiv((imul(m.m[12],a3)-imul(m.m[13],a1)+imul(m.m[14],a0)),det);

	res.m[3]=-idiv((imul(m.m[9],a5)-imul(m.m[10],a4)+imul(m.m[11],a3)),det);
	res.m[7]=idiv((imul(m.m[8],a5)-imul(m.m[10],a2)+imul(m.m[11],a1)),det);
	res.m[11]=-idiv((imul(m.m[8],a4)-imul(m.m[9],a2)+imul(m.m[11],a0)),det);
	res.m[15]=idiv((imul(m.m[8],a3)-imul(m.m[9],a1)+imul(m.m[10],a0)),det);

	return res;
}

imat3x2_t imat3x2affineinverse(imat3x2_t m)
{
	imat3x2_t res;
	int32_t det=imat3x2affinedet(m); // singular if det==0

	res.m[0]=idiv(m.m[3],det);
	res.m[2]=-idiv(m.m[2],det);

	res.m[1]=-idiv(m.m[1],det);
	res.m[3]=idiv(m.m[0],det);

	res.m[4]=-(imul(m.m[4],res.m[0])+imul(m.m[5],res.m[2]));
	res.m[5]=-(imul(m.m[4],res.m[1])+imul(m.m[5],res.m[3]));

	return res;
}

imat3x3_t imat3x3affineinverse(imat3x3_t m)
{
	imat3x3_t res;
	int32_t det=imat3x3affinedet(m); // singular if det==0

	res.m[0]=idiv(m.m[4],det);
	res.m[3]=-idiv(m.m[3],det);

	res.m[1]=-idiv(m.m[1],det);
	res.m[4]=idiv(m.m[0],det);

	res.m[2]=0;
	res.m[5]=0;

	res.m[6]=-(imul(m.m[6],res.m[0])+imul(m.m[7],res.m[3]));
	res.m[7]=-(imul(m.m[6],res.m[1])+imul(m.m[7],res.m[4]));
	res.m[8]=F(1);

	return res;
}

imat4x3_t imat4x3affineinverse(imat4x3_t m)
{
	imat4x3_t res;
	int32_t det=imat4x3affinedet(m); // singular if det==0

	res.m[0]=idiv((imul(m.m[4],m.m[8])-imul(m.m[5],m.m[7])),det);
	res.m[3]=-idiv((imul(m.m[3],m.m[8])-imul(m.m[5],m.m[6])),det);
	res.m[6]=idiv((imul(m.m[3],m.m[7])-imul(m.m[4],m.m[6])),det);

	res.m[1]=-idiv((imul(m.m[1],m.m[8])-imul(m.m[2],m.m[7])),det);
	res.m[4]=idiv((imul(m.m[0],m.m[8])-imul(m.m[2],m.m[6])),det);
	res.m[7]=-idiv((imul(m.m[0],m.m[7])-imul(m.m[1],m.m[6])),det);

	res.m[2]=idiv((imul(m.m[1],m.m[5])-imul(m.m[2],m.m[4])),det);
	res.m[5]=-idiv((imul(m.m[0],m.m[5])-imul(m.m[2],m.m[3])),det);
	res.m[8]=idiv((imul(m.m[0],m.m[4])-imul(m.m[1],m.m[3])),det);

	res.m[9]=-(imul(m.m[9],res.m[0])+imul(m.m[10],res.m[3])+imul(m.m[11],res.m[6]));
	res.m[10]=-(imul(m.m[9],res.m[1])+imul(m.m[10],res.m[4])+imul(m.m[11],res.m[7]));
	res.m[11]=-(imul(m.m[9],res.m[2])+imul(m.m[10],res.m[5])+imul(m.m[11],res.m[8]));


	return res;
}

imat4x4_t imat4x4affineinverse(imat4x4_t m)
{
	imat4x4_t res;
	int32_t det=imat4x4affinedet(m);
	// singular if det==0

	res.m[0]=idiv((imul(m.m[5],m.m[10])-imul(m.m[6],m.m[9])),det);
	res.m[4]=-idiv((imul(m.m[4],m.m[10])-imul(m.m[6],m.m[8])),det);
	res.m[8]=idiv((imul(m.m[4],m.m[9])-imul(m.m[5],m.m[8])),det);

	res.m[1]=-idiv((imul(m.m[1],m.m[10])-imul(m.m[2],m.m[9])),det);
	res.m[5]=idiv((imul(m.m[0],m.m[10])-imul(m.m[2],m.m[8])),det);
	res.m[9]=-idiv((imul(m.m[0],m.m[9])-imul(m.m[1],m.m[8])),det);

	res.m[2]=idiv((imul(m.m[1],m.m[6])-imul(m.m[2],m.m[5])),det);
	res.m[6]=-idiv((imul(m.m[0],m.m[6])-imul(m.m[2],m.m[4])),det);
	res.m[10]=idiv((imul(m.m[0],m.m[5])-imul(m.m[1],m.m[4])),det);

	res.m[3]=0;
	res.m[7]=0;
	res.m[11]=0;

	res.m[12]=-(imul(m.m[12],res.m[0])+imul(m.m[13],res.m[4])+imul(m.m[14],res.m[8]));
	res.m[13]=-(imul(m.m[12],res.m[1])+imul(m.m[13],res.m[5])+imul(m.m[14],res.m[9]));
	res.m[14]=-(imul(m.m[12],res.m[2])+imul(m.m[13],res.m[6])+imul(m.m[14],res.m[10]));
	res.m[15]=F(1);

	return res;
}

ivec3_t imat3x3transform(imat3x3_t m,ivec3_t v)
{
	return ivec3(
	imul(v.x,m.m[0])+imul(v.y,m.m[3])+imul(v.z,m.m[6]),
	imul(v.x,m.m[1])+imul(v.y,m.m[4])+imul(v.z,m.m[7]),
	imul(v.x,m.m[2])+imul(v.y,m.m[5])+imul(v.z,m.m[8]));
}

ivec3_t imat4x3transform(imat4x3_t m,ivec3_t v)
{
	return ivec3(
	imul(v.x,m.m[0])+imul(v.y,m.m[3])+imul(v.z,m.m[6])+m.m[9],
	imul(v.x,m.m[1])+imul(v.y,m.m[4])+imul(v.z,m.m[7])+m.m[10],
	imul(v.x,m.m[2])+imul(v.y,m.m[5])+imul(v.z,m.m[8])+m.m[11]);
}

ivec4_t imat4x4transform(imat4x4_t m,ivec4_t v)
{
	return ivec4(
	imul(v.x,m.m[0])+imul(v.y,m.m[4])+imul(v.z,m.m[8])+imul(v.w,m.m[12]),
	imul(v.x,m.m[1])+imul(v.y,m.m[5])+imul(v.z,m.m[9])+imul(v.w,m.m[13]),
	imul(v.x,m.m[2])+imul(v.y,m.m[6])+imul(v.z,m.m[10])+imul(v.w,m.m[14]),
	imul(v.x,m.m[3])+imul(v.y,m.m[7])+imul(v.z,m.m[11])+imul(v.w,m.m[15]));
}

ivec2_t imat3x3transformvec2(imat3x3_t m,ivec2_t v)
{
	int32_t z=imul(v.x,m.m[2])+imul(v.y,m.m[5])+m.m[8];
	return ivec2(
	idiv((imul(v.x,m.m[0])+imul(v.y,m.m[3])+m.m[6]),z),
	idiv((imul(v.x,m.m[1])+imul(v.y,m.m[4])+m.m[7]),z));
}

ivec3_t imat4x4transformvec3(imat4x4_t m,ivec3_t v)
{
	int32_t w=imul(v.x,m.m[3])+imul(v.y,m.m[7])+imul(v.z,m.m[11])+m.m[15];
	return ivec3(
	idiv((imul(v.x,m.m[0])+imul(v.y,m.m[4])+imul(v.z,m.m[8])+m.m[12]),w),
	idiv((imul(v.x,m.m[1])+imul(v.y,m.m[5])+imul(v.z,m.m[9])+m.m[13]),w),
	idiv((imul(v.x,m.m[2])+imul(v.y,m.m[6])+imul(v.z,m.m[10])+m.m[14]),w));
}

