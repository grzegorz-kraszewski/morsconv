/* Structure holding all the GUI stuff. */


extern struct Library
	*SysBase,
	*DOSBase,
	*UtilityBase,
	*MathIeeeSingBasBase,
	*MathIeeeSingTransBase;

struct Rect
{
	WORD x;
	WORD y;
	WORD w;
	WORD h;
};


struct Gui
{
	struct Screen    *g_Screen;
	struct Window    *g_Window;
	struct DrawInfo  *g_DrawInfo;
	WORD              g_TextSize;
	WORD              g_BorderThk;
	WORD              g_WinWidth;
	WORD              g_WinHeight;
	struct Rect       g_SampleView;
};

extern ULONG MorErr;


static inline SetErr(LONG retval, LONG ecode)
{
	MorErr = (retval << 16) | ecode;
}
