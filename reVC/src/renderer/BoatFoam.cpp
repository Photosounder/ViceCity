//+ rouz edit (ChatGPT)
#include "common.h"
#include "BoatFoam_backend.h"
#include "Camera.h"
#include "Particle.h"
CRGBA FoamColour(255,255,255,255);
BoatFoamVector BoatFoam_BackendCameraForward(void)
{
    // Read only the original two camera components used by the 2D dot product
    const CVector &v=TheCamera.GetForward();
    BoatFoamVector result={v.x,v.y,0.0f};return result;
}
BoatFoamVector BoatFoam_BackendMatrixPosition(void *matrix)
{
    // Copy actual typed matrix Position fields at their original use
    const CVector &v=((CMatrix *)matrix)->GetPosition();
    BoatFoamVector result={v.x,v.y,v.z};return result;
}
BoatFoamVector BoatFoam_BackendMatrixForward(void *matrix)
{
    // Copy actual typed matrix Forward fields at their original use
    const CVector &v=((CMatrix *)matrix)->GetForward();
    BoatFoamVector result={v.x,v.y,v.z};return result;
}
BoatFoamVector BoatFoam_BackendMatrixRight(void *matrix)
{
    // Copy actual typed matrix Right fields at their original use
    const CVector &v=((CMatrix *)matrix)->GetRight();
    BoatFoamVector result={v.x,v.y,v.z};return result;
}
BoatFoamVector BoatFoam_BackendMatrixUp(void *matrix)
{
    // Copy actual typed matrix Up fields at their original use
    const CVector &v=((CMatrix *)matrix)->GetUp();
    BoatFoamVector result={v.x,v.y,v.z};return result;
}
BoatFoamMatrix BoatFoam_BackendReadMatrix(void *matrix)
{
    // Copy original named transform coefficients without assuming matrix layout
    const CMatrix &m=*(CMatrix *)matrix;
    BoatFoamMatrix result={m.rx,m.ry,m.rz,m.fx,m.fy,m.fz,m.ux,m.uy,m.uz,m.px,m.py,m.pz};return result;
}
void BoatFoam_BackendParticle(int32_t type,BoatFoamVector position,float size)
{
    // Enter the exact typed particle overload while retaining its live original color reference
    CParticle::AddParticle((tParticleType)type,CVector(position.x,position.y,position.z),CVector(0.0f,0.0f,0.0f),nil,size,FoamColour,1,0,0,0);
}
//- rouz edit (ChatGPT)
