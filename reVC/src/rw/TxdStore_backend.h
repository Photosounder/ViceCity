//+ rouz edit (ChatGPT)
#pragma once
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
void TxdStore_HostDestroyDictionary(void *dictionary);
void * TxdStore_HostGetCurrent(void);
void TxdStore_HostSetCurrent(void *dictionary);
void * TxdStore_HostCreateDictionary(void);
void TxdStore_HostRemoveStreamingTxd(int slot);
bool TxdStore_HostFindDictionaryChunk(void *stream);
void * TxdStore_HostReadDictionary(void *stream);
void * TxdStore_HostReadDictionary1(void *stream);
void * TxdStore_HostReadDictionary2(void *stream,void *dictionary);
void TxdStore_HostEnableRasterConversion(bool enabled);
void * TxdStore_HostOpenFileStream(const char *filename);
void TxdStore_HostCloseStream(void *stream);
void re3_assert(const char *expr,const char *filename,unsigned int lineno,const char *func);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
