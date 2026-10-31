"""Move the complete pedestrian route table, record and all six operations into C11."""
import re
import object_names_migration as objects
OPERATIONS=['Initialise','GetRouteThisPointIsOn','GetRouteStart','GetPointPosition','AddRoutePoint','RemoveRoute']
RENAMES={'src/peds/PedRoutes.cpp':'src/peds/PedRoutes.c'}

def header(source):
    # Replace the complete class with the same record layout and explicit C linkage
    assert 'class CRouteNode\n' in source
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "config.h"

typedef struct PedRouteVector {
    float x, y, z;
} PedRouteVector;

typedef struct CRouteNode {
    int16_t m_route;
    PedRouteVector m_pos;
} CRouteNode;

#ifdef __cplusplus
extern "C" {
#endif
extern CRouteNode gaRoutes[NUMPEDROUTES];
void PedRoutes_Initialise(void);
int16_t PedRoutes_GetRouteThisPointIsOn(int16_t point);
int16_t PedRoutes_GetRouteStart(int16_t route);
PedRouteVector PedRoutes_GetPointPosition(int16_t point);
void PedRoutes_AddRoutePoint(int16_t route, PedRouteVector pos);
void PedRoutes_RemoveRoute(int16_t route);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def kernel(source):
    # Retain the complete original table and both route-removal policies
    source=re.sub(r'^#include[^\n]*\n','',source,flags=re.M).lstrip()
    source=source.replace('CRouteNode::','PedRoutes_').replace('CVector(0.0f, 0.0f, 0.0f)','(PedRouteVector){0.0f, 0.0f, 0.0f}').replace('CVector','PedRouteVector')
    source=re.sub(r'\bint16\b','int16_t',source);source=re.sub(r'\buint16\b','uint16_t',source)
    source=source.replace('PedRoutes_Initialise()','PedRoutes_Initialise(void)')
    source=source.replace('\n{','\n{\n\t// Preserve the original route operation and its ordered table updates')
    return '//+ rouz edit (ChatGPT)\n#include "PedRoutes.h"\n\n'+source.rstrip()+'\n//- rouz edit (ChatGPT)\n'

def ped_routes_calls(source):
    # Convert all route callers without retaining any C++ vector dependency in the owner
    if 'class CRouteNode\n' in source:return header(source)
    if 'CRouteNode::Initialise()\n{' in source:return kernel(source)
    original=source
    pattern=r'(?m)^(\s*)m_nextRoutePointPos = CRouteNode::GetPointPosition\(([^\n]+)\);'
    def position(match):
        # Adapt the plain returned value at the existing C++ pedestrian boundary
        indent=match[1]
        return indent+'// Copy the C route position into the pedestrian vector\n'+indent+'const PedRouteVector routePosition = PedRoutes_GetPointPosition('+match[2]+');\n'+indent+'m_nextRoutePointPos = CVector(routePosition.x, routePosition.y, routePosition.z);'
    source=re.sub(pattern,position,source)
    old='\t\tCRouteNode::AddRoutePoint(ScriptParams[0], *(CVector*)&ScriptParams[1]);'
    new='''\t\t// Copy the collected script position into the C route value
\t\tconst CVector routePosition = *(CVector*)&ScriptParams[1];
\t\tconst PedRouteVector routePoint = { routePosition.x, routePosition.y, routePosition.z };
\t\tPedRoutes_AddRoutePoint(ScriptParams[0], routePoint);'''
    source=source.replace(old,new)
    source=re.sub(r'CRouteNode::('+ '|'.join(OPERATIONS)+r')\(',r'PedRoutes_\1(',source)
    if source==original:return source
    return source if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)') else '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def ped_routes_project(source):
    # Retain historical project entries while requiring the renamed producer to compile as C11
    source=source.replace('..\\src\\peds\\PedRoutes.cpp','..\\src\\peds\\PedRoutes.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="\.\.\\src\\peds\\PedRoutes\.c"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Preserve unrelated options and explicitly select the C language
            body=match[1] or '\n    '
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body=body.rstrip()+'\n      <'+tag+'>'+value+'</'+tag+'>\n    '
            return '<ClCompile Include="..\\src\\peds\\PedRoutes.c">'+body+'</ClCompile>'
        source=re.sub(pattern,metadata,source)
    return objects.project(source)


def calls(source):
    # Preserve full C route ownership before converting the complete stored-car owner and save callers
    import stored_car_migration as stored
    return stored.calls(ped_routes_calls(source))

def project(source):
    # Retain the route producer rename before the independent C11 stored-car producer
    import stored_car_migration as stored
    return stored.project(ped_routes_project(source))
