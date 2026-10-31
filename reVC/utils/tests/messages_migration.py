"""Move all message queues, UTF-16 operations and public static methods into C11."""
import re
import object_names_migration as objects
OBSERVER=None
TABLES=['BriefMessages','BIGMessages','PreviousBriefs','PreviousMissionTitle']
FIELDS={'tMessage':['m_pText','m_nFlag','m_nTime','m_nStartTime','m_nNumber','m_pString'],'tBigMessage':['m_Stack'],'tPreviousBrief':['m_pText','m_nNumber','m_pString']}

def operations(header):
    # Read every actual public declaration while excluding the explicitly commented cut methods
    clean=re.sub(r'//[^\n]*','',header)
    result=re.findall(r'\bstatic\s+(void|uint16|bool)\s+(\w+)\(([^;]*)\);',clean)
    assert len(result)==27 and len({row[1] for row in result})==27
    return result

def primitives(text):
    # Preserve original scalar widths and UTF-16 units without introducing a second string representation
    for old,new in [('wchar','uint16_t'),('uint16','uint16_t'),('uint32','uint32_t'),('int32','int32_t'),('nil','NULL')]:text=re.sub(r'\b'+old+r'\b',new,text)
    return text

def body(source,name):
    # Retain complete conditional source without counting both preprocessor alternatives as nested C blocks
    pattern=r'(?m)^(?:[\w *]+\n)?(?:[\w *]+ )?(?:CMessages::)?'+name+r'\([^\n]*\)\n\{'
    match=re.search(pattern,source);assert match,name
    return source[match.start():source.index('\n}',match.end())+2]

def actions(source):
    # Preserve the entire conditional controller-action enumeration including all relative values
    start=source.index('enum e_ControllerAction\n');end=source.index('\n};',start)+3
    return source[start:end]

def actions_header(source):
    # Share the existing action identifiers and exact name-array extent with the C text controller
    macro=re.search(r'(?m)^#define ACTIONNAME_LENGTH \d+$',source)[0]
    return '//+ rouz edit (ChatGPT)\n#pragma once\n#include "config.h"\n'+actions(source)+'\n'+macro+'\n//- rouz edit (ChatGPT)\n'

def header(source):
    # Keep all three complete records and public arrays while removing the empty static class
    data=source[source.index('struct tMessage'):source.index('class CMessages')]
    data=re.sub(r'struct (\w+)\s*\{([\s\S]*?)\};',lambda match:'typedef struct '+match[1]+' {'+primitives(match[2])+'} '+match[1]+';',data)
    result='//+ rouz edit (ChatGPT)\n#pragma once\n#include <stdint.h>\n#include <stdbool.h>\n'+data+'\n#ifdef __cplusplus\nextern "C" {\n#endif\n'
    for kind,name,count in [('tMessage','BriefMessages','NUMBRIEFMESSAGES'),('tBigMessage','BIGMessages','NUMBIGMESSAGES'),('tPreviousBrief','PreviousBriefs','NUMPREVIOUSBRIEFS'),('char','PreviousMissionTitle','16')]:result+='extern '+kind+' Messages_'+name+'['+count+'];\n'
    for ret,name,args in operations(source):result+=primitives(ret)+' Messages_'+name+'('+primitives(args if args.strip() else 'void')+');\n'
    return result+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'

BACKEND='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "ControllerActions.h"
typedef uint16_t MessagesActionNames[MAX_CONTROLLERACTIONS][ACTIONNAME_LENGTH];
#ifdef __cplusplus
extern "C" {
#endif
void Messages_BackendDefinedState(void);
bool Messages_BackendJapanese(void);
void Messages_BackendAsciiToUnicode(const char *source,uint16_t *output);
MessagesActionNames *Messages_BackendActionNames(void);
void Messages_BackendCommandKeys(uint16_t action,uint16_t *output,uint16_t length);
void Messages_BackendSetBigMessage(uint16_t *message,uint16_t style);
void Messages_BackendSetMessage(uint16_t *message);
void Messages_BackendClearHud(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
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
void Messages_BackendAsciiToUnicode(const char *source,uint16_t *output)
{
    // Convert the original bounded numeric temporary with the native UTF-16 primitive
    AsciiToUnicode(source,output);
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
'''

def kernel(source,oldheader):
    # Translate every complete public body and both whole private helpers in their original source order
    names=[name for ret,name,args in operations(oldheader)]+['FixupChar','FastWideStringComparison']
    pieces=[body(source,name) for name in names];pieces.sort(key=source.index);chunks=[]
    for text in pieces:
        name=next(name for name in names if re.search(r'(?:CMessages::)?'+name+r'\(',text.split('{',1)[0]))
        text=text.replace('CMessages::','Messages_')
        for item in names:text=re.sub(r'(?<![\w])'+item+r'\(', 'Messages_'+item+'(',text)
        for item in TABLES:text=re.sub(r'(?<![\w])'+item+r'\b','Messages_'+item,text)
        for old,new in [('DefinedState()','Messages_BackendDefinedState()'),('CFont::IsJapanese()','Messages_BackendJapanese()'),('AsciiToUnicode(','Messages_BackendAsciiToUnicode('),('ControlsManager.m_aActionNames[cont]','(*Messages_BackendActionNames())[cont]'),('ControlsManager.GetWideStringOfCommandKeys(','Messages_BackendCommandKeys('),('CHud::SetBigMessage(','Messages_BackendSetBigMessage('),('CHud::SetMessage(','Messages_BackendSetMessage('),('CHud::GetRidOfAllHudMessages()','Messages_BackendClearHud()')]:text=text.replace(old,new)
        text=primitives(text)
        text=re.sub(r'(Messages_'+name+r')\(\)',r'\1(void)',text,1)
        if name=='FixupChar':text='static '+text
        if name=='FastWideStringComparison':text=text.replace('inline bool','static bool',1)
        text=text.replace('\n{','\n{\n    // Preserve the complete original '+name+' operation and ordered live accesses',1);chunks.append(text)
    arrays=''.join(kind+' Messages_'+name+'['+count+'];\n' for kind,name,count in [('tMessage','BriefMessages','NUMBRIEFMESSAGES'),('tPreviousBrief','PreviousBriefs','NUMPREVIOUSBRIEFS'),('tBigMessage','BIGMessages','NUMBIGMESSAGES'),('char','PreviousMissionTitle','16')])
    return '//+ rouz edit (ChatGPT)\n#define _CRT_SECURE_NO_WARNINGS\n#include "Messages.h"\n#include "Messages_backend.h"\n#include "Timer.h"\n#include "Pager.h"\n#include <stddef.h>\n#include <stdio.h>\n#include <string.h>\n'+arrays+'\n'+'\n\n'.join(chunks)+'\n//- rouz edit (ChatGPT)\n'

def message_calls(source):
    # Replace the complete owner/header, preserve controller layout and update all exact qualified callers
    if re.search(r'\bclass CMessages\s*\n',source):return header(source)
    if 'tMessage CMessages::BriefMessages[' in source:
        if OBSERVER:OBSERVER(source)
        return HOST
    old=source
    if 'class CControllerConfigManager' in source and 'enum e_ControllerAction\n' in source:
        source=source.replace(actions(source),'#include "ControllerActions.h"')
        source=re.sub(r'(?m)^#define ACTIONNAME_LENGTH \d+\n','',source)
    source=source.replace('CMessages::','Messages_')
    if source==old:return source
    if old.lstrip().startswith('//+ rouz edit (ChatGPT)') and old.rstrip().endswith('//- rouz edit (ChatGPT)'):return source
    return '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def message_project(source):
    # Register the uniquely named C queues and shared action enum beside their typed native bindings
    for tag,directory,old,new in [('ClCompile','text','Messages.cpp','MessageQueues.c'),('ClInclude','text','Messages.h','Messages_backend.h'),('ClInclude','core','ControllerConfig.h','ControllerActions.h')]:
        entry='<'+tag+' Include="..\\src\\'+directory+'\\'+old+'" />'
        addition='<'+tag+' Include="..\\src\\'+directory+'\\'+new+'"'+('>\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>' if tag=='ClCompile' else ' />')
        if '..\\src\\'+directory+'\\'+new+'"' not in source:
            source=source.replace(entry,entry+'\n    '+addition)
            pattern=r'(<'+tag+r' Include="\.\.\\src\\'+directory+r'\\'+re.escape(old)+r'">\n\s*<Filter>'+directory+r'</Filter>\n\s*</'+tag+'>)'
            source=re.sub(pattern,lambda match:match[0]+'\n    '+match[0].replace(old,new),source)
    return objects.project(source)

def verify_legacy(source,transform,root):
    # Prove the complete relocated historical producer and its full native bindings
    global OBSERVER
    captured=[];saved=OBSERVER;OBSERVER=captured.append
    try:expected=transform(source)
    finally:OBSERVER=saved
    assert len(captured)==1
    normalize=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',value))
    assert normalize(expected)==normalize((root/'src/text/Messages.cpp').read_text())
    header=(root/'build/messages-c-tests/before/src/text/Messages.h').read_text()
    assert normalize(kernel(captured[0],header))==normalize((root/'src/text/MessageQueues.c').read_text())

def verify_dependency(saved,current):
    # Retain exact byte guards unless the entire dependency follows this one verified class/enum removal
    if saved.read_bytes()==current.read_bytes():return
    old=saved.read_text();expected=calls(old);assert expected!=old and expected==current.read_text(),str(current)


def calls(source):
    # Forward historical producers and callers through the complete C text-owner conversion
    import text_migration as text
    return text.calls(message_calls(source))

def project(source):
    # Retain the existing message queues before registering the full C text owner
    import text_migration as text
    return text.project(message_project(source))
