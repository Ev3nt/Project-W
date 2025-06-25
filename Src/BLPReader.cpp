#include "BLPReader.h"

#include <jpeglib.h>
#include <fstream>

DataChunk LoadJpegFromMemory(const DataChunk& jpeg, int& width, int& height, int& channels) {
    jpeg_decompress_struct cinfo;
    jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    jpeg_mem_src(&cinfo, jpeg.data(), (uint32_t)jpeg.size());

    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    width = cinfo.output_width;
    height = cinfo.output_height;
    channels = cinfo.output_components;

    const size_t row_stride = width * channels;
    DataChunk image(row_stride * height);

    while ((int)cinfo.output_scanline < height) {
        uint8_t* row_pointer = &image[cinfo.output_scanline * row_stride];
        uint32_t lines_read = jpeg_read_scanlines(&cinfo, &row_pointer, 1);

        if (!lines_read) {
            throw std::runtime_error("Failed to read JPEG scanlines");
        }
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    return image;
}

std::string GetSignature(const DataChunk& blp) {
    return std::string(blp.begin(), blp.begin() + sizeof(uint32_t));
}

bool CheckSignature(const DataChunk& blp) {
    return GetSignature(blp) == "BLP1";
}

DataChunk LoadBLPFromMemory(const DataChunk& blp, int& width, int& height, int& channels) {
    if (blp.size() < sizeof(BLP1Header)) {
        throw std::runtime_error("BLP file too small for header");
    }

    if (!CheckSignature(blp)) {
        throw std::runtime_error("Unsupported BLP format: " + GetSignature(blp));
    }

    const BLP1Header& header = *(BLP1Header*)blp.data();
    if (header.compressionType != 0) {
        throw std::runtime_error("Unsupported compression type: " + std::to_string(header.compressionType));
    }

    constexpr size_t mipmapNumber = 0;
    if (header.mipmapOffsets[mipmapNumber] + header.mipmapLengths[mipmapNumber] > blp.size()) {
        throw std::runtime_error("BLP mipmap data out of bounds");
    }

    const size_t headerSize = sizeof(header);
    const uint32_t jpegHeaderSize = *(uint32_t*)&blp[headerSize];

    if (headerSize + sizeof(jpegHeaderSize) + jpegHeaderSize > blp.size()) {
        throw std::runtime_error("JPEG header size exceeds BLP file bounds");
    }

    DataChunk jpeg(jpegHeaderSize + header.mipmapLengths[mipmapNumber]);

    memcpy(
        jpeg.data(),
        blp.data() + headerSize + sizeof(jpegHeaderSize),
        jpegHeaderSize
    );

    memcpy(
        jpeg.data() + jpegHeaderSize,
        blp.data() + header.mipmapOffsets[mipmapNumber],
        header.mipmapLengths[mipmapNumber]
    );

    DataChunk image = LoadJpegFromMemory(jpeg, width, height, channels);

    if (channels >= 3) {
        for (size_t i = 0; i < image.size(); i += channels) {
            std::swap(image[i], image[i + 2]);
        }
    }

    return image;
}

DataChunk LoadBLP(const std::string& fileName, int& width, int& height, int& channels) {
    std::ifstream file(fileName, std::ios::binary);

    if (!file) {
        throw std::runtime_error("Couldn't open file: " + fileName);
    }

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    DataChunk blp(size);
    if (!file.read((char*)blp.data(), size)) {
        throw std::runtime_error("Failed to read file: " + fileName);
    }

    return LoadBLPFromMemory(blp, width, height, channels);
}