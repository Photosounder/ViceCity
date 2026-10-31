//+ rouz edit (ChatGPT)
#include "config.h"
#include "ClumpRead.h"
#include "ClumpRead_backend.h"
#include <string.h>
#ifndef MASTER
#define assert(expression) (void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0))
#else
#define assert(expression) (expression)
#endif



static int32_t numberGeometrys;
static int32_t streamPosition;
static ClumpReadGeometryList gGeomList;
static ClumpReadChunkInfo gClumpInfo;

ClumpReadGeometryList*
ClumpRead_GeometryListStreamRead1(void *stream, ClumpReadGeometryList *geomlist)
{
    // Preserve the complete original GeometryListStreamRead1 operation and live mutation order
	int i;
	uint32_t size, version;
	int32_t numGeoms;

	numberGeometrys = 0;
	if(!ClumpRead_HostFindChunk(stream, CLUMPREAD_STRUCT, &size, &version))
		return NULL;
	assert(size == 4);
	if(ClumpRead_HostRead(stream, &numGeoms, 4) != 4)
		return NULL;

	numberGeometrys = numGeoms/2;
	geomlist->numGeoms = numGeoms;
	if(geomlist->numGeoms > 0){
		geomlist->geometries = (void**)ClumpRead_HostAllocate(geomlist->numGeoms * sizeof(void*));
		if(geomlist->geometries == NULL)
			return NULL;
		memset(geomlist->geometries, 0, geomlist->numGeoms * sizeof(void*));
	}else
		geomlist->geometries = NULL;

	for(i = 0; i < numberGeometrys; i++){
		if(!ClumpRead_HostFindChunk(stream, CLUMPREAD_GEOMETRY, NULL, &version))
			return NULL;
		geomlist->geometries[i] = ClumpRead_HostReadGeometry(stream);
		if(geomlist->geometries[i] == NULL)
			return NULL;
	}

	return geomlist;
}

ClumpReadGeometryList*
ClumpRead_GeometryListStreamRead2(void *stream, ClumpReadGeometryList *geomlist)
{
    // Preserve the complete original GeometryListStreamRead2 operation and live mutation order
	int i;
	uint32_t version;

	for(i = numberGeometrys; i < geomlist->numGeoms; i++){
		if(!ClumpRead_HostFindChunk(stream, CLUMPREAD_GEOMETRY, NULL, &version))
			return NULL;
		geomlist->geometries[i] = ClumpRead_HostReadGeometry(stream);
		if(geomlist->geometries[i] == NULL)
			return NULL;
	}

	return geomlist;
}

void
ClumpRead_GeometryListDeinitialize(ClumpReadGeometryList *geomlist)
{
    // Preserve the complete original GeometryListDeinitialize operation and live mutation order
	int i;

	for(i = 0; i < geomlist->numGeoms; i++)
		if(geomlist->geometries[i])
			ClumpRead_HostDestroyGeometry(geomlist->geometries[i]);

	if(geomlist->numGeoms){
		ClumpRead_HostFree(geomlist->geometries);
		geomlist->numGeoms = 0;
	}
}

void*
ClumpRead_AtomicStreamRead(void *stream, void *frmList, ClumpReadGeometryList *geomList)
{
    // Preserve the complete original AtomicStreamRead operation and live mutation order
	uint32_t size, version;
	ClumpReadAtomicBinary a;
	void *atomic;

	numberGeometrys = 0;
	if(!ClumpRead_HostFindChunk(stream, CLUMPREAD_STRUCT, &size, &version))
		return NULL;
	assert(size <= sizeof(ClumpReadAtomicBinary));
	if(ClumpRead_HostRead(stream, &a, size) != size)
		return NULL;

	atomic = ClumpRead_HostCreateAtomic();
	if(atomic == NULL)
		return NULL;

	ClumpRead_HostAtomicFlags(atomic, a.flags);

	if(ClumpRead_HostFrameCount(frmList)){
		assert(a.frameIndex < ClumpRead_HostFrameCount(frmList));
		ClumpRead_HostAtomicFrame(atomic, ClumpRead_HostFrameAt(frmList,a.frameIndex));
	}

	if(geomList->numGeoms){
		assert(a.geomIndex < geomList->numGeoms);
		ClumpRead_HostAtomicGeometry(atomic, geomList->geometries[a.geomIndex], 0);
	}else{
		void *geom;
		if(!ClumpRead_HostFindChunk(stream, CLUMPREAD_GEOMETRY, NULL, &version)){
			ClumpRead_HostDestroyAtomic(atomic);
			return NULL;
		}
		geom = ClumpRead_HostReadGeometry(stream);
		if(geom == NULL){
			ClumpRead_HostDestroyAtomic(atomic);
			return NULL;
		}
		ClumpRead_HostAtomicGeometry(atomic, geom, 0);
		ClumpRead_HostDestroyGeometry(geom);
	}

	return atomic;
}

bool
ClumpRead_Start(void *stream)
{
    // Preserve the complete original Start operation and live mutation order
	uint32_t size, version;

	if(!ClumpRead_HostFindChunk(stream, CLUMPREAD_STRUCT, &size, &version))
		return false;
	if(version >= 0x33000){
		assert(size == 12);
		if(ClumpRead_HostRead(stream, &gClumpInfo, 12) != 12)
			return false;
	}else{
		assert(size == 4);
		if(ClumpRead_HostRead(stream, &gClumpInfo, 4) != 4)
			return false;
	}

	if(!ClumpRead_HostFindChunk(stream, CLUMPREAD_FRAMELIST, NULL, &version))
		return false;
	if(ClumpRead_HostReadFrames(stream, ClumpRead_HostFrames()) == NULL)
		return false;

	if(!ClumpRead_HostFindChunk(stream, CLUMPREAD_GEOMETRYLIST, NULL, &version)){
		ClumpRead_HostDeinitializeFrames(ClumpRead_HostFrames());
		return false;
	}
	if(ClumpRead_GeometryListStreamRead1(stream, &gGeomList) == NULL){
		ClumpRead_HostDeinitializeFrames(ClumpRead_HostFrames());
		return false;
	}
	streamPosition = ClumpRead_HostPosition(stream);
	return true;
}

void*
ClumpRead_Finish(void *stream)
{
    // Preserve the complete original Finish operation and live mutation order
	int i;
	uint32_t version;
	void *atomic;
	void *clump;

	clump = ClumpRead_HostCreateClump();
	if(clump == NULL)
		return NULL;

	ClumpRead_HostSkip(stream, streamPosition - ClumpRead_HostPosition(stream));

	if(ClumpRead_GeometryListStreamRead2(stream, &gGeomList) == NULL){
		ClumpRead_GeometryListDeinitialize(&gGeomList);
		ClumpRead_HostDeinitializeFrames(ClumpRead_HostFrames());
		ClumpRead_HostDestroyClump(clump);
		return NULL;
	}

	ClumpRead_HostClumpFrame(clump, ClumpRead_HostFrameAt(ClumpRead_HostFrames(),0));

	for(i = 0; i < gClumpInfo.numAtomics; i++){
		if(!ClumpRead_HostFindChunk(stream, CLUMPREAD_ATOMIC, NULL, &version)){
			ClumpRead_GeometryListDeinitialize(&gGeomList);
			ClumpRead_HostDeinitializeFrames(ClumpRead_HostFrames());
			ClumpRead_HostDestroyClump(clump);
			return NULL;
		}

		atomic = ClumpRead_AtomicStreamRead(stream, ClumpRead_HostFrames(), &gGeomList);
		if(atomic == NULL){
			ClumpRead_GeometryListDeinitialize(&gGeomList);
			ClumpRead_HostDeinitializeFrames(ClumpRead_HostFrames());
			ClumpRead_HostDestroyClump(clump);
			return NULL;
		}

		ClumpRead_HostAddAtomic(clump, atomic);
	}

	ClumpRead_GeometryListDeinitialize(&gGeomList);
	ClumpRead_HostDeinitializeFrames(ClumpRead_HostFrames());
	return clump;
}

void
ClumpRead_Cancel(void)
{
    // Preserve the complete original Cancel operation and live mutation order
	ClumpRead_GeometryListDeinitialize(&gGeomList);
	ClumpRead_HostDeinitializeFrames(ClumpRead_HostFrames());
	ClumpRead_HostSetFrameCount(ClumpRead_HostFrames(),0);
}

//- rouz edit (ChatGPT)
