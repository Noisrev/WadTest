#include "wad.h"
#include <string.h>

void PrintWad(int index, WADEntry *entry)
{
	printf("%d\n", index);
}

int main()
{
	char *key = "1a65w1tg2s1d3f2a156w1ar3w2d13as";
	char *value = "____________________________";

	Wad *wad = LoadWadFromPath(L"C:\\Apps\\Tencent\\英雄联盟\\Game\\DATA\\FINAL\\DATA.wad.client");

	if (wad)
	{
		AddWadEntry(wad, 1, key, strlen(key), Uncompressed);
		AddWadEntry(wad, 2, key, strlen(key), GZipCompressed);
		AddWadEntry(wad, 3, key, strlen(key), ZStandardCompressed);
		AddWadEntry(wad, 4, key, strlen(key), FileRedirection);

		ChangeWadEntry(wad, 1, value, strlen(value));
		ChangeWadEntry(wad, 2, value, strlen(value));
		ChangeWadEntry(wad, 3, value, strlen(value));
		ChangeWadEntry(wad, 4, value, strlen(value));

		WADEntry *a = FindWadEntry(wad, 1);
		WADEntry *b = FindWadEntry(wad, 2);
		WADEntry *c = FindWadEntry(wad, 3);
		WADEntry *d = FindWadEntry(wad, 4);

		WADEntry *a2 = GetWadEntryWithIndex(wad, 0);
		WADEntry *b2 = GetWadEntryWithIndex(wad, 1);
		WADEntry *c2 = GetWadEntryWithIndex(wad, 2);
		WADEntry *d2 = GetWadEntryWithIndex(wad, 3);

		Buffer *cBuff1 = GetBuffer(wad, 1, R_Compressed);
		Buffer *cBuff2 = GetBuffer(wad, 2, R_Compressed);
		Buffer *cBuff3 = GetBuffer(wad, 3, R_Compressed);
		Buffer *cBuff4 = GetBuffer(wad, 4, R_Compressed);

		Buffer *dBuff1 = GetBuffer(wad, 1, R_Uncmpressed);
		Buffer *dBuff2 = GetBuffer(wad, 2, R_Uncmpressed);
		Buffer *dBuff3 = GetBuffer(wad, 3, R_Uncmpressed);
		Buffer *dBuff4 = GetBuffer(wad, 4, R_Uncmpressed);

		RemoveWadEntry(wad, 1);
		RemoveWadEntry(wad, 2);
		RemoveWadEntry(wad, 3);
		RemoveWadEntry(wad, 4);

		W_ForEach(wad, PrintWad);
	}
	return (0);
}