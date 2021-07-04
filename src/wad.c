#include "wad.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int LoadWadFromPath(Wad *wad, const char *path)
{
    // open file
    FILE* input = fopen(path, "rb+");
    if (input == -1)
    {
        //
        return -1;
    }
    // malloc
    wad->Magic = (char*)malloc(3);
    // memory set
    memset(wad->Magic, '\0', 3);
    fseek(input, 0, SEEK_SET);
    // read magic
    fread(wad->Magic, 1, 2, input);
    // read major
    fread(&wad->Major, 1, 1, input);
    // read minor
    fread(&wad->Minor, 1, 1, input);

    // malloc signature
    wad->Signature = (char*)malloc(256);
    // read signature
    fread(wad->Signature, 256, 1, input);
    // pad ?
    fread(&wad->Pad, 8, 1, input);

    // read count
    fread(&wad->Count, 4, 1, input);

    wad->Entries = malloc(sizeof(WADEntry));
    wad->Entries->Next = NULL;
    for (int i = 0; i < wad->Count; i++)
    {
        WADEntry *node = malloc(sizeof(WADEntry));
        memset(&node->Type, 0, 4);// The enum is int32 ?

        fread(&node->XXHash, 8, 1, input);
        fread(&node->Offset, 4, 1, input);
        fread(&node->CompressedSize, 4, 1, input);
        fread(&node->UncompressedSize, 4, 1, input);
        fread(&node->Type, 1, 1, input);
        fread(&node->IsDuplicated, 1, 1, input);
        fread(&node->Pad, 2, 1, input);
        fread(&node->Checksum, 8, 1, input);

        long start = ftell(input);
        node->Buffer = malloc(node->CompressedSize);

        fseek(input, node->Offset, SEEK_SET);
        fread(node->Buffer, 1, node->CompressedSize, input);
        fseek(input, start, SEEK_SET);

        node->Next = wad->Entries->Next;
		wad->Entries->Next = node;
    }
    // close
    fclose(input);
}

int LoadWadFromHandle(Wad *wad, int handle)
{
    return 0;
}

int AddWadEntry(Wad *wad, void *buffer, size_t size, EntryType type)
{
}

int ChangeWadEntry(Wad *wad, uint64_t hash, void *buffer, size_t size)
{
}

WADEntry* FindWadEntry(Wad *wad, uint64_t hash)
{
    if (wad == NULL || wad->Entries == NULL)
    {
        return NULL;
    }
    WADEntry *temp = wad->Entries;
    while (temp->XXHash != hash)
    {
        if (temp == NULL)
        {
            break;
        }
        temp = temp->Next;
    }
    return temp;
}

WADEntry* GetWadEntryWithIndex(Wad *wad, int index)
{
    if (wad == NULL || wad->Entries == NULL || index < 0)
    {
        return NULL;
    }
    
    WADEntry *node, *temp;
    temp = wad->Entries;

	while (temp != NULL)
    {
		if (index-- == 0)
        {
			node = temp;
			break;
		}
		temp = temp->Next;
	}
    return node;
}

int RemoveWadEntry(Wad *wad, uint64_t hash)
{
}