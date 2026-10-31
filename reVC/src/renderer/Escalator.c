//+ rouz edit (ChatGPT)
#include "Escalator_backend.h"
#include "Timer.h"
#include <math.h>
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
Escalator Escalators_aEscalators[NUM_ESCALATORS];
bool deletingEscalator;
static EscalatorVector Escalator_Subtract(EscalatorVector a,EscalatorVector b)
{
    // Keep each original component subtraction before vector magnitude or speed calculation
    EscalatorVector result={a.x-b.x,a.y-b.y,a.z-b.z};return result;
}
static EscalatorVector Escalator_Add(EscalatorVector a,EscalatorVector b)
{
    // Preserve the original left-associated component additions
    EscalatorVector result={a.x+b.x,a.y+b.y,a.z+b.z};return result;
}
static EscalatorVector Escalator_Divide(EscalatorVector a,float divisor)
{
    // Divide each original component directly instead of multiplying by a reciprocal
    EscalatorVector result={a.x/divisor,a.y/divisor,a.z/divisor};return result;
}
static EscalatorVector Escalator_LeftScale(float factor,EscalatorVector a)
{
    // Retain scalar-left multiplication in the original interpolation expressions
    EscalatorVector result={factor*a.x,factor*a.y,factor*a.z};return result;
}
static float Escalator_Magnitude(EscalatorVector a)
{
    // Preserve the exact original square-root argument and component-product order
    return sqrtf(a.x*a.x+a.y*a.y+a.z*a.z);
}
static void Escalator_Normalise(EscalatorVector *a)
{
    // Preserve the native normalizer including its x-only zero-vector fallback
    float sq=a->x*a->x+a->y*a->y+a->z*a->z;if(sq>0.0f){
        // Multiply each original component by the reciprocal square root
        float inverse=1.0f/sqrtf(sq);a->x*=inverse;a->y*=inverse;a->z*=inverse;
    }else a->x=1.0f;
}
void
Escalator_AddThisOne(int32_t index,EscalatorVector pos0,EscalatorVector pos1,EscalatorVector pos2,EscalatorVector pos3,bool b_isMovingDown) {
    // Preserve every original live field read, allocation decision and state-write order
    Escalator *escalator=&Escalators_aEscalators[index];
	escalator->m_pos0 = pos0;
	escalator->m_pos1 = pos1;
	escalator->m_pos2 = pos2;
	escalator->m_pos3 = pos3;

	float escalatorStepHeight = Escalator_BackendStepHeight();
	escalator->m_pos0.z -= escalatorStepHeight;
	escalator->m_pos1.z -= escalatorStepHeight;
	escalator->m_pos2.z -= escalatorStepHeight;
	escalator->m_pos3.z -= escalatorStepHeight;

	float magnitudes[3];
	magnitudes[0] = Escalator_Magnitude(Escalator_Subtract(escalator->m_pos0,escalator->m_pos1));
	magnitudes[1] = Escalator_Magnitude(Escalator_Subtract(escalator->m_pos1,escalator->m_pos2));
	magnitudes[2] = Escalator_Magnitude(Escalator_Subtract(escalator->m_pos2,escalator->m_pos3));

	float length = magnitudes[0] + magnitudes[1] + magnitudes[2];

	escalator->m_lowerEnd = magnitudes[0] / length;
	escalator->m_upperEnd = (magnitudes[0] + magnitudes[1]) / length;

	escalator->m_stepsCount = Min(24.0f, length / 0.6f);

	EscalatorVector direction={escalator->m_pos0.x-escalator->m_pos1.x,escalator->m_pos0.y-escalator->m_pos1.y,0.0f};
	Escalator_Normalise(&direction);

	Escalator_BackendMatrixUp(index,(EscalatorVector){0.0f, 0.0f, 1.0f});
	Escalator_BackendMatrixForward(index,(EscalatorVector){direction.x, direction.y, 0.0f});
	Escalator_BackendMatrixRight(index,(EscalatorVector){direction.y, -direction.x, 0.0f});
	Escalator_BackendMatrixPosition(index,(EscalatorVector){0.0f, 0.0f, 0.0f});

	escalator->m_bIsMovingDown = b_isMovingDown;

	escalator->m_midPoint = Escalator_Divide(Escalator_Add(escalator->m_pos0,escalator->m_pos3),2.0f);

	escalator->m_radius = Escalator_Magnitude(Escalator_Subtract(escalator->m_pos0,escalator->m_midPoint));
}

void
Escalator_Update(int32_t index) {
    // Preserve every original live field read, allocation decision and state-write order
    Escalator *escalator=&Escalators_aEscalators[index];
	if (!escalator->m_bIsActive) {
		// Preserve the original branch, callback and traversal order
		if (Escalator_Magnitude(Escalator_Subtract(Escalator_BackendCameraPosition(),escalator->m_midPoint)) < 25.0f) {
			// Preserve the original branch, callback and traversal order
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			if (Escalator_BackendSphereVisible(escalator->m_midPoint, escalator->m_radius) && (escalator->m_stepsCount + 10 < CPool_GetNoOfFreeSpaces(Escalator_BackendObjectPool()))) { 
			//- rouz edit (ChatGPT)
				escalator->m_bIsActive = true;
				for (int i = 0; i < escalator->m_stepsCount; i++) {
					// Preserve the original branch, callback and traversal order
//+ rouz edit (ChatGPT)
					// Allocate the escalator step without invoking C++ new.
					CPool *objectPool = Escalator_BackendObjectPool();
					//+ rouz edit (ChatGPT)
					// Access raw storage through the C store or pool API
					escalator->m_pSteps[i] = ((CObject*)CPool_New(objectPool));
					//- rouz edit (ChatGPT)
#ifdef FIX_BUGS
					if (!escalator->m_pSteps[i]) {
						// Preserve the original branch, callback and traversal order
						//+ rouz edit (ChatGPT)
						// Access raw storage through the C store or pool API
						for (int32_t j = 0; j < CPool_GetSize(objectPool); j++) {
							// Preserve the original branch, callback and traversal order
							// Access raw storage through the C store or pool API
							CObject *existing = ((CObject*)CPool_GetSlot(objectPool, j));
						//- rouz edit (ChatGPT)
							if (existing && Escalator_BackendCreatedBy(existing) == ESCALATOR_TEMP_OBJECT) {
								// Preserve the original branch, callback and traversal order
								//+ rouz edit (ChatGPT)
								// Access raw storage through the C store or pool API
								int32_t handle = CPool_GetIndex(objectPool, existing);
								//- rouz edit (ChatGPT)
								Escalator_BackendWorldRemove(existing);
								Escalator_BackendDestroy(existing);
								//+ rouz edit (ChatGPT)
								// Access raw storage through the C store or pool API
								CPool_Delete(objectPool, existing);
								escalator->m_pSteps[i] = ((CObject*)CPool_NewAt(objectPool, handle));
								//- rouz edit (ChatGPT)
								break;
							}
						}
					}
					if (escalator->m_pSteps[i])
#endif
						Escalator_BackendConstruct(escalator->m_pSteps[i]);
//- rouz edit (ChatGPT)
					if (escalator->m_pSteps[i]) {
						// Preserve the original branch, callback and traversal order
						Escalator_BackendSetPosition(escalator->m_pSteps[i],escalator->m_pos1);
						Escalator_BackendWorldAdd(escalator->m_pSteps[i]);
						Escalator_BackendSetCreatedBy(escalator->m_pSteps[i],ESCALATOR_CONTROLLED_SUB_OBJECT);
					}
				}
			}
		}
	}

	if (escalator->m_bIsActive) {
		// Preserve the original branch, callback and traversal order
		float time = ((*Timer_GetTimeInMilliseconds()) % 16384) / 16384.0f;
		for (int i = 0; i < escalator->m_stepsCount; i++) {
			// Preserve the original branch, callback and traversal order
			if (escalator->m_pSteps[i]) {
				// Preserve the original branch, callback and traversal order
				float t = i / (float)escalator->m_stepsCount + time;

				if (t > 1.0f)
					t -= 1.0f;

				if (escalator->m_bIsMovingDown)
					t = 1.0f - t;

				EscalatorVector oldPosition=Escalator_BackendPosition(escalator->m_pSteps[i]);
				Escalator_BackendAssignMatrix(escalator->m_pSteps[i],index);

				EscalatorVector newPosition;
				if (t < escalator->m_lowerEnd) {
					// Preserve the original branch, callback and traversal order
					float ratio = t / escalator->m_lowerEnd;
					newPosition = Escalator_Add(Escalator_LeftScale(ratio,escalator->m_pos1),Escalator_LeftScale(1.0f-ratio,escalator->m_pos0));
				}
				else if (t < escalator->m_upperEnd) {
					// Preserve the original branch, callback and traversal order
					float ratio = (t - escalator->m_lowerEnd) / (escalator->m_upperEnd - escalator->m_lowerEnd);
					newPosition = Escalator_Add(Escalator_LeftScale(ratio,escalator->m_pos2),Escalator_LeftScale(1.0f-ratio,escalator->m_pos1));
				}
				else {
					// Preserve the original branch, callback and traversal order
					float ratio = (t - escalator->m_upperEnd) / (1.0f - escalator->m_upperEnd);
					newPosition = Escalator_Add(Escalator_LeftScale(ratio,escalator->m_pos3),Escalator_LeftScale(1.0f-ratio,escalator->m_pos2));
				}

				Escalator_BackendSetPosition(escalator->m_pSteps[i],newPosition);
				Escalator_BackendSetMoveSpeed(escalator->m_pSteps[i],Escalator_Divide(Escalator_Subtract(newPosition,oldPosition),Max((*Timer_GetTimeStep()),1.0f)));
				Escalator_BackendUpdateRW(escalator->m_pSteps[i]);
				Escalator_BackendUpdateFrame(escalator->m_pSteps[i]);
			}
			if (Escalator_Magnitude(Escalator_Subtract(Escalator_BackendCameraPosition(),escalator->m_midPoint)) > 28.0f || !Escalator_BackendSphereVisible(escalator->m_midPoint, escalator->m_radius))
				Escalator_SwitchOff(index);
		}
	}
}

void
Escalator_SwitchOff(int32_t index) {
    // Preserve every original live field read, allocation decision and state-write order
    Escalator *escalator=&Escalators_aEscalators[index];
	if (escalator->m_bIsActive) {
		// Preserve the original branch, callback and traversal order
		for (int i = 0; i < escalator->m_stepsCount; i++) {
			// Preserve the original branch, callback and traversal order
			if (escalator->m_pSteps[i]) {
				// Preserve the original branch, callback and traversal order
				Escalator_BackendWorldRemove(escalator->m_pSteps[i]);
				deletingEscalator = true;
//+ rouz edit (ChatGPT)
				// Destroy and release the escalator step without invoking C++ delete.
				Escalator_BackendDestroy(escalator->m_pSteps[i]);
				//+ rouz edit (ChatGPT)
				// Access raw storage through the C store or pool API
				CPool_Delete(Escalator_BackendObjectPool(), escalator->m_pSteps[i]);
				//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
				escalator->m_pSteps[i] = NULL;
				deletingEscalator = false;
			}
		}
		escalator->m_bIsActive = false;
	}
}
bool Escalator_IsActive(int32_t index)
{
    // Read the same live activation field used by the former inline audio getter
    return Escalators_aEscalators[index].m_bIsActive;
}
EscalatorVector Escalator_GetPosition(int32_t index)
{
    // Capture the same three midpoint components copied by the original audio caller
    return Escalators_aEscalators[index].m_midPoint;
}
//- rouz edit (ChatGPT)
