#include "wad.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <zlib.h>
#include <zstd.h>

// The wad is null ?
int IsNULL(Wad *wad)
{
    if (wad == NULL || wad->Entries == NULL)
    {
        // true
        return 1;
    }
    // return
    return 0;
}

/* free the entry. */
void _free_entry(WADEntry *entry)
{
    /* The entry is not NULL*/
    if (entry) /* true */
    {
        /* The buffer is not NULL? */
        if (entry->Buffer) /* true */
        {
            /* free the buffer */
            free(entry->Buffer);
        }
        /* free the entry */
        free(entry);
    }
}

Wad *LoadWadFromPath(const wchar_t *path)
{
    /* malloc */
    Wad *wad = malloc(sizeof(Wad));
    // open file
    wad->Buffer = _wfopen(path, L"rb+,ccs=UNICODE");

    if (wad->Buffer == -1)
    {
        /* Open fail */
        return -1;
    }

    fread(wad, 1, 268, wad->Buffer);       /* header */
    fread(&wad->Count, 1, 4, wad->Buffer); /* count */

    /* Set the first is NULL */
    wad->Entries = NULL;
    for (int i = 0; i < wad->Count; i++)
    {
        WADEntry *node = malloc(sizeof(WADEntry));
        /* Set the type */
        memset(&node->Type, 0, sizeof(node->Type)); // The enum is int32 ?

        /* Eead entry header*/
        fread(node, 32, 1, wad->Buffer);

        /* Set buffer */
        node->Buffer = NULL;
        /* Set to the header node */
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
    if (FindWadEntry(wad, hash))
    {
        return 0;
    }
    
    WADEntry *node = malloc(sizeof(WADEntry));
    node->XXHash = hash;

    if (type == Uncompressed)
    {
        // Do something...
    }
    else if (type == GZipCompressed)
    {
        // Do something...
    }
    else if (type == ZStandardCompressed)
    {
        // Do something...
    }
    else
    {
        // Do something...
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
        return 1;
    }
    else
    {
        return 0;
    }
}

WADEntry *FindWadEntry(Wad *wad, uint64_t hash)
{
    if (IsNULL(wad))
    {
        return NULL;
    }
    /* Set the Head Node */
    WADEntry *temp = wad->Entries;
    /* Temp is not null and the hash does not match */
    while (temp && temp->XXHash != hash)
    {
        /* to Next */
        /* if next is null. temp = next, and return temp */
        temp = temp->Next;
    }
    /* Return */
    return temp;
}

Buffer *GetBuffer(Wad *wad, uint64_t hash, int R_Comp)
{
    if (IsNULL(wad))
    {
        return NULL;
    }
    WADEntry *entry;
    /* Find the entry */
    if ((entry = FindWadEntry(wad, hash)))
    {
        Buffer *buff = malloc(sizeof(Buffer));
        /* malloc */
        void *compressBuffer = malloc(entry->CompressedSize);
        
        fseek(wad->Buffer, entry->Offset, SEEK_SET);
        fread(compressBuffer, entry->CompressedSize, 1, wad->Buffer);
        if (R_Comp) /* compressed */
        {
            buff->Cache = compressBuffer;
            buff->Size = entry->CompressedSize;
        }
        else
        {
            if (entry->Type == Uncompressed)
            {
                buff->Cache = compressBuffer;
                buff->Size = entry->CompressedSize;
            }
            else if (entry->Type == GZipCompressed)
            {
                void *dstBuff = malloc(entry->UncompressedSize);
                z_stream z = {0};

                z.next_in = compressBuffer;
                z.avail_in = entry->CompressedSize;
                z.next_out = dstBuff;
                z.avail_out = entry->UncompressedSize;

                if (inflateInit(&z) != Z_OK)
                {
                    printf("inflateInit failed!\n");
                    return -1;
                }

                if (inflate(&z, Z_NO_FLUSH) != Z_STREAM_END)
                {
                    printf("inflate Z_NO_FLUSH failed!\n");
                    return -1;
                }

                if (inflate(&z, Z_FINISH) != Z_STREAM_END)
                {
                    printf("inflate Z_FINISH failed!\n");
                    return -1;
                }

                if (inflateEnd(&z) != Z_OK)
                {
                    printf("inflateEnd failed!\n");
                    return -1;
                }

                buff->Cache = dstBuff;
                buff->Size = entry->UncompressedSize;

                free(compressBuffer);
            }
            else if (entry->Type == ZStandardCompressed)
            {
                buff->Cache = malloc(entry->UncompressedSize);
                buff->Size = entry->UncompressedSize;
                
                size_t dSize = ZSTD_decompress(buff->Cache, buff->Size, compressBuffer, entry->CompressedSize);


                free(compressBuffer);
            }
            else
            {
                buff->Cache = NULL;
                buff->Size = 0;
            }
        }
        return buff;
    }
    return NULL;
}
WADEntry *GetWadEntryWithIndex(Wad *wad, int index)
{
    if (IsNULL(wad) || index < 0)
    {
        return NULL;
    }
    /* is first ? */
    if (index == 0)
    {
        /* Return */
        return wad->Entries;
    }

    WADEntry *node, *temp;
    /* Set the head node */
    temp = wad->Entries;

    /* The maximum index must be length -1 */
    index--;
    while (temp != NULL) /* temp is not null */
    {
        /* index = 2                             _______________
         * temp       --------->                 |______0______|
         * index-- == 0, false. now, temp is --> |______1______|
         *                                       |______2______|
         *                                       |_____..._____|
         * 
         * index = 1                             _______________
         *                                       |______0______|
         * temp       --------->                 |______1______|
         * index-- == 0, true . now, temp is --> |______2______|
         *                                       |_____..._____|
         */
        if (index-- == 0) /* is 0 */
        {
            /* The specified index was reached */
            node = temp;
            break;
        }
        /* to Next */
        temp = temp->Next;
    }
    /* Return */
    return node;
}

void RemoveWadEntry(Wad *wad, uint64_t hash)
{
    if (IsNULL(wad))
    {
        return 1;
    }

    /* is first ? */
    if (wad->Entries->XXHash == hash)
    {
        /* head->next
         * 
         * head = next
         * now, the head is head->next.
         * 
         * free head
         */
        WADEntry *cur = wad->Entries;
        wad->Entries = cur->Next;
        wad->Count--;

        _free_entry(cur);
    }
    else
    {
        /* Head->cur->next
         * 
         * if is cur
         * 
         * Head->next
         * 
         * free cur
         */
        WADEntry *head = wad->Entries;
        WADEntry *cur = head->Next;
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
                cur = cur->Next;
            }
        }
    }
}