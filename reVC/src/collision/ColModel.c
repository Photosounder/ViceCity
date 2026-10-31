//+ rouz edit (ChatGPT)
#include "ColModel.h"
#include "ColModel_backend.h"
#include "GameLevels.h"
#include "config.h"
#include <stdint.h>
#undef assert
#ifndef MASTER
#define assert(expression) (void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0))
#else
#define assert(expression) (expression)
#endif
#define ALIGNPTR(p) (void*)((((uintptr_t)(void*)(p))+sizeof(void*)-1)&~(sizeof(void*)-1))
enum {COLMODEL_MEMID_COLLISION=13};
void ColModel_Init(CColModel *self)
{
    // Initialize only the fields written by the original constructor

	self->numSpheres = 0;
	self->spheres = NULL;
	self->numLines = 0;
	self->lines = NULL;
	self->numBoxes = 0;
	self->boxes = NULL;
	self->numTriangles = 0;
	self->vertices = NULL;
	self->triangles = NULL;
	self->trianglePlanes = NULL;
	self->level = LEVEL_GENERIC;	// generic col slot
	self->ownsCollisionVolumes = true;
}
void ColModel_Destroy(CColModel *self)
{
    // Release volumes through the complete original ownership policy

	ColModel_RemoveCollisionVolumes(self);
}
void ColModel_RemoveCollisionVolumes(CColModel *self)
{
    // Preserve ownership checks, free ordering and the separate collision-cache boundary

	if(self->ownsCollisionVolumes){
		ColModel_BackendFree(self->spheres);
		ColModel_BackendFree(self->lines);
		ColModel_BackendFree(self->boxes);
		ColModel_BackendFree(self->vertices);
		ColModel_BackendFree(self->triangles);
		ColModel_BackendRemoveCachedPlanes(self);
	}
	self->numSpheres = 0;
	self->numLines = 0;
	self->numBoxes = 0;
	self->numTriangles = 0;
	self->spheres = NULL;
	self->lines = NULL;
	self->boxes = NULL;
	self->vertices = NULL;
	self->triangles = NULL;
}
void ColModel_CalculateTrianglePlanes(CColModel *self)
{
    // Allocate the original extra plane element and calculate all live triangle planes

	ColModel_BackendPush(COLMODEL_MEMID_COLLISION);

	// HACK: allocate space for one more element to stuff the link pointer into
	self->trianglePlanes = (CColTrianglePlane*)ColModel_BackendAllocate(sizeof(CColTrianglePlane) * (self->numTriangles+1));
	ColModel_BackendRegister(&self->trianglePlanes);
	for(int i = 0; i < self->numTriangles; i++)
		ColTrianglePlane_SetFromVertices(&self->trianglePlanes[i],self->vertices,&self->triangles[i]);

	ColModel_BackendPop();
}
void ColModel_RemoveTrianglePlanes(CColModel *self)
{
    // Release the current plane allocation before clearing its pointer

	ColModel_BackendFree(self->trianglePlanes);
	self->trianglePlanes = NULL;
}
void ColModel_SetLinkPtr(CColModel *self,CLink *lptr)
{
    // Store the cache link at the original aligned extra-element address

	assert(self->trianglePlanes);
	// Use the C linked-list functions and node types
	//+ rouz edit (ChatGPT)
	*(CLink**)ALIGNPTR(&self->trianglePlanes[self->numTriangles]) = lptr;
	//- rouz edit (ChatGPT)
}
CLink * ColModel_GetLinkPtr(CColModel *self)
{
    // Read the cache link from the original aligned extra-element address

	assert(self->trianglePlanes);
	// Use the C linked-list functions and node types
	//+ rouz edit (ChatGPT)
	return *(CLink**)ALIGNPTR(&self->trianglePlanes[self->numTriangles]);
	//- rouz edit (ChatGPT)
}
CollisionVector3 ColModel_GetTrianglePoint(const CColModel *self,int32_t i)
{
    // Decode the complete compressed vertex before returning its coordinates

	// Copy complete decoded values before the original native vector expression
	const CollisionVector3 collisionVertexValue0_0=CompressedVector_Get(&self->vertices[i]);
	return collisionVertexValue0_0;
}
CColModel * ColModel_Assign(CColModel *self,const CColModel *other)
{
    // Retain deep-copy allocation, reuse and source traversal in the original member order

	int i;
	int numVerts;

	self->boundingSphere = other->boundingSphere;
	self->boundingBox = other->boundingBox;

	// copy spheres
	if(other->numSpheres){
		if(self->numSpheres != other->numSpheres){
			self->numSpheres = other->numSpheres;
			if(self->spheres)
				ColModel_BackendFree(self->spheres);
			self->spheres = (CColSphere*)ColModel_BackendAllocate(self->numSpheres*sizeof(CColSphere));
		}
		for(i = 0; i < self->numSpheres; i++)
			self->spheres[i] = other->spheres[i];
	}else{
		self->numSpheres = 0;
		if(self->spheres)
			ColModel_BackendFree(self->spheres);
		self->spheres = NULL;
	}

	// copy lines
	if(other->numLines){
		if(self->numLines != other->numLines){
			self->numLines = other->numLines;
			if(self->lines)
				ColModel_BackendFree(self->lines);
			self->lines = (CColLine*)ColModel_BackendAllocate(self->numLines*sizeof(CColLine));
		}
		for(i = 0; i < self->numLines; i++)
			self->lines[i] = other->lines[i];
	}else{
		self->numLines = 0;
		if(self->lines)
			ColModel_BackendFree(self->lines);
		self->lines = NULL;
	}

	// copy boxes
	if(other->numBoxes){
		if(self->numBoxes != other->numBoxes){
			self->numBoxes = other->numBoxes;
			if(self->boxes)
				ColModel_BackendFree(self->boxes);
			self->boxes = (CColBox*)ColModel_BackendAllocate(self->numBoxes*sizeof(CColBox));
		}
		for(i = 0; i < self->numBoxes; i++)
			ColBox_Assign(&self->boxes[i],&other->boxes[i]);
	}else{
		self->numBoxes = 0;
		if(self->boxes)
			ColModel_BackendFree(self->boxes);
		self->boxes = NULL;
	}

	// copy mesh
	if(other->numTriangles){
		// copy vertices
		numVerts = 0;
		for(i = 0; i < other->numTriangles; i++){
			if(other->triangles[i].a > numVerts)
				numVerts = other->triangles[i].a;
			if(other->triangles[i].b > numVerts)
				numVerts = other->triangles[i].b;
			if(other->triangles[i].c > numVerts)
				numVerts = other->triangles[i].c;
		}
		numVerts++;
		if(self->vertices)
			ColModel_BackendFree(self->vertices);
		if(numVerts){
			self->vertices = (CompressedVector*)ColModel_BackendAllocate(numVerts*sizeof(CompressedVector));
			for(i = 0; i < numVerts; i++)
				self->vertices[i] = other->vertices[i];
		}

		// copy triangles
		if(self->numTriangles != other->numTriangles){
			self->numTriangles = other->numTriangles;
			if(self->triangles)
				ColModel_BackendFree(self->triangles);
			self->triangles = (CColTriangle*)ColModel_BackendAllocate(self->numTriangles*sizeof(CColTriangle));
		}
		for(i = 0; i < self->numTriangles; i++)
			self->triangles[i] = other->triangles[i];
	}else{
		self->numTriangles = 0;
		if(self->triangles)
			ColModel_BackendFree(self->triangles);
		self->triangles = NULL;
		if(self->vertices)
			ColModel_BackendFree(self->vertices);
		self->vertices = NULL;
	}
	return self;
}
CColModel ColModel_InitStatic(CColModel *self,void (*cleanup)(void))
{
    // Initialize original static storage and register its native storage-address callback at the same lifetime boundary
    ColModel_Init(self);
    if(cleanup)atexit(cleanup);
    return *self;
}
void ColModel_DestroyArray(CColModel *models,int32_t count)
{
    // Preserve the original reverse element destruction order for native static collision arrays
    for(int32_t i=count;i>0;--i)ColModel_Destroy(&models[i-1]);
}
//- rouz edit (ChatGPT)
