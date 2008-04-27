void M_InitDynLump();
const void *W_CacheLumpNumDyn();

struct text_charmap_entry {
	char character;
	char *source_lump;
	int x1;
	int y1;
	int x2;
	int y2;
};
struct text_charmap_entry charmap_entries[] = {
	{ 'f', "M_SFXVOL", 0, 0, 0, 0 },
	{ 'r', "M_SCRNSZ", 0, 0, 0, 0 },
	{ 'a', "M_QUITG" , 0, 0, 0, 0 },
	{ 'g', "M_SAVEG" , 0, 0, 0, 0 },
	{ 's', "M_MUSVOL", 0, 0, 0, 0 },
	{ 'o', "M_MUSVOL", 0, 0, 0, 0 },
	{ 'u', "M_QUITG" , 0, 0, 0, 0 },
	{ 't', "M_QUITG" , 0, 0, 0, 0 },
	{ 'A', "M_NEWG"  , 0, 0, 0, 0 },
	{ 'm', "M_QUITG" , 0, 0, 0, 0 },
	{ 'p', "M_OPTION", 0, 0, 0, 0 },
	{ 'C', "M_SKILL",  0, 0, 0, 0 },
	{ 'S', "M_SFXVOL", 0, 0, 0, 0 },
	{ 'i', "M_QUITG" , 0, 0, 0, 0 },
	{ 'n', "M_SVOL"  , 0, 0, 0, 0 },
	{ 'l', "M_MUSVOL", 0, 0, 0, 0 },
	{ 'y', "M_MSENS",  0, 0, 0, 0 },
	{ 'e', "M_QUITG" , 0, 0, 0, 0 },
	{ 'G', "M_QUITG" , 0, 0, 0, 0 },
	{ 'z', "M_SCRNSZ", 0, 0, 0, 0 },
	{ 'd', "M_SVOL"  , 0, 0, 0, 0 },
	{ 'M', "M_MUSVOL", 0, 0, 0, 0 },
	{ 'H', "M_HURT"  , 0, 0, 0, 0 },
	{ 'U', "M_ULTRA" , 0, 0, 0, 0 },
	{ 'v', "M_MSENS",  0, 0, 0, 0 },
	{ 'c', "M_MUSVOL", 0, 0, 0, 0 },
	{ 'w', "M_NGAME" , 0, 0, 0, 0 },
	{ 'W', "M_NEWG"  , 0, 0, 0, 0 },
	{ 'h', "M_SKILL",  0, 0, 0, 0 },
	{},
};
/*
	characters missing from the above (that we need):
	b: missing
	D: missing
	F: missing
	K: missing
	B: missing
	/: missing
	P: missing
*/
