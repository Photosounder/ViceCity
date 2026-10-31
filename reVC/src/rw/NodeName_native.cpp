//+ rouz edit (ChatGPT)
#include "common.h"
#include "NodeName_backend.h"
static_assert(MAKECHUNKID(rwVENDORID_ROCKSTAR,0xFE)==0x0253F2FE,"Node-name plugin identity");
static RwStream *NodeNameReadNative(RwStream *stream,RwInt32 length,void *object,RwInt32 offset,RwInt32 size)
{
    // Transport the actual SDK stream handle into the complete C read callback
    return static_cast<RwStream*>(NodeNameStreamRead(stream,length,object,offset,size));
}
static RwStream *NodeNameWriteNative(RwStream *stream,RwInt32 length,const void *object,RwInt32 offset,RwInt32 size)
{
    // Transport the actual SDK stream handle into the complete C write callback
    return static_cast<RwStream*>(NodeNameStreamWrite(stream,length,object,offset,size));
}
int32_t NodeName_HostRegister(int32_t size,uint32_t id)
{
    // Bind the complete C lifecycle callbacks at the real RenderWare registration boundary
    return RwFrameRegisterPlugin(size,id,NodeNameConstructor,NodeNameDestructor,NodeNameCopy);
}
void NodeName_HostRegisterStream(uint32_t id)
{
    // Bind typed SDK stream trampolines and the unchanged C size callback
    RwFrameRegisterPluginStream(id,NodeNameReadNative,NodeNameWriteNative,NodeNameStreamGetSize);
}
void NodeName_HostRead(void *stream,void *buffer,uint32_t length)
{
    // Read bytes from the original live SDK stream into caller-owned plugin storage
    RwStreamRead(static_cast<RwStream*>(stream),buffer,length);
}
void NodeName_HostWrite(void *stream,const void *buffer,uint32_t length)
{
    // Write caller-owned plugin bytes to the original live SDK stream
    RwStreamWrite(static_cast<RwStream*>(stream),buffer,length);
}
//- rouz edit (ChatGPT)
