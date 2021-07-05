#ifndef WAD_H
#define WAD_H

#include <stdint.h>

/* Wad entry type*/
typedef enum EntryType
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
    void* Buffer;
    /* Next entry */
    struct W_Entry *Next;
} WADEntry;

// Riot wad file.
typedef struct WADFile
{
    /* Magic Code */
    char *Magic;
    /* Major version */
    int8_t Major;
    /* Minor version */
    int8_t Minor;
    /* Wad Signature*/
    char *Signature;
    /* What is this? */
    uint64_t Pad;
    /* Collection size */
    int Count;
    /* A collection of wad entry */
    WADEntry *Entries;
} Wad;

// Load the WAD from the path.
int LoadWadFromPath(Wad *wad, const wchar_t *path);

// Load the WAD file from handle.
int LoadWadFromHandle(Wad *wad, int handle);

// Add a new entry to WAD Entries.
int AddWadEntry(Wad *wad, void *buffer, size_t size, EntryType type);

// Change the data from the entry that matches the hash.
int ChangeWadEntry(Wad *wad, uint64_t hash, void *buffer, size_t size);

// Find the entry that matches the hash.
WADEntry* FindWadEntry(Wad *wad, uint64_t hash);

// Get the entry using the index.
WADEntry* GetWadEntryWithIndex(Wad *wad, int index);

// Removes all items in the collection that match the hash.
int RemoveWadEntry(Wad *wad, uint64_t hash);

#endif