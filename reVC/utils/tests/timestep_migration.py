"""Replace the complete time-step data class with initialized C globals."""
import re
import curves_migration as curves

FIELDS={'ms_fTimeScale':'TimeStep_timeScale','ms_fFramesPerUpdate':'TimeStep_framesPerUpdate','ms_fTimeStep':'TimeStep_timeStep'}
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
extern float TimeStep_timeScale;
extern float TimeStep_framesPerUpdate;
extern float TimeStep_timeStep;
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def calls(source):
    # Preserve all lvalue uses and replace each class-scoped field with its C storage
    if 'class CTimeStep' in source:return HEADER
    for name,target in FIELDS.items():source=re.sub(r'CTimeStep::'+name+r'\b',target,source)
    return curves.calls(source)

def kernel(source):
    # Retain the exact field definition order, float types and constant initializers
    return '//+ rouz edit (ChatGPT)\n'+calls(source)+'//- rouz edit (ChatGPT)\n'

def project(source):
    # Rename the complete former C++ unit and request C11 in the Visual Studio project
    if 'TimeStep.cpp"' not in source:return curves.project(source)
    entry='<ClCompile Include="..\\src\\core\\TimeStep.cpp" />'
    if entry in source:
        return curves.project(source.replace(entry,'<ClCompile Include="..\\src\\core\\TimeStep.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>'))
    return curves.project(source.replace('TimeStep.cpp"','TimeStep.c"'))
