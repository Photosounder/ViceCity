#include "common.h"

#include "AutoPilot.h"

#include "CarCtrl.h"
#include "Curves.h"
#include "PathFind.h"
#include "SaveBuf.h"

void CAutoPilot::ModifySpeed(float speed)
{
	m_fMaxTrafficSpeed = Max(0.01f, speed);
	float positionBetweenNodes = (float)(CTimer::GetTimeInMilliseconds() - m_nTimeEnteredCurve) / m_nTimeToSpendOnCurrentCurve;
	CCarPathLink* pCurrentLink = &ThePaths.m_carPathLinks[m_nCurrentPathNodeInfo];
	CCarPathLink* pNextLink = &ThePaths.m_carPathLinks[m_nNextPathNodeInfo];
	float currentPathLinkForwardX = m_nCurrentDirection * ThePaths.m_carPathLinks[m_nCurrentPathNodeInfo].GetDirX();
	float currentPathLinkForwardY = m_nCurrentDirection * ThePaths.m_carPathLinks[m_nCurrentPathNodeInfo].GetDirY();
	float nextPathLinkForwardX = m_nNextDirection * ThePaths.m_carPathLinks[m_nNextPathNodeInfo].GetDirX();
	float nextPathLinkForwardY = m_nNextDirection * ThePaths.m_carPathLinks[m_nNextPathNodeInfo].GetDirY();
	CVector positionOnCurrentLinkIncludingLane(
		pCurrentLink->GetX() + ((m_nCurrentLane + 0.5f) * LANE_WIDTH) * currentPathLinkForwardY,
		pCurrentLink->GetY() - ((m_nCurrentLane + 0.5f) * LANE_WIDTH) * currentPathLinkForwardX,
		0.0f);
	CVector positionOnNextLinkIncludingLane(
		pNextLink->GetX() + ((m_nNextLane + 0.5f) * LANE_WIDTH) * nextPathLinkForwardY,
		pNextLink->GetY() - ((m_nNextLane + 0.5f) * LANE_WIDTH) * nextPathLinkForwardX,
		0.0f);
	m_nTimeToSpendOnCurrentCurve = CCurves::CalcSpeedScaleFactor(
		&positionOnCurrentLinkIncludingLane,
		&positionOnNextLinkIncludingLane,
		currentPathLinkForwardX, currentPathLinkForwardY,
		nextPathLinkForwardX, nextPathLinkForwardY
	) * (1000.0f / m_fMaxTrafficSpeed);
#ifdef FIX_BUGS
	/* Casting timer to float is very unwanted, and in this case even causes crashes. */
	m_nTimeEnteredCurve = CTimer::GetTimeInMilliseconds() -
		(uint32)(positionBetweenNodes * m_nTimeToSpendOnCurrentCurve);
#else
	m_nTimeEnteredCurve = CTimer::GetTimeInMilliseconds() - positionBetweenNodes * m_nTimeToSpendOnCurrentCurve;
#endif
}

void CAutoPilot::RemoveOnePathNode()
{
	--m_nPathFindNodesCount;
	for (int i = 0; i < m_nPathFindNodesCount; i++)
		m_aPathFindNodesInfo[i] = m_aPathFindNodesInfo[i + 1];
}

#ifdef COMPATIBLE_SAVES
void CAutoPilot::Save(uint8*& buf)
{
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBuf(&buf, &m_nCurrentRouteNode, sizeof(m_nCurrentRouteNode));
	WriteSaveBuf(&buf, &m_nNextRouteNode, sizeof(m_nNextRouteNode));
	WriteSaveBuf(&buf, &m_nPrevRouteNode, sizeof(m_nPrevRouteNode));
	WriteSaveBuf(&buf, &m_nTimeEnteredCurve, sizeof(m_nTimeEnteredCurve));
	WriteSaveBuf(&buf, &m_nTimeToSpendOnCurrentCurve, sizeof(m_nTimeToSpendOnCurrentCurve));
	WriteSaveBuf(&buf, &m_nCurrentPathNodeInfo, sizeof(m_nCurrentPathNodeInfo));
	WriteSaveBuf(&buf, &m_nNextPathNodeInfo, sizeof(m_nNextPathNodeInfo));
	WriteSaveBuf(&buf, &m_nPreviousPathNodeInfo, sizeof(m_nPreviousPathNodeInfo));
	WriteSaveBuf(&buf, &m_nAntiReverseTimer, sizeof(m_nAntiReverseTimer));
	WriteSaveBuf(&buf, &m_nTimeToStartMission, sizeof(m_nTimeToStartMission));
	WriteSaveBuf(&buf, &m_nPreviousDirection, sizeof(m_nPreviousDirection));
	WriteSaveBuf(&buf, &m_nCurrentDirection, sizeof(m_nCurrentDirection));
	WriteSaveBuf(&buf, &m_nNextDirection, sizeof(m_nNextDirection));
	WriteSaveBuf(&buf, &m_nCurrentLane, sizeof(m_nCurrentLane));
	WriteSaveBuf(&buf, &m_nNextLane, sizeof(m_nNextLane));
	WriteSaveBuf(&buf, &m_nDrivingStyle, sizeof(m_nDrivingStyle));
	WriteSaveBuf(&buf, &m_nCarMission, sizeof(m_nCarMission));
	WriteSaveBuf(&buf, &m_nTempAction, sizeof(m_nTempAction));
	WriteSaveBuf(&buf, &m_nTimeTempAction, sizeof(m_nTimeTempAction));
	WriteSaveBuf(&buf, &m_fMaxTrafficSpeed, sizeof(m_fMaxTrafficSpeed));
	WriteSaveBuf(&buf, &m_nCruiseSpeed, sizeof(m_nCruiseSpeed));
	WriteSaveBuf(&buf, &m_nCruiseSpeedMultiplierType, sizeof(m_nCruiseSpeedMultiplierType));
	ZeroSaveBuf(&buf, 2);
	WriteSaveBuf(&buf, &m_fCruiseSpeedMultiplier, sizeof(m_fCruiseSpeedMultiplier));
	//- rouz edit (ChatGPT)
	uint8 flags = 0;
	if (m_bSlowedDownBecauseOfCars) flags |= BIT(0);
	if (m_bSlowedDownBecauseOfPeds) flags |= BIT(1);
	if (m_bStayInCurrentLevel) flags |= BIT(2);
	if (m_bStayInFastLane) flags |= BIT(3);
	if (m_bIgnorePathfinding) flags |= BIT(4);
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBuf(&buf, &flags, sizeof(flags));
	WriteSaveBuf(&buf, &m_nSwitchDistance, sizeof(m_nSwitchDistance));
	ZeroSaveBuf(&buf, 2);
	WriteSaveBuf(&buf, &m_vecDestinationCoors.x, sizeof(m_vecDestinationCoors.x));
	WriteSaveBuf(&buf, &m_vecDestinationCoors.y, sizeof(m_vecDestinationCoors.y));
	WriteSaveBuf(&buf, &m_vecDestinationCoors.z, sizeof(m_vecDestinationCoors.z));
	ZeroSaveBuf(&buf, 32);
	WriteSaveBuf(&buf, &m_nPathFindNodesCount, sizeof(m_nPathFindNodesCount));
	ZeroSaveBuf(&buf, 6);
	//- rouz edit (ChatGPT)
}

void CAutoPilot::Load(uint8*& buf)
{
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&m_nCurrentRouteNode, &buf, sizeof(m_nCurrentRouteNode));
	ReadSaveBuf(&m_nNextRouteNode, &buf, sizeof(m_nNextRouteNode));
	ReadSaveBuf(&m_nPrevRouteNode, &buf, sizeof(m_nPrevRouteNode));
	ReadSaveBuf(&m_nTimeEnteredCurve, &buf, sizeof(m_nTimeEnteredCurve));
	ReadSaveBuf(&m_nTimeToSpendOnCurrentCurve, &buf, sizeof(m_nTimeToSpendOnCurrentCurve));
	ReadSaveBuf(&m_nCurrentPathNodeInfo, &buf, sizeof(m_nCurrentPathNodeInfo));
	ReadSaveBuf(&m_nNextPathNodeInfo, &buf, sizeof(m_nNextPathNodeInfo));
	ReadSaveBuf(&m_nPreviousPathNodeInfo, &buf, sizeof(m_nPreviousPathNodeInfo));
	ReadSaveBuf(&m_nAntiReverseTimer, &buf, sizeof(m_nAntiReverseTimer));
	ReadSaveBuf(&m_nTimeToStartMission, &buf, sizeof(m_nTimeToStartMission));
	ReadSaveBuf(&m_nPreviousDirection, &buf, sizeof(m_nPreviousDirection));
	ReadSaveBuf(&m_nCurrentDirection, &buf, sizeof(m_nCurrentDirection));
	ReadSaveBuf(&m_nNextDirection, &buf, sizeof(m_nNextDirection));
	ReadSaveBuf(&m_nCurrentLane, &buf, sizeof(m_nCurrentLane));
	ReadSaveBuf(&m_nNextLane, &buf, sizeof(m_nNextLane));
	ReadSaveBuf(&m_nDrivingStyle, &buf, sizeof(m_nDrivingStyle));
	ReadSaveBuf(&m_nCarMission, &buf, sizeof(m_nCarMission));
	ReadSaveBuf(&m_nTempAction, &buf, sizeof(m_nTempAction));
	ReadSaveBuf(&m_nTimeTempAction, &buf, sizeof(m_nTimeTempAction));
	ReadSaveBuf(&m_fMaxTrafficSpeed, &buf, sizeof(m_fMaxTrafficSpeed));
	ReadSaveBuf(&m_nCruiseSpeed, &buf, sizeof(m_nCruiseSpeed));
	ReadSaveBuf(&m_nCruiseSpeedMultiplierType, &buf, sizeof(m_nCruiseSpeedMultiplierType));
	SkipSaveBuf(&buf, 2);
	ReadSaveBuf(&m_fCruiseSpeedMultiplier, &buf, sizeof(m_fCruiseSpeedMultiplier));
	//- rouz edit (ChatGPT)
	uint8 flags;
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&flags, &buf, sizeof(flags));
	//- rouz edit (ChatGPT)
	m_bSlowedDownBecauseOfCars = !!(flags & BIT(0));
	m_bSlowedDownBecauseOfPeds = !!(flags & BIT(1));
	m_bStayInCurrentLevel = !!(flags & BIT(2));
	m_bStayInFastLane = !!(flags & BIT(3));
	m_bIgnorePathfinding = !!(flags & BIT(4));
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&m_nSwitchDistance, &buf, sizeof(m_nSwitchDistance));
	SkipSaveBuf(&buf, 2);
	ReadSaveBuf(&m_vecDestinationCoors.x, &buf, sizeof(m_vecDestinationCoors.x));
	ReadSaveBuf(&m_vecDestinationCoors.y, &buf, sizeof(m_vecDestinationCoors.y));
	ReadSaveBuf(&m_vecDestinationCoors.z, &buf, sizeof(m_vecDestinationCoors.z));
	SkipSaveBuf(&buf, 32);
	ReadSaveBuf(&m_nPathFindNodesCount, &buf, sizeof(m_nPathFindNodesCount));
	SkipSaveBuf(&buf, 6);
	//- rouz edit (ChatGPT)
}
#endif
