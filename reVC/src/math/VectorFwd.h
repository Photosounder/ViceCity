//+ rouz edit (Claude)
#pragma once

// Forward declarations of the shared vector and matrix types, so C headers can take pointers to them without including the full definitions
#ifdef __cplusplus
class CVector;
struct CVector2D;
class CMatrix;
#else
typedef struct CVector CVector;
typedef struct CVector2D CVector2D;
typedef struct CMatrix CMatrix;
#endif
//- rouz edit (Claude)
