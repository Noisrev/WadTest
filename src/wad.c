#include <zlib.h>
#include "zstd.h"
#include "wad.h"

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

/* free the entry->buffer */
void _free_entry_buffer(Buffer *buffer)
{
    /* The buffer is not NULL? */
    if (buffer) /* true */
    {
        if (buffer->Cache)
        {
            free(buffer->Cache);
        }
        /* free the buffer */
        free(buffer);
        buffer = NULL;
    }
}
/* free the entry. */
void _free_entry(WADEntry *entry)
{
    /* The entry is not NULL*/
    if (entry) /* true */
    {
        /* free the buffer */
        _free_entry_buffer(entry->Buffer);
        /* free the entry */
        free(entry);
    }
}

Wad *CreateWad()
{
    Wad *wad = malloc(sizeof(Wad));
    memset(wad, 0, sizeof(Wad));
    return wad;
}
Wad *LoadWadFromPath(const wchar_t *path)
{
    /* malloc */
    Wad *wad = CreateWad();
    // open file
    wad->Buffer = _wfopen(path, L"rb+,ccs=UNICODE");

    if (wad->Buffer == NULL)
    {
        /* Open fail */
        return NULL;
    }

    fread(wad, 1, 268, wad->Buffer);       /* header */
    fread(&wad->Count, 1, 4, wad->Buffer); /* count */

    /* Set the first is NULL */
    wad->Entries = NULL;
    for (int i = 0; i < wad->Count; i++)
    {
        WADEntry *node = malloc(sizeof(WADEntry));
        memset(&node->Type, 0, sizeof(node->Type)); // The enum is int32 ?

        fread(&node->XXHash, 8, 1, wad->Buffer);
        fread(&node->Offset, 4, 1, wad->Buffer);
        fread(&node->CompressedSize, 4, 1, wad->Buffer);
        fread(&node->UncompressedSize, 4, 1, wad->Buffer);
        fread(&node->Type, 1, 1, wad->Buffer);
        fread(&node->IsDuplicated, 1, 1, wad->Buffer);
        fread(&node->Pad, 2, 1, wad->Buffer);
        fread(&node->Checksum, 8, 1, wad->Buffer);
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
    /* wad is NULL ? */
    if (IsNULL(wad)) /* true */
    {
        /* */
        return -1;
    }
    /* Exist ? */
    if (FindWadEntry(wad, hash))
    {
        /* Return */
        return 0;
    }

    WADEntry *node = malloc(sizeof(WADEntry));
    node->Buffer = malloc(sizeof(Buffer));
    node->XXHash = hash;
    node->Type = type;
    node->UncompressedSize = size;

    if (type == Uncompressed)
    {
        node->Buffer->Size = size;
        node->CompressedSize = size;
        node->Buffer->Cache = malloc(size);
        memcpy(node->Buffer->Cache, buffer, size);
    }
    else
    {
        size_t dSize = 0;
        void *dBuff = NULL;
        if (type == GZipCompressed)
        {
            dSize = compressBound(size);
            dBuff = malloc(dSize);

            compress(dBuff, &dSize, buffer, size);
        }
        else if (type == ZStandardCompressed)
        {
            dSize = ZSTD_compressBound(size);
            dBuff = malloc(dSize);

            ZSTD_compress(dBuff, dSize, buffer, size, 23);
        }
        else
        {
            dSize = size;
            dBuff = malloc(dSize);
            memcpy(dBuff, buffer, size);
        }
        node->Checksum = XXH_INLINE_XXH3_64bits(dBuff, dSize);
        node->CompressedSize = dSize;
        node->Buffer->Cache = dBuff;
        node->Buffer->Size = dSize;
    }
    node->Next = wad->Entries;
    wad->Entries = node;
    wad->Count++;

    return 1;
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
        Buffer *oldBuffer = entry->Buffer;
        entry->Buffer = malloc(sizeof(Buffer));
        if (entry->Type == Uncompressed)
        {
            entry->Buffer->Cache = malloc(size);
            entry->Buffer->Size = size;
            memcpy(entry->Buffer->Cache, buffer, size);
        }
        else if (entry->Type == GZipCompressed)
        {
            entry->Buffer->Size = compressBound(size);
            entry->Buffer->Cache = malloc(entry->Buffer->Size);

            compress(entry->Buffer->Cache, &entry->Buffer->Size, buffer, size);
        }
        else if (entry->Type == ZStandardCompressed)
        {
            entry->Buffer->Size = ZSTD_compressBound(size);
            entry->Buffer->Cache = malloc(entry->Buffer->Size);

            ZSTD_compress(entry->Buffer->Cache, entry->Buffer->Size, buffer, size, 23);
        }
        else
        {
            entry->Buffer->Size = size;
            entry->Buffer->Cache = malloc(entry->Buffer->Size);
            memcpy(entry->Buffer->Cache, buffer, size);
        }
        _free_entry_buffer(oldBuffer);

        entry->CompressedSize = entry->Buffer->Size;
        entry->UncompressedSize = size;

        entry->Checksum = XXH_INLINE_XXH3_64bits(entry->Buffer->Cache, entry->Buffer->Size);

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

void W_ForEach(Wad *wad, void(*func)(int index, WADEntry* entry))
{
    WADEntry *entry = wad->Entries;
    int index = 0;
    while(entry)
    {
        (*func)(index, wad->Entries);
        entry = entry->Next;
        index++;
    }
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
        /* Buffer is not null ? */
        if (entry->Buffer) /* true */
        {
            /* Return */
            return entry->Buffer;
        }

        entry->Buffer = malloc(sizeof(Buffer));
        /* malloc */
        void *compressBuffer = malloc(entry->CompressedSize);

        fseek(wad->Buffer, entry->Offset, SEEK_SET);
        fread(compressBuffer, 1, entry->CompressedSize, wad->Buffer);
        if (R_Comp) /* compressed */
        {
            entry->Buffer->Cache = compressBuffer;
            entry->Buffer->Size = entry->CompressedSize;
        }
        else
        {
            if (entry->Type == Uncompressed)
            {
                entry->Buffer->Cache = compressBuffer;
            }
            else
            {
                entry->Buffer->Cache = malloc(entry->UncompressedSize);

                if (entry->Type == GZipCompressed)
                {
                    uncompress(entry->Buffer->Cache, &entry->UncompressedSize, compressBuffer, entry->CompressedSize);
                }
                else if (entry->Type == ZStandardCompressed)
                {
                    ZSTD_decompress(entry->Buffer->Cache, entry->UncompressedSize, compressBuffer, entry->CompressedSize);
                }
                else
                {
                    memcpy(entry->Buffer->Cache, compressBuffer + 4, entry->CompressedSize - 4);
                }
            }
            entry->Buffer->Size = entry->UncompressedSize;
            free(compressBuffer);
        }
        return entry->Buffer;
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
        return;
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