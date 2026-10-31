"""Compare actual native parent assignment and original/C static collision model lifetimes."""
from pathlib import Path
import re,shutil
from col_model_consumers import player_fields,player_methods
from col_model_schema import PLAYER_FIELD_SCHEMA
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/col-model-c-tests';probe=out/'lifetime-runtime';probe.mkdir(exist_ok=True);base=out/'runtime'
for name in ['owners.h','entry.h','original-ColModel.h','original.cpp','native.cpp','before-caller.cpp','after-caller.c','cache.cpp']:
    # Reuse only the already verified complete model producers and allocator-boundary declarations
    shutil.copyfile(base/name,probe/name)
parent='#pragma once\n#include "owners.h"\n#include "Lists.h"\ntypedef int32_t int32;typedef uint32_t uint32;\nstruct RwTexture;\n'
for phase in ['before','after']:
    # Keep the complete actual parent declaration with includes supplied by the fixture's opaque SDK boundary
    tree=out/('before' if phase=='before' else 'overlay');source=re.sub(r'^#include[^\n]*\n','',(tree/'src/core/PlayerInfo.h').read_text(),flags=re.M)
    parent+=('#ifdef ORIGINAL\n' if phase=='before' else '#else\n')+source+'\n'
parent+='#endif\n'+re.sub(r'^#include[^\n]*\n','',(out/'before/src/collision/TempColModels.h').read_text(),flags=re.M)
(probe/'parent-owners.h').write_text(parent,newline='\n')
methods=player_methods(PLAYER_FIELD_SCHEMA);current=(out/'overlay/src/core/PlayerInfo.cpp').read_text();assert methods in current
(probe/'parent-methods.cpp').write_text('#include "parent-owners.h"\n#ifndef ORIGINAL\n'+methods+'#endif\n',newline='\n')
statics='#include "parent-owners.h"\n'
for phase in ['before','after']:
    # Compile actual client static storage definitions and cleanup callbacks on each side
    tree=out/('before' if phase=='before' else 'overlay');temp=(tree/'src/collision/TempColModels.cpp').read_text();temp=re.sub(r'^#include[^\n]*\n','',temp[:temp.index('CColSphere s_aPedSpheres')],flags=re.M)
    vehicle=(tree/'src/vehicles/Vehicle.cpp').read_text();start=vehicle.index('static CColModel rotorColModel;' if phase=='before' else 'static void DestroyStaticRotorModel(void);');rotor=vehicle[start:vehicle.index('static CColSphere rotorColSphere;',start)]
    world=(tree/'src/core/World.cpp').read_text();statics+=('#ifdef ORIGINAL\n' if phase=='before' else '#else\n')+temp+rotor
    if phase=='before':
        assert '\tstatic CColModel OurColModel;' in world
        statics+='CColModel *TestWorldModel(void)\n{\n    // Invoke the original function-local construction and lazy exit registration\n    static CColModel OurColModel;return &OurColModel;\n}\n'
    else:
        start=world.index('static CColModel worldSphereColModel;');scope=world[start:world.index('CEntity *\nCWorld::TestSphereAgainstSectorList(',start)];initialise='\tstatic const bool modelInitialised=(ColModel_InitStatic(&worldSphereColModel,DestroyStaticWorldSphereModel),true);\n\t(void)modelInitialised;';assert initialise in world
        statics+=scope+'CColModel *TestWorldModel(void)\n{\n    // Invoke the actual lazy C model initialization and native storage-address cleanup callback\n'+initialise+'\n    return &worldSphereColModel;\n}\n'
statics+='#endif\nCColModel *TestRotorModel(void)\n{\n    // Expose the actual client static storage for initialization and destructor-order observations\n    return &rotorColModel;\n}\n'
(probe/'static-models.cpp').write_text(statics,newline='\n')
fields=player_fields(PLAYER_FIELD_SCHEMA);helpers='static void FillParent(CPlayerInfo *self,unsigned seed)\n{\n    // Populate each actual native field with deterministic valid values before invoking parent assignment\n'
for index,(kind,name,count) in enumerate(fields):
    # Retain the original conditional parent field set in the generated input and observation code
    if kind=='directive':helpers+=name+'\n'
    elif name=='m_ColModel':helpers+='    CColModel *value=create(seed);invoke(8,&self->m_ColModel,value);invoke(1,value);free(value);\n'
    elif count:helpers+='    memset(self->'+name+',(int)(33+(seed+'+str(index)+')%80),sizeof(self->'+name+'));\n'
    elif kind.endswith('*'):helpers+='    self->'+name+'=('+kind+')(uintptr_t)(4096+seed*64+'+str(index)+');\n'
    elif kind=='bool':helpers+='    self->'+name+'=((seed+'+str(index)+')&1)!=0;\n'
    elif kind=='float':helpers+='    self->'+name+'=(float)((seed+'+str(index)+')%1000)*0.25f;\n'
    else:helpers+='    self->'+name+'=('+kind+')((seed+'+str(index)+')%100);\n'
helpers+='}\nstatic void CheckParent(CPlayerInfo *self,const CPlayerInfo &other)\n{\n    // Compare every actual parent field while the complete model fixture separately observes owned geometry\n'
for kind,name,count in fields:
    if kind=='directive':helpers+=name+'\n'
    elif name=='m_ColModel':helpers+='    observe(&self->m_ColModel);require(self->m_ColModel.spheres==NULL||self->m_ColModel.spheres!=other.m_ColModel.spheres);require(self->m_ColModel.vertices==NULL||self->m_ColModel.vertices!=other.m_ColModel.vertices);\n'
    elif count:helpers+='    require(memcmp(self->'+name+',other.'+name+',sizeof(self->'+name+'))==0);hashBytes(self->'+name+',sizeof(self->'+name+'));\n'
    elif kind.endswith('*'):helpers+='    require(self->'+name+'==other.'+name+');uint32_t '+name+'Bits=(uint32_t)(uintptr_t)self->'+name+';hashBytes(&'+name+'Bits,4);\n'
    else:helpers+='    require(self->'+name+'==other.'+name+');hashBytes(&self->'+name+',sizeof(self->'+name+'));\n'
helpers+='}\n'
baseFixture=(root/'utils/tests/col_model_fixture.cpp.in').read_text();baseFixture=baseFixture.replace('#include "owners.h"','#include "parent-owners.h"\nstatic void ParentObserveBoundary(void);\nstatic void StaticObserveFree(unsigned id);',1).replace('int main(void)','int ColModelBaseMain(void)',1).replace('uint32_t values[]={operation,id,(uint32_t)size};','ParentObserveBoundary();uint32_t values[]={operation,id,(uint32_t)size};',1).replace('event(2,blocks[i].id,blocks[i].size);','StaticObserveFree(blocks[i].id);event(2,blocks[i].id,blocks[i].size);',1)
extra=(root/'utils/tests/col_model_lifetime.cpp.in').read_text();(probe/'fixture.cpp').write_text(baseFixture+'\n'+helpers+'\n'+extra,newline='\n')
print('Prepared complete parent assignment and twenty actual static model lifetimes with an early exit observer')
