//+ rouz edit (ChatGPT)
#include "AutoPilot_backend.h"
#include "Curves.h"
#include "Timer.h"
#include "SaveBuf.h"
#define LANE_WIDTH 5.0f
#define Max(a,b) ((a)>(b)?(a):(b))
#define BIT(bit) (1u<<(bit))
void AutoPilot_Init(CAutoPilot *self)
{
    // Preserve the complete original Init operation and all intentionally unwritten fields
		self->m_nPrevRouteNode = 0;
		self->m_nNextRouteNode = self->m_nPrevRouteNode;
		self->m_nCurrentRouteNode = self->m_nNextRouteNode;
		self->m_nTimeEnteredCurve = 0;
		self->m_nTimeToSpendOnCurrentCurve = 1000;
		self->m_nPreviousPathNodeInfo = 0;
		self->m_nNextPathNodeInfo = self->m_nPreviousPathNodeInfo;
		self->m_nCurrentPathNodeInfo = self->m_nNextPathNodeInfo;
		self->m_nNextDirection = 1;
		self->m_nCurrentDirection = self->m_nNextDirection;
		self->m_nCurrentLane = self->m_nNextLane = 0;
		self->m_nDrivingStyle = DRIVINGSTYLE_STOP_FOR_CARS;
		self->m_nCarMission = MISSION_NONE;
		self->m_nTempAction = TEMPACT_NONE;
		self->m_nCruiseSpeed = 10;
		self->m_fMaxTrafficSpeed = 10.0f;
		self->m_bSlowedDownBecauseOfPeds = false;
		self->m_bSlowedDownBecauseOfCars = false;
		self->m_nPathFindNodesCount = 0;
		self->m_pTargetCar = 0;
		self->m_nTimeToStartMission = (*Timer_GetTimeInMilliseconds());
		self->m_nAntiReverseTimer = self->m_nTimeToStartMission;
		self->m_bStayInFastLane = false;
		self->m_nCruiseSpeedMultiplierType = 0;
		self->m_fCruiseSpeedMultiplier = 1.0f;
	}

void AutoPilot_ModifySpeed(CAutoPilot *self,float speed)
{
    // Preserve the complete original ModifySpeed operation and all intentionally unwritten fields
	self->m_fMaxTrafficSpeed = Max(0.01f, speed);
	float positionBetweenNodes = (float)((*Timer_GetTimeInMilliseconds()) - self->m_nTimeEnteredCurve) / self->m_nTimeToSpendOnCurrentCurve;
	CCarPathLink* pCurrentLink = AutoPilot_BackendLink(self->m_nCurrentPathNodeInfo);
	CCarPathLink* pNextLink = AutoPilot_BackendLink(self->m_nNextPathNodeInfo);
	float currentPathLinkForwardX = self->m_nCurrentDirection * AutoPilot_BackendDirX(AutoPilot_BackendLink(self->m_nCurrentPathNodeInfo));
	float currentPathLinkForwardY = self->m_nCurrentDirection * AutoPilot_BackendDirY(AutoPilot_BackendLink(self->m_nCurrentPathNodeInfo));
	float nextPathLinkForwardX = self->m_nNextDirection * AutoPilot_BackendDirX(AutoPilot_BackendLink(self->m_nNextPathNodeInfo));
	float nextPathLinkForwardY = self->m_nNextDirection * AutoPilot_BackendDirY(AutoPilot_BackendLink(self->m_nNextPathNodeInfo));
	CurvesVector positionOnCurrentLinkIncludingLane={
		AutoPilot_BackendX(pCurrentLink) + ((self->m_nCurrentLane + 0.5f) * LANE_WIDTH) * currentPathLinkForwardY,
		AutoPilot_BackendY(pCurrentLink) - ((self->m_nCurrentLane + 0.5f) * LANE_WIDTH) * currentPathLinkForwardX,
		0.0f};
	CurvesVector positionOnNextLinkIncludingLane={
		AutoPilot_BackendX(pNextLink) + ((self->m_nNextLane + 0.5f) * LANE_WIDTH) * nextPathLinkForwardY,
		AutoPilot_BackendY(pNextLink) - ((self->m_nNextLane + 0.5f) * LANE_WIDTH) * nextPathLinkForwardX,
		0.0f};
	self->m_nTimeToSpendOnCurrentCurve = Curves_CalcSpeedScaleFactor(
		positionOnCurrentLinkIncludingLane,
		positionOnNextLinkIncludingLane,
		currentPathLinkForwardX,
		currentPathLinkForwardY,
		nextPathLinkForwardX,
		nextPathLinkForwardY
	) * (1000.0f / self->m_fMaxTrafficSpeed);
#ifdef FIX_BUGS
	/* Casting timer to float is very unwanted, and in this case even causes crashes. */
	self->m_nTimeEnteredCurve = (*Timer_GetTimeInMilliseconds()) -
		(uint32_t)(positionBetweenNodes * self->m_nTimeToSpendOnCurrentCurve);
#else
	self->m_nTimeEnteredCurve = (*Timer_GetTimeInMilliseconds()) - positionBetweenNodes * self->m_nTimeToSpendOnCurrentCurve;
#endif
}

void AutoPilot_RemoveOnePathNode(CAutoPilot *self)
{
    // Preserve the complete original RemoveOnePathNode operation and all intentionally unwritten fields
	--self->m_nPathFindNodesCount;
	for (int i = 0; i < self->m_nPathFindNodesCount; i++)
		self->m_aPathFindNodesInfo[i] = self->m_aPathFindNodesInfo[i + 1];
}

float AutoPilot_GetCruiseSpeed(const CAutoPilot *self)
{
    // Preserve the complete original GetCruiseSpeed operation and all intentionally unwritten fields
 return self->m_nCruiseSpeed * self->m_fCruiseSpeedMultiplier; 
}

#ifdef COMPATIBLE_SAVES
void AutoPilot_Save(CAutoPilot *self,uint8_t **buf)
{
    // Preserve the complete original Save operation and all intentionally unwritten fields
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBuf(buf, &self->m_nCurrentRouteNode, sizeof(self->m_nCurrentRouteNode));
	WriteSaveBuf(buf, &self->m_nNextRouteNode, sizeof(self->m_nNextRouteNode));
	WriteSaveBuf(buf, &self->m_nPrevRouteNode, sizeof(self->m_nPrevRouteNode));
	WriteSaveBuf(buf, &self->m_nTimeEnteredCurve, sizeof(self->m_nTimeEnteredCurve));
	WriteSaveBuf(buf, &self->m_nTimeToSpendOnCurrentCurve, sizeof(self->m_nTimeToSpendOnCurrentCurve));
	WriteSaveBuf(buf, &self->m_nCurrentPathNodeInfo, sizeof(self->m_nCurrentPathNodeInfo));
	WriteSaveBuf(buf, &self->m_nNextPathNodeInfo, sizeof(self->m_nNextPathNodeInfo));
	WriteSaveBuf(buf, &self->m_nPreviousPathNodeInfo, sizeof(self->m_nPreviousPathNodeInfo));
	WriteSaveBuf(buf, &self->m_nAntiReverseTimer, sizeof(self->m_nAntiReverseTimer));
	WriteSaveBuf(buf, &self->m_nTimeToStartMission, sizeof(self->m_nTimeToStartMission));
	WriteSaveBuf(buf, &self->m_nPreviousDirection, sizeof(self->m_nPreviousDirection));
	WriteSaveBuf(buf, &self->m_nCurrentDirection, sizeof(self->m_nCurrentDirection));
	WriteSaveBuf(buf, &self->m_nNextDirection, sizeof(self->m_nNextDirection));
	WriteSaveBuf(buf, &self->m_nCurrentLane, sizeof(self->m_nCurrentLane));
	WriteSaveBuf(buf, &self->m_nNextLane, sizeof(self->m_nNextLane));
	WriteSaveBuf(buf, &self->m_nDrivingStyle, sizeof(self->m_nDrivingStyle));
	WriteSaveBuf(buf, &self->m_nCarMission, sizeof(self->m_nCarMission));
	WriteSaveBuf(buf, &self->m_nTempAction, sizeof(self->m_nTempAction));
	WriteSaveBuf(buf, &self->m_nTimeTempAction, sizeof(self->m_nTimeTempAction));
	WriteSaveBuf(buf, &self->m_fMaxTrafficSpeed, sizeof(self->m_fMaxTrafficSpeed));
	WriteSaveBuf(buf, &self->m_nCruiseSpeed, sizeof(self->m_nCruiseSpeed));
	WriteSaveBuf(buf, &self->m_nCruiseSpeedMultiplierType, sizeof(self->m_nCruiseSpeedMultiplierType));
	ZeroSaveBuf(buf, 2);
	WriteSaveBuf(buf, &self->m_fCruiseSpeedMultiplier, sizeof(self->m_fCruiseSpeedMultiplier));
	//- rouz edit (ChatGPT)
	uint8_t flags = 0;
	if (self->m_bSlowedDownBecauseOfCars) flags |= BIT(0);
	if (self->m_bSlowedDownBecauseOfPeds) flags |= BIT(1);
	if (self->m_bStayInCurrentLevel) flags |= BIT(2);
	if (self->m_bStayInFastLane) flags |= BIT(3);
	if (self->m_bIgnorePathfinding) flags |= BIT(4);
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBuf(buf, &flags, sizeof(flags));
	WriteSaveBuf(buf, &self->m_nSwitchDistance, sizeof(self->m_nSwitchDistance));
	ZeroSaveBuf(buf, 2);
	WriteSaveBuf(buf, &self->m_vecDestinationCoors.x, sizeof(self->m_vecDestinationCoors.x));
	WriteSaveBuf(buf, &self->m_vecDestinationCoors.y, sizeof(self->m_vecDestinationCoors.y));
	WriteSaveBuf(buf, &self->m_vecDestinationCoors.z, sizeof(self->m_vecDestinationCoors.z));
	ZeroSaveBuf(buf, 32);
	WriteSaveBuf(buf, &self->m_nPathFindNodesCount, sizeof(self->m_nPathFindNodesCount));
	ZeroSaveBuf(buf, 6);
	//- rouz edit (ChatGPT)
}
#endif

#ifdef COMPATIBLE_SAVES
void AutoPilot_Load(CAutoPilot *self,uint8_t **buf)
{
    // Preserve the complete original Load operation and all intentionally unwritten fields
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&self->m_nCurrentRouteNode, buf, sizeof(self->m_nCurrentRouteNode));
	ReadSaveBuf(&self->m_nNextRouteNode, buf, sizeof(self->m_nNextRouteNode));
	ReadSaveBuf(&self->m_nPrevRouteNode, buf, sizeof(self->m_nPrevRouteNode));
	ReadSaveBuf(&self->m_nTimeEnteredCurve, buf, sizeof(self->m_nTimeEnteredCurve));
	ReadSaveBuf(&self->m_nTimeToSpendOnCurrentCurve, buf, sizeof(self->m_nTimeToSpendOnCurrentCurve));
	ReadSaveBuf(&self->m_nCurrentPathNodeInfo, buf, sizeof(self->m_nCurrentPathNodeInfo));
	ReadSaveBuf(&self->m_nNextPathNodeInfo, buf, sizeof(self->m_nNextPathNodeInfo));
	ReadSaveBuf(&self->m_nPreviousPathNodeInfo, buf, sizeof(self->m_nPreviousPathNodeInfo));
	ReadSaveBuf(&self->m_nAntiReverseTimer, buf, sizeof(self->m_nAntiReverseTimer));
	ReadSaveBuf(&self->m_nTimeToStartMission, buf, sizeof(self->m_nTimeToStartMission));
	ReadSaveBuf(&self->m_nPreviousDirection, buf, sizeof(self->m_nPreviousDirection));
	ReadSaveBuf(&self->m_nCurrentDirection, buf, sizeof(self->m_nCurrentDirection));
	ReadSaveBuf(&self->m_nNextDirection, buf, sizeof(self->m_nNextDirection));
	ReadSaveBuf(&self->m_nCurrentLane, buf, sizeof(self->m_nCurrentLane));
	ReadSaveBuf(&self->m_nNextLane, buf, sizeof(self->m_nNextLane));
	ReadSaveBuf(&self->m_nDrivingStyle, buf, sizeof(self->m_nDrivingStyle));
	ReadSaveBuf(&self->m_nCarMission, buf, sizeof(self->m_nCarMission));
	ReadSaveBuf(&self->m_nTempAction, buf, sizeof(self->m_nTempAction));
	ReadSaveBuf(&self->m_nTimeTempAction, buf, sizeof(self->m_nTimeTempAction));
	ReadSaveBuf(&self->m_fMaxTrafficSpeed, buf, sizeof(self->m_fMaxTrafficSpeed));
	ReadSaveBuf(&self->m_nCruiseSpeed, buf, sizeof(self->m_nCruiseSpeed));
	ReadSaveBuf(&self->m_nCruiseSpeedMultiplierType, buf, sizeof(self->m_nCruiseSpeedMultiplierType));
	SkipSaveBuf(buf, 2);
	ReadSaveBuf(&self->m_fCruiseSpeedMultiplier, buf, sizeof(self->m_fCruiseSpeedMultiplier));
	//- rouz edit (ChatGPT)
	uint8_t flags;
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&flags, buf, sizeof(flags));
	//- rouz edit (ChatGPT)
	self->m_bSlowedDownBecauseOfCars = !!(flags & BIT(0));
	self->m_bSlowedDownBecauseOfPeds = !!(flags & BIT(1));
	self->m_bStayInCurrentLevel = !!(flags & BIT(2));
	self->m_bStayInFastLane = !!(flags & BIT(3));
	self->m_bIgnorePathfinding = !!(flags & BIT(4));
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&self->m_nSwitchDistance, buf, sizeof(self->m_nSwitchDistance));
	SkipSaveBuf(buf, 2);
	ReadSaveBuf(&self->m_vecDestinationCoors.x, buf, sizeof(self->m_vecDestinationCoors.x));
	ReadSaveBuf(&self->m_vecDestinationCoors.y, buf, sizeof(self->m_vecDestinationCoors.y));
	ReadSaveBuf(&self->m_vecDestinationCoors.z, buf, sizeof(self->m_vecDestinationCoors.z));
	SkipSaveBuf(buf, 32);
	ReadSaveBuf(&self->m_nPathFindNodesCount, buf, sizeof(self->m_nPathFindNodesCount));
	SkipSaveBuf(buf, 6);
	//- rouz edit (ChatGPT)
}
#endif
//- rouz edit (ChatGPT)
