# WadTest
Test editing the WAD file.

# Create a Wad File
```
Wad *wad = W_Create();
```
# Open Write Close
```
/* Open */
Wad *wad = W_Open(L"your wad file path");

/* Write */
W_Write(wad, L"output path");

/* Close */
W_Close(&wad);
```
# Add Entry
```
int add = W_Add(wad, hash, buffer, size, type);

if (add)
{
    printf("add failed\n");
}
```

# Change
```
int change = W_Change(wad, hash, newBuffer, newSize);

if (change)
{
    printf("change failed\n");
}
```

# Find
```
WADEntry *findEntry = W_Find(wad, hash);

if (findEntry)
{
    printf("xxhash: $I64u\n", findEntry->XXHash);
}
```

# Get Buffer
```
Buffer* buffer = W_GetBuffer(wad, hash);

if (buffer)
{
    printf("uncompressed size: %ld\n", buffer->dSize);
}
```

# Get Entry
```
WADEntry *entry = W_GetEntry(wad, index);

if (entry)
{
    printf("xxhash: $I64u\n", entry->XXHash);
}
```

# Remove
```
W_Remove(wad, hash);
```

# ForEach
```
/* Only takes the WADEntry *entry parameter */
W_ForEach(wad, func);

/* With Wad * Wad, WADEntry *entry parameter */
W_WForEach(wad, func);
```


# Simple
```
#include "wad.h"
#include <io.h>

void Write(Wad *wad, WADEntry *entry)
{
	char s[40] = {0};

	sprintf(s, "asset/%I64u", entry->XXHash);
	FILE *fp = fopen(&s, "wb+");

	if (fp)
	{
		Buffer *buffer = W_GetBuffer(wad, entry->XXHash);

		fwrite(buffer->dData, buffer->dSize, 1, fp);
	}
	fclose(fp);
}
int main()
{
	Wad *wad = W_Create();
	if (wad)
	{
		char *src = "c1a6w51tra32s1dcas231";
		char *dst = "_____________________";

		W_Add(wad, 1, src, strlen(src), Uncompressed);
		W_Add(wad, 2, src, strlen(src), GZipCompressed);
		W_Add(wad, 3, src, strlen(src), ZStandardCompressed);
		W_Add(wad, 4, src, strlen(src), FileRedirection);

		W_Change(wad, 1, dst, strlen(dst));
		W_Change(wad, 2, dst, strlen(dst));
		W_Change(wad, 3, dst, strlen(dst));
		W_Change(wad, 4, dst, strlen(dst));

		W_Find(wad, 1);
		W_Find(wad, 2);
		W_Find(wad, 3);
		W_Find(wad, 4);

		W_GetBuffer(wad, 1);
		W_GetBuffer(wad, 2);
		W_GetBuffer(wad, 3);
		W_GetBuffer(wad, 4);

		W_GetEntry(wad, 0);
		W_GetEntry(wad, 1);
		W_GetEntry(wad, 2);
		W_GetEntry(wad, 3);

		mkdir("asset");
		W_WForEach(wad, Write);
		W_Write(wad, L"asset.wad");

		W_Remove(wad, 1);
		W_Remove(wad, 2);
		W_Remove(wad, 3);
		W_Remove(wad, 4);

		W_Close(&wad);
	}
	return (0);
}
```
