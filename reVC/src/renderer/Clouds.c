//+ rouz edit (ChatGPT)
#include "Clouds_backend.h"
#include "Clock.h"
#include "Timer.h"
#include "General.h"
#include "RenderBuffer.h"
#include "Draw_constants.h"
#include <math.h>
#include <stddef.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
float Clouds_CloudRotation;
uint32_t Clouds_IndividualRotation;
float Clouds_ms_cameraRoll;
float Clouds_ms_horizonZ;
float Clouds_ms_HorizonTilt;
static void *gpCloudTex[5];
static CloudsVector Clouds_Add(CloudsVector a,CloudsVector b)
{
    // Preserve original component addition
    CloudsVector r={a.x+b.x,a.y+b.y,a.z+b.z};return r;
}
static CloudsVector Clouds_Subtract(CloudsVector a,CloudsVector b)
{
    // Preserve original component subtraction
    CloudsVector r={a.x-b.x,a.y-b.y,a.z-b.z};return r;
}
static CloudsVector Clouds_Scale(CloudsVector a,float b)
{
    // Preserve original vector-times-scalar operand order
    CloudsVector r={a.x*b,a.y*b,a.z*b};return r;
}
static float Clouds_Dot(CloudsVector a,CloudsVector b)
{
    // Preserve the original three-product addition order
    return a.x*b.x+a.y*b.y+a.z*b.z;
}
static void Clouds_Normalise(CloudsVector *v)
{
    // Retain the original reciprocal-square-root scale and x-only zero fallback
    float sq=Clouds_Dot(*v,*v);if(sq>0.f){float inv=1.f/sqrtf(sq);v->x*=inv;v->y*=inv;v->z*=inv;}else v->x=1.f;
}
static float Clouds_Max(float a,float b)
{
    // Preserve the original comparison and selected operand
    return a>b?a:b;
}
static float Clouds_Square(float value)
{
    // Evaluate each original squared operand once
    return value*value;
}
static CloudsColor Clouds_Color(uint8_t r,uint8_t g,uint8_t b,uint8_t a)
{
    // Narrow original color arguments before storing their four bytes
    CloudsColor c={r,g,b,a};return c;
}
void
Clouds_Init(void)
{
    // Preserve the complete original effect operation and live state-write order
	Clouds_BackendPushTxd();
	Clouds_BackendSetTxd(Clouds_BackendFindTxd("particle"));
	gpCloudTex[0] = Clouds_BackendReadTexture("cloud1", NULL);
	gpCloudTex[1] = Clouds_BackendReadTexture("cloud2", NULL);
	gpCloudTex[2] = Clouds_BackendReadTexture("cloud3", NULL);
	gpCloudTex[3] = Clouds_BackendReadTexture("cloudhilit", NULL);
	gpCloudTex[4] = Clouds_BackendReadTexture("cloudmasked", NULL);
	Clouds_BackendPopTxd();
	Clouds_CloudRotation = 0.0f;
}

void
Clouds_Shutdown(void)
{
    // Preserve the complete original effect operation and live state-write order
	Clouds_BackendDestroyTexture(gpCloudTex[0]);
	gpCloudTex[0] = NULL;
	Clouds_BackendDestroyTexture(gpCloudTex[1]);
	gpCloudTex[1] = NULL;
	Clouds_BackendDestroyTexture(gpCloudTex[2]);
	gpCloudTex[2] = NULL;
	Clouds_BackendDestroyTexture(gpCloudTex[3]);
	gpCloudTex[3] = NULL;
	Clouds_BackendDestroyTexture(gpCloudTex[4]);
	gpCloudTex[4] = NULL;
}

void
Clouds_Update(void)
{
    // Preserve the complete original effect operation and live state-write order
	float s = sinf(Clouds_BackendOrientation() - 0.85f);
#ifdef FIX_BUGS
	Clouds_CloudRotation += Clouds_BackendWind()*s*0.001f*Timer_GetTimeStepFix();
	Clouds_IndividualRotation += (Clouds_BackendWind()*(*Timer_GetTimeStep())*0.5f + 0.3f*Timer_GetTimeStepFix()) * 60.0f;
#else
	Clouds_CloudRotation += Clouds_BackendWind()*s*0.001f;
	Clouds_IndividualRotation += (Clouds_BackendWind()*(*Timer_GetTimeStep())*0.5f + 0.3f) * 60.0f;
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

uint8_t BowRed[6] = { 30, 30, 30, 10, 0, 15 };
uint8_t BowGreen[6] = { 0, 15, 30, 30, 0, 0 };
uint8_t BowBlue[6] = { 0, 0, 0, 10, 30, 30 };

void
Clouds_Render(void)
{
    // Preserve the complete original effect operation and live state-write order
	int i;
	float szx, szy;
	CloudsVector screenpos;
	CloudsVector worldpos;

	if(!Clouds_BackendOutside())
		return;

	Clouds_BackendPush("CClouds::Render");

	(*Clouds_BackendSunBlocked()) = false;

	Clouds_BackendZWrite(false);
	Clouds_BackendZTest(false);
	Clouds_BackendVertexAlpha(true);
	Clouds_BackendSrcBlend(true);
	Clouds_BackendDestBlend(true);
	Clouds_BackendInitSprites();

	float minute = Clock_GetHours()*60 + Clock_GetMinutes() + Clock_GetSeconds()/60.0f;
	CloudsVector campos = Clouds_BackendCameraPosition();

	// Moon
	float moonfadeout = fabsf(minute - 180.0f);	// fully visible at 3AM
	if((int)moonfadeout < 180){			// fade in/out 3 hours
		float coverage = Clouds_Max(Clouds_BackendFoggyness(), Clouds_BackendCloudCoverage());
		int brightness = (1.0f - coverage) * (180 - (int)moonfadeout);
		CloudsVector pos = { 0.0f, -100.0f, 15.0f };
		(worldpos=Clouds_Add(campos,pos));
		if(Clouds_BackendProject(worldpos, &screenpos, &szx, &szy, false)){
			// Preserve original conditions, arithmetic and owner call ordering
			Clouds_BackendSetRaster( Clouds_BackendRaster(Clouds_BackendCoronaTexture(2)));
			szx *= Clouds_BackendMoonSize()*2.0f + 4.0f;
			szy *= Clouds_BackendMoonSize()*2.0f + 4.0f;
			Clouds_BackendRenderOneXLUSprite(screenpos.x, screenpos.y, screenpos.z,
				szx, szy, brightness, brightness, brightness, 255, 1.0f/screenpos.z, 255);
		}
	}

	// The R* logo
	int starintens = 0;
	if(Clock_GetHours() < 22 && Clock_GetHours() > 5)
		starintens = 0;
	else if(Clock_GetHours() > 22 || Clock_GetHours() < 5)
		starintens = 255;
	else if(Clock_GetHours() == 22)
		starintens = 255 * Clock_GetMinutes()/60.0f;
	else if(Clock_GetHours() == 5)
		starintens = 255 * (60 - Clock_GetMinutes())/60.0f;
	if(starintens != 0){
		// Preserve original conditions, arithmetic and owner call ordering
		float coverage = Clouds_Max(Clouds_BackendFoggyness(), Clouds_BackendCloudCoverage());
		int brightness = (1.0f - coverage) * starintens;

		// R
		Clouds_BackendSetRaster( Clouds_BackendRaster(Clouds_BackendCoronaTexture(0)));
		for(i = 0; i < 11; i++){
			// Preserve original conditions, arithmetic and owner call ordering
			CloudsVector pos = { 100.0f, 0.0f, 10.0f };
			if(i >= 9) pos.x = -pos.x;
			(worldpos=Clouds_Add(campos,pos));
			worldpos.y -= 90.0f*StarCoorsX[i%9];
			worldpos.z += 80.0f*StarCoorsY[i%9];
			if(Clouds_BackendProject(worldpos, &screenpos, &szx, &szy, false)){
				// Preserve original conditions, arithmetic and owner call ordering
				float sz = 0.8f*StarSizes[i%9];
				Clouds_BackendRenderBufferedOneXLUSprite(screenpos.x, screenpos.y, screenpos.z,
					szx*sz, szy*sz, brightness, brightness, brightness, 255, 1.0f/screenpos.z, 255);
			}
		}
		Clouds_BackendFlushSprites();

		// *
		Clouds_BackendSetRaster( Clouds_BackendRaster(Clouds_BackendCoronaTexture(0)));
		CloudsVector pos = { 100.0f, 0.0f, 10.0f };
		(worldpos=Clouds_Add(campos,pos));
		worldpos.y -= 90.0f;
		if(Clouds_BackendProject(worldpos, &screenpos, &szx, &szy, false)){
			// Preserve original conditions, arithmetic and owner call ordering
			brightness *= (General_GetRandomNumber()&127) / 640.0f + 0.5f;
			Clouds_BackendRenderOneXLUSprite(screenpos.x, screenpos.y, screenpos.z,
				szx*5.0f, szy*5.0f, brightness, brightness, brightness, 255, 1.0f/screenpos.z, 255);
		}
	}

	// Low clouds
	float lowcloudintensity = 1.0f - Clouds_Max(Clouds_Max(Clouds_BackendFoggyness(), Clouds_BackendCloudCoverage()), Clouds_BackendExtraSunnyness());
	int r = Clouds_BackendLowCloudsRed() * lowcloudintensity;
	int g = Clouds_BackendLowCloudsGreen() * lowcloudintensity;
	int b = Clouds_BackendLowCloudsBlue() * lowcloudintensity;
	for(int cloudtype = 0; cloudtype < 3; cloudtype++){
		// Preserve original conditions, arithmetic and owner call ordering
		for(i = cloudtype; i < 12; i += 3){
			// Preserve original conditions, arithmetic and owner call ordering
			Clouds_BackendSetRaster( Clouds_BackendRaster(gpCloudTex[cloudtype]));
			CloudsVector pos = { 800.0f*LowCloudsX[i], 800.0f*LowCloudsY[i], 60.0f*LowCloudsZ[i] };
			worldpos.x = campos.x + pos.x;
			worldpos.y = campos.y + pos.y;
			worldpos.z = 40.0f + pos.z;
			if(Clouds_BackendProject(worldpos, &screenpos, &szx, &szy, false))
				Clouds_BackendRenderBufferedOneXLUSprite_Rotate_Dimension(screenpos.x, screenpos.y, screenpos.z,
					szx*320.0f, szy*40.0f, r, g, b, 255, 1.0f/screenpos.z, Clouds_ms_cameraRoll, 255);
		}
		Clouds_BackendFlushSprites();
	}

	// Fluffy clouds
	float rot_sin = sinf(Clouds_CloudRotation);
	float rot_cos = cosf(Clouds_CloudRotation);
	int fluffyalpha = 160 * (1.0f - Clouds_Max(Clouds_BackendFoggyness(), Clouds_BackendExtraSunnyness()));
	if(fluffyalpha != 0){
		// Preserve original conditions, arithmetic and owner call ordering
		static bool bCloudOnScreen[37];
		float sundist, hilight;

		Clouds_BackendSrcBlend(false);
		Clouds_BackendDestBlend(false);
		Clouds_BackendSetRaster( Clouds_BackendRaster(gpCloudTex[4]));
		for(i = 0; i < 37; i++){
			// Preserve original conditions, arithmetic and owner call ordering
			CloudsVector pos = { 2.0f*CoorsOffsetX[i], 2.0f*CoorsOffsetY[i], 40.0f*CoorsOffsetZ[i] + 40.0f };
			worldpos.x = pos.x*rot_cos + pos.y*rot_sin + campos.x;
			worldpos.y = pos.x*rot_sin - pos.y*rot_cos + campos.y;
			worldpos.z = pos.z;

			if(Clouds_BackendProject(worldpos, &screenpos, &szx, &szy, false)){
				// Preserve original conditions, arithmetic and owner call ordering
				sundist = sqrtf(Clouds_Square(screenpos.x-Clouds_BackendSunX()) + Clouds_Square(screenpos.y-Clouds_BackendSunY()));
				int tr = Clouds_BackendFluffyCloudsTopRed();
				int tg = Clouds_BackendFluffyCloudsTopGreen();
				int tb = Clouds_BackendFluffyCloudsTopBlue();
				int br = Clouds_BackendFluffyCloudsBottomRed();
				int bg = Clouds_BackendFluffyCloudsBottomGreen();
				int bb = Clouds_BackendFluffyCloudsBottomBlue();
				int distLimit = (3*Clouds_BackendScreenWidth())/4;
				if(sundist < distLimit){
					// Preserve original conditions, arithmetic and owner call ordering
					hilight = (1.0f - Clouds_Max(Clouds_BackendFoggyness(), Clouds_BackendCloudCoverage())) * (1.0f - sundist/(float)distLimit);
					tr = tr*(1.0f-hilight) + 255*hilight;
					tg = tg*(1.0f-hilight) + 190*hilight;
					tb = tb*(1.0f-hilight) + 190*hilight;
					br = br*(1.0f-hilight) + 255*hilight;
					bg = bg*(1.0f-hilight) + 190*hilight;
					bb = bb*(1.0f-hilight) + 190*hilight;
					if(sundist < Clouds_BackendScreenWidth()/10)
						(*Clouds_BackendSunBlocked()) = true;
				}else
					hilight = 0.0f;
				Clouds_BackendRenderBufferedOneXLUSprite_Rotate_2Colours(screenpos.x, screenpos.y, screenpos.z,
					szx*55.0f, szy*55.0f,
					tr, tg, tb, br, bg, bb, 0.0f, -1.0f,
					1.0f/screenpos.z,
					(uint16_t)Clouds_IndividualRotation/65336.0f * 6.28f + Clouds_ms_cameraRoll,
					fluffyalpha);
				bCloudOnScreen[i] = true;
			}else
				bCloudOnScreen[i] = false;
		}
		Clouds_BackendFlushSprites();

		// Highlights
		Clouds_BackendSrcBlend(true);
		Clouds_BackendDestBlend(true);
		Clouds_BackendSetRaster( Clouds_BackendRaster(gpCloudTex[3]));

		for(i = 0; i < 37; i++){
			// Preserve original conditions, arithmetic and owner call ordering
			CloudsVector pos = { 2.0f*CoorsOffsetX[i], 2.0f*CoorsOffsetY[i], 40.0f*CoorsOffsetZ[i] + 40.0f };
			worldpos.x = pos.x*rot_cos + pos.y*rot_sin + campos.x;
			worldpos.y = pos.x*rot_sin + pos.y*rot_cos + campos.y;
			worldpos.z = pos.z;
			if(bCloudOnScreen[i] && Clouds_BackendProject(worldpos, &screenpos, &szx, &szy, false)){
				// Preserve original conditions, arithmetic and owner call ordering
				if(sundist < Clouds_BackendScreenWidth()/3){
					// Preserve original conditions, arithmetic and owner call ordering
					Clouds_BackendRenderBufferedOneXLUSprite_Rotate_Aspect(screenpos.x, screenpos.y, screenpos.z,
						szx*30.0f, szy*30.0f,
						200*hilight, 0, 0, 255, 1.0f/screenpos.z,
						1.7f - General_GetATanOfXY(screenpos.x-Clouds_BackendSunX(), screenpos.y-Clouds_BackendSunY()) + Clouds_ms_cameraRoll, 255);
				}
			}
		}
		Clouds_BackendFlushSprites();
	}

	// Rainbow
	if(Clouds_BackendRainbow() != 0.0f){
		// Preserve original conditions, arithmetic and owner call ordering
		Clouds_BackendSetRaster( Clouds_BackendRaster(Clouds_BackendCoronaTexture(0)));
		for(i = 0; i < 6; i++){
			// Preserve original conditions, arithmetic and owner call ordering
			CloudsVector pos = { i*1.5f, 100.0f, 5.0f };
			(worldpos=Clouds_Add(campos,pos));
			if(Clouds_BackendProject(worldpos, &screenpos, &szx, &szy, false))
				Clouds_BackendRenderBufferedOneXLUSprite(screenpos.x, screenpos.y, screenpos.z,
					2.0f*szx, 50.0*szy,
					BowRed[i]*Clouds_BackendRainbow(), BowGreen[i]*Clouds_BackendRainbow(), BowBlue[i]*Clouds_BackendRainbow(),
					255, 1.0f/screenpos.z, 255);

		}
		Clouds_BackendFlushSprites();
	}

	Clouds_BackendVertexAlpha(false);
	Clouds_BackendZWrite(true);
	Clouds_BackendZTest(true);
	Clouds_BackendSrcBlend(false);
	Clouds_BackendDestBlend(false);

	Clouds_BackendPop();
}

//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
static void
StoreEnvMapCloudQuad(const CloudsVector *center, const CloudsVector *cameraRight, const CloudsVector *cameraUp,
	float halfWidth, float halfHeight, float rotation,
	const CloudsColor *top, const CloudsColor *bottom)
{
    // Preserve the complete original effect operation and live state-write order
	// Rotate the camera-facing basis to match the cloud sprite orientation
	float cosine = cosf(rotation);
	float sine = sinf(rotation);
	CloudsVector horizontal = Clouds_Add(Clouds_Scale(*cameraRight,halfWidth*cosine),Clouds_Scale(*cameraUp,halfWidth*sine));
	CloudsVector vertical = Clouds_Subtract(Clouds_Scale(*cameraUp,halfHeight*cosine),Clouds_Scale(*cameraRight,halfHeight*sine));
	CloudsVector corners[4] = {
		// Preserve original conditions, arithmetic and owner call ordering
		Clouds_Add(Clouds_Subtract(*center,horizontal),vertical),
		Clouds_Add(Clouds_Add(*center,horizontal),vertical),
		Clouds_Subtract(Clouds_Subtract(*center,horizontal),vertical),
		Clouds_Subtract(Clouds_Add(*center,horizontal),vertical)
	};
	uint16_t *indices;
	void *vertices;
	// Reserve one textured quad in the shared RenderWare immediate buffer
	// Enter the C shared-buffer algorithm with the original arguments
	RenderBuffer_StartStoring(6, 4, &indices, &vertices);
	for(int vertex = 0; vertex < 4; vertex++){
		// Preserve original conditions, arithmetic and owner call ordering
		Clouds_BackendSetPos(vertices,vertex, corners[vertex].x, corners[vertex].y, corners[vertex].z);
		const CloudsColor *color = vertex < 2 ? top : bottom;
		Clouds_BackendSetRGBA(vertices,vertex, color->r, color->g, color->b, color->a);
	}
	Clouds_BackendSetU(vertices,0, 0.0f); Clouds_BackendSetV(vertices,0, 0.0f);
	Clouds_BackendSetU(vertices,1, 1.0f); Clouds_BackendSetV(vertices,1, 0.0f);
	Clouds_BackendSetU(vertices,2, 0.0f); Clouds_BackendSetV(vertices,2, 1.0f);
	Clouds_BackendSetU(vertices,3, 1.0f); Clouds_BackendSetV(vertices,3, 1.0f);
	indices[0] = 0; indices[1] = 1; indices[2] = 2;
	indices[3] = 2; indices[4] = 1; indices[5] = 3;
	// Enter the C shared-buffer algorithm with the original arguments
	RenderBuffer_StopStoring();
}

void
Clouds_RenderForEnvMap(void *camera)
{
    // Preserve the complete original effect operation and live state-write order
	// Skip sky effects without a valid reflection camera or when the current area is indoors
	if(camera == NULL || !Clouds_BackendOutside())
		return;
	void *cameraFrame = Clouds_BackendCameraFrame(camera);
	// Skip rendering if the reflection camera has no frame matrix
	if(cameraFrame == NULL || Clouds_BackendFrameMatrix(cameraFrame) == NULL)
		return;

	// Use the reflection camera basis without touching the main camera's cloud state
	void *cameraMatrix = Clouds_BackendFrameMatrix(cameraFrame);
	CloudsVector cameraPos=Clouds_BackendMatrixPosition(cameraMatrix);
	CloudsVector cameraRight=Clouds_BackendMatrixRight(cameraMatrix);
	CloudsVector cameraUp=Clouds_BackendMatrixUp(cameraMatrix);
	CloudsVector cameraForward=Clouds_BackendMatrixForward(cameraMatrix);
	Clouds_Normalise(&cameraRight);
	Clouds_Normalise(&cameraUp);
	Clouds_Normalise(&cameraForward);
	float nearClip = Clouds_BackendNearClip(camera);
	float farClip = Clouds_BackendFarClip(camera);

	// Preserve pending world geometry before starting the sky effect batches
	// Enter the C shared-buffer algorithm with the original arguments
	RenderBuffer_RenderStuffInBuffer();
	Clouds_BackendPush("CClouds::RenderForEnvMap");
	Clouds_BackendFog(false);
	Clouds_BackendZWrite(false);
	Clouds_BackendZTest(false);
	Clouds_BackendVertexAlpha(true);
	Clouds_BackendCull(true);
	Clouds_BackendSrcBlend(true);
	Clouds_BackendDestBlend(true);
	// Enter the C shared-buffer algorithm with the original arguments
	RenderBuffer_ClearRenderBuffer();

	// Place the moon ahead of the upward-facing reflection camera during its night window
	float minute = Clock_GetHours()*60.0f + Clock_GetMinutes() + Clock_GetSeconds()/60.0f;
	float moonFade = fabsf(minute - 180.0f);
	if((int)moonFade < 180 && Clouds_BackendCoronaTexture(2)){
		// Preserve original conditions, arithmetic and owner call ordering
		float coverage = Clouds_Max(Clouds_BackendFoggyness(), Clouds_BackendCloudCoverage());
		int brightness = (int)((1.0f-coverage)*(180-(int)moonFade));
		if(brightness > 0){
			// Preserve original conditions, arithmetic and owner call ordering
			CloudsVector moonPosition = Clouds_Add(Clouds_Add(cameraPos,Clouds_Scale(cameraForward,190.0f)),Clouds_Scale(cameraUp,45.0f));
			float depth = Clouds_Dot(Clouds_Subtract(moonPosition,cameraPos), cameraForward);
			if(depth > nearClip && depth < farClip){
				// Preserve original conditions, arithmetic and owner call ordering
				Clouds_BackendSetRaster( Clouds_BackendRaster(Clouds_BackendCoronaTexture(2)));
				CloudsColor moonColor=Clouds_Color(brightness, brightness, brightness, 255);
				float moonSize = Clouds_BackendMoonSize()*2.0f + 4.0f;
				StoreEnvMapCloudQuad(&moonPosition,&cameraRight,&cameraUp, moonSize, moonSize, 0.0f,&moonColor,&moonColor);
				// Enter the C shared-buffer algorithm with the original arguments
				RenderBuffer_RenderStuffInBuffer();
			}
		}
	}

	// Draw stable stars from the existing sky layout without invoking random flicker
	int starIntensity = 0;
	if(Clock_GetHours() < 5 || Clock_GetHours() > 22)
		starIntensity = 255;
	else if(Clock_GetHours() == 22)
		starIntensity = 255*Clock_GetMinutes()/60;
	else if(Clock_GetHours() == 5)
		starIntensity = 255*(60-Clock_GetMinutes())/60;
	starIntensity *= 1.0f-Clouds_Max(Clouds_BackendFoggyness(), Clouds_BackendCloudCoverage());
	if(starIntensity > 0 && Clouds_BackendCoronaTexture(0)){
		// Preserve original conditions, arithmetic and owner call ordering
		Clouds_BackendSetRaster( Clouds_BackendRaster(Clouds_BackendCoronaTexture(0)));
		for(int i = 0; i < 11; i++){
			// Preserve original conditions, arithmetic and owner call ordering
			int star = i%9;
			float horizontal = (StarCoorsX[star]-0.375f)*180.0f;
			float vertical = (StarCoorsY[star]-0.35f)*120.0f;
			CloudsVector starPosition = Clouds_Add(Clouds_Add(Clouds_Add(cameraPos,Clouds_Scale(cameraForward,190.0f)),Clouds_Scale(cameraRight,horizontal)),Clouds_Scale(cameraUp,vertical));
			float depth = Clouds_Dot(Clouds_Subtract(starPosition,cameraPos), cameraForward);
			if(depth <= nearClip || depth >= farClip)
				continue;
			float size = 0.8f*StarSizes[star];
			CloudsColor starColor=Clouds_Color(starIntensity, starIntensity, starIntensity, 255);
			StoreEnvMapCloudQuad(&starPosition,&cameraRight,&cameraUp, size, size, 0.0f,&starColor,&starColor);
		}
		// Add a slow, deterministic pulse to the bright R-star in the reflection sky
		//+ rouz edit (ChatGPT)
		const float starPulse = 0.7f + 0.3f*sinf(((*Timer_GetTimeInMilliseconds()) & 4095)*TWOPI/4096.0f);
		CloudsVector brightStarPosition = Clouds_Add(Clouds_Add(Clouds_Add(cameraPos,Clouds_Scale(cameraForward,190.0f)),Clouds_Scale(cameraRight,95.0f)),Clouds_Scale(cameraUp,18.0f));
		float brightStarDepth = Clouds_Dot(Clouds_Subtract(brightStarPosition,cameraPos), cameraForward);
		if(brightStarDepth > nearClip && brightStarDepth < farClip){
			// Preserve original conditions, arithmetic and owner call ordering
			int brightStarIntensity = (int)(starIntensity*starPulse);
			CloudsColor brightStarColor=Clouds_Color(brightStarIntensity, brightStarIntensity, brightStarIntensity, 255);
			StoreEnvMapCloudQuad(&brightStarPosition,&cameraRight,&cameraUp, 5.0f, 5.0f,
				0.0f,&brightStarColor,&brightStarColor);
		}
		//- rouz edit (ChatGPT)
		// Enter the C shared-buffer algorithm with the original arguments
		RenderBuffer_RenderStuffInBuffer();
	}

	// Render the three animated low-cloud texture layers around the reflection viewpoint
	float lowCloudIntensity = 1.0f-Clouds_Max(Clouds_Max(Clouds_BackendFoggyness(), Clouds_BackendCloudCoverage()), Clouds_BackendExtraSunnyness());
	if(lowCloudIntensity > 0.0f){
		// Preserve original conditions, arithmetic and owner call ordering
		int red = Clouds_BackendLowCloudsRed()*lowCloudIntensity;
		int green = Clouds_BackendLowCloudsGreen()*lowCloudIntensity;
		int blue = Clouds_BackendLowCloudsBlue()*lowCloudIntensity;
		for(int cloudType = 0; cloudType < 3; cloudType++){
			// Preserve original conditions, arithmetic and owner call ordering
			if(gpCloudTex[cloudType] == NULL)
				continue;
			Clouds_BackendSetRaster( Clouds_BackendRaster(gpCloudTex[cloudType]));
			for(int i = cloudType; i < 12; i += 3){
				// Preserve original conditions, arithmetic and owner call ordering
				CloudsVector cloudPosition={cameraPos.x + 800.0f*LowCloudsX[i],
					cameraPos.y + 800.0f*LowCloudsY[i], 40.0f + 60.0f*LowCloudsZ[i]};
				float depth = Clouds_Dot(Clouds_Subtract(cloudPosition,cameraPos), cameraForward);
				if(depth <= nearClip || depth >= farClip)
					continue;
				CloudsColor cloudColor=Clouds_Color(red, green, blue, 255);
				StoreEnvMapCloudQuad(&cloudPosition,&cameraRight,&cameraUp, 320.0f, 40.0f, 0.0f,&cloudColor,&cloudColor);
			}
			// Enter the C shared-buffer algorithm with the original arguments
			RenderBuffer_RenderStuffInBuffer();
		}
	}

	// Draw the soft cloud layer with its timecycle gradient and existing wind rotation
	int fluffyAlpha = 160*(1.0f-Clouds_Max(Clouds_BackendFoggyness(), Clouds_BackendExtraSunnyness()));
	if(fluffyAlpha > 0 && gpCloudTex[4]){
		// Preserve original conditions, arithmetic and owner call ordering
		Clouds_BackendSrcBlend(false);
		Clouds_BackendDestBlend(false);
		Clouds_BackendSetRaster( Clouds_BackendRaster(gpCloudTex[4]));
		float rotationSine = sinf(Clouds_CloudRotation);
		float rotationCosine = cosf(Clouds_CloudRotation);
		float spriteRotation = (uint16_t)Clouds_IndividualRotation/65336.0f*TWOPI;
		CloudsColor cloudTop=Clouds_Color(Clouds_BackendFluffyCloudsTopRed(), Clouds_BackendFluffyCloudsTopGreen(),
			Clouds_BackendFluffyCloudsTopBlue(), fluffyAlpha);
		CloudsColor cloudBottom=Clouds_Color(Clouds_BackendFluffyCloudsBottomRed(), Clouds_BackendFluffyCloudsBottomGreen(),
			Clouds_BackendFluffyCloudsBottomBlue(), fluffyAlpha);
		for(int i = 0; i < 37; i++){
			// Preserve original conditions, arithmetic and owner call ordering
			float x = 2.0f*CoorsOffsetX[i];
			float y = 2.0f*CoorsOffsetY[i];
			CloudsVector cloudPosition={cameraPos.x + x*rotationCosine + y*rotationSine,
				cameraPos.y + x*rotationSine - y*rotationCosine,
				40.0f*CoorsOffsetZ[i] + 40.0f};
			float depth = Clouds_Dot(Clouds_Subtract(cloudPosition,cameraPos), cameraForward);
			if(depth <= nearClip || depth >= farClip)
				continue;
			StoreEnvMapCloudQuad(&cloudPosition,&cameraRight,&cameraUp, 55.0f, 55.0f,
				spriteRotation,&cloudTop,&cloudBottom);
		}
		// Enter the C shared-buffer algorithm with the original arguments
		RenderBuffer_RenderStuffInBuffer();
	}

	// Add a camera-relative highlight around the sun without reading main-camera screen coordinates
	CloudsVector sunDirection = Clouds_BackendSunDirection();
	Clouds_Normalise(&sunDirection);
	float sunDepth = Clouds_Dot(sunDirection, cameraForward);
	if(gpCloudTex[3] && sunDepth > 0.05f){
		// Preserve original conditions, arithmetic and owner call ordering
		Clouds_BackendSrcBlend(true);
		Clouds_BackendDestBlend(true);
		Clouds_BackendSetRaster( Clouds_BackendRaster(gpCloudTex[3]));
		float sunScreenX = Clouds_Dot(sunDirection, cameraRight)/sunDepth;
		float sunScreenY = Clouds_Dot(sunDirection, cameraUp)/sunDepth;
		float highlightLimit = 0.55f;
		float cloudRotationSine = sinf(Clouds_CloudRotation);
		float cloudRotationCosine = cosf(Clouds_CloudRotation);
		for(int i = 0; i < 37; i++){
			// Preserve original conditions, arithmetic and owner call ordering
			float x = 2.0f*CoorsOffsetX[i];
			float y = 2.0f*CoorsOffsetY[i];
			CloudsVector cloudPosition={cameraPos.x + x*cloudRotationCosine + y*cloudRotationSine,
				cameraPos.y + x*cloudRotationSine - y*cloudRotationCosine,
				40.0f*CoorsOffsetZ[i] + 40.0f};
			CloudsVector fromCamera = Clouds_Subtract(cloudPosition,cameraPos);
			float depth = Clouds_Dot(fromCamera, cameraForward);
			if(depth <= nearClip || depth >= farClip)
				continue;
			float screenX = Clouds_Dot(fromCamera, cameraRight)/depth;
			float screenY = Clouds_Dot(fromCamera, cameraUp)/depth;
			float deltaX = screenX-sunScreenX;
			float deltaY = screenY-sunScreenY;
			float distance = sqrtf(Clouds_Square(deltaX)+Clouds_Square(deltaY));
			if(distance >= highlightLimit)
				continue;
			float highlight = (1.0f-Clouds_Max(Clouds_BackendFoggyness(), Clouds_BackendCloudCoverage()))*(1.0f-distance/highlightLimit);
			int red = 200.0f*highlight;
			if(red <= 0)
				continue;
			CloudsColor highlightColor=Clouds_Color(red, 0, 0, 255);
			StoreEnvMapCloudQuad(&cloudPosition,&cameraRight,&cameraUp, 30.0f, 30.0f,
				1.7f-atan2f(deltaY, deltaX),&highlightColor,&highlightColor);
		}
		// Enter the C shared-buffer algorithm with the original arguments
		RenderBuffer_RenderStuffInBuffer();
	}

	// Add the weather rainbow as six camera-aligned colored streaks
	if(Clouds_BackendRainbow() != 0.0f && Clouds_BackendCoronaTexture(0)){
		// Preserve original conditions, arithmetic and owner call ordering
		Clouds_BackendSrcBlend(true);
		Clouds_BackendDestBlend(true);
		Clouds_BackendSetRaster( Clouds_BackendRaster(Clouds_BackendCoronaTexture(0)));
		for(int i = 0; i < 6; i++){
			// Preserve original conditions, arithmetic and owner call ordering
			CloudsVector rainbowPosition = Clouds_Add(Clouds_Add(Clouds_Add(cameraPos,Clouds_Scale(cameraForward,190.0f)),Clouds_Scale(cameraUp,35.0f)),Clouds_Scale(cameraRight,i*1.5f-3.75f));
			CloudsColor rainbowColor=Clouds_Color(BowRed[i]*Clouds_BackendRainbow(), BowGreen[i]*Clouds_BackendRainbow(),
				BowBlue[i]*Clouds_BackendRainbow(), 255);
			StoreEnvMapCloudQuad(&rainbowPosition,&cameraRight,&cameraUp, 2.0f, 50.0f,
				0.0f,&rainbowColor,&rainbowColor);
		}
		// Enter the C shared-buffer algorithm with the original arguments
		RenderBuffer_RenderStuffInBuffer();
	}

	// Restore the standard world pass states after drawing the sky
	Clouds_BackendZWrite(true);
	Clouds_BackendZTest(true);
	Clouds_BackendVertexAlpha(false);
	Clouds_BackendFog(false);
	Clouds_BackendSrcBlend(false);
	Clouds_BackendDestBlend(false);
	Clouds_BackendSetRaster( NULL);
	Clouds_BackendCull(false);
	Clouds_BackendPop();
}
#endif
//- rouz edit (ChatGPT)


#define SMALLSTRIPHEIGHT 4.0f
#define HORIZSTRIPHEIGHT 48.0f
#define SCREEN_WIDTH (Clouds_BackendScreenWidth())
#define SCREEN_HEIGHT (Clouds_BackendScreenHeight())
#define SCREEN_STRETCH_Y(a) ((a)*(float)SCREEN_HEIGHT/DEFAULT_SCREEN_HEIGHT)
static CloudsVector Clouds_Cross(CloudsVector a,CloudsVector b)
{
    // Preserve the original component cross-product expressions
    CloudsVector v={a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};return v;
}
static float Clouds_Clamp(float value,float low,float high)
{
    // Preserve original comparison order including equal and unordered operands
    return value<low?low:value>high?high:value;
}
static CloudsVector Clouds_TransformView(CloudsViewMatrix m,CloudsVector v)
{
    // Retain the exact original matrix-vector product and translation sum order
    CloudsVector result={m.rx*v.x+m.fx*v.y+m.ux*v.z+m.px,m.ry*v.x+m.fy*v.y+m.uy*v.z+m.py,m.rz*v.x+m.fz*v.y+m.uz*v.z+m.pz};return result;
}
static CloudsColor Clouds_ReadBackgroundColor(int32_t index)
{
    // Read actual color bytes only where the original copied that color value
    CloudsColorFields f=Clouds_BackendColorFields(index);CloudsColor c={*f.r,*f.g,*f.b,*f.a};return c;
}
static void Clouds_WriteBackgroundColor(int32_t index,CloudsColor c)
{
    // Write the original color components in their original assignment order
    CloudsColorFields f=Clouds_BackendColorFields(index);*f.r=c.r;*f.g=c.g;*f.b=c.b;*f.a=c.a;
}
static float HorizonX;
static float HorizonSkyX;
static float HorizonSkyY;
static float HorizonLineX;
static float HorizonLineY;
typedef struct HorizonState
{
	float x;
	float z;
	float skyX;
	float skyY;
	float lineX;
	float lineY;
} HorizonState;
static HorizonState LastSafeHorizon;
static bool HaveLastSafeHorizon;

static bool
UseDarkBackground(void)
{
    // Preserve the complete original horizon operation, live state and assignment order
	return Clouds_BackendForwardAxis(2) < -0.9f || Clouds_BackendCollisionPolys();
}

//+ rouz edit (ChatGPT)
static bool
UseSkyTopBackground(void)
{
    // Preserve the complete original horizon operation, live state and assignment order
	return Clouds_BackendForwardAxis(2) > 0.98f;
}

static HorizonState
GetCurrentHorizonState(void)
{
    // Preserve the complete original horizon operation, live state and assignment order
	HorizonState state;
	state.x = HorizonX;
	state.z = Clouds_ms_horizonZ;
	state.skyX = HorizonSkyX;
	state.skyY = HorizonSkyY;
	state.lineX = HorizonLineX;
	state.lineY = HorizonLineY;
	return state;
}

static void
SetCurrentHorizonState(HorizonState state)
{
    // Preserve the complete original horizon operation, live state and assignment order
	HorizonX = state.x;
	Clouds_ms_horizonZ = state.z;
	HorizonSkyX = state.skyX;
	HorizonSkyY = state.skyY;
	HorizonLineX = state.lineX;
	HorizonLineY = state.lineY;
}

static HorizonState
GetMirroredPoleHorizonState(HorizonState state)
{
    // Preserve the complete original horizon operation, live state and assignment order
	HorizonState mirrored = state;
	mirrored.x = SCREEN_WIDTH - state.x;
	mirrored.z = SCREEN_HEIGHT - state.z;
	mirrored.skyX = SCREEN_WIDTH - state.skyX;
	mirrored.skyY = SCREEN_HEIGHT - state.skyY;
	mirrored.lineX = -state.lineX;
	mirrored.lineY = -state.lineY;
	return mirrored;
}

static void CalcScreenCoorsNoClip(CloudsVector in,float *x,float *y)
{
    // Preserve the complete original horizon operation, live state and assignment order
	CloudsVector view=Clouds_TransformView(Clouds_BackendViewMatrix(),in);
	if(fabsf(view.z) < 0.001f)
		view.z = view.z < 0.0f ? -0.001f : 0.001f;
	*x = view.x * SCREEN_WIDTH / view.z;
	*y = view.y * SCREEN_HEIGHT / view.z;
	//+ rouz edit (ChatGPT)
	// Match the software framebuffer's top-down screen Y used by projected world sprites
#ifdef REVC_SOFTWARE_POLYGONS
	if(Clouds_BackendCapturingWorldEffects())
		*y = SCREEN_HEIGHT - *y;
#endif
	//- rouz edit (ChatGPT)
}

static void UpdateHorizonCoors(void)
{
    // Preserve the complete original horizon operation, live state and assignment order
	CloudsVector forward={Clouds_BackendForwardAxis(0),Clouds_BackendForwardAxis(1),0.0f};
	if(Clouds_Dot(forward,forward) < 0.0001f){
		// Preserve original branch conditions and live data reads
		HorizonX = SCREEN_WIDTH/2.0f;
		HorizonSkyX = SCREEN_WIDTH/2.0f;
		HorizonSkyY = SCREEN_HEIGHT/2.0f - 1.0f;
		Clouds_ms_horizonZ = SCREEN_HEIGHT/2.0f;
		HorizonLineX = cosf(Clouds_ms_cameraRoll);
		HorizonLineY = -sinf(Clouds_ms_cameraRoll);
		return;
	}
	Clouds_Normalise(&forward);

	CloudsVector right = Clouds_Cross((CloudsVector){0.f,0.f,1.f},forward);
	Clouds_Normalise(&right);

	CloudsVector center = Clouds_Add(Clouds_BackendCameraPosition(),Clouds_Scale(forward,3000.0f));
	center.z = 0.0f;
	CloudsVector left = Clouds_Subtract(center,Clouds_Scale(right,3000.0f));
	CloudsVector rightPos = Clouds_Add(center,Clouds_Scale(right,3000.0f));
	CloudsVector sky = center;
	sky.z += 1.0f;

	CalcScreenCoorsNoClip(center,&HorizonX,&Clouds_ms_horizonZ);
	CalcScreenCoorsNoClip(sky,&HorizonSkyX,&HorizonSkyY);
	float leftX, leftY, rightX, rightY;
	CalcScreenCoorsNoClip(left,&leftX,&leftY);
	CalcScreenCoorsNoClip(rightPos,&rightX,&rightY);

	HorizonLineX = rightX - leftX;
	HorizonLineY = rightY - leftY;
	float len = sqrtf(Clouds_Square(HorizonLineX) + Clouds_Square(HorizonLineY));
	if(len > 0.001f){
		// Preserve original branch conditions and live data reads
		HorizonLineX /= len;
		HorizonLineY /= len;
	}else{
		// Preserve original branch conditions and live data reads
		HorizonLineX = cosf(Clouds_ms_cameraRoll);
		HorizonLineY = -sinf(Clouds_ms_cameraRoll);
	}
}

static void DrawHorizonBand(float centerY, float topOffset, float bottomOffset, int32_t top,int32_t bottom)
{
    // Preserve the complete original horizon operation, live state and assignment order
	float halfWidth = 2.0f*sqrtf(Clouds_Square(SCREEN_WIDTH) + Clouds_Square(SCREEN_HEIGHT));
	float lineX = HorizonLineX;
	float lineY = HorizonLineY;
	float downX = -lineY;
	float downY = lineX;
	float skySide = (HorizonSkyX - HorizonX)*downX + (HorizonSkyY - Clouds_ms_horizonZ)*downY;
	if(skySide > 0.0f){
		// Preserve original branch conditions and live data reads
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
	Clouds_BackendSrcBlend(false);
	Clouds_BackendDestBlend(false);
	//- rouz edit (ChatGPT)
	Clouds_BackendDrawBackgroundQuad(topLeftX, topLeftY, topRightX, topRightY,
		bottomLeftX, bottomLeftY, bottomRightX, bottomRightY,
		top,bottom);
}

//+ rouz edit (ChatGPT)
static float GetHorizonScreenDepth(void)
{
    // Preserve the complete original horizon operation, live state and assignment order
	float downX = -HorizonLineY;
	float downY = HorizonLineX;
	float skySide = (HorizonSkyX - HorizonX)*downX + (HorizonSkyY - Clouds_ms_horizonZ)*downY;
	if(skySide > 0.0f){
		// Preserve original branch conditions and live data reads
		downX = -downX;
		downY = -downY;
	}

	float d0 = fabsf((0.0f - HorizonX)*downX + (0.0f - Clouds_ms_horizonZ)*downY);
	float d1 = fabsf((SCREEN_WIDTH - HorizonX)*downX + (0.0f - Clouds_ms_horizonZ)*downY);
	float d2 = fabsf((0.0f - HorizonX)*downX + (SCREEN_HEIGHT - Clouds_ms_horizonZ)*downY);
	float d3 = fabsf((SCREEN_WIDTH - HorizonX)*downX + (SCREEN_HEIGHT - Clouds_ms_horizonZ)*downY);
	return Clouds_Max(Clouds_Max(d0, d1), Clouds_Max(d2, d3)) + SCREEN_HEIGHT;
}

static CloudsColor
ScaleRGBAAlpha(CloudsColor colour,float scale)
{
    // Preserve the complete original horizon operation, live state and assignment order
	scale = Clouds_Clamp(scale, 0.0f, 1.0f);
	return Clouds_Color(colour.r, colour.g, colour.b, (uint8_t)(Clouds_Clamp(colour.a*scale, 0.0f, 255.0f)));
}

static void
DrawBackgroundGradientBands(int16_t topred, int16_t topgreen, int16_t topblue,
	int16_t botred, int16_t botgreen, int16_t botblue, int16_t alpha, float alphaScale)
{
    // Preserve the complete original horizon operation, live state and assignment order
	int fogr = (topred + 2 * botred) / 3;
	int fogg = (topgreen + 2 * botgreen) / 3;
	int fogb = (topblue + 2 * botblue) / 3;
	float skyDepth = GetHorizonScreenDepth();

	Clouds_WriteBackgroundColor(CLOUDS_COLOR_TOP,ScaleRGBAAlpha(Clouds_Color(topred, topgreen, topblue, alpha), alphaScale));
	Clouds_WriteBackgroundColor(CLOUDS_COLOR_BOTTOM,ScaleRGBAAlpha(Clouds_Color(botred, botgreen, botblue, alpha), alphaScale));
	DrawHorizonBand(Clouds_ms_horizonZ, -skyDepth, 0.0f, CLOUDS_COLOR_TOP, CLOUDS_COLOR_BOTTOM);

	Clouds_WriteBackgroundColor(CLOUDS_COLOR_TOP,ScaleRGBAAlpha(Clouds_Color(fogr, fogg, fogb, alpha), alphaScale));
	DrawHorizonBand(Clouds_ms_horizonZ, 0.0f, SMALLSTRIPHEIGHT, CLOUDS_COLOR_TOP, CLOUDS_COLOR_TOP);

	Clouds_WriteBackgroundColor(CLOUDS_COLOR_TOP,Clouds_Color(fogr, fogg, fogb, alpha));
	Clouds_WriteBackgroundColor(CLOUDS_COLOR_BOTTOM,Clouds_ReadBackgroundColor(CLOUDS_COLOR_TOP));
}
//- rouz edit (ChatGPT)

void
Clouds_RenderBackground(int16_t topred, int16_t topgreen, int16_t topblue,
	int16_t botred, int16_t botgreen, int16_t botblue, int16_t alpha)
{
    // Preserve the complete original horizon operation, live state and assignment order
	Clouds_BackendPush("CClouds::RenderBackground");

	//+ rouz edit (ChatGPT)
	CloudsVector camRight = Clouds_Cross(Clouds_BackendCameraUp(),Clouds_BackendCameraForward());
	Clouds_Normalise(&camRight);
	CloudsVector levelRight = Clouds_Cross((CloudsVector){0.f,0.f,1.f},Clouds_BackendCameraForward());
	if(Clouds_Dot(levelRight,levelRight) > 0.0001f){
		// Preserve original branch conditions and live data reads
		Clouds_Normalise(&levelRight);
		Clouds_ms_cameraRoll = atan2f(camRight.z, Clouds_Dot(camRight, levelRight));
	}else
		Clouds_ms_cameraRoll = 0.0f;
	Clouds_ms_HorizonTilt = 0.0f;
	UpdateHorizonCoors();
	//- rouz edit (ChatGPT)

	if(UseDarkBackground()){
		// Preserve original branch conditions and live data reads
		(*Clouds_BackendColorFields(CLOUDS_COLOR_TOP).r) = 50;
		(*Clouds_BackendColorFields(CLOUDS_COLOR_TOP).g) = 50;
		(*Clouds_BackendColorFields(CLOUDS_COLOR_TOP).b) = 50;
		(*Clouds_BackendColorFields(CLOUDS_COLOR_TOP).a) = 255;
		if(Clouds_BackendCollisionPolys()){
			// Preserve original branch conditions and live data reads
			if((*Timer_GetFrameCounter()) & 1){
				// Preserve original branch conditions and live data reads
				(*Clouds_BackendColorFields(CLOUDS_COLOR_TOP).r) = 0;
				(*Clouds_BackendColorFields(CLOUDS_COLOR_TOP).g) = 0;
				(*Clouds_BackendColorFields(CLOUDS_COLOR_TOP).b) = 0;
			}else{
				// Preserve original branch conditions and live data reads
				(*Clouds_BackendColorFields(CLOUDS_COLOR_TOP).r) = 255;
				(*Clouds_BackendColorFields(CLOUDS_COLOR_TOP).g) = 255;
				(*Clouds_BackendColorFields(CLOUDS_COLOR_TOP).b) = 255;
			}
		}
		Clouds_WriteBackgroundColor(CLOUDS_COLOR_BOTTOM,Clouds_ReadBackgroundColor(CLOUDS_COLOR_TOP));
		float rectRight=SCREEN_WIDTH;float rectBottom=SCREEN_HEIGHT;
		Clouds_BackendDrawBackgroundRect(0.f,0.f,rectRight,rectBottom,CLOUDS_COLOR_BOTTOM,CLOUDS_COLOR_TOP);
	//+ rouz edit (ChatGPT)
	}else if(UseSkyTopBackground()){
		// Preserve original branch conditions and live data reads
		HorizonState currentHorizon = GetCurrentHorizonState();
		if(!HaveLastSafeHorizon){
			// Preserve original branch conditions and live data reads
			LastSafeHorizon = currentHorizon;
			HaveLastSafeHorizon = true;
		}

		float t = Clouds_Clamp((Clouds_BackendForwardAxis(2) - 0.98f)/(1.0f - 0.98f), 0.0f, 1.0f);
		t = t*t*(3.0f - 2.0f*t);

		SetCurrentHorizonState(LastSafeHorizon);
		DrawBackgroundGradientBands(topred, topgreen, topblue, botred, botgreen, botblue, alpha, 1.0f);

		SetCurrentHorizonState(GetMirroredPoleHorizonState(LastSafeHorizon));
		DrawBackgroundGradientBands(topred, topgreen, topblue, botred, botgreen, botblue, alpha, t);

		SetCurrentHorizonState(currentHorizon);
	}else{
		// Preserve original branch conditions and live data reads
		LastSafeHorizon = GetCurrentHorizonState();
		HaveLastSafeHorizon = true;
		DrawBackgroundGradientBands(topred, topgreen, topblue, botred, botgreen, botblue, alpha, 1.0f);
	}
	//- rouz edit (ChatGPT)

	Clouds_BackendPop();
}

void
Clouds_RenderHorizon(void)
{
    // Preserve the complete original horizon operation, live state and assignment order
	// rouz edit (ChatGPT)
	if(UseDarkBackground() || UseSkyTopBackground())
		return;

	Clouds_BackendPush("CClouds::RenderHorizon");

	(*Clouds_BackendColorFields(CLOUDS_COLOR_BOTTOM).a) = 230;
	(*Clouds_BackendColorFields(CLOUDS_COLOR_TOP).a) = 80;

	// rouz edit (ChatGPT)
	DrawHorizonBand(Clouds_ms_horizonZ, 0.0f, SMALLSTRIPHEIGHT, CLOUDS_COLOR_TOP, CLOUDS_COLOR_BOTTOM);

	(*Clouds_BackendColorFields(CLOUDS_COLOR_BACKGROUND).r) = 128.0f*Clouds_BackendAmbientRed();
	(*Clouds_BackendColorFields(CLOUDS_COLOR_BACKGROUND).g) = 128.0f*Clouds_BackendAmbientGreen();
	(*Clouds_BackendColorFields(CLOUDS_COLOR_BACKGROUND).b) = 128.0f*Clouds_BackendAmbientBlue();
	(*Clouds_BackendColorFields(CLOUDS_COLOR_BACKGROUND).a) = 255;

	float horzstrip = SCREEN_STRETCH_Y(HORIZSTRIPHEIGHT);
	//+ rouz edit (ChatGPT)
	float skyDepth = GetHorizonScreenDepth();

	DrawHorizonBand(Clouds_ms_horizonZ, SMALLSTRIPHEIGHT, SMALLSTRIPHEIGHT + horzstrip,
		CLOUDS_COLOR_BOTTOM, CLOUDS_COLOR_BACKGROUND);
	DrawHorizonBand(Clouds_ms_horizonZ, SMALLSTRIPHEIGHT + horzstrip, skyDepth,
		CLOUDS_COLOR_BACKGROUND, CLOUDS_COLOR_BACKGROUND);
	//- rouz edit

	Clouds_BackendPop();
}
//- rouz edit (ChatGPT)
