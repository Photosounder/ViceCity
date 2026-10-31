//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "Streaming.h"
#include "RwHelper.h"
#include "TxdStore_backend.h"
void TxdStore_HostDestroyDictionary(void *dictionary)
{
    // Invoke the original external owner primitive with the same borrowed arguments
    RwTexDictionaryDestroy((RwTexDictionary*)dictionary);
}
void * TxdStore_HostGetCurrent(void)
{
    // Invoke the original external owner primitive with the same borrowed arguments
    return RwTexDictionaryGetCurrent();
}
void TxdStore_HostSetCurrent(void *dictionary)
{
    // Invoke the original external owner primitive with the same borrowed arguments
    RwTexDictionarySetCurrent((RwTexDictionary*)dictionary);
}
void * TxdStore_HostCreateDictionary(void)
{
    // Invoke the original external owner primitive with the same borrowed arguments
    return RwTexDictionaryCreate();
}
void TxdStore_HostRemoveStreamingTxd(int slot)
{
    // Invoke the original external owner primitive with the same borrowed arguments
    CStreaming::RemoveTxd(slot);
}
bool TxdStore_HostFindDictionaryChunk(void *stream)
{
    // Invoke the original external owner primitive with the same borrowed arguments
    return RwStreamFindChunk((RwStream*)stream,rwID_TEXDICTIONARY,NULL,NULL)!=NULL;
}



void TxdStore_HostEnableRasterConversion(bool enabled)
{
    // Invoke the original external owner primitive with the same borrowed arguments
    _rwD3D8TexDictionaryEnableRasterFormatConversion(enabled);
}
void * TxdStore_HostOpenFileStream(const char *filename)
{
    // Invoke the original external owner primitive with the same borrowed arguments
    return RwStreamOpen(rwSTREAMFILENAME,rwSTREAMREAD,filename);
}
void TxdStore_HostCloseStream(void *stream)
{
    // Invoke the original external owner primitive with the same borrowed arguments
    RwStreamClose((RwStream*)stream,NULL);
}
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
