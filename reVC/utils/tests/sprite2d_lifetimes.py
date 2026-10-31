"""Replace each original implicit 2D sprite lifetime without retaining an owner wrapper class."""
import re


def global_lifetimes(source):
    # Register cleanup at each original definition rather than grouping across unrelated global constructors
    pattern=r'(?m)^(static\s+)?CSprite2d\s+([A-Za-z_]\w*(?:::\w+)*)(\s*\[([^\]\n]+)\])?\s*;'
    changes=[]
    for match in re.finditer(pattern,source):
        # Preserve actual configuration-dependent array extents and lexical initializer ordering
        name=match[2];count=match[4] or '1';target=name if match[4] else '&'+name;label=name.replace('::','_');exitName='Sprite2d_Exit_'+label;startName='Sprite2d_Start_'+label
        code=match[0]+'\nstatic void '+exitName+'(void)\n{\n    // Destroy this original object group in its former reverse array order\n    Sprite2d_DeinitArray('+target+',(size_t)('+count+'));\n}\nstatic int '+startName+'(void)\n{\n    // Initialize at the original definition boundary and retain its exit registration position\n    Sprite2d_InitArray('+target+',(size_t)('+count+'));atexit('+exitName+');return 0;\n}\nstatic const int Sprite2d_Started_'+label+'='+startName+'();'
        changes.append((match.start(),match.end(),code))
    for first,last,code in reversed(changes):source=source[:first]+code+source[last:]
    return source


def calls(source):
    # Keep the private font button array as file-local C records without adding access wrappers to the font class
    if 'CSprite2d CFont::ButtonSprite[MAX_BUTTON_ICONS];' in source:
        # Move only the private array whose complete use set is inside this translation unit
        source=source.replace('CSprite2d CFont::ButtonSprite[MAX_BUTTON_ICONS];','static CSprite2d fontButtonSprites[MAX_BUTTON_ICONS];');source=re.sub(r'\bButtonSprite\b','fontButtonSprites',source)
    source=source.replace('\tstatic CSprite2d ButtonSprite[MAX_BUTTON_ICONS];','')
    source=global_lifetimes(source)
    if 'static CSprite2d sprite;' in source:
        # Keep the HUD's original first-call guarded initialization and cleanup registration rather than making it eager
        assert '#include "Sprite2d.h"' in source
        helper="""
static CSprite2d *Sprite2d_HudLocalTarget;
static void Sprite2d_ExitHudLocal(void)
{
    // Destroy the same lazily initialized C record at its original process-exit position
    Sprite2d_Deinit(Sprite2d_HudLocalTarget);
}
"""
        source=source.replace('#include "Sprite2d.h"','#include "Sprite2d.h"\n'+helper,1)
        source=source.replace('static CSprite2d sprite;',"""static CSprite2d sprite;
                            // Preserve the original local-static guard while initializing the C record and registering its cleanup
                            static const int sprite2dLocalStarted=[](){
                                // Initialize and register the same lazily owned C record
                                Sprite2d_Init(&sprite);Sprite2d_HudLocalTarget=&sprite;atexit(Sprite2d_ExitHudLocal);return 0;}();
                            (void)sprite2dLocalStarted;""")
    constructor=re.search(r'CMenuManager::CMenuManager\((?:void)?\)\s*\{',source)
    if constructor:
        # Both menu layouts have only scalar or trivial members after their sprite array so body entry retains construction order
        field='m_aFrontEndSprites' if 'm_aFrontEndSprites' in source else 'm_sprites';code='\n    // Initialize embedded C sprites before the original parent constructor body\n    Sprite2d_InitArray('+field+',sizeof('+field+')/sizeof('+field+'[0]));\n';source=source[:constructor.end()]+code+source[constructor.end():]
    old='~CMenuManager(void) { UnloadTextures(); }'
    if old in source:
        # Retain explicit parent shutdown before the former implicit array destruction
        source=source.replace(old,'~CMenuManager(void)\n    {\n        // Retain parent shutdown before reversing embedded C sprite lifetimes\n        UnloadTextures();Sprite2d_DeinitArray(m_aFrontEndSprites,sizeof(m_aFrontEndSprites)/sizeof(m_aFrontEndSprites[0]));\n    }')
    old='#ifdef FIX_BUGS\n\t~CMenuManager(void)\n\t{\n\t\tUnloadTextures();\n\t}\n#endif'
    if old in source:
        # The original implicit array teardown also exists when the PS2 fix flag disables its explicit destructor body
        source=source.replace(old,"""#ifdef FIX_BUGS
    ~CMenuManager(void)
    {
        // Retain parent shutdown before reversing embedded C sprite lifetimes
        UnloadTextures();Sprite2d_DeinitArray(m_sprites,sizeof(m_sprites)/sizeof(m_sprites[0]));
    }
#else
    ~CMenuManager(void)
    {
        // Retain original implicit member destruction even without the optional parent shutdown body
        Sprite2d_DeinitArray(m_sprites,sizeof(m_sprites)/sizeof(m_sprites[0]));
    }
#endif""")
    return source
