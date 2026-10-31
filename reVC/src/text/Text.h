//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "config.h"
typedef struct CKeyEntry {
#if defined(FIX_BUGS) || defined(FIX_BUGS_64)
    uint32_t valueOffset;
#else
    uint16_t *value;
#endif
    char key[8];
} CKeyEntry;
typedef struct CKeyArray {CKeyEntry *entries;int numEntries;} CKeyArray;
typedef struct CData {uint16_t *chars;int numChars;} CData;
enum {MAX_MISSION_TEXTS=90};
typedef struct CMissionTextOffsetEntry {char szMissionName[8];uint32_t offset;} CMissionTextOffsetEntry;
typedef struct CMissionTextOffsets {CMissionTextOffsetEntry data[MAX_MISSION_TEXTS];uint16_t size;} CMissionTextOffsets;
typedef struct ChunkHeader {char magic[4];int size;} ChunkHeader;
typedef struct CText {
    CKeyArray keyArray;
    CData data;
    CKeyArray mission_keyArray;
    CData mission_data;
    char encoding;
    bool bHasMissionTextOffsets;
    bool bIsMissionTextLoaded;
    char szMissionTableName[8];
    CMissionTextOffsets MissionTextOffsets;
} CText;
#ifdef __cplusplus
extern "C" {
#endif
extern CText TheText;
extern uint16_t WideErrorString[25];
extern uint16_t UpperCaseTable[128];
extern uint16_t FrenchUpperCaseTable[128];
void Text_Init(CText *self);
void Text_Destroy(CText *self);
void Text_Load(CText *self);
void Text_Unload(CText *self);
uint16_t *Text_Get(CText *self,const char *key);
uint16_t Text_GetUpperCase(CText *self,uint16_t c);
void Text_UpperCase(CText *self,uint16_t *s);
void Text_GetNameOfLoadedMissionText(CText *self,char *output);
void Text_ReadChunkHeader(CText *self,ChunkHeader *buf,int32_t file,size_t *offset);
void Text_LoadMissionText(CText *self,char *name);
void TextKeys_Init(CKeyArray *self);
void TextKeys_Load(CKeyArray *self,size_t length,int file,size_t *offset);
void TextKeys_Unload(CKeyArray *self);
void TextKeys_Update(CKeyArray *self,uint16_t *chars);
CKeyEntry *TextKeys_BinarySearch(CKeyArray *self,const char *key,CKeyEntry *entries,int16_t low,int16_t high);
#if defined(FIX_BUGS) || defined(FIX_BUGS_64)
uint16_t *TextKeys_Search(CKeyArray *self,const char *key,uint16_t *data,uint8_t *result);
#else
uint16_t *TextKeys_Search(CKeyArray *self,const char *key,uint8_t *result);
#endif
void TextData_Init(CData *self);
void TextData_Load(CData *self,size_t length,int file,size_t *offset);
void TextData_Unload(CData *self);
void TextOffsets_Init(CMissionTextOffsets *self);
void TextOffsets_Load(CMissionTextOffsets *self,size_t length,int file,size_t *offset,int unused);
char *UnicodeToAscii(uint16_t *source);
char *UnicodeToAsciiForSaveLoad(uint16_t *source);
char *UnicodeToAsciiForMemoryCard(uint16_t *source);
void TextCopy(uint16_t *output,const uint16_t *source);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
