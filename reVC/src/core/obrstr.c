//+ rouz edit (ChatGPT)
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "Debug.h"
#include "obrstr.h"

char obrstr[128];
char obrstr2[128];

void ObrInt(int32_t n1)
{
	IntToStr(n1, obrstr);
	Debug_DebugAddText(obrstr);
}

void ObrInt2(int32_t n1, int32_t n2)
{
	IntToStr(n1, obrstr);
	strcat(obrstr, "  ");
	IntToStr(n2, obrstr2);
	strcat(obrstr, obrstr2);
	Debug_DebugAddText(obrstr);
}

void ObrInt3(int32_t n1, int32_t n2, int32_t n3)
{
	IntToStr(n1, obrstr);
	strcat(obrstr, "  ");
	IntToStr(n2, obrstr2);
	strcat(obrstr, obrstr2);
	strcat(obrstr, "  ");
	IntToStr(n3, obrstr2);
	strcat(obrstr, obrstr2);
	Debug_DebugAddText(obrstr);
}

void ObrInt4(int32_t n1, int32_t n2, int32_t n3, int32_t n4)
{
	IntToStr(n1, obrstr);
	strcat(obrstr, "  ");
	IntToStr(n2, obrstr2);
	strcat(obrstr, obrstr2);
	strcat(obrstr, "  ");
	IntToStr(n3, obrstr2);
	strcat(obrstr, obrstr2);
	strcat(obrstr, "  ");
	IntToStr(n4, obrstr2);
	strcat(obrstr, obrstr2);
	Debug_DebugAddText(obrstr);
}

void ObrInt5(int32_t n1, int32_t n2, int32_t n3, int32_t n4, int32_t n5)
{
	IntToStr(n1, obrstr);
	strcat(obrstr, "  ");
	IntToStr(n2, obrstr2);
	strcat(obrstr, obrstr2);
	strcat(obrstr, "  ");
	IntToStr(n3, obrstr2);
	strcat(obrstr, obrstr2);
	strcat(obrstr, "  ");
	IntToStr(n4, obrstr2);
	strcat(obrstr, obrstr2);
	strcat(obrstr, "  ");
	IntToStr(n5, obrstr2);
	strcat(obrstr, obrstr2);
	Debug_DebugAddText(obrstr);
}

void ObrInt6(int32_t n1, int32_t n2, int32_t n3, int32_t n4, int32_t n5, int32_t n6)
{
	IntToStr(n1, obrstr);
	strcat(obrstr, "  ");
	IntToStr(n2, obrstr2);
	strcat(obrstr, obrstr2);
	strcat(obrstr, "  ");
	IntToStr(n3, obrstr2);
	strcat(obrstr, obrstr2);
	strcat(obrstr, "  ");
	IntToStr(n4, obrstr2);
	strcat(obrstr, obrstr2);
	strcat(obrstr, "  ");
	IntToStr(n5, obrstr2);
	strcat(obrstr, obrstr2);
	strcat(obrstr, "  ");
	IntToStr(n6, obrstr2);
	strcat(obrstr, obrstr2);
	Debug_DebugAddText(obrstr);
}

void IntToStr(int32_t inNum, char *outStr)
{
	bool isNeg = inNum < 0;

	if (isNeg) {
		inNum = -inNum;
		*outStr = '-';
	}

	int16_t digits = 1;

	if (inNum > 9) {
		int32_t _inNum = inNum;
		do {
			digits++;
			_inNum /= 10;
		} while (_inNum > 9);
	}

	int32_t strSize = digits;
	if (isNeg)
		strSize++;

	char *pStr = &outStr[strSize];
	int32_t i = 0;
	do {
		*(pStr-- - 1) = (inNum % 10) + '0';
		inNum /= 10;
	} while (++i < strSize);
	outStr[strSize] = '\0';
}
//- rouz edit (ChatGPT)
