#include "wad.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int IsNULL(Wad *wad)
{
    if (wad == NULL || wad->Entries == NULL)
    {
        return 1;
    }
    return 0;
}

void _free_entry(WADEntry *entry)
{
    if (entry)
    {
        if (entry->Buffer)
        {
            free(entry->Buffer);
        }
        free(entry);
    }
}

Wad *LoadWadFromPath(const wchar_t *path)
{
    Wad* wad = malloc(sizeof(Wad));
    // open file
    wad->Buffer = _wfopen(path, L"rb+,ccs=UNICODE");
    if (wad->Buffer == -1)
    {
        //
        return -1;
    }
    fread(wad, 1, 268, wad->Buffer); // header
    fread(&wad->Count, 1, 4, wad->Buffer); // count
    
    wad->Entries = NULL;
    for (int i = 0; i < wad->Count; i++)
    {
        WADEntry *node = malloc(sizeof(WADEntry));
        memset(&node->Type, 0, sizeof(node->Type));// The enum is int32 ?

        fread(node, 32, 1, wad->Buffer);

        node->Buffer = NULL;
        node->Next = wad->Entries;
		wad->Entries = node;
    }
    return wad;
}
int AddWadEntry(Wad *wad, uint64_t hash, void *buffer, size_t size, EntryType type)
{
    if (IsNULL(wad))
    {
        return -1;
    }
}

int ChangeWadEntry(Wad *wad, uint64_t hash, void *buffer, size_t size)
{
    if (IsNULL(wad))
    {
        return -1;
    }
    WADEntry *entry;
    if ((entry = FindWadEntry(wad, hash)))
    {
        // Do something...
        return 0;
    }
    else
    {
        return AddWadEntry(wad, hash, buffer, size, ZStandardCompressed);
    }
}

WADEntry* FindWadEntry(Wad *wad, uint64_t hash)
{
    if (IsNULL(wad))
    {
        return NULL;
    }
    WADEntry *temp = wad->Entries;
    while (temp && temp->XXHash != hash)
    {
        temp = temp->Next;
    }
    return temp;
}

void *GetBuffer(Wad *wad, uint64_t hash, int R_Comp)
{

}
WADEntry* GetWadEntryWithIndex(Wad *wad, int index)
{
    if (IsNULL(wad) || index < 0)
    {
        return NULL;
    }
    
    if (index == 0)
    {
        return wad->Entries;
    }

    WADEntry *node, *temp;
    temp = wad->Entries;

    index--;
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

void RemoveWadEntry(Wad *wad, uint64_t hash)
{
    if (IsNULL(wad))
    {
        return 1;
    }
    
    if (wad->Entries->XXHash == hash)
    {
        WADEntry *cur = wad->Entries;
        wad->Entries = cur->Next;
        wad->Count--;

        _free_entry(cur);
    }
    else
    {
        WADEntry *head = wad->Entries;
        WADEntry *cur  = head->Next;
        while (cur != NULL)
        {
            if (cur->XXHash == hash)
            {
                WADEntry *next = cur->Next;
                head->Next = next;
                wad->Count--;

                _free_entry(cur);
                break;
            }
            else
            {
                head = cur;
                cur  = cur->Next;
            }
        }
        
    }
}