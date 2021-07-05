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
	
	//RemoveWadEntry(wad, entry->XXHash);

	FILE *output = fopen("output.a", "wb+");
	Buffer *buff = GetBuffer(wad, entry->Next->Next->Next->Next->Next->XXHash, R_Uncmpressed);
	size_t wsize = fwrite(buff->Cache, buff->Size, 1, output);
	fclose(output);
	return (0);
}