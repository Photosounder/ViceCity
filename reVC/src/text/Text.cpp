//+ rouz edit (ChatGPT)
#include "common.h"
#include "Frontend.h"
#include "Game.h"
#include "Text_backend.h"

int32_t Text_BackendLanguage(void)
{
    // Read the original live language preference at each file-selection switch
    return FrontEndMenuManager.m_PrefsLanguage;
}
bool Text_BackendRussianGame(void)
{
    // Query the original native locale flag only at its original short-circuit position
#ifdef MORE_LANGUAGES
    return CGame::russianGame;
#else
    return false;
#endif
}
bool Text_BackendJapaneseGame(void)
{
    // Query the original native locale flag only after the original Russian-language test
#ifdef MORE_LANGUAGES
    return CGame::japaneseGame;
#else
    return false;
#endif
}
void *Text_BackendAllocate(size_t size)
{
    // Preserve the native allocation provider selected by the existing common header
    return malloc(size);
}
void Text_BackendFree(void *pointer)
{
    // Release text ownership through the same native provider that allocated its buffers
    free(pointer);
}
//- rouz edit (ChatGPT)
