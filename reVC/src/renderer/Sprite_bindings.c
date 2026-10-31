//+ rouz edit (ChatGPT)
#include "Sprite.h"
#include "Clouds_backend.h"
#include "PointLights_backend.h"
#include "MoneyMessages_backend.h"
#include "ScrollBar_backend.h"
bool Clouds_BackendProject(CloudsVector value,CloudsVector *out,float *width,float *height,bool farclip)
{
    // Publish the projected point only on the original successful path
    SpriteVector input={value.x,value.y,value.z},projected;
    bool visible=Sprite_CalcScreenCoors(Sprite_VectorRef(&input.x,&input.y,&input.z),Sprite_VectorOut(&projected.x,&projected.y,&projected.z),width,height,farclip);
    if(visible){
        // Copy the defined C result into the actual peer receiver
        out->x=projected.x;out->y=projected.y;out->z=projected.z;
    }
    return visible;
}
void Clouds_BackendInitSprites(void)
{
    // Retain the original typed owner operation at its original C call point
    Sprite_InitSpriteBuffer();
}
void Clouds_BackendFlushSprites(void)
{
    // Retain the original typed owner operation at its original C call point
    Sprite_FlushSpriteBuffer();
}
void Clouds_BackendRenderOneXLUSprite(float x,float y,float z,float w,float h,uint8_t r,uint8_t g,uint8_t b,int16_t intens,float recipz,uint8_t a)
{
    // Retain the original typed owner operation at its original C call point
    Sprite_RenderOneXLUSprite(x,y,z,w,h,r,g,b,intens,recipz,a);
}
void Clouds_BackendRenderBufferedOneXLUSprite(float x,float y,float z,float w,float h,uint8_t r,uint8_t g,uint8_t b,int16_t intens,float recipz,uint8_t a)
{
    // Retain the original typed owner operation at its original C call point
    Sprite_RenderBufferedOneXLUSprite(x,y,z,w,h,r,g,b,intens,recipz,a);
}
void Clouds_BackendRenderBufferedOneXLUSprite_Rotate_Dimension(float x,float y,float z,float w,float h,uint8_t r,uint8_t g,uint8_t b,int16_t intens,float recipz,float roll,uint8_t a)
{
    // Retain the original typed owner operation at its original C call point
    Sprite_RenderBufferedOneXLUSprite_Rotate_Dimension(x,y,z,w,h,r,g,b,intens,recipz,roll,a);
}
void Clouds_BackendRenderBufferedOneXLUSprite_Rotate_Aspect(float x,float y,float z,float w,float h,uint8_t r,uint8_t g,uint8_t b,int16_t intens,float recipz,float roll,uint8_t a)
{
    // Retain the original typed owner operation at its original C call point
    Sprite_RenderBufferedOneXLUSprite_Rotate_Aspect(x,y,z,w,h,r,g,b,intens,recipz,roll,a);
}
void Clouds_BackendRenderBufferedOneXLUSprite_Rotate_2Colours(float x,float y,float z,float w,float h,uint8_t r1,uint8_t g1,uint8_t b1,uint8_t r2,uint8_t g2,uint8_t b2,float cx,float cy,float recipz,float rotation,uint8_t a)
{
    // Retain the original typed owner operation at its original C call point
    Sprite_RenderBufferedOneXLUSprite_Rotate_2Colours(x,y,z,w,h,r1,g1,b1,r2,g2,b2,cx,cy,recipz,rotation,a);
}
void PointLights_BackendSpriteInit(void)
{
    // Initialize the original sprite buffer
    Sprite_InitSpriteBuffer();
}
void PointLights_BackendSpriteFlush(void)
{
    // Flush the original sprite buffer
    Sprite_FlushSpriteBuffer();
}
bool PointLights_BackendScreen(PointLightsVector coors,PointLightsVector *screen,float *width,float *height,bool farclip)
{
    // Publish the projected point only on the original successful path
    SpriteVector input={coors.x,coors.y,coors.z},projected;
    bool visible=Sprite_CalcScreenCoors(Sprite_VectorRef(&input.x,&input.y,&input.z),Sprite_VectorOut(&projected.x,&projected.y,&projected.z),width,height,farclip);
    if(visible){
        // Copy the defined C result into the actual peer receiver
        screen->x=projected.x;screen->y=projected.y;screen->z=projected.z;
    }
    return visible;
}
void PointLights_BackendSprite(float x,float y,float z,float width,float height,uint8_t red,uint8_t green,uint8_t blue,int16_t intensity,float reciprocal,float rotation,uint8_t alpha)
{
    // Retain the exact original narrowed sprite signature
    Sprite_RenderBufferedOneXLUSprite_Rotate_Aspect(x,y,z,width,height,red,green,blue,intensity,reciprocal,rotation,alpha);
}
bool MoneyMessages_BackendProject(MoneyMessageVector position,MoneyMessageVector *out,float *width,float *height,bool farclip)
{
    // Publish the projected point only on the original successful path
    SpriteVector input={position.x,position.y,position.z},projected;
    bool visible=Sprite_CalcScreenCoors(Sprite_VectorRef(&input.x,&input.y,&input.z),Sprite_VectorOut(&projected.x,&projected.y,&projected.z),width,height,farclip);
    if(visible){
        // Copy the defined C result into the actual peer receiver
        out->x=projected.x;out->y=projected.y;out->z=projected.z;
    }
    return visible;
}
void ScrollBar_BackendInitSprites(void)
{
    // Forward to the complete C sprite owner with the original argument values
    Sprite_InitSpriteBuffer();
}
void ScrollBar_BackendFlushSprites(void)
{
    // Forward to the complete C sprite owner with the original argument values
    Sprite_FlushSpriteBuffer();
}
void ScrollBar_BackendWithPoints(ScrollBar *bar,uint8_t r,uint8_t g,uint8_t b,ScrollBarPointsCallback callback)
{
    // Forward to the complete C sprite owner with the original argument values
    SpriteVector input,output;ScrollBarPoints points={{&input,&input.x,&input.y,&input.z},{&output,&output.x,&output.y,&output.z}};callback(bar,points,r,g,b);
}
bool ScrollBar_BackendProject(ScrollBarPoint input,ScrollBarPoint output,float *width,float *height,bool farclip)
{
    // Project directly through the live point scalar views
    return Sprite_CalcScreenCoors(Sprite_VectorRef(input.x,input.y,input.z),Sprite_VectorOut(output.x,output.y,output.z),width,height,farclip);
}
void ScrollBar_BackendSprite(float x,float y,float z,float width,float height,uint8_t r,uint8_t g,uint8_t b,int16_t intensity,float reciprocal,uint8_t alpha)
{
    // Forward to the complete C sprite owner with the original argument values
    Sprite_RenderBufferedOneXLUSprite(x,y,z,width,height,r,g,b,intensity,reciprocal,alpha);
}
//- rouz edit (ChatGPT)
