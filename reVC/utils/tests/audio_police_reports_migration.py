"""Known police scheduler and zone report C migrations."""
import re
import audio_game_entries_migration as entries
import audio_police_suspect_migration as suspect
from audio_controls_migration import function
HOST='//+ rouz edit (ChatGPT)\nuint8_t\nAudioPoliceHost_MusicMode(void)\n{\n\t// Read the existing music-mode gate at the report decision point\n\treturn MusicManager.m_nMusicMode;\n}\n\nCPlayerPed *\nAudioPoliceHost_FindPlayer(void)\n{\n\t// Preserve each individual player lookup and its short-circuit position\n\treturn FindPlayerPed();\n}\n\nuint8_t\nAudioPoliceHost_HasWanted(const CPlayerPed *player)\n{\n\t// Read the original wanted pointer without changing the player lookup sequence\n\treturn player->m_pWanted != NULL;\n}\n\nint32_t\nAudioPoliceHost_WantedLevel(const CPlayerPed *player)\n{\n\t// Read the existing wanted level after the original player gate\n\treturn player->m_pWanted->GetWantedLevel();\n}\n\n#ifdef FIX_BUGS\nuint8_t\nAudioPoliceHost_IsReplay(void)\n{\n\t// Retain the original replay gate and short-circuit order\n\treturn CReplay::IsPlayingBack();\n}\n#endif\n\nint16_t\nAudioPoliceHost_FindZone(const AudioCrimePosition *position)\n{\n\t// Supply the same three coordinates to the existing audio-zone lookup\n\tCVector gamePosition(position->x, position->y, position->z);\n\treturn CTheZones::FindAudioZone(&gamePosition);\n}\n\nvoid\nAudioPoliceHost_GetZone(int16_t id, AudioPoliceZoneView *view)\n{\n\t// Expose pointers to the existing zone fields without copying their values\n\tconst CZone *zone = CTheZones::GetAudioZone(id);\n\tview->name = zone->name;\n\tview->minx = &zone->minx;\n\tview->miny = &zone->miny;\n\tview->maxx = &zone->maxx;\n\tview->maxy = &zone->maxy;\n}\n\nvoid\nAudioPoliceHost_SuspectReport(cAudioManager *manager)\n{\n\t// Build the existing vehicle or foot report at the scheduled report point\n\tmanager->SetupSuspectLastSeenReport();\n}\n//- rouz edit (ChatGPT)\n'
PROTOTYPES='void AudioPolice_Service(cAudioManager *manager);\nuint8_t AudioPolice_SetupCrimeReport(cAudioManager *manager);\nvoid AudioPolice_PlaySuspect(cAudioManager *manager, float x, float y, float z);\nvoid AudioPoliceHost_SuspectReport(cAudioManager *manager);\n'

METHODS={'ServicePoliceRadio':'Service','SetupCrimeReport':'SetupCrimeReport','PlaySuspectLastSeen':'PlaySuspect'}
def calls(source):
    # Preserve owner identity in service and public coordinate-report callers
    source=source.replace('ServicePoliceRadio();','AudioPolice_Service(this);').replace('AudioManager.PlaySuspectLastSeen(', 'AudioPolice_PlaySuspect(&AudioManager, ')
    return entries.calls(source)

def radio(source):
    # Remove only the three migrated methods and add game field access boundaries
    for name in METHODS:source=source.replace(function(source,'cAudioManager::',name)+'\n','')
    return suspect.radio(source.replace('#include "PolRadio.h"','#include "PolRadio.h"\n#include "AudioPoliceGame.h"')+HOST)

def header(source):
    # Retain the owner record and expose the actual C report operations
    source='\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+name+r'\(',line) for name in METHODS))
    return suspect.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def facade(source):
    # Move the public coordinate-based report facade into actual C
    return entries.facade(source.replace(function(source,'DMAudio_','PlaySuspectLastSeen')+'\n',''))

def body(source):
    # Keep each lookup, branch, persistent counter, zone read and queue write in order
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    for a,b in [('int32','int32_t'),('int16','int16_t'),('uint32','uint32_t'),('bool8','uint8_t')]:source=re.sub(r'\b'+a+r'\b',b,source)
    source=source.replace('FALSE','0').replace('TRUE','1').replace('AudioPolice_ServiceChannel(this,','AudioPolice_ServiceChannel(manager,')
    source=source.replace('SetupCrimeReport()', 'AudioPolice_SetupCrimeReport(manager)').replace('SetupSuspectLastSeenReport()', 'AudioPoliceHost_SuspectReport(manager)')
    source=source.replace('CTimer::GetLogicalFramesPassed()', 'AudioPoliceHost_LogicalFramesPassed()').replace('MusicManager.m_nMusicMode','AudioPoliceHost_MusicMode()')
    source=source.replace('CReplay::IsPlayingBack()', 'AudioPoliceHost_IsReplay()').replace('!FindPlayerPed()->m_pWanted','!AudioPoliceHost_HasWanted(AudioPoliceHost_FindPlayer())').replace('FindPlayerPed()', 'AudioPoliceHost_FindPlayer()').replace('playerPed->m_pWanted->GetWantedLevel()', 'AudioPoliceHost_WantedLevel(playerPed)')
    source=source.replace('CZone *zone;', 'AudioPoliceZoneView zone;').replace('CVector crimePosition(manager->m_aCrimes[i].position.x, manager->m_aCrimes[i].position.y, manager->m_aCrimes[i].position.z);','AudioCrimePosition crimePosition = manager->m_aCrimes[i].position;')
    source=source.replace('CVector vec = CVector(x, y, z);', 'AudioCrimePosition vec = { x, y, z };')
    source=source.replace('CTheZones::FindAudioZone(', 'AudioPoliceHost_FindZone(').replace('zone = CTheZones::GetAudioZone(audioZoneId);','AudioPoliceHost_GetZone(audioZoneId, &zone);').replace('zone = CTheZones::GetAudioZone(audioZone);','AudioPoliceHost_GetZone(audioZone, &zone);')
    source=source.replace('zone->name','zone.name')
    for name in ('maxx','maxy','minx','miny'):source=source.replace('zone->'+name,'*zone.'+name)
    return suspect.calls(source)
