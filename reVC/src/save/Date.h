//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
typedef struct CDate
{
	int m_nSecond;
	int m_nMinute;
	int m_nHour;
	int m_nDay;
	int m_nMonth;
	int m_nYear;
} CDate;
#ifdef __cplusplus
extern "C" {
#endif
void Date_Initialize(CDate *date);
bool Date_IsAfter(const CDate *date, const CDate *right);
bool Date_IsBefore(const CDate *date, const CDate *right);
bool Date_IsEqual(const CDate *date, const CDate *right);
void Date_PopulateDateFields(CDate *date, const int8_t *second, const int8_t *minute, const int8_t *hour, const int8_t *day, const int8_t *month, int16_t year);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
