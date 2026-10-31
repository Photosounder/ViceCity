"""Replace all four input-state classes with C records and complete C-owned operations."""
import re
import object_names_migration as objects
RECORDS=['CControllerState','CMouseControllerState','CMousePointerStateHelper','CKeyboardState']

def record(original,name):
    # Extract each complete original record with its inline operations
    return re.search(r'class '+name+r'\n\{[\s\S]*?\n\};',original)[0]

def method(original,name):
    # Match balanced method bodies so platform selection and nested blocks remain intact
    marker=original.index(name+'(');start=original.rfind('\n',0,marker)+1
    if original[start:marker].strip()=='':start=original.rfind('\n',0,start-1)+1
    else:
        prefix=original[start:marker].strip()
        if prefix.startswith(('CControllerState::','CMouseControllerState::')):start=original.rfind('\n',0,start-1)+1
    opening=original.index('{',marker);depth=1;end=opening+1
    while depth:
        if original[end]=='{':depth+=1
        elif original[end]=='}':depth-=1
        end+=1
    return original[start:end]

def fields(original,name):
    # Retain the original declarations up to the first operation without inventing initialization
    source=record(original,name).split('public:\n',1)[1]
    first={'CControllerState':'\tfloat GetLeftStickX','CMouseControllerState':'\tCMouseControllerState();','CMousePointerStateHelper':'\tCMouseControllerState GetMouseSetUp','CKeyboardState':'\tvoid Clear();'}[name]
    return source[:source.index(first)].rstrip()

def header(original):
    # Expose the records and operations without depending on the C++ pad manager
    source='//+ rouz edit (ChatGPT)\n#pragma once\n#include <stdint.h>\n#include <stdbool.h>\n'
    for name in RECORDS:source+='typedef struct '+name+' {\n'+fields(original,name).replace('int16 ','int16_t ')+'\n} '+name+';\n'
    source+='''#ifdef __cplusplus
extern "C" {
#endif
extern CMousePointerStateHelper MousePointerStateHelper;
bool ControllerState_CheckForInput(CControllerState *self);
void ControllerState_Clear(CControllerState *self);
'''
    for axis in ['LeftStickX','LeftStickY','RightStickX','RightStickY']:source+='float ControllerState_Get'+axis+'(CControllerState *self);\n'
    source+='''void KeyboardState_Clear(CKeyboardState *self);
void MouseControllerState_Init(CMouseControllerState *self);
void MouseControllerState_Clear(CMouseControllerState *self);
CMouseControllerState MousePointerState_GetMouseSetUp(CMousePointerStateHelper *self);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
    return source

BACKEND='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
bool InputState_BackendHasMouse(void);
bool InputState_BackendMenuActive(void);
bool InputState_BackendNeedsExclusive(void);
void InputState_BackendInitialiseMouse(bool exclusive);
void InputState_BackendCapabilities(uint32_t *buttons,uint32_t *axes);
double InputState_BackendCursorX(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

HOST='''
//+ rouz edit (ChatGPT)
#if defined RW_D3D9 || defined RWLIBS
bool InputState_BackendHasMouse(void)
{
    // Borrow the current platform mouse availability
    return PSGLOBAL(mouse) != nil;
}
bool InputState_BackendMenuActive(void)
{
    // Borrow the current frontend flag without changing it
    return FrontEndMenuManager.m_bMenuActive;
}
bool InputState_BackendNeedsExclusive(void)
{
    // Retain the platform's original exclusivity query
    return _InputMouseNeedsExclusive();
}
void InputState_BackendInitialiseMouse(bool exclusive)
{
    // Pass the selected exclusivity directly to the existing device initializer
    _InputInitialiseMouse(exclusive);
}
void InputState_BackendCapabilities(uint32_t *buttons,uint32_t *axes)
{
    // Query the original device-capability structure without changing failure handling
    DIDEVCAPS devCaps;
    devCaps.dwSize=sizeof(DIDEVCAPS);
    PSGLOBAL(mouse)->GetCapabilities(&devCaps);
    *buttons=devCaps.dwButtons;
    *axes=devCaps.dwAxes;
}
#else
double InputState_BackendCursorX(void)
{
    // Retain the original cursor default if the platform query leaves it untouched
    double xpos=1.0f,ypos;
    glfwGetCursorPos(PSGLOBAL(window),&xpos,&ypos);
    return xpos;
}
#endif
//- rouz edit (ChatGPT)
'''

METHODS=[('CControllerState::CheckForInput','ControllerState_CheckForInput','CControllerState'),('CControllerState::Clear','ControllerState_Clear','CControllerState'),('CKeyboardState::Clear','KeyboardState_Clear','CKeyboardState'),('CMouseControllerState::CMouseControllerState','MouseControllerState_Init','CMouseControllerState'),('CMouseControllerState::Clear','MouseControllerState_Clear','CMouseControllerState'),('CMousePointerStateHelper::GetMouseSetUp','MousePointerState_GetMouseSetUp','CMousePointerStateHelper')]

def kernel(original,originalHeader):
    # Project each complete original body while preserving partial clears and platform policy
    source='//+ rouz edit (ChatGPT)\n#include "config.h"\n#include "InputState.h"\n#include "InputState_backend.h"\n#define ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))\nCMousePointerStateHelper MousePointerStateHelper;\n\n'
    for old,new,owner in METHODS:
        body=method(original,old);body=body[body.index('{')+1:-1]
        if owner=='CMousePointerStateHelper':
            body=body.replace('CMouseControllerState state;','CMouseControllerState state;\n\t// Initialize the automatic record at the former constructor boundary\n\tMouseControllerState_Init(&state);')
            body=body.replace('PSGLOBAL(mouse) == nil','!InputState_BackendHasMouse()').replace('PSGLOBAL(mouse) != nil','InputState_BackendHasMouse()')
            body=body.replace('_InputInitialiseMouse(!FrontEndMenuManager.m_bMenuActive && _InputMouseNeedsExclusive())','InputState_BackendInitialiseMouse(!InputState_BackendMenuActive() && InputState_BackendNeedsExclusive())')
            body=body.replace('DIDEVCAPS devCaps;\n\t\tdevCaps.dwSize = sizeof(DIDEVCAPS);\n\n\t\tPSGLOBAL(mouse)->GetCapabilities(&devCaps);','uint32_t buttons,axes;\n\t\tInputState_BackendCapabilities(&buttons,&axes);')
            body=body.replace('devCaps.dwButtons','buttons').replace('devCaps.dwAxes','axes')
            body=body.replace('double xpos = 1.0f, ypos;\n\tglfwGetCursorPos(PSGLOBAL(window), &xpos, &ypos);','double xpos = InputState_BackendCursorX();')
            body='\n\t// Preserve the complete platform-dependent mouse capability selection\n\t(void)self;'+body
            body=body.replace('\n\t{','\n\t{\n\t\t// Select the supported buttons and wheel from the queried capabilities')
            body=body.replace('\n\t\t{','\n\t\t{\n\t\t\t// Preserve the original capability cases and fallthrough behavior')
            body=body.replace('if (xpos != 0.f) {','if (xpos != 0.f) {\n\t\t// Expose the same buttons whenever the cursor query reports a nonzero X value')
            result='CMouseControllerState'
        else:
            names=re.findall(r'\b\w+\b',re.sub(r'//[^\n]*','',fields(originalHeader,owner)))
            names=set(names)-{'int16','bool','char','float'}
            for name in sorted(names,key=len,reverse=True):
                if name[0].isdigit():continue
                body=re.sub(r'(?<![\w>])'+re.escape(name)+r'\b','self->'+name,body)
            body=body.replace('int32 i','int32_t i')
            body='\n\t// Preserve the original field updates and intentionally untouched members'+body
            result='bool' if old.endswith('CheckForInput') else 'void'
        source+=result+'\n'+new+'('+owner+' *self)\n{'+body+'}\n\n'
    for axis in ['LeftStickX','LeftStickY','RightStickX','RightStickY']:
        source+='float\nControllerState_Get'+axis+'(CControllerState *self)\n{\n    // Retain the original signed axis normalization including the negative endpoint\n    return self->'+axis+'/32767.0f;\n}\n\n'
    return source+'//- rouz edit (ChatGPT)\n'

def input_state_calls(source):
    # Remove the four complete classes, their methods and every typed call site
    original=source
    if 'class CControllerState\n' in source:
        for name in RECORDS:source=source.replace(record(source,name),'')
        source=source.replace('#pragma once','#pragma once\n#include "InputState.h"',1)
        source=source.replace('extern CMousePointerStateHelper MousePointerStateHelper;','')
    if 'CControllerState::CheckForInput' in source:
        for old,new,owner in METHODS:source=source.replace(method(source,old)+'\n','')
        source=source.replace('CMousePointerStateHelper MousePointerStateHelper;','')
        source=source.replace('#include "Pad.h"','#include "Pad.h"\n#include "InputState_backend.h"',1)
        for name in ['OldMouseControllerState','NewMouseControllerState','PCTempMouseControllerState']:
            declaration='CMouseControllerState CPad::'+name+';'
            source=source.replace(declaration,declaration+'\nstatic const int '+name+'Initialized=(MouseControllerState_Init(&CPad::'+name+'),0);')
        source+=HOST
    for name in RECORDS:source=source.replace('class '+name+';','struct '+name+';')
    receiver=r'((?:(?:CPad::)?GetPad\([01]\)->)?(?:NewState|OldState|PCTempKeyState|PCTempJoyState|PCTempMouseState|ReconState))'
    source=re.sub(receiver+r'\.(Clear|CheckForInput)\(\)',r'ControllerState_\2(&\1)',source)
    source=re.sub(r'\b(NewKeyState|OldKeyState|TempKeyState)\.Clear\(\)',r'KeyboardState_Clear(&\1)',source)
    source=re.sub(r'\b(NewMouseControllerState|OldMouseControllerState|PCTempMouseControllerState)\.Clear\(\)',r'MouseControllerState_Clear(&\1)',source)
    source=source.replace('MousePointerStateHelper.GetMouseSetUp()','MousePointerState_GetMouseSetUp(&MousePointerStateHelper)')
    if source==original:return source
    return source if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)') else '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def input_state_project(source):
    # Add the C producer and native declarations to historical projects without object collisions
    for tag,old,new in [('ClCompile','Pad.cpp','InputState.c'),('ClInclude','Pad.h','InputState.h'),('ClInclude','Pad.h','InputState_backend.h')]:
        entry='<'+tag+' Include="..\\src\\core\\'+old+'" />'
        addition='<'+tag+' Include="..\\src\\core\\'+new+'"'+('>\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>' if tag=='ClCompile' else ' />')
        if '..\\src\\core\\'+new+'"' not in source:
            source=source.replace(entry,entry+'\n    '+addition)
            pattern=r'(<'+tag+r' Include="\.\.\\src\\core\\'+re.escape(old)+r'">\n\s*<Filter>core</Filter>\n\s*</'+tag+'>)';source=re.sub(pattern,lambda match:match[0]+'\n    '+match[0].replace(old,new),source)
    return objects.project(source)


def calls(source):
    # Preserve complete input ownership before the C transmission producer and direct audio field reads
    import transmission_migration as transmission
    return transmission.calls(input_state_calls(source))

def project(source):
    # Retain prior registrations before renaming the complete transmission producer to C11
    import transmission_migration as transmission
    return transmission.project(input_state_project(source))
