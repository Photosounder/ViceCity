#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "ModelAnimFile.h"

static int lookups;
int32_t AnimManager_GetAnimationBlockIndex(const char *name)
{
    // Check the copied name at the animation manager boundary
    ++lookups;
    assert(strcmp(name,"weapon_block")==0);
    return 42;
}
int main(void)
{
    // Preserve the sentinel for all case variants of the null token
    ModelAnimFile anim;
    anim.index=-1;
    ModelAnimFile_Set(&anim,"NuLl");
    ModelAnimFile_Convert(&anim);
    assert(anim.index==-1 && lookups==0);
    // Copy and resolve a non-null animation name
    char name[]="weapon_block";
    ModelAnimFile_Set(&anim,name);
    assert(anim.name!=name && strcmp(anim.name,name)==0);
    ModelAnimFile_Convert(&anim);
    assert(anim.index==42 && lookups==1);
    return 0;
}
