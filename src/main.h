#ifndef MORSCONV_MAIN_H
#define MORSCONV_MAIN_H

extern struct Library
	*SysBase,
	*DOSBase,
	*UtilityBase,
	*MathIeeeSingBasBase,
	*MathIeeeSingTransBase,
	*IFFParseBase;
	

#ifdef __mc68000__

#define mul16(a,b) ({ \
long _r; \
short _a = (a), _b = (b); \
asm("MULS.W %2,%0": "=d" (_r): "0" (_a), "dmi" (_b): "cc"); \
_r;})

#define div16(a,b) ({ \
short _r, _b = (b); \
long _a = (a); \
asm("DIVS.W %2,%0": "=d" (_r): "0" (_a), "dmi" (_b): "cc"); \
_r;})

#define bswap16(a) ({ \
short _r, _a = (a); \
asm ("ROR.W #8,%0": "=d" (_r) : "0" (_a) : "cc"); \
_r;})

#define bswap32(a) ({ \
long _r, _a = (a); \
asm("ROR.W #8,%0; SWAP %0; ROR.W #8,%0" : "=d" (_r) : "0" (_a) : "cc"); \
_r;})

#else

#define mul16(a,b) a * b
#define div16(a,b) a / b

#endif

#endif  /* MORSCONV_MAIN_H */
