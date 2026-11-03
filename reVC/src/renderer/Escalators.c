//+ rouz edit (ChatGPT)
#include "ReplayState.h"
#include "Escalators_backend.h"
int32_t Escalators_Num;
void
Escalators_AddOne(EscalatorVector pos0,EscalatorVector pos1,EscalatorVector pos2,EscalatorVector pos3,bool b_isMovingDown) {
    // Preserve all original registration, replay and shutdown write ordering
	Escalator_AddThisOne(Escalators_Num++,pos0,pos1,pos2,pos3,b_isMovingDown);
}

void
Escalators_Init(void) {
    // Preserve all original registration, replay and shutdown write ordering
	Escalators_Shutdown();
	Escalators_Num = 0;

	Escalators_AddOne((EscalatorVector){-9.82999f, -938.04498f, 9.4219f}, (EscalatorVector){-8.573f, -938.04498f, 9.4219f},
		(EscalatorVector){-0.747f, -938.045f, 15.065f}, (EscalatorVector){0.88f, -938.045f, 15.065f}, true);

	Escalators_AddOne((EscalatorVector){-9.83f, -939.966f, 9.422f}, (EscalatorVector){-8.573f, -939.966f, 9.422f},
		(EscalatorVector){-0.747f, -939.966f, 15.065f}, (EscalatorVector){0.880f, -939.966f, 15.065f}, false);

	Escalators_AddOne((EscalatorVector){408.116f, 1058.36f, 18.261f}, (EscalatorVector){408.094f, 1057.04f, 18.261f},
		(EscalatorVector){408.116f, 1048.0f, 24.765f}, (EscalatorVector){408.094f, 1046.57f, 24.799f}, true);

	Escalators_AddOne((EscalatorVector){406.195f, 1058.36f, 18.261f}, (EscalatorVector){406.173f, 1057.04f, 18.261f},
		(EscalatorVector){406.195f, 1048.0f, 24.729f}, (EscalatorVector){406.173f, 1046.57f, 24.79f}, false);

	Escalators_AddOne((EscalatorVector){421.729f, 1058.3789f, 18.075f}, (EscalatorVector){421.707f, 1057.052f, 18.099f},
		(EscalatorVector){421.729f, 1048.016f, 24.604f}, (EscalatorVector){421.707f, 1046.589f, 24.637f}, true);

	Escalators_AddOne((EscalatorVector){419.808f, 1058.378f, 18.099f}, (EscalatorVector){419.786f, 1057.052f, 18.099f},
		(EscalatorVector){419.808f, 1048.016f, 24.568f}, (EscalatorVector){419.786f, 1046.589f, 24.637f}, false);

	Escalators_AddOne((EscalatorVector){412.69901f, 1102.729f, 17.569f}, (EscalatorVector){412.72198f, 1104.057f, 17.57f},
		(EscalatorVector){412.69901f, 1113.092f, 24.073f}, (EscalatorVector){412.72198f, 1114.3201f, 24.108f}, true);

	Escalators_AddOne((EscalatorVector){414.62f, 1102.729f, 17.569f}, (EscalatorVector){414.64301f, 1104.057f, 17.57f},
		(EscalatorVector){414.62f, 1113.092f, 24.037001f}, (EscalatorVector){414.64301f, 1114.3201f, 24.099001f}, false);

	Escalators_AddOne((EscalatorVector){414.64301f, 1145.589f, 17.57f}, (EscalatorVector){414.62f, 1144.261f, 17.569f},
		(EscalatorVector){414.64301f, 1135.226f, 24.073999f}, (EscalatorVector){414.62f, 1133.798f, 24.107f}, true);

	Escalators_AddOne((EscalatorVector){412.72198f, 1145.589f, 17.57f}, (EscalatorVector){412.69901f, 1144.261f, 17.569f},
		(EscalatorVector){412.72198f, 1135.226f, 24.038f}, (EscalatorVector){412.69901f, 1133.798f, 24.098f}, false);

	Escalators_AddOne((EscalatorVector){406.05099f, 1193.4771f, 18.016001f}, (EscalatorVector){406.07401f, 1194.8051f, 18.017f},
		(EscalatorVector){406.05099f, 1203.84f, 24.52f}, (EscalatorVector){406.07401f, 1205.2679f, 24.555f}, true);

	Escalators_AddOne((EscalatorVector){407.97198f, 1193.4771f, 18.016001f}, (EscalatorVector){407.995f, 1194.8051f, 18.017f},
		(EscalatorVector){407.97198f, 1203.84f, 24.483999f}, (EscalatorVector){407.995f, 1205.2679f, 24.546f}, false);

	Escalators_AddOne((EscalatorVector){419.659f, 1193.479f, 17.979f}, (EscalatorVector){419.68201f, 1194.807f, 17.98f},
		(EscalatorVector){419.659f, 1203.842f, 24.483f}, (EscalatorVector){419.68201f, 1205.27f, 24.518f}, true);

	Escalators_AddOne((EscalatorVector){421.57999f, 1193.479f, 17.979f}, (EscalatorVector){421.603f, 1194.807f, 17.98f},
		(EscalatorVector){421.57999f, 1203.842f, 24.447001f}, (EscalatorVector){421.603f, 1205.27f, 24.509001f}, false);

	Escalators_AddOne((EscalatorVector){406.23199f, 1022.857f, 17.917f}, (EscalatorVector){406.23199f, 1024.1851f, 17.917f},
		(EscalatorVector){406.23199f, 1033.22f, 24.521f}, (EscalatorVector){406.23199f, 1034.647f, 24.555f}, true);

	Escalators_AddOne((EscalatorVector){408.15302f, 1022.857f, 17.917f}, (EscalatorVector){408.15302f, 1024.1851f, 17.916f},
		(EscalatorVector){408.15302f, 1033.22f, 24.486f}, (EscalatorVector){408.15302f, 1034.647f, 24.52f}, false);

	Escalators_AddOne((EscalatorVector){-1506.39f, -813.13f, 13.834f}, (EscalatorVector){-1506.177f, -814.51703f, 13.834f},
		(EscalatorVector){-1504.566f, -823.20898f, 19.836f}, (EscalatorVector){-1504.329f, -824.48499f, 19.837f}, false);

	Escalators_AddOne((EscalatorVector){-1481.951f, -859.05402f, 13.834f}, (EscalatorVector){-1482.7791f, -858.22498f, 13.834f},
		(EscalatorVector){-1489.03f, -851.974f, 19.836f}, (EscalatorVector){-1489.948f, -851.05701f, 19.837f}, true);

	Escalators_AddOne((EscalatorVector){-1461.743f, -871.35901f, 13.834f}, (EscalatorVector){-1460.62f, -871.69202f, 13.834f},
		(EscalatorVector){-1452.144f, -874.20203f, 19.836f}, (EscalatorVector){-1450.9f, -874.57098f, 19.837f}, false);

	Escalators_AddOne((EscalatorVector){-1409.889f, -871.41498f, 13.834f}, (EscalatorVector){-1411.0129f, -871.74701f, 13.834f},
		(EscalatorVector){-1419.489f, -874.258f, 19.836f}, (EscalatorVector){-1420.733f, -874.62701f, 19.837f}, true);

	Escalators_AddOne((EscalatorVector){-1389.577f, -858.89301f, 13.834f}, (EscalatorVector){-1388.7271f, -858.08698f, 13.834f},
		(EscalatorVector){-1382.314f, -852.00201f, 19.836f}, (EscalatorVector){-1381.373f, -851.10797f, 19.837f}, false);

	Escalators_AddOne((EscalatorVector){-1364.981f, -813.13f, 13.834f}, (EscalatorVector){-1365.204f, -814.28003f, 13.834f},
		(EscalatorVector){-1366.891f, -822.95801f, 19.83f}, (EscalatorVector){-1367.139f, -824.23199f, 19.837f}, true);

	for (int i = 0; i < NUM_ESCALATORS; i++) {
		// Visit every original slot regardless of the live registration count
		Escalator_SwitchOff(i);
	}
}

void
Escalators_Update(void) {
    // Preserve all original registration, replay and shutdown write ordering
	if (Replay_IsPlayingBack())
		return;

	for (int i = 0; i < NUM_ESCALATORS; i++) {
		// Visit every original slot regardless of the live registration count
		Escalator_Update(i);
	}
}

void
Escalators_Shutdown(void) {
    // Preserve all original registration, replay and shutdown write ordering
	for (int i = 0; i < NUM_ESCALATORS; i++) {
		// Visit every original slot regardless of the live registration count
		Escalator_SwitchOff(i);
	}
	Escalators_Num = 0;
}
//- rouz edit (ChatGPT)
