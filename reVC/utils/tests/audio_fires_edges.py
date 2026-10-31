"""Generate defined cached-distance extinguishing comparisons without rewriting live fixtures."""
from pathlib import Path
root=Path(__file__).resolve().parents[2];base=root/'build/audio-fires-c-tests';out=base/'edges';out.mkdir(exist_ok=True)
main='''int main() {
    // Keep console-only failures quiet on Windows
#ifdef _WIN32
    SetErrorMode(3);
#endif
    // Exercise extinguishing after the burning distance gate skips its distance update
    const float distances[]={0,15.999f,16,60,79.999f,80,80.001f,125};
    const float countdowns[]={0,0.01f,0.5f,1};
    for(unsigned owner=0;owner<2;++owner)for(float distance:distances)for(float countdown:countdowns){
        prepare(owner);seedValue=0;for(unsigned i=0;i<NUM_FIRES;++i)gFireManager.m_aFires[i].m_bIsOngoing.v=0;
        // Enable one far-away unattached fire while retaining a defined cached distance
        CFire *f=&gFireManager.m_aFires[0];f->m_bIsOngoing.v=1;f->m_bAudioSet.v=1;f->m_pEntity=NULL;f->m_vecPos={500,0,0};f->m_bExtinguishedWithWater.v=1;f->m_fWaterExtinguishCountdown.v=countdown;active->m_sQueueSample.m_fDistance=distance;
        @RUN@;observe();assert(!f->m_bExtinguishedWithWater.v);assert(active->m_sQueueSample.m_fDistance==distance);++cases;
        // Verify a second scan retains the cached distance after the water flag was consumed
        @RUN@;observe();assert(active->m_sQueueSample.m_fDistance==distance);++cases;
    }
    // Require audible cached-distance extinguishing and strict burning rejection
    assert(waters&&submits==waters);printf("%u %u %u %llu\\n",cases,submits,waters,(unsigned long long)digest);return 0;
}
'''
for variant in ['before','after']:
    # Retain the exact generated original or C implementation and all observers
    source=(base/(variant+'.cpp')).read_text();source=source[:source.index('\nint main() {')]+'\n'+main.replace('@RUN@','active->ProcessFires(0)' if variant=='before' else 'TestFiresC(active,0)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
for name in ['caller.c','fast_hash.c','default.h']:(out/name).write_bytes((base/name).read_bytes())
print('Cached-distance extinguishing fixture generated separately')
