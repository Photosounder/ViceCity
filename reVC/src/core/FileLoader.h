#pragma once

//+ rouz edit (Claude)
// Formerly the static class CFileLoader, now C globals and functions shared with C
#ifdef __cplusplus
extern "C" {
#endif
extern char FileLoader_ms_line[256];
void FileLoader_LoadLevel(const char *filename);
char *FileLoader_LoadLine(int fd);
RwTexDictionary *FileLoader_LoadTexDictionary(const char *filename);
bool FileLoader_LoadCollisionFileFirstTime(uint8 *buffer, uint32 size, uint8 colSlot);
void FileLoader_LoadModelFile(const char *filename);
RpAtomic *FileLoader_FindRelatedModelInfoCB(RpAtomic *atomic, void *data);
bool FileLoader_StartLoadClumpFile(RwStream *stream, uint32 id);
bool FileLoader_FinishLoadClumpFile(RwStream *stream, uint32 id);
bool FileLoader_LoadAtomicFile(RwStream *stream, uint32 id);
RpAtomic *FileLoader_SetRelatedModelInfoCB(RpAtomic *atomic, void *data);
RpClump *FileLoader_LoadAtomicFile2Return(const char *filename);
void FileLoader_AddTexDictionaries(RwTexDictionary *dst, RwTexDictionary *src);
void FileLoader_LoadObjectTypes(const char *filename);
int FileLoader_LoadObject(const char *line);
int FileLoader_LoadTimeObject(const char *line);
int FileLoader_LoadWeaponObject(const char *line);
void FileLoader_LoadClumpObject(const char *line);
void FileLoader_LoadVehicleObject(const char *line);
void FileLoader_LoadPedObject(const char *line);
void FileLoader_LoadPedPathNode(const char *line, int id, int node);
void FileLoader_LoadCarPathNode(const char *line, int id, int node, bool waterPath);
void FileLoader_LoadScene(const char *filename);
void FileLoader_LoadObjectInstance(const char *line);
void FileLoader_LoadZone(const char *line);
void FileLoader_LoadCullZone(const char *line);
void FileLoader_LoadPickup(const char *line);
void FileLoader_LoadOcclusionVolume(const char *line);
void FileLoader_ReloadPaths(const char *filename);
void FileLoader_ReloadObjectTypes(const char *filename);
void FileLoader_ReloadObject(const char *line);
void FileLoader_ReLoadScene(const char *filename); // unused mobile function
#ifdef __cplusplus
}
// Members that C cannot declare yet (references, default arguments, overloads, C++ class values), with C++ linkage
void FileLoader_LoadCollisionFile(const char *filename, uint8 colSlot);
bool FileLoader_LoadCollisionFile(uint8 *buffer, uint32 size, uint8 colSlot);
void FileLoader_LoadCollisionModel(uint8 *buf, struct CColModel &model, char *name);
void FileLoader_LoadClumpFile(const char *filename);
bool FileLoader_LoadClumpFile(RwStream *stream, uint32 id);
int FileLoader_LoadPathHeader(const char *line, int &type);
#endif
//- rouz edit (Claude)
