//+ rouz edit (ChatGPT)
#include "Lines.h"
#include "Lines_backend.h"
#include <math.h>
#include "config.h"

typedef struct LinesRGBA { uint8_t red, green, blue, alpha; } LinesRGBA;

#ifdef THIS_IS_STUPID
static float
Lines_Square(float x)
{
	// Preserve the original float square helper
	return x*x;
}
#endif

// This is super inefficient, why split the line into segments at all?
void
Lines_RenderLineWithClipping(float x1, float y1, float z1, float x2, float y2, float z2, uint32_t c1, uint32_t c2)
{
	// Obtain the original persistent buffer before preparing either rendering path
	void *v = Lines_BackendVertices();
#ifdef THIS_IS_STUPID
	// Preserve the original segment count and interpolation arithmetic
	int i;
	float f1, f2;
	float len = sqrt(Lines_Square(x1-x2) + Lines_Square(y1-y2) + Lines_Square(z1-z2));
	int numsegs = len/1.5f + 1.0f;

	// Decode the packed endpoint colors into the original byte channels
	LinesRGBA col1;
	col1.red = c1>>24;
	col1.green = c1>>16;
	col1.blue = c1>>8;
	col1.alpha = c1;
	LinesRGBA col2;
	col2.red = c2>>24;
	col2.green = c2>>16;
	col2.blue = c2>>8;
	col2.alpha = c2;

	// Interpolate each segment in the original vertex-update order
	float dx = x2 - x1;
	float dy = y2 - y1;
	float dz = z2 - z1;
	for(i = 0; i < numsegs; i++){
		f1 = (float)i/numsegs;
		f2 = (float)(i+1)/numsegs;

		// Prepare the two endpoint colors and positions for this segment
		Lines_BackendSetRGBA(v, 0, (int)(col1.red + (col2.red-col1.red)*f1),
			(int)(col1.green + (col2.green-col1.green)*f1),
			(int)(col1.blue + (col2.blue-col1.blue)*f1),
			(int)(col1.alpha + (col2.alpha-col1.alpha)*f1));
		Lines_BackendSetRGBA(v, 1, (int)(col1.red + (col2.red-col1.red)*f2),
			(int)(col1.green + (col2.green-col1.green)*f2),
			(int)(col1.blue + (col2.blue-col1.blue)*f2),
			(int)(col1.alpha + (col2.alpha-col1.alpha)*f2));
		Lines_BackendSetPos(v, 0, x1 + dx*f1, y1 + dy*f1, z1 + dz*f1);
		Lines_BackendSetPos(v, 1, x1 + dx*f2, y1 + dy*f2, z1 + dz*f2);

		// Run the hook and draw only when transformation succeeds
		Lines_BackendLittleTest();
		if(Lines_BackendTransform(v)){
			Lines_BackendRenderLine();
			Lines_BackendEnd();
		}
	}
#else
	// Decode the packed endpoint colors into the original byte channels
	LinesRGBA col1;
	col1.red = c1>>24;
	col1.green = c1>>16;
	col1.blue = c1>>8;
	col1.alpha = c1;
	LinesRGBA col2;
	col2.red = c2>>24;
	col2.green = c2>>16;
	col2.blue = c2>>8;
	col2.alpha = c2;

	// Prepare the original two endpoints without segmenting the line
	Lines_BackendSetRGBA(v, 0, col1.red, col1.green, col1.blue, col1.alpha);
	Lines_BackendSetRGBA(v, 1, col2.red, col2.green, col2.blue, col2.alpha);
	Lines_BackendSetPos(v, 0, x1, y1, z1);
	Lines_BackendSetPos(v, 1, x2, y2, z2);
	// Run the hook and draw only when transformation succeeds
	Lines_BackendLittleTest();
	if(Lines_BackendTransform(v)){
		Lines_BackendRenderLine();
		Lines_BackendEnd();
	}
#endif
}
//- rouz edit (ChatGPT)
