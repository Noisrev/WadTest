#include "wad.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main()
{
	Wad* wad = LoadWadFromPath(L"C:\\Apps\\Tencent\\英雄联盟\\Game\\DATA\\FINAL\\DATA.wad.client");
	WADEntry* entry = GetWadEntryWithIndex(wad, 1);
	WADEntry* find = FindWadEntry(wad, entry->XXHash);
	
	RemoveWadEntry(wad, entry->XXHash);
	return (0);
}