#ifndef WAD_H
#define WAD_H

#define XXH_INLINE_ALL

#include <stdio.h>
#include "xxhash.h"
/* Wad entry type*/
typedef enum W_Type
{
    /* Default */
    Uncompressed,
    /* Gzip */
    GZipCompressed,
    /* Link */
    FileRedirection,
    /* Zstd */
    ZStandardCompressed
} EntryType;

// The Buffer
typedef struct W_Buffer
{
    /* compressed */
    void *cData;
    /* compressed size*/
    uint32_t cSize;
    /* uncompressed */
    void *dData;
    /* uncompressed size*/
    uint32_t dSize;
} Buffer;

// Wad entry
typedef struct _W_Entry
{
    /* Wad entry hash */
    uint64_t XXHash;
    /* Wad entry offset */
    uint32_t Offset;
    /* The compressed size*/
    uint32_t CompressedSize;
    /* The uncompressed size*/
    uint32_t UncompressedSize;
    /* Entry type*/
    EntryType Type;
    /* 0 or 1*/
    int8_t IsDuplicated;
    /* pad */
    uint16_t Pad;
    /* check sum*/
    uint64_t Checksum;
    /* Data */
    Buffer *Buffer;
    /* Next entry */
    struct _W_Entry *Next;
} WADEntry;

// Riot wad file.
typedef struct _WADFile
{
    /* Magic Code */
    char Magic[2];
    /* Major version */
    int8_t Major;
    /* Minor version */
    int8_t Minor;
    /* Wad Signature*/
    char Signature[256];
    /* What is this? */
    uint64_t Pad;
    /* Collection size */
    int Count;
    /* A collection of wad entry */
    WADEntry *Entries;
    /* Wad data */
    FILE *Buffer;
} Wad;

// Create an empty WAD
Wad *W_Create();

// Load the WAD from the path.
Wad *W_Open(const wchar_t *path);

// Add a new entry to WAD Entries.
int W_Add(Wad *wad, uint64_t hash, void *buffer, size_t size, EntryType type);

// Change the data from the entry that matches the hash.
int W_Change(Wad *wad, uint64_t hash, void *buffer, size_t size);

// Find the entry that matches the hash.
WADEntry *W_Find(Wad *wad, uint64_t hash);

// Get the content of the buffer for the specified hash.
Buffer *W_GetBuffer(Wad *wad, uint64_t hash);

// Get the entry using the index.
WADEntry *W_GetEntry(Wad *wad, int index);

// Removes all items in the collection that match the hash.
void W_Remove(Wad *wad, uint64_t hash);

/* Close the wad */
void W_Close(Wad **wad);

/* Foreach in wad entries. */
void W_ForEach(Wad *wad, void (*func)(WADEntry *entry));

/* Foreach in wad entries. Attach the Wad* Wad parameter. */
void W_WForEach(Wad *wad, void (*func)(Wad *wad, WADEntry *entry));

/* Write Wad File */
void W_Write(Wad *wad, const wchar_t *path);

#endif