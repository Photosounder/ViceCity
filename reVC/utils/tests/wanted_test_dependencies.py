"""Keep historical event dependency observations across the separately tested C wanted boundary."""
FACADE='''
typedef CVector WantedVector;
struct WantedPositionRef {const CVector *plain,*native;};
inline WantedVector Wanted_VectorFromNative(const CVector &value)
{
    // Preserve the historical observed dependency's original by-value coordinates
    return value;
}
inline WantedPositionRef Wanted_ReferenceNative(const CVector &value)
{
    // Keep the actual historical native coordinate reference until the original provider reads it
    WantedPositionRef result={NULL,&value};return result;
}
extern "C" inline int32_t Wanted_GetWantedLevel(CWanted *self)
{
    // Preserve the existing native mock's wanted-level observation
    return self->GetWantedLevel();
}
extern "C" inline int32_t Wanted_WorkOutPolicePresence(WantedVector position,float radius)
{
    // Preserve the existing native mock's police-presence observation and original input values
    return CWanted::WorkOutPolicePresence(position,radius);
}
extern "C" inline void Wanted_RegisterCrime(CWanted *self,eCrimeType type,WantedPositionRef position,uint32_t id,bool dontCare)
{
    // Preserve queued crime observations through the same live historical native coordinate object
    self->RegisterCrime(type,*(position.native?position.native:position.plain),id,dontCare);
}
extern "C" inline void Wanted_RegisterCrime_Immediately(CWanted *self,eCrimeType type,WantedPositionRef position,uint32_t id,bool dontCare)
{
    // Preserve immediate crime observations through the same live historical native coordinate object
    self->RegisterCrime_Immediately(type,*(position.native?position.native:position.plain),id,dontCare);
}
extern "C" inline void Wanted_SetWantedLevelNoDrop(CWanted *self,int32_t level)
{
    // Preserve the historical native wanted escalation receiver and observation
    self->SetWantedLevelNoDrop(level);
}
'''
