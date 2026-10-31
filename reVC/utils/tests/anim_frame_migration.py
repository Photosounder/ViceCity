"""Move the complete animation frame record to C and remove its native stride query."""
import re
from rect_migration import mask,closing
from doors_migration import method
import object_names_migration as objects
RENAMES={}
FLAGS={'IGNORE_ROTATION':2,'IGNORE_TRANSLATION':4,'VELOCITY_EXTRACTION':8,'VELOCITY_EXTRACTION_3D':16,'UPDATE_KEYFRAMES':32,'COMPRESSED':64}

def header():
    # Keep the complete original frame layout independent of SDK classes and scoped enums
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "KeyFrames.h"
enum {
'''+''.join('    ANIM_FRAME_'+name+'='+str(value)+',\n' for name,value in FLAGS.items())+'''};
typedef struct AnimBlendFrameData {
    uint8_t flag;
    KeyFrameVector3 resetPos;
    union {void *frame;void *hanimFrame;};
    int32_t nodeID;
} AnimBlendFrameData;
//- rouz edit (ChatGPT)
'''

def native():
    # Retain only live SDK handle recovery and exact native coordinate transfer
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include "AnimBlendFrameData.h"
#include <string.h>
static inline RwFrame *AnimBlendFrame_GetFrame(const AnimBlendFrameData *value)
{
    // Recover the original SDK object without introducing a synthetic frame view
    return static_cast<RwFrame*>(value->frame);
}
static inline RpHAnimStdInterpFrame *AnimBlendFrame_GetHAnim(const AnimBlendFrameData *value)
{
    // Keep writes through the actual live SDK pose object
    return static_cast<RpHAnimStdInterpFrame*>(value->hanimFrame);
}
static inline KeyFrameVector3 AnimBlendFrame_FromNativePosition(const RwV3d &value)
{
    // Transfer all native reset-position coordinate bits without arithmetic
    KeyFrameVector3 result;memcpy(&result.x,&value.x,4);memcpy(&result.y,&value.y,4);memcpy(&result.z,&value.z,4);return result;
}
//- rouz edit (ChatGPT)
'''

def anim_frame_calls(source):
    # Keep the completed C callback owner on opaque SDK handles without reintroducing native inline adapters
    if '#include "FrameUpdate_backend.h"' in source and re.search(r'(?m)^void FrameUpdateCallBackNonSkinned\(',source):return source
    # Preserve direct C frame storage after the complete animation plugin conversion
    if '#include "RpAnimBlend_backend.h"' in source and 'AnimPlugin_HostFramePosition(frameData->frame)' in source:return source
    # Replace the complete SDK-dependent record with its C definition and native-only value adapters
    original=source
    if 'struct AnimBlendFrameData\n' in source and 'RwV3d resetPos;' in source:return native()
    source=re.sub(r'\bAnimBlendFrameData::('+('|'.join(FLAGS))+r')\b',lambda m:'ANIM_FRAME_'+m[1],source)
    if 'typedef struct CAnimBlendClumpData {' in source:
        source=source.replace('#include "AnimBlendList.h"','#include "AnimBlendList.h"\n#include "AnimBlendFrameData.h"',1) if '#include "AnimBlendFrameData.h"' not in source else source
        source=source.replace('typedef struct AnimBlendFrameData AnimBlendFrameData;\n','')
    source=source.replace('size_t AnimBlendClump_BackendFrameSize(void);\n','')
    if re.search(r'(?m)^size_t AnimBlendClump_BackendFrameSize\(void\)\n\{',source):source=source.replace(method(source,'AnimBlendClump_BackendFrameSize'),'')
    source=source.replace('AnimBlendClump_BackendFrameSize()','sizeof(AnimBlendFrameData)')
    source=source.replace('    const size_t stride=sizeof(AnimBlendFrameData);\n    for(int32_t i=0;i<self->numFrames;++i)callback((AnimBlendFrameData*)((unsigned char*)self->frames+(size_t)i*stride),arg);','    for(int32_t i=0;i<self->numFrames;++i)callback(&self->frames[i],arg);')
    # Preserve both complete native reset-position assignment expressions
    source=re.sub(r'(?m)^(\s*)(frameData->resetPos|frames\[i\]\.resetPos) = ([^;\n]+);',lambda m:m[1]+m[2]+' = AnimBlendFrame_FromNativePosition('+m[3]+');' if not m[3].startswith('AnimBlendFrame_FromNativePosition(') else m[0],source)
    # Recover native pointers only at reads and leave native pointer storage assignments in C fields
    cutscene='(*RPANIMBLENDCLUMPDATA(GetClump()))->frames[0].hanimFrame'
    source=source.replace(cutscene,'AnimBlendFrame_GetHAnim(&(*RPANIMBLENDCLUMPDATA(GetClump()))->frames[0])')
    code=mask(source);protected=[]
    for match in re.finditer(r'\bAnimBlendFrame_(?:GetFrame|GetHAnim)\(',code):protected.append((match.start(),closing(code,match.end()-1)))
    pattern=r'\b((?:m_ped->)?m_pFrames\[[^\]\n]+\]|node|frame|frameData)->(hanimFrame|frame)\b'
    for match in reversed(list(re.finditer(pattern,code))):
        if re.match(r'\s*=(?!=)',source[match.end():]) or any(a<=match.start()<b for a,b in protected):continue
        operation='GetHAnim' if match[2]=='hanimFrame' else 'GetFrame'
        source=source[:match.start()]+'AnimBlendFrame_'+operation+'('+source[match.start(1):match.end(1)]+')'+source[match.end():]
    if source==original:return source
    # Describe each changed native record operation at its existing function-body boundary
    oldLines=set(original.splitlines());lines=[]
    for line in source.splitlines():
        if line not in oldLines and not line.lstrip().startswith('#') and ('AnimBlendFrame_' in line or 'ANIM_FRAME_' in line or 'callback(&self->frames[i]' in line):lines.append(re.match(r'\s*',line)[0]+'// Use the C animation frame while retaining the original live SDK objects')
        lines.append(line)
    source='\n'.join(lines)+'\n'
    if any(name in source for name in ['AnimBlendFrame_GetFrame(','AnimBlendFrame_GetHAnim(','AnimBlendFrame_FromNativePosition(']) and '#include "common.h"' in source and '#include "AnimBlendFrameData_native.h"' not in source:source=source.replace('#include "common.h"','#include "common.h"\n#include "AnimBlendFrameData_native.h"',1)
    return source if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)') else '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def anim_frame_project(source):
    # Register the complete C frame record beside its unchanged native adapter path
    anchor='<ClInclude Include="..\\src\\animation\\AnimBlendFrameData_native.h" />';entry='<ClInclude Include="..\\src\\animation\\AnimBlendFrameData.h" />'
    if entry not in source:source=source.replace(anchor,anchor+'\n    '+entry)
    return objects.project(source)



def calls(source):
    # Preserve complete C frame ownership before complete sequence ownership
    import anim_sequence_migration as clumps
    return clumps.calls(anim_frame_calls(source))


def project(source):
    # Retain frame integration before the renamed C11 sequence producer
    import anim_sequence_migration as clumps
    return clumps.project(anim_frame_project(source))
