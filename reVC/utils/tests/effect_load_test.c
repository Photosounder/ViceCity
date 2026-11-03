#include <assert.h>
#include <string.h>
#include "2dEffect.h"
#include "Store.h"
#include "ModelRegistry.h"

static C2dEffect entries[4];
static CStore store={0,entries,sizeof(C2dEffect),4};
static int trace,model;
void re3_assert(const char *e,const char *f,unsigned int l,const char *n) {
    // Fail if the fixture exceeds the fixed store
    (void)e;(void)f;(void)l;(void)n;assert(0);
}
void TxdStore_PushCurrentTxd(void) {
    // Record the texture dictionary push
    trace=trace*10+1;
}
int TxdStore_FindTxdSlot(const char *name) {
    // Verify and record the particle dictionary lookup
    assert(!strcmp(name,"particle"));trace=trace*10+2;return 5;
}
void TxdStore_SetCurrentTxd(int slot) {
    // Record the selected dictionary
    assert(slot==5);trace=trace*10+3;
}
void TxdStore_PopCurrentTxd(void) {
    // Record the dictionary restoration
    trace=trace*10+6;
}
void *ModelRegistry_Get(int id) {
    // Resolve the controlled model
    assert(id==7);trace=trace*10+4;return &model;
}
CStore *ModelRegistry_EffectsStore(void) {
    // Return the controlled effect store
    return &store;
}
void ModelRegistry_AddEffect(void *m,void *e) {
    // Verify and record the allocated effect registration
    assert(m==&model && e==&entries[store.allocPtr-1]);trace=trace*10+5;
}
void *RwHelper_ReadTexture(const char *name,const char *mask) {
    // Return distinct handles for the original texture names
    assert(!mask);return !strcmp(name,"corona") ? &entries[0] : !strcmp(name,"shadow") ? &entries[1] : 0;
}
int main(void) {
    // Check common fields, light flags, texture selection, and call order
    Effect2d_Load("7 1 2 3 260 2 3 4 0 \"corona\" \"shadow\" 10 20 30 40 50 2 3 4 6");
    assert(trace==123456 && entries[0].pos.x==1 && entries[0].col.r==4 && entries[0].light.flags==4);
    assert(entries[0].light.corona==&entries[0] && entries[0].light.shadow==&entries[1]);
    // Check particle payload and attractor probability clamping
    trace=0;Effect2d_Load("7 1 2 3 1 2 3 4 1 9 4 5 6 7");
    assert(trace==123456 && entries[1].particle.particleType==9 && entries[1].particle.dir.y==5 && entries[1].particle.scale==7);
    trace=0;Effect2d_Load("7 1 2 3 1 2 3 4 2 8 4 5 6 300");
    assert(trace==123456 && entries[2].attractor.type==8 && entries[2].attractor.probability==255);
    // Check pedestrian attractor vectors
    trace=0;Effect2d_Load("7 1 2 3 1 2 3 4 3 8 4 5 6 7 8 9");
    assert(trace==123456 && entries[3].pedattr.type==8 && entries[3].pedattr.useDir.z==9);
    return 0;
}
