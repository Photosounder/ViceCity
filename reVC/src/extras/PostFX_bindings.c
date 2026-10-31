//+ rouz edit (ChatGPT)
#include "config.h"
#include "postfx.h"
#include "MBlur.h"
#include "MBlur_backend.h"
#include "Lights_backend.h"
bool Lights_HostBlurColours(void)
{
    // Read the original active colour-filter preference or classic C blur state at each call site
#ifdef EXTENDED_COLOURFILTER
    return PostFX_UseBlurColours();
#else
    return MBlur_BlurOn;
#endif
}
#ifdef EXTENDED_COLOURFILTER
void MBlur_HostPostOpen(void *camera)
{
    // Forward the complete C blur operation directly into the complete C post-processing owner
    PostFX_Open(camera);
}
void MBlur_HostPostClose(void)
{
    // Retain the original delegation without routing application policy through C++
    PostFX_Close();
}
void MBlur_HostPostRender(void *camera,uint32_t red,uint32_t green,uint32_t blue,uint32_t blur,int32_t type,uint32_t alpha)
{
    // Retain all original C argument conversions before the complete post-processing operation
    PostFX_Render(camera,red,green,blue,blur,type,alpha);
}
#endif
//- rouz edit (ChatGPT)
