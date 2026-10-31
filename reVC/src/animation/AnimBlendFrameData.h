//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "KeyFrames.h"
enum {
    ANIM_FRAME_IGNORE_ROTATION=2,
    ANIM_FRAME_IGNORE_TRANSLATION=4,
    ANIM_FRAME_VELOCITY_EXTRACTION=8,
    ANIM_FRAME_VELOCITY_EXTRACTION_3D=16,
    ANIM_FRAME_UPDATE_KEYFRAMES=32,
    ANIM_FRAME_COMPRESSED=64,
};
typedef struct AnimBlendFrameData {
    uint8_t flag;
    KeyFrameVector3 resetPos;
    union {void *frame;void *hanimFrame;};
    int32_t nodeID;
} AnimBlendFrameData;
//- rouz edit (ChatGPT)
