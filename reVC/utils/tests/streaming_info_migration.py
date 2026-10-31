"""Move all streaming-info record operations and shared identifiers into standalone C11."""
import re
import object_names_migration as objects
OPERATIONS=['GetCdPosnAndSize','SetCdPosnAndSize','AddToList','RemoveFromList','GetCdSize','IsPriority']

def header(original):
    # Preserve both shared enums and every field without importing the C++ streaming manager
    enums=original[original.index('enum StreamFlags'):original.index('enum ChannelState')]
    fields=original[original.index('\tCStreamingInfo *m_next;'):original.index('\n\tbool GetCdPosnAndSize')]
    fields=fields.replace('CStreamingInfo *','struct CStreamingInfo *').replace('uint8','uint8_t').replace('int16','int16_t').replace('uint32','uint32_t')
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
'''+enums+'''typedef struct CStreamingInfo {
'''+fields+'''
} CStreamingInfo;
#ifdef __cplusplus
extern "C" {
#endif
bool StreamingInfo_GetCdPosnAndSize(CStreamingInfo *self,uint32_t *posn,uint32_t *size);
void StreamingInfo_SetCdPosnAndSize(CStreamingInfo *self,uint32_t posn,uint32_t size);
void StreamingInfo_AddToList(CStreamingInfo *self,CStreamingInfo *link);
void StreamingInfo_RemoveFromList(CStreamingInfo *self);
uint32_t StreamingInfo_GetCdSize(CStreamingInfo *self);
bool StreamingInfo_IsPriority(CStreamingInfo *self);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def definitions(original):
    # Extract all four complete out-of-line methods without changing their statement ordering
    pattern=r'(?:bool|void)\nCStreamingInfo::(?:GetCdPosnAndSize|SetCdPosnAndSize|AddToList|RemoveFromList)\([^)]*\)\n\{[\s\S]*?\n\}'
    found=re.findall(pattern,original);assert len(found)==4
    return found

def kernel(original):
    # Project the complete original operations and retain ordered reads through aliased outputs
    methods=[]
    for definition in definitions(original):
        name=re.search(r'CStreamingInfo::(\w+)',definition)[1]
        source=definition.replace('CStreamingInfo::'+name,'StreamingInfo_'+name).replace('uint32','uint32_t').replace('uint32_t &posn, uint32_t &size','uint32_t *posn, uint32_t *size')
        source=source.replace('(void)','(CStreamingInfo *self)') if name=='RemoveFromList' else source.replace('(', '(CStreamingInfo *self, ',1)
        source=re.sub(r'(?<![\w>])m_(next|prev|position|size)\b',r'self->m_\1',source)
        source=re.sub(r'\bthis\b','self',source).replace('nil','NULL')
        if name=='GetCdPosnAndSize':source=source.replace('\tposn =','\t*posn =').replace('\tsize =','\t*size =')
        source=source.replace('\n{','\n{\n\t// Preserve the original record operation and its ordered writes',1)
        methods.append(source)
    methods+=['''uint32_t
StreamingInfo_GetCdSize(CStreamingInfo *self)
{
    // Return the record's stored sector count without changing its state
    return self->m_size;
}''','''bool
StreamingInfo_IsPriority(CStreamingInfo *self)
{
    // Test only the original priority flag bit
    return !!(self->m_flags & STREAMFLAGS_PRIORITY);
}''']
    return '//+ rouz edit (ChatGPT)\n#include "StreamingInfo.h"\n#include <stddef.h>\n\n'+'\n\n'.join(methods)+'\n//- rouz edit (ChatGPT)\n'

def streaming_info_calls(source):
    # Remove the full class and all native methods before replacing each typed manager caller
    original=source
    if 'class CStreamingInfo\n' in source:
        source=re.sub(r'enum StreamFlags\n\{[\s\S]*?(?=enum ChannelState)', '#include "StreamingInfo.h"\n\n',source,count=1)
        source=re.sub(r'class CStreamingInfo\n\{[\s\S]*?\n\};\n','',source,count=1)
    if 'CStreamingInfo::GetCdPosnAndSize' in source:
        for definition in definitions(source):source=source.replace(definition+'\n','')
    record=r'(ms_aInfoForModel\[[^\]]+\])'
    source=re.sub(record+r'\.(GetCdPosnAndSize)\(([^,()]+), ([^,()]+)\)',r'StreamingInfo_\2(&\1, &\3, &\4)',source)
    source=re.sub(r'\b(streamingInfo)->GetCdPosnAndSize\(([^,()]+), ([^,()]+)\)',r'StreamingInfo_GetCdPosnAndSize(\1, &\2, &\3)',source)
    source=re.sub(record+r'\.(SetCdPosnAndSize|AddToList)\(',r'StreamingInfo_\2(&\1, ',source)
    source=re.sub(record+r'\.(GetCdSize|RemoveFromList|IsPriority)\(\)',r'StreamingInfo_\2(&\1)',source)
    source=re.sub(r'\b(si)->(RemoveFromList|IsPriority)\(\)',r'StreamingInfo_\2(\1)',source)
    if source==original:return source
    return source if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)') else '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def streaming_info_project(source):
    # Register the independent C producer in historical projects while retaining unique object names
    for tag,old,new in [('ClCompile','Streaming.cpp','StreamingInfo.c'),('ClInclude','Streaming.h','StreamingInfo.h')]:
        entry='<'+tag+' Include="..\\src\\core\\'+old+'" />'
        addition='<'+tag+' Include="..\\src\\core\\'+new+'"'+('>\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>' if tag=='ClCompile' else ' />')
        if '..\\src\\core\\'+new+'"' not in source:
            source=source.replace(entry,entry+'\n    '+addition)
            pattern=r'(<'+tag+r' Include="\.\.\\src\\core\\'+re.escape(old)+r'">\n\s*<Filter>core</Filter>\n\s*</'+tag+'>)'
            source=re.sub(pattern,lambda match:match[0]+'\n    '+match[0].replace(old,new),source)
    return objects.project(source)


def calls(source):
    # Preserve streaming-info ownership before removing the four complete input-state classes
    import input_state_migration as input_state
    return input_state.calls(streaming_info_calls(source))

def project(source):
    # Retain prior registrations before the independent C input-state producer
    import input_state_migration as input_state
    return input_state.project(streaming_info_project(source))
