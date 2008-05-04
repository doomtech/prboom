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

struct dynlump {
	char *lumpname;
	char *text;
	int lumpnum;
	int width;
	int height;
};
