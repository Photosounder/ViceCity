//+ rouz edit (ChatGPT)
#include "CompressedVector.h"
CollisionVector3 CompressedVector_Get(const CompressedVector *self)
{
    // Decode every coordinate before returning the original by-value result
#ifdef COMPRESSED_COL_VECTORS
    const CollisionVector3 value={self->x/128.0f,self->y/128.0f,self->z/128.0f};
#else
    const CollisionVector3 value={self->x,self->y,self->z};
#endif
    return value;
}
void CompressedVector_Set(CompressedVector *self,float x,float y,float z)
{
    // Preserve coordinate assignment order and the original floating point conversion contract
#ifdef COMPRESSED_COL_VECTORS
    self->x=x*128.0f;self->y=y*128.0f;self->z=z*128.0f;
#else
    self->x=x;self->y=y;self->z=z;
#endif
}
#ifdef COMPRESSED_COL_VECTORS
void CompressedVector_Unpack(const CompressedVector *self,void *quadword)
{
    // Retain the native quadword unpack operation and its original fourth-lane behavior
#ifdef GTA_PS2
    typedef unsigned int CollisionQuadWord __attribute__((mode(TI)));
		__asm__ volatile (
			"lh      $8, 0(%1)\n"
			"lh      $9, 2(%1)\n"
			"lh      $10, 4(%1)\n"
			"pextlw  $10, $8\n"
			"pextlw  $2, $9, $10\n"
			"sq      $2, %0\n"
			: "=m" (*(CollisionQuadWord*)quadword)
			: "r" (self)
			: "$8", "$9", "$10", "$2"
		);
#else
    int32_t *qword=(int32_t*)quadword;
    qword[0]=self->x;qword[1]=self->y;qword[2]=self->z;qword[3]=0;
#endif
}
#endif
//- rouz edit (ChatGPT)
