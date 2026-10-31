//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "ScrollBar_backend.h"
#include "General.h"
#include "Clock.h"
#include "RenderBuffer.h"
#include <math.h>
#include <string.h>
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#define ABS(a) (((a)<0)?(-(a)):(a))
enum { SCROLL_ARENA_STRING=0 };
ScrollBar aScrollBars[NUMSCROLLBARS];
int32_t ScrollBar_TonightsEvent;
static char String_Time[]="THE TIME IS 12:34    ";
static uint8_t ScrollCharSet[59][5] = {
	{ 0x00, 0x00, 0x00, 0x00, 0x00 }, // ' '
	{ 0x00, 0x00, 0x1D, 0x00, 0x00 }, // '!'
	{ 0x00, 0x00, 0x00, 0x00, 0x00 }, // '"'
	{ 0x0A, 0x1F, 0x0A, 0x1F, 0x0A }, // '#'
	{ 0x00, 0x09, 0x1F, 0x12, 0x00 }, // '$'
	{ 0x18, 0x18, 0x00, 0x03, 0x03 }, // '%'
	{ 0x00, 0x00, 0x00, 0x00, 0x00 }, // '&'
	{ 0x00, 0x00, 0x00, 0x00, 0x00 }, // '''
	{ 0x01, 0x02, 0x04, 0x08, 0x10 }, // '('
	{ 0x00, 0x00, 0x18, 0x00, 0x00 }, // ')'
	{ 0x15, 0x04, 0x1F, 0x04, 0x15 }, // '*'
	{ 0x00, 0x04, 0x0E, 0x04, 0x00 }, // '+'
	{ 0x00, 0x00, 0x03, 0x00, 0x00 }, // ','
	{ 0x00, 0x04, 0x04, 0x04, 0x00 }, // '-'
	{ 0x00, 0x00, 0x01, 0x00, 0x00 }, // '.'
	{ 0x00, 0x00, 0x00, 0x00, 0x00 }, // '/'
	{ 0x0E, 0x11, 0x11, 0x11, 0x0E }, // '0'
	{ 0x01, 0x09, 0x1F, 0x01, 0x01 }, // '1'
	{ 0x03, 0x15, 0x15, 0x15, 0x09 }, // '2'
	{ 0x11, 0x11, 0x15, 0x15, 0x0A }, // '3'
	{ 0x02, 0x06, 0x0A, 0x1F, 0x02 }, // '4'
	{ 0x1D, 0x15, 0x15, 0x15, 0x12 }, // '5'
	{ 0x0E, 0x15, 0x15, 0x15, 0x12 }, // '6'
	{ 0x18, 0x10, 0x13, 0x14, 0x18 }, // '7'
	{ 0x0A, 0x15, 0x15, 0x15, 0x0A }, // '8'
	{ 0x08, 0x15, 0x15, 0x15, 0x0E }, // '9'
	{ 0x00, 0x00, 0x0A, 0x00, 0x00 }, // ':'
	{ 0x18, 0x18, 0x00, 0x03, 0x03 }, // ';'
	{ 0x04, 0x08, 0x1F, 0x08, 0x04 }, // '<'
	{ 0x00, 0x0A, 0x0A, 0x0A, 0x00 }, // '='
	{ 0x04, 0x02, 0x1F, 0x02, 0x04 }, // '>'
	{ 0x10, 0x10, 0x15, 0x14, 0x1D }, // '?'
	{ 0x00, 0x1C, 0x14, 0x1C, 0x00 }, // '@'
	{ 0x0F, 0x12, 0x12, 0x12, 0x0F }, // 'A'
	{ 0x1F, 0x15, 0x15, 0x15, 0x0A }, // 'B'
	{ 0x0E, 0x11, 0x11, 0x11, 0x0A }, // 'C'
	{ 0x1F, 0x11, 0x11, 0x11, 0x0E }, // 'D'
	{ 0x1F, 0x15, 0x15, 0x11, 0x11 }, // 'E'
	{ 0x1F, 0x14, 0x14, 0x10, 0x10 }, // 'F'
	{ 0x0E, 0x11, 0x15, 0x15, 0x06 }, // 'G'
	{ 0x1F, 0x04, 0x04, 0x04, 0x1F }, // 'H'
	{ 0x11, 0x11, 0x1F, 0x11, 0x11 }, // 'I'
	{ 0x02, 0x01, 0x01, 0x01, 0x1E }, // 'J'
	{ 0x1F, 0x04, 0x0C, 0x12, 0x01 }, // 'K'
	{ 0x1F, 0x01, 0x01, 0x01, 0x01 }, // 'L'
	{ 0x1F, 0x08, 0x06, 0x08, 0x1F }, // 'M'
	{ 0x1F, 0x08, 0x04, 0x02, 0x1F }, // 'N'
	{ 0x0E, 0x11, 0x11, 0x11, 0x0E }, // 'O'
	{ 0x1F, 0x12, 0x12, 0x12, 0x0C }, // 'P'
	{ 0x0C, 0x12, 0x12, 0x13, 0x0D }, // 'Q'
	{ 0x1F, 0x14, 0x14, 0x16, 0x09 }, // 'R'
	{ 0x09, 0x15, 0x15, 0x15, 0x02 }, // 'S'
	{ 0x10, 0x10, 0x1F, 0x10, 0x10 }, // 'T'
	{ 0x1E, 0x01, 0x01, 0x01, 0x1E }, // 'U'
	{ 0x1C, 0x02, 0x01, 0x02, 0x1C }, // 'V'
	{ 0x1E, 0x01, 0x06, 0x01, 0x1E }, // 'W'
	{ 0x11, 0x0A, 0x04, 0x0A, 0x11 }, // 'X'
	{ 0x18, 0x04, 0x03, 0x04, 0x18 }, // 'Y'
	{ 0x11, 0x13, 0x15, 0x19, 0x11 }  // 'Z'
};
static ScrollBarVector ScrollBar_Add(ScrollBarVector a,ScrollBarVector b)
{
    // Preserve each original component addition and intermediate vector value
    ScrollBarVector value={a.x+b.x,a.y+b.y,a.z+b.z};return value;
}
static ScrollBarVector ScrollBar_Subtract(ScrollBarVector a,ScrollBarVector b)
{
    // Preserve each original component subtraction and intermediate vector value
    ScrollBarVector value={a.x-b.x,a.y-b.y,a.z-b.z};return value;
}
static ScrollBarVector ScrollBar_Scale(ScrollBarVector a,float factor)
{
    // Retain the original per-component multiplication before corner composition
    ScrollBarVector value={a.x*factor,a.y*factor,a.z*factor};return value;
}
static float ScrollBar_Dot(ScrollBarVector a,ScrollBarVector b)
{
    // Preserve the original left-associated component-product sum
    return a.x*b.x+a.y*b.y+a.z*b.z;
}
static float ScrollBar_Magnitude(ScrollBarVector a)
{
    // Keep the exact original square-root argument and arithmetic order
    return sqrtf(a.x*a.x+a.y*a.y+a.z*a.z);
}
static void ScrollBar_Normalise(ScrollBarVector *a)
{
    // Retain the original normalizer including its x-only zero fallback
    float sq=ScrollBar_Dot(*a,*a);if(sq>0.0f){
        // Multiply each declared component by the original reciprocal square root
        float inv=1.0f/sqrtf(sq);a->x*=inv;a->y*=inv;a->z*=inv;
    }else a->x=1.0f;
}
static void ScrollBar_RenderPixels(ScrollBar *bar,ScrollBarPoints points,uint8_t r,uint8_t g,uint8_t b);
static const char* ScrollBar_FindTimeMessage(void)
{
    // Preserve original live reads, pointer identity and complete state-write ordering
	String_Time[12] = '0' + Clock_GetHours() / 10;
	String_Time[13] = '0' + Clock_GetHours() % 10;
	String_Time[15] = '0' + Clock_GetMinutes() / 10;
	String_Time[16] = '0' + Clock_GetMinutes() % 10;
	return String_Time;
}
void ScrollBar_Init(ScrollBar *bar,ScrollBarVector pos1,ScrollBarVector pos2,uint8_t type,uint8_t red,uint8_t green,uint8_t blue,float scale)
{
    // Preserve original live reads, pointer identity and complete state-write ordering
	for (int i = 0; i < ARRAY_SIZE(bar->m_MessageBar); ++i)
		bar->m_MessageBar[i] = 0;

	bar->m_pMessage = ". ";
	bar->m_MessageCurrentChar = 0;
	bar->m_MessageLength = strlen(bar->m_pMessage);

	bar->m_Counter  = 0;
	bar->m_bVisible = false;
	bar->m_Position = pos1;
	bar->m_Type     = type;
	bar->m_Size.x   = (pos2.x - pos1.x) * 0.025f;
	bar->m_Size.y   = (pos2.y - pos1.y) * 0.025f;
	bar->m_Size.z   = (pos2.z - pos1.z) * 0.2f;
	bar->m_uRed     = red;
	bar->m_uGreen   = green;
	bar->m_uBlue    = blue;
	bar->m_fScale   = scale;
}
void ScrollBar_Update(ScrollBar *bar)
{
    // Preserve original live reads, pointer identity and complete state-write ordering
	float distanceFromCamera = ScrollBar_Magnitude(ScrollBar_Subtract(ScrollBar_BackendCameraPosition(),bar->m_Position));
	if (distanceFromCamera > 100.0f)
	{
		// Preserve the original branch and traversal order
		bar->m_bVisible = false;
		return;
	}

	bar->m_bVisible = true;

	if (distanceFromCamera < 75.0f)
		bar->m_fIntensity = 1.0f;
	else
		bar->m_fIntensity = 1.0f - 4.0f * (distanceFromCamera - 75.0f) / 100.0f;

	bar->m_Counter = (bar->m_Counter + 1) % 8;
		
	// if message is fully printed, load up the next one
	if (bar->m_Counter == 0 && ++bar->m_MessageCurrentChar >= bar->m_MessageLength)
	{
		// Preserve the original branch and traversal order
		const char* previousMessage = bar->m_pMessage;
		if (bar->m_Type == SCROLL_ARENA_STRING) {
			// Preserve the original branch and traversal order
			while (previousMessage == bar->m_pMessage)
			{
				// Preserve the original branch and traversal order
				switch (General_GetRandomNumber() % 4)
				{
					// Preserve the original branch and traversal order
				case 0:
					switch (ScrollBar_TonightsEvent) {
						// Preserve the original branch and traversal order
					case 0:
						bar->m_pMessage = "MAIN EVENT TONIGHT: CAR RACING . . .   ";
						break;
					case 1:
						bar->m_pMessage = "MAIN EVENT TONIGHT: DESTRUCTION DERBY . . .   ";
						break;
					case 2:
						bar->m_pMessage = "MAIN EVENT TONIGHT: BIKE RACING . . .   ";
						break;
					}
					break;
				case 1:
					switch (ScrollBar_TonightsEvent) {
						// Preserve the original branch and traversal order
					case 0:
						bar->m_pMessage = "FOR TICKETS TO THE HOT RING EVENT CALL 555-3764 . . .   ";
						break;
					case 1:
						bar->m_pMessage = "FOR TICKETS TO THE BLOOD RING EVENT CALL 555-3765 . . .   ";
						break;
					case 2:
						bar->m_pMessage = "FOR TICKETS TO THE DIRT RING EVENT CALL 555-3766 . . .   ";
						break;
					}
					break;
				case 2:
					bar->m_pMessage = "HYMAN MEMORIAL STADIUM. HOME TO SOME OF THE BIGGEST EVENTS OF"
						" THE WESTERN HEMISPHERE. ALSO AVAILABLE FOR CHILDREN PARTIES. . . ";
					break;
				case 3:
					bar->m_pMessage = ScrollBar_FindTimeMessage();
					break;
				default:
					break;
				}
			}
		}

		bar->m_MessageLength = (uint32_t)strlen(bar->m_pMessage);
		bar->m_MessageCurrentChar = 0;
	}

	// Scroll
	for (int i = 0; i < ARRAY_SIZE(bar->m_MessageBar)-1; i++)
		bar->m_MessageBar[i] = bar->m_MessageBar[i + 1];
	bar->m_MessageBar[ARRAY_SIZE(bar->m_MessageBar)-1] = bar->m_Counter < 5 ? ScrollCharSet[bar->m_pMessage[bar->m_MessageCurrentChar] - ' '][bar->m_Counter] : 0;

	// Introduce some random displaying glitches; signs aren't supposed to be perfect :P
	switch (General_GetRandomNumber() & 0xFF)
	{
		// Preserve the original branch and traversal order
	case 0x0D: bar->m_MessageBar[ARRAY_SIZE(bar->m_MessageBar)-1] = 0;                 break;
	case 0xE3: bar->m_MessageBar[ARRAY_SIZE(bar->m_MessageBar)-1] = 0xE3;              break;
	case 0x64: bar->m_MessageBar[ARRAY_SIZE(bar->m_MessageBar)-1] = ~bar->m_MessageBar[ARRAY_SIZE(bar->m_MessageBar)-1]; break;
	}
}
void ScrollBar_Render(ScrollBar *bar)
{
    // Preserve original live reads, pointer identity and complete state-write ordering
	if (!ScrollBar_BackendSphereVisible(bar->m_Position, 2.0f * 20.0f * (ABS(bar->m_Size.x) + ABS(bar->m_Size.y))))
		return;

	ScrollBar_BackendInitSprites();

	// Calculate intensity of colours
	uint8_t r = bar->m_fIntensity * bar->m_uRed;
	uint8_t g = bar->m_fIntensity * bar->m_uGreen;
	uint8_t b = bar->m_fIntensity * bar->m_uBlue;

	// Set render states
	ScrollBar_BackendZWrite(false);
	ScrollBar_BackendVertexAlpha(true);
	ScrollBar_BackendSrcBlend(true);
	ScrollBar_BackendDestBlend(true);
	ScrollBar_BackendLinear();
	ScrollBar_BackendSetRaster(ScrollBar_BackendRaster(ScrollBar_BackendTexture()));
	ScrollBar_BackendZTest(true);

    // Keep both actual native projection vectors alive throughout the pixel loop
    ScrollBar_BackendWithPoints(bar,r,g,b,ScrollBar_RenderPixels);
}
static void
ScrollBar_RenderPixels(ScrollBar *bar,ScrollBarPoints points,uint8_t r,uint8_t g,uint8_t b)
{
    // Preserve original live reads, pointer identity and complete state-write ordering
	float screenW, screenH;
	for (int i = 1; i < ARRAY_SIZE(bar->m_MessageBar); ++i)
	{
		// Preserve the original branch and traversal order
		for (int j = 0; j < 5; ++j)
		{
			// Preserve the original branch and traversal order
			(*points.input.x) = bar->m_Position.x + bar->m_Size.x * i;
			(*points.input.y) = bar->m_Position.y + bar->m_Size.y * i;
			(*points.input.z) = bar->m_Position.z + bar->m_Size.z * j;

			// Render main coronas
			if (bar->m_MessageBar[i] & (1 << j))
			{
				// Preserve the original branch and traversal order
				if (ScrollBar_BackendProject(points.input,points.output, &screenW, &screenH, true))
				{
					// Preserve the original branch and traversal order
					ScrollBar_BackendSprite(
						(*points.output.x), (*points.output.y), (*points.output.z),
						screenW * bar->m_fScale, screenH * bar->m_fScale,
						r, g, b,
						255, 1.0f / (*points.output.z), 255);
				}
			}
			// Render smaller and faded coronas for a trailing effect
			else if (bar->m_MessageBar[i - 1] & (1 << j))
			{
				// Preserve the original branch and traversal order
				if (ScrollBar_BackendProject(points.input,points.output, &screenW, &screenH, true))
				{
					// Preserve the original branch and traversal order
					ScrollBar_BackendSprite(
						(*points.output.x), (*points.output.y), (*points.output.z),
						screenW * bar->m_fScale * 0.8f,
						screenH * bar->m_fScale * 0.8f,
						r / 2,
						g / 2,
						b / 2,
						255, 1.0f / (*points.output.z), 255);
				}
			}
		}
	}

	ScrollBar_BackendFlushSprites();
}
#ifdef REVC_SOFTWARE_POLYGONS
void
ScrollBar_RenderForEnvMap(ScrollBar *bar,void *camera)
{
    // Preserve original live reads, pointer identity and complete state-write ordering
	// Skip boards outside the reflection camera's range or without a glow texture
	if(camera == NULL || ScrollBar_BackendTexture() == NULL)
		return;
	void *cameraFrame = ScrollBar_BackendCameraFrame(camera);
	// Skip rendering if the reflection camera has no frame matrix
	if(cameraFrame == NULL || ScrollBar_BackendFrameMatrix(cameraFrame) == NULL)
		return;

	// Orient each sign pixel to the reflection camera without reading main-camera projection
	void *cameraMatrix = ScrollBar_BackendFrameMatrix(cameraFrame);
    ScrollBarBasis basis=ScrollBar_BackendBasis(cameraMatrix);
    ScrollBarVector cameraPos=basis.position,cameraRight=basis.right,cameraUp=basis.up,cameraForward=basis.forward;
	ScrollBar_Normalise(&cameraRight);
	ScrollBar_Normalise(&cameraUp);
	ScrollBar_Normalise(&cameraForward);
	// Recompute visibility and glow strength from the reflection camera, not the main view
	float cameraDistance = ScrollBar_Magnitude(ScrollBar_Subtract(cameraPos,bar->m_Position));
	if(cameraDistance > 100.0f)
		return;
	float intensity = cameraDistance < 75.0f ? 1.0f : 1.0f - 4.0f * (cameraDistance - 75.0f) / 100.0f;
	float nearClip = ScrollBar_BackendNearClip(camera);
	float farClip = ScrollBar_BackendFarClip(camera);
	uint8_t red = intensity*bar->m_uRed;
	uint8_t green = intensity*bar->m_uGreen;
	uint8_t blue = intensity*bar->m_uBlue;

	// Preserve pending reflection geometry before batching the glowing sign pixels
	// Enter the C shared-buffer algorithm with the original arguments
	RenderBuffer_RenderStuffInBuffer();
	ScrollBar_BackendPush("CScrollBar::RenderForEnvMap");
	ScrollBar_BackendFog(false);
	ScrollBar_BackendZWrite(false);
	ScrollBar_BackendZTest(true);
	ScrollBar_BackendVertexAlpha(true);
	ScrollBar_BackendCull(true);
	ScrollBar_BackendLinear();
	ScrollBar_BackendSrcBlend(true);
	ScrollBar_BackendDestBlend(true);
	ScrollBar_BackendSetRaster(ScrollBar_BackendRaster(ScrollBar_BackendTexture()));
	// Enter the C shared-buffer algorithm with the original arguments
	RenderBuffer_ClearRenderBuffer();

	// Recreate the current and trailing glow pixels from the saved five-row message bitmap
	for(int i = 1; i < ARRAY_SIZE(bar->m_MessageBar); i++){
		// Preserve the original branch and traversal order
		for(int j = 0; j < 5; j++){
			// Preserve the original branch and traversal order
			bool currentPixel = (bar->m_MessageBar[i] & (1 << j)) != 0;
			bool trailingPixel = !currentPixel && (bar->m_MessageBar[i-1] & (1 << j)) != 0;
			if(!currentPixel && !trailingPixel)
				continue;
			ScrollBarVector pixelPosition={bar->m_Position.x+bar->m_Size.x*i,bar->m_Position.y+bar->m_Size.y*i,bar->m_Position.z+bar->m_Size.z*j};
			float depth = ScrollBar_Dot(ScrollBar_Subtract(pixelPosition,cameraPos),cameraForward);
			if(depth <= nearClip || depth >= farClip)
				continue;
			float pixelScale = bar->m_fScale*(currentPixel ? 3.0f : 2.4f);
			int pixelRed = currentPixel ? red : red/2;
			int pixelGreen = currentPixel ? green : green/2;
			int pixelBlue = currentPixel ? blue : blue/2;
			uint16_t *indices;
			void *vertices;
			// Reserve one reflection-camera billboard in the shared immediate buffer
			// Enter the C shared-buffer algorithm with the original arguments
			RenderBuffer_StartStoring(6,4,&indices,&vertices);
			ScrollBarVector horizontal=ScrollBar_Scale(cameraRight,pixelScale);
			ScrollBarVector vertical=ScrollBar_Scale(cameraUp,pixelScale);
			ScrollBarVector corners[4] = {
				// Preserve the original branch and traversal order
				ScrollBar_Add(ScrollBar_Subtract(pixelPosition,horizontal),vertical),
				ScrollBar_Add(ScrollBar_Add(pixelPosition,horizontal),vertical),
				ScrollBar_Subtract(ScrollBar_Subtract(pixelPosition,horizontal),vertical),
				ScrollBar_Subtract(ScrollBar_Add(pixelPosition,horizontal),vertical)
			};
			for(int vertex = 0; vertex < 4; vertex++){
				// Preserve the original branch and traversal order
				ScrollBar_BackendSetPos(vertices,vertex, corners[vertex].x, corners[vertex].y, corners[vertex].z);
				ScrollBar_BackendSetRGBA(vertices,vertex, pixelRed, pixelGreen, pixelBlue, 255);
			}
			ScrollBar_BackendSetU(vertices,0, 0.0f); ScrollBar_BackendSetV(vertices,0, 0.0f);
			ScrollBar_BackendSetU(vertices,1, 1.0f); ScrollBar_BackendSetV(vertices,1, 0.0f);
			ScrollBar_BackendSetU(vertices,2, 0.0f); ScrollBar_BackendSetV(vertices,2, 1.0f);
			ScrollBar_BackendSetU(vertices,3, 1.0f); ScrollBar_BackendSetV(vertices,3, 1.0f);
			indices[0] = 0; indices[1] = 1; indices[2] = 2;
			indices[3] = 2; indices[4] = 1; indices[5] = 3;
			// Enter the C shared-buffer algorithm with the original arguments
			RenderBuffer_StopStoring();
		}
	}

	// Flush the sign pixels and restore standard world effect states
	// Enter the C shared-buffer algorithm with the original arguments
	RenderBuffer_RenderStuffInBuffer();
	ScrollBar_BackendFog(false);
	ScrollBar_BackendZWrite(true);
	ScrollBar_BackendZTest(true);
	ScrollBar_BackendVertexAlpha(false);
	ScrollBar_BackendSrcBlend(false);
	ScrollBar_BackendDestBlend(false);
	ScrollBar_BackendSetRaster(NULL);
	ScrollBar_BackendCull(false);
	ScrollBar_BackendPop();
}
#endif
void ScrollBar_SetVisibility(ScrollBar *bar,bool visible)
{
    // Preserve the original direct visibility field assignment
    bar->m_bVisible=visible;
}
bool ScrollBar_IsVisible(ScrollBar *bar)
{
    // Return the same live flag used by the original inline getter
    return bar->m_bVisible;
}
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
