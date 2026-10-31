//+ rouz edit (ChatGPT)
#include "AnimBlendList.h"
#include <stddef.h>
void AnimBlendLink_Init(CAnimBlendLink *self)
{
    // Clear the two original link fields in their original order
    self->next=NULL;self->prev=NULL;
}
void AnimBlendLink_Prepend(CAnimBlendLink *self,CAnimBlendLink *link)
{
    // Keep each original pointer read and write live when either node aliases a neighbor
    if(self->next)self->next->prev=link;
    link->next=self->next;link->prev=self;self->next=link;
}
void AnimBlendLink_Remove(CAnimBlendLink *self)
{
    // Retain both conditional neighbor writes before clearing the detached link
    if(self->prev)self->prev->next=self->next;
    if(self->next)self->next->prev=self->prev;
    AnimBlendLink_Init(self);
}
//- rouz edit (ChatGPT)
