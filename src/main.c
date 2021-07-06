#include "wad.h"
#include <string.h>

int main()
{
	Wad *wad = LoadWadFromPath(L"C:\\Apps\\Tencent\\英雄联盟\\Game\\DATA\\FINAL\\UI.wad.client");

	if (wad)
	{
		W_Write(wad, L"Test.wad");
		W_Close(&wad);
	}
	return (0);
}