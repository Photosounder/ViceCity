"""Known collision-report gates, midpoint and public C facade migration."""
import re
import audio_collision_sounds_migration as sounds
from audio_controls_migration import function
HOST='''//+ rouz edit (ChatGPT)
uint8_t
AudioCollisionReportHost_IsBuilding(CEntity *entity)
{
    // Query each original building test at its short-circuit point
    return entity->IsBuilding();
}

void
AudioCollisionReportHost_Position(CEntity *entity, AudioSoundPosition *position)
{
    // Copy the current game position into plain C coordinates
    *position = AudioSoundPosition_FromVector(entity->GetPosition());
}
//- rouz edit (ChatGPT)
'''
PROTOTYPES='void AudioCollisionReport_Report(cAudioManager *manager, CEntity *entity1, CEntity *entity2, uint8_t surface1, uint8_t surface2, float collisionPower, float velocity);\nuint8_t AudioCollisionReportHost_IsBuilding(CEntity *entity);\nvoid AudioCollisionReportHost_Position(CEntity *entity, AudioSoundPosition *position);\n'
def extract(source):
    # Accept the original multiline parameter list before finding the complete method body
    start=source.index('void\ncAudioManager::ReportCollision(')
    opening=source.index('{',start);depth=1;end=opening+1
    while depth:
        depth += (source[end]=='{')-(source[end]=='}');end+=1
    return source[start:end]

def calls(source):
    # Route the ped impact report through the explicit C owner operation
    return sounds.calls(source.replace('ReportCollision(params.m_pPed,', 'AudioCollisionReport_Report(this, params.m_pPed,'))

def owner(source):
    # Remove the business member while retaining only game field access
    if 'cAudioManager::ReportCollision(' in source:
        source=source.replace(extract(source)+'\n','')+HOST
    return sounds.owner(source)

def header(source):
    # Expose the C collision report operation without changing owner storage
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\bReportCollision\(',line))
    return sounds.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def facade(source):
    # Move the existing public C-linkage facade into its actual C translation unit
    if re.search(r'(?m)^DMAudio_ReportCollision\(',source):
        source=source.replace(function(source,'DMAudio_','ReportCollision')+'\n','')
    return source

def body(source):
    # Keep rejection order, chained position-copy order and the original float operation sequence
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    source=source.replace('CVector v1;', 'AudioSoundPosition v1;').replace('CVector v2;', 'AudioSoundPosition v2;')
    source=source.replace('entity1->IsBuilding()', 'AudioCollisionReportHost_IsBuilding(entity1)').replace('entity2->IsBuilding()', 'AudioCollisionReportHost_IsBuilding(entity2)')
    source=source.replace('v1 = v2 = entity2->GetPosition();','AudioCollisionReportHost_Position(entity2, &v2);\n        v1 = v2;').replace('v1 = v2 = entity1->GetPosition();','AudioCollisionReportHost_Position(entity1, &v2);\n        v1 = v2;')
    source=source.replace('v1 = entity1->GetPosition();', 'AudioCollisionReportHost_Position(entity1, &v1);').replace('v2 = entity2->GetPosition();', 'AudioCollisionReportHost_Position(entity2, &v2);')
    source=source.replace('CVector pos = (v1 + v2) * 0.5f;', 'const AudioSoundPosition sum = {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};\n    const AudioSoundPosition pos = {sum.x * 0.5f, sum.y * 0.5f, sum.z * 0.5f};')
    source=source.replace('AudioGeometry_DistanceVector(pos)', 'AudioGeometry_DistanceSquared(&pos)').replace('SQR(COLLISION_MAX_DIST)', '(COLLISION_MAX_DIST * COLLISION_MAX_DIST)')
    return source
