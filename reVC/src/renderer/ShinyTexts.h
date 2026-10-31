//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
typedef struct ShinyTextVector { float x,y,z; } ShinyTextVector;
typedef struct ShinyTextUV { float x,y; } ShinyTextUV;
typedef struct ShinyText {
    ShinyTextVector m_verts[4];
    ShinyTextUV m_texCoords[4];
    float m_camDist;
    uint8_t m_type,m_red,m_green,m_blue;
} ShinyText;
enum { SHINYTEXT_WALK=1,SHINYTEXT_FLAT };
#ifdef __cplusplus
extern "C" {
#endif
void ShinyTexts_Init(void);
void ShinyTexts_RegisterOne(ShinyTextVector p0,ShinyTextVector p1,ShinyTextVector p2,ShinyTextVector p3,float u0,float v0,float u1,float v1,float u2,float v2,float u3,float v3,uint8_t type,uint8_t red,uint8_t green,uint8_t blue,float maxDist);
void ShinyTexts_Render(void);
void ShinyTexts_RenderForEnvMap(void);
void ShinyTexts_RenderOutGeometryBuffer(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
