//+ rouz edit (ChatGPT)
#include "AnimManager.h"
#include "ModelIndices.h"
#include "AnimBlendAssociation_flags.h"
#define ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))
static CAnimBlock ms_aAnimBlocks[NUMANIMBLOCKS];
static CAnimBlendHierarchy ms_aAnimations[NUMANIMATIONS];
static int32_t ms_numAnimBlocks;
static int32_t ms_numAnimations;
static CAnimBlendAssocGroup *ms_aAnimAssocGroups;
//+ rouz edit (ChatGPT)
static CLinkList ms_animCache;
//- rouz edit (ChatGPT)

AnimAssocDesc aStdAnimDescs[] = {
	{ ANIM_STD_WALK, ASSOC_REPEAT | ASSOC_MOVEMENT | ASSOC_HAS_TRANSLATION | ASSOC_WALK },
	{ ANIM_STD_RUN, ASSOC_REPEAT | ASSOC_MOVEMENT | ASSOC_HAS_TRANSLATION | ASSOC_WALK },
	{ ANIM_STD_RUNFAST, ASSOC_REPEAT | ASSOC_MOVEMENT | ASSOC_HAS_TRANSLATION | ASSOC_WALK },
	{ ANIM_STD_IDLE, ASSOC_REPEAT },
	{ ANIM_STD_STARTWALK, ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_RUNSTOP1, ASSOC_DELETEFADEDOUT | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_RUNSTOP2, ASSOC_DELETEFADEDOUT | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_IDLE_CAM, ASSOC_REPEAT | ASSOC_PARTIAL },
	{ ANIM_STD_IDLE_HBHB, ASSOC_REPEAT | ASSOC_PARTIAL },
	{ ANIM_STD_IDLE_TIRED, ASSOC_REPEAT },
	{ ANIM_STD_IDLE_BIGGUN, ASSOC_REPEAT | ASSOC_PARTIAL },
	{ ANIM_STD_CHAT, ASSOC_REPEAT | ASSOC_PARTIAL },
	{ ANIM_STD_HAILTAXI, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_KO_FRONT, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION | ASSOC_FRONTAL },
	{ ANIM_STD_KO_LEFT, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION | ASSOC_FRONTAL },
	{ ANIM_STD_KO_BACK, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION | ASSOC_FRONTAL },
	{ ANIM_STD_KO_RIGHT, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION | ASSOC_FRONTAL },
	{ ANIM_STD_KO_SHOT_FACE, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION | ASSOC_FRONTAL },
	{ ANIM_STD_KO_SHOT_STOMACH, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_KO_SHOT_ARM_L, ASSOC_PARTIAL | ASSOC_FRONTAL },
	{ ANIM_STD_KO_SHOT_ARM_R, ASSOC_PARTIAL | ASSOC_FRONTAL },
	{ ANIM_STD_KO_SHOT_LEG_L, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_KO_SHOT_LEG_R, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_SPINFORWARD_LEFT, ASSOC_PARTIAL | ASSOC_FRONTAL },
	{ ANIM_STD_SPINFORWARD_RIGHT, ASSOC_PARTIAL | ASSOC_FRONTAL },
	{ ANIM_STD_HIGHIMPACT_FRONT, ASSOC_PARTIAL },
	{ ANIM_STD_HIGHIMPACT_LEFT, ASSOC_PARTIAL },
	{ ANIM_STD_HIGHIMPACT_BACK, ASSOC_PARTIAL | ASSOC_FRONTAL },
	{ ANIM_STD_HIGHIMPACT_RIGHT, ASSOC_PARTIAL },
	{ ANIM_STD_HITBYGUN_FRONT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_NOWALK },
	{ ANIM_STD_HITBYGUN_LEFT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_NOWALK },
	{ ANIM_STD_HITBYGUN_BACK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_NOWALK },
	{ ANIM_STD_HITBYGUN_RIGHT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_NOWALK },
	{ ANIM_STD_HIT_FRONT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_HIT_LEFT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_HIT_BACK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_HIT_RIGHT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_HIT_FLOOR, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_HIT_BODYBLOW, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_HIT_CHEST, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_HIT_HEAD, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_HIT_WALK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_HIT_WALL, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_HIT_FLOOR_FRONT, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_FRONTAL },
	{ ANIM_STD_HIT_BEHIND, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_FIGHT_IDLE, ASSOC_REPEAT },
	{ ANIM_STD_FIGHT_2IDLE, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_FIGHT_SHUFFLE_F, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_FIGHT_BODYBLOW, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_FIGHT_HEAD, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_FIGHT_KICK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_FIGHT_KNEE, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_FIGHT_LHOOK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_FIGHT_PUNCH, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_FIGHT_ROUNDHOUSE, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_FIGHT_LONGKICK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_PARTIAL_PUNCH, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_NOWALK },
	{ ANIM_STD_FIGHT_JAB, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_FIGHT_ELBOW_L, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_FIGHT_ELBOW_R, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_FIGHT_BKICK_L, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_FIGHT_BKICK_R, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_DETONATE, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_PUNCH, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_PARTIALPUNCH, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_KICKGROUND, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_THROW_UNDER, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_FIGHT_SHUFFLE_B, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_JACKEDCAR_RHS, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_JACKEDCAR_LO_RHS, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_JACKEDCAR_LHS, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_JACKEDCAR_LO_LHS, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_QUICKJACK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_QUICKJACKED, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_ALIGN_DOOR_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_ALIGNHI_DOOR_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_OPEN_DOOR_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CARDOOR_LOCKED_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_PULL_OUT_PED_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_PULL_OUT_PED_LO_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_GET_IN_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_GET_IN_LO_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_CLOSE_DOOR_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_CLOSE_DOOR_LO_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_CLOSE_DOOR_ROLLING_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_CLOSE_DOOR_ROLLING_LO_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_JUMP_IN_LO_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_GETOUT_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_GETOUT_LO_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_CLOSE_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_ALIGN_DOOR_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_ALIGNHI_DOOR_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_OPEN_DOOR_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CARDOOR_LOCKED_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_PULL_OUT_PED_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_PULL_OUT_PED_LO_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_GET_IN_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_GET_IN_LO_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_CLOSE_DOOR_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_CLOSE_DOOR_LO_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_SHUFFLE_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_SHUFFLE_LO_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_SIT, ASSOC_DELETEFADEDOUT},
	{ ANIM_STD_CAR_SIT_LO, ASSOC_DELETEFADEDOUT},
	{ ANIM_STD_CAR_SIT_P, ASSOC_DELETEFADEDOUT},
	{ ANIM_STD_CAR_SIT_P_LO, ASSOC_DELETEFADEDOUT},
	{ ANIM_STD_CAR_DRIVE_LEFT, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_STD_CAR_DRIVE_RIGHT, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_STD_CAR_DRIVE_LEFT_LO, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_STD_CAR_DRIVE_RIGHT_LO, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_STD_CAR_DRIVEBY_LEFT, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_STD_CAR_DRIVEBY_RIGHT, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_STD_CAR_DRIVEBY_LEFT_LO, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_STD_CAR_DRIVEBY_RIGHT_LO, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_STD_CAR_LOOKBEHIND, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_STD_BOAT_DRIVE, ASSOC_DELETEFADEDOUT | ASSOC_DRIVING },
	{ ANIM_STD_BOAT_DRIVE_LEFT, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_STD_BOAT_DRIVE_RIGHT, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_STD_BOAT_LOOKBEHIND, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_STD_BIKE_PICKUP_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_BIKE_PICKUP_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_BIKE_PULLUP_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_BIKE_PULLUP_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_BIKE_ELBOW_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_BIKE_ELBOW_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_BIKE_FALLOFF, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_BIKE_FALLBACK, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_GETOUT_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_GETOUT_LO_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_CLOSE_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CAR_HOOKERTALK, ASSOC_REPEAT | ASSOC_PARTIAL },
	{ ANIM_STD_TRAIN_GETIN, ASSOC_PARTIAL },
	{ ANIM_STD_TRAIN_GETOUT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CRAWLOUT_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_CRAWLOUT_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_ROLLOUT_LHS, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION | ASSOC_HAS_X_TRANSLATION },
	{ ANIM_STD_ROLLOUT_RHS, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION | ASSOC_HAS_X_TRANSLATION },
	{ ANIM_STD_GET_UP, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_GET_UP_LEFT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_GET_UP_RIGHT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_GET_UP_FRONT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_JUMP_LAUNCH, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_JUMP_GLIDE, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_JUMP_LAND, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_FALL, ASSOC_REPEAT | ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_FALL_GLIDE, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_FALL_LAND, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_FALL_COLLAPSE, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_FALL_ONBACK, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_FALL_ONFRONT, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_FRONTAL },
	{ ANIM_STD_EVADE_STEP, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_EVADE_DIVE, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION | ASSOC_FRONTAL },
	{ ANIM_STD_XPRESS_SCRATCH, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_IDLE },
	{ ANIM_STD_ROADCROSS, ASSOC_REPEAT | ASSOC_PARTIAL },
	{ ANIM_STD_TURN180, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_ARREST, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_DROWN, ASSOC_PARTIAL },
	{ ANIM_STD_DUCK_DOWN, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_DUCK_LOW, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_DUCK_WEAPON, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_RBLOCK_SHOOT, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_HANDSUP, ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_HANDSCOWER, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_STD_PARTIAL_FUCKU, ASSOC_DELETEFADEDOUT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_NOWALK },
	{ ANIM_STD_PHONE_IN, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_PHONE_OUT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_PHONE_TALK, ASSOC_REPEAT | ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
	{ ANIM_STD_SEAT_DOWN, ASSOC_DELETEFADEDOUT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_SEAT_UP, ASSOC_DELETEFADEDOUT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_SEAT_IDLE, ASSOC_REPEAT | ASSOC_DELETEFADEDOUT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_SEAT_RVRS, ASSOC_DELETEFADEDOUT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_ATM, ASSOC_DELETEFADEDOUT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_ABSEIL, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL },
};
AnimAssocDesc aVanAnimDescs[] = {
	{ ANIM_STD_VAN_OPEN_DOOR_REAR_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_VAN_GET_IN_REAR_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_VAN_CLOSE_DOOR_REAR_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_VAN_GET_OUT_REAR_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_VAN_OPEN_DOOR_REAR_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_VAN_GET_IN_REAR_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_VAN_CLOSE_DOOR_REAR_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_VAN_GET_OUT_REAR_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
};
AnimAssocDesc aCoachAnimDescs[] = {
	{ ANIM_STD_COACH_OPEN_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_COACH_OPEN_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_COACH_GET_IN_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_COACH_GET_IN_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STD_COACH_GET_OUT_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
};
AnimAssocDesc aBikeAnimDescs[] = {
	{ ANIM_BIKE_RIDE, ASSOC_DELETEFADEDOUT},
	{ ANIM_BIKE_READY, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_BIKE_LEFT, ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_BIKE_RIGHT, ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_BIKE_LEANB, ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_BIKE_LEANF, ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_BIKE_WALKBACK, ASSOC_REPEAT | ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_BIKE_JUMPON_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_BIKE_JUMPON_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_BIKE_KICK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_BIKE_HIT, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_BIKE_GETOFF_LHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_BIKE_GETOFF_RHS, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_BIKE_GETOFF_BACK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
	{ ANIM_BIKE_DRIVEBY_LHS, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_BIKE_DRIVEBY_RHS, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_BIKE_DRIVEBY_FORWARD, ASSOC_DELETEFADEDOUT | ASSOC_PARTIAL | ASSOC_DRIVING },
	{ ANIM_BIKE_RIDE_P, ASSOC_DELETEFADEDOUT | ASSOC_DRIVING },
};
AnimAssocDesc aMeleeAnimDescs[] = {
	{ ANIM_MELEE_ATTACK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_MELEE_ATTACK_2ND, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_MELEE_ATTACK_START, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_NOWALK },
	{ ANIM_MELEE_IDLE_FIGHTMODE, ASSOC_REPEAT },
	{ ANIM_MELEE_ATTACK_FINISH, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION },
};
AnimAssocDesc aSwingAnimDescs[] = {
	{ ANIM_MELEE_ATTACK, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_MELEE_ATTACK_2ND, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_MELEE_ATTACK_START, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_MELEE_IDLE_FIGHTMODE, ASSOC_REPEAT },
	{ ANIM_MELEE_ATTACK_FINISH, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
};
AnimAssocDesc aWeaponAnimDescs[] = {
	{ ANIM_WEAPON_FIRE, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_WEAPON_CROUCHFIRE, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_WEAPON_RELOAD, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_WEAPON_CROUCHRELOAD, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_WEAPON_FIRE_3RD, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
};
AnimAssocDesc aMedicAnimDescs[] = {
	{ ANIM_MEDIC_CPR, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
};
AnimAssocDesc aSunbatheAnimDescs[] = {
	{ ANIM_SUNBATHE_IDLE, ASSOC_REPEAT | ASSOC_PARTIAL },
	{ ANIM_SUNBATHE_DOWN, ASSOC_REPEAT | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION | ASSOC_HAS_X_TRANSLATION },
	{ ANIM_SUNBATHE_UP, ASSOC_REPEAT | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION | ASSOC_HAS_X_TRANSLATION },
	{ ANIM_SUNBATHE_ESCAPE, ASSOC_REPEAT | ASSOC_PARTIAL | ASSOC_HAS_TRANSLATION | ASSOC_HAS_X_TRANSLATION },
};
AnimAssocDesc aPlayerIdleAnimDescs[] = {
	{ ANIM_PLAYER_IDLE1, ASSOC_DELETEFADEDOUT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_PLAYER_IDLE2, ASSOC_DELETEFADEDOUT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_PLAYER_IDLE3, ASSOC_DELETEFADEDOUT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_PLAYER_IDLE4, ASSOC_DELETEFADEDOUT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
};
AnimAssocDesc aRiotAnimDescs[] = {
	{ ANIM_RIOT_ANGRY, ASSOC_REPEAT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_RIOT_ANGRY_B, ASSOC_REPEAT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_RIOT_CHANT, ASSOC_REPEAT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_RIOT_PUNCHES, ASSOC_REPEAT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_RIOT_SHOUT, ASSOC_REPEAT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_RIOT_CHALLENGE, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_RIOT_FUCKYOU, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
};
AnimAssocDesc aStripAnimDescs[] = {
	{ ANIM_STRIP_A, ASSOC_REPEAT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STRIP_B, ASSOC_REPEAT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STRIP_C, ASSOC_REPEAT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STRIP_D, ASSOC_REPEAT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STRIP_E, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STRIP_F, ASSOC_REPEAT | ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
	{ ANIM_STRIP_G, ASSOC_FADEOUTWHENDONE | ASSOC_PARTIAL },
};
#ifdef PC_PLAYER_CONTROLS
AnimAssocDesc aStdAnimDescsSide[] = {
	{ ANIM_STD_WALK, ASSOC_REPEAT | ASSOC_MOVEMENT | ASSOC_HAS_TRANSLATION | ASSOC_HAS_X_TRANSLATION | ASSOC_WALK },
	{ ANIM_STD_RUN, ASSOC_REPEAT | ASSOC_MOVEMENT | ASSOC_HAS_TRANSLATION | ASSOC_HAS_X_TRANSLATION | ASSOC_WALK },
	{ ANIM_STD_RUNFAST, ASSOC_REPEAT | ASSOC_MOVEMENT | ASSOC_HAS_TRANSLATION | ASSOC_HAS_X_TRANSLATION | ASSOC_WALK },
	{ ANIM_STD_IDLE, ASSOC_REPEAT },
	{ ANIM_STD_STARTWALK, ASSOC_HAS_TRANSLATION | ASSOC_HAS_X_TRANSLATION },
};
#endif
char const* aStdAnimations[] = {
	"walk_civi",
	"run_civi",
	"sprint_panic",
	"idle_stance",
	"walk_start",
	"run_stop",
	"run_stopR",
	"idle_hbhb",
	"idle_hbhb",
	"idle_tired",
	"idle_armed",
	"idle_chat",
	"idle_taxi",
	"KO_shot_front",
	"KO_shot_front",
	"KO_shot_front",
	"KO_shot_front",
	"KO_shot_face",
	"KO_shot_stom",
	"KO_shot_arml",
	"KO_shot_armR",
	"KO_shot_legl",
	"KO_shot_legR",
	"KD_left",
	"KD_right",
	"KO_skid_front",
	"KO_spin_R",
	"KO_skid_back",
	"KO_spin_L",
	"SHOT_partial",
	"SHOT_leftP",
	"SHOT_partial",
	"SHOT_rightP",
	"HIT_front",
	"HIT_L",
	"HIT_back",
	"HIT_R",
	"FLOOR_hit",
	"HIT_bodyblow",
	"HIT_chest",
	"HIT_head",
	"HIT_walk",
	"HIT_wall",
	"FLOOR_hit_f",
	"HIT_behind",
	"FIGHTIDLE",
	"FIGHT2IDLE",
	"FIGHTsh_F",
	"FIGHTbodyblow",
	"FIGHThead",
	"FIGHTkick",
	"FIGHTknee",
	"FIGHTLhook",
	"FIGHTpunch",
	"FIGHTrndhse",
	"FIGHTlngkck",
	"FIGHTppunch",
	"FIGHTjab",
	"FIGHTelbowL",
	"FIGHTelbowR",
	"FIGHTbkickL",
	"FIGHTbkickR",
	"bomber",
	"punchR",
	"FIGHTppunch",
	"KICK_floor",
	"WEAPON_throwu",
	"FIGHTsh_back",
	"car_jackedRHS",
	"car_LjackedRHS",
	"car_jackedLHS",
	"car_LjackedLHS",
	"CAR_Qjack",
	"CAR_Qjacked",
	"CAR_align_LHS",
	"CAR_alignHI_LHS",
	"CAR_open_LHS",
	"CAR_doorlocked_LHS",
	"CAR_pullout_LHS",
	"CAR_pulloutL_LHS",
	"CAR_getin_LHS",
	"CAR_getinL_LHS",
	"CAR_closedoor_LHS",
	"CAR_closedoorL_LHS",
	"CAR_rolldoor",
	"CAR_rolldoorLO",
	"CAR_jumpin_LHS",
	"CAR_getout_LHS",
	"CAR_getoutL_LHS",
	"CAR_close_LHS",
	"CAR_align_RHS",
	"CAR_alignHI_RHS",
	"CAR_open_RHS",
	"CAR_doorlocked_RHS",
	"CAR_pullout_RHS",
	"CAR_pulloutL_RHS",
	"CAR_getin_RHS",
	"CAR_getinL_RHS",
	"CAR_closedoor_RHS",
	"CAR_closedoorL_RHS",
	"CAR_shuffle_RHS",
	"CAR_Lshuffle_RHS",
	"CAR_sit",
	"CAR_Lsit",
	"CAR_sitp",
	"CAR_sitpLO",
	"DRIVE_L",
	"Drive_R",
	"Drive_LO_l",
	"Drive_LO_R",
	"Driveby_L",
	"Driveby_R",
	"DrivebyL_L",
	"DrivebyL_R",
	"CAR_LB",
	"DRIVE_BOAT",
	"DRIVE_BOAT_L",
	"DRIVE_BOAT_R",
	"DRIVE_BOAT_back",
	"BIKE_pickupR",
	"BIKE_pickupL",
	"BIKE_pullupR",
	"BIKE_pullupL",
	"BIKE_elbowR",
	"BIKE_elbowL",
	"BIKE_fall_off",
	"BIKE_fallR",
	"CAR_getout_RHS",
	"CAR_getoutL_RHS",
	"CAR_close_RHS",
	"car_hookertalk",
	"idle_stance",
	"idle_stance",
	"CAR_crawloutRHS",
	"CAR_crawloutRHS",
	"CAR_rollout_LHS",
	"CAR_rollout_LHS",
	"Getup",
	"Getup",
	"Getup",
	"Getup_front",
	"JUMP_launch",
	"JUMP_glide",
	"JUMP_land",
	"FALL_fall",
	"FALL_glide",
	"FALL_land",
	"FALL_collapse",
	"FALL_back",
	"FALL_front",
	"EV_step",
	"EV_dive",
	"XPRESSscratch",
	"roadcross",
	"TURN_180",
	"ARRESTgun",
	"DROWN",
	"DUCK_down",
	"DUCK_low",
	"WEAPON_crouch",
	"RBLOCK_Cshoot",
	"handsup",
	"handsCOWER",
	"FUCKU",
	"PHONE_in",
	"PHONE_out",
	"PHONE_talk",
	"SEAT_down",
	"SEAT_up",
	"SEAT_idle",
	"SEAT_down",
	"ATM",
	"abseil",
};
char const* aVanAnimations[] = {
	"VAN_openL",
	"VAN_getinL",
	"VAN_closeL",
	"VAN_getoutL",
	"VAN_open",
	"VAN_getin",
	"VAN_close",
	"VAN_getout",
};
char const* aCoachAnimations[] = {
	"COACH_opnL",
	"COACH_opnL",
	"COACH_inL",
	"COACH_inL",
	"COACH_outL",
};
char const* aBikesAnimations[] = {
	"BIKEs_Ride",
	"BIKEs_Still",
	"BIKEs_Left",
	"BIKEs_Right",
	"BIKEs_Back",
	"BIKEs_Fwd",
	"BIKEs_pushes",
	"BIKEs_jumponR",
	"BIKEs_jumponL",
	"BIKEs_kick",
	"BIKEs_hit",
	"BIKEs_getoffRHS",
	"BIKEs_getoffLHS",
	"BIKEs_getoffBACK",
	"BIKEs_drivebyLHS",
	"BIKEs_drivebyRHS",
	"BIKEs_drivebyFT",
	"BIKEs_passenger",
};
char const* aBikevAnimations[] = {
	"BIKEv_Ride",
	"BIKEv_Still",
	"BIKEv_Left",
	"BIKEv_Right",
	"BIKEv_Back",
	"BIKEv_Fwd",
	"BIKEv_pushes",
	"BIKEv_jumponR",
	"BIKEv_jumponL",
	"BIKEv_kick",
	"BIKEv_hit",
	"BIKEv_getoffRHS",
	"BIKEv_getoffLHS",
	"BIKEv_getoffBACK",
	"BIKEv_drivebyLHS",
	"BIKEv_drivebyRHS",
	"BIKEv_drivebyFT",
	"BIKEv_passenger",
};
char const* aBikehAnimations[] = {
	"BIKEh_Ride",
	"BIKEh_Still",
	"BIKEh_Left",
	"BIKEh_Right",
	"BIKEh_Back",
	"BIKEh_Fwd",
	"BIKEh_pushes",
	"BIKEh_jumponR",
	"BIKEh_jumponL",
	"BIKEh_kick",
	"BIKEh_hit",
	"BIKEh_getoffRHS",
	"BIKEh_getoffLHS",
	"BIKEh_getoffBACK",
	"BIKEh_drivebyLHS",
	"BIKEh_drivebyRHS",
	"BIKEh_drivebyFT",
	"BIKEh_passenger",
};
char const* aBikedAnimations[] = {
	"BIKEd_Ride",
	"BIKEd_Still",
	"BIKEd_Left",
	"BIKEd_Right",
	"BIKEd_Back",
	"BIKEd_Fwd",
	"BIKEd_pushes",
	"BIKEd_jumponR",
	"BIKEd_jumponL",
	"BIKEd_kick",
	"BIKEd_hit",
	"BIKEd_getoffRHS",
	"BIKEd_getoffLHS",
	"BIKEd_getoffBACK",
	"BIKEd_drivebyLHS",
	"BIKEd_drivebyRHS",
	"BIKEd_drivebyFT",
	"BIKEd_passenger",
};
char const* aUnarmedAnimations[] = {
	"punchR",
	"KICK_floor",
	"FIGHTppunch",
};
char const* aScrewdriverAnimations[] = {
	"FIGHTbodyblow",
	"FIGHTbodyblow",
	"FIGHTppunch",
	"FIGHTIDLE",
	"FIGHTbodyblow",
};
char const* aKnifeAnimations[] = {
	"WEAPON_knife_1",
	"WEAPON_knife_2",
	"knife_part",
	"WEAPON_knifeidle",
	"WEAPON_knife_3",
};
char const* aBaseballbatAnimations[] = {
	"WEAPON_bat_h",
	"WEAPON_bat_v",
	"BAT_PART",
	"WEAPON_bat_h",
	"WEAPON_golfclub",
};
char const* aGolfclubAnimations[] = {
	"WEAPON_bat_h",
	"WEAPON_golfclub",
	"BAT_PART",
	"WEAPON_bat_h",
	"WEAPON_bat_v",
};
char const* aChainsawAnimations[] = {
	"WEAPON_csaw",
	"WEAPON_csawlo",
	"csaw_part",
};
char const* aPythonAnimations[] = {
	"python_fire",
	"python_crouchfire",
	"python_reload",
	"python_crouchreload",
};
char const* aColtAnimations[] = {
	"colt45_fire",
	"colt45_crouchfire",
	"colt45_reload",
	"colt45_crouchreload",
	"colt45_cop",
};
char const* aShotgunAnimations[] = {
	"shotgun_fire",
	"shotgun_crouchfire",
};
char const* aBuddyAnimations[] = {
	"buddy_fire",
	"buddy_crouchfire",
};
char const* aTecAnimations[] = {
	"TEC_fire",
	"TEC_crouchfire",
	"TEC_reload",
	"TEC_crouchreload",
};
char const* aUziAnimations[] = {
	"UZI_fire",
	"UZI_crouchfire",
	"UZI_reload",
	"UZI_crouchreload",
};
char const* aRifleAnimations[] = {
	"RIFLE_fire",
	"RIFLE_crouchfire",
	"RIFLE_load",
	"RIFLE_crouchload",
};
char const* aM60Animations[] = {
	"M60_fire",
	"M60_fire",
	"M60_reload",
};
char const* aSniperAnimations[] = {
	"WEAPON_sniper",
};
char const* aThrowAnimations[] = {
	"WEAPON_throw",
	"WEAPON_throwu",
	"WEAPON_start_throw",
};
char const* aFlamethrowerAnimations[] = {
	"FLAME_fire",
};
char const* aMedicAnimations[] = {
	"CPR",
};
char const* aSunbatheAnimations[] = {
	"bather",
	"batherdown",
	"batherup",
	"batherscape",
};
char const* aPlayerIdleAnimations[] = {
	"stretch",
	"time",
	"shldr",
	"strleg",
};
char const* aRiotAnimations[] = {
	"riot_angry",
	"riot_angry_b",
	"riot_chant",
	"riot_punches",
	"riot_shout",
	"riot_challenge",
	"riot_fuku",
};
char const* aStripAnimations[] = {
	"strip_A",
	"strip_B",
	"strip_C",
	"strip_D",
	"strip_E",
	"strip_F",
	"strip_G",
};
char const* aLanceAnimations[] = {
	"lance",
};
char const* aPlayerAnimations[] = {
	"walk_player",
	"run_player",
	"SPRINT_civi",
	"IDLE_STANCE",
	"walk_start",
};
char const* aPlayerWithRocketAnimations[] = {
	"walk_rocket",
	"run_rocket",
	"run_rocket",
	"idle_rocket",
	"walk_start_rocket",
};
char const* aPlayer1ArmedAnimations[] = {
	"walk_player",
	"run_1armed",
	"SPRINT_civi",
	"IDLE_STANCE",
	"walk_start",
};
char const* aPlayer2ArmedAnimations[] = {
	"walk_armed",
	"run_armed",
	"run_armed",
	"idle_armed",
	"walk_start_armed",
};
char const* aPlayerBBBatAnimations[] = {
	"walk_player",
	"run_player",
	"run_player",
	"IDLE_STANCE",
	"walk_start",
};
char const* aPlayerChainsawAnimations[] = {
	"walk_csaw",
	"run_csaw",
	"run_csaw",
	"IDLE_csaw",
	"walk_start_csaw",
};
char const* aShuffleAnimations[] = {
	"WALK_shuffle",
	"RUN_civi",
	"SPRINT_civi",
	"IDLE_STANCE",
};
char const* aOldAnimations[] = {
	"walk_old",
	"run_civi",
	"sprint_civi",
	"idle_stance",
};
char const* aGang1Animations[] = {
	"walk_gang1",
	"run_gang1",
	"sprint_civi",
	"idle_stance",
};
char const* aGang2Animations[] = {
	"walk_gang2",
	"run_gang1",
	"sprint_civi",
	"idle_stance",
};
char const* aFatAnimations[] = {
	"walk_fat",
	"run_civi",
	"woman_runpanic",
	"idle_stance",
};
char const* aOldFatAnimations[] = {
	"walk_fatold",
	"run_fatold",
	"woman_runpanic",
	"idle_stance",
};
char const* aJoggerAnimations[] = {
	"JOG_maleA",
	"run_civi",
	"sprint_civi",
	"idle_stance",
};
char const* aStdWomanAnimations[] = {
	"woman_walknorm",
	"woman_run",
	"woman_runpanic",
	"woman_idlestance",
};
char const* aWomanShopAnimations[] = {
	"woman_walkshop",
	"woman_run",
	"woman_run",
	"woman_idlestance",
};
char const* aBusyWomanAnimations[] = {
	"woman_walkbusy",
	"woman_run",
	"woman_runpanic",
	"woman_idlestance",
};
char const* aSexyWomanAnimations[] = {
	"woman_walksexy",
	"woman_run",
	"woman_runpanic",
	"woman_idlestance",
};
char const* aFatWomanAnimations[] = {
	"walk_fat",
	"woman_run",
	"woman_runpanic",
	"woman_idlestance",
};
char const* aOldWomanAnimations[] = {
	"woman_walkold",
	"woman_run",
	"woman_runpanic",
	"woman_idlestance",
};
char const* aJoggerWomanAnimations[] = {
	"JOG_maleB",
	"woman_run",
	"woman_runpanic",
	"woman_idlestance",
};
char const* aPanicChunkyAnimations[] = {
	"run_fatold",
	"woman_runpanic",
	"woman_runpanic",
	"idle_stance",
};
char const* aSkateAnimations[] = {
	"skate_run",
	"skate_sprint",
	"skate_sprint",
	"skate_idle",
};
#ifdef PC_PLAYER_CONTROLS
char const* aPlayerStrafeBackAnimations[] = {
	"walk_back",
	"run_back",
	"run_back",
	"IDLE_STANCE",
	"walk_start_back",
};
char const* aPlayerStrafeLeftAnimations[] = {
	"walk_left",
	"run_left",
	"run_left",
	"IDLE_STANCE",
	"walk_start_left",
};
char const* aPlayerStrafeRightAnimations[] = {
	"walk_right",
	"run_right",
	"run_right",
	"IDLE_STANCE",
	"walk_start_right",
};
char const* aRocketStrafeBackAnimations[] = {
	"walk_rocket_back",
	"run_rocket_back",
	"run_rocket_back",
	"idle_rocket",
	"walkst_rocket_back",
};
char const* aRocketStrafeLeftAnimations[] = {
	"walk_rocket_left",
	"run_rocket_left",
	"run_rocket_left",
	"idle_rocket",
	"walkst_rocket_left",
};
char const* aRocketStrafeRightAnimations[] = {
	"walk_rocket_right",
	"run_rocket_right",
	"run_rocket_right",
	"idle_rocket",
	"walkst_rocket_right",
};
char const* aChainsawStrafeBackAnimations[] = {
	"walk_csaw_back",
	"run_csaw_back",
	"run_csaw_back",
	"idle_csaw",
	"walkst_csaw_back",
};
char const* aChainsawStrafeLeftAnimations[] = {
	"walk_csaw_left",
	"run_csaw_left",
	"run_csaw_left",
	"idle_csaw",
	"walkst_csaw_left",
};
char const* aChainsawStrafeRightAnimations[] = {
	"walk_csaw_right",
	"run_csaw_right",
	"run_csaw_right",
	"idle_csaw",
	"walkst_csaw_right",
};
#endif

#define awc(a) ARRAY_SIZE(a), a
static const AnimAssocDefinition ms_aAnimAssocDefinitions[NUM_ANIM_ASSOC_GROUPS] = {
	{ "man", "ped", MI_COP, awc(aStdAnimations), aStdAnimDescs },
	{ "van", "van", MI_COP, awc(aVanAnimations), aVanAnimDescs },
	{ "coach", "coach", MI_COP, awc(aCoachAnimations), aCoachAnimDescs },
	{ "bikes", "bikes", MI_COP, awc(aBikesAnimations), aBikeAnimDescs },
	{ "bikev", "bikev", MI_COP, awc(aBikevAnimations), aBikeAnimDescs },
	{ "bikeh", "bikeh", MI_COP, awc(aBikehAnimations), aBikeAnimDescs },
	{ "biked", "biked", MI_COP, awc(aBikedAnimations), aBikeAnimDescs },
	{ "unarmed", "ped", MI_COP, awc(aUnarmedAnimations), aMeleeAnimDescs },
	{ "screwdrv", "ped", MI_COP, awc(aScrewdriverAnimations), aMeleeAnimDescs },
	{ "knife", "knife", MI_COP, awc(aKnifeAnimations), aMeleeAnimDescs },
	{ "baseball", "baseball", MI_COP, awc(aBaseballbatAnimations), aSwingAnimDescs },
	{ "golfclub", "baseball", MI_COP, awc(aGolfclubAnimations), aSwingAnimDescs },
	{ "chainsaw", "chainsaw", MI_COP, awc(aChainsawAnimations), aMeleeAnimDescs },
	{ "python", "python", MI_COP, awc(aPythonAnimations), aWeaponAnimDescs },
	{ "colt45", "colt45", MI_COP, awc(aColtAnimations), aWeaponAnimDescs },
	{ "shotgun", "shotgun", MI_COP, awc(aShotgunAnimations), aWeaponAnimDescs },
	{ "buddy", "buddy", MI_COP, awc(aBuddyAnimations), aWeaponAnimDescs },
	{ "tec", "tec", MI_COP, awc(aTecAnimations), aWeaponAnimDescs },
	{ "uzi", "uzi", MI_COP, awc(aUziAnimations), aWeaponAnimDescs },
	{ "rifle", "rifle", MI_COP, awc(aRifleAnimations), aWeaponAnimDescs },
	{ "m60", "m60", MI_COP, awc(aM60Animations), aWeaponAnimDescs },
	{ "sniper", "sniper", MI_COP, awc(aSniperAnimations), aWeaponAnimDescs },
	{ "grenade", "grenade", MI_COP, awc(aThrowAnimations), aWeaponAnimDescs },
	{ "flame", "flame", MI_COP, awc(aFlamethrowerAnimations), aWeaponAnimDescs },
	{ "medic", "medic", MI_COP, awc(aMedicAnimations), aMedicAnimDescs },
	{ "sunbathe", "sunbathe", MI_COP, 1, aSunbatheAnimations, aSunbatheAnimDescs },	// NB: not using awc here!
	{ "playidles", "playidles", MI_COP, awc(aPlayerIdleAnimations), aPlayerIdleAnimDescs },
	{ "riot", "riot", MI_COP, awc(aRiotAnimations), aRiotAnimDescs },
	{ "strip", "strip", MI_COP, awc(aStripAnimations), aStripAnimDescs },
	{ "lance", "lance", MI_COP, awc(aLanceAnimations), aSunbatheAnimDescs },
	{ "player", "ped", MI_COP, awc(aPlayerAnimations), aStdAnimDescs },
	{ "playerrocket", "ped", MI_COP, awc(aPlayerWithRocketAnimations), aStdAnimDescs },
	{ "player1armed", "ped", MI_COP, awc(aPlayer1ArmedAnimations), aStdAnimDescs },
	{ "player2armed", "ped", MI_COP, awc(aPlayer2ArmedAnimations), aStdAnimDescs },
	{ "playerBBBat", "ped", MI_COP, awc(aPlayerBBBatAnimations), aStdAnimDescs },
	{ "playercsaw", "ped", MI_COP, awc(aPlayerChainsawAnimations), aStdAnimDescs },
	{ "shuffle", "ped", MI_COP, awc(aShuffleAnimations), aStdAnimDescs },
	{ "oldman", "ped", MI_COP, awc(aOldAnimations), aStdAnimDescs },
	{ "gang1", "ped", MI_COP, awc(aGang1Animations), aStdAnimDescs },
	{ "gang2", "ped", MI_COP, awc(aGang2Animations), aStdAnimDescs },
	{ "fatman", "ped", MI_COP, awc(aFatAnimations), aStdAnimDescs },
	{ "oldfatman", "ped", MI_COP, awc(aOldFatAnimations), aStdAnimDescs },
	{ "jogger", "ped", MI_COP, awc(aJoggerAnimations), aStdAnimDescs },
	{ "woman", "ped", MI_COP, awc(aStdWomanAnimations), aStdAnimDescs },
	{ "shopping", "ped", MI_COP, awc(aWomanShopAnimations), aStdAnimDescs },
	{ "busywoman", "ped", MI_COP, awc(aBusyWomanAnimations), aStdAnimDescs },
	{ "sexywoman", "ped", MI_COP, awc(aSexyWomanAnimations), aStdAnimDescs },
	{ "fatwoman", "ped", MI_COP, awc(aFatWomanAnimations), aStdAnimDescs },
	{ "oldwoman", "ped", MI_COP, awc(aOldWomanAnimations), aStdAnimDescs },
	{ "jogwoman", "ped", MI_COP, awc(aJoggerWomanAnimations), aStdAnimDescs },
	{ "panicchunky", "ped", MI_COP, awc(aPanicChunkyAnimations), aStdAnimDescs },
	{ "skate", "skate", MI_COP, awc(aSkateAnimations), aStdAnimDescs },
#ifdef PC_PLAYER_CONTROLS
	{ "playerback", "ped", MI_COP, awc(aPlayerStrafeBackAnimations), aStdAnimDescs },
	{ "playerleft", "ped", MI_COP, awc(aPlayerStrafeLeftAnimations), aStdAnimDescsSide },
	{ "playerright", "ped", MI_COP, awc(aPlayerStrafeRightAnimations), aStdAnimDescsSide },
	{ "rocketback", "ped", MI_COP, awc(aRocketStrafeBackAnimations), aStdAnimDescs },
	{ "rocketleft", "ped", MI_COP, awc(aRocketStrafeLeftAnimations), aStdAnimDescsSide },
	{ "rocketright", "ped", MI_COP, awc(aRocketStrafeRightAnimations), aStdAnimDescsSide },
	{ "csawback", "ped", MI_COP, awc(aChainsawStrafeBackAnimations), aStdAnimDescs },
	{ "csawleft", "ped", MI_COP, awc(aChainsawStrafeLeftAnimations), aStdAnimDescsSide },
	{ "csawright", "ped", MI_COP, awc(aChainsawStrafeRightAnimations), aStdAnimDescsSide },
#endif
};
#undef awc

//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include <stdlib.h>
#include <string.h>
#include "General.h"
#include "AnimManager_backend.h"
#include "AnimBlendAssocGroup_backend.h"
#include "AnimBlendAssociation_backend.h"
#include "AnimBlendHierarchy_backend.h"
#include "KeyFrames.h"
#ifndef MASTER
#define AnimManager_Assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define AnimManager_Assert(expression) (expression)
#endif
#define AnimManager_Debug(...) re3_debug("[DBG]: " __VA_ARGS__)
static KeyFrameVectorRef AnimManager_TranslationRef(const KeyFrameVector3 *value)
{
    // Borrow the actual C translation fields for the existing compression operation
    KeyFrameVectorRef reference = {&value->x,&value->y,&value->z};
    return reference;
}
void AnimManager_Initialise(void)
{
    // Preserve the original manager operation and its live field access order
	ms_numAnimations = 0;
	ms_numAnimBlocks = 0;
	// Zero-initialize list nodes here so CITA records the owning subsystem
	//+ rouz edit (ChatGPT)
	CLinkList_Init(&ms_animCache, (CLink*)calloc(25, sizeof(CLink)), 25);
	//- rouz edit (ChatGPT)
}
void AnimManager_Shutdown(void)
{
    // Preserve the original manager operation and its live field access order
	int i;

	for(i = 0; i < NUMANIMBLOCKS; i++)
		AnimManager_HostRemoveAnim(i);

	for(i = 0; i < ms_numAnimations; i++)
		// Use the complete C hierarchy at the original native operation boundary
		AnimHierarchy_Shutdown(&ms_aAnimations[i]);

	// Free the subsystem node array and reset its list sentinels
	//+ rouz edit (ChatGPT)
	free(ms_animCache.links);

	CLinkList_Init(&ms_animCache, NULL, 0);
	//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
	// Destroy and release association groups without invoking C++ array delete.
	for(i = 0; i < NUM_ANIM_ASSOC_GROUPS; i++)
		// Use complete C group ownership at the original native call boundary
		AnimGroup_Destroy(&ms_aAnimAssocGroups[i]);
	free(ms_aAnimAssocGroups);
//- rouz edit (ChatGPT)
}
void AnimManager_UncompressAnimation(CAnimBlendHierarchy *hier)
{
    // Preserve the original manager operation and its live field access order
	if(hier->keepCompressed){
		if(hier->totalLength == 0.0f)
			// Use the complete C hierarchy at the original native operation boundary
			AnimHierarchy_CalcTotalTimeCompressed(hier);
	}else{
		if(!hier->compressed){
			if(hier->linkPtr){
				// Use the C linked-list functions and node types
				//+ rouz edit (ChatGPT)
				CLink_Remove(hier->linkPtr);
				CLink_Insert(&ms_animCache.head, hier->linkPtr);
				//- rouz edit (ChatGPT)
			}
		}else{
			// Use the C linked-list functions and node types
			//+ rouz edit (ChatGPT)
			CLink *link = CLinkList_Insert(&ms_animCache, hier);
			//- rouz edit (ChatGPT)
			if(link == NULL){
				// Access the object payload through the C list node
				//+ rouz edit (ChatGPT)
				CAnimBlendHierarchy *lastHier = (CAnimBlendHierarchy*)ms_animCache.tail.prev->item;
				//- rouz edit (ChatGPT)
				// Use the complete C hierarchy at the original native operation boundary
				AnimHierarchy_RemoveUncompressedData(lastHier);
				// Use the C linked-list functions and node types
				//+ rouz edit (ChatGPT)
				CLinkList_Remove(&ms_animCache, ms_animCache.tail.prev);
				//- rouz edit (ChatGPT)
				lastHier->linkPtr = NULL;
				// Use the C linked-list functions and node types
				//+ rouz edit (ChatGPT)
				link = CLinkList_Insert(&ms_animCache, hier);
				//- rouz edit (ChatGPT)
			}
			hier->linkPtr = link;
			// Use the complete C hierarchy at the original native operation boundary
			AnimHierarchy_Uncompress(hier);
		}
	}
}
void AnimManager_RemoveFromUncompressedCache(CAnimBlendHierarchy *hier)
{
    // Preserve the original manager operation and its live field access order
	if(hier->linkPtr){
		// Use the C linked-list functions and node types
		//+ rouz edit (ChatGPT)
		CLinkList_Remove(&ms_animCache, hier->linkPtr);
		//- rouz edit (ChatGPT)
		hier->linkPtr = NULL;
	}
}
CAnimBlock * AnimManager_GetAnimationBlockById(int32_t block)
{
    // Return the original manager storage directly
    return &ms_aAnimBlocks[block];
}
CAnimBlock * AnimManager_GetAnimationBlockByName(const char *name)
{
    // Preserve the original manager operation and its live field access order
	int i;

	for(i = 0; i < ms_numAnimBlocks; i++)
		if(AnimManager_HostCompareNames(ms_aAnimBlocks[i].name, name) == 0)
			return &ms_aAnimBlocks[i];
	return NULL;
}
int32_t AnimManager_GetAnimationBlockIndex(const char *name)
{
    // Preserve the original manager operation and its live field access order
	int i;

	for(i = 0; i < ms_numAnimBlocks; i++)
		if(AnimManager_HostCompareNames(ms_aAnimBlocks[i].name, name) == 0)
			return i;
	return -1;
}
int32_t AnimManager_RegisterAnimBlock(const char *name)
{
    // Preserve the original manager operation and its live field access order
	CAnimBlock *animBlock = AnimManager_GetAnimationBlockByName(name);
	if(animBlock == NULL){
		animBlock = &ms_aAnimBlocks[ms_numAnimBlocks++];
		strncpy(animBlock->name, name, MAX_ANIMBLOCK_NAME);
		animBlock->numAnims = 0;
		AnimManager_Assert(animBlock->refCount == 0);
	}
	return animBlock - ms_aAnimBlocks;
}
int32_t AnimManager_GetNumRefsToAnimBlock(int32_t block)
{
    // Preserve the original manager operation and its live field access order
	return ms_aAnimBlocks[block].refCount;
}
void AnimManager_AddAnimBlockRef(int32_t block)
{
    // Preserve the original manager operation and its live field access order
	ms_aAnimBlocks[block].refCount++;
}
void AnimManager_RemoveAnimBlockRefWithoutDelete(int32_t block)
{
    // Preserve the original manager operation and its live field access order
	ms_aAnimBlocks[block].refCount--;
}
void AnimManager_RemoveAnimBlockRef(int32_t block)
{
    // Preserve the original manager operation and its live field access order
	ms_aAnimBlocks[block].refCount--;
	if(ms_aAnimBlocks[block].refCount == 0)
		AnimManager_HostRemoveAnim(block);
}
void AnimManager_RemoveAnimBlock(int32_t block)
{
    // Preserve the original manager operation and its live field access order
	int i;
	CAnimBlock *animblock;

	animblock = &ms_aAnimBlocks[block];
	AnimManager_Debug("Removing ANIMS %s\n", animblock->name);
	for(i = 0; i < NUM_ANIM_ASSOC_GROUPS; i++)
		if(ms_aAnimAssocGroups[i].animBlock == animblock)
			// Use complete C group ownership at the original native call boundary
			AnimGroup_DestroyAssociations(&ms_aAnimAssocGroups[i]);
	for(i = 0; i < animblock->numAnims; i++)
		// Use the complete C hierarchy at the original native operation boundary
		AnimHierarchy_Shutdown(&ms_aAnimations[animblock->firstIndex + i]);
	animblock->isLoaded = false;
	animblock->refCount = 0;
}
CAnimBlendHierarchy * AnimManager_GetAnimationByName(const char *name,CAnimBlock *animBlock)
{
    // Preserve the original manager operation and its live field access order
	int i;
	CAnimBlendHierarchy *hier = &ms_aAnimations[animBlock->firstIndex];

	for(i = 0; i < animBlock->numAnims; i++){
		if(AnimManager_HostCompareNames(hier->name, name) == 0)
			return hier;
		hier++;
	}
	return NULL;
}
CAnimBlendHierarchy * AnimManager_GetAnimationById(int32_t n)
{
    // Return the original manager storage directly
    return &ms_aAnimations[n];
}
const char * AnimManager_GetAnimGroupName(int32_t groupId)
{
    // Preserve the original manager operation and its live field access order
	return ms_aAnimAssocDefinitions[groupId].name;
}
CAnimBlendAssociation * AnimManager_CreateAnimAssociation(int32_t groupId,int32_t animId)
{
    // Preserve the original manager operation and its live field access order
	// Use complete C group ownership at the original native call boundary
	return AnimGroup_CopyById(&ms_aAnimAssocGroups[groupId],animId);
}
CAnimBlendAssociation * AnimManager_GetAnimAssociationById(int32_t groupId,int32_t animId)
{
    // Preserve the original manager operation and its live field access order
	// Use complete C group ownership at the original native call boundary
	return AnimGroup_GetById(&ms_aAnimAssocGroups[groupId],animId);
}
CAnimBlendAssociation * AnimManager_GetAnimAssociationByName(int32_t groupId,const char *name)
{
    // Preserve the original manager operation and its live field access order
	// Use complete C group ownership at the original native call boundary
	return AnimGroup_GetByName(&ms_aAnimAssocGroups[groupId],name);
}
CAnimBlendAssociation * AnimManager_AddAnimation(void *clump,int32_t groupId,int32_t animId)
{
    // Preserve the original manager operation and its live field access order
	CAnimBlendAssociation *anim = AnimManager_CreateAnimAssociation(groupId, animId);
	CAnimBlendClumpData *clumpData = AnimAssociation_HostClumpData(clump);
	// Use the complete C association at the original native operation boundary
	if(AnimAssociation_IsMovement(anim)){
		CAnimBlendAssociation *syncanim = NULL;
		CAnimBlendLink *link;
		for(link = clumpData->link.next; link; link = link->next){
			// Use the complete C association at the original native operation boundary
			syncanim = AnimAssociation_FromLink(link);
			// Use the complete C association at the original native operation boundary
			if(AnimAssociation_IsMovement(syncanim))
				break;
		}
		if(link){
			// Use the complete C association at the original native operation boundary
			AnimAssociation_SyncAnimation(anim,syncanim);
			anim->flags |= ASSOC_RUNNING;
		}else
			// Use the complete C association at the original native operation boundary
			AnimAssociation_Start(anim,0.0f);
	}else
		// Use the complete C association at the original native operation boundary
		AnimAssociation_Start(anim,0.0f);

	// Use the C animation owner at the original native operation boundary
	AnimBlendLink_Prepend(&clumpData->link,&anim->link);
	return anim;
}
CAnimBlendAssociation * AnimManager_AddAnimationAndSync(void *clump,CAnimBlendAssociation *syncanim,int32_t groupId,int32_t animId)
{
    // Preserve the original manager operation and its live field access order
	CAnimBlendAssociation *anim = AnimManager_CreateAnimAssociation(groupId, animId);
	CAnimBlendClumpData *clumpData = AnimAssociation_HostClumpData(clump);
	// Use the complete C association at the original native operation boundary
	if (AnimAssociation_IsMovement(anim) && syncanim){
		// Use the complete C association at the original native operation boundary
		AnimAssociation_SyncAnimation(anim,syncanim);
		anim->flags |= ASSOC_RUNNING;
	}else
		// Use the complete C association at the original native operation boundary
		AnimAssociation_Start(anim,0.0f);

	// Use the C animation owner at the original native operation boundary
	AnimBlendLink_Prepend(&clumpData->link,&anim->link);
	return anim;
}
CAnimBlendAssociation * AnimManager_BlendAnimation(void *clump,int32_t groupId,int32_t animId,float delta)
{
    // Preserve the original manager operation and its live field access order
	int removePrevAnim = 0;
	CAnimBlendClumpData *clumpData = AnimAssociation_HostClumpData(clump);
	CAnimBlendAssociation *anim = AnimManager_GetAnimAssociationById(groupId, animId);
	// Use the complete C association at the original native operation boundary
	bool isMovement = AnimAssociation_IsMovement(anim);
	// Use the complete C association at the original native operation boundary
	bool isPartial = AnimAssociation_IsPartial(anim);
	CAnimBlendLink *link;
	CAnimBlendAssociation *found = NULL, *movementAnim = NULL;
	for(link = clumpData->link.next; link; link = link->next){
		// Use the complete C association at the original native operation boundary
		anim = AnimAssociation_FromLink(link);
		// Use the complete C association at the original native operation boundary
		if(isMovement && AnimAssociation_IsMovement(anim))
			movementAnim = anim;
		if(anim->animId == animId)
			found = anim;
		else{
			// Use the complete C association at the original native operation boundary
			if(isPartial == AnimAssociation_IsPartial(anim)){
				if(anim->blendAmount > 0.0f){
					float blendDelta = -delta*anim->blendAmount;
					if(blendDelta < anim->blendDelta || !isPartial)
						anim->blendDelta = blendDelta;
				}else{
					anim->blendDelta = -1.0f;
				}
				anim->flags |= ASSOC_DELETEFADEDOUT;
				removePrevAnim = 1;
			}
		}
	}
	if(found){
		found->blendDelta = (1.0f - found->blendAmount)*delta;
		// Use the complete C association at the original native operation boundary
		if(!AnimAssociation_IsRunning(found) && found->currentTime == found->hierarchy->totalLength)
			// Use the complete C association at the original native operation boundary
			AnimAssociation_Start(found,0.0f);
	}else{
		found = AnimManager_AddAnimationAndSync(clump, movementAnim, groupId, animId);
		if(!removePrevAnim && !isPartial){
			found->blendAmount = 1.0f;
			return found;
		}
		found->blendAmount = 0.0f;
		found->blendDelta = delta;
	}
	AnimManager_UncompressAnimation(found->hierarchy);
	return found;
}
void AnimManager_LoadAnimFiles(void)
{
    // Preserve the original manager operation and its live field access order
	AnimManager_LoadAnimFileByName("ANIM\\PED.IFP");
//+ rouz edit (ChatGPT)
	// Allocate and construct association groups without invoking C++ array new.
	ms_aAnimAssocGroups = (CAnimBlendAssocGroup*)malloc(sizeof(CAnimBlendAssocGroup)*NUM_ANIM_ASSOC_GROUPS);
	for(int i = 0; i < NUM_ANIM_ASSOC_GROUPS; i++)
		// Use complete C group ownership at the original native call boundary
		AnimGroup_Init(&ms_aAnimAssocGroups[i]);
//- rouz edit (ChatGPT)
	AnimManager_CreateAnimAssocGroups();
}
void AnimManager_LoadAnimFileByName(const char *filename)
{
    // Preserve the original manager operation and its live field access order
	void *stream;
	stream = AnimManager_HostOpenStream(filename);
	AnimManager_Assert(stream);
	AnimManager_LoadAnimStream(stream, true, NULL);
	AnimManager_HostCloseStream(stream);
}
void AnimManager_LoadAnimStream(void *stream,bool compress,char (*uncompressedAnims)[32])
{
    // Preserve the original manager operation and its live field access order
	#define ROUNDSIZE(x) if((x) & 3) (x) += 4 - ((x)&3)
	typedef struct IfpHeader {
		char ident[4];
		uint32_t size;
	} IfpHeader;
	IfpHeader anpk, info, name, dgan, cpan, anim;
	char buf[256];
	int j, k, l;
	float *fbuf = (float*)buf;

	// block name
	AnimManager_HostReadStream(stream, &anpk, sizeof(IfpHeader));
	ROUNDSIZE(anpk.size);
	AnimManager_HostReadStream(stream, &info, sizeof(IfpHeader));
	ROUNDSIZE(info.size);
	AnimManager_HostReadStream(stream, buf, info.size);
	CAnimBlock *animBlock = AnimManager_GetAnimationBlockByName(buf+4);
	if(animBlock){
		if(animBlock->numAnims == 0){
			animBlock->numAnims = *(int*)buf;
			animBlock->firstIndex = ms_numAnimations;
		}
	}else{
		animBlock = &ms_aAnimBlocks[ms_numAnimBlocks++];
		strncpy(animBlock->name, buf+4, MAX_ANIMBLOCK_NAME);
		animBlock->numAnims = *(int*)buf;
		animBlock->firstIndex = ms_numAnimations;
	}

	AnimManager_Debug("Loading ANIMS %s\n", animBlock->name);
	animBlock->isLoaded = true;

	int animIndex = animBlock->firstIndex;
	for(j = 0; j < animBlock->numAnims; j++){
		AnimManager_Assert(animIndex < ARRAY_SIZE(ms_aAnimations));
		CAnimBlendHierarchy *hier = &ms_aAnimations[animIndex++];

		// animation name
		AnimManager_HostReadStream(stream, &name, sizeof(IfpHeader));
		ROUNDSIZE(name.size);
		AnimManager_HostReadStream(stream, buf, name.size);
		// Use the complete C hierarchy at the original native operation boundary
		AnimHierarchy_SetName(hier,buf);

#ifdef ANIM_COMPRESSION
		bool compressHier = compress;
#else
		bool compressHier = false;
#endif
		if (uncompressedAnims) {
			for (int i = 0; uncompressedAnims[i][0]; i++) {
				if (!General_faststricmp(uncompressedAnims[i], hier->name)){
					AnimManager_Debug("Loading %s uncompressed\n", hier->name);
					compressHier = false;
				}
			}
		}

		hier->compressed = compressHier;
		hier->keepCompressed = false;

		// DG info has number of nodes/sequences
		AnimManager_HostReadStream(stream, (char*)&dgan, sizeof(IfpHeader));
		ROUNDSIZE(dgan.size);
		AnimManager_HostReadStream(stream, (char*)&info, sizeof(IfpHeader));
		ROUNDSIZE(info.size);
		AnimManager_HostReadStream(stream, buf, info.size);
		hier->numSequences = *(int*)buf;
//+ rouz edit (ChatGPT)
		// Allocate and construct animation sequences without invoking C++ array new.
		hier->sequences = (CAnimBlendSequence*)malloc(sizeof(CAnimBlendSequence)*hier->numSequences);
		for(k = 0; k < hier->numSequences; k++)
			// Use the complete C sequence at the original native operation boundary
			AnimSequence_Init(&hier->sequences[k]);
//- rouz edit (ChatGPT)

		CAnimBlendSequence *seq = hier->sequences;
		for(k = 0; k < hier->numSequences; k++, seq++){
			// Each node has a name and key frames
			AnimManager_HostReadStream(stream, &cpan, sizeof(IfpHeader));
			ROUNDSIZE(dgan.size);
			AnimManager_HostReadStream(stream, &anim, sizeof(IfpHeader));
			ROUNDSIZE(anim.size);
			AnimManager_HostReadStream(stream, buf, anim.size);
			int numFrames = *(int*)(buf+28);
			// Use the complete C sequence at the original native operation boundary
			AnimSequence_SetName(seq,buf);
			if(anim.size == 44)
				// Use the complete C sequence at the original native operation boundary
				AnimSequence_SetBoneTag(seq,*(int*)(buf+40));
			if(numFrames == 0)
				continue;

			bool hasScale = false;
			bool hasTranslation = false;
			AnimManager_HostReadStream(stream, &info, sizeof(info));
			if(strncmp(info.ident, "KRTS", 4) == 0){
				hasScale = true;
				// Use the complete C sequence at the original native operation boundary
				AnimSequence_SetNumFrames(seq,numFrames, true, compressHier);
			}else if(strncmp(info.ident, "KRT0", 4) == 0){
				hasTranslation = true;
				// Use the complete C sequence at the original native operation boundary
				AnimSequence_SetNumFrames(seq,numFrames, true, compressHier);
			}else if(strncmp(info.ident, "KR00", 4) == 0){
				// Use the complete C sequence at the original native operation boundary
				AnimSequence_SetNumFrames(seq,numFrames, false, compressHier);
			}
			if(strstr(seq->name, "L Toe"))
				AnimManager_Debug("anim %s has toe keyframes\n", hier->name); // BUG: seq->name

			for(l = 0; l < numFrames; l++){
				if(hasScale){
					AnimManager_HostReadStream(stream, buf, 0x2C);
					// Use the C quaternion operation at the original animation boundary
					CQuaternion rot=Quaternion_Make(fbuf[0], fbuf[1], fbuf[2], fbuf[3]);
					// Use the C quaternion operation at the original animation boundary
					Quaternion_Invert(&rot);
					KeyFrameVector3 trans = {fbuf[4], fbuf[5], fbuf[6]};

					if(compressHier){
						// Use the complete C sequence at the original native operation boundary
						KeyFrameTransCompressed *kf = (KeyFrameTransCompressed*)AnimSequence_GetKeyFrameCompressed(seq,l);
						// Use the C keyframe operation and its actual embedded base fields
						KeyFrameCompressed_SetRotation(&kf->base,&rot);
						// Use the C keyframe operation and its actual embedded base fields
						KeyFrameTransCompressed_SetTranslation(kf,AnimManager_TranslationRef(&trans));
						// scaling ignored
						// Use the C keyframe operation and its actual embedded base fields
						KeyFrameCompressed_SetTime(&kf->base,fbuf[10]);	// absolute time here
					}else{
						// Use the complete C sequence at the original native operation boundary
						KeyFrameTrans *kf = (KeyFrameTrans*)AnimSequence_GetKeyFrame(seq,l);
						// Use the C keyframe operation and its actual embedded base fields
						kf->base.rotation = rot;
						// Use the C keyframe operation and its actual embedded base fields
						kf->translation = trans;
						// scaling ignored
						// Use the C keyframe operation and its actual embedded base fields
						kf->base.deltaTime = fbuf[10];	// absolute time here
					}
				}else if(hasTranslation){
					AnimManager_HostReadStream(stream, buf, 0x20);
					// Use the C quaternion operation at the original animation boundary
					CQuaternion rot=Quaternion_Make(fbuf[0], fbuf[1], fbuf[2], fbuf[3]);
					// Use the C quaternion operation at the original animation boundary
					Quaternion_Invert(&rot);
					KeyFrameVector3 trans = {fbuf[4], fbuf[5], fbuf[6]};

					if(compressHier){
						// Use the complete C sequence at the original native operation boundary
						KeyFrameTransCompressed *kf = (KeyFrameTransCompressed*)AnimSequence_GetKeyFrameCompressed(seq,l);
						// Use the C keyframe operation and its actual embedded base fields
						KeyFrameCompressed_SetRotation(&kf->base,&rot);
						// Use the C keyframe operation and its actual embedded base fields
						KeyFrameTransCompressed_SetTranslation(kf,AnimManager_TranslationRef(&trans));
						// Use the C keyframe operation and its actual embedded base fields
						KeyFrameCompressed_SetTime(&kf->base,fbuf[7]);	// absolute time here
					}else{
						// Use the complete C sequence at the original native operation boundary
						KeyFrameTrans *kf = (KeyFrameTrans*)AnimSequence_GetKeyFrame(seq,l);
						// Use the C keyframe operation and its actual embedded base fields
						kf->base.rotation = rot;
						// Use the C keyframe operation and its actual embedded base fields
						kf->translation = trans;
						// Use the C keyframe operation and its actual embedded base fields
						kf->base.deltaTime = fbuf[7];	// absolute time here
					}
				}else{
					AnimManager_HostReadStream(stream, buf, 0x14);
					// Use the C quaternion operation at the original animation boundary
					CQuaternion rot=Quaternion_Make(fbuf[0], fbuf[1], fbuf[2], fbuf[3]);
					// Use the C quaternion operation at the original animation boundary
					Quaternion_Invert(&rot);

					if(compressHier){
						// Use the complete C sequence at the original native operation boundary
						KeyFrameCompressed *kf = (KeyFrameCompressed*)AnimSequence_GetKeyFrameCompressed(seq,l);
						// Use the C keyframe operation and its actual embedded base fields
						KeyFrameCompressed_SetRotation(kf,&rot);
						// Use the C keyframe operation and its actual embedded base fields
						KeyFrameCompressed_SetTime(kf,fbuf[4]);	// absolute time here
					}else{
						// Use the complete C sequence at the original native operation boundary
						KeyFrame *kf = (KeyFrame*)AnimSequence_GetKeyFrame(seq,l);
						kf->rotation = rot;
						kf->deltaTime = fbuf[4];	// absolute time here
					}
				}
			}
		}

		if(!compressHier){
			// Use the complete C hierarchy at the original native operation boundary
			AnimHierarchy_RemoveQuaternionFlips(hier);
			// Use the complete C hierarchy at the original native operation boundary
			AnimHierarchy_CalcTotalTime(hier);
		}
	}
	if(animIndex > ms_numAnimations)
		ms_numAnimations = animIndex;
}
void AnimManager_CreateAnimAssocGroups(void)
{
    // Preserve the original manager operation and its live field access order
	int i, j;

	for(i = 0; i < NUM_ANIM_ASSOC_GROUPS; i++){
		CAnimBlock *block = AnimManager_GetAnimationBlockByName(ms_aAnimAssocDefinitions[i].blockName);
		if(block == NULL || !block->isLoaded || ms_aAnimAssocGroups[i].assocList)
			continue;

		void *mi = AnimGroup_HostModelAt(ms_aAnimAssocDefinitions[i].modelIndex);
		void *clump = AnimGroup_HostCreateInstance(mi);
		AnimGroup_HostInitClump(clump);
		CAnimBlendAssocGroup *group = &ms_aAnimAssocGroups[i];
		const AnimAssocDefinition *def = &ms_aAnimAssocDefinitions[i];
		group->groupId = i;
		group->firstAnimId = def->animDescs[0].animId;
		// Use complete C group ownership at the original native call boundary
		AnimGroup_CreateForClump(group,def->blockName, clump, def->animNames, def->numAnims);
		for(j = 0; j < group->numAssociations; j++)
			// GetAnimation(i) in III (but it's in LoadAnimFiles), GetAnimation(group->animDesc[j].animId) in VC
			// Use complete C group ownership at the original native call boundary
			AnimGroup_GetById(group,def->animDescs[j].animId)->flags |= def->animDescs[j].flags;
		if(AnimGroup_HostIsSkinned(clump))
			AnimGroup_HostRemoveSkinAnimation(clump);
		AnimGroup_HostDestroyClump(clump);
	}
}
void AnimManager_RemoveLastAnimFile(void)
{
    // Preserve the original manager operation and its live field access order
	int i;
	ms_numAnimBlocks--;
	ms_numAnimations = ms_aAnimBlocks[ms_numAnimBlocks].firstIndex;
	for(i = 0; i < ms_aAnimBlocks[ms_numAnimBlocks].numAnims; i++)
		// Use the complete C hierarchy at the original native operation boundary
		AnimHierarchy_Shutdown(&ms_aAnimations[ms_aAnimBlocks[ms_numAnimBlocks].firstIndex + i]);
	ms_aAnimBlocks[ms_numAnimBlocks].isLoaded = false;
}
CAnimBlendAssocGroup * AnimManager_GetAnimAssocGroups(void)
{
    // Return the original manager storage directly
    return ms_aAnimAssocGroups;
}
void AnimHierarchy_HostRemoveCache(CAnimBlendHierarchy *self)
{
    // Forward the existing C owner boundary to manager storage and policy
    AnimManager_RemoveFromUncompressedCache(self);
}
void AnimAssociation_HostUncompressAnimation(CAnimBlendHierarchy *hierarchy)
{
    // Forward the existing C owner boundary to manager storage and policy
    AnimManager_UncompressAnimation(hierarchy);
}
CAnimBlock * AnimGroup_HostBlock(const char *name)
{
    // Forward the existing C owner boundary to manager storage and policy
    return AnimManager_GetAnimationBlockByName(name);
}
CAnimBlendHierarchy * AnimGroup_HostAnimationId(int32_t id)
{
    // Forward the existing C owner boundary to manager storage and policy
    return AnimManager_GetAnimationById(id);
}
CAnimBlendHierarchy * AnimGroup_HostAnimationName(const char *name,CAnimBlock *block)
{
    // Forward the existing C owner boundary to manager storage and policy
    return AnimManager_GetAnimationByName(name,block);
}
//- rouz edit (ChatGPT)
