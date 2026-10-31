"""Expose live scalar sphere receivers to C peers without aliasing C records as SDK sphere objects."""
import re

def header(source):
    # Extend the existing C helper API while retaining the original SDK-receiver entry point
    if 'typedef struct RwHelperSphereRef' in source:return source
    pattern=r'(?m)^(void\s*\*\s*RpClumpGetBoundingSphere\b)';assert len(re.findall(pattern,source))==1
    return re.sub(pattern,'typedef struct RwHelperSphereRef {float *x,*y,*z,*radius;} RwHelperSphereRef;\nvoid *RwHelper_ClumpSphereView(void *clump,RwHelperSphereRef sphere,bool useLTM);\n'+r'\1',source)

def producer(source):
    # Keep the complete original bounding algorithm and SDK scratch objects while writing canonical C receiver fields
    if re.search(r'void\s*\*\s*RwHelper_ClumpSphereView\s*\(',source):return source
    first=source.index('typedef struct RwHelper_RpClumpGetBoundingSphereScope');last=source.index('\nvoid CameraSize(',first);value=source[first:last]
    value=value.replace('RwHelper_RpClumpGetBoundingSphereScope','RwHelper_ClumpSphereViewScope').replace('RwHelper_RpClumpGetBoundingSphereBody','RwHelper_ClumpSphereViewBody').replace('void * sphere;bool useLTM;','RwHelperSphereRef sphere;bool useLTM;').replace('void * sphere=scope->sphere;','RwHelperSphereRef sphere=scope->sphere;').replace('sphere == nil','sphere.radius == nil')
    value=value.replace('(*RwHelper_HostSphereRadius(sphere))','(*sphere.radius)')
    for field,index in [('x',0),('y',1),('z',2)]:value=value.replace('(*RwHelper_HostVectorField(RwHelper_HostSphereCenter(sphere),'+str(index)+'))','(*sphere.'+field+')')
    old='RwHelper_HostCopySphere(sphere,result);';assert old in value
    value=value.replace(old,'''// Copy defined SDK result fields into the actual caller receiver without casting its containing C record
    void *resultCenter=RwHelper_HostSphereCenter(result);
    memcpy(sphere.x,RwHelper_HostVectorField(resultCenter,0),sizeof(float));
    memcpy(sphere.y,RwHelper_HostVectorField(resultCenter,1),sizeof(float));
    memcpy(sphere.z,RwHelper_HostVectorField(resultCenter,2),sizeof(float));
    memcpy(sphere.radius,RwHelper_HostSphereRadius(result),sizeof(float));''')
    old='RpClumpGetBoundingSphere(void * clump,void * sphere,bool useLTM)';assert old in value
    value=value.replace(old,'RwHelper_ClumpSphereView(void * clump,RwHelperSphereRef sphere,bool useLTM)')
    value+='''
void *RpClumpGetBoundingSphere(void *clump,void *sphere,bool useLTM)
{
    // Retain the original SDK-receiver ABI and ignored-pointer behavior for null clumps
    RwHelperSphereRef receiver={NULL,NULL,NULL,NULL};
    if(clump&&sphere){
        // Borrow actual SDK scalar subobjects while the complete policy remains in the C view operation
        void *center=RwHelper_HostSphereCenter(sphere);receiver.x=RwHelper_HostVectorField(center,0);receiver.y=RwHelper_HostVectorField(center,1);receiver.z=RwHelper_HostVectorField(center,2);receiver.radius=RwHelper_HostSphereRadius(sphere);
    }
    return RwHelper_ClumpSphereView(clump,receiver,useLTM);
}
'''
    result=source[:first]+value+source[last:]
    return result.replace('#include <stddef.h>','#include <stddef.h>\n#include <string.h>')
