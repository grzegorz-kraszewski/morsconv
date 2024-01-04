extern struct Library
	*SysBase,
	*DOSBase,
	*UtilityBase,
	*MathIeeeSingBasBase,
	*MathIeeeSingTransBase,
	*IFFParseBase;
	
extern ULONG MorErr;

static inline SetErr(LONG retval, LONG ecode)
{
	MorErr = (retval << 16) | ecode;
}

#ifdef __mc68000__

#define mul16(a,b) ({ \
LONG _r; \
WORD _a = (a), _b = (b); \
asm("MULS.W %2,%0": "=d" (_r): "0" (_a), "dmi" (_b): "cc"); \
_r;})

#define div16(a,b) ({ \
WORD _r, _b = (b); \
LONG _a = (a); \
asm("DIVS.W %2,%0": "=d" (_r): "0" (_a), "dmi" (_b): "cc"); \
_r;})

#else

#define mul16(a,b) a * b
#define div16(a,b) a / b

#endif

