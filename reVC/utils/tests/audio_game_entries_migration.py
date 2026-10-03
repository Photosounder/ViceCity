"""Known crime entry and player vehicle C migrations."""
import re
import audio_reload_migration as reload
import audio_requests_migration as requests
import audio_distance_migration as distance
from audio_controls_migration import function
HOST='//+ rouz edit (ChatGPT)\nCVehicle *\nAudioPoliceHost_PlayerVehicle(void)\n{\n\t// Preserve the original player vehicle lookup before the separate player lookup\n\treturn FindPlayerVehicle();\n}\n\nCEntity *\nAudioPoliceHost_AttachedTo(const CPlayerPed *player)\n{\n\t// Read the existing attachment only when no player vehicle was found\n\treturn player->m_attachedTo;\n}\n\nuint8_t\nAudioPoliceHost_IsVehicle(CEntity *entity)\n{\n\t// Preserve the original game entity type test before downcasting\n\treturn entity->IsVehicle();\n}\n\nCVehicle *\nAudioPoliceHost_AsVehicle(CEntity *entity)\n{\n\t// Retain the original C++ base-to-derived conversion for the game object layout\n\treturn (CVehicle*)entity;\n}\n\nvoid\nAudioPoliceHost_CrimePosition(const CVector *position, AudioCrimePosition *plain)\n{\n\t// Read the game coordinates only after all original crime eligibility checks pass\n\tplain->x = position->x;\n\tplain->y = position->y;\n\tplain->z = position->z;\n}\n//- rouz edit (ChatGPT)\n'
PROTOTYPES='void AudioPolice_ReportCrime(cAudioManager *manager, enum eCrimeType type, const CVector *position);\n'
GAME_PROTOTYPES='CVehicle *AudioGame_FindVehicle(void);\nCVehicle *AudioPoliceHost_PlayerVehicle(void);\nCEntity *AudioPoliceHost_AttachedTo(const CPlayerPed *player);\nuint8_t AudioPoliceHost_IsVehicle(CEntity *entity);\nCVehicle *AudioPoliceHost_AsVehicle(CEntity *entity);\nvoid AudioPoliceHost_CrimePosition(const CVector *position, AudioCrimePosition *plain);\n'

def calls(source):
    # Replace every original vehicle lookup without introducing owner state reads
    source=source.replace('AudioManager.FindVehicleOfPlayer()', 'AudioGame_FindVehicle()')
    return distance.logic(re.sub(r'(?<![\w.:>])FindVehicleOfPlayer\(\)', 'AudioGame_FindVehicle()',source))

def logic(source):
    # Remove the original state-independent lookup member before updating its callers
    if 'cAudioManager::FindVehicleOfPlayer(' in source:source=source.replace(function(source,'cAudioManager::','FindVehicleOfPlayer')+'\n','')
    return calls(source)

def remove_find_bridge(source):
    # Remove the old wrapper now that the lookup itself is C
    return source.replace(function(source,'AudioPoliceHost_','FindVehicle')+'\n','')

def radio(source):
    # Remove the final crime owner member and add only the required game field adapters
    source=source.replace(function(source,'cAudioManager::','ReportCrime')+'\n','')
    return requests.radio(remove_find_bridge(source)+HOST)

def header(source):
    # Preserve every owner field and expose the actual C crime operation
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:FindVehicleOfPlayer|ReportCrime)\(',line))
    source=source.replace('#include "PolRadio.h"','#include "PolRadio.h"\n#include "AudioPoliceGame.h"')
    return distance.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def game_header(source):
    # Expose opaque game entities and vectors without assuming their C++ layouts
    source=source.replace('class CVehicle;', 'class CVehicle;\nclass CEntity;\nclass CVector;').replace('typedef struct CVehicle CVehicle;', 'typedef struct CVehicle CVehicle;\ntypedef struct CEntity CEntity;\ntypedef struct CVector CVector;')
    source=source.replace('CVehicle *AudioPoliceHost_FindVehicle(cAudioManager *manager);\n',GAME_PROTOTYPES)
    return source

def facade(source):
    # Move the public crime facade into C with the same opaque vector pointer ABI
    if re.search(r'(?m)^DMAudio_ReportCrime\(',source):source=source.replace(function(source,'DMAudio_','ReportCrime')+'\n','')
    return reload.facade(source)

def suspect(source):
    # Use the owner-independent lookup at the same existing report point
    return source.replace('AudioPoliceHost_FindVehicle(manager)', 'AudioGame_FindVehicle()')

def vehicle_body(source):
    # Preserve both lookups, attachment gates and the original game-aware downcast
    source=source[source.index('{'):].replace('nil','NULL')
    source=source.replace('FindPlayerVehicle()', 'AudioPoliceHost_PlayerVehicle()').replace('FindPlayerPed()', 'AudioPoliceHost_FindPlayer()').replace('ped->m_attachedTo', 'AudioPoliceHost_AttachedTo(ped)').replace('attachedTo->IsVehicle()', 'AudioPoliceHost_IsVehicle(attachedTo)').replace('(CVehicle*)attachedTo', 'AudioPoliceHost_AsVehicle(attachedTo)')
    return source

def crime_body(source):
    # Preserve short-circuit order, the original OR gate, cooldown and coordinate read timing
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    source=source.replace('MusicManager.m_nMusicMode','AudioPoliceHost_MusicMode()').replace('FindPlayerPed()->m_pWanted->GetWantedLevel()', 'AudioPoliceHost_WantedLevel(AudioPoliceHost_FindPlayer())')
    source=source.replace('AudioCrimes_Report(', 'AudioCrimePosition plain;\n\t\tAudioPoliceHost_CrimePosition(position, &plain);\n\t\tAudioCrimes_Report(').replace('pos.x, pos.y, pos.z','plain.x, plain.y, plain.z')
    return source
