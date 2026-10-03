"""Prepare current C and original C++ device enumeration fixtures."""
from pathlib import Path
import sys
root = Path(__file__).resolve().parents[2]
baseline = '--baseline' in sys.argv
if baseline:
    header = 'build/audio-device-c-tests/before-aldlist.h'
    source = (root / 'build/audio-device-c-tests/before-aldlist.cpp').read_text().replace('#include "aldlist.h"', '')
else:
    header = 'src/audio/oal/aldlist.h'
    source = '#include "src/audio/oal/aldlist.c"'
host_test = ''
if not baseline:
    game = (root / 'src/audio/sampman_oal.c').read_text()
    host_body = game[game.index('static ALDeviceList cachedDeviceList;'):game.index('static void\nrelease_existing()')]
    host_test = """
#if defined(__cplusplus) && !defined(BASELINE_DEVICE)
static void configure(unsigned test, unsigned count);
static void (*exit_cleanup)(void);
static unsigned exit_registrations;
static int fake_atexit(void (*callback)(void)) {
    // Capture process-exit cleanup without running any application shutdown code
    exit_cleanup = callback; exit_registrations++; return 0;
}
#define atexit fake_atexit
#define MAXPROVIDERS 64
static int defaultProvider;
static struct { const char *id; char name[256]; unsigned sources; bool bSupportsFx; } providers[MAXPROVIDERS];
static unsigned provider_count;
static struct FakeSampleManager { unsigned unused; } SampleManager;
static void SampleManager_SetNum3DProvidersAvailable(FakeSampleManager*, unsigned count) {
    // Record provider count publication through the actual add_providers function
    provider_count = count;
}
static void SampleManager_Set3DProviderName(FakeSampleManager*, int, const char*) {
    // Leave the menu name copy outside cached device-name ownership checks
}
static unsigned Min(unsigned a, unsigned b) {
    // Supply the original provider-count clamp from the game header
    return a < b ? a : b;
}
extern "C" ALenum AL_APIENTRY alGetEnumValue(const ALchar*) {
    // Force the effects-capable provider variants without consulting a driver
    return 1;
}
""" + host_body + """
#undef atexit
static void check_cache_lifetime(void) {
    // Execute the actual provider initialization and exit-cleanup functions from sampman_oal.c
    configure(1110,3);
    add_providers();
    assert(provider_count == 3 && allocations == 3 && exit_registrations == 1);
    const char *name = providers[0].id;
    assert(name && strcmp(name,names[0]) == 0);
    char saved_name[32]; strcpy(saved_name,name);
    strcpy(names[0],"changed-driver-name");
    unsigned names_owned = allocations;
    add_providers();
    assert(allocations == names_owned && providers[0].id == name && exit_registrations == 1);
    assert(strcmp(name,saved_name) == 0);
    assert(exit_cleanup);
    exit_cleanup(); exit_cleanup();
    assert(allocations == 0 && opens == 0 && contexts == 0);
}
#endif
"""
result = (root / 'utils/tests/audio_device.c.in').read_text().replace('@HEADER@',header).replace('@SOURCE@',source).replace('@HOST_TEST@',host_test)
if baseline: result = '#define BASELINE_DEVICE\n' + result
Path(sys.argv[1]).write_text(result,newline='\n')
