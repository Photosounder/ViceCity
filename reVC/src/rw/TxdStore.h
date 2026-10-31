//+ rouz edit (ChatGPT)
#pragma once
#include <stdbool.h>
typedef struct TxdDef {
    void *texDict;
    int refCount;
    char name[20];
} TxdDef;
#ifdef __cplusplus
extern "C" {
#endif
void TxdStore_Initialise(void);
void TxdStore_Shutdown(void);
void TxdStore_GameShutdown(void);
int TxdStore_AddTxdSlot(const char *name);
void TxdStore_RemoveTxdSlot(int slot);
int TxdStore_FindTxdSlot(const char *name);
char * TxdStore_GetTxdName(int slot);
void TxdStore_PushCurrentTxd(void);
void TxdStore_PopCurrentTxd(void);
void TxdStore_SetCurrentTxd(int slot);
void TxdStore_Create(int slot);
int TxdStore_GetNumRefs(int slot);
void TxdStore_AddRef(int slot);
void TxdStore_RemoveRef(int slot);
void TxdStore_RemoveRefWithoutDelete(int slot);
bool TxdStore_LoadStream(int slot,void *stream);
bool TxdStore_LoadFile(int slot,const char *filename);
bool TxdStore_StartLoadTxd(int slot,void *stream);
bool TxdStore_FinishLoadTxd(int slot,void *stream);
void TxdStore_RemoveTxd(int slot);
TxdDef *TxdStore_GetSlot(int slot);
bool TxdStore_isTxdLoaded(int slot);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
