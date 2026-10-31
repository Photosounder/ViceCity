//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
typedef struct tMessage {
	uint16_t *m_pText;
	uint16_t m_nFlag;
	uint32_t m_nTime;
	uint32_t m_nStartTime;
	int32_t m_nNumber[6];
	uint16_t *m_pString;
} tMessage;

typedef struct tBigMessage {
	tMessage m_Stack[4];
} tBigMessage;

typedef struct tPreviousBrief {
	uint16_t *m_pText;
	int32_t m_nNumber[6];
	uint16_t *m_pString;
} tPreviousBrief;

#define NUMBRIEFMESSAGES 8
#define NUMBIGMESSAGES 6
#define NUMPREVIOUSBRIEFS 5


#ifdef __cplusplus
extern "C" {
#endif
extern tMessage Messages_BriefMessages[NUMBRIEFMESSAGES];
extern tBigMessage Messages_BIGMessages[NUMBIGMESSAGES];
extern tPreviousBrief Messages_PreviousBriefs[NUMPREVIOUSBRIEFS];
extern char Messages_PreviousMissionTitle[16];
void Messages_Init(void);
uint16_t Messages_GetWideStringLength(uint16_t *src);
void Messages_WideStringCopy(uint16_t *dst, uint16_t *src, uint16_t size);
bool Messages_WideStringCompare(uint16_t *str1, uint16_t *str2, uint16_t size);
void Messages_Process(void);
void Messages_Display(void);
void Messages_AddMessage(uint16_t *key, uint32_t time, uint16_t pos);
void Messages_AddMessageJumpQ(uint16_t *key, uint32_t time, uint16_t pos);
void Messages_AddMessageSoon(uint16_t *key, uint32_t time, uint16_t pos);
void Messages_ClearMessages(void);
void Messages_ClearSmallMessagesOnly(void);
void Messages_AddBigMessage(uint16_t *key, uint32_t time, uint16_t pos);
void Messages_AddBigMessageQ(uint16_t *key, uint32_t time, uint16_t pos);
void Messages_AddToPreviousBriefArray(uint16_t *text, int32_t n1, int32_t n2, int32_t n3, int32_t n4, int32_t n5, int32_t n6, uint16_t *string);
void Messages_InsertNumberInString(uint16_t *src, int32_t n1, int32_t n2, int32_t n3, int32_t n4, int32_t n5, int32_t n6, uint16_t *dst);
void Messages_InsertStringInString(uint16_t *str1, uint16_t *str2);
void Messages_InsertPlayerControlKeysInString(uint16_t *src);
void Messages_AddMessageWithNumber(uint16_t *key, uint32_t time, uint16_t pos, int32_t n1, int32_t n2, int32_t n3, int32_t n4, int32_t n5, int32_t n6);
void Messages_AddMessageJumpQWithNumber(uint16_t *key, uint32_t time, uint16_t pos, int32_t n1, int32_t n2, int32_t n3, int32_t n4, int32_t n5, int32_t n6);
void Messages_AddMessageSoonWithNumber(uint16_t *key, uint32_t time, uint16_t pos, int32_t n1, int32_t n2, int32_t n3, int32_t n4, int32_t n5, int32_t n6);
void Messages_AddBigMessageWithNumber(uint16_t *key, uint32_t time, uint16_t pos, int32_t n1, int32_t n2, int32_t n3, int32_t n4, int32_t n5, int32_t n6);
void Messages_AddBigMessageWithNumberQ(uint16_t *key, uint32_t time, uint16_t pos, int32_t n1, int32_t n2, int32_t n3, int32_t n4, int32_t n5, int32_t n6);
void Messages_AddMessageWithString(uint16_t *text, uint32_t time, uint16_t flag, uint16_t *str);
void Messages_AddMessageJumpQWithString(uint16_t *text, uint32_t time, uint16_t flag, uint16_t *str);
void Messages_ClearThisPrint(uint16_t *str);
void Messages_ClearThisBigPrint(uint16_t *str);
void Messages_ClearAllMessagesDisplayedByGame(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
