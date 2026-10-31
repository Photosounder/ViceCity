"""Retain actual SDK vertex storage and individual texture and drawing primitives."""
import re


def build(source):
    # Keep all owner decisions in C while borrowing the actual preexisting render-buffer storage
    constants=sorted(set(re.findall(r'\brw(?:RENDERSTATE\w+|SHADEMODE\w+|BLEND\w+|FILTER\w+|PRIMTYPE\w+)\b',source)))
    header='//+ rouz edit (ChatGPT)\n#pragma once\n#include "Sprite2d.h"\n#include "Sprite_backend.h"\n#include "Shadow_backend.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'
    native='//+ rouz edit (ChatGPT)\n#include "common.h"\n#include "Sprite2d_backend.h"\n#include "RenderBuffer.h"\nstatic RwIm2DVertex sprite2dVertices[8];\n'
    rows=[('void *','Vertices','void','return sprite2dVertices;'),('void *','TempVertices','void','return TempVertexBuffer.im2d;'),('void *','TextureRead','const char *name,const char *mask','return RwTextureRead(name,mask);'),('void *','TextureRaster','void *texture','return RwTextureGetRaster((RwTexture*)texture);'),('void','IndexedDraw','int32_t kind,void *vertices,int32_t count,uint16_t *indices,int32_t indexCount','RwIm2DRenderIndexedPrimitive((RwPrimitiveType)kind,(RwIm2DVertex*)vertices,count,indices,indexCount);')]
    for kind,name,args,expression in rows:
        # Forward one original operation using actual native objects and SDK scalar types
        declaration=kind+' Sprite2d_Host'+name+'('+args+')';header+=declaration+';\n';native+=declaration+'\n{\n    // Forward the original SDK primitive using its actual native storage\n    '+expression+'\n}\n'
    header+='enum {'+','.join('SPRITE2D_CONST_'+v for v in constants)+'};\nint32_t Sprite2d_HostConstant(int32_t identity);\n'
    native+='int32_t Sprite2d_HostConstant(int32_t identity)\n{\n    // Translate SDK identities while retaining every draw and state choice in C\n    switch(identity){'+''.join('case SPRITE2D_CONST_'+v+':return '+v+';' for v in constants)+'}return 0;\n}\n'
    header+='#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n';native+='//- rouz edit (ChatGPT)\n';return header,native
