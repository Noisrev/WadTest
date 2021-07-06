#include <zlib.h>
#include "zstd.h"
#include "wad.h"

// The wad is null ?
int IsNULL(Wad *wad)
{
    /* wad is NULL ? */
    if (wad == NULL)
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
        entry = NULL;
    }
}

void *w_malloc(size_t size)
{
    /* create */
    void *buffer = malloc(size);
    /* Set the buffer byte to 0 */
    memset(buffer, 0, size);
    /* Return */
    return buffer;
}

Wad *CreateWad()
{
    /* malloc a new Wad */
    Wad* wad = (Wad *)w_malloc(sizeof(Wad));
    /* Set the magic */
    wad->Magic[0] = 'R';
    wad->Magic[0] = 'W';
    /* Set the version */
    wad->Major = 3;
    wad->Minor = 1;
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
        /* malloc a new node */
        WADEntry *node = (WADEntry *)w_malloc(sizeof(WADEntry));

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
    /* a new node */
    WADEntry *node = (WADEntry *)w_malloc(sizeof(WADEntry));
    /* Set the buffer */
    node->Buffer = (Buffer *)w_malloc(sizeof(Buffer));
    /* Set the xxhash */
    node->XXHash = hash;
    /* Set the type */
    node->Type = type;
    /* Set the uncompressed size*/
    node->UncompressedSize = size;

    /* uncompressed */
    if (type == Uncompressed)
    {
        /* Set the buffer size */
        node->Buffer->Size = size;
        /* Set the compressed size */
        node->CompressedSize = size;
        /* Set the cache */
        node->Buffer->Cache = w_malloc(size);
        /* Copy the buffer to the cache */
        memcpy(node->Buffer->Cache, buffer, size);
    }
    else /* compressed */
    {
        /* destination */
        size_t dSize = 0;
        void *dBuff = NULL;
        /* gzip */
        if (type == GZipCompressed)
        {
            /* compressed size */
            dSize = compressBound(size);
            /* Set the dbuff */
            dBuff = w_malloc(dSize);

            /* compress */
            compress(dBuff, &dSize, buffer, size);
        } /* zstd */
        else if (type == ZStandardCompressed)
        {
            /* compressed size */
            dSize = ZSTD_compressBound(size);
            /* Set the dbuff */
            dBuff = w_malloc(dSize);

            /* compress */
            ZSTD_compress(dBuff, dSize, buffer, size, 23);
        } /* link */
        else
        {
            /* Set the dSize */
            dSize = size;
            /* Set the dBuff */
            dBuff = w_malloc(dSize);
            /* Copy the buffer to the dBuff */
            memcpy(dBuff, buffer, size);
        }
        /* Set the compressed size */
        node->CompressedSize = dSize;
        /* Set the cache */
        node->Buffer->Cache = dBuff;
        /* Set the size */
        node->Buffer->Size = dSize;
    }
    /* Set the checksum */
    node->Checksum = XXH_INLINE_XXH3_64bits(node->Buffer->Cache, node->Buffer->Size);
    /* node->head */
    node->Next = wad->Entries;
    wad->Entries = node;
    /* count + 1 */
    wad->Count++;

    /* Return true */
    return 1;
}

int ChangeWadEntry(Wad *wad, uint64_t hash, void *buffer, size_t size)
{
    if (IsNULL(wad))
    {
        return -1;
    }
    /* node */
    WADEntry *entry;
    /* Exist ? */
    if ((entry = FindWadEntry(wad, hash)))
    {
        /* Set the old buffer */
        Buffer *oldBuffer = entry->Buffer;
        /* create a new buffer */
        entry->Buffer = (Buffer*)w_malloc(sizeof(Buffer));
        /* Uncompressed */
        if (entry->Type == Uncompressed)
        {
            /* Set the buffer */
            entry->Buffer->Cache = w_malloc(size);
            entry->Buffer->Size = size;
            /* Copy the buffer */
            memcpy(entry->Buffer->Cache, buffer, size);
        } /* Gzip */
        else if (entry->Type == GZipCompressed)
        {
            /* Set the buffer */
            entry->Buffer->Size = compressBound(size);
            entry->Buffer->Cache = w_malloc(entry->Buffer->Size);
            /* compress */
            compress(entry->Buffer->Cache, &entry->Buffer->Size, buffer, size);
        } /* Zstd */
        else if (entry->Type == ZStandardCompressed)
        {
            /* Set the buffer */
            entry->Buffer->Size = ZSTD_compressBound(size);
            entry->Buffer->Cache = w_malloc(entry->Buffer->Size);
            /* compress */
            ZSTD_compress(entry->Buffer->Cache, entry->Buffer->Size, buffer, size, 23);
        } /* Link */
        else
        {
            /* Set the buffer */
            entry->Buffer->Size = size;
            entry->Buffer->Cache = w_malloc(entry->Buffer->Size);
            /* Copy the buffer*/
            memcpy(entry->Buffer->Cache, buffer, size);
        }
        /* free old buffer */
        _free_entry_buffer(oldBuffer);
        /* Set the compressed size */
        entry->CompressedSize = entry->Buffer->Size;
        /* Set the uncompressed size */
        entry->UncompressedSize = size;
        /* Set the checksum */
        entry->Checksum = XXH_INLINE_XXH3_64bits(entry->Buffer->Cache, entry->Buffer->Size);
        /* Return true */
        return 1;
    }
    else
    {
        /* Return */
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
        /* Buffer is not null ? */
        if (entry->Buffer) /* true */
        {
            /* Return */
            return entry->Buffer;
        }
        /* Set the buffer*/
        entry->Buffer = malloc(sizeof(Buffer));
        /* malloc */
        void *compressBuffer = w_malloc(entry->CompressedSize);
        /* Seek */
        fseek(wad->Buffer, entry->Offset, SEEK_SET);
        /* Read buffer */
        fread(compressBuffer, 1, entry->CompressedSize, wad->Buffer);
        /* Data Type */
        if (R_Comp) /* compressed */
        {
            /* Set the cache */
            entry->Buffer->Cache = compressBuffer;
            /* Set the size*/
            entry->Buffer->Size = entry->CompressedSize;
        } /* uncompressed */
        else
        {
            /* uncompressed */
            if (entry->Type == Uncompressed)
            {
                /* Set the cache */
                entry->Buffer->Cache = compressBuffer;
            }
            else
            {
                /* malloc a new cache */
                entry->Buffer->Cache = w_malloc(entry->UncompressedSize);

                if (entry->Type == GZipCompressed)
                {
                    /* gzip decompress */
                    uncompress(entry->Buffer->Cache, &entry->UncompressedSize, compressBuffer, entry->CompressedSize);
                }
                else if (entry->Type == ZStandardCompressed)
                {
                    /* zstd decompress */
                    ZSTD_decompress(entry->Buffer->Cache, entry->UncompressedSize, compressBuffer, entry->CompressedSize);
                }
                else
                {
                    /* link ? */
                    memcpy(entry->Buffer->Cache, compressBuffer + 4, entry->CompressedSize - 4);
                }
            }
            /* Set the size */
            entry->Buffer->Size = entry->UncompressedSize;
            /* free the compressBuffer */
            free(compressBuffer);
        }
        /* Return */
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
        /* Now. Entries is the second element */
        wad->Entries = cur->Next;
        /* Count - 1 */
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
        /* cur is not NULL */
        while (cur != NULL)
        {
            /* Equal ? */
            if (cur->XXHash == hash)
            {
                /* Set the next node */
                WADEntry *next = cur->Next;
                /* head->cur->next */
                /* Now, it's head->next */
                head->Next = next;
                /* count - 1*/
                wad->Count--;

                /* free the cur */
                _free_entry(cur);
                /* Return */
                break;
            }
            else
            {
                /* to Next */
                head = cur;
                cur = cur->Next;
            }
        }
    }
}

void W_Close(Wad **wad)
{
    W_ForEach(*wad, _free_entry);
    free(*wad);
    *wad = NULL;
}

void W_ForEach(Wad *wad, void (*func)(WADEntry *entry))
{
    /* The first node */
    WADEntry *entry = wad->Entries;
    /* entry is not NULL */
    while (entry)
    {
        /* invoke */
        (*func)(entry);
        /* to Next */
        entry = entry->Next;
    }
}