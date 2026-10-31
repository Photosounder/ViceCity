//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
int32_t NodeName_HostRegister(int32_t size,uint32_t id);
void NodeName_HostRegisterStream(uint32_t id);
void NodeName_HostRead(void *stream,void *buffer,uint32_t length);
void NodeName_HostWrite(void *stream,const void *buffer,uint32_t length);
void *NodeNameConstructor(void *object,int32_t offsetInObject,int32_t sizeInObject);
void *NodeNameDestructor(void *object,int32_t offsetInObject,int32_t sizeInObject);
void *NodeNameCopy(void *dstObject,const void *srcObject,int32_t offsetInObject,int32_t sizeInObject);
void *NodeNameStreamRead(void *stream,int32_t binaryLength,void *object,int32_t offsetInObject,int32_t sizeInObject);
void *NodeNameStreamWrite(void *stream,int32_t binaryLength,const void *object,int32_t offsetInObject,int32_t sizeInObject);
int32_t NodeNameStreamGetSize(const void *object,int32_t offsetInObject,int32_t sizeInObject);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
