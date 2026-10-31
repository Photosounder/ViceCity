//+ rouz edit (ChatGPT)
#pragma once
typedef struct CAnimBlendLink {struct CAnimBlendLink *next,*prev;} CAnimBlendLink;
#ifdef __cplusplus
extern "C" {
#endif
void AnimBlendLink_Init(CAnimBlendLink *self);
void AnimBlendLink_Prepend(CAnimBlendLink *self,CAnimBlendLink *link);
void AnimBlendLink_Remove(CAnimBlendLink *self);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
