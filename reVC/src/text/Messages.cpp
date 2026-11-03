//+ rouz edit (ChatGPT)
#include "common.h"
#include "RwHelper.h"
#include "Font.h"
#include "ControllerConfig.h"
#include "Hud.h"
#include "Messages_backend.h"

void Messages_BackendDefinedState(void)
{
    // Enter the original render-state operation before reading any display queue
    DefinedState();
}
bool Messages_BackendJapanese(void)
{
    // Read the native locale at each original text comparison or substitution
#ifdef MORE_LANGUAGES
    return CFont::IsJapanese();
#else
    return false;
#endif
}
MessagesActionNames *Messages_BackendActionNames(void)
{
    // Borrow the full actual mutable action-name array without assuming a controller layout
    return &ControlsManager.m_aActionNames;
}
void Messages_BackendCommandKeys(uint16_t action,uint16_t *output,uint16_t length)
{
    // Resolve the original captured action identifier through the native input owner
    ControlsManager.GetWideStringOfCommandKeys(action,output,length);
}
void Messages_BackendSetBigMessage(uint16_t *message,uint16_t style)
{
    // Publish the complete original UTF-16 display buffer and style through the HUD owner
    CHud::SetBigMessage(message,style);
}
void Messages_BackendSetMessage(uint16_t *message)
{
    // Publish the complete original brief display buffer through the HUD owner
    CHud::SetMessage(message);
}
void Messages_BackendClearHud(void)
{
    // Invoke the original full HUD reset between the ordered message and pager clears
    CHud::GetRidOfAllHudMessages();
}
//- rouz edit (ChatGPT)
