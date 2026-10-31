//+ rouz edit (ChatGPT)
#include "Date.h"

void
Date_Initialize(CDate *date)
{
	// Clear all six fields in the original construction order
	date->m_nYear = 0;
	date->m_nSecond = 0;
	date->m_nMinute = 0;
	date->m_nHour = 0;
	date->m_nDay = 0;
	date->m_nMonth = 0;
}

bool
Date_IsAfter(const CDate *date, const CDate *right)
{
	// Compare signed date fields from year through second for later dates
	if (date->m_nYear > right->m_nYear)
		return true;
	if (date->m_nYear != right->m_nYear)
		return false;

	if (date->m_nMonth > right->m_nMonth)
		return true;
	if (date->m_nMonth != right->m_nMonth)
		return false;

	if (date->m_nDay > right->m_nDay)
		return true;
	if (date->m_nDay != right->m_nDay)
		return false;

	if (date->m_nHour > right->m_nHour)
		return true;
	if (date->m_nHour != right->m_nHour)
		return false;

	if (date->m_nMinute > right->m_nMinute)
		return true;
	if (date->m_nMinute != right->m_nMinute)
		return false;
	return date->m_nSecond > right->m_nSecond;
}

bool
Date_IsBefore(const CDate *date, const CDate *right)
{
	// Compare signed date fields from year through second for earlier dates
	if (date->m_nYear < right->m_nYear)
		return true;
	if (date->m_nYear != right->m_nYear)
		return false;

	if (date->m_nMonth < right->m_nMonth)
		return true;
	if (date->m_nMonth != right->m_nMonth)
		return false;

	if (date->m_nDay < right->m_nDay)
		return true;
	if (date->m_nDay != right->m_nDay)
		return false;

	if (date->m_nHour < right->m_nHour)
		return true;
	if (date->m_nHour != right->m_nHour)
		return false;

	if (date->m_nMinute < right->m_nMinute)
		return true;
	if (date->m_nMinute != right->m_nMinute)
		return false;
	return date->m_nSecond < right->m_nSecond;
}

bool
Date_IsEqual(const CDate *date, const CDate *right)
{
	// Check all six fields in the original equality order
	if (date->m_nYear != right->m_nYear || date->m_nMonth != right->m_nMonth || date->m_nDay != right->m_nDay || date->m_nHour != right->m_nHour || date->m_nMinute != right->m_nMinute)
		return false;
	return date->m_nSecond == right->m_nSecond;
}

void
Date_PopulateDateFields(CDate *date, const int8_t *second, const int8_t *minute, const int8_t *hour, const int8_t *day, const int8_t *month, int16_t year)
{
	// Read each original input address before assigning its date field
	date->m_nSecond = *second;
	date->m_nMinute = *minute;
	date->m_nHour = *hour;
	date->m_nDay = *day;
	date->m_nMonth = *month;
	date->m_nYear = year;
}
//- rouz edit (ChatGPT)
