#ifndef WAD_H
#define WAD_H

/* Compressed type */
#define R_Compressed 1
/* Uncompressed type */
#define R_Uncmpressed 0

#include <stdio.h>
#include <stdint.h>
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
    /* Data */
    void *Cache;
    /* Size */
    size_t Size;
} Buffer;

// Wad entry
typedef struct W_Entry
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
    void *Buffer;
    /* Next entry */
    struct W_Entry *Next;
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

// Load the WAD from the path.
Wad *LoadWadFromPath(const wchar_t *path);

// Add a new entry to WAD Entries.
int AddWadEntry(Wad *wad, uint64_t hash, void *buffer, size_t size, EntryType type);

// Change the data from the entry that matches the hash.
int ChangeWadEntry(Wad *wad, uint64_t hash, void *buffer, size_t size);

// Find the entry that matches the hash.
WADEntry *FindWadEntry(Wad *wad, uint64_t hash);

// Get the content of the buffer for the specified hash.
Buffer *GetBuffer(Wad *wad, uint64_t hash, int R_Comp);

// Get the entry using the index.
WADEntry *GetWadEntryWithIndex(Wad *wad, int index);

// Removes all items in the collection that match the hash.
void RemoveWadEntry(Wad *wad, uint64_t hash);

#endif