// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_STORAGE_ARCHIVE_HPP
#define OUZEL_STORAGE_ARCHIVE_HPP

#include <cstdint>
#include <fstream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include "../utils/Utils.hpp"

namespace ouzel
{
    namespace storage
    {
        class Archive final
        {
        public:
            Archive() = default;

            explicit Archive(const std::string& path):
                file{path, std::ios::binary}
            {
                constexpr std::uint32_t centralDirectory = 0x02014B50U;
                constexpr std::uint32_t headerSignature = 0x04034B50U;

                for (;;)
                {
                    std::uint8_t signatureData[4];
                    file.read(reinterpret_cast<char*>(signatureData), sizeof(signatureData));

                    if (decodeLittleEndian<std::uint32_t>(signatureData) == centralDirectory)
                        break;

                    if (decodeLittleEndian<std::uint32_t>(signatureData) != headerSignature)
                        throw std::runtime_error("Bad signature");

                    file.seekg(2, std::ios::cur); // skip version
                    file.seekg(2, std::ios::cur); // skip flags

                    std::uint8_t compressionData[2];
                    file.read(reinterpret_cast<char*>(&compressionData), sizeof(compressionData));

                    if (decodeLittleEndian<std::uint16_t>(compressionData) != 0x00)
                        throw std::runtime_error("Unsupported compression");

                    file.seekg(2, std::ios::cur); // skip modification time
                    file.seekg(2, std::ios::cur); // skip modification date
                    file.seekg(4, std::ios::cur); // skip CRC-32

                    std::uint8_t compressedSizeData[4];
                    file.read(reinterpret_cast<char*>(compressedSizeData), sizeof(compressedSizeData));

                    std::uint8_t uncompressedSizeData[4];
                    file.read(reinterpret_cast<char*>(uncompressedSizeData), sizeof(uncompressedSizeData));
                    const size_t uncompressedSize = decodeLittleEndian<std::uint32_t>(uncompressedSizeData);

                    std::uint8_t fileNameLengthData[2];
                    file.read(reinterpret_cast<char*>(fileNameLengthData), sizeof(fileNameLengthData));
                    const size_t fileNameLength = decodeLittleEndian<std::uint16_t>(fileNameLengthData);

                    std::uint8_t extraFieldLengthData[2];
                    file.read(reinterpret_cast<char*>(extraFieldLengthData), sizeof(extraFieldLengthData));
                    const size_t extraFieldLength = decodeLittleEndian<std::uint16_t>(extraFieldLengthData);

                    auto name = std::make_unique<char[]>(fileNameLength + 1); // +1 for null character

                    file.read(name.get(), static_cast<std::streamsize>(fileNameLength));

                    Entry& entry = entries[name.get()];
                    entry.size = uncompressedSize;

                    file.seekg(static_cast<std::streamoff>(extraFieldLength),
                               std::ios::cur); // skip extra field

                    entry.offset = file.tellg();

                    file.seekg(static_cast<std::streamoff>(uncompressedSize),
                               std::ios::cur); // skip uncompressed size
                }
            }

            std::vector<std::byte> readFile(const std::string& filename)
            {
                auto i = entries.find(filename);

                if (i == entries.end())
                    throw std::runtime_error("File " + filename + " does not exist");

                file.seekg(i->second.offset, std::ios::beg);

                std::vector<std::byte> data(i->second.size);
                file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(i->second.size));

                return data;
            }

            bool fileExists(const std::string& filename) const
            {
                return entries.find(filename) != entries.end();
            }

        private:
            std::ifstream file;

            struct Entry final
            {
                std::streamoff offset;
                std::size_t size;
            };

            std::map<std::string, Entry> entries;
        };
    } // namespace storage
} // namespace ouzel

#endif // OUZEL_STORAGE_ARCHIVE_HPP
