#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "eax-util.h"
static uint64_t digest = UINT64_C(1469598103934665603);
static unsigned calls;
static void record(const void *data, size_t length) {
    // Compare every output byte and rejected-input sentinel with the saved C++ implementation
    const unsigned char *bytes=(const unsigned char *)data;
    for(size_t i=0; i<length; i++) { digest^=bytes[i]; digest*=UINT64_C(1099511628211); }
}
static void exercise(EAXLISTENERPROPERTIES *start, EAXLISTENERPROPERTIES *end, float ratio, bool check) {
    // Exercise the complete production interpolation entry point
    EAXLISTENERPROPERTIES output; memset(&output,0xa5,sizeof(output));
    bool result=EAX3ListenerInterpolate(start,end,ratio,&output,check);
    unsigned accepted=result; record(&accepted,sizeof(accepted)); record(&output,sizeof(output)); calls++;
}
int main(void) {
    // Compare every preset pair across extrapolation, endpoint and fractional ratios
    EAXLISTENERPROPERTIES *groups[]={EAX30_ORIGINAL_PRESETS,EAX30_STANDARD_PRESETS[0],EAX30_STANDARD_PRESETS[1],EAX30_STANDARD_PRESETS[2],EAX30_STANDARD_PRESETS[3],EAX30_STANDARD_PRESETS[4],EAX30_SPORTS_PRESETS,EAX30_PREFAB_PRESETS,EAX30_DOMESNPIPES_PRESETS,EAX30_OUTDOORS_PRESETS,EAX30_MOOD_PRESETS,EAX30_DRIVING_PRESETS,EAX30_CITY_PRESETS,EAX30_MISC_PRESETS};
    unsigned counts[]={EAX30_NUM_ORIGINAL_PRESETS,EAX30_NUM_CASTLE_PRESETS,EAX30_NUM_FACTORY_PRESETS,EAX30_NUM_ICEPALACE_PRESETS,EAX30_NUM_SPACESTATION_PRESETS,EAX30_NUM_WOODGALLEON_PRESETS,EAX30_NUM_SPORTS_PRESETS,EAX30_NUM_PREFAB_PRESETS,EAX30_NUM_DOMESNPIPES_PRESETS,EAX30_NUM_OUTDOORS_PRESETS,EAX30_NUM_MOOD_PRESETS,EAX30_NUM_DRIVING_PRESETS,EAX30_NUM_CITY_PRESETS,EAX30_NUM_MISC_PRESETS};
    for(unsigned a=0; a<sizeof(counts)/sizeof(counts[0]); a++) {
        for(unsigned b=0; b<sizeof(counts)/sizeof(counts[0]); b++) {
            for(unsigned i=0; i<counts[a]; i++) for(unsigned j=0; j<counts[b]; j++) {
                for(int ratio=-1; ratio<=11; ratio++) {
                    exercise(&groups[a][i],&groups[b][j],ratio/10.f,false);
                    exercise(&groups[a][i],&groups[b][j],ratio/10.f,true);
                }
            }
        }
    }
    // Exercise nonzero pan vectors including the normalization path
    EAXLISTENERPROPERTIES start=EAX30_ORIGINAL_PRESETS[0], end=EAX30_ORIGINAL_PRESETS[1];
    start.vReflectionsPan.x=3.f; end.vReflectionsPan.z=-2.f;
    start.vReverbPan.y=4.f; end.vReverbPan.x=-3.f;
    for(int ratio=0; ratio<=10; ratio++) exercise(&start,&end,ratio/10.f,true);
    // Reject every out-of-range scalar in either endpoint without modifying the output sentinel
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.lRoom=EAXLISTENER_MINROOM-1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.lRoom=EAXLISTENER_MAXROOM+1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.lRoomHF=EAXLISTENER_MINROOMHF-1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.lRoomHF=EAXLISTENER_MAXROOMHF+1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.lRoomLF=EAXLISTENER_MINROOMLF-1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.lRoomLF=EAXLISTENER_MAXROOMLF+1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.ulEnvironment=EAXLISTENER_MINENVIRONMENT-1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.ulEnvironment=EAXLISTENER_MAXENVIRONMENT+1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flEnvironmentSize=EAXLISTENER_MINENVIRONMENTSIZE-1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flEnvironmentSize=EAXLISTENER_MAXENVIRONMENTSIZE+1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flEnvironmentDiffusion=EAXLISTENER_MINENVIRONMENTDIFFUSION-1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flEnvironmentDiffusion=EAXLISTENER_MAXENVIRONMENTDIFFUSION+1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flDecayTime=EAXLISTENER_MINDECAYTIME-1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flDecayTime=EAXLISTENER_MAXDECAYTIME+1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flDecayHFRatio=EAXLISTENER_MINDECAYHFRATIO-1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flDecayHFRatio=EAXLISTENER_MAXDECAYHFRATIO+1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flDecayLFRatio=EAXLISTENER_MINDECAYLFRATIO-1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flDecayLFRatio=EAXLISTENER_MAXDECAYLFRATIO+1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.lReflections=EAXLISTENER_MINREFLECTIONS-1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.lReflections=EAXLISTENER_MAXREFLECTIONS+1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flReflectionsDelay=EAXLISTENER_MINREFLECTIONSDELAY-1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flReflectionsDelay=EAXLISTENER_MAXREFLECTIONSDELAY+1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.lReverb=EAXLISTENER_MINREVERB-1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.lReverb=EAXLISTENER_MAXREVERB+1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flReverbDelay=EAXLISTENER_MINREVERBDELAY-1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flReverbDelay=EAXLISTENER_MAXREVERBDELAY+1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flEchoTime=EAXLISTENER_MINECHOTIME-1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flEchoTime=EAXLISTENER_MAXECHOTIME+1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flEchoDepth=EAXLISTENER_MINECHODEPTH-1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flEchoDepth=EAXLISTENER_MAXECHODEPTH+1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flModulationTime=EAXLISTENER_MINMODULATIONTIME-1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flModulationTime=EAXLISTENER_MAXMODULATIONTIME+1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flModulationDepth=EAXLISTENER_MINMODULATIONDEPTH-1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flModulationDepth=EAXLISTENER_MAXMODULATIONDEPTH+1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flAirAbsorptionHF=EAXLISTENER_MINAIRABSORPTIONHF-1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flAirAbsorptionHF=EAXLISTENER_MAXAIRABSORPTIONHF+1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flHFReference=EAXLISTENER_MINHFREFERENCE-1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flHFReference=EAXLISTENER_MAXHFREFERENCE+1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flLFReference=EAXLISTENER_MINLFREFERENCE-1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flLFReference=EAXLISTENER_MAXLFREFERENCE+1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flRoomRolloffFactor=EAXLISTENER_MINROOMROLLOFFFACTOR-1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
{ EAXLISTENERPROPERTIES invalid=EAX30_ORIGINAL_PRESETS[0]; invalid.flRoomRolloffFactor=EAXLISTENER_MAXROOMROLLOFFFACTOR+1; exercise(&invalid,&end,0.5f,true); exercise(&start,&invalid,0.5f,true); }
    printf("EAX calls %u digest %llx\n",calls,(unsigned long long)digest);
    return 0;
}
