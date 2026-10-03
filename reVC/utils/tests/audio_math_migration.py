"""Known scalar-call substitutions for audits of earlier audio conversions."""
import re
import audio_distance_migration as distance

def calls(source):
    # Audit only the diagnosed scalar and explicit-state substitutions at game callers
    source = source.replace('AudioManager.ComputePan(55.0f, &panVec)','AudioMath_ComputePan(55.0f, panVec.x)').replace('AudioManager.RandomDisplacement(', 'AudioMath_RandomDisplacement(AudioManager.m_anRandomTable, ')
    source = re.sub(r'\bComputePan\(([^\n]*?), &(\w+)\)',r'AudioMath_ComputePan(\1, \2.x)',source)
    source = source.replace('ComputeDopplerEffectedFrequency(', 'AudioMath_ComputeDopplerFrequency(TheCamera.Get_Just_Switched_Status(), m_nTimeSpent, m_fSpeedOfSound, ')
    source = re.sub(r'(?<![.\w])RandomDisplacement\(', 'AudioMath_RandomDisplacement(m_anRandomTable, ',source)
    source = re.sub(r'\bComputeVolume\(', 'AudioMath_ComputeVolume(',source)
    return distance.logic(re.sub(r'\bComputeEmittingVolume\(', 'AudioMath_ComputeEmittingVolume(',source))

