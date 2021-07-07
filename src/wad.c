/* Author: Noisrev */
#include <inttypes.h>
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
        /* free the cData */
        free(buffer->cData);
        /* free the dData */
        free(buffer->dData);
        /* free the buffer */
        free(buffer);
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

void *w_malloc(size_t size)
{
    /* create */
    void *buffer = malloc(size);
    /* Set the buffer byte to 0 */
    memset(buffer, 0, size);
    /* Return */
    return buffer;
}

Wad *W_Create()
{
    /* malloc a new Wad */
    Wad *wad = (Wad *)w_malloc(sizeof(Wad));
    /* Set the magic */
    wad->Magic[0] = 'R';
    wad->Magic[1] = 'W';
    /* Set the version */
    wad->Major = 3;
    wad->Minor = 1;
    return wad;
}
Wad *W_Open(const wchar_t *path)
{
    /* malloc */
    Wad *wad = W_Create();
    // open file
    wad->Buffer = _wfopen(path, L"rb+,ccs=UNICODE");

    /* The buffer is NULL ? */
    if (wad->Buffer == NULL)
    {
        /* Open fail */
        return NULL;
    }
    /* Read the header */
    if ((fread(wad, 1, 268, wad->Buffer) < 268))
    {
        /* print error */
        printf("W_Open: \"Header buffer is too small: %ld\"\n", ftell(wad->Buffer));
        /* Close the wad */
        W_Close(&wad);
        /* Return */
        return NULL;
    }
    /* Check magic code */
    if (wad->Magic[0] != 'R' && wad->Magic[1] != 'W')
    {
        /* print error */
        printf("W_Open: \"Invalid magic code: %c%c\"\n", wad->Magic[0], wad->Magic[1]);
        /* Close the wad */
        W_Close(&wad);
        /* Return */
        return NULL;
    }
    /* Is version 3.1? */
    if (wad->Major != 3 && wad->Minor != 1)
    {
        /* print */
        printf("W_Open: \"Invalid wad version : %d.%d\"", wad->Major, wad->Minor);
        /* Close the wad */
        W_Close(&wad);
        /* Return */
        return NULL;
    }
    /* Read the count*/
    fread(&wad->Count, 1, 4, wad->Buffer);

    /* Set the first is NULL */
    wad->Entries = NULL;
    /* Loop. Read header properties */
    for (int i = 0; i < wad->Count; i++)
    {
        /* malloc a new node */
        WADEntry *node = (WADEntry *)w_malloc(sizeof(WADEntry));
        /* Read the xxhash */
        fread(&node->XXHash, 8, 1, wad->Buffer);
        /* Read the offset */
        fread(&node->Offset, 4, 1, wad->Buffer);
        /* Read the compressed size */
        fread(&node->CompressedSize, 4, 1, wad->Buffer);
        /* Read the uncompressed size */
        fread(&node->UncompressedSize, 4, 1, wad->Buffer);
        /* Read the type */
        fread(&node->Type, 1, 1, wad->Buffer);
        /* Read the 'is duplicated' */
        fread(&node->IsDuplicated, 1, 1, wad->Buffer);
        /* Read the pad */
        fread(&node->Pad, 2, 1, wad->Buffer);
        /* Read the checksum */
        fread(&node->Checksum, 8, 1, wad->Buffer);
        /* Set to the header node */
        node->Next = wad->Entries;
        /* to Next */
        wad->Entries = node;
    }
    /* Return */
    return wad;
}
int W_Add(Wad *wad, uint64_t hash, void *buffer, size_t size, EntryType type)
{
    /* wad is NULL ? */
    if (IsNULL(wad)) /* true */
    {
        /* */
        return -1;
    }
    /* Exist ? */
    if (W_Find(wad, hash))
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
    /* Set the is duplicated */
    node->IsDuplicated = 0;

    /* Set the uncompressed size*/
    node->UncompressedSize = size;
    /* Set the dSize */
    node->Buffer->dSize = size;
    /* Set the cache */
    node->Buffer->dData = w_malloc(size);
    /* Copy the buffer to the cache */
    memcpy(node->Buffer->dData, buffer, size);

    /* uncompressed */
    if (type == Uncompressed)
    {
        /* Set the cSize */
        node->Buffer->cSize = size;
        /* Set the compressed size */
        node->CompressedSize = size;
        /* Set the cache */
        node->Buffer->cData = w_malloc(size);
        /* Copy the buffer to the cache */
        memcpy(node->Buffer->cData, buffer, size);
    }
    else /* compressed */
    {
        /* destination */

        /* size */
        size_t dSize = 0;
        /* buffer */
        void *dBuff = NULL;

        /* gzip */
        if (type == GZipCompressed)
        {
            /* compressed size */
            dSize = compressBound(size);
            /* Set the dbuff */
            dBuff = w_malloc(dSize);

            /* compress */
            if (compress(dBuff, &dSize, buffer, size) != Z_OK)
            {
                /* print error */
                printf("W_Add: \"compress failed : %" PRIu64 "\"\n", hash);
                /* free the dBuff */
                free(dBuff);
                /* free the entry */
                _free_entry(node);
                /* Return */
                return -1;
            }
        } /* zstd */
        else if (type == ZStandardCompressed)
        {
            /* compressed size */
            dSize = ZSTD_compressBound(size);
            /* Set the dbuff */
            dBuff = w_malloc(dSize);

            /* compress */
            ZSTD_compress(dBuff, dSize, buffer, size, 3);
        } /* link */
        else
        {
            /* Set the dSize */
            dSize = size + 4;
            /* Set the dBuff */
            dBuff = w_malloc(dSize);
            /* Copy the buffer to the dBuff */
            memcpy(dBuff, &size, 4);
            /* Copy the buffer to the dBuff */
            memcpy(dBuff + 4, buffer, size);
        }
        /* Set the compressed size */
        node->CompressedSize = dSize;
        /* Set the cache */
        node->Buffer->cData = dBuff;
        /* Set the size */
        node->Buffer->cSize = dSize;
    }
    /* Set the checksum */
    node->Checksum = XXH_INLINE_XXH3_64bits(node->Buffer->cData, node->Buffer->cSize);
    /* node->head */
    node->Next = wad->Entries;
    /* Set to first */
    wad->Entries = node;
    /* count + 1 */
    wad->Count++;

    /* Return true */
    return 1;
}

int W_Change(Wad *wad, uint64_t hash, void *buffer, size_t size)
{
    if (IsNULL(wad))
    {
        return -1;
    }
    /* node */
    WADEntry *entry;
    /* Exist ? */
    if ((entry = W_Find(wad, hash)))
    {
        /* Set the old buffer */
        Buffer *oldBuffer = entry->Buffer;
        /* create a new buffer */
        entry->Buffer = (Buffer *)w_malloc(sizeof(Buffer));

        /* Set the dSize */
        entry->Buffer->dSize = size;
        /* Set the dData */
        entry->Buffer->dData = w_malloc(size);
        /* copy the buffer to dData */
        memcpy(entry->Buffer->dData, buffer, size);

        /* Uncompressed */
        if (entry->Type == Uncompressed)
        {
            /* Set the cData */
            entry->Buffer->cData = w_malloc(size);
            /* Set the cSize*/
            entry->Buffer->cSize = size;
            /* Copy the buffer */
            memcpy(entry->Buffer->cData, buffer, size);
        } /* Gzip */
        else if (entry->Type == GZipCompressed)
        {
            /* Set the cSize */
            entry->Buffer->cSize = compressBound(size);
            /* malloc the cData */
            entry->Buffer->cData = w_malloc(entry->Buffer->cSize);
            /* compress */
            if (compress(entry->Buffer->cData, &entry->Buffer->cSize, buffer, size))
            {
                /* print error */
                printf("W_Change: \"compress failed : %" PRIu64 "\"\n", hash);
                /* free the buffer */
                free(entry->Buffer);
                /* failed. set to oldBuffer */
                entry->Buffer = oldBuffer;
                /* Return */
                return -1;
            }
        } /* Zstd */
        else if (entry->Type == ZStandardCompressed)
        {
            /* Set the cSize */
            entry->Buffer->cSize = ZSTD_compressBound(size);
            /* malloc the cData */
            entry->Buffer->cData = w_malloc(entry->Buffer->cSize);
            /* compress */
            ZSTD_compress(entry->Buffer->cData, entry->Buffer->cSize, buffer, size, 3);
        } /* Link */
        else
        {
            /* Set the cSize */
            entry->Buffer->cSize = size + 4; /* (int) 4 bytes + data size */
            /* malloc the cData */
            entry->Buffer->cData = w_malloc(size + 4);
            /* Copy the buffer to the dBuff */
            memcpy(entry->Buffer->cData, &size, 4); /* Set the Length, 4 bytes */
            /* Copy the buffer to the cData */
            memcpy(entry->Buffer->cData + 4, buffer, size); /* Set the data */
        }
        /* free the old buffer */
        free(oldBuffer);
        /* Set to null */
        oldBuffer = NULL;
        /* Set the compressed size */
        entry->CompressedSize = entry->Buffer->cSize;
        /* Set the uncompressed size */
        entry->UncompressedSize = size;
        /* Set the checksum */
        entry->Checksum = XXH_INLINE_XXH3_64bits(entry->Buffer->cData, entry->Buffer->cSize);
        /* Return true */
        return 1;
    }
    else
    {
        /* Return */
        return 0;
    }
}

WADEntry *W_Find(Wad *wad, uint64_t hash)
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

WADEntry *W_Find_Chceksum(Wad *wad, uint64_t checksum)
{
    if (IsNULL(wad))
    {
        return NULL;
    }
    /* Set the Head Node */
    WADEntry *temp = wad->Entries;
    /* Temp is not null and the hash does not match */
    while (temp && temp->Checksum != checksum)
    {
        /* to Next */
        /* if next is null. temp = next, and return temp */
        temp = temp->Next;
    }
    /* Return */
    return temp;
}

Buffer *W_GetBuffer(Wad *wad, uint64_t hash)
{
    if (IsNULL(wad))
    {
        return NULL;
    }
    WADEntry *entry;
    /* Find the entry */
    if ((entry = W_Find(wad, hash)))
    {
        /* The buffer is not NULL ? */
        if (entry->Buffer)
        {
            /* Return the buffer */
            return entry->Buffer;
        }
        /* Set the buffer*/
        entry->Buffer = w_malloc(sizeof(Buffer));
        /* Set the cSize */
        entry->Buffer->cSize = entry->CompressedSize;
        /* Set the cData */
        entry->Buffer->cData = w_malloc(entry->CompressedSize);
        /* Set the dSize */
        entry->Buffer->dSize = entry->UncompressedSize;
        /* Set the dData */
        entry->Buffer->dData = w_malloc(entry->UncompressedSize);

        /* Seek */
        fseek(wad->Buffer, entry->Offset, SEEK_SET);
        /* Read data to cData */
        fread(entry->Buffer->cData, entry->CompressedSize, 1, wad->Buffer);

        /* uncompressed */
        if (entry->Type == Uncompressed)
        {
            /* Set the dData */
            memcpy(entry->Buffer->dData, entry->Buffer->cData, entry->CompressedSize);
        }
        else
        {
            /* gzip */
            if (entry->Type == GZipCompressed)
            {
                /* gzip decompress */
                if (uncompress(entry->Buffer->dData, &entry->UncompressedSize, entry->Buffer->cData, entry->CompressedSize) != Z_OK)
                {
                    /* print erro */
                    printf("W_GetBuffer: \"uncompress failed : %" PRIu64 "\"\n");
                    /* free the buffer */
                    _free_entry_buffer(entry->Buffer);
                    /* Return */
                    return NULL;
                }
            } /* zstd */
            else if (entry->Type == ZStandardCompressed)
            {
                /* zstd decompress */
                ZSTD_decompress(entry->Buffer->dData, entry->UncompressedSize, entry->Buffer->cData, entry->CompressedSize);
            } /* link */
            else
            {
                /* copy the cData to dData. --- cData = (int) length + data */
                memcpy(entry->Buffer->dData, entry->Buffer->cData + 4, entry->CompressedSize - 4);
                /* 417. It is already set to full size */
                entry->Buffer->dSize -= 4;
            }
        }
        /* Return */
        return entry->Buffer;
    }
    /* Return */
    return NULL;
}

WADEntry *W_GetEntry(Wad *wad, int index)
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

void W_Remove(Wad *wad, uint64_t hash)
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
    /* invoke */
    W_ForEach(*wad, _free_entry);
    /* The buffer is not null ? */
    if ((*(wad))->Buffer)
    {
        /* Close the buffer */
        fclose((*wad)->Buffer);
    }
    /* free the wad */
    free(*wad);
    *wad = NULL;
}

void W_ForEach(Wad *wad, void (*func)(WADEntry *entry))
{
    /* The first node */
    WADEntry *node = wad->Entries;
    /* entry is not NULL */
    while (node)
    {
        /* Set the next */
        WADEntry *next = (node)->Next;
        /* invoke */
        (*func)(node);
        /* to Next */
        node = next;
    }
}

void W_WForEach(Wad *wad, void (*func)(Wad *wad, WADEntry *entry))
{
    /* The first node */
    WADEntry *node = wad->Entries;
    /* entry is not NULL */
    while (node)
    {
        /* Set the next */
        WADEntry *next = (node)->Next;
        /* invoke */
        (*func)(wad, node);
        /* to Next */
        node = next;
    }
}

void W_Write(Wad *wad, const wchar_t *path)
{
    /* Create a file */
    FILE *output = _wfopen(path, L"wb+,ccs=UNICODE");
    /* The output is not null ?? */
    if (output)
    {
        /* Write the magic code */
        fwrite(&wad->Magic, 2, 1, output);
        /* Write the major */
        fwrite(&wad->Major, 1, 1, output);
        /* Write the minor */
        fwrite(&wad->Minor, 1, 1, output);
        /* Write the signature */
        fwrite(&wad->Signature, 256, 1, output);
        /* Write the Pad */
        fwrite(&wad->Pad, 8, 1, output);
        /* Write the Count */
        fwrite(&wad->Count, 4, 1, output);

        /* hashes offset */
        long hOffset = ftell(output);
        /* hashes size */
        int hSize = 32 * wad->Count;

        /* data offset */
        long dataOffset = hOffset + hSize;
        /* go to the data offset */
        fseek(output, dataOffset, SEEK_SET);

        /* Checksum list */
        uint64_t *checksums = w_malloc(sizeof(uint64_t) * wad->Count);
        /* offset list */
        uint32_t *offsets = w_malloc(sizeof(uint32_t) * wad->Count);

        /* The index */
        int index = 0;

        /* Set to first */
        WADEntry *entry = wad->Entries;
        /* entry is not NULL */
        while (entry)
        {
            /* Get the buffer */
            W_GetBuffer(wad, entry->XXHash);
            /* The type is FileRedirection ? */
            if (entry->Type == FileRedirection)
            {
                /* Set the offset */
                entry->Offset = ftell(output);
                /* Write the cData */
                fwrite(entry->Buffer->cData, entry->Buffer->cSize, 1, output);
            }
            else
            {
                /* Set to zero */
                uint32_t offset = 0;
                /* Loop, Find duplicate */
                for (size_t i = 0; i < index; i++)
                {
                    /* if it matches */
                    if (checksums[i] == entry->Checksum)
                    {
                        /* Set the offset */
                        offset = offsets[i];
                        /* Break */
                        break;
                    }
                }
                /* If a match is found, offset is not zero */
                if (offset != 0)
                {
                    /* Set the offset */
                    entry->Offset = offset;
                    /* Is duplicated */
                    entry->IsDuplicated = 1;
                } /* No match found */
                else
                {
                    /* Set the offset */
                    entry->Offset = ftell(output);
                    /* Write the cData */
                    fwrite(entry->Buffer->cData, entry->Buffer->cSize, 1, output);

                    /* Set checksum to list */
                    checksums[index] = entry->Checksum;
                    /* Set offset to list */
                    offsets[index] = entry->Offset;
                }
            }
            /* index + 1 */
            index++;
            /* to Next */
            entry = entry->Next;
        }
        /* free the checksum list */
        free(checksums);
        /* free the offset list */
        free(offsets);

        /* go to the hashes offset */
        fseek(output, hOffset, SEEK_SET);
        /* Set to first */
        entry = wad->Entries;
        /* entry is not Null */
        while (entry)
        {
            /* Write the xxhash */
            fwrite(&entry->XXHash, 8, 1, output);
            /* Write the offset */
            fwrite(&entry->Offset, 4, 1, output);
            /* Write the compressed size */
            fwrite(&entry->CompressedSize, 4, 1, output);
            /* Write the uncompressed size */
            fwrite(&entry->UncompressedSize, 4, 1, output);
            /* Write the type */
            fwrite(&entry->Type, 1, 1, output);
            /* Write the is duplicated*/
            fwrite(&entry->IsDuplicated, 1, 1, output);
            /* Write the pad */
            fwrite(&entry->Pad, 2, 1, output);
            /* Write the checksum*/
            fwrite(&entry->Checksum, 8, 1, output);
            /* to Next */
            entry = entry->Next;
        }
        /* flush */
        fflush(output);
        /* Close the 'output' */
        fclose(output);
    }
}