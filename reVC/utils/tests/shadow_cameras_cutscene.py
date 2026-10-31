"""Project complete cutscene creation and update through genuine C sphere storage and SDK scalar adapters."""
import re
from rect_migration import mask,closing,arguments
from memory_heap_migration import body
from shadow_cameras_schema import CUTSCENE_FIELDS
import shadow_cameras_resources as resources

def switches(source):
    # Preserve each original selector snapshot while using SDK identities supplied by the private C boundary
    pattern=r'switch\s*\(\s*self->m_nRwObjectType\s*\)\s*\{'
    while True:
        match=re.search(pattern,mask(source))
        if not match:return source
        code=mask(source);first=match.end();depth=1;end=first
        while depth:
            if code[end]=='{':depth+=1
            elif code[end]=='}':depth-=1
            end+=1
        value=source[first:end-1];labels=list(re.finditer(r'\bcase\s+(rpCLUMP|rpATOMIC)\s*:|\bdefault\s*:',mask(value)));assert labels
        pieces=[]
        for index,label in enumerate(labels):
            # Retain complete branch bodies and remove only the switch-local terminating break statements
            branch=value[label.end():labels[index+1].start() if index+1<len(labels) else len(value)];assert not re.search(r'\b(?:for|while|switch)\s*\(',mask(branch));branch=re.sub(r'\bbreak\s*;','',branch)
            if not branch.strip().startswith('{'):branch='{'+branch+'}'
            condition=('if' if not pieces else 'else if')+'(objectType==Shadow_HostCutsceneConstant(SHADOW_CUT_CONST_'+label[1]+'))' if label[1] else 'else'
            pieces.append(condition+branch)
        replacement='{\n    // Capture the canonical object type once at the original switch boundary\n    int32_t objectType=self->m_nRwObjectType;\n'+''.join(pieces)+'\n}'
        source=source[:match.start()]+replacement+source[end:]

def member_calls(source):
    # Resolve former camera member methods without caching canonical camera handles or changing overload selection
    pattern=r'self->(m_Camera|m_ResampleCamera|m_BlurCamera|m_GradientCamera)\.(\w+)\s*\('
    while True:
        match=re.search(pattern,mask(source))
        if not match:return source
        opening=source.index('(',match.start());end=closing(mask(source),opening);args=arguments(source[opening+1:end-1]);args=[] if args==[''] else args;method=match[2]
        if method=='Update':method='UpdateClump' if 'm_pClump' in ','.join(args) else 'UpdateAtomic' if 'm_pAtomic' in ','.join(args) else None;assert method
        if method=='SetCenter':assert len(args)==1;args=['Shadow_VectorRef('+args[0]+')']
        value='ShadowCamera_'+method+'(&self->'+match[1]+(' ,'+','.join(args) if args else '')+')';source=source[:match.start()]+value+source[end:]

def methods(original):
    # Keep full original cutscene policy in C and preserve live output writes through the C helper peer
    result=[]
    for name,returns,parameters in [('Create','bool',',void *object,int32_t rasterSize,bool resample,int32_t blurPasses,bool gradient'),('Update','void *','')]:
        value=body(original,'CCutsceneShadow::'+name+'(')
        for field in CUTSCENE_FIELDS+['m_pAtomic','m_pClump']:value=re.sub(r'\b'+field+r'\b','self->'+field,value)
        value=value.replace('RwRGBAReal','ShadowLightColour').replace('DLIGHT_VALUE','0.8f')
        value=value.replace('RpClumpGetBoundingSphere(self->m_pClump, &self->m_BoundingSphere, 1)','RwHelper_ClumpSphereView(self->m_pClump,Shadow_SphereRef(&self->m_BoundingSphere),true)')
        value=value.replace('self->m_BoundingSphere = *RpAtomicGetBoundingSphere(self->m_pAtomic);','self->m_BoundingSphere=Shadow_HostReadSphere(RwHelper_HostAtomicSphere(self->m_pAtomic));')
        calls=[];code=mask(value)
        for match in re.finditer(r'\bRwV3dTransformPoints\s*\(',code):
            # Adapt the original SDK point transform at its call boundary without treating owned C vectors as SDK objects
            opening=code.index('(',match.start());end=closing(code,opening);args=arguments(value[opening+1:end-1]);assert len(args)==4 and args[2]=='1';calls.append((match.start(),end,'Shadow_HostTransformVector('+','.join([args[0],args[1],args[3]])+')'))
        for start,end,replacement in reversed(calls):value=value[:start]+replacement+value[end:]
        aliases={**resources.ALIASES,'RpLightCreate':'Shadow_HostLightCreate','RpLightSetColor':'Shadow_HostLightColour','RwObjectGetType':'Shadow_HostObjectType','RpClumpGetFrame':'RwHelper_HostClumpFrame','RpAtomicGetFrame':'Shadow_HostAtomicFrame'}
        for old,new in aliases.items():value=re.sub(r'\b'+old+r'\b',new,value)
        value=value.replace('SetLightProperties(180.0f, 90.0f, false)','CutsceneShadow_SetLightProperties(self,180.0f,90.0f,false)').replace('Destroy();','CutsceneShadow_Destroy(self);')
        value=member_calls(switches(value))
        value=value.replace('((RpClump*)RwHelper_ClumpSphereView','(void)(RwHelper_ClumpSphereView')
        for old in ['RwFrame','RwRaster','RpClump','RpAtomic']:value=re.sub(r'\b'+old+r'\b','void',value)
        value=value.replace('rpLIGHTDIRECTIONAL','Shadow_HostCutsceneConstant(SHADOW_CUT_CONST_rpLIGHTDIRECTIONAL)')
        assert not re.search(r'\b(?:rpCLUMP|rpATOMIC|RwV3dTransformPoints|RpAtomicGetBoundingSphere)\b',mask(value))
        result.append(returns+' CutsceneShadow_'+name+'(CutsceneShadow *self'+parameters+')\n{\n    // Retain the complete original cutscene lifecycle or update policy with canonical C sphere records\n'+value+'\n}\n')
    return '\n'.join(result)

def extend(header,native):
    # Keep SDK record construction and primitive math at a typed boundary with no application routing decisions
    extra='''typedef struct ShadowLightColour {float red,green,blue,alpha;} ShadowLightColour;
void *Shadow_HostLightCreate(int32_t kind);
void Shadow_HostLightColour(void *light,const ShadowLightColour *colour);
int32_t Shadow_HostObjectType(void *object);
void *Shadow_HostAtomicFrame(void *atomic);
ShadowSphere Shadow_HostReadSphere(void *sphere);
void Shadow_HostTransformVector(ShadowVector *destination,const ShadowVector *source,void *matrix);
enum {SHADOW_CUT_CONST_rpLIGHTDIRECTIONAL,SHADOW_CUT_CONST_rpCLUMP,SHADOW_CUT_CONST_rpATOMIC};
int32_t Shadow_HostCutsceneConstant(int32_t identity);
static inline RwHelperSphereRef Shadow_SphereRef(ShadowSphere *sphere)
{
    // Borrow actual C scalar receiver fields while leaving all SDK scratch storage inside the helper peer
    RwHelperSphereRef value={NULL,NULL,NULL,NULL};if(sphere){value.x=&sphere->center.x;value.y=&sphere->center.y;value.z=&sphere->center.z;value.radius=&sphere->radius;}return value;
}
'''
    definitions='''#include <string.h>
void *Shadow_HostLightCreate(int32_t kind)
{
    // Preserve the original SDK directional light allocation primitive
    return RpLightCreate(kind);
}
void Shadow_HostLightColour(void *light,const ShadowLightColour *colour)
{
    // Construct the actual SDK colour record without aliasing an unrelated C record type
    RwRGBAReal value={colour->red,colour->green,colour->blue,colour->alpha};RpLightSetColor((RpLight*)light,&value);
}
int32_t Shadow_HostObjectType(void *object)
{
    // Return the original SDK object type code for canonical application storage
    return RwObjectGetType((RwObject*)object);
}
void *Shadow_HostAtomicFrame(void *atomic)
{
    // Preserve the actual SDK atomic frame lookup
    return RpAtomicGetFrame((RpAtomic*)atomic);
}
ShadowSphere Shadow_HostReadSphere(void *sphere)
{
    // Copy only actual SDK sphere field representations into the owned C record
    const RwSphere *source=(RwSphere*)sphere;ShadowSphere value;memcpy(&value.center.x,&source->center.x,sizeof(float));memcpy(&value.center.y,&source->center.y,sizeof(float));memcpy(&value.center.z,&source->center.z,sizeof(float));memcpy(&value.radius,&source->radius,sizeof(float));return value;
}
void Shadow_HostTransformVector(ShadowVector *destination,const ShadowVector *source,void *matrix)
{
    // Invoke one original SDK point transform using real SDK vectors and copy its completed result into C storage
    RwV3d input,output;memcpy(&input.x,&source->x,sizeof(float));memcpy(&input.y,&source->y,sizeof(float));memcpy(&input.z,&source->z,sizeof(float));RwV3dTransformPoints(&output,&input,1,(RwMatrix*)matrix);memcpy(&destination->x,&output.x,sizeof(float));memcpy(&destination->y,&output.y,sizeof(float));memcpy(&destination->z,&output.z,sizeof(float));
}
int32_t Shadow_HostCutsceneConstant(int32_t identity)
{
    // Translate SDK object and light identities without changing canonical type values or application selection
    switch(identity){case SHADOW_CUT_CONST_rpLIGHTDIRECTIONAL:return rpLIGHTDIRECTIONAL;case SHADOW_CUT_CONST_rpCLUMP:return rpCLUMP;case SHADOW_CUT_CONST_rpATOMIC:return rpATOMIC;}return 0;
}
'''
    header=header.replace('#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)',extra+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)')
    return header,native+'\n//+ rouz edit (ChatGPT)\n'+definitions+'//- rouz edit (ChatGPT)\n'
