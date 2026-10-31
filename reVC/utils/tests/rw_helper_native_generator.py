"""Generate complete private helper primitives without reading or writing source files."""
from rw_helper_schema import HOST,LIB_HOST,ITERATORS

def build(original):
    # Derive the entire SDK boundary from the immutable original owner and explicit primitive schemas
    header='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#if defined(LIBRW) && (defined(RW_GL3) || defined(RW_WDGL)) && !defined(RW_OPENGL)
#define RW_OPENGL
#endif
typedef void *(*RwHelperVisit)(void *object,void *data);
typedef void (*RwHelperPairScope)(void *data,void *first,void *second);
typedef void (*RwHelperVertexScope)(void *data,void *vertices);
#ifdef __cplusplus
extern "C" {
#endif
'''
    native='''//+ rouz edit (ChatGPT)
#define WITHD3D
#include "common.h"
#include <rpskin.h>
#include "skeleton.h"
#include "RwHelper_backend.h"
#if !defined(FINAL) || defined(DEBUGMENU)
#include "rtcharse.h"
#endif
#ifdef LIBRW
#include <rpmatfx.h>
#include "VehicleModelInfo.h"
#endif
'''
    for group,guard in [(HOST,None),(LIB_HOST,'LIBRW')]:
        # Keep every SDK operation separate so all application decisions can remain in the C producer
        if guard:header+='#ifdef '+guard+'\n';native+='#ifdef '+guard+'\n'
        for result,name,args,expression in group:
            # Keep charset declarations on the exact original debug-font configuration boundary
            charset=name.startswith('Charset');condition='!defined(FINAL) || defined(DEBUGMENU)'
            if charset:header+='#if '+condition+'\n';native+='#if '+condition+'\n'
            declaration=result+' RwHelper_Host'+name+'('+args+')';header+=declaration+';\n';native+=declaration+'\n{\n    // Forward one original SDK operation or access its actual typed metadata\n    '+('return ' if result!='void' else '')+expression+';\n}\n'
            if charset:header+='#endif\n';native+='#endif\n'
        if guard:header+='#endif\n';native+='#endif\n'
    native+='struct RwHelperVisitContext {RwHelperVisit callback;void *data;};\n'
    for name,parent,operation,item in ITERATORS:
        # Allocate a distinct synchronous callback context for every traversal and nested invocation
        declaration='void RwHelper_Host'+name+'(void *parent,RwHelperVisit callback,void *data)';header+=declaration+';\n';native+='static '+item+' *RwHelper_Visit'+name+'('+item+' *object,void *data)\n{\n    // Return the C callback result through the exact SDK callback signature\n    RwHelperVisitContext *scope=(RwHelperVisitContext*)data;return ('+item+'*)scope->callback(object,scope->data);\n}\n'+declaration+'\n{\n    // Preserve independent borrowed callback state for synchronous traversal and reentry\n    RwHelperVisitContext scope={callback,data};'+operation+'(('+parent+'*)parent,RwHelper_Visit'+name+',&scope);\n}\n'
    for name,types in [('BoneMatrices',('RwMatrix','RwMatrix')),('AtomicVectors',('RwV3d','RwV3d')),('BoundingSphere',('RwMatrix','RwSphere'))]:
        # Supply actual uninitialized SDK stack objects while keeping the entire algorithm in a synchronous C callback
        declaration='void RwHelper_Host'+name+'(RwHelperPairScope callback,void *data)';header+=declaration+';\n';native+=declaration+'\n{\n    // Borrow distinct native stack objects for the complete synchronous C operation\n    '+types[0]+' first;'+types[1]+' second;callback(data,&first,&second);\n}\n'
    declaration='void RwHelper_HostQuadVertices(RwHelperVertexScope callback,void *data)';header+=declaration+';\n';native+=declaration+'\n{\n    // Preserve the original four uninitialized SDK vertices and their per-invocation lifetime\n    RwIm2DVertex vertices[4];callback(data,vertices);\n}\n'
    header+='#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n';native+='//- rouz edit (ChatGPT)\n'
    from rw_helper_migration import kernel
    from rw_helper_extension import extend
    candidate,constants=kernel(original)
    header,native=extend(header,native,candidate,constants)
    return header,native
