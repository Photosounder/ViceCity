//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"

#include "Sprite2d.h"
#include "TxdStore.h"
#include "Font.h"
#ifdef BUTTON_ICONS
#include "FileMgr.h"
#endif
#include "Timer.h"

CFontDetails Font_Details;
bool16 Font_NewLine;
Sprite2d Font_Sprite[MAX_FONTS];
static void Sprite2d_Exit_CFont_Sprite(void)
{
    // Destroy this original object group in its former reverse array order
    Sprite2d_DeinitArray(Font_Sprite,(size_t)(MAX_FONTS));
}
static int Sprite2d_Start_CFont_Sprite(void)
{
    // Initialize at the original definition boundary and retain its exit registration position
    Sprite2d_InitArray(Font_Sprite,(size_t)(MAX_FONTS));atexit(Sprite2d_Exit_CFont_Sprite);return 0;
}
static const int Sprite2d_Started_CFont_Sprite=Sprite2d_Start_CFont_Sprite();
CFontRenderState Font_RenderState;

#ifdef MORE_LANGUAGES
uint8 Font_LanguageSet = FONT_LANGSET_EFIGS;
int32 Font_Slot = -1;
#define JAP_TERMINATION (0x8000 | '~')

int16 Font_Size[LANGSET_MAX][MAX_FONTS][210] = {
	{
#else
int16 Font_Size[MAX_FONTS][210] = {
#endif
	{
		//FONT2 EFIGS
		//SPC,!,         $,  %,  &,  ',  [,  ],      +,  ,   -,  .,    
		12, 9,  22, 17, 19, 19, 25, 4,  33, 33, 25, 35, 11, 10,  6, 33,
		//0, 1,  2,  3,  4,  5,  6,  7,  8,  9,  :,                 ??,
		18, 10, 17, 17, 17, 17, 17, 15, 12, 16,  5, 30, 30, 30, 30, 30,
		//   A,  B,  C,  D,  E,  F,  G,  H,  I,  J,  K,  L,  M,  N,  O,
		12, 16, 19, 16, 19, 18, 18, 17, 22, 11, 17, 18, 18, 30, 22, 19,
		//P, Q,  R,  S,  T,  U,  V,  W,  X,  Y,  Z, ??, ??, ??,  ¡,  \,
		#ifdef FIX_BUGS
		22, 19, 19, 20, 18, 19, 19, 29, 19, 18, 19, 19, 33, 33, 10, 19,
		#else
		22, 19, 19, 20, 18, 19, 19, 29, 19, 18, 19, 19, 33, 33, 19, 19,
		#endif
		//??,a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,
		12, 14, 11, 11, 16, 11, 12, 14, 14, 10, 13, 12, 10, 19, 18, 12,
		//p, q,  r,  s,  t,  u,  v,  w,  x,  y,  z, ??, ??, ??, ??, ??,
		16, 13, 13, 11, 12, 15, 12, 15, 13, 12, 12, 37, 33, 37, 35, 37,
		//À, Á,  Â,  Ä,  Æ,  Ç,  È,  É,  Ê,  Ë,  Ì,  Í,  Î,  Ï,  Ò,  Ó,
		16, 16, 16, 16, 33, 17, 18, 18, 18, 18, 11, 11, 11, 11, 19, 19,
		//Ô, Ö,  Ù,  Ú,  Û,  Ü,  ß,  à,  á,  â,  ä,  æ,  ç,  è,  é,  ê,
		19, 19, 19, 19, 19, 19, 15, 14, 14, 14, 14, 20, 14, 11, 11, 11,
		//ë, ì,  í,  î,  ï,  ò,  ó,  ô,  ö,  ù,  ú,  û,  ü,  Ñ,  ñ,  ¿,
		#ifdef FIX_BUGS
		11, 10, 10, 10, 10, 12, 12, 12, 12, 15, 15, 15, 15, 22, 18, 21,
		#else
		11, 10, 10, 10, 10, 12, 12, 12, 12, 15, 15, 15, 15, 24, 18, 21,
		#endif
		//i,BLANKS
		10, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
		19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
		19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
		19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
		//space, unprop
		19, 16
	},
	{
		//FONT1 EFIGS
		//Characters with a '2' refer to the Pricedown font.
		//Characters that are referred as '*I' are characters that contain icons for PS2/XBOX, but contain regular characters on PC
		//in order to display them properly in the Keyboard controls menu.
		//!2,!, *I,(R),  $,  %,  &,  ',  [,  ], *I,  +,  ,   -,  ., *I,
		15,  7, 31, 25, 20, 23, 21,  7, 11, 10, 26, 14,  6, 12,  6, 26,
		//0, 1,  2,  3,  4,  5,  6,  7,  8,  9,  :, *I, *I, *I, *I,  ?,
		20,  7, 20, 20, 21, 20, 20, 19, 21, 20,  8, 30, 24, 30, 24, 19,
		//TM,A,  B,  C,  D,  E,  F,  G,  H,  I,  J,  K,  L,  M,  N,  O,
		20, 22, 22, 21, 22, 18, 18, 22, 22,  9, 14, 21, 18, 27, 21, 24,
		//P, Q,  R,  S,  T,  U,  V,  W,  X,  Y,  Z, *I,  \, *I,  ¡,  °,
		#ifdef FIX_BUGS
		22, 22, 23, 20, 19, 23, 22, 31, 23, 23, 21, 25, 13, 30,  7, 19,
		#else
		22, 22, 23, 20, 19, 23, 22, 31, 23, 23, 21, 25, 13, 30, 10, 19,
		#endif
		//(C),a, b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,
		10, 17, 17, 16, 17, 17, 11, 17, 17,  7,  7, 18,  7, 25, 17, 17,
		//p, q,  r,  s,  t,  u,  v,  w,  x,  y,  z, *I, *I, $2, (2, )2,
		17, 17, 11, 17, 11, 17, 18, 25, 19, 18, 17, 28, 26, 20, 15, 15,
		//À, Á,  Â,  Ä,  Æ,  Ç,  È,  É,  Ê,  Ë,  Ì,  Í,  Î,  Ï,  Ò,  Ó,
		20, 20, 20, 20, 29, 22, 19, 19, 19, 19,  9,  9,  9,  9, 23, 23,
		//Ô, Ö,  Ù,  Ú,  Û,  Ü,  ß,  à,  á,  â,  ä,  æ,  ç,  è,  é,  ê,
		23, 23, 24, 24, 24, 24, 20, 19, 17, 17, 17, 30, 16, 17, 17, 17,
		//ë, ì,  í,  î,  ï,  ò,  ó,  ô,  ö,  ù,  ú,  û,  ü,  Ñ,  ñ,  ¿,
		#ifdef FIX_BUGS
		17, 11, 11, 15, 12, 17, 17, 17, 17, 17, 17, 17, 17, 21, 17, 19,
		#else
		17, 11, 11, 15, 12, 17, 17, 17, 17, 17, 17, 17, 17, 19, 20, 20,
		#endif
		//02,12,22, 32, 42, 52, 62, 72, 82, 92, :2, A2, B2, C2, D2, E2,
		20, 18, 19, 19, 21, 19, 19, 19, 19, 19, 16, 19, 19, 19, 20, 19,
		//F2,G2,H2, I2, J2, K2, L2, M2, N2, O2, P2, Q2, R2, S2, T2, U2,
		16, 19, 19,  9, 19, 20, 14, 29, 19, 19, 19, 19, 19, 19, 21, 19,
		//V2,W2,X2, Y2, Z2, À2, Á2, Â2, Ä2, Æ2, Ç2, È2, É2, Ê2, Ë2, Ì2,
		20, 32, 20, 19, 19, 19, 19, 19, 19, 29, 19, 19, 19, 19, 19,  9,
		//Í2,Î2,Ï2, Ò2, Ó2, Ô2, Ö2, Ù2, Ú2, Û2, Ü2, ß2, Ñ2, ¿2, '2, .2,
		#ifdef FIX_BUGS
		 9,  9,  9, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 10,  9,
		#else
		 9,  9,  9, 19, 19, 19, 19, 19, 19, 19, 19, 19, 21, 21, 10,  9,
		#endif
		//space, unprop
		10, 20
	}

#ifdef MORE_LANGUAGES
	},
	{
        {
			5,  9,  9,  0,  17, 17, 23, 3,  21, 18, 0,  8,  3,  8,  3,  0,
			16, 9,  16, 16, 15, 19, 15, 14, 17, 17, 4,  4,  0,  0,  0,  17,
			19, 17, 19, 15, 21, 18, 19, 16, 21, 13, 15, 21, 20, 28, 21, 18,
			22, 17, 21, 20, 18, 18, 20, 26, 22, 18, 18, 0,  8,  0,  9,  8,
			0,  14, 11, 12, 16, 11, 13, 13, 15, 10, 14, 15, 11, 21, 17, 10,
			20, 15, 12, 12, 16, 17, 13, 16, 13, 21, 11, 0,  0,  0,  0,  0, 
			20, 19, 19, 22, 27, 15, 18, 18, 20, 26, 21, 23, 17, 22, 21, 17,
			26, 25, 26, 17, 20, 26, 17, 16, 11, 12, 13, 21, 11, 17, 17, 12,
			21, 17, 17, 15, 24, 16, 10, 20, 23, 16, 7,  9,  16, 23, 12, 11,
			0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  19,
			19, 16
		},
        {
			11, 5,  10, 15, 19, 22, 20, 5,  9,  8,  11, 12, 5,  12, 6,  12,
			19, 5,  18, 19, 20, 18, 19, 18, 20, 19, 5,  6,  26, 12, 30, 19,
			23, 21, 20, 20, 20, 16, 16, 21, 19, 5,  13, 19, 16, 24, 20, 21,
			20, 21, 20, 19, 17, 20, 21, 30, 22, 21, 20, 25, 13, 30,  5,  9,
			10, 15, 15, 14, 15, 16, 10, 15, 15, 5,  5,  15, 5,  23, 15, 16,
			15, 15, 9,  16, 10, 15, 17, 24, 18, 15, 15, 27, 5,  19, 2,  2, 
			20, 20, 16, 23, 30, 19, 20, 20, 21, 24, 19, 19, 20, 23, 22, 19,
			27, 29, 25, 20, 20, 28, 24, 16, 16, 14, 19, 25, 16, 16, 16, 17,
			19, 16, 16, 17, 25, 19, 15, 23, 26, 21, 16, 14, 22, 20, 16, 19,
			15, 14, 15, 16, 17, 15, 15, 15, 15, 15, 7,  15, 15, 15, 15, 15,
			13, 15, 15, 7,  15, 16, 13, 23, 15, 15, 15, 15, 15, 15, 17, 15,
			16, 24, 17, 17, 17, 15, 15, 13, 20, 23, 15, 17, 17, 16, 24, 15,
			15, 15, 23, 18, 15, 23, 26, 23, 16, 15, 23, 15, 15, 19, 2,  2,
			10, 20
		},
	},	
	{
		{
			//FONT2 EFIGS
			//SPC,!,         $,  %,  &,  ',  [,  ],      +,  ,   -,  .,    
			12, 9,  22, 17, 19, 19, 25, 4,  33, 33, 25, 35, 11, 10,  6, 33,
			//0, 1,  2,  3,  4,  5,  6,  7,  8,  9,  :,                 ??,
			18, 10, 17, 17, 17, 17, 17, 15, 12, 16,  5, 30, 30, 30, 30, 30,
			//   A,  B,  C,  D,  E,  F,  G,  H,  I,  J,  K,  L,  M,  N,  O,
			12, 16, 19, 16, 19, 18, 18, 17, 22, 11, 17, 18, 18, 30, 22, 19,
			//P, Q,  R,  S,  T,  U,  V,  W,  X,  Y,  Z, ??, ??, ??,  ¡,  \,
			22, 19, 19, 20, 18, 19, 19, 29, 19, 18, 19, 19, 33, 33, 10, 19,
			//??,a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,
			12, 14, 11, 11, 16, 11, 12, 14, 14, 10, 13, 12, 10, 19, 18, 12,
			//p, q,  r,  s,  t,  u,  v,  w,  x,  y,  z, ??, ??, ??, ??, ??,
			16, 13, 13, 11, 12, 15, 12, 15, 13, 12, 12, 37, 33, 37, 35, 37,
			//À, Á,  Â,  Ä,  Æ,  Ç,  È,  É,  Ê,  Ë,  Ì,  Í,  Î,  Ï,  Ò,  Ó,
			16, 16, 16, 16, 33, 17, 18, 18, 18, 18, 11, 11, 11, 11, 19, 19,
			//Ô, Ö,  Ù,  Ú,  Û,  Ü,  ß,  à,  á,  â,  ä,  æ,  ç,  è,  é,  ê,
			19, 19, 19, 19, 19, 19, 15, 14, 14, 14, 14, 20, 14, 11, 11, 11,
			//ë, ì,  í,  î,  ï,  ò,  ó,  ô,  ö,  ù,  ú,  û,  ü,  Ñ,  ñ,  ¿,
			11, 10, 10, 10, 10, 12, 12, 12, 12, 15, 15, 15, 15, 22, 18, 21,
			//i,BLANKS
			10, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
			19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
			19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
			19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
			//space, unprop
			19, 16
		},
		{
			//FONT1 EFIGS
			//Characters with a '2' refer to the Pricedown font.
			//Characters that are referred as '*I' are characters that contain icons for PS2/XBOX, but contain regular characters on PC
			//in order to display them properly in the Keyboard controls menu.
			//!2,!, *I,(R),  $,  %,  &,  ',  [,  ], *I,  +,  ,   -,  ., *I,
			15,  7, 31, 25, 20, 23, 21,  7, 11, 10, 26, 14,  6, 12,  6, 26,
			//0, 1,  2,  3,  4,  5,  6,  7,  8,  9,  :, *I, *I, *I, *I,  ?,
			20,  7, 20, 20, 21, 20, 20, 19, 21, 20,  8, 30, 24, 30, 24, 19,
			//TM,A,  B,  C,  D,  E,  F,  G,  H,  I,  J,  K,  L,  M,  N,  O,
			20, 22, 22, 21, 22, 18, 18, 22, 22,  9, 14, 21, 18, 27, 21, 24,
			//P, Q,  R,  S,  T,  U,  V,  W,  X,  Y,  Z, *I,  \, *I,  ¡,  °,
			22, 22, 23, 20, 19, 23, 22, 31, 23, 23, 21, 25, 13, 30,  7, 19,
			//(C),a, b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,
			10, 17, 17, 16, 17, 17, 11, 17, 17,  7,  7, 18,  7, 25, 17, 17,
			//p, q,  r,  s,  t,  u,  v,  w,  x,  y,  z, *I, *I, $2, (2, )2,
			17, 17, 11, 17, 11, 17, 18, 25, 19, 18, 17, 28, 26, 20, 15, 15,
			//À, Á,  Â,  Ä,  Æ,  Ç,  È,  É,  Ê,  Ë,  Ì,  Í,  Î,  Ï,  Ò,  Ó,
			20, 20, 20, 20, 29, 22, 19, 19, 19, 19,  9,  9,  9,  9, 23, 23,
			//Ô, Ö,  Ù,  Ú,  Û,  Ü,  ß,  à,  á,  â,  ä,  æ,  ç,  è,  é,  ê,
			23, 23, 24, 24, 24, 24, 20, 19, 17, 17, 17, 30, 16, 17, 17, 17,
			//ë, ì,  í,  î,  ï,  ò,  ó,  ô,  ö,  ù,  ú,  û,  ü,  Ñ,  ñ,  ¿,
			17, 11, 11, 15, 12, 17, 17, 17, 17, 17, 17, 17, 17, 21, 17, 19,
			//02,12,22, 32, 42, 52, 62, 72, 82, 92, :2, A2, B2, C2, D2, E2,
			20, 18, 19, 19, 21, 19, 19, 19, 19, 19, 16, 19, 19, 19, 20, 19,
			//F2,G2,H2, I2, J2, K2, L2, M2, N2, O2, P2, Q2, R2, S2, T2, U2,
			16, 19, 19,  9, 19, 20, 14, 29, 19, 19, 19, 19, 19, 19, 21, 19,
			//V2,W2,X2, Y2, Z2, À2, Á2, Â2, Ä2, Æ2, Ç2, È2, É2, Ê2, Ë2, Ì2,
			20, 32, 20, 19, 19, 19, 19, 19, 19, 29, 19, 19, 19, 19, 19,  9,
			//Í2,Î2,Ï2, Ò2, Ó2, Ô2, Ö2, Ù2, Ú2, Û2, Ü2, ß2, Ñ2, ¿2, '2, .2,
			 9,  9,  9, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 10,  9,
			//space, unprop
			10, 20
		}
	}
#endif
};

#ifdef MORE_LANGUAGES
int16 Size_jp[] = {
	15, 14, 16, 20, 19, 26, 22, 11, 18, 18, 27, 26, 13, //; 0
	19, 20, 27, 19, 15, 19, 19, 21, 19, 20, 18, 19, 15, //; 13
	13, 28, 15, 32, 15, 35, 15, 19, 19, 19, 19, 17, 16, //; 26
	19, 20, 15, 19, 20, 14, 17, 19, 19, 19, 19, 19, 19, //; 39
	19, 19, 20, 25, 20, 19, 19, 33, 31, 39, 37, 39, 37, //; 52
	21, 21, 21, 19, 17, 15, 23, 21, 15, 19, 20, 16, 19, //; 65
	19, 19, 20, 20, 17, 22, 19, 22, 22, 19, 22, 22, 23, //; 78
	35, 35, 35, 35, 37, 19, 19, 19, 19, 29, 19, 19, 19, //; 91
	19, 19, 9, 9, 9, 9, 19, 19, 19, 19, 19, 19, 19, 19, //; 104
	19, 19, 19, 19, 19, 30, 19, 19, 19, 19, 19, 10, 10, //; 118
	10, 10, 19, 19, 19, 19, 19, 19, 19, 19, 19, 23, 35, //; 131
	12, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, //; 144
	19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, //; 157
	19, 19, 19, 11, 19, 19, 19, 19, 19, 19, 19, 19, 19, //; 170
	19, 19, 19, 19, 19, 19, 19, 19, 19, 21
};
#endif

wchar foreign_table[128] = {
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0, 176,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0, 177,   0,   0,   0,   0,   0,  95,   0,   0,   0,   0, 175,
	128, 129, 130,   0, 131,   0, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141,
	  0, 173, 142, 143, 144,   0, 145,   0,   0, 146, 147, 148, 149,   0,   0, 150,
	151, 152, 153,   0, 154,   0, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164,
	  0, 174, 165, 166, 167,   0, 168,   0,   0, 169, 170, 171, 172,   0,   0,   0,
};

union tFontRenderStatePointer
{
	CFontRenderState *pRenderState;
	wchar *pStr;

	void Align()
	{
		if ((uintptr)pStr % 4)
			pStr++;
	}
};

tFontRenderStatePointer FontRenderStatePointer;
uint8 FontRenderStateBuf[1024];

#ifdef BUTTON_ICONS
static Sprite2d fontButtonSprites[MAX_BUTTON_ICONS];
static void Sprite2d_Exit_fontButtonSprites(void)
{
    // Destroy this original object group in its former reverse array order
    Sprite2d_DeinitArray(fontButtonSprites,(size_t)(MAX_BUTTON_ICONS));
}
static int Sprite2d_Start_fontButtonSprites(void)
{
    // Initialize at the original definition boundary and retain its exit registration position
    Sprite2d_InitArray(fontButtonSprites,(size_t)(MAX_BUTTON_ICONS));atexit(Sprite2d_Exit_fontButtonSprites);return 0;
}
static const int Sprite2d_Started_fontButtonSprites=Sprite2d_Start_fontButtonSprites();
int Font_PS2Symbol = BUTTON_NONE;
int Font_ButtonsSlot = -1;
#endif // BUTTON_ICONS

void
Font_Initialise(void)
{
	int slot;

	// Use the complete C texture dictionary owner with the original live arguments
	slot = TxdStore_AddTxdSlot("fonts");
#ifdef MORE_LANGUAGES
	Font_Slot = slot;
	switch (Font_LanguageSet)
	{
	case FONT_LANGSET_EFIGS:
	default:
		// Use the complete C texture dictionary owner with the original live arguments
		TxdStore_LoadFile(slot, "MODELS/FONTS.TXD");
		break;
	case FONT_LANGSET_POLISH:
		// Use the complete C texture dictionary owner with the original live arguments
		TxdStore_LoadFile(slot, "MODELS/FONTS_P.TXD");
		break;
	case FONT_LANGSET_RUSSIAN:
		// Use the complete C texture dictionary owner with the original live arguments
		TxdStore_LoadFile(slot, "MODELS/FONTS_R.TXD");
		break;
	case FONT_LANGSET_JAPANESE:
		// Use the complete C texture dictionary owner with the original live arguments
		TxdStore_LoadFile(slot, "MODELS/FONTS_J.TXD");
		break;
	}
#else
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_LoadFile(slot, "MODELS/FONTS.TXD");
#endif
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_AddRef(slot);
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_PushCurrentTxd();
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_SetCurrentTxd(slot);
	([](Sprite2d *spriteSelf,const char *name,const char *mask){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_SetTextureMasked(spriteSelf,name,mask);
})(&(Font_Sprite[0]),"font2", "font2m");
#ifdef MORE_LANGUAGES
	if (Font_IsJapanese()) {
		([](Sprite2d *spriteSelf,const char *name,const char *mask){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_SetTextureMasked(spriteSelf,name,mask);
})(&(Font_Sprite[1]),"FONTJAP", "FONTJAP_mask");
		([](Sprite2d *spriteSelf,const char *name,const char *mask){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_SetTextureMasked(spriteSelf,name,mask);
})(&(Font_Sprite[3]),"FONTJAP", "FONTJAP_mask");
	}
#endif // MORE_LANGUAGES
	([](Sprite2d *spriteSelf,const char *name,const char *mask){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_SetTextureMasked(spriteSelf,name,mask);
})(&(Font_Sprite[1]),"font1", "font1m");
	Font_SetScale(1.0f, 1.0f);
	Font_SetSlantRefPoint(SCREEN_WIDTH, 0.0f);
	Font_SetSlant(0.0f);
	Font_SetColor(CRGBA(255, 255, 255, 0));
	Font_SetJustifyOff();
	Font_SetCentreOff();
	Font_SetWrapx(SCREEN_WIDTH);
	Font_SetCentreSize(SCREEN_WIDTH);
	Font_SetBackgroundOff();
	Font_SetBackgroundColor(CRGBA(128, 128, 128, 128));
	Font_SetBackGroundOnlyTextOff();
	Font_SetPropOn();
	Font_SetFontStyle(FONT_BANK);
	Font_SetRightJustifyWrap(0.0f);
	Font_SetAlphaFade(255.0f);
	Font_SetDropShadowPosition(0);
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_PopCurrentTxd();

#if !defined(GAMEPAD_MENU) && defined(BUTTON_ICONS)
	// loaded in CMenuManager with GAMEPAD_MENU defined
	Font_LoadButtons("MODELS/X360BTNS.TXD");
#endif
}

#ifdef BUTTON_ICONS
void
Font_LoadButtons(const char *txdPath)
{
	if (int file = FileMgr_OpenFileDefault(txdPath)) {
		FileMgr_CloseFile(file);
		if (Font_ButtonsSlot == -1)
			// Use the complete C texture dictionary owner with the original live arguments
			Font_ButtonsSlot = TxdStore_AddTxdSlot("buttons");
		else {
			for (int i = 0; i < MAX_BUTTON_ICONS; i++)
				([](Sprite2d *spriteSelf){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_Delete(spriteSelf);
})(&(fontButtonSprites[i]));
			// Use the complete C texture dictionary owner with the original live arguments
			TxdStore_RemoveTxd(Font_ButtonsSlot);
		}
		// Use the complete C texture dictionary owner with the original live arguments
		TxdStore_LoadFile(Font_ButtonsSlot, txdPath);
		// Use the complete C texture dictionary owner with the original live arguments
		TxdStore_AddRef(Font_ButtonsSlot);
		// Use the complete C texture dictionary owner with the original live arguments
		TxdStore_PushCurrentTxd();
		// Use the complete C texture dictionary owner with the original live arguments
		TxdStore_SetCurrentTxd(Font_ButtonsSlot);
		([](Sprite2d *spriteSelf,const char *name){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_SetTexture(spriteSelf,name);
})(&(fontButtonSprites[BUTTON_UP]),"thumblyu");
		([](Sprite2d *spriteSelf,const char *name){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_SetTexture(spriteSelf,name);
})(&(fontButtonSprites[BUTTON_DOWN]),"thumblyd");
		([](Sprite2d *spriteSelf,const char *name){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_SetTexture(spriteSelf,name);
})(&(fontButtonSprites[BUTTON_LEFT]),"thumblxl");
		([](Sprite2d *spriteSelf,const char *name){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_SetTexture(spriteSelf,name);
})(&(fontButtonSprites[BUTTON_RIGHT]),"thumblxr");
		([](Sprite2d *spriteSelf,const char *name){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_SetTexture(spriteSelf,name);
})(&(fontButtonSprites[BUTTON_CROSS]),"cross");
		([](Sprite2d *spriteSelf,const char *name){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_SetTexture(spriteSelf,name);
})(&(fontButtonSprites[BUTTON_CIRCLE]),"circle");
		([](Sprite2d *spriteSelf,const char *name){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_SetTexture(spriteSelf,name);
})(&(fontButtonSprites[BUTTON_SQUARE]),"square");
		([](Sprite2d *spriteSelf,const char *name){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_SetTexture(spriteSelf,name);
})(&(fontButtonSprites[BUTTON_TRIANGLE]),"triangle");
		([](Sprite2d *spriteSelf,const char *name){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_SetTexture(spriteSelf,name);
})(&(fontButtonSprites[BUTTON_L1]),"l1");
		([](Sprite2d *spriteSelf,const char *name){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_SetTexture(spriteSelf,name);
})(&(fontButtonSprites[BUTTON_L2]),"l2");
		([](Sprite2d *spriteSelf,const char *name){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_SetTexture(spriteSelf,name);
})(&(fontButtonSprites[BUTTON_L3]),"l3");
		([](Sprite2d *spriteSelf,const char *name){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_SetTexture(spriteSelf,name);
})(&(fontButtonSprites[BUTTON_R1]),"r1");
		([](Sprite2d *spriteSelf,const char *name){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_SetTexture(spriteSelf,name);
})(&(fontButtonSprites[BUTTON_R2]),"r2");
		([](Sprite2d *spriteSelf,const char *name){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_SetTexture(spriteSelf,name);
})(&(fontButtonSprites[BUTTON_R3]),"r3");
		([](Sprite2d *spriteSelf,const char *name){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_SetTexture(spriteSelf,name);
})(&(fontButtonSprites[BUTTON_RSTICK_UP]),"thumbryu");
		([](Sprite2d *spriteSelf,const char *name){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_SetTexture(spriteSelf,name);
})(&(fontButtonSprites[BUTTON_RSTICK_DOWN]),"thumbryd");
		([](Sprite2d *spriteSelf,const char *name){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_SetTexture(spriteSelf,name);
})(&(fontButtonSprites[BUTTON_RSTICK_LEFT]),"thumbrxl");
		([](Sprite2d *spriteSelf,const char *name){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_SetTexture(spriteSelf,name);
})(&(fontButtonSprites[BUTTON_RSTICK_RIGHT]),"thumbrxr");
		// Use the complete C texture dictionary owner with the original live arguments
		TxdStore_PopCurrentTxd();
	}
	else {
		if (Font_ButtonsSlot != -1) {
			for (int i = 0; i < MAX_BUTTON_ICONS; i++)
				([](Sprite2d *spriteSelf){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_Delete(spriteSelf);
})(&(fontButtonSprites[i]));
			// Use the complete C texture dictionary owner with the original live arguments
			TxdStore_RemoveTxdSlot(Font_ButtonsSlot);
			Font_ButtonsSlot = -1;
		}
	}
}
#endif // BUTTON_ICONS

#ifdef MORE_LANGUAGES
void
Font_ReloadFonts(uint8 set)
{
	if (Font_Slot != -1 && Font_LanguageSet != set) {
		([](Sprite2d *spriteSelf){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_Delete(spriteSelf);
})(&(Font_Sprite[0]));
		([](Sprite2d *spriteSelf){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_Delete(spriteSelf);
})(&(Font_Sprite[1]));
		if (Font_IsJapanese())
			([](Sprite2d *spriteSelf){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_Delete(spriteSelf);
})(&(Font_Sprite[2]));
		// Use the complete C texture dictionary owner with the original live arguments
		TxdStore_PushCurrentTxd();
		// Use the complete C texture dictionary owner with the original live arguments
		TxdStore_RemoveTxd(Font_Slot);
		switch (set)
		{
		case FONT_LANGSET_EFIGS:
		default:
			// Use the complete C texture dictionary owner with the original live arguments
			TxdStore_LoadFile(Font_Slot, "MODELS/FONTS.TXD");
			break;
		case FONT_LANGSET_POLISH:
			// Use the complete C texture dictionary owner with the original live arguments
			TxdStore_LoadFile(Font_Slot, "MODELS/FONTS_P.TXD");
			break;
		case FONT_LANGSET_RUSSIAN:
			// Use the complete C texture dictionary owner with the original live arguments
			TxdStore_LoadFile(Font_Slot, "MODELS/FONTS_R.TXD");
			break;
		case FONT_LANGSET_JAPANESE:
			// Use the complete C texture dictionary owner with the original live arguments
			TxdStore_LoadFile(Font_Slot, "MODELS/FONTS_J.TXD");
			break;
		}
		// Use the complete C texture dictionary owner with the original live arguments
		TxdStore_SetCurrentTxd(Font_Slot);
		([](Sprite2d *spriteSelf,const char *name,const char *mask){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_SetTextureMasked(spriteSelf,name,mask);
})(&(Font_Sprite[0]),"font2", "font2_mask");
		if (set == FONT_LANGSET_JAPANESE) {
			([](Sprite2d *spriteSelf,const char *name,const char *mask){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_SetTextureMasked(spriteSelf,name,mask);
})(&(Font_Sprite[2]),"FONTJAP", "FONTJAP_mask");
		}
		([](Sprite2d *spriteSelf,const char *name,const char *mask){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_SetTextureMasked(spriteSelf,name,mask);
})(&(Font_Sprite[1]),"font1", "font1_mask");
		// Use the complete C texture dictionary owner with the original live arguments
		TxdStore_PopCurrentTxd();
	}
	Font_LanguageSet = set;
}
#endif

void
Font_Shutdown(void)
{
#ifdef BUTTON_ICONS
	if (Font_ButtonsSlot != -1) {
		for (int i = 0; i < MAX_BUTTON_ICONS; i++)
			([](Sprite2d *spriteSelf){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_Delete(spriteSelf);
})(&(fontButtonSprites[i]));
		// Use the complete C texture dictionary owner with the original live arguments
		TxdStore_RemoveTxdSlot(Font_ButtonsSlot);
		Font_ButtonsSlot = -1;
	}
#endif
	([](Sprite2d *spriteSelf){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_Delete(spriteSelf);
})(&(Font_Sprite[0]));
	([](Sprite2d *spriteSelf){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_Delete(spriteSelf);
})(&(Font_Sprite[1]));
#ifdef MORE_LANGUAGES
	if (Font_IsJapanese())
		([](Sprite2d *spriteSelf){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_Delete(spriteSelf);
})(&(Font_Sprite[3]));
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_RemoveTxdSlot(Font_Slot);
	Font_Slot = -1;
#else
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_RemoveTxdSlot(TxdStore_FindTxdSlot("fonts"));
#endif
}

void
Font_InitPerFrame(void)
{
	Font_RenderState.style = -1;
	Font_Details.anonymous_25 = 0;
	FontRenderStatePointer.pRenderState = (CFontRenderState*)FontRenderStateBuf;
	Font_SetDropShadowPosition(0);
	Font_NewLine = false;
#ifdef BUTTON_ICONS
	Font_PS2Symbol = BUTTON_NONE;
#endif
}

#ifdef BUTTON_ICONS
void
Font_DrawButton(float x, float y)
{
	if (x <= 0.0f || x > SCREEN_WIDTH || y <= 0.0f || y > SCREEN_HEIGHT)
		return;

	if (Font_PS2Symbol != BUTTON_NONE) {
		CRect rect;
		// Initialize rectangle bounds at the former local constructor boundary
		Rect_Init(&rect);
		rect.left = x;
		rect.top = Font_RenderState.scaleY + Font_RenderState.scaleY + y;
		rect.right = Font_RenderState.scaleY * 17.0f + x;
		rect.bottom = Font_RenderState.scaleY * 19.0f + y;

		int vertexAlphaState;
		void *raster;
		RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, &vertexAlphaState);
		RwRenderStateGet(rwRENDERSTATETEXTURERASTER, &raster);
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
		if (Font_RenderState.bIsShadow)
			([](Sprite2d *spriteSelf,const CRect &rect,const CRGBA &col){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawRect(spriteSelf,&rect,Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a));
})(&(fontButtonSprites[Font_PS2Symbol]),rect, Font_RenderState.color);
		else
			([](Sprite2d *spriteSelf,const CRect &rect,const CRGBA &col){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawRect(spriteSelf,&rect,Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a));
})(&(fontButtonSprites[Font_PS2Symbol]),rect, CRGBA(255, 255, 255, Font_RenderState.color.a));
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, raster);
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)vertexAlphaState);
	}
}
#endif

void
Font_PrintChar(float x, float y, wchar c)
{
	bool bDontPrint = false;
	if(x <= 0.0f || x > SCREEN_WIDTH ||
	   y <= 0.0f || y > SCREEN_HEIGHT)	// BUG: game uses SCREENW again
		return;

	bDontPrint = c == '\0';
	float w = Font_GetCharacterWidth(c) / 32.0f;
	if (Font_Details.bFontHalfTexture && c == 208)
		c = '\0';
	float xoff = c % 16;
	float yoff = c / 16;
#ifdef MORE_LANGUAGES
	if (Font_IsJapaneseFont()) {
		w = 21.0f;
		xoff = (float)(c % 48);
		yoff = c / 48;
	}
#endif

	if(Font_RenderState.style == FONT_BANK || Font_RenderState.style == FONT_STANDARD){
		if (bDontPrint) return;
		if (Font_RenderState.slant == 0.0f) {
#ifdef FIX_BUGS
			if (c < 192) {
#else
			if (c < 193) {
#endif
				([](const CRect &r,const CRGBA &c,float u0,float v0,float u1,float v1,float u3,float v3,float u2,float v2){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_AddToBuffer(&r,Sprite_ColourRef(&c.r,&c.g,&c.b,&c.a),u0,v0,u1,v1,u3,v3,u2,v2);
})(
					Rect_Make(x, y,
						x + 32.0f * Font_RenderState.scaleX * 1.0f,
						y + 40.0f * Font_RenderState.scaleY * 0.5f),
					Font_RenderState.color,
					xoff / 16.0f, yoff / 12.8f + 0.0021f,
					(xoff + 1.0f) / 16.0f - 0.001f, yoff / 12.8f + 0.0021f,
					xoff / 16.0f, (yoff + 1.0f) / 12.8f - 0.0021f,
					(xoff + 1.0f) / 16.0f - 0.001f, (yoff + 1.0f) / 12.8f - 0.0021f);
			} else {
				([](const CRect &r,const CRGBA &c,float u0,float v0,float u1,float v1,float u3,float v3,float u2,float v2){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_AddToBuffer(&r,Sprite_ColourRef(&c.r,&c.g,&c.b,&c.a),u0,v0,u1,v1,u3,v3,u2,v2);
})(
					Rect_Make(x, y,
						x + 32.0f * Font_RenderState.scaleX * 1.0f,
						y + 33.0f * Font_RenderState.scaleY * 0.5f),
					Font_RenderState.color,
					xoff / 16.0f, yoff / 12.8f + 0.0021f,
					(xoff + 1.0f) / 16.0f - 0.001f, yoff / 12.8f + 0.0021f,
					xoff / 16.0f, (yoff + 1.0f) / 12.8f - 0.017f,
					(xoff + 1.0f) / 16.0f - 0.001f, (yoff + 1.0f) / 12.8f - 0.017f);
			}
		} else
			([](const CRect &r,const CRGBA &c,float u0,float v0,float u1,float v1,float u3,float v3,float u2,float v2){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_AddToBuffer(&r,Sprite_ColourRef(&c.r,&c.g,&c.b,&c.a),u0,v0,u1,v1,u3,v3,u2,v2);
})(
				Rect_Make(x, y,
					x + 32.0f * Font_RenderState.scaleX * 1.0f,
					y + 40.0f * Font_RenderState.scaleY * 0.5f),
				Font_RenderState.color,
				xoff / 16.0f, yoff / 12.8f + 0.00055f,
				(xoff + 1.0f) / 16.0f - 0.001f, yoff / 12.8f + 0.0021f + 0.01f,
				xoff / 16.0f, (yoff + 1.0f) / 12.8f - 0.009f,
				(xoff + 1.0f) / 16.0f - 0.001f, (yoff + 1.0f) / 12.8f - 0.0021f + 0.01f);
#ifdef MORE_LANGUAGES
	/*}else if (IsJapaneseFont()) {
		if (Details.dropShadowPosition != 0) {
			CSprite2d::AddSpriteToBank(Details.bank + Details.style,	// BUG: game doesn't add bank
				CRect(x + SCREEN_SCALE_X(Details.dropShadowPosition),
					y + SCREEN_SCALE_Y(Details.dropShadowPosition),
					x + SCREEN_SCALE_X(Details.dropShadowPosition) + 32.0f * Details.scaleX * 1.0f,
					y + SCREEN_SCALE_Y(Details.dropShadowPosition) + 40.0f * Details.scaleY / 2.75f),
				Details.dropColor,
				xoff * w / 1024.0f, yoff / 25.6f,
				xoff * w / 1024.0f + (1.0f / 48.0f) - 0.001f, yoff / 25.6f,
				xoff * w / 1024.0f, (yoff + 1.0f) / 25.6f,
				xoff * w / 1024.0f + (1.0f / 48.0f) - 0.001f, (yoff + 1.0f) / 25.6f - 0.0001f);
		}
		CSprite2d::AddSpriteToBank(Details.bank + Details.style,	// BUG: game doesn't add bank
			CRect(x, y,
				x + 32.0f * Details.scaleX * 1.0f,
				y + 40.0f * Details.scaleY / 2.75f),
			Details.color,
			xoff * w / 1024.0f, yoff / 25.6f,
			xoff * w / 1024.0f + (1.0f / 48.0f) - 0.001f, yoff / 25.6f,
			xoff * w / 1024.0f, (yoff + 1.0f) / 25.6f - 0.002f,
			xoff * w / 1024.0f + (1.0f / 48.0f) - 0.001f, (yoff + 1.0f) / 25.6f - 0.0001f);*/
#endif
	} else {
		if (bDontPrint) return;
		([](const CRect &r,const CRGBA &c,float u0,float v0,float u1,float v1,float u3,float v3,float u2,float v2){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_AddToBuffer(&r,Sprite_ColourRef(&c.r,&c.g,&c.b,&c.a),u0,v0,u1,v1,u3,v3,u2,v2);
})(
			Rect_Make(x, y,
				x + 32.0f * Font_RenderState.scaleX * w,
				y + 32.0f * Font_RenderState.scaleY * 0.5f),
			Font_RenderState.color,
			xoff / 16.0f, yoff / 16.0f,
			(xoff + w) / 16.0f, yoff / 16.0f,
			xoff / 16.0f, (yoff + 1.0f) / 16.0f,
			(xoff + w) / 16.0f - 0.0001f, (yoff + 1.0f) / 16.0f - 0.0001f);
	}
}

#ifdef MORE_LANGUAGES
bool Font_IsJapanesePunctuation(wchar *str)
{
	return (*str == 0xE7 || *str == 0x124 || *str == 0x126 || *str == 0x128 || *str == 0x104 || *str == ',' || *str == '>' || *str == '!' || *str == 0x99 || *str == '?' || *str == ':');
}

bool Font_IsAnsiCharacter(wchar *s)
{
	if (*s >= 'A' && *s <= 'Z')
		return true;
	if (*s >= 'a' && *s <= 'z')
		return true;
	if (*s >= '0' && *s <= ':')
		return true;
	if (*s == '(' || *s == ')')
		return true;
	if (*s == 'D' || *s == '$')
		return true;
	return false;
}
#endif

void
Font_RenderFontBuffer()
{
	if (FontRenderStatePointer.pRenderState == (CFontRenderState*)FontRenderStateBuf) return;

	float textPosX;
	float textPosY;
	CRGBA color;
	bool bBold = false;
	bool bFlash = false;

	([](Sprite2d *spriteSelf){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_SetRenderState(spriteSelf);
})(&(Font_Sprite[Font_RenderState.style]));
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	Font_RenderState = *(CFontRenderState*)&FontRenderStateBuf[0];
	textPosX = Font_RenderState.fTextPosX;
	textPosY = Font_RenderState.fTextPosY;
	color = Font_RenderState.color;
	tFontRenderStatePointer pRenderStateBufPointer;
	pRenderStateBufPointer.pRenderState = (CFontRenderState*)&FontRenderStateBuf[0];
	for (++pRenderStateBufPointer.pRenderState; pRenderStateBufPointer.pStr < FontRenderStatePointer.pStr; pRenderStateBufPointer.pStr++) {
		if (*pRenderStateBufPointer.pStr == '\0') {
			tFontRenderStatePointer tmpPointer = pRenderStateBufPointer;
			tmpPointer.pStr++;
			tmpPointer.Align();
			if (tmpPointer.pStr >= FontRenderStatePointer.pStr)
				break;

			Font_RenderState = *(tmpPointer.pRenderState++);

			pRenderStateBufPointer = tmpPointer;

			textPosX = Font_RenderState.fTextPosX;
			textPosY = Font_RenderState.fTextPosY;
			color = Font_RenderState.color;
		}
		if (*pRenderStateBufPointer.pStr == '~') {
#ifdef BUTTON_ICONS
			Font_PS2Symbol = BUTTON_NONE;
#endif
			pRenderStateBufPointer.pStr = Font_ParseToken(pRenderStateBufPointer.pStr, color, bFlash, bBold);
#ifdef BUTTON_ICONS
			if(Font_PS2Symbol != BUTTON_NONE) {
				Font_DrawButton(textPosX, textPosY);
				textPosX += Font_RenderState.scaleY * 17.0f;
				Font_PS2Symbol = BUTTON_NONE;
			}
#endif
			if (bFlash) {
				if ((*Timer_GetTimeInMilliseconds()) - Font_Details.nFlashTimer > 300) {
					Font_Details.bFlashState = !Font_Details.bFlashState;
					Font_Details.nFlashTimer = (*Timer_GetTimeInMilliseconds());
				}
				Font_Details.color.alpha = Font_Details.bFlashState ? 0 : 255;
			}
			if (!Font_RenderState.bIsShadow)
				Font_RenderState.color = color;
		}
		wchar c = *pRenderStateBufPointer.pStr;
		c -= ' ';
		if (Font_RenderState.bFontHalfTexture) 
			c = Font_FindNewCharacter(c); 
		else if (c > 155) 
			c = '\0'; 

		if (Font_RenderState.slant != 0.0f)
			textPosY = (Font_RenderState.slantRefX - textPosX) * Font_RenderState.slant + Font_RenderState.slantRefY;
		Font_PrintChar(textPosX, textPosY, c);
		if (bBold) {
			Font_PrintChar(textPosX + 1.0f, textPosY, c);
			Font_PrintChar(textPosX + 2.0f, textPosY, c);
			textPosX += 2.0f;
		}
#ifdef FIX_BUGS
		// PS2 uses different chars for some symbols
		if (!Font_RenderState.bFontHalfTexture && c == 30) c = 61; // wanted star
#endif
		textPosX += Font_RenderState.scaleX * Font_GetCharacterWidth(c);
		if (c == '\0')
			textPosX += Font_RenderState.fExtraSpace;
	}
	// Preserve the original operation through the C sprite interface
	Sprite2d_RenderVertexBuffer();
	FontRenderStatePointer.pRenderState = (CFontRenderState*)FontRenderStateBuf;
}

#if 0 //def MORE_LANGUAGES
bool
Font_PrintString(float x, float y, wchar *start, wchar *&end, float spwidth, float japX)
{
	wchar *s, c, unused;

	if (Font_IsJapanese()) {
		float jx = 0.0f;
		for (s = start; s < end; s++) {
			if (*s == JAP_TERMINATION || *s == '~')
				s = Font_ParseToken(s, &unused, true);
			if (Font_NewLine) {
				Font_NewLine = false;
				break;
			}
			jx += Font_GetCharacterSize(*s - ' ');
		}
		s = start;
		if (Font_Details.centre)
			x = japX - jx / 2.0f;
		else if (Font_Details.rightJustify)
			x = japX - jx;
	}

	for (s = start; s < end; s++) {
		if (*s == '~' || (Font_IsJapanese() && *s == JAP_TERMINATION))
			s = Font_ParseToken(s, &unused);
		if (Font_NewLine && Font_IsJapanese()) {
			Font_NewLine = false;
			end = s;
			return true;
		}
		c = *s - ' ';
		if (Font_Details.slant != 0.0f && !Font_IsJapanese())
			y = (Font_Details.slantRefX - x) * Font_Details.slant + Font_Details.slantRefY;

		Font_PrintChar(x, y, c);
		x += Font_GetCharacterSize(c);
		if (c == 0 && (!Font_NewLine || !Font_IsJapanese()))	// space
			x += spwidth;
	}
	return false;
}
#else
void
Font_PrintString(float x, float y, uint32, wchar *start, wchar *end, float spwidth)
{
	wchar *s;

	if (Font_RenderState.style != Font_Details.style) {
		Font_RenderFontBuffer();
		Font_RenderState.style = Font_Details.style;
	}

	float dropShadowPosition = Font_Details.dropShadowPosition;
	if (dropShadowPosition != 0.0f && (Font_Details.style == FONT_BANK || Font_Details.style == FONT_STANDARD)) {
		CRGBA color = Font_Details.color;
		Font_Details.color = Font_Details.dropColor;
		Font_Details.dropShadowPosition = 0;
		Font_Details.bIsShadow = true;
		if (Font_Details.slant != 0.0f) {
			Font_Details.slantRefX += SCREEN_SCALE_X(dropShadowPosition);
			Font_Details.slantRefY += SCREEN_SCALE_Y(dropShadowPosition);
			Font_PrintString(SCREEN_SCALE_X(dropShadowPosition) + x, SCREEN_SCALE_Y(dropShadowPosition) + y, Font_Details.anonymous_25, start, end, spwidth);
			Font_Details.slantRefX -= SCREEN_SCALE_X(dropShadowPosition);
			Font_Details.slantRefY -= SCREEN_SCALE_Y(dropShadowPosition);
		} else {
			Font_PrintString(SCREEN_SCALE_X(dropShadowPosition) + x, SCREEN_SCALE_Y(dropShadowPosition) + y, Font_Details.anonymous_25, start, end, spwidth);
		}
		Font_Details.color = color;
		Font_Details.dropShadowPosition = dropShadowPosition;
		Font_Details.bIsShadow = false;
	}
	if (FontRenderStatePointer.pStr >= (wchar*)&FontRenderStateBuf[ARRAY_SIZE(FontRenderStateBuf)] - (end - start + 26)) // why 26?
		Font_RenderFontBuffer();
	CFontRenderState *pRenderState = FontRenderStatePointer.pRenderState;
	pRenderState->fTextPosX = x;
	pRenderState->fTextPosY = y;
	pRenderState->scaleX = Font_Details.scaleX;
	pRenderState->scaleY = Font_Details.scaleY;
	pRenderState->color = Font_Details.color;
	pRenderState->fExtraSpace = spwidth;
	pRenderState->slant = Font_Details.slant;
	pRenderState->slantRefX = Font_Details.slantRefX;
	pRenderState->slantRefY = Font_Details.slantRefY;
	pRenderState->bFontHalfTexture = Font_Details.bFontHalfTexture;
	pRenderState->proportional = Font_Details.proportional;
	pRenderState->style = Font_Details.style;
	pRenderState->bIsShadow = Font_Details.bIsShadow;
	FontRenderStatePointer.pRenderState++;

	for(s = start; s < end;){
		if (*s == '~') {
			for (wchar *i = Font_ParseToken(s); s != i; FontRenderStatePointer.pStr++) {
				*FontRenderStatePointer.pStr = *(s++);
			}
			if (Font_Details.bFlash) {
				if ((*Timer_GetTimeInMilliseconds()) - Font_Details.nFlashTimer > 300) {
					Font_Details.bFlashState = !Font_Details.bFlashState;
					Font_Details.nFlashTimer = (*Timer_GetTimeInMilliseconds());
				}
				Font_Details.color.a = Font_Details.bFlashState ? 0 : 255;
			}
		} else
			*(FontRenderStatePointer.pStr++) = *(s++);
	}
	*(FontRenderStatePointer.pStr++) = '\0';
	FontRenderStatePointer.Align();
}
#endif

void
Font_PrintStringFromBottom(float x, float y, wchar *str)
{
	y -= (32.0f * Font_Details.scaleY / 2.0f + 2.0f * Font_Details.scaleY) * Font_GetNumberLines(x, y, str);
	if (Font_Details.slant != 0.0f)
		y -= ((Font_Details.slantRefX - x) * Font_Details.slant + Font_Details.slantRefY);
	Font_PrintString(x, y, str);
}

void
Font_PrintString(float xstart, float ystart, wchar *s)
{
	CRect rect;
	// Initialize rectangle bounds at the former local constructor boundary
	Rect_Init(&rect);
	int numSpaces;
	float lineLength;
	float x, y;
	bool first;
	wchar *start, *t;

	Font_Details.bFlash = false;

	if(*s == '*')
		return;

	Font_Details.anonymous_25++;
	if(Font_Details.background){
		Font_RenderState.color = Font_Details.color;
		Font_GetNumberLines(xstart, ystart, s);	// BUG: result not used
		Font_GetTextRect(&rect, xstart, ystart, s);
		([](const CRect &r,const CRGBA &col){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawSolidRect(&r,Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a));
})(rect, Font_Details.backgroundColor);
	}

	lineLength = 0.0f;
	numSpaces = 0;
	first = true;
	if(Font_Details.centre || Font_Details.rightJustify)
		x = 0.0f;
	else
		x = xstart;
	y = ystart;
	start = s;

	// This is super ugly, I blame R*
	for(;;){
		for(;;){
			for(;;){
				if(*s == '\0')
					return;
				float xend = Font_Details.centre ? Font_Details.centreSize :
				           Font_Details.rightJustify ? xstart - Font_Details.rightJustifyWrap :
				           Font_Details.wrapX;
#ifdef MORE_LANGUAGES
				if (Font_IsJapaneseFont())
					xend -= SCREEN_SCALE_X(21.0f * 2.0f);
#endif
				if(x + Font_GetStringWidth(s) > xend && !first){
#ifdef MORE_LANGUAGES
					if (Font_IsJapanese() && Font_IsJapanesePunctuation(s))
						s--;
#endif
					// flush line
					float spaceWidth = !Font_Details.justify || Font_Details.centre ? 0.0f :
						(Font_Details.wrapX - lineLength) / numSpaces;
					float xleft = Font_Details.centre ? xstart - x/2 :
					              Font_Details.rightJustify ? xstart - x :
					              xstart;
#if 0//def MORE_LANGUAGES
					Font_PrintString(xleft, y, start, s, spaceWidth, xstart);
#else
					Font_PrintString(xleft, y, Font_Details.anonymous_25, start, s, spaceWidth);
#endif
					// reset things
					lineLength = 0.0f;
					numSpaces = 0;
					first = true;
					if(Font_Details.centre || Font_Details.rightJustify)
						x = 0.0f;
					else
						x = xstart;
#ifdef MORE_LANGUAGES
					if (Font_IsJapaneseFont())
						y += 32.0f * Font_Details.scaleY / 2.75f + 2.0f * Font_Details.scaleY;
					else
#endif
						y += 32.0f * Font_Details.scaleY * 0.5f + 2.0f * Font_Details.scaleY;
					start = s;
				}else
					break;
			}
			// advance by one word
			t = Font_GetNextSpace(s);
			if(t[0] == '\0' ||
			   t[0] == ' ' && t[1] == '\0')
				break;
			if(!first)
				numSpaces++;
			first = false;
			x += Font_GetStringWidth(s) + Font_GetCharacterSize(*t - ' ');
#ifdef MORE_LANGUAGES
			if (Font_IsJapaneseFont() && Font_IsAnsiCharacter(s))
				x += 21.0f;
#endif
			lineLength = x;
			s = t+1;
#if 0 //def MORE_LANGUAGES
			if (Font_IsJapaneseFont() && !*s) {
				x += Font_GetStringWidth(s);
				if (Font_IsAnsiCharacter(s))
					x += 21.0f;
				float xleft = Font_Details.centre ? xstart - x / 2 :
					Font_Details.rightJustify ? xstart - x :
					xstart;
				if (Font_PrintString(xleft, y, start, s, 0.0f, xstart))
				{
					start = s;
					if (!Font_Details.centre && !Font_Details.rightJustify)
						x = xstart;
					else
						x = 0.0f;

					y += 32.0f * Font_Details.scaleY / 2.75f + 2.0f * Font_Details.scaleY;
					numSpaces = 0;
					first = true;
					lineLength = 0.0f;
				}
			}
#endif
		}
		// print rest
		if(t[0] == ' ' && t[1] == '\0')
			t[0] = '\0';
		x += Font_GetStringWidth(s);
		s = t;
		float xleft = Font_Details.centre ? xstart - x/2 :
		              Font_Details.rightJustify ? xstart - x :
		              xstart;
#if 0 //def MORE_LANGUAGES
		if (Font_PrintString(xleft, y, start, s, 0.0f, xstart) && Font_IsJapaneseFont()) {
			start = s;
			if (!Font_Details.centre && !Font_Details.rightJustify)
				x = xstart;
			else
				x = 0.0f;
			y += 32.0f * Font_Details.scaleY / 2.75f + 2.0f * Font_Details.scaleY;
			numSpaces = 0;
			first = true;
			lineLength = 0.0f;
		}
#else
		Font_PrintString(xleft, y, Font_Details.anonymous_25, start, s, 0.0f);
#endif
	}
}

int
Font_GetNumberLines(float xstart, float ystart, wchar *s)
{
	int n;
	float x, y;
	wchar *t;
	n = 0;

#if 0//def MORE_LANGUAGES
	bool bSomeJapBool = false;

	if (Font_IsJapanese()) {
		t = s;
		wchar unused;
		while (*t) {
			if (*t == JAP_TERMINATION || *t == '~')
				t = Font_ParseToken(t, &unused, true);
			if (Font_NewLine) {
				n++;
				Font_NewLine = false;
				bSomeJapBool = true;
			}
			t++;
		}
	}

	if (bSomeJapBool) n--;
#endif

	if(Font_Details.centre || Font_Details.rightJustify)
		x = 0.0f;
	else
		x = xstart;
	y = ystart;

	while(*s){
#ifdef FIX_BUGS
		float f = Font_Details.centre ? Font_Details.centreSize :
		           Font_Details.rightJustify ? xstart - Font_Details.rightJustifyWrap :
		           Font_Details.wrapX;
#else
		float f = (Font_Details.centre ? Font_Details.centreSize : Font_Details.wrapX);
#endif

#ifdef MORE_LANGUAGES
		if (Font_IsJapaneseFont())
			f -= SCREEN_SCALE_X(21.0f * 2.0f);
#endif

		if(x + Font_GetStringWidth(s) > f){
#ifdef MORE_LANGUAGES
			if (Font_IsJapanese())
			{
				if (Font_IsJapanesePunctuation(s))
					s--;
			}
#endif
			// reached end of line
			if(Font_Details.centre || Font_Details.rightJustify)
				x = 0.0f;
			else
				x = xstart;
			n++;
			// Why even?
#ifdef MORE_LANGUAGES
			if (Font_IsJapanese())
				y += 32.0f * Font_Details.scaleY / 2.75f + 2.0f * Font_Details.scaleY;
			else
#endif
				y += 32.0f * Font_Details.scaleY * 0.5f + 2.0f * Font_Details.scaleY;
		}else{
			// still space in current line
			t = Font_GetNextSpace(s);
			if(*t == '\0'){
				// end of string
				x += Font_GetStringWidth(s);
#ifdef MORE_LANGUAGES
				if (Font_IsJapanese() && Font_IsAnsiCharacter(s))
					x += 21.0f;
#endif
				n++;
				s = t;
			}else{
				x += Font_GetStringWidth(s);
#ifdef MORE_LANGUAGES
				if (Font_IsJapanese() && Font_IsAnsiCharacter(s))
					x += 21.0f;
#endif
				s = t+1;
				x += Font_GetCharacterSize(*t - ' ');
#ifdef MORE_LANGUAGES
				if (Font_IsJapanese() && !*s)
					n++;
#endif
			}
		}
	}

	return n;
}

void
Font_GetTextRect(CRect *rect, float xstart, float ystart, wchar *s)
{
	int numLines;
	float x, y;
	int16 maxlength;
	wchar *t;

	maxlength = 0;
	numLines = 0;

#ifdef MORE_LANGUAGES
	if (Font_IsJapanese()) {
		numLines = Font_GetNumberLines(xstart, ystart, s);
	}else{
#endif
		if(Font_Details.centre || Font_Details.rightJustify)
			x = 0.0f;
		else
			x = xstart;
		y = ystart;

#ifdef FIX_BUGS
		float xEnd = Font_Details.centre ? Font_Details.centreSize :
		           Font_Details.rightJustify ? xstart - Font_Details.rightJustifyWrap :
		           Font_Details.wrapX;
#else
		float xEnd = (Font_Details.centre ? Font_Details.centreSize : Font_Details.wrapX);
#endif
		while(*s){
			if(x + Font_GetStringWidth(s) > xEnd){
				// reached end of line
				if(x > maxlength)
					maxlength = x;
				if(Font_Details.centre || Font_Details.rightJustify)
					x = 0.0f;
				else
					x = xstart;
				numLines++;
				y += 32.0f * Font_Details.scaleY * 0.5f + 2.0f * Font_Details.scaleY;
			}else{
				// still space in current line
				t = Font_GetNextSpace(s);
				if(*t == '\0'){
					// end of string
					x += Font_GetStringWidth(s);
					if(x > maxlength)
						maxlength = x;
					numLines++;
					s = t;
				}else{
					x += Font_GetStringWidth(s);
					x += Font_GetCharacterSize(*t - ' ');
					s = t+1;
				}
			}
		}
#ifdef MORE_LANGUAGES
	}
#endif

	if(Font_Details.centre){
		if(Font_Details.backgroundOnlyText){
			rect->left = xstart - maxlength/2 - 4.0f;
			rect->right = xstart + maxlength/2 + 4.0f;
#ifdef MORE_LANGUAGES
			if (Font_IsJapaneseFont()) {
				rect->bottom = (32.0f * Font_Details.scaleY / 2.75f + 2.0f * Font_Details.scaleY) * numLines + ystart + (4.0f / 2.75f);
				rect->top = ystart - (4.0f / 2.75f);
			} else {
#endif
				rect->bottom = (32.0f * Font_Details.scaleY * 0.5f + 2.0f * Font_Details.scaleY) * numLines + ystart + 2.0f;
				rect->top = ystart - 2.0f;
#ifdef MORE_LANGUAGES
			}
#endif
		}else{
			rect->left = xstart - Font_Details.centreSize*0.5f - 4.0f;
			rect->right = xstart + Font_Details.centreSize*0.5f + 4.0f;
#ifdef MORE_LANGUAGES
			if (Font_IsJapaneseFont()) {
				rect->bottom = (32.0f * Font_Details.scaleY / 2.75f + 2.0f * Font_Details.scaleY) * numLines + ystart + (4.0f / 2.75f);
				rect->top = ystart - (4.0f / 2.75f);
			} else {
#endif
				rect->bottom = (32.0f * Font_Details.scaleY * 0.5f + 2.0f * Font_Details.scaleY) * numLines + ystart + 2.0f;
				rect->top = ystart - 2.0f;
#ifdef MORE_LANGUAGES
			}
#endif
		}
	}else{
		rect->left = xstart - 4.0f;
		rect->right = Font_Details.wrapX;
		// WTF?
		rect->bottom = ystart - 4.0f + 4.0f;
#ifdef MORE_LANGUAGES
		if (Font_IsJapaneseFont())
			rect->top = (32.0f * Font_Details.scaleY / 2.75f + 2.0f * Font_Details.scaleY) * numLines + ystart + 2.0f + (4.0f / 2.75f);
		else
#endif
			rect->top = (32.0f * Font_Details.scaleY * 0.5f + 2.0f * Font_Details.scaleY) * numLines + ystart + 2.0f + 2.0f;
	}
}

float
Font_GetCharacterWidth(wchar c)
{
#ifdef MORE_LANGUAGES
	if (Font_IsJapanese()) {
		if (!Font_RenderState.proportional)
			return Font_Size[0][Font_Details.style][192];
		if (c <= 94 || Font_Details.style == FONT_HEADING || Font_RenderState.style == FONT_BANK) {
			switch (Font_RenderState.style)
			{
			case FONT_JAPANESE:
				return Size_jp[c];
			default:
				return Font_Size[0][Font_RenderState.style][c];
			}
		}

		switch (Font_RenderState.style)
		{
		case FONT_JAPANESE:
			return 29.4f;
		case FONT_BANK:
			return 10.0f;
		default:
			return Font_Size[0][Font_RenderState.style][c];
		}
	}

	else if (Font_RenderState.proportional)
		return Font_Size[Font_LanguageSet][Font_RenderState.style][c];
	else
		return Font_Size[Font_LanguageSet][Font_RenderState.style][209];
#else

	if (Font_RenderState.proportional)
		return Font_Size[Font_RenderState.style][c];
	else
		return Font_Size[Font_RenderState.style][209];
#endif // MORE_LANGUAGES
}

float
Font_GetCharacterSize(wchar c)
{
#ifdef MORE_LANGUAGES

	if (Font_IsJapanese())
	{
		if (!Font_Details.proportional)
			return Font_Size[0][Font_Details.style][209] * Font_Details.scaleX;
		if (c <= 94 || Font_Details.style == FONT_HEADING || Font_Details.style == FONT_BANK) {
			switch (Font_Details.style)
			{
			case FONT_JAPANESE:
				return Size_jp[c] * Font_Details.scaleX;
			default:
				return Font_Size[0][Font_Details.style][c] * Font_Details.scaleX;
			}
		}

		switch (Font_Details.style)
		{
		case FONT_JAPANESE:
			return 29.4f * Font_Details.scaleX;
		case FONT_BANK:
			return 10.0f * Font_Details.scaleX;
		default:
			return Font_Size[0][Font_Details.style][c] * Font_Details.scaleX;
		}
	}
	else
	{
		if (!Font_Details.bFontHalfTexture && c == 30) c = 61; // wanted star
		if (Font_Details.bFontHalfTexture)
			c = Font_FindNewCharacter(c);
		if (Font_Details.proportional)
			return Font_Size[Font_LanguageSet][Font_Details.style][c] * Font_Details.scaleX;
		else
			return Font_Size[Font_LanguageSet][Font_Details.style][209] * Font_Details.scaleX;
	}
#else

#ifdef FIX_BUGS
	// PS2 don't call FindNewCharacter in here at all, and also uses different chars for some symbols
	if (!Font_Details.bFontHalfTexture && c == 30) c = 61; // wanted star
#endif
	if (Font_Details.bFontHalfTexture)
		c = Font_FindNewCharacter(c);
	if (Font_Details.proportional)
		return Font_Size[Font_Details.style][c] * Font_Details.scaleX;
	else
		return Font_Size[Font_Details.style][209] * Font_Details.scaleX;
#endif // MORE_LANGUAGES
}

float
Font_GetStringWidth(wchar *s, bool spaces)
{
	float w;

	w = 0.0f;
#ifdef MORE_LANGUAGES
	if (Font_IsJapanese())
	{
		do
		{
			if ((*s != ' ' || spaces) && *s != '\0') {
				do {
					while (*s == '~' || *s == JAP_TERMINATION) {
						s++;
#ifdef BUTTON_ICONS
						switch (*s) {
						case 'U':
						case 'D':
						case '<':
						case '>':
						case 'X':
						case 'O':
						case 'Q':
						case 'T':
						case 'K':
						case 'M':
						case 'A':
						case 'J':
						case 'V':
						case 'C':
						case '(':
						case ')':
							w += 17.0f * Font_Details.scaleY;
							break;
						default:
							break;
						}
#endif
						while (!(*s == '~' || *s == JAP_TERMINATION)) s++;
						s++;
					}
					w += Font_GetCharacterSize(*s - ' ');
					++s;
				} while (*s == '~' || *s == JAP_TERMINATION);
			}
		} while (Font_IsAnsiCharacter(s));
	} else
#endif
	{
		for (wchar c = *s; (c != ' ' || spaces) && c != '\0'; c = *(++s)) {
			if (c == '~') {
				s++;
#ifdef BUTTON_ICONS
				switch (*s) {
				case 'U':
				case 'D':
				case '<':
				case '>':
				case 'X':
				case 'O':
				case 'Q':
				case 'T':
				case 'K':
				case 'M':
				case 'A':
				case 'J':
				case 'V':
				case 'C':
				case '(':
				case ')':
					w += 17.0f * Font_Details.scaleY;
					break;
				default:
					break;
				}
#endif
				while (*s != '~') {
					s++;
				}
			}
			else {
				w += Font_GetCharacterSize(c - ' ');
			}
		}
	}
	return w;
}


#ifdef MORE_LANGUAGES
float
Font_GetStringWidth_Jap(wchar* s)
{
	float w;

	w = 0.0f;
	for (; *s != '\0';) {
		do {
			while (*s == '~' || *s == JAP_TERMINATION) {
				s++;
				while (!(*s == '~' || *s == JAP_TERMINATION)) s++;
				s++;
			}
			w += Font_GetCharacterSize(*s - ' ');
			++s;
		} while (*s == '~' || *s == JAP_TERMINATION);
	}
	return w;
}
#endif

wchar*
Font_GetNextSpace(wchar *s)
{
#ifdef MORE_LANGUAGES
	if (Font_IsJapanese()) {
		do
		{
			if (*s != ' ' && *s != '\0') {
				do {
					while (*s == '~' || *s == JAP_TERMINATION) {
						s++;
						while (!(*s == '~' || *s == JAP_TERMINATION)) s++;
						s++;
					}
					++s;
				} while (*s == '~' || *s == JAP_TERMINATION);
			}
		} while (Font_IsAnsiCharacter(s));
	} else
#endif
	{
		// Delegate the plain case to the C implementation
		s = FontText_GetNextSpace(s);
	}
	return s;
}

wchar*
Font_ParseToken(wchar* str, CRGBA &color, bool &flash, bool &bold)
{
	Font_Details.anonymous_23 = false;
	wchar *s = str + 1;
	if (Font_Details.color.r || Font_Details.color.g || Font_Details.color.b)
	{
		switch (*s)
		{
		case 'B':
			bold = !bold;
			break;
		case 'b':
			color.r = 27;
			color.g = 89;
			color.b = 130;
			break;
		case 'f':
			flash = !flash;
			break;
		case 'g':
			color.r = 255;
			color.g = 150;
			color.b = 225;
			break;
		case 'h':
			color.r = 225;
			color.g = 225;
			color.b = 225;
			break;
		case 'l':
			color.r = 0;
			color.g = 0;
			color.b = 0;
			break;
		case 'o':
			color.r = 229;
			color.g = 125;
			color.b = 126;
			break;
		case 'p':
			color.r = 168;
			color.g = 110;
			color.b = 252;
			break;
		case 'q':
			color.r = 199;
			color.g = 144;
			color.b = 203;
			break;
		case 'r':
			color.r = 255;
			color.g = 150;
			color.b = 225;
			break;
		case 't':
			color.r = 86;
			color.g = 212;
			color.b = 146;
			break;
		case 'w':
			color.r = 175;
			color.g = 175;
			color.b = 175;
			break;
#ifdef FIX_BUGS
		case 'x':
			color.r = 0;
			color.g = 255;
			color.b = 255;
			break;
#else
		case 'x':
			color.r = 132;
			color.g = 146;
			color.b = 197;
			break;
#endif
		case 'y':
			color.r = 255;
			color.g = 227;
			color.b = 79;
			break;
#ifdef BUTTON_ICONS
		case 'U': Font_PS2Symbol = BUTTON_UP; break;
		case 'D': Font_PS2Symbol = BUTTON_DOWN; break;
		case '<': Font_PS2Symbol = BUTTON_LEFT; break;
		case '>': Font_PS2Symbol = BUTTON_RIGHT; break;
		case 'X': Font_PS2Symbol = BUTTON_CROSS; break;
		case 'O': Font_PS2Symbol = BUTTON_CIRCLE; break;
		case 'Q': Font_PS2Symbol = BUTTON_SQUARE; break;
		case 'T': Font_PS2Symbol = BUTTON_TRIANGLE; break;
		case 'K': Font_PS2Symbol = BUTTON_L1; break;
		case 'M': Font_PS2Symbol = BUTTON_L2; break;
		case 'A': Font_PS2Symbol = BUTTON_L3; break;
		case 'J': Font_PS2Symbol = BUTTON_R1; break;
		case 'V': Font_PS2Symbol = BUTTON_R2; break;
		case 'C': Font_PS2Symbol = BUTTON_R3; break;
		case 'H': Font_PS2Symbol = BUTTON_RSTICK_UP; break;
		case 'L': Font_PS2Symbol = BUTTON_RSTICK_DOWN; break;
		case '(': Font_PS2Symbol = BUTTON_RSTICK_LEFT; break;
		case ')': Font_PS2Symbol = BUTTON_RSTICK_RIGHT; break;
#endif
		default:
			break;
		}
	}
	while (*s != '~')
		++s;
	if (*(++s) == '~')
		s = Font_ParseToken(s, color, flash, bold);
	return s;
}

#if 0//def MORE_LANGUAGES
wchar*
Font_ParseToken(wchar *s, bool japShit)
{
	s++;
	if ((Font_Details.color.r || Font_Details.color.g || Font_Details.color.b) && !japShit) {
		wchar c = *s;
		if (Font_IsJapanese())
			c &= 0x7FFF;
		switch (c) {
		case 'N':
		case 'n':
			Font_NewLine = true;
			break;
		case 'b': Font_SetColor(CRGBA(128, 167, 243, 255)); break;
		case 'g': Font_SetColor(CRGBA(95, 160, 106, 255)); break;
		case 'h': Font_SetColor(CRGBA(225, 225, 225, 255)); break;
		case 'l': Font_SetColor(CRGBA(0, 0, 0, 255)); break;
		case 'p': Font_SetColor(CRGBA(168, 110, 252, 255)); break;
		case 'r': Font_SetColor(CRGBA(113, 43, 73, 255)); break;
		case 'w': Font_SetColor(CRGBA(175, 175, 175, 255)); break;
		case 'y': Font_SetColor(CRGBA(210, 196, 106, 255)); break;
#ifdef BUTTON_ICONS
		case 'U': Font_PS2Symbol = BUTTON_UP; break;
		case 'D': Font_PS2Symbol = BUTTON_DOWN; break;
		case '<': Font_PS2Symbol = BUTTON_LEFT; break;
		case '>': Font_PS2Symbol = BUTTON_RIGHT; break;
		case 'X': Font_PS2Symbol = BUTTON_CROSS; break;
		case 'O': Font_PS2Symbol = BUTTON_CIRCLE; break;
		case 'Q': Font_PS2Symbol = BUTTON_SQUARE; break;
		case 'T': Font_PS2Symbol = BUTTON_TRIANGLE; break;
		case 'K': Font_PS2Symbol = BUTTON_L1; break;
		case 'M': Font_PS2Symbol = BUTTON_L2; break;
		case 'A': Font_PS2Symbol = BUTTON_L3; break;
		case 'J': Font_PS2Symbol = BUTTON_R1; break;
		case 'V': Font_PS2Symbol = BUTTON_R2; break;
		case 'C': Font_PS2Symbol = BUTTON_R3; break;
		case 'H': Font_PS2Symbol = BUTTON_RSTICK_UP; break;
		case 'L': Font_PS2Symbol = BUTTON_RSTICK_DOWN; break;
		case '(': Font_PS2Symbol = BUTTON_RSTICK_LEFT; break;
		case ')': Font_PS2Symbol = BUTTON_RSTICK_RIGHT; break;
#endif
		}
	} else if (Font_IsJapanese()) {
		if ((*s & 0x7FFF) == 'N' || (*s & 0x7FFF) == 'n')
			Font_NewLine = true;
	}
	while ((!Font_IsJapanese() || (*s != JAP_TERMINATION)) && *s != '~') s++;
	return s + 1;
}
#else
wchar*
Font_ParseToken(wchar *s)
{
	Font_Details.anonymous_23 = false;
	s++;
	if(Font_Details.color.r || Font_Details.color.g || Font_Details.color.b)
		switch(*s){
		case 'B':
			Font_Details.bBold = !Font_Details.bBold;
			break;
		case 'N':
		case 'n':
			Font_NewLine = true;
			break;
		case 'b':
			Font_Details.color.r = 27;
			Font_Details.color.g = 89;
			Font_Details.color.b = 130;
			Font_Details.anonymous_23 = true;
			break;
		case 'f':
			Font_Details.bFlash = !Font_Details.bFlash;
			if (!Font_Details.bFlash)
				Font_Details.color.a = 255;
			break;
		case 'g':
			Font_Details.color.r = 255;
			Font_Details.color.g = 150;
			Font_Details.color.b = 225;
			Font_Details.anonymous_23 = true;
			break;
		case 'h':
			Font_Details.color.r = 225;
			Font_Details.color.g = 225;
			Font_Details.color.b = 225;
			Font_Details.anonymous_23 = true;
			break;
		case 'l':
			Font_Details.color.r = 0;
			Font_Details.color.g = 0;
			Font_Details.color.b = 0;
			Font_Details.anonymous_23 = true;
			break;
		case 'o':
			Font_Details.color.r = 229;
			Font_Details.color.g = 125;
			Font_Details.color.b = 126;
			Font_Details.anonymous_23 = true;
			break;
		case 'p':
			Font_Details.color.r = 168;
			Font_Details.color.g = 110;
			Font_Details.color.b = 252;
			Font_Details.anonymous_23 = true;
			break;
		case 'q':
			Font_Details.color.r = 199;
			Font_Details.color.g = 144;
			Font_Details.color.b = 203;
			Font_Details.anonymous_23 = true;
			break;
		case 'r':
			Font_Details.color.r = 255;
			Font_Details.color.g = 150;
			Font_Details.color.b = 225;
			Font_Details.anonymous_23 = true;
			break;
		case 't':
			Font_Details.color.r = 86;
			Font_Details.color.g = 212;
			Font_Details.color.b = 146;
			Font_Details.anonymous_23 = true;
			break;
		case 'w':
			Font_Details.color.r = 175;
			Font_Details.color.g = 175;
			Font_Details.color.b = 175;
			Font_Details.anonymous_23 = true;
			break;
		case 'x':
#ifdef FIX_BUGS
			Font_Details.color.r = 0;
			Font_Details.color.g = 255;
			Font_Details.color.b = 255;
#else
			Font_Details.color.r = 132;
			Font_Details.color.g = 146;
			Font_Details.color.b = 197;
#endif
			Font_Details.anonymous_23 = true;
			break;
		case 'y':
			Font_Details.color.r = 255;
			Font_Details.color.g = 227;
			Font_Details.color.b = 79;
			Font_Details.anonymous_23 = true;
			break;
#ifdef BUTTON_ICONS
		case 'U': Font_PS2Symbol = BUTTON_UP; break;
		case 'D': Font_PS2Symbol = BUTTON_DOWN; break;
		case '<': Font_PS2Symbol = BUTTON_LEFT; break;
		case '>': Font_PS2Symbol = BUTTON_RIGHT; break;
		case 'X': Font_PS2Symbol = BUTTON_CROSS; break;
		case 'O': Font_PS2Symbol = BUTTON_CIRCLE; break;
		case 'Q': Font_PS2Symbol = BUTTON_SQUARE; break;
		case 'T': Font_PS2Symbol = BUTTON_TRIANGLE; break;
		case 'K': Font_PS2Symbol = BUTTON_L1; break;
		case 'M': Font_PS2Symbol = BUTTON_L2; break;
		case 'A': Font_PS2Symbol = BUTTON_L3; break;
		case 'J': Font_PS2Symbol = BUTTON_R1; break;
		case 'V': Font_PS2Symbol = BUTTON_R2; break;
		case 'C': Font_PS2Symbol = BUTTON_R3; break;
		case 'H': Font_PS2Symbol = BUTTON_RSTICK_UP; break;
		case 'L': Font_PS2Symbol = BUTTON_RSTICK_DOWN; break;
		case '(': Font_PS2Symbol = BUTTON_RSTICK_LEFT; break;
		case ')': Font_PS2Symbol = BUTTON_RSTICK_RIGHT; break;
#endif
		}
	while(*s != '~') s++;
	if (*(++s) == '~')
		s = Font_ParseToken(s);
	return s;
}
#endif

void
Font_FilterOutTokensFromString(wchar *str)
{
	// Delegate to the C implementation
	FontText_FilterOutTokensFromString(str);
}

void
Font_DrawFonts(void)
{
	Font_RenderFontBuffer();
}

void
Font_SetScale(float x, float y)
{
#ifdef MORE_LANGUAGES
	/*if (IsJapanese()) {
		x *= 1.35f;
		y *= 1.25f;
	}*/
#endif
	Font_Details.scaleX = x;
	Font_Details.scaleY = y;
}

void
Font_SetSlantRefPoint(float x, float y)
{
	Font_Details.slantRefX = x;
	Font_Details.slantRefY = y;
}

void
Font_SetSlant(float s)
{
	Font_Details.slant = s;
}

void
Font_SetColor(CRGBA col)
{
	Font_Details.color = col;
	if (Font_Details.alphaFade < 255.0f)
		Font_Details.color.a *= Font_Details.alphaFade / 255.0f;
}

void
Font_SetJustifyOn(void)
{
	Font_Details.justify = true;
	Font_Details.centre = false;
	Font_Details.rightJustify = false;
}

void
Font_SetJustifyOff(void)
{
	Font_Details.justify = false;
	Font_Details.rightJustify = false;
}

void
Font_SetCentreOn(void)
{
	Font_Details.centre = true;
	Font_Details.justify = false;
	Font_Details.rightJustify = false;
}

void
Font_SetCentreOff(void)
{
	Font_Details.centre = false;
}

void
Font_SetWrapx(float x)
{
	Font_Details.wrapX = x;
}

void
Font_SetCentreSize(float s)
{
	Font_Details.centreSize = s;
}

void
Font_SetBackgroundOn(void)
{
	Font_Details.background = true;
}

void
Font_SetBackgroundOff(void)
{
	Font_Details.background = false;
}

void
Font_SetBackgroundColor(CRGBA col)
{
	Font_Details.backgroundColor = col;
}

void
Font_SetBackGroundOnlyTextOn(void)
{
	Font_Details.backgroundOnlyText = true;
}

void
Font_SetBackGroundOnlyTextOff(void)
{
	Font_Details.backgroundOnlyText = false;
}

void
Font_SetRightJustifyOn(void)
{
	Font_Details.rightJustify = true;
	Font_Details.justify = false;
	Font_Details.centre = false;
}

void
Font_SetRightJustifyOff(void)
{
	Font_Details.rightJustify = false;
	Font_Details.justify = false;
	Font_Details.centre = false;
}

void
Font_SetPropOff(void)
{
	Font_Details.proportional = false;
}

void
Font_SetPropOn(void)
{
	Font_Details.proportional = true;
}

void
Font_SetFontStyle(int16 style)
{
	if (style == FONT_HEADING) {
		Font_Details.style = FONT_STANDARD;
		Font_Details.bFontHalfTexture = true;
	} else {
		Font_Details.style = style;
		Font_Details.bFontHalfTexture = false;
	}
}

void
Font_SetRightJustifyWrap(float wrap)
{
	Font_Details.rightJustifyWrap = wrap;
}

void
Font_SetAlphaFade(float fade)
{
	Font_Details.alphaFade = fade;
}

void
Font_SetDropColor(CRGBA col)
{
	Font_Details.dropColor = col;
	if (Font_Details.alphaFade < 255.0f)
		Font_Details.dropColor.a *= Font_Details.alphaFade / 255.0f;
}

void
Font_SetDropShadowPosition(int16 pos)
{
	Font_Details.dropShadowPosition = pos;
}

wchar Font_FindNewCharacter(wchar c)
{
	// Delegate to the C implementation
	return FontText_FindNewCharacter(c);
}

wchar
Font_character_code(uint8 c)
{
	if(c < 128)
		return c;
	return foreign_table[c-128];
}
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
