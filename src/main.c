#include "wad.h"
#include <stdio.h>
#include <stdlib.h>
int main()
{
	Wad *wad = malloc(sizeof(Wad));
	int a = LoadWadFromPath(wad, L"C:\\Apps\\Tencent\\英雄联盟\\Game\\DATA\\FINAL\\DATA.wad.client");
	WADEntry* entry = GetWadEntryWithIndex(wad, wad->Count);
	WADEntry* find = FindWadEntry(wad, entry->XXHash);
	
	return (0);
}