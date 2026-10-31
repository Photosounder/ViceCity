"""Move particle configuration parsing, initialization and every record into C11."""
import re
import surface_table_migration as surface_table

BACKEND='''//+ rouz edit (ChatGPT)
#pragma once
#include "ParticleMgr.h"
enum { PARTICLE_MGR_WORK_BUFFER_SIZE=55000 };
#ifdef __cplusplus
extern "C" {
#endif
uint8_t *ParticleMgr_BackendWorkBuffer(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "main.h"
#include "ParticleMgr_backend.h"
static_assert(sizeof(work_buff)==PARTICLE_MGR_WORK_BUFFER_SIZE,"Actual shared work buffer extent");
uint8_t *ParticleMgr_BackendWorkBuffer(void)
{
    // Borrow the original shared main-module work buffer without copying or owning it
    return work_buff;
}
//- rouz edit (ChatGPT)
'''

def header(source):
    # Retain all native record fields and replace only their C++ value owners and manager class
    flags=source[source.index('enum\n{'):source.index('\n\n\nstruct tParticleSystemData')]
    flags=re.sub(r'BIT\((\d+)\)',r'(1 << \1)',flags)
    a=source.index('struct tParticleSystemData');b=source.index('\n};',a)+3
    record=source[a:b].replace('struct tParticleSystemData','typedef struct tParticleSystemData').replace('\n};','\n} tParticleSystemData;')
    for old,new in [('uint32','uint32_t'),('int32','int32_t'),('uint16','uint16_t'),('int16','int16_t'),('uint8','uint8_t'),('CRGBA','ParticleSystemColor'),('CVector2D','ParticleSystemStretch'),('RwRaster','ParticleRaster')]:record=re.sub(r'\b'+old+r'\b',new,record)
    record=record.replace('tParticleType m_Type','enum tParticleType m_Type')
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "ParticleType.h"
#ifdef __cplusplus
class CParticle;
#ifdef LIBRW
namespace rw { struct Raster; }
typedef rw::Raster ParticleRaster;
#else
typedef struct RwRaster ParticleRaster;
#endif
#else
typedef struct CParticle CParticle;
typedef struct ParticleRaster ParticleRaster;
#endif
typedef union ParticleSystemColor {
    uint32_t color32;
    struct { uint8_t red,green,blue,alpha; };
} ParticleSystemColor;
typedef struct ParticleSystemStretch { float x,y; } ParticleSystemStretch;
'''+flags+'\n\n'+record+'''
typedef struct ParticleSystemMgr {
    tParticleSystemData m_aParticles[MAX_PARTICLES];
} ParticleSystemMgr;
#ifdef __cplusplus
extern "C" {
#endif
extern ParticleSystemMgr mod_ParticleSystemManager;
extern const char *ParticleFilename;
void ParticleMgr_Initialise(void);
void ParticleMgr_LoadParticleData(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def kernel(source,original_header):
    # Translate both complete algorithms while preserving tokenization, conversions and callback ordering
    enum=original_header[original_header.index('\tenum\n\t{',original_header.index('class cParticleSystemMgr')):original_header.index('\n\npublic:',original_header.index('class cParticleSystemMgr'))].strip()
    enum='\n'.join(line[1:] if line.startswith('\t') else line for line in enum.splitlines())
    body=source[source.index('void cParticleSystemMgr::Initialise()'):source.rindex('//- rouz edit (ChatGPT)')].rstrip()
    body=body.replace('cParticleSystemMgr::Initialise()','ParticleMgr_Initialise(void)').replace('cParticleSystemMgr::LoadParticleData()','ParticleMgr_LoadParticleData(void)').replace('\tLoadParticleData();','\tParticleMgr_LoadParticleData();')
    body=re.sub(r'\bm_aParticles\b','mod_ParticleSystemManager.m_aParticles',body)
    body=body.replace('work_buff, ARRAY_SIZE(work_buff)','ParticleMgr_BackendWorkBuffer(), PARTICLE_MGR_WORK_BUFFER_SIZE').replace('(char *)work_buff','(char *)ParticleMgr_BackendWorkBuffer()')
    body=body.replace('(tParticleType)','(enum tParticleType)').replace('nil','NULL').replace('while ( true )','while ( 1 )')
    for old,new in [('int32','int32_t'),('uint32','uint32_t')]:body=re.sub(r'\b'+old+r'\b',new,body)
    body=body.replace('void ParticleMgr_Initialise(void)\n{','void ParticleMgr_Initialise(void)\n{\n    // Load configuration before clearing all original live particle-list pointers')
    body=body.replace('void ParticleMgr_LoadParticleData(void)\n{','void ParticleMgr_LoadParticleData(void)\n{\n    // Read the original shared buffer and preserve every token conversion and retained field')
    body=re.sub(r'(?m)^(\s*)\{\t?\n',lambda m:m[0]+m[1]+'\t// Preserve the original parser branch and token traversal\n',body)
    return '''//+ rouz edit (ChatGPT)
#include "ParticleMgr_backend.h"
#include "FileMgr.h"
#include "config.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#define Min(a,b) ((a)<(b)?(a):(b))
#define SQR(x) ((x)*(x))
#ifndef MASTER
void re3_assert(const char *,const char *,unsigned int,const char *);
#define ASSERT(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define ASSERT(expression) (expression)
#endif
ParticleSystemMgr mod_ParticleSystemManager;
const char *ParticleFilename="PARTICLE.CFG";
'''+enum+'\n\n'+body+'\n//- rouz edit (ChatGPT)\n'

def calls(source):
    # Remove the manager class and replace the three actual renderer calls with explicit C data transfers
    if 'cParticleSystemMgr::cParticleSystemMgr()' in source:return surface_table.calls(HOST)
    if 'class cParticleSystemMgr\n' in source:return surface_table.calls(header(source))
    source=source.replace('\tmod_ParticleSystemManager.Initialise();','\t//+ rouz edit (ChatGPT)\n\t// Enter the C configuration parser and clear the same particle list slots\n\tParticleMgr_Initialise();\n\t//- rouz edit (ChatGPT)')
    source=source.replace('\t\tRwRGBAAssign(&pParticle->m_Color, psystem->m_RenderColouring);','\t\t//+ rouz edit (ChatGPT)\n\t\t// Copy the original four C color channels into the native particle color\n\t\tRwRGBA renderColour={psystem->m_RenderColouring.red,psystem->m_RenderColouring.green,psystem->m_RenderColouring.blue,psystem->m_RenderColouring.alpha};\n\t\tRwRGBAAssign(&pParticle->m_Color, &renderColour);\n\t\t//- rouz edit (ChatGPT)')
    source=source.replace('\t\t\t\telse\n\t\t\t\t\tRwRGBAAssign(&particle->m_Color, psystem->m_FadeDestinationColor);','\t\t\t\telse {\n\t\t\t\t\t//+ rouz edit (ChatGPT)\n\t\t\t\t\t// Copy the same four destination channels after the original color-fade gate\n\t\t\t\t\tRwRGBA fadeColour={psystem->m_FadeDestinationColor.red,psystem->m_FadeDestinationColor.green,psystem->m_FadeDestinationColor.blue,psystem->m_FadeDestinationColor.alpha};\n\t\t\t\t\tRwRGBAAssign(&particle->m_Color, &fadeColour);\n\t\t\t\t\t//- rouz edit (ChatGPT)\n\t\t\t\t}')
    return surface_table.calls(source)

def project(source):
    # Register a uniquely named C11 parser unit beside its one native shared-buffer accessor
    entry='<ClCompile Include="..\\src\\renderer\\ParticleMgr.cpp" />'
    if '..\\src\\renderer\\ParticleConfig.c"' not in source:source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\renderer\\ParticleConfig.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>')
    entry='<ClInclude Include="..\\src\\renderer\\ParticleMgr.h" />'
    if '..\\src\\renderer\\ParticleMgr_backend.h"' not in source:source=source.replace(entry,entry+'\n    <ClInclude Include="..\\src\\renderer\\ParticleMgr_backend.h" />')
    return surface_table.project(source)
