"""Compare exact menu destructor and UnloadTextures bodies with implicit or C member cleanup."""
from pathlib import Path
import ctypes,json,os,re,subprocess
from memory_heap_migration import body
from rect_migration import mask

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/sprite2d-c-tests';probe=out/'runtime';results=[]
ctypes.windll.kernel32.SetErrorMode(0x8003)
for profile in ['pc','ps2','ps2-classic']:
    # Exercise the resource member lifetimes separately from unrelated parent constructor policy
    pc=profile=='pc';field='m_aFrontEndSprites' if pc else 'm_sprites';count=31 if pc else 28
    filename='Frontend' if pc else 'Frontend_PS2'
    for phase in ['before','after']:
        # Extract complete actual shutdown bodies from each immutable original or coherent staged source
        before=phase=='before';tree=out/('before' if before else 'overlay');header=(tree/f'src/core/{filename}.h').read_text();source=(tree/f'src/core/{filename}.cpp').read_text()
        unload=body(source,'CMenuManager::UnloadTextures(')
        if before and profile=='ps2-classic':destructor=''
        elif not before and profile=='ps2-classic':
            # Select the actual explicit cleanup added for the former implicit-only destructor
            branch=header[header.index('// Retain original implicit member destruction'):]
            destructor='~CMenuManager(void){'+branch[:branch.index('}')]+ '}'
        else:destructor='~CMenuManager(void){'+body(header,'~CMenuManager(void)')+'}'
        init=''
        if not before:
            # Use the exact initialization expression inserted at the actual parent constructor boundary
            constructor=body(source,'CMenuManager::CMenuManager(');init=re.search(r'Sprite2d_InitArray\([^;]+;',constructor)[0]
        sdk=(root/'utils/tests/sprite2d_sdk.cpp.in').read_text().replace('need(texture&&texture->alive);event(3,texture->id);','need(texture&&texture->alive);need(expectedAt<expectedCount&&expected[expectedAt++]==texture->id);event(3,texture->id);')
        code='#include "owners.h"\n#include <new>\n'+('#include "original-Sprite2d.h"\n' if before else '#include "Sprite2d.h"\n')+('#undef FIX_BUGS\n' if profile=='ps2-classic' else '')+'static unsigned expected[1024],expectedAt,expectedCount;\n'+sdk
        code+='''
enum {MENUPAGE_SOUND_SETTINGS=7,SOUND_FRONTEND_MENU_STARTING=8,MUSICMODE_GAME=9,NUM_MENU_SPRITES=31,NUM_SPRIRES=28};
static int PlaceName_state;
static void DMAudio_StopFrontEndTrack(void) {event(40);}
static void DMAudio_PlayFrontEndSound(int value,int extra) {event(41,value,extra);}
static void DMAudio_ChangeMusicMode(int value) {event(42,value);}
static int TxdStore_FindTxdSlot(const char *name) {event(43);bytes(name,strlen(name)+1);return 3;}
static void TxdStore_RemoveTxd(int slot) {event(44,slot);}
static void PlaceName_ProcessAfterFrontEndShutDown(int *state) {need(state==&PlaceName_state);event(45);}
'''
        record='CSprite2d' if before else 'Sprite2d'
        code+='struct CMenuManager { '+record+' '+field+'['+str(count)+'];bool m_bSpritesLoaded,m_OnlySaveMenu,m_bTexturesLoaded;int m_nCurrScreen;\nCMenuManager(void){\n    // Limit this projection to the exact embedded sprite initialization boundary\n'+init+'\n}\n'+destructor+'\nvoid UnloadTextures(void);};\nvoid CMenuManager::UnloadTextures(void){'+unload+'}\n'
        code+='int main(void)\n{\n    // Cover unloaded loaded partial save-menu repeated-shutdown and audio-screen paths\n    for(unsigned scenario=0;scenario<6;++scenario){\n        // Construct actual sprite members and populate independently owned SDK textures\n        alignas(CMenuManager) unsigned char storage[sizeof(CMenuManager)]={};auto *menu=new(storage) CMenuManager();menu->m_bSpritesLoaded=menu->m_bTexturesLoaded=scenario!=0;menu->m_OnlySaveMenu=scenario==2;menu->m_nCurrScreen=scenario==3?MENUPAGE_SOUND_SETTINGS:0;unsigned ids['+str(count)+']={};\n        for(unsigned i=0;i<'+str(count)+';++i){need(menu->'+field+'[i].m_pTexture==NULL);if(scenario==4&&i%3==0)continue;auto *texture=RwTextureRead("menu",NULL);menu->'+field+'[i].m_pTexture=texture;ids[i]=texture->id;}\n        expectedAt=expectedCount=0;\n'
        forward='scenario!=0' if pc or profile!='ps2-classic' else 'scenario==5'
        # Predict resource order independently from actual implicit array destruction or C loops
        code+='        unsigned split='+('scenario==2?3:'+str(count) if pc else str(count))+';if('+forward+'){for(unsigned i=0;i<split;++i)if(ids[i])expected[expectedCount++]=ids[i];}else split=0;for(unsigned i='+str(count)+';i>split;--i)if(ids[i-1])expected[expectedCount++]=ids[i-1];\n'
        # A legacy PS2 explicit unload does not delete sprite textures; its later implicit destruction does
        if profile=='ps2-classic':code=code.replace('unsigned split='+str(count)+';if(scenario==5)', 'unsigned split='+str(count)+';if(false)')
        code+='        if(scenario==5){menu->UnloadTextures();menu->UnloadTextures();}menu->~CMenuManager();need(expectedAt==expectedCount);\n    }\n    need(reads==destroys);printf("scenarios=6 reads=%u destroys=%u events=%u hash=%016llx\\n",reads,destroys,events,(unsigned long long)hashValue);return 0;\n}\n'
        (probe/f'menu-{profile}-{phase}.cpp').write_text(code,newline='\n')
    for bits in [32,64]:
        for opt in ['Od','O2']:
            # Link the complete original or C sprite producer rather than a lifecycle policy stub
            outputs=[]
            for phase in ['before','after']:
                label=f'menu-{profile}-{bits}-{opt}-{phase}';existing=f'msvc-{bits}-{opt}-d3d9-{phase}';tree=out/('before' if phase=='before' else 'overlay');obj=probe/(label+'.obj');exe=probe/(label+'.exe')
                units=['Rect','original-owner','before-caller'] if phase=='before' else ['Rect','after-owner','after-caller','native','peers-native'];objects=[probe/(existing+'-'+u+'.obj') for u in units];assert all(p.exists() for p in objects)
                flags='/nologo /c /MT /EHsc /std:c++14 /fp:precise /'+opt+' /DLIBRW /DRW_D3D9 /D_CRT_SECURE_NO_WARNINGS '+' '.join('/I"'+str(p)+'"' for p in [probe]+[p for p in (tree/'src').rglob('*') if p.is_dir()])
                lines=['@echo off',f'call "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build/vcvars{bits}.bat" >nul','if errorlevel 1 exit /b 1',f'cl {flags} /Fo"{obj}" "{probe / ("menu-"+profile+"-"+phase+".cpp")}"','if errorlevel 1 exit /b 1','link /nologo '+' '.join('"'+str(p)+'"' for p in [obj]+objects)+' /OUT:"'+str(exe)+'"','exit /b %errorlevel%'];driver=probe/(label+'.cmd');driver.write_text('\n'.join(lines)+'\n',newline='\n')
                run=subprocess.run(['cmd.exe','/c',str(driver)],capture_output=True,text=True);(probe/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.stdout[-4000:],run.stderr)
                env=dict(os.environ);key=next(k for k in env if k.lower()=='path');env[key]='C:/Windows/System32;C:/Windows';run=subprocess.run([str(exe)],capture_output=True,text=True,env=env,timeout=30);assert run.returncode==0,(label,run.returncode,run.stdout,run.stderr);outputs.append(run.stdout.strip())
            assert outputs[0]==outputs[1],(profile,bits,opt,outputs);results.append({'profile':profile,'bits':bits,'optimization':opt,'before':outputs[0],'after':outputs[1],'exit':0});print(profile,bits,opt,outputs[0].splitlines()[-1],flush=True)
(out/'menu-lifetime-runs.json').write_text(json.dumps(results,indent=2),newline='\n')
