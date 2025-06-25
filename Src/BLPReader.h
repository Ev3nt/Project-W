#pragma once

#include <vector>
#include <string>

struct BLP1Header {
    char magic[4];
    uint32_t compressionType;
    uint32_t alphaBits;
    uint32_t width;
    uint32_t height;
    uint32_t type;
    uint32_t hasMipmaps;
    uint32_t mipmapOffsets[16];
    uint32_t mipmapLengths[16];
};

typedef std::vector<uint8_t> DataChunk;

DataChunk LoadBLPFromMemory(const DataChunk& blp, int& width, int& height, int& channels);

DataChunk LoadBLP(const std::string& fileName, int& width, int& height, int& channels);