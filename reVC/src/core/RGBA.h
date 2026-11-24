//+ rouz edit (Claude)
#pragma once
#include <stdint.h>

// Colour with red, green, blue and alpha bytes, shared with C: C sees the plain data, C++ also gets the constructors, comparisons and RenderWare conversions
typedef struct CRGBA CRGBA;
struct CRGBA
{
	union
	{
		uint32_t color32;
		struct { uint8_t r, g, b, a; };
		struct { uint8_t red, green, blue, alpha; };
#if defined(__cplusplus) && defined(RWCORE_H)
		struct  { RwRGBA rwRGBA; };
#endif
	};
#ifdef __cplusplus

	CRGBA(void) { }
	CRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) { }

	bool operator ==(const CRGBA &right)
	{
		return this->r == right.r && this->g == right.g && this->b == right.b && this->a == right.a;
	}

	bool operator !=(const CRGBA &right)
	{
		return !(*this == right);
	}

	CRGBA &operator =(const CRGBA &right)
	{
		this->r = right.r;
		this->g = right.g;
		this->b = right.b;
		this->a = right.a;
		return *this;
	}
#ifdef RWCORE_H
	operator RwRGBA &(void) {
		return rwRGBA;
	}

	operator RwRGBA *(void) {
		return &rwRGBA;
	}

	operator RwRGBA (void) const {
		return rwRGBA;
	}

	CRGBA &operator =(const RwRGBA &right)
	{
		this->r = right.red;
		this->g = right.green;
		this->b = right.blue;
		this->a = right.alpha;
		return *this;
	}
#endif
#endif
};

#ifdef __cplusplus
static_assert(sizeof(CRGBA) == 4, "CRGBA must stay four bytes so C sees the same layout");
#endif
//- rouz edit (Claude)
