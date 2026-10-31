"""Preserve complete collision-model callers and native client lifetime boundaries."""
import re
from rect_migration import mask,closing,arguments

def player_fields(source):
    # Read original native player fields in their declaration order including conditional members
    start=source.index('\tCPlayerPed *m_pPed;');end=source.index('\n\tvoid MakePlayerSafe',start);fields=[]
    for line in source[start:end].splitlines():
        code=line.split('//')[0].strip()
        if not code:continue
        if code.startswith('#'):fields.append(('directive',code,None));continue
        match=re.fullmatch(r'([\w]+)\s+(\*?)(\w+)(?:\[(\d+)\])?;',code);assert match,line
        fields.append((match[1]+match[2],match[3],int(match[4]) if match[4] else None))
    return fields

def player_methods(header):
    # Preserve the existing parent owner while making its formerly implicit model lifetime and deep assignment explicit
    source='''
CPlayerInfo::CPlayerInfo(void)
{
    // Initialize the embedded C collision owner at its original native member-construction boundary
    ColModel_Init(&m_ColModel);
}
CPlayerInfo::~CPlayerInfo(void)
{
    // Release the embedded C collision owner at its original native member-destruction boundary
    ColModel_Destroy(&m_ColModel);
}
CPlayerInfo &CPlayerInfo::operator=(const CPlayerInfo &other)
{
    // Copy native parent fields in their original declaration order around the complete C model assignment
'''
    for kind,name,count in player_fields(header):
        # Retain conditional storage and array element assignment without copying padding or a synthetic parent layout
        if kind=='directive':source+=name+'\n'
        elif name=='m_ColModel':source+='    ColModel_Assign(&m_ColModel,&other.m_ColModel);\n'
        elif count:source+='    for(unsigned i=0;i<'+str(count)+';++i) '+name+'[i]=other.'+name+'[i];\n'
        else:source+='    '+name+'=other.'+name+';\n'
    return source+'    return *this;\n}\n'

def static_temps(source):
    # Preserve individual and array initialization timing with native storage-address cleanup callbacks
    pattern=r'(?m)^CColModel CTempColModels::(ms_colModel\w+)(\[5\])?;'
    def replace(match):
        # Keep registration after the last array element and release the original array in reverse order
        name=match[1];callback='DestroyStatic_'+name
        if match[2]:
            cleanup='ColModel_DestroyArray(CTempColModels::'+name+',5);'
            values=',\n    '.join('ColModel_InitStatic(&CTempColModels::'+name+'['+str(i)+'],'+(callback if i==4 else 'nil')+')' for i in range(5))
            definition='CColModel CTempColModels::'+name+'[5]={\n    '+values+'\n};'
        else:cleanup='ColModel_Destroy(&CTempColModels::'+name+');';definition='CColModel CTempColModels::'+name+'=ColModel_InitStatic(&CTempColModels::'+name+','+callback+');'
        return 'static void '+callback+'(void)\n{\n    // Release the same static collision storage at its original process-exit lifetime boundary\n    '+cleanup+'\n}\n'+definition
    source,count=re.subn(pattern,replace,source);assert count==14,count
    return source

def calls(source):
    # Rewrite diagnosed owner consumers while preserving native control flow and external class ownership
    original=source
    source=source.replace('class CColModel;','struct CColModel;')
    source=re.sub(r'std::allocator<CColModel>\(\)\.construct\((\w+)\);',r'ColModel_Init(\1);',source)
    source=re.sub(r'\b(\w+)->~CColModel\(\);',r'ColModel_Destroy(\1);',source)
    source=source.replace('playerInfo->m_ColModel = *normalColModel;','ColModel_Assign(&playerInfo->m_ColModel,normalColModel);')
    pattern=r'(\(\(CColModel\*\)lptr->item\)|\b\w+)(->|\.)(RemoveCollisionVolumes|CalculateTrianglePlanes|RemoveTrianglePlanes|SetLinkPtr|GetLinkPtr|GetTrianglePoint)\('
    code=mask(source)
    for match in reversed(list(re.finditer(pattern,code))):
        # Keep complete nested receiver and argument expressions before replacing each member invocation
        end=closing(code,match.end()-1);receiver=source[match.start(1):match.end(1)];values=arguments(source[match.end():end-1]);receiver='&'+receiver if match[2]=='.' else receiver
        if match[3]=='GetTrianglePoint':
            assert len(values)==2,values;replacement=values[0]+' = CollisionVector_ToNative(ColModel_GetTrianglePoint('+receiver+','+values[1]+'))'
        else:replacement='ColModel_'+match[3]+'('+receiver+('' if values==[''] else ','+','.join(values))+')'
        source=source[:match.start()]+replacement+source[end:]
    if 'CollisionVector_ToNative(ColModel_GetTrianglePoint(' in source and '#include "CollisionVector_native.h"' not in source:source=source.replace('#include "common.h"','#include "common.h"\n#include "CollisionVector_native.h"',1)
    if 'CColModel CTempColModels::ms_colModelPed1;' in source:source=static_temps(source)
    if 'static CColModel rotorColModel;' in source:
        # Retain the rotor model pre-main initialization and its process-exit destructor position
        source=source.replace('static CColModel rotorColModel;','''static void DestroyStaticRotorModel(void);
static CColModel rotorColModel=ColModel_InitStatic(&rotorColModel,DestroyStaticRotorModel);
static void DestroyStaticRotorModel(void)
{
    // Release the rotor model through the complete C owner at process exit
    ColModel_Destroy(&rotorColModel);
}''')
    if 'static CColModel OurColModel;' in source:
        # Keep first-use initialization guarded at the original function while making storage C-owned
        anchor='CEntity *\nCWorld::TestSphereAgainstSectorList('
        storage='''static CColModel worldSphereColModel;
static void DestroyStaticWorldSphereModel(void)
{
    // Release the lazily initialized world sphere model at its original process-exit position
    ColModel_Destroy(&worldSphereColModel);
}

'''
        assert source.count(anchor)==1;source=source.replace(anchor,storage+anchor,1)
        source=source.replace('static CColModel OurColModel;','static const bool modelInitialised=(ColModel_InitStatic(&worldSphereColModel,DestroyStaticWorldSphereModel),true);\n\t(void)modelInitialised;')
        source=re.sub(r'\bOurColModel\b','worldSphereColModel',source)
    if 'CColModel tyreCol;' in source and 'ColModel_Init(&tyreCol);' not in source:
        # Preserve construction and both return-time cleanups after the original output-distance mutation
        source=source.replace('CColModel tyreCol;','CColModel tyreCol;\n\tColModel_Init(&tyreCol);',1)
        old='\ttyreCol.spheres = nil;\n\n\tif(mindist < dist) {\n\t\tdist = mindist;\n\t\treturn true;\n\t} else\n\t\treturn false;'
        new='\ttyreCol.spheres = nil;\n\n\tif(mindist < dist) {\n\t\tdist = mindist;\n\t\tColModel_Destroy(&tyreCol);\n\t\treturn true;\n\t} else {\n\t\tColModel_Destroy(&tyreCol);\n\t\treturn false;\n\t}'
        assert source.count(old)==1;source=source.replace(old,new,1)
    if 'class CPlayerInfo\n' in source and '\tCColModel m_ColModel;' in source and 'CPlayerInfo &operator=' not in source:
        # Keep the parent public API while forwarding its embedded model operations to C
        source=source.replace('\tvoid MakePlayerSafe(bool);','\tCPlayerInfo(void);\n\t~CPlayerInfo(void);\n\tCPlayerInfo &operator=(const CPlayerInfo &other);\n\n\tvoid MakePlayerSafe(bool);',1)
    if source==original:return source
    lines=[];oldLines=set(original.splitlines())
    for line in source.splitlines():
        # Describe changed executable model calls only inside existing native function bodies
        if line not in oldLines and 'ColModel_' in line and not re.match(r'\s*(?:static )?CColModel\s',line) and not line.lstrip().startswith(('ColModel_InitStatic(', '//')):lines.append(line[:len(line)-len(line.lstrip())]+'// Invoke the complete C collision owner at the original native lifetime or operation boundary')
        lines.append(line)
    return '//+ rouz edit (ChatGPT)\n'+'\n'.join(lines)+'\n//- rouz edit (ChatGPT)\n'
