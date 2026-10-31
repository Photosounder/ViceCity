"""Remove the date constructor, references and comparison operators from production."""
import re
import lines_migration as lines

METHODS={'CDate':'Initialize','operator>':'IsAfter','operator<':'IsBefore','operator==':'IsEqual','PopulateDateFields':'PopulateDateFields'}

def header(source):
    # Preserve all six int fields in a C record without constructor or operator declarations
    fields=re.findall(r'\tint m_n\w+;',source)
    assert len(fields)==6
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
typedef struct CDate
{
'''+ '\n'.join(fields)+'''
} CDate;
#ifdef __cplusplus
extern "C" {
#endif
void Date_Initialize(CDate *date);
bool Date_IsAfter(const CDate *date, const CDate *right);
bool Date_IsBefore(const CDate *date, const CDate *right);
bool Date_IsEqual(const CDate *date, const CDate *right);
void Date_PopulateDateFields(CDate *date, const int8_t *second, const int8_t *minute, const int8_t *hour, const int8_t *day, const int8_t *month, int16_t year);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def calls(source):
    # Replace each actual date comparison and retain the original automatic initialization points
    if 'class CDate\n' in source:return header(source)
    source=re.sub(r'\b(date1?|date2)\s*(==|>|<)\s*(CompileDateAndTime|date[12])\b',lambda match:'Date_'+({'==':'IsEqual','>':'IsAfter','<':'IsBefore'}[match[2]])+'(&'+match[1]+', &'+match[3]+')',source)
    source=source.replace('CDate dummy; // unused','CDate dummy; // unused\n\t// Initialize the former date object at its original declaration point\n\tDate_Initialize(&dummy);')
    source=source.replace('CDate date;','CDate date;\n\t// Initialize the former date object before any subsequent load work\n\tDate_Initialize(&date);')
    source=source.replace('CDate date1, date2;','CDate date1, date2;\n\t// Initialize both former date objects in their original construction order\n\tDate_Initialize(&date1);\n\tDate_Initialize(&date2);')
    source=source.replace('CMemoryCard::CMemoryCard(void)\n{','CMemoryCard::CMemoryCard(void)\n{\n\t// Initialize the compile date before the memory-card constructor body uses it\n\tDate_Initialize(&CompileDateAndTime);')
    return lines.calls(source)

def kernel(source):
    # Preserve every original constructor, signed comparison and population statement
    source=source[source.index('CDate::CDate()'):]
    source=source.replace('CDate::CDate()','void\nDate_Initialize(CDate *date)')
    for operation in ['>','<','==']:
        source=source.replace('CDate::operator'+operation+'(const CDate &right)','Date_'+METHODS['operator'+operation]+'(const CDate *date, const CDate *right)')
    source=source.replace('CDate::PopulateDateFields(int8 &second, int8 &minute, int8 &hour, int8 &day, int8 &month, int16 year)','Date_PopulateDateFields(CDate *date, const int8_t *second, const int8_t *minute, const int8_t *hour, const int8_t *day, const int8_t *month, int16_t year)')
    source=re.sub(r'(?<![.\w])\bm_n(\w+)\b',r'date->m_n\1',source).replace('right.m_n','right->m_n')
    for name in ['second','minute','hour','day','month']:source=source.replace('= '+name+';','= *'+name+';')
    comments={'Initialize':'Clear all six fields in the original construction order','IsAfter':'Compare signed date fields from year through second for later dates','IsBefore':'Compare signed date fields from year through second for earlier dates','IsEqual':'Check all six fields in the original equality order','PopulateDateFields':'Read each original input address before assigning its date field'}
    for name,comment in comments.items():
        a=source.index('Date_'+name+'(');brace=source.index('{',a)+1
        source=source[:brace]+'\n\t// '+comment+source[brace:]
    assert 'CDate::' not in source and '&right' not in source
    return '//+ rouz edit (ChatGPT)\n#include "Date.h"\n\n'+source.rstrip()+'\n//- rouz edit (ChatGPT)\n'

def project(source):
    # Compile the complete renamed date implementation as C11
    entry='<ClCompile Include="..\\src\\save\\Date.cpp" />'
    if entry in source:return lines.project(source.replace(entry,'<ClCompile Include="..\\src\\save\\Date.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>'))
    return lines.project(source.replace('Date.cpp"','Date.c"'))
