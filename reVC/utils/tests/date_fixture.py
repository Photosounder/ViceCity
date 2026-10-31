"""Audit full date conversion and generate original constructor/operator versus C models."""
from pathlib import Path
import json,re
import date_migration as m

root=Path(__file__).resolve().parents[2];out=root/'build/date-c-tests'

def normalize(source):
    # Compare complete statements while ignoring comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))

old=(out/'before/src/save/Date.cpp').read_text();oldh=(out/'before/src/save/Date.h').read_text()
for path in json.loads((out/'changes.json').read_text()):
    original=(out/'before'/path).read_text()
    if path.endswith('/Date.cpp'):
        target=root/'src/save/Date.c';expected=m.kernel(original)
        assert not (root/path).exists()
    else:target=root/path;expected=m.project(original) if path.startswith('build/') else m.calls(original)
    assert normalize(expected)==normalize(target.read_text()),path
    assert b'\r' not in target.read_bytes(),path
assert len(re.findall(r'^CDate::',old,re.M))==5
assert 'class CDate' not in (root/'src/save/Date.h').read_text()
card=(out/'before/src/save/MemoryCard.cpp').read_text()
assert len(re.findall(r'\bdate[12]?\s*[<>]\s*(?:CompileDateAndTime|date2)',card))==10
assert (root/'src/save/MemoryCard.cpp').read_text().count('Date_IsAfter(')==5
assert (root/'src/save/MemoryCard.cpp').read_text().count('Date_IsBefore(')==5
assert 'date >' not in (root/'src/save/MemoryCard.cpp').read_text()
for field in ['Second','Minute','Hour','Day','Month','Year']:
    assert re.search(r'\bint m_n'+field+r';',oldh)
    assert re.search(r'\bint m_n'+field+r';',(root/'src/save/Date.h').read_text())
for name in ['Initialize','IsAfter','IsBefore','IsEqual','PopulateDateFields']:
    assert 'Date_'+name+'(' in (root/'src/save/Date.c').read_text()
assert 'Date_Initialize(&CompileDateAndTime);' in (root/'src/save/MemoryCard.cpp').read_text()
info=re.search(r'CMemoryCardInfo::CMemoryCardInfo\(void\)\n\{[^}]+\}',card)[0]
assert 'Date' not in info and 'TheMemoryCard' not in info
declarations='''#pragma once
#include "Date.h"
#ifdef __cplusplus
extern "C" {
#endif
void TestDate_Initialize(CDate *date);
bool TestDate_IsAfter(const CDate *date,const CDate *right);
bool TestDate_IsBefore(const CDate *date,const CDate *right);
bool TestDate_IsEqual(const CDate *date,const CDate *right);
void TestDate_Populate(CDate *date,int8_t *second,int8_t *minute,int8_t *hour,int8_t *day,int8_t *month,int16_t year);
#ifdef __cplusplus
}
#endif
'''
(out/'caller.h').write_text(declarations,newline='\n')
(out/'caller.c').write_text('''#include "caller.h"
void TestDate_Initialize(CDate *date)
{
    // Exercise the complete former constructor through the C API
    Date_Initialize(date);
}
bool TestDate_IsAfter(const CDate *date,const CDate *right)
{
    // Exercise the original greater-than operation through C11
    return Date_IsAfter(date,right);
}
bool TestDate_IsBefore(const CDate *date,const CDate *right)
{
    // Exercise the original less-than operation through C11
    return Date_IsBefore(date,right);
}
bool TestDate_IsEqual(const CDate *date,const CDate *right)
{
    // Exercise the original equality operation through C11
    return Date_IsEqual(date,right);
}
void TestDate_Populate(CDate *date,int8_t *second,int8_t *minute,int8_t *hour,int8_t *day,int8_t *month,int16_t year)
{
    // Preserve the original input addresses and signed year conversion through C11
    Date_PopulateDateFields(date,second,minute,hour,day,month,year);
}
''',newline='\n')
baseline='''static void TestDate_Initialize(CDate *date)
{
    // Run the untouched original constructor at the existing record address
    new(date) CDate();
}
static bool TestDate_IsAfter(const CDate *date,const CDate *right)
{
    // Invoke the exact original non-const member without mutating either record
    return const_cast<CDate *>(date)->operator>(*right);
}
static bool TestDate_IsBefore(const CDate *date,const CDate *right)
{
    // Invoke the exact original less-than member
    return const_cast<CDate *>(date)->operator<(*right);
}
static bool TestDate_IsEqual(const CDate *date,const CDate *right)
{
    // Invoke the exact original equality member
    return const_cast<CDate *>(date)->operator==(*right);
}
static void TestDate_Populate(CDate *date,int8_t *second,int8_t *minute,int8_t *hour,int8_t *day,int8_t *month,int16_t year)
{
    // Invoke the exact original reference arguments without preloading their values
    date->PopulateDateFields(*second,*minute,*hour,*day,*month,year);
}
'''
macros='\n'.join(line for line in card.splitlines() if line.startswith(('#define ReadDataFromBufferPointer(','#define WriteDataToBufferPointer(')))
save='\n'.join(re.findall(r'^\s*WriteDataToBufferPointer\(buf, CompileDateAndTime.m_n\w+\);',card,re.M))
load='\n'.join(re.findall(r'^\s*ReadDataFromBufferPointer\(buf, date.m_n\w+\);',card,re.M))
assert len(save.splitlines())==len(load.splitlines())==6
assert re.findall(r'm_n(\w+)',save)==['Second','Minute','Hour','Day','Month','Year']
save=save.replace('CompileDateAndTime.','date->');load=load.replace('date.','date->')
serialization=macros+'''
static int align4bytes(int32_t size)
{
    // Retain the existing save fragment's four-byte cursor alignment
    return (size+3)&0xfffffffc;
}
static uint32_t saveDate(CDate *date,uint8_t *buffer)
{
    // Execute the real date save fields in their original order
    uint8_t *buf=buffer;
'''+save+'''
    return (uint32_t)(buf-buffer);
}
static uint32_t loadDate(CDate *date,uint8_t *buffer)
{
    // Execute the real date load fields into the live record
    uint8_t *buf=buffer;
'''+load+'''
    return (uint32_t)(buf-buffer);
}
'''
template=(root/'utils/tests/date.cpp.in').read_text()
for variant in ['before','after']:
    owner=oldh+'\n'+old.replace('#include "common.h"','').replace('#include "Date.h"','')+'\n'+baseline if variant=='before' else '#include "caller.h"'
    (out/(variant+'.cpp')).write_text(template.replace('@OWNER@',owner).replace('@SERIALIZATION@',serialization),newline='\n')
print('All five original date methods, six int fields, ten comparisons, construction points, 24-byte save order and eight full source/project changes audited')
