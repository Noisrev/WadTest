#ifndef WAD_H
#define WAD_H

#include <stdint.h>

/* Wad entry type*/
enum EntryType
{
    /* Default */
    Uncompressed,
    /* Gzip */
    GZipCompressed,
    /* Link */
    FileRedirection,
    /* Zstd */
    ZStandardCompressed
};

// Wad entry
typedef struct WADEntry
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
    /* Next entry */
    WADEntry* Next;
};

// Riot wad file.
typedef struct WADFile
{
    /* Magic Code */
    char* Magic;
    /* Major version */
    int8_t Major;
    /* Minor version */
    int8_t Minor;
    /* Wad Signature*/
    char* Signature;
    /* What is this? */
    uint64_t Pad;
    /* Collection size */
    uint32_t Count;
    /* A collection of wad entry */
    WADEntry* Entries; 
};

// Load the WAD from the path.
static int LoadWadFromPath(WADFile* wad, const char* path);

// Load the WAD from the buffer.
static int LoadWadFromBuffer(WADFile* wad, void* buffer, size_t size);

// Add a new entry to WAD Entries.
static int AddWadEntry(WADFile* wad, void* buffer, size_t size, EntryType type);

// Change the data from the entry that matches the hash.
static int ChangeWadEntry(WADFile* wad, uint64_t hash, void* buffer, size_t size);

// Find the entry that matches the hash.
static WADEntry* FindWadEntry(WADFile* wad, uint64_t hash);

// Get the entry using the index.
static WADEntry* GetWadEntryWithIndex(WADFile* wad, int index);

// Removes all items in the collection that match the hash.
static int RemoveWadEntry(WADFile* wad, uint64_t hash);

#endif