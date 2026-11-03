//+ rouz edit (ChatGPT)
#include "config.h"
#include "StreamingData.h"
#include <stdint.h>
#ifndef MASTER
void re3_assert(const char *expression, const char *filename, unsigned int line, const char *function);
#define StreamingImage_Assert(expr) ((void)((!!(expr)) || (re3_assert(#expr, __FILE__, __LINE__, __func__), 0)))
#else
#define StreamingImage_Assert(expr) (expr)
#endif

int32_t
Streaming_GetCdImageOffset(int32_t lastPosn)
{
    // Preserve the original image distance comparisons and last-image update
	int offset, off;
	int i, img;
	int dist, mindist;

	img = -1;
	mindist = INT32_MAX;
	offset = Streaming_imageOffsets[Streaming_lastImageRead];
	if(lastPosn <= offset || lastPosn > offset + Streaming_imageSize){
		// last read position is not in last image
		for(i = 0; i < NUMCDIMAGES; i++){
			off = Streaming_imageOffsets[i];
			if(off == -1) continue;
			if((uint32_t)lastPosn > (uint32_t)off)
				// after start of image, get distance from end
				// negative if before end!
				dist = lastPosn - (off + Streaming_imageSize);
			else
				// before image, get offset to start
				// this will never be negative
				dist = off - lastPosn;
			if(dist < mindist){
				img = i;
				mindist = dist;
			}
		}
		StreamingImage_Assert(img >= 0);
		offset = Streaming_imageOffsets[img];
		Streaming_lastImageRead = img;
	}
	return offset;
}
//- rouz edit (ChatGPT)
