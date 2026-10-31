//+ rouz edit (ChatGPT)
#include "Markers3D_backend.h"
#include "Timer.h"
#include "FileMgr.h"
#include <math.h>
#include <stddef.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "Draw_constants.h"

typedef float (*Markers3DRead)(const void *,int32_t);
typedef void (*Markers3DWrite)(void *,int32_t,float);
static float Markers3D_Read(const void *pos,int32_t axis)
{
    // Read only the selected original C coordinate
    const Markers3DVector *v=(const Markers3DVector *)pos;return axis==0?v->x:axis==1?v->y:v->z;
}
static void Markers3D_Write(void *pos,int32_t axis,float value)
{
    // Write the selected original borrowed C coordinate
    Markers3DVector *v=(Markers3DVector *)pos;if(axis==0)v->x=value;else if(axis==1)v->y=value;else v->z=value;
}
static Markers3DVector Markers3D_ReadPosition(const void *pos,Markers3DRead read)
{
    // Retain live component reads at each original whole-position use
    Markers3DVector v={read(pos,0),read(pos,1),read(pos,2)};return v;
}
static Markers3DVector2D Markers3D_ReadPosition2D(const void *pos,Markers3DRead read)
{
    // Avoid reading the height where the original conversion used only x and y
    Markers3DVector2D v={read(pos,0),read(pos,1)};return v;
}
static void Markers3D_AddZ(void *pos,float value,Markers3DRead read,Markers3DWrite write)
{
    // Preserve the original in-place height update and aliasing
    write(pos,2,read(pos,2)+value);
}
static Markers3DVector Markers3D_Subtract(Markers3DVector a,Markers3DVector b)
{
    // Preserve original component subtraction
    Markers3DVector v={a.x-b.x,a.y-b.y,a.z-b.z};return v;
}
static float Markers3D_Squared(Markers3DVector v)
{
    // Preserve the original three-product sum order
    return v.x*v.x+v.y*v.y+v.z*v.z;
}
static float Markers3D_Dot(Markers3DVector a,Markers3DVector b)
{
    // Retain the exact original camera-depth dot product
    return a.x*b.x+a.y*b.y+a.z*b.z;
}
static Markers3DVector2D Markers3D_Subtract2D(Markers3DVector2D a,Markers3DVector2D b)
{
    // Preserve the original two-dimensional displacement
    Markers3DVector2D v={a.x-b.x,a.y-b.y};return v;
}
static float Markers3D_Squared2D(Markers3DVector2D v)
{
    // Preserve the original squared planar distance
    return v.x*v.x+v.y*v.y;
}
static void Markers3D_Normalise(Markers3DVector *v)
{
    // Retain the original reciprocal-square-root normalizer and x-only zero fallback
    float sq=Markers3D_Squared(*v);
    if(sq>0.0f){
        // Scale each component in the original order
        float invsqrt=1.0f/sqrtf(sq);v->x*=invsqrt;v->y*=invsqrt;v->z*=invsqrt;
    }else{
        // Leave the original y and z values untouched
        v->x=1.0f;
    }
}
int32_t Markers3D_NumActiveMarkers;
static int EnvMapMarkerIndices[NUM3DMARKERS];
static int EnvMapMarkerCount;

void
Markers3D_Init(void)
{
    // Retain the complete original controller operation and live data-write order
	for (int i = 0; i < NUM3DMARKERS; i++) {
		// Preserve original conditions, resource calls and borrowed-position mutations
        void *marker=Markers3D_BackendMarker(i);Markers3DFields fields=Markers3D_BackendFields(marker);
		Markers3D_BackendSetAtomic(marker,NULL);
		(*fields.m_nType) = MARKERTYPE_INVALID;
		(*fields.m_bIsUsed) = false;
		(*fields.m_bFindZOnNextPlacement) = false;
		(*fields.m_nIdentifier) = 0;
		(*fields.red) = 255;
		(*fields.green) = 255;
		(*fields.blue) = 255;
		(*fields.alpha) = 255;
		(*fields.m_nPulsePeriod) = 1024;
		(*fields.m_nRotateRate) = 5;
		(*fields.m_nStartTime) = 0;
		(*fields.m_fPulseFraction) = 0.25f;
		(*fields.m_fStdSize) = 1.0f;
		(*fields.m_fSize) = 1.0f;
		(*fields.m_fBrightness) = 1.0f;
		(*fields.m_fCameraRange) = 0.0f;
	}
	Markers3D_NumActiveMarkers = 0;
	// Drop marker snapshots when initializing a new scene
		EnvMapMarkerCount = 0;
		int txdSlot = Markers3D_BackendFindTxd("particle");
	Markers3D_BackendPushTxd();
	Markers3D_BackendSetTxd(txdSlot);
	FileMgr_ChangeDir("\\");
	Markers3D_BackendSetClump(MARKERTYPE_ARROW,Markers3D_BackendLoad("models/generic/arrow.dff"));
	Markers3D_BackendSetClump(MARKERTYPE_CYLINDER,Markers3D_BackendLoad("models/generic/zonecylb.dff"));
	Markers3D_BackendPopTxd();
}

void
Markers3D_Shutdown(void)
{
    // Retain the complete original controller operation and live data-write order
	for (int i = 0; i < NUM3DMARKERS; i++) {
		// Preserve original conditions, resource calls and borrowed-position mutations
        void *marker=Markers3D_BackendMarker(i);Markers3DFields fields=Markers3D_BackendFields(marker);
		if (Markers3D_BackendAtomic(marker) != NULL)
			Markers3D_RecordDelete(marker);
	}

	for (int i = 0; i < NUMMARKERTYPES; i++) {
		// Preserve original conditions, resource calls and borrowed-position mutations
		if (Markers3D_BackendClump(i) != NULL)
			Markers3D_BackendDestroyClump(Markers3D_BackendClump(i));
	}
}

void
Markers3D_Render(void)
{
    // Retain the complete original controller operation and live data-write order
	// Save the markers the main pass will draw before it clears their used flags
		EnvMapMarkerCount = 0;
	for(int i = 0; i < NUM3DMARKERS; i++){
		// Preserve original conditions, resource calls and borrowed-position mutations
        void *marker=Markers3D_BackendMarker(i);Markers3DFields fields=Markers3D_BackendFields(marker);
		if((*fields.m_bIsUsed) && Markers3D_BackendAtomic(marker) != NULL)
			EnvMapMarkerIndices[EnvMapMarkerCount++] = i;
		
    }
    Markers3D_NumActiveMarkers = 0;
	Markers3D_BackendDirectional();
	for (int i = 0; i < NUM3DMARKERS; i++) {
		// Preserve original conditions, resource calls and borrowed-position mutations
        void *marker=Markers3D_BackendMarker(i);Markers3DFields fields=Markers3D_BackendFields(marker);
		if ((*fields.m_bIsUsed)) {
			// Preserve original conditions, resource calls and borrowed-position mutations
			if ((*fields.m_fCameraRange) < 150.0f) {
				// Preserve original conditions, resource calls and borrowed-position mutations
				Markers3D_RecordRender(marker);
				if ((*fields.m_nType) == MARKERTYPE_ARROW) {
					// Preserve original conditions, resource calls and borrowed-position mutations
					Markers3D_BackendCorona(marker,1.2f * (*fields.m_fSize),50.0f * Markers3D_BackendLOD());
				}
			}
			Markers3D_NumActiveMarkers++;
			(*fields.m_bIsUsed) = false;
		} else if (Markers3D_BackendAtomic(marker) != NULL) {
			// Preserve original conditions, resource calls and borrowed-position mutations
			Markers3D_RecordDelete(marker);
		}
	}
}

void
Markers3D_RenderForEnvMap(void *camera)
{
    // Retain the complete original controller operation and live data-write order
	// Match the directional lighting setup used by the main marker pass
	if(camera == NULL || EnvMapMarkerCount <= 0){
		// Preserve original conditions, resource calls and borrowed-position mutations
		EnvMapMarkerCount = 0;
		return;
	}
	// Read the reflection camera position and forward axis without changing the game camera
	void *cameraFrame = Markers3D_BackendCameraFrame(camera);
	// Discard the marker snapshot if the reflection camera has no matrix
	if(cameraFrame == NULL || Markers3D_BackendFrameMatrix(cameraFrame) == NULL){
		// Preserve original conditions, resource calls and borrowed-position mutations
		EnvMapMarkerCount = 0;
		return;
	}
	void *cameraMatrix = Markers3D_BackendFrameMatrix(cameraFrame);
	Markers3DVector cameraPosition=Markers3D_BackendMatrixPosition(cameraMatrix);
	Markers3DVector cameraForward=Markers3D_BackendMatrixForward(cameraMatrix);
	Markers3D_Normalise(&cameraForward);
	float nearClip = Markers3D_BackendNearClip(camera);
	float farClip = Markers3D_BackendFarClip(camera);
	Markers3D_BackendDirectional();

	// Draw captured markers that lie within this camera's range and depth planes
	for(int i = 0; i < EnvMapMarkerCount; i++){
		// Preserve original conditions, resource calls and borrowed-position mutations
		void *marker = Markers3D_BackendMarker(EnvMapMarkerIndices[i]);
		Markers3DVector toMarker = Markers3D_Subtract(Markers3D_BackendPosition(marker),cameraPosition);
		// Skip markers outside the reflection camera's world range
		if(Markers3D_Squared(toMarker) >= 150.0f*150.0f)
			continue;
		float cameraDepth = Markers3D_Dot(toMarker,cameraForward);
		// Skip markers behind the near clip or beyond the far clip
		if(cameraDepth <= nearClip || cameraDepth >= farClip)
			continue;
		Markers3D_RecordRender(marker);
	}
	EnvMapMarkerCount = 0;
}

static void *
Markers3D_PlaceInput(uint32_t identifier,uint16_t type,void *pos,float size,uint8_t r,uint8_t g,uint8_t b,uint8_t a,uint16_t pulsePeriod,float pulseFraction,int16_t rotateRate,Markers3DRead read,Markers3DWrite write)
{
    // Retain the complete original controller operation and live data-write order
	void *pMarker;
	Markers3DVector2D playerPos = Markers3D_BackendPlayer();
	pMarker = NULL;
	float dist = sqrtf(Markers3D_Squared2D(Markers3D_Subtract2D(Markers3D_ReadPosition2D(pos,read),playerPos)));

	if (type != MARKERTYPE_ARROW && type != MARKERTYPE_CYLINDER) return NULL;

	for (int i = 0; i < NUM3DMARKERS; i++) {
		// Preserve original conditions, resource calls and borrowed-position mutations
        void *marker=Markers3D_BackendMarker(i);Markers3DFields fields=Markers3D_BackendFields(marker);
		if (!(*fields.m_bIsUsed) && (*fields.m_nIdentifier) == identifier) {
			// Preserve original conditions, resource calls and borrowed-position mutations
			pMarker = marker;
			break;
		}
	}

	if (pMarker == NULL) {
		// Preserve original conditions, resource calls and borrowed-position mutations
		for (int i = 0; i < NUM3DMARKERS; i++) {
			// Preserve original conditions, resource calls and borrowed-position mutations
        void *marker=Markers3D_BackendMarker(i);Markers3DFields fields=Markers3D_BackendFields(marker);
			if ((*fields.m_nType) == MARKERTYPE_INVALID) {
				// Preserve original conditions, resource calls and borrowed-position mutations
				pMarker = marker;
				break;
			}
		}
	}

	if (pMarker == NULL && type == MARKERTYPE_ARROW) {
		// Preserve original conditions, resource calls and borrowed-position mutations
		for (int i = 0; i < NUM3DMARKERS; i++) {
			// Preserve original conditions, resource calls and borrowed-position mutations
        void *marker=Markers3D_BackendMarker(i);Markers3DFields fields=Markers3D_BackendFields(marker);
			if (dist < (*fields.m_fCameraRange) && (*fields.m_nType) == MARKERTYPE_ARROW && (pMarker == NULL || (*fields.m_fCameraRange) > (*Markers3D_BackendFields(pMarker).m_fCameraRange))) {
				// Preserve original conditions, resource calls and borrowed-position mutations
				pMarker = marker;
				break;
			}
		}

		if (pMarker != NULL)
			(*Markers3D_BackendFields(pMarker).m_nType) = MARKERTYPE_INVALID;
	}

	if (pMarker == NULL) return pMarker;
    Markers3DFields placed=Markers3D_BackendFields(pMarker);

	(*placed.m_fCameraRange) = dist;
	if ((*placed.m_nIdentifier) == identifier && (*placed.m_nType) == type) {
		// Preserve original conditions, resource calls and borrowed-position mutations
		if (type == MARKERTYPE_ARROW) {
			// Preserve original conditions, resource calls and borrowed-position mutations
			if (dist < 25.0f) {
				// Preserve original conditions, resource calls and borrowed-position mutations
				if (dist > 5.0f)
					(*placed.m_fStdSize) = size - (25.0f - dist) * (0.3f * size) / 20.0f;
				else
					(*placed.m_fStdSize) = size - 0.3f * size;
			} else {
				// Preserve original conditions, resource calls and borrowed-position mutations
				(*placed.m_fStdSize) = size;
			}
		} else if (type == MARKERTYPE_CYLINDER) {
			// Preserve original conditions, resource calls and borrowed-position mutations
			if (dist < size + 12.0f) {
				// Preserve original conditions, resource calls and borrowed-position mutations
				if (dist > size + 1.0f)
					(*placed.alpha) = (1.0f - (size + 12.0f - dist) * 0.7f / 11.0f) * (float)a;
				else
					(*placed.alpha) = (float)a * 0.3f;
			} else {
				// Preserve original conditions, resource calls and borrowed-position mutations
				(*placed.alpha) = a;
			}
		}
		float someSin = sinf(TWOPI * (float)(((*placed.m_nPulsePeriod) - 1) & ((*Timer_GetTimeInMilliseconds()) - (*placed.m_nStartTime))) / (float)(*placed.m_nPulsePeriod));
		(*placed.m_fSize) = (*placed.m_fStdSize) - pulseFraction * (*placed.m_fStdSize) * someSin;

		if (type == MARKERTYPE_ARROW) {
			// Preserve original conditions, resource calls and borrowed-position mutations
			Markers3D_AddZ(pos,0.25f * (*placed.m_fStdSize) * someSin,read,write);
		} else if (type == MARKERTYPE_0) {
			// Preserve original conditions, resource calls and borrowed-position mutations
			if (someSin > 0.0f)
				(*placed.alpha) = (float)a * 0.7f * someSin + a;
			else
				(*placed.alpha) = (float)a * 0.4f * someSin + a;
		}
		if ((*placed.m_nRotateRate) != 0) {
			// Preserve original conditions, resource calls and borrowed-position mutations
            Markers3DVector savedPosition=Markers3D_BackendPosition(pMarker);
            Markers3D_BackendRotateZ(pMarker,DEGTORAD((*placed.m_nRotateRate) * (*Timer_GetTimeStep())));
            Markers3D_BackendSetPosition(pMarker,savedPosition);
        }
		if (type == MARKERTYPE_ARROW)
			Markers3D_BackendSetPosition(pMarker,Markers3D_ReadPosition(pos,read));

		if ((*placed.m_bFindZOnNextPlacement)) {
			// Preserve original conditions, resource calls and borrowed-position mutations
			if (Markers3D_Squared2D(Markers3D_Subtract2D(playerPos,Markers3D_ReadPosition2D(pos,read))) < (100.f*100.f) && Markers3D_BackendCollision(Markers3D_ReadPosition2D(pos,read))) {
				// Preserve original conditions, resource calls and borrowed-position mutations
				float z = Markers3D_BackendGround(read(pos,0),read(pos,1),read(pos,2)+1.0f);
				if (z != 0.0f)
					Markers3D_BackendSetZ(pMarker,z - 0.05f * size);
				(*placed.m_bFindZOnNextPlacement) = false;
			}
		}
		(*placed.m_bIsUsed) = true;
		return pMarker;
	}

	if ((*placed.m_nIdentifier) != 0)
		Markers3D_RecordDelete(pMarker);

	Markers3D_RecordAdd(pMarker,identifier, type, size, r, g, b, a, pulsePeriod, pulseFraction, rotateRate);
	if (type == MARKERTYPE_CYLINDER || type == MARKERTYPE_0 || type == MARKERTYPE_2) {
		// Preserve original conditions, resource calls and borrowed-position mutations
		if (Markers3D_Squared2D(Markers3D_Subtract2D(playerPos,Markers3D_ReadPosition2D(pos,read))) < (100.f*100.f) && Markers3D_BackendCollision(Markers3D_ReadPosition2D(pos,read))) {
			// Preserve original conditions, resource calls and borrowed-position mutations
			float z = Markers3D_BackendGround(read(pos,0),read(pos,1),read(pos,2)+1.0f);
			if (z != 0.0f)
				write(pos,2,z - 0.05f * size);
			(*placed.m_bFindZOnNextPlacement) = false;
		} else {
			// Preserve original conditions, resource calls and borrowed-position mutations
			(*placed.m_bFindZOnNextPlacement) = true;
		}
	}
	Markers3D_BackendSetTranslate(pMarker,Markers3D_ReadPosition(pos,read));
	if (type == MARKERTYPE_2) {
		// Preserve original conditions, resource calls and borrowed-position mutations
		Markers3D_BackendRotateX(pMarker,PI);
		Markers3D_BackendSetPosition(pMarker,Markers3D_ReadPosition(pos,read));
	}
	Markers3D_BackendUpdateRW(pMarker);
	if (type == MARKERTYPE_ARROW) {
		// Preserve original conditions, resource calls and borrowed-position mutations
		if (dist < 25.0f) {
			// Preserve original conditions, resource calls and borrowed-position mutations
			if (dist > 5.0f)
				(*placed.m_fStdSize) = size - (25.0f - dist) * (0.3f * size) / 20.0f;
			else
				(*placed.m_fStdSize) = size - 0.3f * size;
		} else {
			// Preserve original conditions, resource calls and borrowed-position mutations
			(*placed.m_fStdSize) = size;
		}
	} else if (type == MARKERTYPE_CYLINDER) {
		// Preserve original conditions, resource calls and borrowed-position mutations
		if (dist < size + 12.0f) {
			// Preserve original conditions, resource calls and borrowed-position mutations
			if (dist > size + 1.0f)
				(*placed.alpha) = (1.0f - (size + 12.0f - dist) * 0.7f / 11.0f) * (float)a;
			else
				(*placed.alpha) = (float)a * 0.3f;
		} else {
			// Preserve original conditions, resource calls and borrowed-position mutations
			(*placed.alpha) = a;
		}
	}
	(*placed.m_bIsUsed) = true;
	return pMarker;
}

static void
Markers3D_SetInput(uint32_t id,uint16_t type,void *pos,float size,uint8_t r,uint8_t g,uint8_t b,uint8_t a,uint16_t pulsePeriod,float pulseFraction,int16_t rotateRate,Markers3DRead read,Markers3DWrite write)
{
    // Retain the complete original controller operation and live data-write order
	Markers3D_PlaceInput(id, type, pos, size, r, g, b, a, pulsePeriod, pulseFraction, 1,read,write);
	Markers3D_PlaceInput(id, type, pos, size * 0.93f, r, g, b, a, pulsePeriod, pulseFraction, 2,read,write);
	Markers3D_PlaceInput(id, type, pos, size * 0.86f, r, g, b, a, pulsePeriod, pulseFraction, -1,read,write);
}


void
Markers3D_Update(void)
{
    // Retain the complete original controller operation and live data-write order
}

void * Markers3D_PlaceMarker(uint32_t identifier,uint16_t type,Markers3DVector *pos,float size,uint8_t r,uint8_t g,uint8_t b,uint8_t a,uint16_t pulsePeriod,float pulseFraction,int16_t rotateRate)
{
    // Preserve all original lazy reads and in-place reference writes across complete placement calls
    return Markers3D_PlaceInput(identifier,type,pos,size,r,g,b,a,pulsePeriod,pulseFraction,rotateRate,Markers3D_Read,Markers3D_Write);
}
void * Markers3D_PlaceMarkerLegacyInput(uint32_t identifier,uint16_t type,void *pos,float size,uint8_t r,uint8_t g,uint8_t b,uint8_t a,uint16_t pulsePeriod,float pulseFraction,int16_t rotateRate)
{
    // Preserve all original lazy reads and in-place reference writes across complete placement calls
    return Markers3D_PlaceInput(identifier,type,pos,size,r,g,b,a,pulsePeriod,pulseFraction,rotateRate,Markers3D_BackendReadLegacy,Markers3D_BackendWriteLegacy);
}
void Markers3D_PlaceMarkerSet(uint32_t identifier,uint16_t type,Markers3DVector *pos,float size,uint8_t r,uint8_t g,uint8_t b,uint8_t a,uint16_t pulsePeriod,float pulseFraction,int16_t rotateRate)
{
    // Preserve all original lazy reads and in-place reference writes across complete placement calls
    Markers3D_SetInput(identifier,type,pos,size,r,g,b,a,pulsePeriod,pulseFraction,rotateRate,Markers3D_Read,Markers3D_Write);
}
void Markers3D_PlaceMarkerSetLegacyInput(uint32_t identifier,uint16_t type,void *pos,float size,uint8_t r,uint8_t g,uint8_t b,uint8_t a,uint16_t pulsePeriod,float pulseFraction,int16_t rotateRate)
{
    // Preserve all original lazy reads and in-place reference writes across complete placement calls
    Markers3D_SetInput(identifier,type,pos,size,r,g,b,a,pulsePeriod,pulseFraction,rotateRate,Markers3D_BackendReadLegacy,Markers3D_BackendWriteLegacy);
}

static void Markers3D_RenderMatrix(void *marker,void *matrix);

bool
Markers3D_RecordAdd(void *marker,uint32_t identifier,uint16_t type,float fSize,uint8_t r,uint8_t g,uint8_t b,uint8_t a,uint16_t pulsePeriod,float pulseFraction,int16_t rotateRate)
{
    // Preserve each original resource operation and live field-write order
    Markers3DFields fields=Markers3D_BackendFields(marker);
	(*fields.m_nIdentifier) = identifier;

	Markers3D_BackendUnity(marker);

	void *origAtomic=Markers3D_BackendLastAtomic(Markers3D_BackendClump(type));

	void *atomic = Markers3D_BackendCloneAtomic(origAtomic);
	void *frame = Markers3D_BackendCreateFrame();
	Markers3D_BackendSetFrame(atomic,frame);
	Markers3D_BackendDefaultCallback(atomic);
	
	void *geometry = Markers3D_BackendAtomicGeometry(atomic);
	Markers3D_BackendSetGeometryFlags(geometry,Markers3D_BackendGeometryFlags(geometry)|MARKERS3D_GEOMETRY_MODULATE_MATERIAL_COLOR);

	Markers3D_BackendSetAtomic(marker,atomic);
	Markers3D_BackendAttachMarker(marker,Markers3D_BackendFrameMatrix(Markers3D_BackendAtomicFrame(Markers3D_BackendAtomic(marker))));
	Markers3D_BackendSetMaterial(marker,Markers3D_BackendGeometryMaterial(geometry,0));
	(*fields.m_fSize) = fSize;
	(*fields.m_fStdSize) = (*fields.m_fSize);
	(*fields.red) = r;
	(*fields.green) = g;
	(*fields.blue) = b;
	(*fields.alpha) = a;
	(*fields.m_nPulsePeriod) = pulsePeriod;
	(*fields.m_fPulseFraction) = pulseFraction;
	(*fields.m_nRotateRate) = rotateRate;
	(*fields.m_nStartTime) = (*Timer_GetTimeInMilliseconds());
	(*fields.m_nType) = type;
	return Markers3D_BackendAtomic(marker) != NULL;
}

void
Markers3D_RecordDelete(void *marker)
{
    // Preserve each original resource operation and live field-write order
    Markers3DFields fields=Markers3D_BackendFields(marker);
	void *frame;

	(*fields.m_nIdentifier) = 0;
	(*fields.m_nStartTime) = 0;
	(*fields.m_bIsUsed) = false;
	(*fields.m_bFindZOnNextPlacement) = false;
	(*fields.m_nType) = MARKERTYPE_INVALID;

	frame = Markers3D_BackendAtomicFrame(Markers3D_BackendAtomic(marker));
	Markers3D_BackendDestroyAtomic(Markers3D_BackendAtomic(marker));
	Markers3D_BackendDestroyFrame(frame);
	Markers3D_BackendSetAtomic(marker,NULL);
}

void
Markers3D_RecordRender(void *marker)
{
    // Preserve each original resource operation and live field-write order
    Markers3DFields fields=Markers3D_BackendFields(marker);
	if (Markers3D_BackendAtomic(marker) == NULL) return;

	Markers3D_BackendMaterialColor(marker);

	Markers3D_BackendUpdateRW(marker);

	Markers3D_BackendWithRenderMatrix(marker,Markers3D_RenderMatrix);
}

static void
Markers3D_RenderMatrix(void *marker,void *matrix)
{
    // Preserve each original resource operation and live field-write order
    Markers3DFields fields=Markers3D_BackendFields(marker);
    Markers3D_BackendAttachMatrix(matrix,Markers3D_BackendAttachment(marker));
    float scale=(*fields.m_fSize);
    Markers3DBasis basis=Markers3D_BackendMatrixBasis(matrix);
    for(int i=0;i<3;i++){
        // Scale the original nine basis coefficients in their original order
        for(int j=0;j<3;j++)*basis.axis[i][j]*=scale;
    }
    Markers3D_BackendPublishMatrix(matrix);

	Markers3D_BackendUpdateFrame(Markers3D_BackendAtomicFrame(Markers3D_BackendAtomic(marker)));
	Markers3D_BackendBright((*fields.m_fBrightness));
	if ((*fields.m_nType) != MARKERTYPE_ARROW)
		Markers3D_BackendZWrite(false);
	Markers3D_BackendDrawAtomic(Markers3D_BackendAtomic(marker));
	if ((*fields.m_nType) != MARKERTYPE_ARROW)
		Markers3D_BackendZWrite(true);
	Markers3D_BackendRestoreLighting();
}
//- rouz edit (ChatGPT)
