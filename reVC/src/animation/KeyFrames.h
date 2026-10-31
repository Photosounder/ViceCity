//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "Quaternion.h"
enum {ANIM_KEYFRAME_ROT=1,ANIM_KEYFRAME_TRANS=2};
typedef struct KeyFrameVector3 {float x,y,z;} KeyFrameVector3;
typedef struct KeyFrameVectorRef {const float *x,*y,*z;} KeyFrameVectorRef;
typedef struct KeyFrameVectorOut {float *x,*y,*z;} KeyFrameVectorOut;
typedef struct KeyFrame {CQuaternion rotation;float deltaTime;} KeyFrame;
typedef struct KeyFrameTrans {KeyFrame base;KeyFrameVector3 translation;} KeyFrameTrans;
typedef struct KeyFrameCompressed {int16_t rot[4];int16_t deltaTime;} KeyFrameCompressed;
typedef struct KeyFrameTransCompressed {KeyFrameCompressed base;int16_t trans[3];} KeyFrameTransCompressed;
#ifdef __cplusplus
extern "C" {
#endif
void KeyFrameCompressed_GetRotation(const KeyFrameCompressed *self,CQuaternion *value);
void KeyFrameCompressed_SetRotation(KeyFrameCompressed *self,const CQuaternion *value);
float KeyFrameCompressed_GetDeltaTime(const KeyFrameCompressed *self);
void KeyFrameCompressed_SetTime(KeyFrameCompressed *self,float value);
void KeyFrameTransCompressed_GetTranslation(const KeyFrameTransCompressed *self,KeyFrameVectorOut value);
void KeyFrameTransCompressed_SetTranslation(KeyFrameTransCompressed *self,KeyFrameVectorRef value);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
