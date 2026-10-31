//+ rouz edit (ChatGPT)
#define _CRT_SECURE_NO_WARNINGS
#include "Text.h"
#include "Text_backend.h"
#include "Messages.h"
#include "FileMgr.h"
#include "Timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
CText TheText;
uint16_t WideErrorString[25];
uint16_t UpperCaseTable[128] = {
	128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138,
	139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
	150, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137,
	138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148,
	149, 173, 173, 175, 176, 177, 178, 179, 180, 181, 182,
	183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193,
	194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204,
	205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215,
	216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226,
	227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237,
	238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248,
	249, 250, 251, 252, 253, 254, 255
};

uint16_t FrenchUpperCaseTable[128] = {
	128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138,
	139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
	150, 65, 65, 65, 65, 132, 133, 69, 69, 69, 69, 73, 73,
	73, 73, 79, 79, 79, 79, 85, 85, 85, 85, 173, 173, 175,
	176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186,
	187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197,
	198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208,
	209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219,
	220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230,
	231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241,
	242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252,
	253, 254, 255
};

void TextKeys_Init(CKeyArray *self)
{
    // Preserve the original key-array constructor's named-field initialization
    self->entries=NULL;
    self->numEntries=0;
}
void TextData_Init(CData *self)
{
    // Preserve the original character-array constructor's named-field initialization
    self->chars=NULL;
    self->numChars=0;
}
void TextOffsets_Init(CMissionTextOffsets *self)
{
    // Preserve the original offset-table constructor without clearing inactive entries
    self->size=0;
}
void Text_Init(CText *self)
{
    // Construct each original owning subrecord before the original text-constructor body
    TextKeys_Init(&self->keyArray);
    TextData_Init(&self->data);
    TextKeys_Init(&self->mission_keyArray);
    TextData_Init(&self->mission_data);
    TextOffsets_Init(&self->MissionTextOffsets);
    self->encoding='e';
    self->bHasMissionTextOffsets=false;
    self->bIsMissionTextLoaded=false;
    memset(self->szMissionTableName,0,sizeof(self->szMissionTableName));
    memset(WideErrorString,0,sizeof(WideErrorString));
}
void Text_Destroy(CText *self)
{
    // Destroy the four original owning subrecords in reverse member-declaration order
    TextData_Unload(&self->mission_data);
    TextKeys_Unload(&self->mission_keyArray);
    TextData_Unload(&self->data);
    TextKeys_Unload(&self->keyArray);
}
static void Text_ShutdownGlobal(void)
{
    // Retain original process-exit ownership cleanup without a C++ global destructor
    Text_Destroy(&TheText);
}
#if !defined(_MSC_VER) && (defined(__GNUC__) || defined(__clang__))
static void Text_InitialiseGlobal(void) __attribute__((constructor));
#endif
static void Text_InitialiseGlobal(void)
{
    // Retain original pre-main construction and register its matching C exit cleanup
    Text_Init(&TheText);
    atexit(Text_ShutdownGlobal);
}
#if defined(_MSC_VER)
#pragma section(".CRT$XCU",read)
__declspec(allocate(".CRT$XCU")) void (__cdecl *const Text_globalInitializer)(void)=Text_InitialiseGlobal;
#if defined(_M_IX86)
#pragma comment(linker,"/include:_Text_globalInitializer")
#else
#pragma comment(linker,"/include:Text_globalInitializer")
#endif
#elif !defined(__GNUC__) && !defined(__clang__)
#error "Register Text_InitialiseGlobal with the platform C runtime startup"
#endif


void
Text_Load(CText *self)
{
    // Preserve the complete original Load operation and ordered live accesses
	char filename[32];
	size_t offset;
	int file;
	bool tkey_loaded = false, tdat_loaded = false;
	ChunkHeader m_ChunkHeader;

	self->bIsMissionTextLoaded = false;
	self->bHasMissionTextOffsets = false;

	Text_Unload(self);

	FileMgr_SetDir("TEXT");
	switch(Text_BackendLanguage()){
	case TEXT_LANGUAGE_AMERICAN:
		sprintf(filename, "AMERICAN.GXT");
		break;
	case TEXT_LANGUAGE_FRENCH:
		sprintf(filename, "FRENCH.GXT");
		break;
	case TEXT_LANGUAGE_GERMAN:
		sprintf(filename, "GERMAN.GXT");
		break;
	case TEXT_LANGUAGE_ITALIAN:
		sprintf(filename, "ITALIAN.GXT");
		break;
	case TEXT_LANGUAGE_SPANISH:
		sprintf(filename, "SPANISH.GXT");
		break;
#ifdef MORE_LANGUAGES
	case TEXT_LANGUAGE_POLISH:
		sprintf(filename, "POLISH.GXT");
		break;
	case TEXT_LANGUAGE_RUSSIAN:
		sprintf(filename, "RUSSIAN.GXT");
		break;
	case TEXT_LANGUAGE_JAPANESE:
		sprintf(filename, "JAPANESE.GXT");
		break;
#endif
	}

	file = FileMgr_OpenFile(filename, "rb");

	offset = 0;
	while (!tkey_loaded || !tdat_loaded) {
		Text_ReadChunkHeader(self,&m_ChunkHeader, file, &offset);
		if (m_ChunkHeader.size != 0) {
			if (strncmp(m_ChunkHeader.magic, "TABL", 4) == 0) {
				TextOffsets_Load(&self->MissionTextOffsets,m_ChunkHeader.size, file, &offset, 0x58000);
				self->bHasMissionTextOffsets = true;
			} else if (strncmp(m_ChunkHeader.magic, "TKEY", 4) == 0) {
				TextKeys_Load(&self->keyArray,m_ChunkHeader.size, file, &offset);
				tkey_loaded = true;
			} else if (strncmp(m_ChunkHeader.magic, "TDAT", 4) == 0) {
				TextData_Load(&self->data,m_ChunkHeader.size, file, &offset);
				tdat_loaded = true;
			} else {
				FileMgr_Seek(file, m_ChunkHeader.size, SEEK_CUR);
				offset += m_ChunkHeader.size;
			}
		}
	}

	TextKeys_Update(&self->keyArray,self->data.chars);
	FileMgr_CloseFile(file);
	FileMgr_SetDir("");
}

void
Text_Unload(CText *self)
{
    // Preserve the complete original Unload operation and ordered live accesses
	Messages_ClearAllMessagesDisplayedByGame();
	TextKeys_Unload(&self->keyArray);
	TextData_Unload(&self->data);
	TextKeys_Unload(&self->mission_keyArray);
	TextData_Unload(&self->mission_data);
	self->bIsMissionTextLoaded = false;
	memset(self->szMissionTableName, 0, sizeof(self->szMissionTableName));
}

uint16_t*
Text_Get(CText *self,const char *key)
{
    // Preserve the complete original Get operation and ordered live accesses
	uint8_t result = false;
#if defined (FIX_BUGS) || defined(FIX_BUGS_64)
	uint16_t *outstr = TextKeys_Search(&self->keyArray,key, self->data.chars, &result);
#else
	uint16_t *outstr = TextKeys_Search(&self->keyArray,key, &result);
#endif

	if (!result && self->bHasMissionTextOffsets && self->bIsMissionTextLoaded)
#if defined (FIX_BUGS) || defined(FIX_BUGS_64)
		outstr = TextKeys_Search(&self->mission_keyArray,key, self->mission_data.chars, &result);
#else
		outstr = TextKeys_Search(&self->mission_keyArray,key, &result);
#endif
	return outstr;
}

uint16_t
Text_GetUpperCase(CText *self,uint16_t c)
{
    // Preserve the complete original GetUpperCase operation and ordered live accesses
	switch (self->encoding)
	{
	case 'e':
		if (c >= 'a' && c <= 'z')
			return c - 32;
		break;
	case 'f':
		if (c >= 'a' && c <= 'z')
			return c - 32;

		if (c >= 128 && c <= 255)
			return FrenchUpperCaseTable[c-128];
		break;
	case 'g':
	case 'i':
	case 's':
		if (c >= 'a' && c <= 'z')
			return c - 32;

		if (c >= 128 && c <= 255)
			return UpperCaseTable[c-128];
		break;
	default:
		break;
	}
	return c;
}

void
Text_UpperCase(CText *self,uint16_t *s)
{
    // Preserve the complete original UpperCase operation and ordered live accesses
	while(*s){
		*s = Text_GetUpperCase(self,*s);
		s++;
	}
}

void
Text_GetNameOfLoadedMissionText(CText *self,char *outName)
{
    // Preserve the complete original GetNameOfLoadedMissionText operation and ordered live accesses
	strcpy(outName, self->szMissionTableName);
}

void
Text_ReadChunkHeader(CText *self,ChunkHeader *buf, int32_t file, size_t *offset)
{
    // Preserve the complete original ReadChunkHeader operation and ordered live accesses
#ifdef THIS_IS_STUPID
	char *_buf = (char*)buf;
	for (int i = 0; i < sizeof(ChunkHeader); i++) {
		FileMgr_Read(file, &_buf[i], 1);
		(*offset)++;
	}
#else
	// original code loops 8 times to read 1 byte with CFileMgr::Read, that's retarded
	FileMgr_Read(file, (char*)buf, sizeof(ChunkHeader));
	*offset += sizeof(ChunkHeader);
#endif
}

void
Text_LoadMissionText(CText *self,char *MissionTableName)
{
    // Preserve the complete original LoadMissionText operation and ordered live accesses
	char filename[32];
	Messages_ClearAllMessagesDisplayedByGame();

	TextKeys_Unload(&self->mission_keyArray);
	TextData_Unload(&self->mission_data);

	bool search_result = false;
	int missionTableId = 0;

	for (missionTableId = 0; missionTableId < self->MissionTextOffsets.size; missionTableId++) {
		if (strncmp(self->MissionTextOffsets.data[missionTableId].szMissionName, MissionTableName, strlen(self->MissionTextOffsets.data[missionTableId].szMissionName)) == 0) {
			search_result = true;
			break;
		}
	}

	if (!search_result) {
		printf("CText::LoadMissionText - couldn't find %s", MissionTableName);
		return;
	}

	FileMgr_SetDir("TEXT");
	switch (Text_BackendLanguage()) {
	case TEXT_LANGUAGE_AMERICAN:
		sprintf(filename, "AMERICAN.GXT");
		break;
	case TEXT_LANGUAGE_FRENCH:
		sprintf(filename, "FRENCH.GXT");
		break;
	case TEXT_LANGUAGE_GERMAN:
		sprintf(filename, "GERMAN.GXT");
		break;
	case TEXT_LANGUAGE_ITALIAN:
		sprintf(filename, "ITALIAN.GXT");
		break;
	case TEXT_LANGUAGE_SPANISH:
		sprintf(filename, "SPANISH.GXT");
		break;
#ifdef MORE_LANGUAGES
	case TEXT_LANGUAGE_POLISH:
		sprintf(filename, "POLISH.GXT");
		break;
	case TEXT_LANGUAGE_RUSSIAN:
		sprintf(filename, "RUSSIAN.GXT");
		break;
	case TEXT_LANGUAGE_JAPANESE:
		sprintf(filename, "JAPANESE.GXT");
		break;
#endif
	}
	Timer_Suspend();
	int file = FileMgr_OpenFile(filename, "rb");
	FileMgr_Seek(file, self->MissionTextOffsets.data[missionTableId].offset, SEEK_SET);

	char TableCheck[8];
	FileMgr_Read(file, TableCheck, 8);
	if (strncmp(TableCheck, MissionTableName, 8) != 0)
		printf("CText::LoadMissionText - expected to find %s in the text file", MissionTableName);

	bool tkey_loaded = false, tdat_loaded = false;
	ChunkHeader m_ChunkHeader;
	while (!tkey_loaded || !tdat_loaded) {
		size_t bytes_read = 0;
		Text_ReadChunkHeader(self,&m_ChunkHeader, file, &bytes_read);
		if (m_ChunkHeader.size != 0) {
			if (strncmp(m_ChunkHeader.magic, "TKEY", 4) == 0) {
				size_t bytes_read = 0;
				TextKeys_Load(&self->mission_keyArray,m_ChunkHeader.size, file, &bytes_read);
				tkey_loaded = true;
			} else if (strncmp(m_ChunkHeader.magic, "TDAT", 4) == 0) {
				size_t bytes_read = 0;
				TextData_Load(&self->mission_data,m_ChunkHeader.size, file, &bytes_read);
				tdat_loaded = true;
			} else
				FileMgr_Seek(file, m_ChunkHeader.size, SEEK_CUR);
		}
	}

	TextKeys_Update(&self->mission_keyArray,self->mission_data.chars);
	FileMgr_CloseFile(file);
	Timer_Resume();
	FileMgr_SetDir("");
	strcpy(self->szMissionTableName, MissionTableName);
	self->bIsMissionTextLoaded = true;
}

void
TextKeys_Load(CKeyArray *self,size_t length, int file, size_t* offset)
{
    // Preserve the complete original Load operation and ordered live accesses
	char *rawbytes;

	// You can make self->numEntries size_t if you want to exceed 32-bit boundaries, everything else should be ready.
	self->numEntries = (int)(length / sizeof(CKeyEntry));
	self->entries = (CKeyEntry*)Text_BackendAllocate(sizeof(CKeyEntry)*self->numEntries);
	rawbytes = (char*)self->entries;

#ifdef THIS_IS_STUPID
	for (uint32_t i = 0; i < length; i++) {
		FileMgr_Read(file, &rawbytes[i], 1);
		(*offset)++;
	}
#else
	FileMgr_Read(file, rawbytes, length);
	*offset += length;
#endif
}

void
TextKeys_Unload(CKeyArray *self)
{
    // Preserve the complete original Unload operation and ordered live accesses
	Text_BackendFree(self->entries);
	self->entries = NULL;
	self->numEntries = 0;
}

void
TextKeys_Update(CKeyArray *self,uint16_t *chars)
{
    // Preserve the complete original Update operation and ordered live accesses
#if !defined(FIX_BUGS) && !defined(FIX_BUGS_64)
	int i;
	for(i = 0; i < self->numEntries; i++)
		self->entries[i].value = (uint16_t*)((uint8_t*)chars + (uintptr_t)self->entries[i].value);
#endif
}

CKeyEntry*
TextKeys_BinarySearch(CKeyArray *self,const char *key, CKeyEntry *entries, int16_t low, int16_t high)
{
    // Preserve the complete original BinarySearch operation and ordered live accesses
	int mid;
	int diff;

	if(low > high)
		return NULL;

	mid = (low + high)/2;
	diff = strcmp(key, entries[mid].key);
	if(diff == 0)
		return &entries[mid];
	if(diff < 0)
		return TextKeys_BinarySearch(self,key, entries, low, mid-1);
	if(diff > 0)
		return TextKeys_BinarySearch(self,key, entries, mid+1, high);
	return NULL;
}

uint16_t*
#if defined (FIX_BUGS) || defined(FIX_BUGS_64)
TextKeys_Search(CKeyArray *self,const char *key, uint16_t *data, uint8_t *result)
#else
TextKeys_Search(CKeyArray *self,const char *key, uint8_t *result)
#endif
{
    // Preserve the complete original Search operation and ordered live accesses
	CKeyEntry *found;
	char errstr[25];
	int i;

#if defined (FIX_BUGS) || defined(FIX_BUGS_64)
	found = TextKeys_BinarySearch(self,key, self->entries, 0, self->numEntries-1);
	if (found) {
		*result = true;
		return (uint16_t*)((uint8_t*)data + found->valueOffset);
	}
#else
	found = TextKeys_BinarySearch(self,key, self->entries, 0, self->numEntries-1);
	if (found) {
		*result = true;
		return found->value;
	}
#endif
	*result = false;
#ifdef MASTER
	sprintf(errstr, "");
#else
	sprintf(errstr, "%s missing", key);
#endif // MASTER
	for(i = 0; i < 25; i++)
		WideErrorString[i] = errstr[i];
	return WideErrorString;
}

void
TextData_Load(CData *self,size_t length, int file, size_t * offset)
{
    // Preserve the complete original Load operation and ordered live accesses
	char *rawbytes;

	// You can make self->numChars size_t if you want to exceed 32-bit boundaries, everything else should be ready.
	self->numChars = (int)(length / sizeof(uint16_t));
	self->chars = (uint16_t*)Text_BackendAllocate(sizeof(uint16_t)*self->numChars);
	rawbytes = (char*)self->chars;

#ifdef THIS_IS_STUPID
	for(uint32_t i = 0; i < length; i++){
		FileMgr_Read(file, &rawbytes[i], 1);
		(*offset)++;
	}
#else
	FileMgr_Read(file, rawbytes, length);
	*offset += length;
#endif
}

void
TextData_Unload(CData *self)
{
    // Preserve the complete original Unload operation and ordered live accesses
	Text_BackendFree(self->chars);
	self->chars = NULL;
	self->numChars = 0;
}

void
TextOffsets_Load(CMissionTextOffsets *self,size_t table_size, int file, size_t *offset, int unused)
{
    // Preserve the complete original Load operation and ordered live accesses
    (void)unused;
#ifdef THIS_IS_STUPID
	size_t num_of_entries = table_size / sizeof(CMissionTextOffsetEntry);
	for (size_t mi = 0; mi < num_of_entries; mi++) {
		for (uint32_t i = 0; i < sizeof(self->data[mi].szMissionName); i++) {
			FileMgr_Read(file, &self->data[i].szMissionName[i], 1);
			(*offset)++;
		}
		char* _buf = (char*)&self->data[mi].offset;
		for (uint32_t i = 0; i < sizeof(self->data[mi].offset); i++) {
			FileMgr_Read(file, &_buf[i], 1);
			(*offset)++;
		}
	}
	self->size = (uint16_t)num_of_entries;
#else
	// not exact VC code but smaller and better :P

	// You can make this size_t if you want to exceed 32-bit boundaries, everything else should be ready.
	self->size = (uint16_t) (table_size / sizeof(CMissionTextOffsetEntry));
	FileMgr_Read(file, (char*)self->data, sizeof(CMissionTextOffsetEntry) * self->size);
	*offset += sizeof(CMissionTextOffsetEntry) * self->size;
#endif
}

char*
UnicodeToAscii(uint16_t *src)
{
    // Preserve the complete original UnicodeToAscii conversion and static-buffer contract
	static char aStr[256];
	int len;
	for(len = 0; *src != '\0' && len < 256-1; len++, src++)
#ifdef MORE_LANGUAGES
		if(*src < 128 || ((Text_BackendRussianGame() || Text_BackendJapaneseGame()) && *src < 256))
#else
		if(*src < 128)
#endif
			aStr[len] = *src;
		// convert to CP1252
		else if(*src <= 131)
			aStr[len] = *src + 64;
		else if (*src <= 141)
			aStr[len] = *src + 66;
		else if (*src <= 145)
			aStr[len] = *src + 68;
		else if (*src <= 149)
			aStr[len] = *src + 71;
		else if (*src <= 154)
			aStr[len] = *src + 73;
		else if (*src <= 164)
			aStr[len] = *src + 75;
		else if (*src <= 168)
			aStr[len] = *src + 77;
		else if (*src <= 204)
			aStr[len] = *src + 80;
		else switch (*src) {
		case 205: aStr[len] = 209; break;
		case 206: aStr[len] = 241; break;
		case 207: aStr[len] = 191; break;
		default: aStr[len] = '#'; break;
		}
	aStr[len] = '\0';
	return aStr;
}

char*
UnicodeToAsciiForSaveLoad(uint16_t *src)
{
    // Preserve the complete original UnicodeToAsciiForSaveLoad conversion and static-buffer contract
	static char aStr[256];
	int len;
	for(len = 0; *src != '\0' && len < 256; len++, src++)
		if(*src < 256)
			aStr[len] = *src;
		else
			aStr[len] = '#';
	aStr[len] = '\0';
	return aStr;
}

char*
UnicodeToAsciiForMemoryCard(uint16_t *src)
{
    // Preserve the complete original UnicodeToAsciiForMemoryCard conversion and static-buffer contract
	static char aStr[256];
	int len;
	for(len = 0; *src != '\0' && len < 256; len++, src++)
		if(*src < 256)
			aStr[len] = *src;
		else
			aStr[len] = '#';
	aStr[len] = '\0';
	return aStr;
}

void
TextCopy(uint16_t *dst, const uint16_t *src)
{
    // Preserve the complete original TextCopy conversion and static-buffer contract
	while((*dst++ = *src++) != '\0');
}
//- rouz edit (ChatGPT)
