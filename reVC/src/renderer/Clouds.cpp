#include "common.h"

#include "main.h"
#include "Sprite.h"
#include "Sprite2d.h"
#include "General.h"
#include "Game.h"
#include "Coronas.h"
#include "Camera.h"
#include "TxdStore.h"
#include "Weather.h"
#include "Clock.h"
#include "Timer.h"
#include "Timecycle.h"
#include "Renderer.h"
#include "Clouds.h"

#define SMALLSTRIPHEIGHT 4.0f
#define HORIZSTRIPHEIGHT 48.0f

RwTexture *gpCloudTex[5];

float CClouds::CloudRotation;
uint32 CClouds::IndividualRotation;

float CClouds::ms_cameraRoll;
float CClouds::ms_horizonZ;
float CClouds::ms_HorizonTilt;
CRGBA CClouds::ms_colourTop;
CRGBA CClouds::ms_colourBottom;
CRGBA CClouds::ms_colourBkGrd;

//+ rouz edit (ChatGPT)
static float HorizonX;
static float HorizonSkyX;
static float HorizonSkyY;
static float HorizonLineX;
static float HorizonLineY;
struct HorizonState
{
	float x;
	float z;
	float skyX;
	float skyY;
	float lineX;
	float lineY;
};
static HorizonState LastSafeHorizon;
static bool HaveLastSafeHorizon;
//- rouz edit (ChatGPT)

void
CClouds::Init(void)
{
	CTxdStore::PushCurrentTxd();
	CTxdStore::SetCurrentTxd(CTxdStore::FindTxdSlot("particle"));
	gpCloudTex[0] = RwTextureRead("cloud1", nil);
	gpCloudTex[1] = RwTextureRead("cloud2", nil);
	gpCloudTex[2] = RwTextureRead("cloud3", nil);
	gpCloudTex[3] = RwTextureRead("cloudhilit", nil);
	gpCloudTex[4] = RwTextureRead("cloudmasked", nil);
	CTxdStore::PopCurrentTxd();
	CloudRotation = 0.0f;
}

void
CClouds::Shutdown(void)
{
	RwTextureDestroy(gpCloudTex[0]);
	gpCloudTex[0] = nil;
	RwTextureDestroy(gpCloudTex[1]);
	gpCloudTex[1] = nil;
	RwTextureDestroy(gpCloudTex[2]);
	gpCloudTex[2] = nil;
	RwTextureDestroy(gpCloudTex[3]);
	gpCloudTex[3] = nil;
	RwTextureDestroy(gpCloudTex[4]);
	gpCloudTex[4] = nil;
}

void
CClouds::Update(void)
{
	float s = Sin(TheCamera.Orientation - 0.85f);
#ifdef FIX_BUGS
	CloudRotation += CWeather::Wind*s*0.001f*CTimer::GetTimeStepFix();
	IndividualRotation += (CWeather::Wind*CTimer::GetTimeStep()*0.5f + 0.3f*CTimer::GetTimeStepFix()) * 60.0f;
#else
	CloudRotation += CWeather::Wind*s*0.001f;
	IndividualRotation += (CWeather::Wind*CTimer::GetTimeStep()*0.5f + 0.3f) * 60.0f;
#endif
}

float StarCoorsX[9] = { 0.0f, 0.05f, 0.13f, 0.4f, 0.7f, 0.6f, 0.27f, 0.55f, 0.75f };
float StarCoorsY[9] = { 0.0f, 0.45f, 0.9f, 1.0f, 0.85f, 0.52f, 0.48f, 0.35f, 0.2f };
float StarSizes[9] = { 1.0f, 1.4f, 0.9f, 1.0f, 0.6f, 1.5f, 1.3f, 1.0f, 0.8f };

float LowCloudsX[12] = { 1.0f, 0.7f, 0.0f, -0.7f, -1.0f, -0.7f, 0.0f, 0.7f, 0.8f, -0.8f, 0.4f, -0.4f };
float LowCloudsY[12] = { 0.0f, -0.7f, -1.0f, -0.7f, 0.0f, 0.7f, 1.0f, 0.7f, 0.4f, 0.4f, -0.8f, -0.8f };
float LowCloudsZ[12] = { 0.0f, 1.0f, 0.5f, 0.0f, 1.0f, 0.3f, 0.9f, 0.4f, 1.3f, 1.4f, 1.2f, 1.7f };

float CoorsOffsetX[37] = {
	0.0f, 60.0f, 72.0f, 48.0f, 21.0f, 12.0f,
	9.0f, -3.0f, -8.4f, -18.0f, -15.0f, -36.0f,
	-40.0f, -48.0f, -60.0f, -24.0f, 100.0f, 100.0f,
	100.0f, 100.0f, 100.0f, 100.0f, 100.0f, 100.0f,
	100.0f, 100.0f, -30.0f, -20.0f, 10.0f, 30.0f,
	0.0f, -100.0f, -100.0f, -100.0f, -100.0f, -100.0f, -100.0f
};
float CoorsOffsetY[37] = {
	100.0f, 100.0f, 100.0f, 100.0f, 100.0f, 100.0f,
	100.0f, 100.0f, 100.0f, 100.0f, 100.0f, 100.0f,
	100.0f, 100.0f, 100.0f, 100.0f, -30.0f, 10.0f,
	-25.0f, -5.0f, 28.0f, -10.0f, 10.0f, 0.0f,
	15.0f, 40.0f, -100.0f, -100.0f, -100.0f, -100.0f,
	-100.0f, -40.0f, -20.0f, 0.0f, 10.0f, 30.0f, 35.0f
};
float CoorsOffsetZ[37] = {
	2.0f, 1.0f, 0.0f, 0.3f, 0.7f, 1.4f,
	1.7f, 0.24f, 0.7f, 1.3f, 1.6f, 1.0f,
	1.2f, 0.3f, 0.7f, 1.4f, 0.0f, 0.1f,
	0.5f, 0.4f, 0.55f, 0.75f, 1.0f, 1.4f,
	1.7f, 2.0f, 2.0f, 2.3f, 1.9f, 2.4f,
	2.0f, 2.0f, 1.5f, 1.2f, 1.7f, 1.5f, 2.1f
};

uint8 BowRed[6] = { 30, 30, 30, 10, 0, 15 };
uint8 BowGreen[6] = { 0, 15, 30, 30, 0, 0 };
uint8 BowBlue[6] = { 0, 0, 0, 10, 30, 30 };

void
CClouds::Render(void)
{
	int i;
	float szx, szy;
	RwV3d screenpos;
	RwV3d worldpos;

	if(!CGame::CanSeeOutSideFromCurrArea())
		return;

	PUSH_RENDERGROUP("CClouds::Render");

	CCoronas::SunBlockedByClouds = false;

	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDONE);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDONE);
	CSprite::InitSpriteBuffer();

	float minute = CClock::GetHours()*60 + CClock::GetMinutes() + CClock::GetSeconds()/60.0f;
	RwV3d campos = TheCamera.GetPosition();

	// Moon
	float moonfadeout = Abs(minute - 180.0f);	// fully visible at 3AM
	if((int)moonfadeout < 180){			// fade in/out 3 hours
		float coverage = Max(CWeather::Foggyness, CWeather::CloudCoverage);
		int brightness = (1.0f - coverage) * (180 - (int)moonfadeout);
		RwV3d pos = { 0.0f, -100.0f, 15.0f };
		RwV3dAdd(&worldpos, &campos, &pos);
		if(CSprite::CalcScreenCoors(worldpos, &screenpos, &szx, &szy, false)){
			RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpCoronaTexture[2]));
			szx *= CCoronas::MoonSize*2.0f + 4.0f;
			szy *= CCoronas::MoonSize*2.0f + 4.0f;
			CSprite::RenderOneXLUSprite(screenpos.x, screenpos.y, screenpos.z,
				szx, szy, brightness, brightness, brightness, 255, 1.0f/screenpos.z, 255);
		}
	}

	// The R* logo
	int starintens = 0;
	if(CClock::GetHours() < 22 && CClock::GetHours() > 5)
		starintens = 0;
	else if(CClock::GetHours() > 22 || CClock::GetHours() < 5)
		starintens = 255;
	else if(CClock::GetHours() == 22)
		starintens = 255 * CClock::GetMinutes()/60.0f;
	else if(CClock::GetHours() == 5)
		starintens = 255 * (60 - CClock::GetMinutes())/60.0f;
	if(starintens != 0){
		float coverage = Max(CWeather::Foggyness, CWeather::CloudCoverage);
		int brightness = (1.0f - coverage) * starintens;

		// R
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpCoronaTexture[0]));
		for(i = 0; i < 11; i++){
			RwV3d pos = { 100.0f, 0.0f, 10.0f };
			if(i >= 9) pos.x = -pos.x;
			RwV3dAdd(&worldpos, &campos, &pos);
			worldpos.y -= 90.0f*StarCoorsX[i%9];
			worldpos.z += 80.0f*StarCoorsY[i%9];
			if(CSprite::CalcScreenCoors(worldpos, &screenpos, &szx, &szy, false)){
				float sz = 0.8f*StarSizes[i%9];
				CSprite::RenderBufferedOneXLUSprite(screenpos.x, screenpos.y, screenpos.z,
					szx*sz, szy*sz, brightness, brightness, brightness, 255, 1.0f/screenpos.z, 255);
			}
		}
		CSprite::FlushSpriteBuffer();

		// *
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpCoronaTexture[0]));
		RwV3d pos = { 100.0f, 0.0f, 10.0f };
		RwV3dAdd(&worldpos, &campos, &pos);
		worldpos.y -= 90.0f;
		if(CSprite::CalcScreenCoors(worldpos, &screenpos, &szx, &szy, false)){
			brightness *= (CGeneral::GetRandomNumber()&127) / 640.0f + 0.5f;
			CSprite::RenderOneXLUSprite(screenpos.x, screenpos.y, screenpos.z,
				szx*5.0f, szy*5.0f, brightness, brightness, brightness, 255, 1.0f/screenpos.z, 255);
		}
	}

	// Low clouds
	float lowcloudintensity = 1.0f - Max(Max(CWeather::Foggyness, CWeather::CloudCoverage), CWeather::ExtraSunnyness);
	int r = CTimeCycle::GetLowCloudsRed() * lowcloudintensity;
	int g = CTimeCycle::GetLowCloudsGreen() * lowcloudintensity;
	int b = CTimeCycle::GetLowCloudsBlue() * lowcloudintensity;
	for(int cloudtype = 0; cloudtype < 3; cloudtype++){
		for(i = cloudtype; i < 12; i += 3){
			RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpCloudTex[cloudtype]));
			RwV3d pos = { 800.0f*LowCloudsX[i], 800.0f*LowCloudsY[i], 60.0f*LowCloudsZ[i] };
			worldpos.x = campos.x + pos.x;
			worldpos.y = campos.y + pos.y;
			worldpos.z = 40.0f + pos.z;
			if(CSprite::CalcScreenCoors(worldpos, &screenpos, &szx, &szy, false))
				CSprite::RenderBufferedOneXLUSprite_Rotate_Dimension(screenpos.x, screenpos.y, screenpos.z,
					szx*320.0f, szy*40.0f, r, g, b, 255, 1.0f/screenpos.z, ms_cameraRoll, 255);
		}
		CSprite::FlushSpriteBuffer();
	}

	// Fluffy clouds
	float rot_sin = Sin(CloudRotation);
	float rot_cos = Cos(CloudRotation);
	int fluffyalpha = 160 * (1.0f - Max(CWeather::Foggyness, CWeather::ExtraSunnyness));
	if(fluffyalpha != 0){
		static bool bCloudOnScreen[37];
		float sundist, hilight;

		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpCloudTex[4]));
		for(i = 0; i < 37; i++){
			RwV3d pos = { 2.0f*CoorsOffsetX[i], 2.0f*CoorsOffsetY[i], 40.0f*CoorsOffsetZ[i] + 40.0f };
			worldpos.x = pos.x*rot_cos + pos.y*rot_sin + campos.x;
			worldpos.y = pos.x*rot_sin - pos.y*rot_cos + campos.y;
			worldpos.z = pos.z;

			if(CSprite::CalcScreenCoors(worldpos, &screenpos, &szx, &szy, false)){
				sundist = Sqrt(sq(screenpos.x-CCoronas::SunScreenX) + sq(screenpos.y-CCoronas::SunScreenY));
				int tr = CTimeCycle::GetFluffyCloudsTopRed();
				int tg = CTimeCycle::GetFluffyCloudsTopGreen();
				int tb = CTimeCycle::GetFluffyCloudsTopBlue();
				int br = CTimeCycle::GetFluffyCloudsBottomRed();
				int bg = CTimeCycle::GetFluffyCloudsBottomGreen();
				int bb = CTimeCycle::GetFluffyCloudsBottomBlue();
				int distLimit = (3*SCREEN_WIDTH)/4;
				if(sundist < distLimit){
					hilight = (1.0f - Max(CWeather::Foggyness, CWeather::CloudCoverage)) * (1.0f - sundist/(float)distLimit);
					tr = tr*(1.0f-hilight) + 255*hilight;
					tg = tg*(1.0f-hilight) + 190*hilight;
					tb = tb*(1.0f-hilight) + 190*hilight;
					br = br*(1.0f-hilight) + 255*hilight;
					bg = bg*(1.0f-hilight) + 190*hilight;
					bb = bb*(1.0f-hilight) + 190*hilight;
					if(sundist < SCREEN_WIDTH/10)
						CCoronas::SunBlockedByClouds = true;
				}else
					hilight = 0.0f;
				CSprite::RenderBufferedOneXLUSprite_Rotate_2Colours(screenpos.x, screenpos.y, screenpos.z,
					szx*55.0f, szy*55.0f,
					tr, tg, tb, br, bg, bb, 0.0f, -1.0f,
					1.0f/screenpos.z,
					(uint16)IndividualRotation/65336.0f * 6.28f + ms_cameraRoll,
					fluffyalpha);
				bCloudOnScreen[i] = true;
			}else
				bCloudOnScreen[i] = false;
		}
		CSprite::FlushSpriteBuffer();

		// Highlights
		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDONE);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDONE);
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpCloudTex[3]));

		for(i = 0; i < 37; i++){
			RwV3d pos = { 2.0f*CoorsOffsetX[i], 2.0f*CoorsOffsetY[i], 40.0f*CoorsOffsetZ[i] + 40.0f };
			worldpos.x = pos.x*rot_cos + pos.y*rot_sin + campos.x;
			worldpos.y = pos.x*rot_sin + pos.y*rot_cos + campos.y;
			worldpos.z = pos.z;
			if(bCloudOnScreen[i] && CSprite::CalcScreenCoors(worldpos, &screenpos, &szx, &szy, false)){
				if(sundist < SCREEN_WIDTH/3){
					CSprite::RenderBufferedOneXLUSprite_Rotate_Aspect(screenpos.x, screenpos.y, screenpos.z,
						szx*30.0f, szy*30.0f,
						200*hilight, 0, 0, 255, 1.0f/screenpos.z,
						1.7f - CGeneral::GetATanOfXY(screenpos.x-CCoronas::SunScreenX, screenpos.y-CCoronas::SunScreenY) + CClouds::ms_cameraRoll, 255);
				}
			}
		}
		CSprite::FlushSpriteBuffer();
	}

	// Rainbow
	if(CWeather::Rainbow != 0.0f){
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpCoronaTexture[0]));
		for(i = 0; i < 6; i++){
			RwV3d pos = { i*1.5f, 100.0f, 5.0f };
			RwV3dAdd(&worldpos, &campos, &pos);
			if(CSprite::CalcScreenCoors(worldpos, &screenpos, &szx, &szy, false))
				CSprite::RenderBufferedOneXLUSprite(screenpos.x, screenpos.y, screenpos.z,
					2.0f*szx, 50.0*szy,
					BowRed[i]*CWeather::Rainbow, BowGreen[i]*CWeather::Rainbow, BowBlue[i]*CWeather::Rainbow,
					255, 1.0f/screenpos.z, 255);

		}
		CSprite::FlushSpriteBuffer();
	}

	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);

	POP_RENDERGROUP();
}

bool
UseDarkBackground(void)
{
	return TheCamera.GetForward().z < -0.9f || gbShowCollisionPolys;
}

//+ rouz edit (ChatGPT)
static bool
UseSkyTopBackground(void)
{
	return TheCamera.GetForward().z > 0.98f;
}

static HorizonState
GetCurrentHorizonState(void)
{
	HorizonState state;
	state.x = HorizonX;
	state.z = CClouds::ms_horizonZ;
	state.skyX = HorizonSkyX;
	state.skyY = HorizonSkyY;
	state.lineX = HorizonLineX;
	state.lineY = HorizonLineY;
	return state;
}

static void
SetCurrentHorizonState(const HorizonState &state)
{
	HorizonX = state.x;
	CClouds::ms_horizonZ = state.z;
	HorizonSkyX = state.skyX;
	HorizonSkyY = state.skyY;
	HorizonLineX = state.lineX;
	HorizonLineY = state.lineY;
}

static HorizonState
GetMirroredPoleHorizonState(const HorizonState &state)
{
	HorizonState mirrored = state;
	mirrored.x = SCREEN_WIDTH - state.x;
	mirrored.z = SCREEN_HEIGHT - state.z;
	mirrored.skyX = SCREEN_WIDTH - state.skyX;
	mirrored.skyY = SCREEN_HEIGHT - state.skyY;
	mirrored.lineX = -state.lineX;
	mirrored.lineY = -state.lineY;
	return mirrored;
}

static void CalcScreenCoorsNoClip(const CVector &in, float &x, float &y)
{
	CVector view = TheCamera.m_viewMatrix * in;
	if(Abs(view.z) < 0.001f)
		view.z = view.z < 0.0f ? -0.001f : 0.001f;
	x = view.x * SCREEN_WIDTH / view.z;
	y = view.y * SCREEN_HEIGHT / view.z;
}

static void UpdateHorizonCoors(void)
{
	CVector forward(TheCamera.GetForward().x, TheCamera.GetForward().y, 0.0f);
	if(forward.MagnitudeSqr() < 0.0001f){
		HorizonX = SCREEN_WIDTH/2.0f;
		HorizonSkyX = SCREEN_WIDTH/2.0f;
		HorizonSkyY = SCREEN_HEIGHT/2.0f - 1.0f;
		CClouds::ms_horizonZ = SCREEN_HEIGHT/2.0f;
		HorizonLineX = Cos(CClouds::ms_cameraRoll);
		HorizonLineY = -Sin(CClouds::ms_cameraRoll);
		return;
	}
	forward.Normalise();

	CVector right = CrossProduct(CVector(0.0f, 0.0f, 1.0f), forward);
	right.Normalise();

	CVector center = TheCamera.GetPosition() + forward*3000.0f;
	center.z = 0.0f;
	CVector left = center - right*3000.0f;
	CVector rightPos = center + right*3000.0f;
	CVector sky = center;
	sky.z += 1.0f;

	CalcScreenCoorsNoClip(center, HorizonX, CClouds::ms_horizonZ);
	CalcScreenCoorsNoClip(sky, HorizonSkyX, HorizonSkyY);
	float leftX, leftY, rightX, rightY;
	CalcScreenCoorsNoClip(left, leftX, leftY);
	CalcScreenCoorsNoClip(rightPos, rightX, rightY);

	HorizonLineX = rightX - leftX;
	HorizonLineY = rightY - leftY;
	float len = Sqrt(SQR(HorizonLineX) + SQR(HorizonLineY));
	if(len > 0.001f){
		HorizonLineX /= len;
		HorizonLineY /= len;
	}else{
		HorizonLineX = Cos(CClouds::ms_cameraRoll);
		HorizonLineY = -Sin(CClouds::ms_cameraRoll);
	}
}

static void DrawHorizonBand(float centerY, float topOffset, float bottomOffset, const CRGBA &top, const CRGBA &bottom)
{
	float halfWidth = 2.0f*Sqrt(SQR(SCREEN_WIDTH) + SQR(SCREEN_HEIGHT));
	float lineX = HorizonLineX;
	float lineY = HorizonLineY;
	float downX = -lineY;
	float downY = lineX;
	float skySide = (HorizonSkyX - HorizonX)*downX + (HorizonSkyY - CClouds::ms_horizonZ)*downY;
	if(skySide > 0.0f){
		downX = -downX;
		downY = -downY;
	}

	float topLeftX = HorizonX - lineX*halfWidth + downX*topOffset;
	float topLeftY = centerY - lineY*halfWidth + downY*topOffset;
	float topRightX = HorizonX + lineX*halfWidth + downX*topOffset;
	float topRightY = centerY + lineY*halfWidth + downY*topOffset;
	float bottomLeftX = HorizonX - lineX*halfWidth + downX*bottomOffset;
	float bottomLeftY = centerY - lineY*halfWidth + downY*bottomOffset;
	float bottomRightX = HorizonX + lineX*halfWidth + downX*bottomOffset;
	float bottomRightY = centerY + lineY*halfWidth + downY*bottomOffset;

	//+ rouz edit (ChatGPT)
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	//- rouz edit (ChatGPT)
	CSprite2d::DrawAnyRect(topLeftX, topLeftY, topRightX, topRightY,
		bottomLeftX, bottomLeftY, bottomRightX, bottomRightY,
		top, top, bottom, bottom);
}

//+ rouz edit (ChatGPT)
static float GetHorizonScreenDepth(void)
{
	float downX = -HorizonLineY;
	float downY = HorizonLineX;
	float skySide = (HorizonSkyX - HorizonX)*downX + (HorizonSkyY - CClouds::ms_horizonZ)*downY;
	if(skySide > 0.0f){
		downX = -downX;
		downY = -downY;
	}

	float d0 = Abs((0.0f - HorizonX)*downX + (0.0f - CClouds::ms_horizonZ)*downY);
	float d1 = Abs((SCREEN_WIDTH - HorizonX)*downX + (0.0f - CClouds::ms_horizonZ)*downY);
	float d2 = Abs((0.0f - HorizonX)*downX + (SCREEN_HEIGHT - CClouds::ms_horizonZ)*downY);
	float d3 = Abs((SCREEN_WIDTH - HorizonX)*downX + (SCREEN_HEIGHT - CClouds::ms_horizonZ)*downY);
	return Max(Max(d0, d1), Max(d2, d3)) + SCREEN_HEIGHT;
}

static CRGBA
ScaleRGBAAlpha(const CRGBA &colour, float scale)
{
	scale = Clamp(scale, 0.0f, 1.0f);
	return CRGBA(colour.r, colour.g, colour.b, uint8(Clamp(colour.a*scale, 0.0f, 255.0f)));
}

static void
DrawBackgroundGradientBands(int16 topred, int16 topgreen, int16 topblue,
	int16 botred, int16 botgreen, int16 botblue, int16 alpha, float alphaScale)
{
	int fogr = (topred + 2 * botred) / 3;
	int fogg = (topgreen + 2 * botgreen) / 3;
	int fogb = (topblue + 2 * botblue) / 3;
	float skyDepth = GetHorizonScreenDepth();

	CClouds::ms_colourTop = ScaleRGBAAlpha(CRGBA(topred, topgreen, topblue, alpha), alphaScale);
	CClouds::ms_colourBottom = ScaleRGBAAlpha(CRGBA(botred, botgreen, botblue, alpha), alphaScale);
	DrawHorizonBand(CClouds::ms_horizonZ, -skyDepth, 0.0f, CClouds::ms_colourTop, CClouds::ms_colourBottom);

	CClouds::ms_colourTop = ScaleRGBAAlpha(CRGBA(fogr, fogg, fogb, alpha), alphaScale);
	DrawHorizonBand(CClouds::ms_horizonZ, 0.0f, SMALLSTRIPHEIGHT, CClouds::ms_colourTop, CClouds::ms_colourTop);

	CClouds::ms_colourTop = CRGBA(fogr, fogg, fogb, alpha);
	CClouds::ms_colourBottom = CClouds::ms_colourTop;
}
//- rouz edit (ChatGPT)

void
CClouds::RenderBackground(int16 topred, int16 topgreen, int16 topblue,
	int16 botred, int16 botgreen, int16 botblue, int16 alpha)
{
	PUSH_RENDERGROUP("CClouds::RenderBackground");

	//+ rouz edit (ChatGPT)
	CVector camRight = CrossProduct(TheCamera.GetUp(), TheCamera.GetForward());
	camRight.Normalise();
	CVector levelRight = CrossProduct(CVector(0.0f, 0.0f, 1.0f), TheCamera.GetForward());
	if(levelRight.MagnitudeSqr() > 0.0001f){
		levelRight.Normalise();
		ms_cameraRoll = Atan2(camRight.z, DotProduct(camRight, levelRight));
	}else
		ms_cameraRoll = 0.0f;
	ms_HorizonTilt = 0.0f;
	UpdateHorizonCoors();
	//- rouz edit (ChatGPT)

	if(UseDarkBackground()){
		ms_colourTop.r = 50;
		ms_colourTop.g = 50;
		ms_colourTop.b = 50;
		ms_colourTop.a = 255;
		if(gbShowCollisionPolys){
			if(CTimer::GetFrameCounter() & 1){
				ms_colourTop.r = 0;
				ms_colourTop.g = 0;
				ms_colourTop.b = 0;
			}else{
				ms_colourTop.r = 255;
				ms_colourTop.g = 255;
				ms_colourTop.b = 255;
			}
		}
		ms_colourBottom = ms_colourTop;
		CRect r(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		CSprite2d::DrawRect(r, ms_colourBottom, ms_colourBottom, ms_colourTop, ms_colourTop);
	//+ rouz edit (ChatGPT)
	}else if(UseSkyTopBackground()){
		HorizonState currentHorizon = GetCurrentHorizonState();
		if(!HaveLastSafeHorizon){
			LastSafeHorizon = currentHorizon;
			HaveLastSafeHorizon = true;
		}

		float t = Clamp((TheCamera.GetForward().z - 0.98f)/(1.0f - 0.98f), 0.0f, 1.0f);
		t = t*t*(3.0f - 2.0f*t);

		SetCurrentHorizonState(LastSafeHorizon);
		DrawBackgroundGradientBands(topred, topgreen, topblue, botred, botgreen, botblue, alpha, 1.0f);

		SetCurrentHorizonState(GetMirroredPoleHorizonState(LastSafeHorizon));
		DrawBackgroundGradientBands(topred, topgreen, topblue, botred, botgreen, botblue, alpha, t);

		SetCurrentHorizonState(currentHorizon);
	}else{
		LastSafeHorizon = GetCurrentHorizonState();
		HaveLastSafeHorizon = true;
		DrawBackgroundGradientBands(topred, topgreen, topblue, botred, botgreen, botblue, alpha, 1.0f);
	}
	//- rouz edit (ChatGPT)

	POP_RENDERGROUP();
}

void
CClouds::RenderHorizon(void)
{
	// rouz edit (ChatGPT)
	if(UseDarkBackground() || UseSkyTopBackground())
		return;

	PUSH_RENDERGROUP("CClouds::RenderHorizon");

	ms_colourBottom.a = 230;
	ms_colourTop.a = 80;

	// rouz edit (ChatGPT)
	DrawHorizonBand(ms_horizonZ, 0.0f, SMALLSTRIPHEIGHT, ms_colourTop, ms_colourBottom);

	ms_colourBkGrd.r = 128.0f*CTimeCycle::GetAmbientRed();
	ms_colourBkGrd.g = 128.0f*CTimeCycle::GetAmbientGreen();
	ms_colourBkGrd.b = 128.0f*CTimeCycle::GetAmbientBlue();
	ms_colourBkGrd.a = 255;

	float horzstrip = SCREEN_STRETCH_Y(HORIZSTRIPHEIGHT);
	//+ rouz edit (ChatGPT)
	float skyDepth = GetHorizonScreenDepth();

	DrawHorizonBand(ms_horizonZ, SMALLSTRIPHEIGHT, SMALLSTRIPHEIGHT + horzstrip,
		ms_colourBottom, ms_colourBkGrd);
	DrawHorizonBand(ms_horizonZ, SMALLSTRIPHEIGHT + horzstrip, skyDepth,
		ms_colourBkGrd, ms_colourBkGrd);
	//- rouz edit

	POP_RENDERGROUP();
}
