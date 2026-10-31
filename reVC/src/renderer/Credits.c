//+ rouz edit (ChatGPT)
#include "Credits_backend.h"
#include "Timer.h"
#include "Draw.h"
#include "Draw_constants.h"
#include "FontStyles.h"
#define SCREEN_WIDTH (Credits_BackendScreenWidth())
#define SCREEN_HEIGHT (Credits_BackendScreenHeight())
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


static bool bCreditsGoing;
static uint32_t CreditsStartTime;

void
Credits_Init(void)
{
    // Stop credits without resetting the original start timestamp
	Credits_Stop();
}

void
Credits_Start(void)
{
    // Publish the active flag before reading the original live timer
	bCreditsGoing = true;
	CreditsStartTime = (*Timer_GetTimeInMilliseconds());
}

void
Credits_Stop(void)
{
    // Clear only the original active flag
	bCreditsGoing = false;
}

void
Credits_PrintCreditSpace(float space, uint32_t *line)
{
    // Preserve float compound-assignment rounding before the original unsigned conversion
	(*line) += space * 25.0f;
}

void
Credits_PrintCreditText(float scaleX, float scaleY, uint16_t *text, uint32_t *lineoffset, float scrolloffset)
{
    // Update input before printing and advancing the live borrowed line offset
	Credits_BackendUpdatePads();
	if (Credits_BackendCrossJustDown())
		bCreditsGoing = false;
	else {
        // Preserve the original clipping and line advance after all font callbacks
		float start = DEFAULT_SCREEN_HEIGHT + 20.0f;
		float y = (*lineoffset) + start - scrolloffset;
		if (y > 20.0f && DEFAULT_SCREEN_HEIGHT - 20.0f > y) {
            // Submit both original shadow and foreground strings through the same live text pointer
			Credits_BackendFontScale(SCREEN_SCALE_X(scaleX), SCREEN_SCALE_Y(scaleY));
			Credits_BackendFontColor(0, 0, 0, 255);
			Credits_BackendPrintString(SCREEN_WIDTH / 2.0f, SCREEN_SCALE_Y(y), text);
			Credits_BackendFontColor(220, 220, 220, 220);
			Credits_BackendPrintString(SCREEN_WIDTH / 2.0f - SCREEN_SCALE_X(1.0f), SCREEN_SCALE_Y(y - 1.0f), text);
		}
		(*lineoffset) += scaleY*25.0f;
	}
}

void
Credits_Render(void)
{
    // Render the complete original sequence with live language and viewport settings
	uint32_t lineoffset;
	float scrolloffset;

	if(!bCreditsGoing || Credits_BackendMenuActive())
		return;

	Credits_BackendDefinedState();
	lineoffset = 0;
	scrolloffset = ((*Timer_GetTimeInMilliseconds()) - CreditsStartTime) / 24.0f;
	Credits_BackendFontJustifyOff();
	Credits_BackendFontBackgroundOff();
	Credits_BackendFontCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH * 0.75f));
	Credits_BackendFontCentreOn();
	Credits_BackendFontPropOn();
	Credits_BackendFontStyle(FONT_STANDARD);

    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED001"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED002"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED003"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED004"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED005"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED006"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED007"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED008"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED025"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED026"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED027"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED028"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED029"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED030"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED031"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD031A"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD031B"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD031C"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRD031D"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD031E"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRD024A"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian() || Credits_BackendFrench() || Credits_BackendSpanish())
        Credits_PrintCreditSpace(0.5f, &lineoffset);

    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED024"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED023"), &lineoffset, scrolloffset);
    if (Credits_BackendFrench())
        Credits_PrintCreditSpace(0.5f, &lineoffset);

    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD023A"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD023B"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED018"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED019"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRD018A"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian() || Credits_BackendFrench())
        Credits_PrintCreditSpace(0.5f, &lineoffset);

    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD019A"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD019B"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED020"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED021"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRD022A"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED022"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD022B"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD022C"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED032"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED033"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRD032A"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED034"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED035"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED036"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED037"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD037A"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD037B"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD037C"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRD041B"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED042"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED039"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED044"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED040"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD042A"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED142"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD142A"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED009"), &lineoffset, scrolloffset);
    if (Credits_BackendFrench())
        Credits_PrintCreditSpace(0.5f, &lineoffset);

    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED010"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED011"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED012"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED013"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD013A"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD013B"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD013C"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED089"), &lineoffset, scrolloffset);
    if (Credits_BackendSpanish())
        Credits_PrintCreditSpace(0.5f, &lineoffset);

    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED090"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED347"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED047"), &lineoffset, scrolloffset);
    if (Credits_BackendSpanish())
        Credits_PrintCreditSpace(0.5f, &lineoffset);

    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED048"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED049"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED348"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED050"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED051"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED052"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED053"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED054"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED055"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED056"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD056A"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD056B"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD056C"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD056D"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED349"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED350"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED351"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED352"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED353"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED354"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED355"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED356"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED357"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED359"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED360"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED361"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED362"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED363"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED364"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED365"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED366"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED367"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED368"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED369"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED370"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED371"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED372"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED373"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED256"), &lineoffset, scrolloffset);
    if (Credits_BackendFrench())
        Credits_PrintCreditSpace(0.5f, &lineoffset);

    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED257"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED258"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED057"), &lineoffset, scrolloffset);
    if (Credits_BackendFrench() || Credits_BackendSpanish())
        Credits_PrintCreditSpace(0.5f, &lineoffset);

    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED058"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRD057A"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED059"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRD060A"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD060B"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD060C"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRD002A"), &lineoffset, scrolloffset);
    if (Credits_BackendFrench() || Credits_BackendSpanish())
        Credits_PrintCreditSpace(0.5f, &lineoffset);

    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED003"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRD001A"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD001B"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED060"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED061"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED062"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED063"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED064"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED069"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED070"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED065"), &lineoffset, scrolloffset);
    if (Credits_BackendFrench() || Credits_BackendSpanish())
        Credits_PrintCreditSpace(0.5f, &lineoffset);

    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED066"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED067"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED068"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRD071A"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD072A"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED091"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED094"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED095"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian() || Credits_BackendSpanish() || Credits_BackendFrench())
        Credits_PrintCreditSpace(0.5f, &lineoffset);

    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED097"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED098"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD098A"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD098B"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD098C"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED099"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED096"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED273"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD092A"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED092"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD092B"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED073"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED074"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED076"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED075"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED077"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED078"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED081"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED082"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED079"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED080"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED083"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED084"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD084A"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD084B"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD084C"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRD084D"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD084E"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.7f, Credits_BackendText("CRED085"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED086"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRD086A"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.7f, Credits_BackendText("CRED087"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED088"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRD088A"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRD088B"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRD088C"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRD088D"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRD088E"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRD088F"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRD088G"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED107"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED108"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED109"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    if (Credits_BackendItalian())
        Credits_PrintCreditSpace(0.5f, &lineoffset);

    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED110"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRD110A"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED111"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED112"), &lineoffset, scrolloffset);

    if (Credits_BackendItalian())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED113"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED114"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED115"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED116"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED117"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED118"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED119"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED120"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED121"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED122"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED123"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED124"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED125"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED126"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED127"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED128"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian())
        Credits_PrintCreditSpace(0.5f, &lineoffset);

    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED129"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);

    Credits_BackendFontCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH * 0.8f));

    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRD111A"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);

    if (Credits_BackendItalian() || Credits_BackendSpanish() || Credits_BackendFrench())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED130"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian() || Credits_BackendSpanish() || Credits_BackendFrench())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED131"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian() || Credits_BackendSpanish() || Credits_BackendFrench())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED132"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian() || Credits_BackendSpanish() || Credits_BackendFrench())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED133"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian() || Credits_BackendSpanish() || Credits_BackendFrench())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED134"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian() || Credits_BackendSpanish() || Credits_BackendFrench())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRD134A"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian() || Credits_BackendSpanish() || Credits_BackendFrench())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRD134B"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian() || Credits_BackendSpanish() || Credits_BackendFrench())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRD134C"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian() || Credits_BackendSpanish() || Credits_BackendFrench())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRD134D"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian() || Credits_BackendSpanish() || Credits_BackendFrench())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRD134E"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian() || Credits_BackendSpanish() || Credits_BackendFrench())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRD134F"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian() || Credits_BackendSpanish() || Credits_BackendFrench())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRD134G"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian() || Credits_BackendSpanish() || Credits_BackendFrench())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRD134H"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian() || Credits_BackendSpanish() || Credits_BackendFrench())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRD134I"), &lineoffset, scrolloffset);

    Credits_BackendFontCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH * 0.7f));

    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED135"), &lineoffset, scrolloffset);

    if (Credits_BackendFrench() || Credits_BackendSpanish())
        Credits_PrintCreditSpace(0.5f, &lineoffset);

    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD136A"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD137A"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED138"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED066"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD138B"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED139"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 0.8f, Credits_BackendText("CRED140"), &lineoffset, scrolloffset);
    if (Credits_BackendFrench())
        Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD140A"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD140B"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD140C"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD140D"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD140E"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD140F"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD140G"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD140H"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD140I"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD140J"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD140K"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD140L"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    
    Credits_BackendFontCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH * 0.85f));

    Credits_PrintCreditText(0.95f, 0.7f, Credits_BackendText("CRED259"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED260"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED261"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED262"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED263"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED264"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED265"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED266"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.7f, Credits_BackendText("CRED141"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRD141A"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRD141B"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED143"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED144"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.7f, Credits_BackendText("CRED145"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED146"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED147"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED148"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED149"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED150"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED151"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED152"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED153"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED154"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED155"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED156"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED157"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED158"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED159"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED160"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED161"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED162"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED163"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED164"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED165"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED166"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED167"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED168"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED169"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED170"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED171"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED172"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);

    Credits_BackendFontCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH * 0.75f));

    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.7f, Credits_BackendText("CRED217"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.7f, Credits_BackendText("CRED218"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.0f, &lineoffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRD218A"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.7f, Credits_BackendText("CRED219"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.0f, &lineoffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED220"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.7f, Credits_BackendText("CRED221"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.0f, &lineoffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED222"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.7f, Credits_BackendText("CRED223"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.0f, &lineoffset);
    Credits_PrintCreditText(1.1f, 1.1f, Credits_BackendText("CRED224"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.7f, Credits_BackendText("CRED227"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.0f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED228"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.7f, Credits_BackendText("CRED229"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRD229A"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRD229B"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.7f, Credits_BackendText("CRED274"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED275"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED276"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED277"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.7f, Credits_BackendText("CRED278"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED279"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED280"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED281"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.7f, Credits_BackendText("CRED282"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED283"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED284"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED285"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED286"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.7f, Credits_BackendText("CRED287"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED288"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED289"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED290"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.7f, Credits_BackendText("CRED291"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED292"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.7f, Credits_BackendText("CRED293"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED294"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED295"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED296"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.7f, Credits_BackendText("CRED297"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED298"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED299"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED300"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED301"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED302"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED303"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED304"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED305"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED306"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED307"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED308"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.7f, Credits_BackendText("CRED309"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED310"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.7f, Credits_BackendText("CRED314"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.7f, Credits_BackendText("CRED315"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.7f, Credits_BackendText("CRED316"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.0f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED317"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED318"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED319"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED320"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED321"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED322"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED323"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED324"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED325"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED326"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED327"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED328"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.7f, Credits_BackendText("CRED329"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED330"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED331"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED332"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    
    Credits_BackendFontCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH * 0.8f));

    Credits_PrintCreditText(0.95f, 0.7f, Credits_BackendText("CRED333"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED334"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED335"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED336"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED337"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED338"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED339"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED340"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED341"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED342"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditSpace(1.0f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRD344A"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian())
        Credits_PrintCreditSpace(0.5f, &lineoffset);

    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED344"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED345"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRD345A"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.0f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED346"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian())
        Credits_PrintCreditSpace(1.0f, &lineoffset);

    Credits_PrintCreditSpace(1.5f, &lineoffset);

    Credits_BackendFontCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH * 0.75f));

    Credits_PrintCreditText(0.95f, 0.7f, Credits_BackendText("CRED267"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(0.5f, &lineoffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED268"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.7f, Credits_BackendText("CRED269"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED270"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED271"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.65f, 0.65f, Credits_BackendText("CRED272"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditText(0.95f, 0.7f, Credits_BackendText("CRED230"), &lineoffset, scrolloffset);
    if (Credits_BackendItalian())
        Credits_PrintCreditSpace(0.5f, &lineoffset);

    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED231"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED232"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED233"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED234"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED235"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED236"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED237"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED238"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED239"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED240"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED241"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED242"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED243"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED244"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED245"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED246"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED247"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED248"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED249"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED358"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED250"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED251"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED252"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRD251A"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRD252A"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED253"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED254"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED374"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED375"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED376"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED377"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED378"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED379"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED380"), &lineoffset, scrolloffset);
    Credits_PrintCreditText(0.95f, 0.95f, Credits_BackendText("CRED381"), &lineoffset, scrolloffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_PrintCreditSpace(1.5f, &lineoffset);
    Credits_BackendDrawFonts();
#ifdef CUTSCENE_BORDERS_SWITCH
    if (Credits_BackendCutsceneBorders())
#endif
	if(Credits_BackendWidescreen())
		Credits_BackendDrawBorders();

#ifdef FIX_BUGS
	if(lineoffset + DEFAULT_SCREEN_HEIGHT - scrolloffset < -10.0f)
#else
	if(lineoffset + SCREEN_HEIGHT - scrolloffset < -10.0f)
#endif
	{
        // Stop only after the original final scroll-boundary test
		bCreditsGoing = false;
	}
}

bool Credits_AreCreditsDone(void)
{
    // Return the inverse of the original active flag
	return !bCreditsGoing;
}
//- rouz edit (ChatGPT)
