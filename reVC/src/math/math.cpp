//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"

#include "VuVector.h"
#include "ColLine_native.h"
#include "ColSphere_native.h"

// TODO: move more stuff into here


void TransformPoint(CVuVector &out, const CMatrix &mat, const CVuVector &in)
{
#ifdef GTA_PS2
	__asm__ __volatile__("\n\
		lqc2    vf01,0x0(%2)\n\
		lqc2    vf02,0x0(%1)\n\
		lqc2    vf03,0x10(%1)\n\
		lqc2    vf04,0x20(%1)\n\
		lqc2    vf05,0x30(%1)\n\
		vmulax.xyz	ACC,   vf02,vf01\n\
		vmadday.xyz	ACC,   vf03,vf01\n\
		vmaddaz.xyz	ACC,   vf04,vf01\n\
		vmaddw.xyz	vf06,vf05,vf00\n\
		sqc2    vf06,0x0(%0)\n\
		": : "r" (&out) , "r" (&mat) ,"r" (&in): "memory");
#else
	out = mat * in;
#endif
}

void TransformPoint(CVuVector &out, const CMatrix &mat, const RwV3d &in)
{
#ifdef GTA_PS2
	__asm__ __volatile__("\n\
		ldr	$8,0x0(%2)\n\
		ldl	$8,0x7(%2)\n\
		lw	$9,0x8(%2)\n\
		pcpyld	$10,$9,$8\n\
		qmtc2	$10,vf01\n\
		lqc2    vf02,0x0(%1)\n\
		lqc2    vf03,0x10(%1)\n\
		lqc2    vf04,0x20(%1)\n\
		lqc2    vf05,0x30(%1)\n\
		vmulax.xyz	ACC,   vf02,vf01\n\
		vmadday.xyz	ACC,   vf03,vf01\n\
		vmaddaz.xyz	ACC,   vf04,vf01\n\
		vmaddw.xyz	vf06,vf05,vf00\n\
		sqc2    vf06,0x0(%0)\n\
		": : "r" (&out) , "r" (&mat) ,"r" (&in): "memory");
#else
	out = mat * in;
#endif
}

void TransformPoints(CVuVector *out, int n, const CMatrix &mat, const RwV3d *in, int stride)
{
#ifdef GTA_PS3
	__asm__ __volatile__("\n\
		paddub	$3,%4,$0\n\
		lqc2    vf02,0x0(%2)\n\
		lqc2    vf03,0x10(%2)\n\
		lqc2    vf04,0x20(%2)\n\
		lqc2    vf05,0x30(%2)\n\
		ldr	$8,0x0(%3)\n\
		ldl	$8,0x7(%3)\n\
		lw	$9,0x8(%3)\n\
		pcpyld	$10,$9,$8\n\
		qmtc2	$10,vf01\n\
	1:	vmulax.xyz	ACC,   vf02,vf01\n\
		vmadday.xyz	ACC,   vf03,vf01\n\
		vmaddaz.xyz	ACC,   vf04,vf01\n\
		vmaddw.xyz	vf06,vf05,vf00\n\
		add	%3,%3,$3\n\
		ldr	$8,0x0(%3)\n\
		ldl	$8,0x7(%3)\n\
		lw	$9,0x8(%3)\n\
		pcpyld	$10,$9,$8\n\
		qmtc2	$10,vf01\n\
		addi	%1,%1,-1\n\
		addiu	%0,%0,0x10\n\
		sqc2    vf06,-0x10(%0)\n\
		bnez	%1,1b\n\
		": : "r" (out) , "r" (n), "r" (&mat), "r" (in), "r" (stride): "memory");
#else
	while(n--){
		*out = mat * *in;
		in = (RwV3d*)((uint8*)in + stride);
		out++;
	}
#endif
}

void TransformPoints(CVuVector *out, int n, const CMatrix &mat, const CVuVector *in)
{
#ifdef GTA_PS2
	__asm__ __volatile__("\n\
		lqc2    vf02,0x0(%2)\n\
		lqc2    vf03,0x10(%2)\n\
		lqc2    vf04,0x20(%2)\n\
		lqc2    vf05,0x30(%2)\n\
		lqc2    vf01,0x0(%3)\n\
		nop\n\
	1:	vmulax.xyz	ACC,   vf02,vf01\n\
		vmadday.xyz	ACC,   vf03,vf01\n\
		vmaddaz.xyz	ACC,   vf04,vf01\n\
		vmaddw.xyz	vf06,vf05,vf00\n\
		lqc2	vf01,0x10(%3)\n\
		addiu	%3,%3,0x10\n\
		addi	%1,%1,-1\n\
		addiu	%0,%0,0x10\n\
		sqc2    vf06,-0x10(%0)\n\
		bnez	%1,1b\n\
		": : "r" (out) , "r" (n), "r" (&mat) ,"r" (in): "memory");
#else
	while(n--){
		*out = mat * *in;
		in++;
		out++;
	}
#endif
}

void TransformColLines(CVuVector *out,int numLines,const CMatrix &mat,const CColLine *lines)
{
#ifdef GTA_PS3
    // Retain the existing native batch path with the same two sixteen-byte endpoints per line
    int n=numLines*2;const void *in=lines;const int stride=sizeof(CColLine)/2;
	__asm__ __volatile__("\n\
		paddub	$3,%4,$0\n\
		lqc2    vf02,0x0(%2)\n\
		lqc2    vf03,0x10(%2)\n\
		lqc2    vf04,0x20(%2)\n\
		lqc2    vf05,0x30(%2)\n\
		ldr	$8,0x0(%3)\n\
		ldl	$8,0x7(%3)\n\
		lw	$9,0x8(%3)\n\
		pcpyld	$10,$9,$8\n\
		qmtc2	$10,vf01\n\
	1:	vmulax.xyz	ACC,   vf02,vf01\n\
		vmadday.xyz	ACC,   vf03,vf01\n\
		vmaddaz.xyz	ACC,   vf04,vf01\n\
		vmaddw.xyz	vf06,vf05,vf00\n\
		add	%3,%3,$3\n\
		ldr	$8,0x0(%3)\n\
		ldl	$8,0x7(%3)\n\
		lw	$9,0x8(%3)\n\
		pcpyld	$10,$9,$8\n\
		qmtc2	$10,vf01\n\
		addi	%1,%1,-1\n\
		addiu	%0,%0,0x10\n\
		sqc2    vf06,-0x10(%0)\n\
		bnez	%1,1b\n\
		": : "r" (out) , "r" (n), "r" (&mat), "r" (in), "r" (stride): "memory");
#else
    // Preserve the original point order and native matrix multiplication without unrelated struct aliases
    while(numLines--){
        *out=mat*CollisionVector_ToNative(lines->p0);out++;
        *out=mat*CollisionVector_ToNative(lines->p1);out++;
        lines++;
    }
#endif
}
//- rouz edit (ChatGPT)

void TransformColSpheres(CVuVector *out,int numSpheres,const CMatrix &mat,const CColSphere *spheres)
{
#ifdef GTA_PS3
    // Keep the original point count and twenty-byte input stride for the native batch path
    const int n=numSpheres;const void *in=spheres;const int stride=sizeof(CColSphere);
	__asm__ __volatile__("\n\
		paddub	$3,%4,$0\n\
		lqc2    vf02,0x0(%2)\n\
		lqc2    vf03,0x10(%2)\n\
		lqc2    vf04,0x20(%2)\n\
		lqc2    vf05,0x30(%2)\n\
		ldr	$8,0x0(%3)\n\
		ldl	$8,0x7(%3)\n\
		lw	$9,0x8(%3)\n\
		pcpyld	$10,$9,$8\n\
		qmtc2	$10,vf01\n\
	1:	vmulax.xyz	ACC,   vf02,vf01\n\
		vmadday.xyz	ACC,   vf03,vf01\n\
		vmaddaz.xyz	ACC,   vf04,vf01\n\
		vmaddw.xyz	vf06,vf05,vf00\n\
		add	%3,%3,$3\n\
		ldr	$8,0x0(%3)\n\
		ldl	$8,0x7(%3)\n\
		lw	$9,0x8(%3)\n\
		pcpyld	$10,$9,$8\n\
		qmtc2	$10,vf01\n\
		addi	%1,%1,-1\n\
		addiu	%0,%0,0x10\n\
		sqc2    vf06,-0x10(%0)\n\
		bnez	%1,1b\n\
		": : "r" (out) , "r" (n), "r" (&mat), "r" (in), "r" (stride): "memory");
#else
    // Preserve scalar transformation order while accessing actual C coordinate subobjects
    while(numSpheres--){*out=mat*CollisionVector_ToNative(spheres->bounds.center);out++;spheres++;}
#endif
}
//- rouz edit (ChatGPT)
