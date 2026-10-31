"""Move all remaining horizon state, background algorithms and cloud methods into C."""
import re
from audio_controls_migration import function
import plane_trails_migration as plane_trails
ARGS='int16_t topred,int16_t topgreen,int16_t topblue,int16_t botred,int16_t botgreen,int16_t botblue,int16_t alpha'
HEADER='//+ rouz edit (ChatGPT)\n#pragma once\n#include "Clouds_control.h"\n//- rouz edit (ChatGPT)\n'
PUBLIC='void Clouds_RenderBackground('+ARGS+');\nvoid Clouds_RenderHorizon(void);\n'
TYPES='''enum { CLOUDS_COLOR_TOP,CLOUDS_COLOR_BOTTOM,CLOUDS_COLOR_BACKGROUND };
typedef struct CloudsColorFields { uint8_t *r,*g,*b,*a; } CloudsColorFields;
typedef struct CloudsViewMatrix { float rx,ry,rz,fx,fy,fz,ux,uy,uz,px,py,pz; } CloudsViewMatrix;
'''
READERS=[
('CloudsColorFields','ColorFields','int32_t index','CRGBA &c=BackgroundColors[index];CloudsColorFields result={&c.r,&c.g,&c.b,&c.a};return result;'),
('CloudsVector','CameraUp','void','const CVector &v=TheCamera.GetUp();CloudsVector result={v.x,v.y,v.z};return result;'),
('CloudsVector','CameraForward','void','const CVector &v=TheCamera.GetForward();CloudsVector result={v.x,v.y,v.z};return result;'),
('float','ForwardAxis','int32_t axis','const CVector &v=TheCamera.GetForward();return axis==0?v.x:axis==1?v.y:v.z;'),
('bool','CollisionPolys','void','return gbShowCollisionPolys;'),
('float','ScreenHeight','void','return SCREEN_HEIGHT;'),
('CloudsViewMatrix','ViewMatrix','void','const CMatrix &m=TheCamera.m_viewMatrix;CloudsViewMatrix result={m.rx,m.ry,m.rz,m.fx,m.fy,m.fz,m.ux,m.uy,m.uz,m.px,m.py,m.pz};return result;'),
('bool','CapturingWorldEffects','void','#ifdef REVC_SOFTWARE_POLYGONS\n    return SoftwarePolygons::CapturingWorldEffects();\n#else\n    return false;\n#endif'),
('void','DrawBackgroundQuad','float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4,int32_t top,int32_t bottom','CSprite2d::DrawAnyRect(x1,y1,x2,y2,x3,y3,x4,y4,BackgroundColors[top],BackgroundColors[top],BackgroundColors[bottom],BackgroundColors[bottom]);'),
('void','DrawBackgroundRect','float left,float top,float right,float bottom,int32_t lower,int32_t upper','CSprite2d::DrawRect(CRect(left,top,right,bottom),BackgroundColors[lower],BackgroundColors[lower],BackgroundColors[upper],BackgroundColors[upper]);')]
for name in ['Red','Green','Blue']:READERS.append(('float','Ambient'+name,'void','return CTimeCycle::GetAmbient'+name+'();'))
DECLARATIONS=TYPES+''.join(result+' Clouds_Backend'+name+'('+args+');\n' for result,name,args,body in READERS)
HOST='''//+ rouz edit (ChatGPT)
static CRGBA BackgroundColors[3];
'''+''.join(result+' Clouds_Backend'+name+'('+args+')\n{\n    // Preserve the original actual owner fields, references and typed drawing operation\n    '+body+'\n}\n' for result,name,args,body in READERS)+'//- rouz edit (ChatGPT)\n'
HELPERS='''static CloudsVector Clouds_Cross(CloudsVector a,CloudsVector b)
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
'''

def globals_section(source):
    # Delimit only the original private horizon declarations and their cache
    a=source.index('static float HorizonX;');b=source.index('static bool HaveLastSafeHorizon;',a)+len('static bool HaveLastSafeHorizon;');return source[a:b]

def methods_section(source):
    # Include both complete public methods and all eleven original helpers
    a=source.index('bool\nUseDarkBackground(');tail=function(source,'CClouds::','RenderHorizon');b=source.index(tail,a)+len(tail);return source[a:b]

def calls(source):
    # Remove the final cloud class and preserve all unrelated rendering code
    if 'CRGBA CClouds::ms_colourTop;' in source:
        source=source.replace(globals_section(source),'').replace(methods_section(source),'')
        for name in ['Top','Bottom','BkGrd']:source=source.replace('CRGBA CClouds::ms_colour'+name+';\n','')
        source+='\n'+HOST
    if 'class CClouds\n' in source:return HEADER
    if 'typedef struct CloudsVector' in source and 'Clouds_RenderBackground(' not in source:source=source.replace('#ifdef __cplusplus\n}\n#endif',PUBLIC+'#ifdef __cplusplus\n}\n#endif')
    if 'Clouds_BackendProject(' in source and 'Clouds_BackendColorFields(' not in source:source=source.replace('#ifdef __cplusplus\n}\n#endif',DECLARATIONS+'#ifdef __cplusplus\n}\n#endif')
    lines=[]
    for line in source.splitlines(keepends=True):
        if re.search(r'CClouds::Render(?:Background|Horizon)\(',line) and not line.lstrip().startswith('//'):lines.append(re.match(r'\s*',line)[0]+'// Enter the C sky background and horizon algorithms with the original arguments\n')
        lines.append(line.replace('CClouds::RenderBackground(','Clouds_RenderBackground(').replace('CClouds::RenderHorizon(','Clouds_RenderHorizon('))
    return plane_trails.calls(''.join(lines))

def kernel(source,original):
    # Translate every original horizon and background statement into the existing C module
    body=globals_section(original)+'\n\n'+methods_section(original)
    body=body.replace('struct HorizonState\n{','typedef struct HorizonState\n{').replace('\n};','\n} HorizonState;',1)
    body=body.replace('const HorizonState &state','HorizonState state')
    body=body.replace('const CVector &in, float &x, float &y','CloudsVector in,float *x,float *y')
    a=body.index('static void CalcScreenCoorsNoClip(');b=body.index('static void UpdateHorizonCoors',a);part=body[a:b]
    part=part.replace('CVector view = TheCamera.m_viewMatrix * in','CloudsVector view=Clouds_TransformView(Clouds_BackendViewMatrix(),in)')
    part=part.replace('\n\tx =','\n\t*x =').replace('\n\ty =','\n\t*y =').replace('\n\t\ty = SCREEN_HEIGHT - y','\n\t\t*y = SCREEN_HEIGHT - *y')
    body=body[:a]+part+body[b:]
    body=body.replace('CVector forward(TheCamera.GetForward().x, TheCamera.GetForward().y, 0.0f)','CloudsVector forward={Clouds_BackendForwardAxis(0),Clouds_BackendForwardAxis(1),0.0f}')
    body=body.replace('CrossProduct(CVector(0.0f, 0.0f, 1.0f), forward)','Clouds_Cross((CloudsVector){0.f,0.f,1.f},forward)').replace('CrossProduct(TheCamera.GetUp(), TheCamera.GetForward())','Clouds_Cross(Clouds_BackendCameraUp(),Clouds_BackendCameraForward())').replace('CrossProduct(CVector(0.0f, 0.0f, 1.0f), TheCamera.GetForward())','Clouds_Cross((CloudsVector){0.f,0.f,1.f},Clouds_BackendCameraForward())')
    body=body.replace('TheCamera.GetPosition() + forward*3000.0f','Clouds_Add(Clouds_BackendCameraPosition(),Clouds_Scale(forward,3000.0f))').replace('center - right*3000.0f','Clouds_Subtract(center,Clouds_Scale(right,3000.0f))').replace('center + right*3000.0f','Clouds_Add(center,Clouds_Scale(right,3000.0f))')
    body=body.replace('CVector ','CloudsVector ').replace('TheCamera.GetForward().z','Clouds_BackendForwardAxis(2)')
    body=re.sub(r'(\w+).MagnitudeSqr\(\)',r'Clouds_Dot(\1,\1)',body).replace('DotProduct(','Clouds_Dot(');body=re.sub(r'(\w+).Normalise\(\)',r'Clouds_Normalise(&\1)',body)
    body=re.sub(r'CalcScreenCoorsNoClip\((\w+), (\w+), ([\w:]+)\);',r'CalcScreenCoorsNoClip(\1,&\2,&\3);',body)
    for name in ['ms_cameraRoll','ms_horizonZ','ms_HorizonTilt']:
        body=body.replace('CClouds::'+name,'Clouds_'+name);body=re.sub(r'(?<![\w:])'+name+r'\b','Clouds_'+name,body)
    body=body.replace('const CRGBA &top, const CRGBA &bottom','int32_t top,int32_t bottom')
    body=body.replace('CSprite2d::DrawAnyRect(','Clouds_BackendDrawBackgroundQuad(').replace('top, top, bottom, bottom);','top,bottom);')
    body=body.replace('static CRGBA\nScaleRGBAAlpha(const CRGBA &colour, float scale)','static CloudsColor\nScaleRGBAAlpha(CloudsColor colour,float scale)').replace('CRGBA(','Clouds_Color(').replace('uint8(','(uint8_t)(')
    for name in ['Top','Bottom','BkGrd']:body=body.replace('CClouds::ms_colour'+name,'ms_colour'+name)
    colors={'ms_colourTop':'CLOUDS_COLOR_TOP','ms_colourBottom':'CLOUDS_COLOR_BOTTOM','ms_colourBkGrd':'CLOUDS_COLOR_BACKGROUND'}
    def assign(match):
        # Keep original RHS capture before publishing each complete color
        rhs=match[2];rhs='Clouds_ReadBackgroundColor('+colors[rhs]+')' if rhs in colors else rhs
        return 'Clouds_WriteBackgroundColor('+colors[match[1]]+','+rhs+');'
    body=re.sub(r'\b('+'|'.join(colors)+r') = ([^;]+);',assign,body)
    for name,index in colors.items():
        body=re.sub(r'\b'+name+r'\.([rgba])',lambda match:'(*Clouds_BackendColorFields('+index+').'+match[1]+')',body)
        body=re.sub(r'\b'+name+r'\b',index,body)
    body=body.replace('CRect r(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);','float rectRight=SCREEN_WIDTH;float rectBottom=SCREEN_HEIGHT;').replace('CSprite2d::DrawRect(r, CLOUDS_COLOR_BOTTOM, CLOUDS_COLOR_BOTTOM, CLOUDS_COLOR_TOP, CLOUDS_COLOR_TOP)','Clouds_BackendDrawBackgroundRect(0.f,0.f,rectRight,rectBottom,CLOUDS_COLOR_BOTTOM,CLOUDS_COLOR_TOP)')
    body=body.replace('CClouds::RenderBackground(','Clouds_RenderBackground(').replace('CClouds::RenderHorizon(','Clouds_RenderHorizon(')
    body=body.replace('gbShowCollisionPolys','Clouds_BackendCollisionPolys()').replace('SoftwarePolygons::CapturingWorldEffects()','Clouds_BackendCapturingWorldEffects()')
    for name in ['Red','Green','Blue']:body=body.replace('CTimeCycle::GetAmbient'+name+'()','Clouds_BackendAmbient'+name+'()')
    for old,new in [('Sin','sinf'),('Cos','cosf'),('Atan2','atan2f'),('Sqrt','sqrtf'),('Abs','fabsf'),('SQR','Clouds_Square'),('Max','Clouds_Max'),('Clamp','Clouds_Clamp')]:body=body.replace(old+'(',new+'(')
    for old,new in [('uint8','uint8_t'),('int16','int16_t')]:body=re.sub(r'\b'+old+r'\b',new,body)
    for state,name in [('SRCBLEND','SrcBlend'),('DESTBLEND','DestBlend')]:body=re.sub(r'RwRenderStateSet\(rwRENDERSTATE'+state+r', \(void\*\)rwBLEND\w+\)',r'Clouds_Backend'+name+'(false)',body)
    body=body.replace('PUSH_RENDERGROUP(','Clouds_BackendPush(').replace('POP_RENDERGROUP()','Clouds_BackendPop()')
    body=body.replace('\n{\n','\n{\n    // Preserve the complete original horizon operation, live state and assignment order\n');body=re.sub(r'^(\t+)([^\n]*\{\n)',r'\1\2\1\t// Preserve original branch conditions and live data reads\n',body,flags=re.M)
    body=body.replace('typedef struct HorizonState\n{\n    // Preserve the complete original horizon operation, live state and assignment order\n','typedef struct HorizonState\n{\n')
    body=body.replace('bool\nUseDarkBackground','static bool\nUseDarkBackground')
    assert not re.search(r'\b(?:CVector|CMatrix|CRect|CRGBA|CClouds|TheCamera|CTimeCycle|CSprite2d|SoftwarePolygons|gbShowCollisionPolys)\b',re.sub(r'"[^"\n]*"','',body))
    prefix='\n#define SMALLSTRIPHEIGHT 4.0f\n#define HORIZSTRIPHEIGHT 48.0f\n#define SCREEN_WIDTH (Clouds_BackendScreenWidth())\n#define SCREEN_HEIGHT (Clouds_BackendScreenHeight())\n#define SCREEN_STRETCH_Y(a) ((a)*(float)SCREEN_HEIGHT/DEFAULT_SCREEN_HEIGHT)\n'
    assert source.endswith('//- rouz edit (ChatGPT)\n')
    return source[:-len('//- rouz edit (ChatGPT)\n')]+prefix+HELPERS+body+'\n//- rouz edit (ChatGPT)\n'

def project(source):
    # Retain the existing twenty-eight paired compilation units and all metadata
    return plane_trails.project(source)
