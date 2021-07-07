#include "wad.h"

int main()
{
	Wad *wad = W_Create();
	W_Close(&wad);
	return (0);
}