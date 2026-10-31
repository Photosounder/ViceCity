"""Remove the three marker member functions while preserving actual matrix lifetimes."""
import re
import credits_migration as credits

ARGS='void *marker,uint32_t identifier,uint16_t type,float size,uint8_t r,uint8_t g,uint8_t b,uint8_t a,uint16_t pulsePeriod,float pulseFraction,int16_t rotateRate'
PROTOTYPES='''bool Markers3D_RecordAdd('''+ARGS+''');
void Markers3D_RecordDelete(void *marker);
void Markers3D_RecordRender(void *marker);
'''
READERS=[
('void','Unity','void *marker','((C3dMarker *)marker)->m_Matrix.SetUnity();','Retain original typed matrix initialization'),
('void *','LastAtomic','void *clump','RpAtomic *atomic=nil;RpClumpForAllAtomics((RpClump *)clump,MarkerAtomicCB,&atomic);return atomic;','Run the exact typed callback with an actual typed atomic slot'),
('void *','CloneAtomic','void *atomic','return RpAtomicClone((RpAtomic *)atomic);','Clone the originally selected atomic'),
('void *','CreateFrame','void','return RwFrameCreate();','Create the original resource frame'),
('void','SetFrame','void *atomic,void *frame','RpAtomicSetFrame((RpAtomic *)atomic,(RwFrame *)frame);','Attach the original atomic and frame'),
('void','DefaultCallback','void *atomic','CVisibilityPlugins::SetAtomicRenderCallback((RpAtomic *)atomic,nil);','Retain the original null atomic-render callback'),
('void *','AtomicGeometry','void *atomic','return RpAtomicGetGeometry((RpAtomic *)atomic);','Read the original typed atomic geometry'),
('uint32_t','GeometryFlags','void *geometry','return RpGeometryGetFlags((RpGeometry *)geometry);','Read the actual geometry flags'),
('void','SetGeometryFlags','void *geometry,uint32_t flags','RpGeometrySetFlags((RpGeometry *)geometry,flags);','Publish the C-computed geometry flags'),
('void *','GeometryMaterial','void *geometry,int32_t index','return RpGeometryGetMaterial((RpGeometry *)geometry,index);','Read the original typed material slot'),
('void','SetMaterial','void *marker,void *material','((C3dMarker *)marker)->m_pMaterial=(RpMaterial *)material;','Publish the original material through its actual pointer type'),
('void','AttachMarker','void *marker,void *matrix','((C3dMarker *)marker)->m_Matrix.Attach((RwMatrix *)matrix);','Preserve the original embedded matrix attachment and ownership'),
('void *','AtomicFrame','void *atomic','return RpAtomicGetFrame((RpAtomic *)atomic);','Read the current atomic frame at the original call point'),
('void','DestroyAtomic','void *atomic','RpAtomicDestroy((RpAtomic *)atomic);','Destroy the original live atomic'),
('void','DestroyFrame','void *frame','RwFrameDestroy((RwFrame *)frame);','Destroy the original captured frame'),
('void','MaterialColor','void *marker','C3dMarker &m=*(C3dMarker *)marker;RpMaterialSetColor(m.m_pMaterial,&m.m_Color);','Borrow the actual live material color without copying its bytes'),
('void *','Attachment','void *marker','return ((C3dMarker *)marker)->m_Matrix.m_attachment;','Read the current embedded matrix attachment'),
('void','AttachMatrix','void *matrix,void *attachment','((CMatrix *)matrix)->Attach((RwMatrix *)attachment);','Attach the independent original stack matrix'),
('Markers3DBasis','MatrixBasis','void *matrix','CMatrix &m=*(CMatrix *)matrix;Markers3DBasis result={{{&m.rx,&m.ry,&m.rz},{&m.fx,&m.fy,&m.fz},{&m.ux,&m.uy,&m.uz}}};return result;','Borrow all nine actual matrix basis coefficients through typed pointers'),
('void','PublishMatrix','void *matrix','((CMatrix *)matrix)->UpdateRW();','Publish the independent original render matrix'),
('void','UpdateFrame','void *frame','RwFrameUpdateObjects((RwFrame *)frame);','Update the original frame objects'),
('void','Bright','float value','SetBrightMarkerColours(value);','Set the original live marker brightness'),
('void','ZWrite','bool enabled','RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,(void *)(uintptr)(enabled?TRUE:FALSE));','Set the original depth-write render state'),
('void','DrawAtomic','void *atomic','RpAtomicRender((RpAtomic *)atomic);','Render the current original atomic'),
('void','RestoreLighting','void','ReSetAmbientAndDirectionalColours();','Restore original ambient and directional lighting')]
EXTRA_HEADER='''typedef struct Markers3DBasis { float *axis[3][3]; } Markers3DBasis;
enum { MARKERS3D_GEOMETRY_MODULATE_MATERIAL_COLOR=0x40 };
'''
DECLARATIONS=EXTRA_HEADER+''.join(result+' Markers3D_Backend'+name+'('+args+');\n' for result,name,args,body,comment in READERS)+'''void Markers3D_BackendWithRenderMatrix(void *marker,void (*render)(void *,void *));
'''+PROTOTYPES
CALLBACK='''static RpAtomic *
MarkerAtomicCB(RpAtomic *atomic,void *data)
{
    // Retain the exact typed callback slot used to select the last atomic
    *(RpAtomic **)data=atomic;return atomic;
}
'''
HOST=CALLBACK+'static_assert(MARKERS3D_GEOMETRY_MODULATE_MATERIAL_COLOR==rpGEOMETRYMODULATEMATERIALCOLOR,"Original geometry modulation flag");\n'+''.join(result+' Markers3D_Backend'+name+'('+args+')\n{\n    // '+comment+'\n    '+body+'\n}\n' for result,name,args,body,comment in READERS)+'''void Markers3D_BackendWithRenderMatrix(void *marker,void (*render)(void *,void *))
{
    // Keep each temporary CMatrix alive until its complete C render sequence returns
    CMatrix matrix;render(marker,&matrix);
}
'''

def record_section(source):
    # Delimit the exact original callback and three complete record methods
    a=source.index('RpAtomic *\nMarkerAtomicCB(');b=source.index('\n}',source.index('C3dMarker::Render()',a))+2
    return source[a:b]

def methods(source):
    # Translate all original statements while retaining typed owner calls and live reads
    body=record_section(source);body=body[body.index('bool\nC3dMarker::AddMarker'):]
    body=body.replace('C3dMarkers::m_pRpClumpArray[type]','(RpClump *)Markers3D_BackendClump(type)')
    signature='bool\nC3dMarker::AddMarker(uint32 identifier, uint16 type, float fSize, uint8 r, uint8 g, uint8 b, uint8 a, uint16 pulsePeriod, float pulseFraction, int16 rotateRate)'
    assert signature in body
    body=body.replace(signature,'bool\nMarkers3D_RecordAdd('+ARGS.replace('float size','float fSize')+')')
    body=body.replace('void\nC3dMarker::DeleteMarkerObject()','void\nMarkers3D_RecordDelete(void *marker)')
    body=body.replace('void\nC3dMarker::Render()','void\nMarkers3D_RecordRender(void *marker)')
    body=body.replace('m_Matrix.SetUnity()','Markers3D_BackendUnity(marker)')
    selection='''RpAtomic *origAtomic;
	origAtomic = nil;
	RpClumpForAllAtomics((RpClump *)Markers3D_BackendClump(type), MarkerAtomicCB, &origAtomic);'''
    assert selection in body
    body=body.replace(selection,'void *origAtomic=Markers3D_BackendLastAtomic(Markers3D_BackendClump(type));')
    body=body.replace('RpAtomic *atomic = RpAtomicClone(origAtomic)','void *atomic = Markers3D_BackendCloneAtomic(origAtomic)').replace('RwFrame *frame = RwFrameCreate()','void *frame = Markers3D_BackendCreateFrame()')
    body=body.replace('RpAtomicSetFrame(atomic, frame)','Markers3D_BackendSetFrame(atomic,frame)').replace('CVisibilityPlugins::SetAtomicRenderCallback(atomic, nil)','Markers3D_BackendDefaultCallback(atomic)')
    body=body.replace('RpGeometry *geometry = RpAtomicGetGeometry(atomic)','void *geometry = Markers3D_BackendAtomicGeometry(atomic)').replace('RpGeometrySetFlags(geometry, RpGeometryGetFlags(geometry) | rpGEOMETRYMODULATEMATERIALCOLOR)','Markers3D_BackendSetGeometryFlags(geometry,Markers3D_BackendGeometryFlags(geometry)|MARKERS3D_GEOMETRY_MODULATE_MATERIAL_COLOR)')
    body=body.replace('m_pAtomic = atomic','Markers3D_BackendSetAtomic(marker,atomic)').replace('m_Matrix.Attach(RwFrameGetMatrix(RpAtomicGetFrame(m_pAtomic)))','Markers3D_BackendAttachMarker(marker,Markers3D_BackendFrameMatrix(Markers3D_BackendAtomicFrame(Markers3D_BackendAtomic(marker))))')
    body=body.replace('m_pMaterial = RpGeometryGetMaterial(geometry, 0)','Markers3D_BackendSetMaterial(marker,Markers3D_BackendGeometryMaterial(geometry,0))')
    body=body.replace('RwFrame *frame;','void *frame;').replace('RpAtomicGetFrame(m_pAtomic)','Markers3D_BackendAtomicFrame(Markers3D_BackendAtomic(marker))').replace('RpAtomicDestroy(m_pAtomic)','Markers3D_BackendDestroyAtomic(Markers3D_BackendAtomic(marker))').replace('RwFrameDestroy(frame)','Markers3D_BackendDestroyFrame(frame)').replace('m_pAtomic = nil','Markers3D_BackendSetAtomic(marker,NULL)').replace('m_pAtomic != nil','Markers3D_BackendAtomic(marker) != NULL').replace('m_pAtomic == nil','Markers3D_BackendAtomic(marker) == NULL')
    body=body.replace('RpMaterialSetColor(m_pMaterial, &m_Color)','Markers3D_BackendMaterialColor(marker)').replace('m_Matrix.UpdateRW()','Markers3D_BackendUpdateRW(marker)')
    lifetime='''CMatrix matrix;
	matrix.Attach(m_Matrix.m_attachment);
	matrix.Scale(m_fSize);
	matrix.UpdateRW();'''
    assert lifetime in body
    body=body.replace(lifetime,'''Markers3D_BackendWithRenderMatrix(marker,Markers3D_RenderMatrix);
}

static void
Markers3D_RenderMatrix(void *marker,void *matrix)
{
    Markers3D_BackendAttachMatrix(matrix,Markers3D_BackendAttachment(marker));
    float scale=m_fSize;
    Markers3DBasis basis=Markers3D_BackendMatrixBasis(matrix);
    for(int i=0;i<3;i++){
        // Scale the original nine basis coefficients in their original order
        for(int j=0;j<3;j++)*basis.axis[i][j]*=scale;
    }
    Markers3D_BackendPublishMatrix(matrix);''')
    body=body.replace('RwFrameUpdateObjects(','Markers3D_BackendUpdateFrame(').replace('SetBrightMarkerColours(','Markers3D_BackendBright(').replace('RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE)','Markers3D_BackendZWrite(false)').replace('RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE)','Markers3D_BackendZWrite(true)').replace('RpAtomicRender(m_pAtomic)','Markers3D_BackendDrawAtomic(Markers3D_BackendAtomic(marker))').replace('ReSetAmbientAndDirectionalColours()','Markers3D_BackendRestoreLighting()')
    fields=['m_nType','m_bIsUsed','m_bFindZOnNextPlacement','m_nIdentifier','m_nPulsePeriod','m_nRotateRate','m_nStartTime','m_fPulseFraction','m_fStdSize','m_fSize','m_fBrightness','m_fCameraRange']
    for name in fields:body=re.sub(r'(?<![\w.])'+name+r'\b','(*fields.'+name+')',body)
    for name in ['red','green','blue','alpha']:body=body.replace('m_Color.'+name,'(*fields.'+name+')')
    body=body.replace('\n{\n','\n{\n    // Preserve each original resource operation and live field-write order\n    Markers3DFields fields=Markers3D_BackendFields(marker);\n')
    assert not re.search(r'\b(?:C3dMarker|CMatrix|RpAtomic|RwFrame|RpGeometry|m_pAtomic|m_pMaterial|m_Matrix|m_Color|nil)\b',body)
    return '\nstatic void Markers3D_RenderMatrix(void *marker,void *matrix);\n\n'+body+'\n'

def calls(source):
    # Replace only record members and their complete controller entry points
    if 'bool\nC3dMarker::AddMarker(' in source:source=source.replace(record_section(source),'')
    for result,name,args in [('void','Delete','void *marker'),('void','Render','void *marker'),('bool','Add',ARGS)]:
        pattern=result+r' Markers3D_Backend'+name+r'\([^\n]*\)\n\{[\s\S]*?\n\}\n'
        source=re.sub(pattern,'',source)
        source=re.sub(r'^'+result+r' Markers3D_Backend'+name+r'\([^\n]*\);\n','',source,flags=re.M)
        source=source.replace('Markers3D_Backend'+name+'(','Markers3D_Record'+name+'(')
    if 'class C3dMarker\n' in source:
        source=source.replace('class C3dMarker\n{\npublic:','struct C3dMarker\n{')
        source=re.sub(r'^\t(?:bool AddMarker\([^\n]*\);|void DeleteMarkerObject\(\);|void Render\(\);)\n','',source,flags=re.M)
    source=source.replace('class C3dMarker;','struct C3dMarker;')
    if '#include "Script.h"\nstatic C3dMarker MarkerOwners' in source:
        source=source.replace('#include "Script.h"\nstatic C3dMarker MarkerOwners','#include "Script.h"\n#include "VisibilityPlugins.h"\n'+HOST+'static C3dMarker MarkerOwners')
    if 'typedef struct Markers3DFields' in source and 'Markers3DBasis' not in source:
        source=source.replace('#ifdef __cplusplus\n}\n#endif',DECLARATIONS+'#ifdef __cplusplus\n}\n#endif')
    return credits.calls(source)

def kernel(source,original):
    # Append the complete C record implementation to its existing C controller
    assert source.endswith('//- rouz edit (ChatGPT)\n')
    return calls(source[:-len('//- rouz edit (ChatGPT)\n')])+methods(original)+'//- rouz edit (ChatGPT)\n'

def project(source):
    # Keep the existing paired compilation units and all object names intact
    return credits.project(source)
