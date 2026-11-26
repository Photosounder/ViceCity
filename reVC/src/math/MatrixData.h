//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>

// Plain data of a CMatrix: right, forward, up and position rows of four floats, then the RenderWare attachment
#ifdef __cplusplus
#define MATRIXDATA_ATTACHMENT_TYPE RwMatrix
#else
#define MATRIXDATA_ATTACHMENT_TYPE void
#endif

// Fields shared by CMatrixData and the C view of CMatrix
#define CMATRIXDATA_FIELDS \
	union { \
		float f[4][4]; \
		struct { \
			float rx, ry, rz, rw; \
			float fx, fy, fz, fw; \
			float ux, uy, uz, uw; \
			float px, py, pz, pw; \
		}; \
	}; \
	MATRIXDATA_ATTACHMENT_TYPE *m_attachment; \
	bool m_hasRwMatrix;	/* are we the owner? */

typedef struct CMatrixData {
	CMATRIXDATA_FIELDS
} CMatrixData;
//- rouz edit (Claude)
