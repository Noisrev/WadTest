#include "wad.h"
#include <stdio.h>
#include <stdlib.h>
int main()
{
	Wad *wad = malloc(sizeof(Wad));
	int a = LoadWadFromPath(wad, "C:\\Users\\Noisr\\Downloads\\UI.wad.client");
	WADEntry* entry = GetWadEntryWithIndex(wad, wad->Count);
	WADEntry* find = FindWadEntry(wad, entry->XXHash);
	return (0);
}