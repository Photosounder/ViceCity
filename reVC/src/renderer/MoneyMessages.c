//+ rouz edit (ChatGPT)
#include "MoneyMessages_backend.h"
#include "Timer.h"
#include "Draw.h"
#include "Draw_constants.h"
#include "FontStyles.h"

#define SCREEN_WIDTH (MoneyMessages_BackendScreenWidth())
#define SCREEN_HEIGHT (MoneyMessages_BackendScreenHeight())
#define SCREEN_ASPECT_RATIO (Draw_GetAspectRatio())
#define SCREEN_STRETCH_X(a)   ((a) * (float) SCREEN_WIDTH / DEFAULT_SCREEN_WIDTH)
#define SCREEN_STRETCH_Y(a)   ((a) * (float) SCREEN_HEIGHT / DEFAULT_SCREEN_HEIGHT)
#define SCREEN_STRETCH_FROM_RIGHT(a)  (SCREEN_WIDTH - SCREEN_STRETCH_X(a))
#define SCREEN_STRETCH_FROM_BOTTOM(a) (SCREEN_HEIGHT - SCREEN_STRETCH_Y(a))

// This scales from PS2 pixel coordinates while optionally maintaining the aspect ratio
#define SCREEN_SCALE_X(a) SCREEN_SCALE_AR(SCREEN_STRETCH_X(a))
#define SCREEN_SCALE_Y(a) SCREEN_STRETCH_Y(a)
#define SCREEN_SCALE_FROM_RIGHT(a) (SCREEN_WIDTH - SCREEN_SCALE_X(a))
#define SCREEN_SCALE_FROM_BOTTOM(a) (SCREEN_HEIGHT - SCREEN_SCALE_Y(a))

#ifdef ASPECT_RATIO_SCALE
#define SCREEN_SCALE_AR(a) ((a) * DEFAULT_ASPECT_RATIO / SCREEN_ASPECT_RATIO)
#define SCALE_AND_CENTER_X(x) ((SCREEN_WIDTH == DEFAULT_SCREEN_WIDTH) ? (x) : (SCREEN_WIDTH - SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH)) / 2 + SCREEN_SCALE_X((x)))
#ifdef PROPER_SCALING
	#ifndef FORCE_PC_SCALING			
		#undef SCREEN_SCALE_Y	
		#define SCREEN_SCALE_Y(a) Draw_ScaleY(SCREEN_STRETCH_Y(a))	
	#endif
#endif
#else
#define SCREEN_SCALE_AR(a) (a)
#define SCALE_AND_CENTER_X(x) SCREEN_STRETCH_X(x)
#endif

#define MONEY_MESSAGES_MIN(a,b) ((a)<(b)?(a):(b))
#ifdef MORE_LANGUAGES
#define MONEY_MESSAGES_FONT_LOCALE(style) (MoneyMessages_BackendJapanese()?FONT_JAPANESE:(style))
#else
#define MONEY_MESSAGES_FONT_LOCALE(style) (style)
#endif
static MoneyMessageVector MoneyMessages_Rise(MoneyMessageVector position,float height)
{
    // Preserve all three additions including the original signed-zero behavior
    MoneyMessageVector result={position.x+0.0f,position.y+0.0f,position.z+height};return result;
}
#define MONEY_MESSAGE_LIFETIME_MS 2000

static MoneyMessage aMoneyMessages[NUMMONEYMESSAGES];

void
MoneyMessage_Render(MoneyMessage *message)
{
	// Retain the complete original money-message operation and write order
	const float MAX_SCALE = 4.0f;
	uint32_t nLifeTime = (*Timer_GetTimeInMilliseconds()) - message->m_nTimeRegistered;
	if (nLifeTime >= MONEY_MESSAGE_LIFETIME_MS) {
		// Preserve original conditions and live owner reads
		message->m_nTimeRegistered = 0;
	} else {
		// Preserve original conditions and live owner reads
		float fLifeTime = (float)nLifeTime / MONEY_MESSAGE_LIFETIME_MS;
		MoneyMessageVector vecOut;
		float fDistX, fDistY;
		if (MoneyMessages_BackendProject(MoneyMessages_Rise(message->m_vecPosition,fLifeTime), &vecOut, &fDistX, &fDistY, true)) {
			// Preserve original conditions and live owner reads
			fDistX *= (0.7f * fLifeTime + 2.0f) * message->m_fSize;
			fDistY *= (0.7f * fLifeTime + 2.0f) * message->m_fSize;
			MoneyMessages_BackendFontPropOn();
			MoneyMessages_BackendFontBackgroundOff();
			float fScaleY = MONEY_MESSAGES_MIN(fDistY / 100.0f, MAX_SCALE);
			float fScaleX = MONEY_MESSAGES_MIN(fDistX / 100.0f, MAX_SCALE);

#ifdef FIX_BUGS
			MoneyMessages_BackendFontScale(SCREEN_SCALE_X(fScaleX), SCREEN_SCALE_Y(fScaleY));
#else
			MoneyMessages_BackendFontScale(fScaleX, fScaleY);
#endif
			MoneyMessages_BackendFontCentreOn();
			MoneyMessages_BackendFontCentreSize(SCREEN_WIDTH);
			MoneyMessages_BackendFontJustifyOff();
			MoneyMessages_BackendFontColor(message->m_Colour.r,message->m_Colour.g,message->m_Colour.b,(255.0f - 255.0f * fLifeTime) * message->m_fOpacity);
			MoneyMessages_BackendFontBackGroundOnlyTextOff();
			MONEY_MESSAGES_FONT_LOCALE(FONT_STANDARD);
			MoneyMessages_BackendPrintString(vecOut.x, vecOut.y, message->m_aText);
		}
	}
}

void
MoneyMessages_Init(void)
{
	// Retain the complete original money-message operation and write order
	for (int32_t i = 0; i < NUMMONEYMESSAGES; i++)
		aMoneyMessages[i].m_nTimeRegistered = 0;
}

void
MoneyMessages_Render(void)
{
	// Retain the complete original money-message operation and write order
	for (int32_t i = 0; i < NUMMONEYMESSAGES; i++) {
		// Preserve original conditions and live owner reads
		if (aMoneyMessages[i].m_nTimeRegistered != 0)
			MoneyMessage_Render(&aMoneyMessages[i]);
	}
}

void
MoneyMessages_RegisterOne(MoneyMessageVector vecPos,const char *pText,uint8_t bRed,uint8_t bGreen,uint8_t bBlue,float fSize,float fOpacity)
{
	// Retain the complete original money-message operation and write order
	uint32_t i;
#ifdef FIX_BUGS
	for(i = 0; i < NUMMONEYMESSAGES && aMoneyMessages[i].m_nTimeRegistered != 0; i++);
#else
	for(i = 0; aMoneyMessages[i].m_nTimeRegistered != 0 && i < NUMMONEYMESSAGES; i++);
#endif

	if(i < NUMMONEYMESSAGES) {
		// Preserve original conditions and live owner reads
		// Add data of this money message to the array
		MoneyMessages_BackendAsciiToUnicode(pText, aMoneyMessages[i].m_aText);

		aMoneyMessages[i].m_nTimeRegistered = (*Timer_GetTimeInMilliseconds());
		aMoneyMessages[i].m_vecPosition = vecPos;
		aMoneyMessages[i].m_Colour.r = bRed;
		aMoneyMessages[i].m_Colour.g = bGreen;
		aMoneyMessages[i].m_Colour.b = bBlue;
		aMoneyMessages[i].m_fSize = fSize;
		aMoneyMessages[i].m_fOpacity = fOpacity;
	}
}
//- rouz edit (ChatGPT)
