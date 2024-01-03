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
