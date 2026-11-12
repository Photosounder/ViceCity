//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "VectorMath.h"

#ifdef __cplusplus
extern "C" {
#endif
// Movement an animation of an animation group makes by its last key frame, zero when it has no movement, the result is false when the animation has no key frames and the offset is left alone
bool PedAnim_LoadOffset(int32_t groupId, int32_t animId, VectorMath3 *out);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
